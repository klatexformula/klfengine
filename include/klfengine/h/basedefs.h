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

#include <cstdio> // fprintf()
#include <string>
#include <vector>
#include <exception>
#include <type_traits>
#include <typeinfo>

#include <nlohmann/json.hpp>


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





/** \brief Namespace collecting all \a klfengine-related definitions
 *
 */
namespace klfengine {


// simple WARNING / ERROR output handles with printf-like formatting
//
// !!! Use out-of-line implementation so that if we use a separate
// implementation, only that compilation unit needs the KLFENGINE_HANDLE_XXX
// macros set and there are no differences in the implementation !!!
void warn(const std::string & what, const std::string & msg);
void error(const std::string & what, const std::string & msg);




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




/** \brief Error parsing object from JSON representation
 *
 * This exception is thrown when loading objects (e.g., \ref klfengine::margins,
 * klfengine::input, etc.) from JSON, e.g., if the given JSON does not have the
 * expected structure.
 */
class invalid_json_value : public klfengine::exception
{
public:
  invalid_json_value(std::string what, const nlohmann::json & json,
                     std::string moremsg = std::string{})
    : klfengine::exception{
        "Cannot parse " + what + " from `" + json.dump() + "'"
        + (moremsg.size() ? (": "+moremsg) : "")
      }
  {}
};



/** \brief Invalid parameter provided
 *
 * This exception is thrown in methods that accept paramters, typically from a
 * general dictionary object, to indicate that an invalid parameter name or
 * value was encountered.
 */
class invalid_parameter : public klfengine::exception
{
public:
  invalid_parameter(std::string what_, std::string msg)
    : klfengine::exception{
        (what_.empty()?std::string{}:what_+": ")
        + "invalid parameter: " + msg
      }
  {}
};

/** \brief Invalid value
 *
 * This exception is thrown when an invalid value is encountered, though not
 * necessarily given as an argument.  This is used, e.g., to report invalid
 * casts in \ref klfengine::value::get_cast().
 */
class invalid_value : public klfengine::exception
{
public:
  invalid_value(std::string msg) : klfengine::exception{msg}
  {}
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


// Custom hash class to help smooth out some inconsistencies between different
// C++ std lib's implementations.  E.g. std::hash(nullptr) etc.
template<typename T>
struct hash
{
  template<
    typename TT,
    typename T2 = T,
    typename std::enable_if<std::is_default_constructible<std::hash<T2>>::value &&
                            !std::is_same<T2, std::nullptr_t>::value, int>::type = 99
    >
  std::size_t operator()(TT && x) const noexcept
  {
    return std::hash<T>{}(std::forward<TT>(x));
  }

  // include hash function for nullptr_t
  template<
    typename T2 = T,
    typename std::enable_if<std::is_same<T2, std::nullptr_t>::value, int>::type = 99
    >
  std::size_t operator()(std::nullptr_t ) const noexcept
  {
    return 0;
  }
};


// useful way of combining hashes as in boost: (cf
// https://stackoverflow.com/a/2595226/1694896)
inline void hash_combine(std::size_t& seed, std::size_t b)
{
  seed ^= (b + 0x9e3779b9 + (seed<<6) + (seed>>2));
}



} // namespace detail

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/basedefs.hxx>
#endif
