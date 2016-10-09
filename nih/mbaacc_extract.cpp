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

enum class Operation {
  NONE,
  EXTRACT,
  LIST,
  DUMP,
  UPDATE,
};

[[noreturn]] static void usage(int exit_code) {
  fprintf(stderr, "usage: %s [OPTION]... PACK_FILE...\n", progname);
  fprintf(stderr, "Operation mode:\n");
  fprintf(stderr, "  -x\t\t\textract the contents of the specified pack file(s)\n");
  fprintf(stderr, "  -t\t\t\tlist the contents of the specified pack file(s)\n");
  // fprintf(stderr, "  -c DIR\t\tcreate new packfile with changed files in DIR (experimental)\n");
  fprintf(stderr, "  -u DIR\t\tupdate packfile with changed files in DIR (experimental)\n");
  fprintf(stderr, "  -d\t\t\tdump pack file headers\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o DIR/FILE\t\tspecify output directory for -x, output file for -u\n");
  fprintf(stderr, "  -h\t\t\tdisplays this message\n");
  exit(exit_code);
}

static void check_op(Operation* op) {
  if (*op != Operation::NONE) {
    fprintf(stderr, "%s: exactly one operation mode must be specified\n", progname);
    usage(1);
  }
}

static bool parse_args(int argc, char* argv[], Operation* op, std::string* output_path,
                       std::vector<gsl::zstring>* pack_file_paths,
                       std::string* update_output_path) {
  gsl::zstring output_dir_opt = nullptr;
  int opt;
  while ((opt = getopt(argc, argv, "xtu:do:h")) != -1) {
    switch (opt) {
      case 'x':
        check_op(op);
        *op = Operation::EXTRACT;
        break;
      case 't':
        check_op(op);
        *op = Operation::LIST;
        break;
      case 'u':
        check_op(op);
        *op = Operation::UPDATE;
        *update_output_path = optarg;
        break;
      case 'd':
        check_op(op);
        *op = Operation::DUMP;
        break;
      case 'o':
        if (output_dir_opt) {
          fprintf(stderr, "%s: -o specified multiple times\n", progname);
        }
        output_dir_opt = optarg;
        break;
      case 'h':
        usage(0);
        break;
      default:
        fprintf(stderr, "%s: unknown argument -%c\n", progname, opt);
        usage(1);
        break;
    }
  }
  if (!output_dir_opt) {
    *output_path = ".";
  } else {
    // Make sure that the parent of the output dir exists.
    if (!directory_exists(dirname(output_dir_opt))) {
      fatal_errno("parent of output directory must be a directory");
    }
    *output_path = output_dir_opt;
  }

  if (*op == Operation::NONE) {
    usage(0);
  }

  // Check if we can access all of the files before we try to open any of them.
  for (int i = optind; i < argc; ++i) {
    pack_file_paths->push_back(argv[i]);
    if (access(argv[i], R_OK)) {
      fatal_errno("can't access '%s'", argv[i]);
    }
  }

  if (pack_file_paths->size() == 0) {
    usage(0);
  } else if (pack_file_paths->size() != 1 && *op == Operation::UPDATE) {
    fprintf(stderr, "%s: update currently only supports one pack file at a time.\n", progname);
    usage(1);
  }

  if (*op == Operation::UPDATE) {
    if (!directory_exists(*update_output_path)) {
      fatal_errno("invalid update directory '%s'", update_output_path->c_str());
    }
  }

  return true;
}

