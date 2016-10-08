#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char* progname;

#define fatal(fmt, ...)                  \
  do {                                   \
    fprintf(stderr, "%s: ", progname);   \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    fprintf(stderr, "\n");               \
    _Exit(1);                            \
  } while (0)

#define fatal_errno(fmt, ...)                 \
  do {                                        \
    int err = errno;                          \
    fprintf(stderr, "%s: ", progname);        \
    fprintf(stderr, fmt, ##__VA_ARGS__);      \
    fprintf(stderr, ": %s\n", strerror(err)); \
    _Exit(1);                                 \
  } while (0)
