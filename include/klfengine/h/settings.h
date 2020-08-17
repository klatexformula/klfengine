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

//#include <klfengine/run_ghostscript>

namespace klfengine {


struct cannot_find_executable : exception
{
  cannot_find_executable(const std::string & exe_name, const std::string & msg)
    : exception("Cannot find executable " + exe_name + ": " + msg)
  { }
};


/** \brief Where to find latex, find ghostscript, create temporary dirs etc.
 *
 * A default-constructed object will have empty fields.  Use \ref
 * detect_settings() (or the other more specific <code>detect_*</code>
 * functions) to auto-detect reasonable settings.
 */
struct settings
{
  std::string temporary_directory;

  std::string texbin_directory;

  /**
   * This is one of "none", "process", "linked-libgs", "load-libgs".
   *
   * \todo TODO: The libgs variants are not yet implemented in the internal
   *       detail::simple_gs_interface.
   */
  std::string gs_method;

  std::string gs_executable_path;

  std::map<std::string, std::string> subprocess_add_environment;

  std::string get_tex_executable_path(const std::string & exe_name) const;

  // static:

  static settings detect_settings(
      const std::vector<std::string> & extra_paths = std::vector<std::string>{}
      );

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
