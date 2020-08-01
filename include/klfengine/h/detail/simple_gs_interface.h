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

#include <klfengine/basedefs>
#include <klfengine/settings>

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
  enum struct method {
    None,
    Process,
    LinkedLibgs,
    LoadLibgs
  };
  
  static method get_method(const std::string & method_s);
  
  explicit simple_gs_interface(method method_, std::string gs_path = std::string());
  explicit simple_gs_interface(std::string method_s, std::string gs_path = std::string());

  struct gs_info_t {
    std::string head;
    std::vector<std::string> devices;
    std::vector<std::string> search_path;
  };

  std::pair<int,int> gs_version();
  gs_info_t gs_info();

  binary_data run_gs(const std::vector<std::string> & gs_args,
                     const binary_data & stdin_data,
                     bool add_standard_batch_flags = true,
                     std::string * set_stderr = nullptr);

private:
  method _method;
  std::string _gs_path;

  void _init();
};


} // namespace detail

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/detail/simple_gs_interface.hxx>
#endif
