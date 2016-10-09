#include <assert.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>

#include "mbaacc_pack.h"
#include "util/fatal.h"
#include "util/mkdirs.h"
#include "util/split.h"

const char* progname;

#if defined(_WIN32)
#define PATH_SEPARATOR "\\"
#define TEMP_FAILURE_RETRY(x) x
#else
#define PATH_SEPARATOR "/"
#endif

[[noreturn]] static void usage(int exit_code) {
  fprintf(stderr, "usage: %s [OPTION]... PACK_FILE...\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h\t\t\tdisplays this message\n");
  fprintf(stderr, "  -x\t\t\textract the contents of the specified pack file(s)\n");
  fprintf(stderr, "  -t\t\t\tlist the contents of the specified pack file(s)\n");
  fprintf(stderr, "  -d\t\t\tdump pack file headers\n");
  fprintf(stderr, "  -o OUTPUT_DIR\t\textract files to a specified output directory\n");
  fprintf(stderr, "  -s FILE_NAME\t\tonly extract or list specified files (can be repeated)\n");
  exit(exit_code);
}

int main(int argc, char* argv[]) {
  progname = basename(argv[0]);

  std::vector<gsl::zstring> pack_file_paths;

  bool extract = false;
  bool list = false;
  bool dump = false;

  std::string root;

  {
    gsl::zstring output_dir_opt = nullptr;
    int opt;
    while ((opt = getopt(argc, argv, "hxtdo:")) != -1) {
      switch(opt) {
        case 'h':
          usage(0);
        case 'x':
          extract = true;
          break;
        case 't':
          list = true;
          break;
        case 'd':
          dump = true;
          break;
        case 'o':
          if (output_dir_opt) {
            fprintf(stderr, "%s: -o specified multiple times\n", progname);
          }
          output_dir_opt = optarg;
          break;
        default:
          usage(1);
      }
    }
    if (!output_dir_opt) {
      root = ".";
    } else {
      // Make sure that the parent of the output dir exists.
      if (!directory_exists(dirname(output_dir_opt))) {
        fatal_errno("parent of output directory must be a directory");
      }
      root = output_dir_opt;
    }
  }

  if (extract + list + dump != 1) {
    fprintf(stderr, "%s: exactly one of options -x, -t and -d must be specified\n", progname);
    usage(1);
  }

  if (optind == argc) {
    usage(0);
  }

  // Check if we can access all of the files before we try to open any of them.
  for (int i = optind; i < argc; ++i) {
    pack_file_paths.push_back(argv[i]);
    if (access(argv[i], R_OK)) {
      fatal_errno("can't access '%s'", argv[i]);
    }
  }

  std::vector<mbaacc::Pack> packs;
  for (gsl::zstring pack_file_path : pack_file_paths) {
    FILE* file = fopen(pack_file_path, "rb");
    if (!file) {
      fatal_errno("failed to open '%s'", pack_file_path);
    }
    packs.emplace_back(unique_fd(dup(fileno(file))));
    fclose(file);
  }

  // TODO: Close packs that we're finished with.
  for (size_t pack_index = 0; pack_index < packs.size(); ++pack_index) {
    gsl::zstring pack_file_name = basename(pack_file_paths[pack_index]);
    mbaacc::Pack& pack = packs[pack_index];

    if (dump) {
      pack.header().dump(pack_file_name);
    }

    gsl::span<mbaacc::FolderIndex> folders = pack.folders();
    gsl::span<mbaacc::FileIndex> files = pack.files();

    for (size_t folder_index = 0; folder_index < folders.size(); ++folder_index) {
      mbaacc::FolderIndex& folder = folders[folder_index];
      std::vector<gsl::cstring_span> dir_path_fragments =
          Split(gsl::cstring_span(folder.filename), "\\");

      // Do some validation if we're extracting.
      if (extract) {
        for (gsl::cstring_span fragment : dir_path_fragments) {
          // Not strictly correct, the field could hypothetically contain "foo\0/".
          if (memchr(fragment.data(), fragment.length(), '/')) {
            fatal("directory name contains slash: %.*s", int(sizeof(folder.filename)),
                  folder.filename);
          }

          if (strcmp(fragment.data(), "..") == 0) {
            fatal("directory name contains '..': %.*s", int(sizeof(folder.filename)),
                  folder.filename);
          }
        }
      }

      std::string normalized_dir_path = Join(dir_path_fragments, PATH_SEPARATOR);
      std::string output_dir = root + PATH_SEPARATOR;
      if (!normalized_dir_path.empty()) {
        output_dir += normalized_dir_path + PATH_SEPARATOR;
      }

      if (list) {
        printf("%-12s  dir %10u %.*s\\\n", pack_file_name, 0u, int(sizeof(folder.filename)),
               folder.filename);
      } else if (dump) {
        folder.dump(folder_index);
      } else if (extract) {
        if (!mkdirs(output_dir)) {
          fatal_errno("failed to create directory '%s'", output_dir.c_str());
        }
      }

      // We can't trust the FolderIndex entries to be accurate, so we need to iterate through the
      // files at least once. The file counts are generally pretty low, so just do this repeatedly.
      for (size_t file_index = 0; file_index < files.size(); ++file_index) {
        mbaacc::FileIndex& file = files[file_index];
        if (file.folder_id != folder_index) {
          continue;
        }

        if (list) {
          printf("%-12s file %10u %.*s\\%.*s\n", pack_file_name, file.size,
                 int(sizeof(folder.filename)), folder.filename, int(sizeof(file.filename)),
                 file.filename);
        } else if (dump) {
          file.dump(file_index);
        } else if (extract) {
          printf("%.*s\\%.*s\n", int(sizeof(folder.filename)), folder.filename,
                 int(sizeof(file.filename)), file.filename);
          std::string file_path = output_dir + PATH_SEPARATOR;
          file_path.append(file.filename, strnlen(file.filename, sizeof(file.filename)));

          gsl::span<char> file_data = pack.file_data(file_index);
          FILE* f = fopen(file_path.c_str(), "wb");
          if (fwrite(file_data.data(), 1, file_data.length(), f) != file_data.length()) {
            fatal("failed to write file");
          }
          fclose(f);
        }
      }
    }
  }
}
