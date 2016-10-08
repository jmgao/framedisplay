#pragma once

#if defined(_WIN32)
#include <windef.h>
#endif

#include <stdint.h>
#include <stdio.h>

#include <vector>
#include <gsl.h>

#include "util/unique_fd.h"

namespace mbaacc {
#pragma pack(push, 1)
struct PackHeader {
  char name[16];
  uint32_t flag;
  uint32_t xor_key;
  uint32_t data_offset;
  uint32_t data_size;
  uint32_t folder_count;
  uint32_t file_count;
  uint32_t unknown[2];
  uint32_t encrypted_length;

  void dump(FILE* out = stderr) const {
    fprintf(out, "PackHeader:\n");
    fprintf(out, "  name: %.*s\n", int(sizeof(name)), name);
    fprintf(out, "  flag: %#x\n", flag);
    fprintf(out, "  xor_key: %#x\n", xor_key);
    fprintf(out, "  data_offset: %u\n", data_offset);
    fprintf(out, "  data_size: %u\n", data_size);
    fprintf(out, "  folder_count: %u\n", folder_count);
    fprintf(out, "  file_count: %u\n", file_count);
    fprintf(out, "  unknown[0]: %#x\n", unknown[0]);
    fprintf(out, "  unknown[1]: %#x\n", unknown[1]);
    fprintf(out, "  encrypted_length: %#x\n", encrypted_length);
  }
};

struct FolderIndex {
  uint32_t offset;
  uint32_t file_start_id;
  uint32_t size;
  char filename[256];

  void dump(FILE* out = stderr) const {
    fprintf(out, "FolderIndex:\n");
    fprintf(out, "  name: %.*s\n", int(sizeof(filename)), filename);
    fprintf(out, "  offset: %u\n", offset);
    fprintf(out, "  file_start_id: %u\n", file_start_id);
    fprintf(out, "  size: %u\n", size);
  }
};

struct FileIndex {
  uint32_t offset;
  uint32_t folder_id;
  uint32_t size;
  char filename[32];

  void dump(uint32_t file_id = UINT32_MAX, FILE* out = stderr) const {
    if (file_id == UINT32_MAX) {
      fprintf(out, "FileIndex(?):\n");
    } else {
      fprintf(out, "FileIndex(%u):\n", file_id);
    }

    fprintf(out, "  name: %.*s\n", int(sizeof(filename)), filename);
    fprintf(out, "  offset: %u\n", offset);
    fprintf(out, "  folder_id: %u\n", folder_id);
    fprintf(out, "  size: %u\n", size);
  }
};
#pragma pack(pop)

class Pack {
 public:
  explicit Pack(unique_fd fd);
  Pack(const Pack& copy) = delete;
  Pack(Pack&& move);
  ~Pack();

  Pack& operator=(const Pack& copy) = delete;
  Pack& operator=(Pack&& move);

  PackHeader& header() const;
  gsl::span<FolderIndex> folders() const;
  gsl::span<FileIndex> files() const;

  gsl::span<char> file_data(uint32_t file_id);

 private:
  unique_fd fd_;
  size_t pack_file_size_;
  char* pack_file_data_;

  std::vector<bool> file_decrypted_;

#if defined(_WIN32)
  HANDLE file_mapping_;
#endif
};
} /* namespace mbaacc */
