// Copyright 2017 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef BAZEL_SRC_MAIN_NATIVE_WINDOWS_FILE_H_
#define BAZEL_SRC_MAIN_NATIVE_WINDOWS_FILE_H_

#include <windows.h>

#include <memory>
#include <string>

namespace bazel {
namespace windows {

using std::unique_ptr;
using std::wstring;

template <typename char_type>
bool HasUncPrefix(const char_type* path) {
  // Return true iff `path` starts with "\\?\", "\\.\", or "\??\".
  return path[0] == '\\' &&
         ((path[1] == '\\' && (path[2] == '?' || path[2] == '.')) ||
          (path[1] == '?' && path[2] == '?')) &&
         path[3] == '\\';
}

// Keep in sync with j.c.g.devtools.build.lib.windows.WindowsFileOperations
enum {
  IS_JUNCTION_YES = 0,
  IS_JUNCTION_NO = 1,
  IS_JUNCTION_ERROR = 2,
};

// Keep in sync with j.c.g.devtools.build.lib.windows.WindowsFileOperations
enum {
  DELETE_PATH_SUCCESS = 0,
  DELETE_PATH_DOES_NOT_EXIST = 1,
  DELETE_PATH_DIRECTORY_NOT_EMPTY = 2,
  DELETE_PATH_ACCESS_DENIED = 3,
  DELETE_PATH_ERROR = 4,
};

// Determines whether `path` is a junction (or directory symlink).
//
// `path` should be an absolute, normalized, Windows-style path, with "\\?\"
// prefix if it's longer than MAX_PATH.
//
// To read about differences between junctions and directory symlinks,
// see http://superuser.com/a/343079. In Bazel we only ever create junctions.
//
// Returns:
// - IS_JUNCTION_YES, if `path` exists and is either a directory junction or a
//   directory symlink
// - IS_JUNCTION_NO, if `path` exists but is neither a directory junction nor a
//   directory symlink; also when `path` is a symlink to a directory but it was
//   created using "mklink" instead of "mklink /d", as such symlinks don't
//   behave the same way as directories (e.g. they can't be listed)
// - IS_JUNCTION_ERROR, if `path` doesn't exist or some error occurred
int IsJunctionOrDirectorySymlink(const WCHAR* path);

// Computes the long version of `path` if it has any 8dot3 style components.
// Returns the empty string upon success, or a human-readable error message upon
// failure.
// `path` must be an absolute, normalized, Windows style path, with a "\\?\"
// prefix if it's longer than MAX_PATH. The result will have a "\\?\" prefix if
// and only if `path` had one as well. (It's the caller's responsibility to keep
// or remove this prefix.)
// TODO(laszlocsomor): update GetLongPath so it succeeds even if the path does
// not (fully) exist.
wstring GetLongPath(const WCHAR* path, unique_ptr<WCHAR[]>* result);

// Opens a directory using CreateFileW.
// `path` must be a valid Windows path, with "\\?\" prefix if it's long.
// If `read_write` is true then the directory is opened for reading and writing,
// otherwise only for reading.
HANDLE OpenDirectory(const WCHAR* path, bool read_write);

// Creates a junction at `name`, pointing to `target`.
// Returns the empty string upon success, or a human-readable error message upon
// failure.
// Neither `junction_name` nor `junction_target` needs to have a "\\?\" prefix,
// not even if they are longer than MAX_PATH, though it's okay if they do. This
// function will add the right prefixes as necessary.
wstring CreateJunction(const wstring& junction_name,
                       const wstring& junction_target);

// Deletes the file, junction, or empty directory at `path`.
// Returns DELETE_PATH_SUCCESS if it successfully deleted the path, otherwise
// returns one of the other DELETE_PATH_* constants (e.g. when the directory is
// not empty or the file is in use by another process).
// Returns DELETE_PATH_ERROR for unexpected errors. If `error` is not null, the
// function writes an error message into it.
int DeletePath(const wstring& path, wstring* error);

}  // namespace windows
}  // namespace bazel

#endif  // BAZEL_SRC_MAIN_NATIVE_WINDOWS_FILE_H_
