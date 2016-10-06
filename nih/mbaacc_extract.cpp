#include <assert.h>
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <memory>

static void decrapt(char* data, uint32_t size, uint32_t xorkey, uint32_t xormod) {
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

  // Only applies to the first 4096 bytes of data.
  if (size > 4096) {
    size = 4096;
  }

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
#pragma pack(push, 1)
struct PackHeader {
  char name[16];
  uint32_t flag;
  uint32_t xor_key;
  uint32_t data_offset;
  uint32_t data_size;
  uint32_t folder_count;
  uint32_t file_count;
  uint32_t unknown[3];

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
    fprintf(out, "  unknown[2]: %#x\n", unknown[2]);
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
}

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "usage: %s FILE.p [FILE_TO_EXTRACT]\n", argv[0]);
    exit(1);
  }

  char* filename = argv[1];
  FILE* file = fopen(filename, "rb");

  if (!file) {
    err(1, "failed to open '%s'", filename);
  }

  mbaacc::PackHeader header;
  assert(fread(&header, sizeof(header), 1, file) == 1);
  header.dump();

  auto folders = std::make_unique<mbaacc::FolderIndex[]>(header.folder_count);
  assert(folders);
  assert(fread(folders.get(), sizeof(*folders.get()), header.folder_count, file) ==
         header.folder_count);
  for (size_t i = 0; i < header.folder_count; ++i) {
    decrapt(folders[i].filename, sizeof(folders[i].filename), header.xor_key, folders[i].size);
    folders[i].dump();
  }

  auto files = std::make_unique<mbaacc::FileIndex[]>(header.file_count);
  assert(fread(files.get(), sizeof(*files.get()), header.file_count, file) == header.file_count);

  for (size_t i = 0; i < header.file_count; ++i) {
    decrapt(files[i].filename, sizeof(files[i].filename), header.xor_key, files[i].size);
    files[i].dump(i);
  }

  off_t current_offset = ftello(file);
  assert(current_offset == header.data_offset);

  if (argc == 3) {
    for (size_t i = 0; i < header.file_count; ++i) {
      if (strcmp(argv[2], files[i].filename) == 0) {
        fseek(file, header.data_offset + files[i].offset, SEEK_SET);
        auto buf = std::make_unique<char[]>(files[i].size);
        assert(fread(buf.get(), 1, files[i].size, file) == files[i].size);
        decrapt(buf.get(), files[i].size, header.xor_key, 0x03 /* ??? */);

        char* p = buf.get();
        size_t len = files[i].size;
        while (len > 0) {
          ssize_t rc = TEMP_FAILURE_RETRY(write(STDOUT_FILENO, p, len));
          if (rc < 0) {
            err(1, "write failed");
          }
          len -= rc;
        }
        exit(0);
      }
    }
    errx(1, "failed to find '%s'", argv[2]);
  }
}