int main(int argc, char* argv[]) {
  progname = basename(argv[0]);

  Operation op = Operation::NONE;
  std::string output_path;
  std::vector<gsl::zstring> pack_file_paths;
  std::string update_output_path;

  if (!parse_args(argc, argv, &op, &output_path, &pack_file_paths, &update_output_path)) {
    fatal("failed to parse arguments");
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

  for (size_t pack_index = 0; pack_index < packs.size(); ++pack_index) {
    gsl::zstring pack_file_name = basename(pack_file_paths[pack_index]);
    mbaacc::Pack& pack = packs[pack_index];

    gsl::span<mbaacc::FolderIndex> folders = pack.folders();
    gsl::span<mbaacc::FileIndex> files = pack.files();

    if (op == Operation::DUMP) {
      if (op == Operation::DUMP) {
        pack.header().dump(pack_file_name);
      }

      for (size_t folder_index = 0; folder_index < folders.size(); ++folder_index) {
        folders[folder_index].dump(folder_index);
      }

      for (size_t file_index = 0; file_index < files.size(); ++file_index) {
        files[file_index].dump(file_index);
      }
      continue;
    }

    for (size_t folder_index = 0; folder_index < folders.size(); ++folder_index) {
      mbaacc::FolderIndex& folder = folders[folder_index];
      std::vector<gsl::cstring_span> dir_path_fragments =
          Split(gsl::cstring_span(folder.filename), "\\");

      // Make sure there's nothing tricky in the directory path like "..".
      for (gsl::cstring_span fragment : dir_path_fragments) {
        if (strcmp(fragment.data(), "..") == 0) {
          fatal("directory name contains '..': %.*s", int(sizeof(folder.filename)),
                folder.filename);
        }
      }

      std::string normalized_dir_path = Join(dir_path_fragments, PATH_SEPARATOR);
      std::string output_dir = output_path + PATH_SEPARATOR;
      std::string update_dir = update_output_path + PATH_SEPARATOR;
      if (!normalized_dir_path.empty()) {
        output_dir += normalized_dir_path + PATH_SEPARATOR;
        update_dir += normalized_dir_path + PATH_SEPARATOR;
      }

      if (op == Operation::LIST) {
        printf("%-12s  dir %10u %.*s\\\n", pack_file_name, 0u, int(sizeof(folder.filename)),
               folder.filename);
      } else if (op == Operation::EXTRACT) {
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

        if (op == Operation::LIST) {
          printf("%-12s file %10u %.*s\\%.*s\n", pack_file_name, file.size,
                 int(sizeof(folder.filename)), folder.filename, int(sizeof(file.filename)),
                 file.filename);
          continue;
        }

        gsl::span<char> file_data = pack.file_data(file_index);
        if (op == Operation::EXTRACT) {
          printf("%.*s\\%.*s\n", int(sizeof(folder.filename)), folder.filename,
                 int(sizeof(file.filename)), file.filename);
          std::string file_path = output_dir;
          file_path.append(file.filename, strnlen(file.filename, sizeof(file.filename)));

          FILE* f = fopen(file_path.c_str(), "wb");
          if (!f) {
            fatal("failed to open file for writing '%s'", file_path.c_str());
          }
          if (fwrite(file_data.data(), 1, file_data.length(), f) != file_data.length()) {
            fatal("failed to write file");
          }
          fclose(f);
        } else if (op == Operation::UPDATE) {
          std::string file_path = update_dir;
          file_path.append(file.filename, strnlen(file.filename, sizeof(file.filename)));

          FILE* f = fopen(file_path.c_str(), "rb");
          if (!f) {
            fatal("failed to open file for reading '%s'", file_path.c_str());
          }

          struct stat st;
          if (fstat(fileno(f), &st) != 0) {
            fatal_errno("failed to stat file '%s'", file_path.c_str());
          }

          if (size_t(st.st_size) != file.size) {
            fatal("file '%.*s\\%.*s' has differing size: %u in pack, %zu on filesystem",
                  int(sizeof(folder.filename)), folder.filename, int(sizeof(file.filename)),
                  file.filename, file.size, size_t(st.st_size));
          }

          if (fread(file_data.data(), 1, file_data.length(), f) != file_data.length()) {
            fatal("failed to read file");
          }

          fclose(f);
        }
      }
    }

    if (op == Operation::UPDATE) {
      FILE* out = fopen(output_path.c_str(), "wb");
      if (!pack.write(out)) {
        fatal("failed to write updated pack file");
      }
      fclose(out);
      fprintf(stderr, "%s: successfully wrote updated packfile to '%s'\n", progname,
              output_path.c_str());
    }
  }
}
