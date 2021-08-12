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

#include <string>
#include <vector>
#include <map>

#include <nlohmann/json.hpp>

#include <klfengine/basedefs>


namespace klfengine {


/** \brief Exception indicates that a requested executable couldn't be found
 *
 * Thrown e.g. by \ref settings::get_tex_executable_path() to report that the
 * requested executable couldn't be found.
 */
struct cannot_find_executable : exception
{
  cannot_find_executable(const std::string & exe_name, const std::string & msg)
    : exception("Cannot find executable " + exe_name + ": " + msg)
  { }
};


/** \brief Where to find latex, find ghostscript, create temporary dirs etc.
 *
 * A default-constructed object will have empty fields.  Use \ref
 * detect_settings() to auto-detect reasonable settings.
 */
struct settings
{
  /** \brief A location where we can create temporary files and directories
   *
   * This can be something like "/tmp".  The method \ref detect_settings() tries
   * to find a standard system temporary directory.
   *
   */
  std::string temporary_directory;

  /** \brief The location where latex executables can be found
   *
   * You can use the convenience function \ref get_tex_executable_path() to get
   * the full path to a specific executable that you expect should be here,
   * e.g., "latex" or "pdflatex".
   *
   * To search or find other standard TeX directories, files, style files,
   * support files, fonts, etc., you can use the "kpsewhich" executable which
   * should also reside in this directory (you can use
   * <code>get_tex_executable_path("kpsewhich")</code>) to query any TeX-related
   * search paths, standard directories or other settings.
   */
  std::string texbin_directory;

  /** \brief How to invoke Ghostscript (external process, linked C library, or
   *         library loaded at run-time)
   *
   * This is one of "none", "process", "linked-libgs", "load-libgs".
   *
   */
  std::string gs_method;

  /** \brief Path to the Ghostscript \c gs executable
   *
   * Used in case \a gs_method is set to \c "process".
   */
  std::string gs_executable_path;

  /** \brief Path to the Ghostscript \c libgs dynamic library
   *
   * Used in case \a gs_method is set to \c "load-libgs".
   *
   * \todo TODO: The "load-libgs" variant is not yet implemented in the internal
   *       detail::simple_gs_interface.
   */
  std::string gs_libgs_path;

  /** \brief Environment variables to include when launching suprocesses, such
   *         as Ghostscript
   */
  std::map<std::string, std::string> subprocess_add_environment;

  /** \brief Get the path to a latex executable in texbin_directory
   *
   * Ensures that an executable called \a exe_name (or \a exe_name .exe on
   * Windows) exists in \ref texbin_directory, and returns the path to that
   * executable obtained by path-concatenating \a texbin_directory with the
   * executable file name.
   *
   * If no such executable exists, then this method throws \ref
   * cannot_find_executable.
   */
  std::string get_tex_executable_path(const std::string & exe_name) const;

  // static:

  /** \brief Return a settings object with fields initialized to system-detected values
   *
   * This method searches \a extra_paths, the system \a $PATH, any compile-time
   * specified paths with
   * <code>-DKLFENGINE_EXTRA_SEARCH_PATHS="/A/a:/B/b/b"</code>, and other
   * hard-coded standard LaTeX and Ghostscript paths to initialize the settings
   * object.  See \ref get_wildcard_search_paths() for more information about
   * the paths that are searched.
   *
   * If a texbin directory or a ghostscript installation cannot be found, the
   * corresponding fields in the returned settings object are empty (no
   * exception is thrown in this case).
   */
  static settings detect_settings(
      const std::vector<std::string> & extra_paths = std::vector<std::string>{}
      );

  /** \brief Collect the paths where we should search for latex and ghostscript
   *
   * You should not normally need this function if you use \ref
   * detect_settings().  If you need more fine-tuning over how the settings are
   * detected, you can call this function to get the paths (with wildcard
   * expressions that you can use with \ref detail::find_wildcard_path()) that
   * \a detect_settings() would search in.
   *
   * This static method returns a list of search paths collected (in this order)
   * from:
   *
   *   - the argument \a extra_paths,
   *
   *   - the system \a $PATH
   *
   *   - any compile-time paths specified by
   * <code>-DKLFENGINE_EXTRA_SEARCH_PATHS="..."</code>,
   *
   *   - and some hard-coded standard paths (such as
   *     <code>"/usr/local/texlive/<em></em>*<em></em>/bin/<em></em>*"</code> etc.).
   *
   * You can specify at compile-time any additional paths that should be
   * included in this list by specifying the
   * <code>-DKLFENGINE_EXTRA_SEARCH_PATHS="/A/aa/a:/b:/c/cc"</code> preprocessor
   * compilation flag.  The path separator is ":" on Unixes and ";" on Windows
   * as for the system \a $PATH variable.  In case you are compiling klfengine's
   * implementation hxx files separately (with the
   * <code>-DKLFENGINE_SEPARATE_IMPLEMENTATION</code> compilation flag), then
   * the <code>-DKLFENGINE_EXTRA_SEARCH_PATHS="..."</code> flag should be
   * applied on the compilation of the implementation files. The flag is
   * irrelevant for the klfengine headers.
   */
  static std::vector<std::string> get_wildcard_search_paths(
      const std::vector<std::string> & extra_paths = std::vector<std::string>{}
      );
};


bool operator==(const settings & a, const settings & b);
bool operator!=(const settings & a, const settings & b);

void to_json(nlohmann::json & j, const settings & v);
void from_json(const nlohmann::json & j, settings & v);


} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/settings.hxx>
#endif
