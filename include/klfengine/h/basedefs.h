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
#include <exception>
#include <type_traits>
#include <typeinfo>
#include <regex>


//
// If KLFENGINE_SEPARATE_IMPLEMENTATION was defined, then the separate
// compilation of the implementation of hxx sources into a separate translation
// unit was requested
//
#ifdef KLFENGINE_SEPARATE_IMPLEMENTATION

#  define _KLFENGINE_INLINE /* not inline, define function/method normally */

// don't automatically include klfengine/impl/[...].hxx implementation sources.
#  define _KLFENGINE_DONT_INCLUDE_IMPL_HXX

#else

// keep everything inline, no separate compilation of implementation sources

#  define _KLFENGINE_INLINE inline

#endif



#if defined(_WIN32) || defined(_WIN64) || defined(_Windows)
#  define _KLFENGINE_OS_WIN
#elif defined(__APPLE__) || defined(__MACH__)
#  define _KLFENGINE_OS_MACOSX
#else
#  define _KLFENGINE_OS_LINUX
#endif




namespace klfengine {

/** \brief Storage type for (binary) data resulting from a compilation
 *
 * Currently we only guarantee that binary_data provides vector-like or
 * string-like C++ STL iterator access via \a begin() and \a end().  For
 * instance, we might change the typedef to \a std::string depending on what
 * works best in implementations.
 *
 * \fixme FIXME: Determine what interface binary_data allows.  
 *
 */
using binary_data = std::vector<std::uint8_t>;


/** \brief Exception class for klfengine
 *
 * Errors reported by the klfengine library are thrown as exceptions which
 * inherit \ref klfengine::exception.
 */
class exception : public std::exception
{
public:
  /** \brief Constructor with full error message
   *
   */
  explicit exception(std::string msg_)
    : _msg(std::move(msg_))
  {
  }
  /** \brief Constructor with full error message provided as C-style string
   *
   * The string is copied to an internal member so the pointed data does not
   * need to outlive the constructor call.
   */
  explicit exception(const char * msg_c_str)
    : _msg(msg_c_str)
  {
  }
  virtual ~exception() = default;

  /** \brief Returns the error message provided to the constructor
   */
  inline const char * what() const noexcept { return _msg.c_str(); }

private:
  std::string _msg;
};



namespace detail {

std::string get_type_name_impl(
    const char * typeid_name,
    bool is_const,
    bool is_volatile,
    bool is_lvalue_reference,
    bool is_rvalue_reference
    );

template<typename T>
std::string get_type_name()
{
  using TR = typename std::remove_reference<T>::type;
  return get_type_name_impl(
    typeid(TR).name(),
    std::is_const<TR>::value,
    std::is_volatile<TR>::value,
    std::is_lvalue_reference<T>::value,
    std::is_rvalue_reference<T>::value
    );
}


std::vector<std::string> str_split_rx(
    std::string::const_iterator a, std::string::const_iterator b,
    const std::regex & rx_sep, bool skip_empty = false
    );

} // namespace detail

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/basedefs.hxx>
#endif
