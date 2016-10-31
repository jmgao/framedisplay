#include <assert.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>

#include "mbaacc_fs.h"
#include "mbaacc_pack.h"
#include "util/fatal.h"
#include "util/mkdirs.h"
#include "util/split.h"

const char* progname;

enum class Operation {
  NONE,
  EXTRACT,
  CREATE,
  LIST,
};

static constexpr long kDefaultLastPackFile = 8;

[[noreturn]] static void usage(int exit_code) {
  fprintf(stderr, "usage: %s [OPTION]... PACK_FILE_DIR\n", progname);
  fprintf(stderr, "Operation mode:\n");
  fprintf(stderr, "  -x\t\textract the contents of the pack files in PACK_FILE_DIR\n");
  fprintf(stderr, "    \t\toutput path defaults to ./out/\n");
  fprintf(stderr, "  -c DIR\tcreate pack file updating PACK_FILE_DIR files to DIR\n");
  fprintf(stderr, "    \t\toutput path defaults to PACK_FILE_DIR/LAST_PACK + 1.p\n");
  fprintf(stderr, "  -t\t\tlist the contents of the pack files in PACK_FILE_DIR\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o\t\toutput path for -x/-c\n");
  fprintf(stderr, "  -m LAST_PACK\tset the last pack file index to use\n");
  fprintf(stderr, "    \t\tdefaults to ∞ for -x/-c, %ld for -c\n", kDefaultLastPackFile);
  fprintf(stderr, "  -h\t\tdisplays this message\n");
  exit(exit_code);
}

static void check_op(Operation* op) {
  if (*op != Operation::NONE) {
    fprintf(stderr, "%s: exactly one operation mode must be specified\n", progname);
    usage(1);
  }
}

static bool parse_args(int argc, char* argv[], Operation* op, std::string* output_path,
                       std::string* comparison_path, gsl::zstring* pack_file_dir,
                       long* max_packfile) {
  bool found_output_path = false;
  *max_packfile = -1;

  int opt;
  while ((opt = getopt(argc, argv, "xc:to:mh")) != -1) {
    switch (opt) {
      case 'x':
        check_op(op);
        *op = Operation::EXTRACT;
        break;
      case 'c':
        check_op(op);
        *op = Operation::CREATE;
        *comparison_path = optarg;
        break;
      case 't':
        check_op(op);
        *op = Operation::LIST;
        break;
      case 'o':
        if (found_output_path) {
          fatal("output path specified multiple times");
        }
        found_output_path = true;
        *output_path = optarg;
        break;
      case 'm': {
        char* endptr;
        *max_packfile = strtol(optarg, &endptr, 10);
        if (*max_packfile < 0 || *endptr != '\0') {
          fatal("invalid argument to -m: '%s'", optarg);
        }
        break;
      }
      case 'h':
        usage(0);
        break;
      default:
        fprintf(stderr, "%s: unknown argument -%c\n", progname, opt);
        usage(1);
        break;
    }
  }

  if (*op == Operation::NONE) {
    usage(0);
  }

  if (output_path->empty()) {
    if (*op == Operation::CREATE) {
      char buf[strlen("0000.p") + 1];
      snprintf(buf, sizeof(buf), "%04ld.p", *max_packfile);
      *output_path = std::string(*pack_file_dir) + PATH_SEPARATOR + buf;
    } else {
      *output_path = "./out/";
    }
  }

  // Make sure that the parent of the output dir exists.
  std::string path_copy = *output_path;
  if (!directory_exists(dirname(&path_copy[0]))) {
    fatal_errno("invalid output directory");
  }

  // Make sure the comparison path exists.
  if (*op == Operation::CREATE) {
    path_copy = *comparison_path;
    if (!directory_exists(dirname(&path_copy[0]))) {
      fatal_errno("invalid comparison directory");
    }
  }

  // Make sure that the pack file directory exists.
  if (optind != argc - 1) {
    usage(1);
  }

  if (!directory_exists(argv[optind])) {
    fatal_errno("can't access '%s'", argv[optind]);
  }

  if (*max_packfile == -1) {
    if (*op == Operation::CREATE) {
      *max_packfile = kDefaultLastPackFile;
    } else {
      *max_packfile = LONG_MAX;
    }
  }

  *pack_file_dir = argv[optind];
  return true;
}

int main(int argc, char* argv[]) {
  progname = basename(argv[0]);

  Operation op = Operation::NONE;
  std::string output_path;
  std::string comparison_path;
  gsl::zstring pack_file_dir;
  long max_packfile = -1;

  if (!parse_args(argc, argv, &op, &output_path, &comparison_path, &pack_file_dir, &max_packfile)) {
    fatal("failed to parse arguments");
  }

  mbaacc::FS fs;
  for (long i = 1; i <= max_packfile; ++i) {
    // TODO: PATH_MAX is a lie, but doing this correctly in a cross-platform way is annoying:
    //       openat on linux/darwin, UNC paths on windows
    char path_buf[PATH_MAX];
    auto rc = snprintf(path_buf, sizeof(path_buf), "%s%s%04ld.p", pack_file_dir, PATH_SEPARATOR, i);
    if (rc >= PATH_MAX) {
      fatal("path buffer overflow");
    }

    FILE* file = fopen(path_buf, "rb");
    if (!file) {
      if (op == Operation::CREATE || i == 1) {
        fatal_errno("failed to open '%s'", path_buf);
      }
      max_packfile = i - 1;
      break;
    }

    fprintf(stderr, "reading packfile at %s\n", path_buf);
    auto pack = std::make_unique<mbaacc::Pack>(unique_fd(dup(fileno(file))));
    fs.AddPack(std::move(pack));
    fclose(file);
  }

  switch (op) {
    case Operation::EXTRACT: {
      return fs.Extract(output_path) ? 0 : 1;
    }

    case Operation::LIST: {
      fatal("-t unimplemented");
      break;
    }

    case Operation::CREATE: {
      // TODO: Output to a file.
      return fs.GenerateUpdatePack(stdout, comparison_path) ? 0 : 1;
    }

    case Operation::NONE:
      fatal("operation not set?");
  }
}
