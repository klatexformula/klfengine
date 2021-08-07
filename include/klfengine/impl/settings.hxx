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
  std::string s = texbin_directory + "/" + exe_name
#ifdef _KLFENGINE_OS_WIN
                                                    + ".exe"
#endif
    ;

  if (!fs::exists(s)) {
    throw cannot_find_executable(exe_name, "no such executable in " + texbin_directory);
  }
  return s;
}


// static
_KLFENGINE_INLINE
std::vector<std::string> settings::get_wildcard_search_paths(
    const std::vector<std::string> & extra_paths
    )
{
  // prepare paths.
  std::vector<std::string> search_paths;

  // start with any user-given paths
  search_paths.insert(search_paths.end(), extra_paths.begin(), extra_paths.end());

  // then add any hard-coded paths provided via a preprocessor define
#ifdef KLFENGINE_EXTRA_SEARCH_PATHS
  std::vector<std::string> extra_compiled_paths{
    KLFENGINE_EXTRA_SEARCH_PATHS
  };
  search_paths.insert(search_paths.end(),
                      extra_compiled_paths.begin(), extra_compiled_paths.end());
#endif
  
  // then add the usual $PATH
  std::vector<std::string> env_paths = detail::get_environment_PATH();
  search_paths.insert(search_paths.end(), env_paths.begin(), env_paths.end());

  // then finally add some hard-coded common paths.
  std::vector<std::string> sys_paths{
#if defined(_KLFENGINE_OS_WIN)
    "C:\\Program Files*\\MiKTeX*\\miktex\\bin",
    "C:\\texlive\\<texlive-year>\\bin\\*",
    "C:\\texlive\\*\\bin\\*",
    "C:\\texlive\\<texlive-year>\\tlpkg\\tlgs\\bin",
//    "C:\\texlive\\*\\tlpkg\\tlgs\\bin" // TODO -- add this but also associated
//                                       //         GS_LIB value (klf/klf issue #15)
    "C:\\Program Files*\\gs\\gs*\\bin", // ??
#elif defined(_KLFENGINE_OS_MACOSX)
    "/usr/texbin",
    "/Library/TeX/texbin",
    "/usr/local/bin",
    "/opt/local/bin",
    "/usr/local/opt/ghostscript*/bin",
    "/sw/bin",
    "/sw/usr/bin"
#else
    "/usr/local/bin"
#endif
  };
  search_paths.insert(search_paths.end(), sys_paths.begin(), sys_paths.end());

  // these are the search paths.
  return search_paths;
}


// -------------------------------------

namespace detail {
inline std::string detect_libgs(const std::vector<std::string> & extra_paths)
{
  //
  // look for ghostscript libgs.{dll|so|dylib}
  //

  // set up search paths first.  Initialize with all the given extra_paths first.
  std::vector<std::string> search_paths{extra_paths};

  // then add any hard-coded paths provided via a preprocessor define
#ifdef KLFENGINE_EXTRA_SEARCH_PATHS
  std::vector<std::string> extra_compiled_paths{
    KLFENGINE_EXTRA_SEARCH_PATHS
  };
  search_paths.insert(search_paths.end(),
                      extra_compiled_paths.begin(), extra_compiled_paths.end());
#endif
  
  std::vector<std::string> libgs_search_paths = {
#if defined(_KLFENGINE_OS_WIN)
    "C:\\Program Files*\\gs\\gs*\\bin", // ?? "lib?"
    "C:\\Windows\\System",
    // Note: gsdll64.dll can be in System32 ? according to
    // https://docs.manim.community/en/v0.1.1/installation/troubleshooting.html
    "C:\\Windows\\System32",
    "C:\\Windows\\System64"
#else
#  if defined(_KLFENGINE_OS_MACOSX)
    "/usr/local/opt/ghostscript*/lib",
#  else
#  endif
    "/usr/lib",
    "/opt/lib",
    "/usr/local/lib"
#endif
  };
    
  std::vector<std::string> libgs_fnames{
#if defined(_KLFENGINE_OS_MACOSX)
    "libgs.dylib",
    "libgs.so"
#elif defined(_KLFENGINE_OS_WIN)
    "gsdll64.dll", // todo, pick correct one according to current process
    "gsdll32.dll", // todo, pick correct one according to current process
    "gs.dll",
    "libgs-*.dll"
#elif defined(_KLFENGINE_OS_LINUX)
    "libgs.so"
#else
#endif
  };

  fs::path libgs_path;
  std::vector<fs::path> libgs_results = detail::find_wildcard_path(
    search_paths,
    libgs_fnames,
    1 // limit - a single match is good
  );
  if (!libgs_results.empty()) {
    libgs_path = libgs_results.front();
  }

  return libgs_path.generic_string();
}

} // namespace detail

// static
_KLFENGINE_INLINE
settings settings::detect_settings(
    const std::vector<std::string> & extra_paths
    )
{
  std::vector<std::string> exe_search_paths = get_wildcard_search_paths(extra_paths);

  settings s;

#ifdef _KLFENGINE_OS_WIN
  std::vector<std::string> latex_exe_names{"latex.exe"},
  std::vector<std::string> gs_exe_names{"gswin32c.exe", "gswin64c.exe", "mgs.exe"},
#else
  std::vector<std::string> latex_exe_names{"latex"};
  std::vector<std::string> gs_exe_names{"gs"};
#endif


  //
  // look for executable "latex" in $PATH + some hard-coded standard paths
  //

  std::vector<fs::path> latex_results =
    detail::find_wildcard_path(
        exe_search_paths,
        latex_exe_names,
        detail::is_executable,
        1 // limit - a single match is good
        );
  if (!latex_results.empty()) {
    s.texbin_directory = latex_results.front().parent_path().generic_string();
  }


  //
  // look for ghostscript executable in the given search paths
  //
  fs::path gs_exe_path;
  std::vector<fs::path> gs_results =
    detail::find_wildcard_path(
        exe_search_paths,
        gs_exe_names,
        detail::is_executable,
        1 // limit - a single match is good
        );
  if (!gs_results.empty()) {
    gs_exe_path = gs_results.front();
    s.gs_executable_path = gs_exe_path.generic_string();
  }

  // see if subprocess environment needs to be adjusted according to gs executable

  if (gs_exe_path.filename() == "mgs.exe") {
    // detect MikTeX mgs.exe as ghostscript and setup its environment properly
    fs::path gs_parent_path = gs_exe_path.parent_path();
    s.subprocess_add_environment["MIKTEX_GS_LIB"] =
      (gs_parent_path / ".." / ".." / "ghostscript" / "base").native() +
      detail::path_separator +
      (gs_parent_path / ".." / ".." / "fonts").native() ;
  }


  //
  // look for ghostscript libgs.{dll|so|dylib}
  //
  s.gs_libgs_path = detail::detect_libgs(extra_paths);


  // 
  // pick a method for ghostscript
  //
  if (s.gs_executable_path.size() != 0) {
    s.gs_method = "process";
  } else {
    s.gs_method = "none";
  }
  // TODO: set loadlibgs as default method if applicable

  return s;
}





// -------------------------------------


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
