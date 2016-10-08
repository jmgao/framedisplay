#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>

#include "mbaacc_pack.h"
#include "util/fatal.h"

const char* progname;

#if defined(_WIN32)
#define TEMP_FAILURE_RETRY(x) x
static const char* basename(const char* name) {
  size_t len = strlen(name);
  const char* end = name + len;
  auto rbegin = std::reverse_iterator<const char*>(end);
  auto rend = std::reverse_iterator<const char*>(name);
  auto it = std::find(rbegin, rend, '\\');
  if (it == rend) {
    return name;
  }
  return &*it + 1;
}
#endif

int main(int argc, char* argv[]) {
  progname = basename(argv[0]);

  if (argc < 2 || argc > 4) {
    fprintf(stderr, "usage: %s FILE.p [FILE_TO_EXTRACT] [OUTPUT FILE NAME]\n", progname);
    exit(1);
  }

  char* filename = argv[1];
  FILE* file = fopen(filename, "rb");

  if (!file) {
    fatal_errno("failed to open '%s'", filename);
  }

  mbaacc::Pack pack(unique_fd(dup(fileno(file))));

  gsl::span<mbaacc::FolderIndex> folders = pack.folders();
  gsl::span<mbaacc::FileIndex> files = pack.files();
  if (argc == 2) {
    pack.header().dump();
    for (const auto& folder : folders) {
      folder.dump();
    }

    for (size_t i = 0; i < files.size(); ++i) {
      files[i].dump(i);
    }
  } else {
    for (size_t i = 0; i < files.size(); ++i) {
      if (strcmp(argv[2], files[i].filename) == 0) {
        uint32_t folder_id = files[i].folder_id;
        if (folder_id >= folders.size()) {
          fatal("file %zu references a non-existent folder", i);
        }

        gsl::span<char> file_data = pack.file_data(i);
        size_t len = file_data.size();
        const char* p = file_data.begin();

        std::string filename(files[i].filename, sizeof(files[i].filename));
        const char* output_path = basename(filename.c_str());
        if (argc == 4) {
          output_path = argv[3];
        }

        FILE* out = fopen(output_path, "wb");
        if (!out) {
          fatal_errno("failed to open output path '%s'", output_path);
        }

        fprintf(stderr, "Saving %.*s\\%.*s to %s\n", int(sizeof(folders[folder_id].filename)),
                folders[folder_id].filename, int(sizeof(files[i].filename)), files[i].filename,
                output_path);

        fwrite(p, 1, len, out);
        fclose(out);
        exit(0);
      }
    }
    fatal("%s: failed to find '%s'", progname, argv[2]);
  }
}
