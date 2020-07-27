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


#include <subprocess.hpp>

namespace klfengine {

namespace detail {

/**
 * \internal
 *
 * For now, this class is internal API because I'm not sure how I want to
 * structure it, and we can expect major API changes to this at any point.
 */
class simple_gs_interface
{
public:
  simple_gs_interface() = delete;

  struct gs_info {
    std::string version_line;
    std::string copyright_line;
    std::vector<std::string> devices;
    std::vector<std::string> search_path;
  };

  static std::pair<int,int> gs_version(const settings& settings_);

  static gs_info gs_info(const settings& settings_);

  static void run(const settings& settings_, const std::vector<std::string> & gs_cmd,
                  bool add_standard_batch_flags);


  // ### Hmm, no, prefer to have a single location (settings) where method & gs
  // ### path are stored
  //
  // enum struct method {
  //   None,
  //   Process,
  //   LinkedLibgs,
  //   LoadLibgs
  // };
  //
  // static method get_method(const std::string & method_s) {
  //   if (method_s == "none") {
  //     return None;
  //   } else if (method_s == "process") {
  //     return Process;
  //   } else if (method_s == "linked-libgs") {
  //     return LinkedLibgs;
  //   } else if (method_s == "load-libgs") {
  //     return LoadLibgs;
  //   }
  //   throw std::invalid_argument("Invalid gs interface method: " + method_s);
  // }
  //
  // simple_gs_interface(method method_)
  //   : _method(method_)
  // {
  // }
  // simple_gs_interface(std::string method_s)
  //   : _method(get_method(method_s))
  // {
  // }

private:
  GhostscriptMethod _method;
};


} // namespace detail

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/detail/simple_gs_interface.hxx>
#endif
