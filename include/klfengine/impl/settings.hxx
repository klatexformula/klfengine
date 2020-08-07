/* This file is part of the klfengine library, which is distributed under the
 * terms of the MIT license.
 *
 *     https://github.com/klatexformula/klfengine
 *
 * The MIT License (MIT)
 *
 * Copyright 2020 Philippe Faist
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <cstdlib> // getenv()

#include <klfengine/basedefs>
#include <klfengine/settings>

#include <klfengine/h/detail/filesystem.h>



namespace klfengine {

namespace detail {


_KLFENGINE_INLINE
bool is_executable(const fs::path& p) { // can be used as predicate to check executable
  auto st = fs::status(p); // follows symlinks
  // check owner_exec, should be enough
  if (!fs::is_regular_file(st)) {
    return false;
  }
  return ( st.permissions() & fs::perms::owner_exec ) == fs::perms::owner_exec;
}


} // namespace detail


_KLFENGINE_INLINE
std::string settings::get_tex_executable_path(const std::string & exe_name) const
{
  std::string s = texbin_directory + "/" + exe_name;
  if (!fs::exists(s)) {
    throw cannot_find_executable(exe_name, "no such executable in " + texbin_directory);
  }
  return s;
}



// namespace detail {
// inline bool check_good_tmpdir(const std::string & tmpdir)
// {
//   // TODO: check for writeable permissions
//   return !tmpdir.empty() && fs::exists(tmpdir);
// }
// } // namespace detail

//static
_KLFENGINE_INLINE
std::string settings::detect_temporary_directory()
{
  return fs::temp_directory_path();
  // { std::vector<const char *> varnames{"TMPDIR", "TEMP", "TEMPDIR", "TMP"};
  //   for (auto it = varnames.begin(); it != varnames.end(); ++it) {
  //     auto tmpdir_p = std::getenv(*it);
  //     std::string tmpdir{tmpdir_p != nullptr ? tmpdir_p : ""};
  //     if (detail::check_good_tmpdir(tmpdir)) {
  //       return tmpdir;
  //     }
  //   }
  // }
  // { std::vector<const char *> stdpaths{"/var/tmp", "/tmp"};
  //   for (auto it = stdpaths.begin(); it != stdpaths.end(); ++it) {
  //     std::string tmpdir{*it};
  //     if (detail::check_good_tmpdir(tmpdir)) {
  //       return tmpdir;
  //     }
  //   }
  // }
  // throw std::runtime_error("Can't find a suitable temporary directory to use");
}


// static
_KLFENGINE_INLINE
std::string settings::detect_texbin_directory()
{
  // prepare paths.
  std::vector<std::string> paths;

  // search $PATH.
  paths = detail::get_environment_PATH();

  std::vector<std::string> sys_paths{
#if defined(_KLFENGINE_OS_WIN)
    "C:\\Program Files*\\MiKTeX*\\miktex\\bin",
    "C:\\texlive\\*\\bin\\*"
#elif defined(_KLFENGINE_OS_MACOSX)
    "/usr/texbin",
    "/Library/TeX/texbin",
    "/usr/local/bin",
    "/opt/local/bin",
    "/sw/bin",
    "/sw/usr/bin"
#else
    "/usr/local/bin"
#endif
  };
  // append these paths
  paths.insert(paths.end(), sys_paths.begin(), sys_paths.end());


  // prepend any hard-coded paths provided via a preprocessor define
#ifdef KLFENGINE_EXTRA_SEARCH_PATHS
  std::vector<std::string> extra_compiled_paths{
    KLFENGINE_EXTRA_SEARCH_PATHS
  };
  paths.insert(paths.begin(), extra_compiled_paths.begin(), extra_compiled_paths.end());
#endif
  
  // add "/latex" to each path
  for (std::string & s : paths) {
#ifdef _KLFENGINE_OS_WIN
    s += "\\latex.exe";
#else
    s += "/latex";
#endif
  }

  // look for "latex" in $PATH + some hard-coded standard paths
  std::vector<fs::path> latex_results =
    detail::find_wildcard_path(
        paths, detail::is_executable,
        1 // limit - a single match is good
        );
  if (latex_results.empty()) {
    // no matches
    return std::string{};
  }
  return latex_results.front().native();
}

// static
_KLFENGINE_INLINE
std::string settings::detect_gs_executable_path()
{
  throw std::runtime_error("not implemented");
}

// not static --
_KLFENGINE_INLINE
std::map<std::string, std::string> settings::detect_subprocess_add_environment() const
{
  throw std::runtime_error("not implemented");
}

// static
_KLFENGINE_INLINE
settings settings::detect_settings()
{
  settings s{
    detect_temporary_directory(),
    detect_texbin_directory(),
    "process",
    detect_gs_executable_path(),
    {}
  };
  s.subprocess_add_environment = s.detect_subprocess_add_environment();
  return s;
}




_KLFENGINE_INLINE
bool operator==(const settings & a, const settings & b)
{
  return (
      a.temporary_directory == b.temporary_directory &&
      a.texbin_directory == b.texbin_directory &&
      a.gs_method == b.gs_method &&
      a.gs_executable_path == b.gs_executable_path &&
      a.subprocess_add_environment == b.subprocess_add_environment
      );
}

_KLFENGINE_INLINE
bool operator!=(const settings & a, const settings & b)
{
  return ! (a == b);
}



_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const settings & v)
{
  j = nlohmann::json{
    {"temporary_directory", v.temporary_directory},
    {"texbin_directory", v.texbin_directory},
    {"gs_method", v.gs_method},
    {"gs_executable_path", v.gs_executable_path},
    {"subprocess_add_environment", v.subprocess_add_environment}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, settings & v)
{
  j.at("temporary_directory").get_to(v.temporary_directory);
  j.at("texbin_directory").get_to(v.texbin_directory);
  j.at("gs_method").get_to(v.gs_method);
  j.at("gs_executable_path").get_to(v.gs_executable_path);
  j.at("subprocess_add_environment").get_to(v.subprocess_add_environment);
}



} // namespace klfengine
