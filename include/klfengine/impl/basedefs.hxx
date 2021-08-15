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


#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <string>
#include <cstdlib>

#include <klfengine/basedefs>



namespace klfengine {

namespace detail {


// thanks https://stackoverflow.com/a/20170989/1694896
_KLFENGINE_INLINE
std::string get_type_name_impl(
    const char * typeid_name,
    bool is_const,
    bool is_volatile,
    bool is_lvalue_reference,
    bool is_rvalue_reference
    )
{
  std::unique_ptr<char, void(*)(void*)> own{
#ifndef _MSC_VER
    abi::__cxa_demangle(typeid_name, nullptr,
                        nullptr, nullptr),
#else
    nullptr,
#endif
    std::free
  };
  std::string r = ((own != nullptr) ? own.get() : typeid_name);
  if (is_const) {
    r += " const";
  }
  if (is_volatile) {
    r += " volatile";
  }
  if (is_lvalue_reference) {
    r += "&";
  }
  else if (is_rvalue_reference) {
    r += "&&";
  }
  return r;
}






} // namespace detail

} // namespace klfengine

