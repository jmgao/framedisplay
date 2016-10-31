/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#if defined(_WIN32)
#define PATH_SEPARATOR "\\"
#define TEMP_FAILURE_RETRY(x) x
#else
#define PATH_SEPARATOR "/"
#endif

inline bool directory_exists(const std::string& path) {
  struct stat sb;
  if (stat(path.c_str(), &sb) == -1) {
    return false;
  }
  if (!S_ISDIR(sb.st_mode)) {
    errno = ENOTDIR;
    return false;
  }
  return true;
}

#if defined(_WIN32)
inline int mkdir(const char* path, int /* mode */) {
  return mkdir(path);
}
#endif

// Given a relative or absolute filepath, create the directory hierarchy
// as needed. Returns true if the hierarchy is/was setup.
inline bool mkdirs(const std::string& path) {
  // TODO: all the callers do unlink && mkdirs && adb_creat ---
  // that's probably the operation we should expose.
  // Implementation Notes:
  //
  // Pros:
  // - Uses dirname, so does not need to deal with OS_PATH_SEPARATOR.
  // - On Windows, uses mingw dirname which accepts '/' and '\\', drive letters
  //   (C:\foo), UNC paths (\\server\share\dir\dir\file), and Unicode (when
  //   combined with our adb_mkdir() which takes UTF-8).
  // - Is optimistic wrt thinking that a deep directory hierarchy will exist.
  //   So it does as few stat()s as possible before doing mkdir()s.
  // Cons:
  // - Recursive, so it uses stack space relative to number of directory
  //   components.
  // If path points to a symlink to a directory, that's fine.
  if (directory_exists(path)) {
    return true;
  }

  std::string path_copy = path;
  const std::string parent(dirname(&path_copy[0]));
  // If dirname returned the same path as what we passed in, don't go recursive.
  // This can happen on Windows when walking up the directory hierarchy and not
  // finding anything that already exists (unlike POSIX that will eventually
  // find . or /).
  if (parent == path) {
    errno = ENOENT;
    return false;
  }
  // Recursively make parent directories of 'path'.
  if (!mkdirs(parent)) {
    return false;
  }
  // Now that the parent directory hierarchy of 'path' has been ensured,
  // create path itself.
  if (mkdir(path.c_str(), 0755) == -1) {
    const int saved_errno = errno;
    // If someone else created the directory, that is ok.
    if (directory_exists(path)) {
      return true;
    }
    // There might be a pre-existing file at 'path', or there might have been some other error.
    errno = saved_errno;
    return false;
  }
  return true;
}
