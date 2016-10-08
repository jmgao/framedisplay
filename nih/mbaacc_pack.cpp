#include "mbaacc_pack.h"

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include <utility>

#include <gsl.h>

#include "util/fatal.h"
#include "util/unique_fd.h"

static void decrapt(char* data, uint32_t size, uint32_t encrypted_length, uint32_t xorkey,
                    uint32_t xormod) {
  union {
    uint32_t key;
    struct {
      unsigned char a;
      unsigned char b;
      unsigned char c;
      unsigned char d;
    } b;
  } key_a;
  uint32_t key_b;

  key_a.key = xorkey;

  key_b = xormod & 0xff;
  if (key_b == 0) {
    key_b = 1;
  }

  size = std::min(size, encrypted_length);

  uint32_t* p = (uint32_t*)data;
  size = (size + 3) / 4;
  for (size_t i = 0; i < size; ++i) {
    *p++ ^= key_a.key;

    key_a.b.a += key_b;
    key_a.b.b += key_b;
    key_a.b.c += key_b;
    key_a.b.d += key_b;
  }
}

namespace mbaacc {
Pack::Pack(unique_fd _) : fd_(std::move(_)) {
  struct stat st;
  if (fstat(fd_.get(), &st) != 0) {
    fatal_errno("stat failed while constructing mbaacc::Pack");
  }

  pack_file_size_ = st.st_size;

#if defined(_WIN32)
  HANDLE file_handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd_.get()));
  file_mapping_ = CreateFileMapping(file_handle, nullptr, PAGE_WRITECOPY, 0, 0, nullptr);
  pack_file_data_ =
      static_cast<char*>(MapViewOfFile(file_mapping_, FILE_MAP_COPY, 0, 0, pack_file_size_));
#else
  pack_file_data_ = static_cast<char*>(
      mmap(nullptr, pack_file_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_.get(), 0));
#endif

  // Sanity check lengths.
  size_t expected_size = sizeof(PackHeader);
  if (pack_file_size_ < expected_size) {
    fatal("pack file not long enough to contain a PackHeader");
  }

  uint32_t folder_count = header().folder_count;
  uint32_t file_count = header().file_count;

  if (folder_count > UINT16_MAX) {
    fatal("unexpectedly high number of folders: %u", folder_count);
  } else if (file_count > UINT16_MAX) {
    fatal("unexpectedly high number of files: %u", file_count);
  }

  expected_size += sizeof(FolderIndex) * folder_count + sizeof(FileIndex) * file_count;
  if (pack_file_size_ < expected_size) {
    fatal("pack file not long enough to contain all Folder/FileIndexes");
  }

  if (expected_size != header().data_offset) {
    fatal("pack file has gap between last file index and data");
  }

  expected_size += header().data_size;
  if (pack_file_size_ != expected_size) {
    fatal("pack file expected to be %zu bytes long (actually %zu)", expected_size, pack_file_size_);
  }

  // Decrypt FolderIndexes.
  for (FolderIndex& folder : folders()) {
    decrapt(folder.filename, sizeof(folder.filename), header().encrypted_length, header().xor_key,
            folder.size);
  }

  // Decrypt FileIndexes.
  for (FileIndex& file : files()) {
    decrapt(file.filename, sizeof(file.filename), header().encrypted_length, header().xor_key,
            file.size);
  }

  // Decrypt files on demand.
  file_decrypted_.resize(header().file_count);
}

Pack::~Pack() {
#if defined(_WIN32)
  UnmapViewOfFile(pack_file_data_);
  CloseHandle(file_mapping_);
#else
  munmap(pack_file_data_, pack_file_size_);
#endif
}

PackHeader& Pack::header() const { return *reinterpret_cast<PackHeader*>(pack_file_data_); }

gsl::span<FolderIndex> Pack::folders() const {
  FolderIndex* folder_start = reinterpret_cast<FolderIndex*>(pack_file_data_ + sizeof(PackHeader));
  return gsl::span<FolderIndex>(folder_start, header().folder_count);
}

gsl::span<FileIndex> Pack::files() const {
  FileIndex* file_start = reinterpret_cast<FileIndex*>(pack_file_data_ + sizeof(PackHeader) +
                                                       header().folder_count * sizeof(FolderIndex));
  return gsl::span<FileIndex>(file_start, header().file_count);
}

gsl::span<char> Pack::file_data(uint32_t file_id) {
  // Make sure it's in range.
  if (file_id >= header().file_count) {
    fatal("file id %u out of range (file_count = %u)", file_id, header().file_count);
  }

  uint32_t file_offset = files()[file_id].offset;
  uint32_t file_size = files()[file_id].size;

  // Sanity check the file's borders.
  if (file_offset + file_size < file_offset) {
    fatal("file %u overflows (offset = %u, size = %u)", file_id, file_offset, file_size);
  }
  if (file_offset + file_size > pack_file_size_) {
    fatal("file %u extends past the end of the pack file", file_id);
  }

  if (file_id != 0) {
    uint32_t prev_offset = files()[file_id - 1].offset;
    uint32_t prev_size = files()[file_id - 1].size;
    if (file_offset < prev_offset + prev_size) {
      fatal("file %u overlaps with the previous file (%u = [%#x-%#x], %u=[%#x-%#x])", file_id,
            file_id - 1, prev_offset, prev_offset + prev_size - 1, file_id, file_offset,
            file_offset + file_size - 1);
    }
  }
  if (file_id != header().file_count - 1) {
    uint32_t next_offset = files()[file_id + 1].offset;
    uint32_t next_size = files()[file_id + 1].size;
    if (file_offset + file_size > next_offset) {
      fatal("file %u overlaps with the next file (%u = [%#x-%#x], %u=[%#x-%#x])", file_id, file_id,
            file_offset, file_offset + file_size - 1, file_id + 1, next_offset,
            next_offset + next_size - 1);
    }
  }

  char* start = pack_file_data_ + header().data_offset + file_offset;

  // Decrypt the file if needed.
  if (!file_decrypted_[file_id]) {
    decrapt(start, file_size, header().encrypted_length, header().xor_key, 0x03 /* ??? */);
    file_decrypted_[file_id] = true;
  }

  return gsl::span<char>(start, file_size);
}
}