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

#include <cstdio> // FILE, fopen, fwrite
#include <cerrno>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <system_error>
#include <algorithm>
#include <regex>

#include <klfengine/h/basedefs.h>
#include <klfengine/h/detail/provide_fs.h>


namespace klfengine {

namespace detail {

// separate namespace so that we can import all these functions in a scope with
// "using namespace klfengine::detail::utils"
namespace utils {



// ---------------------------------------------------------
//
// kwargs-style parsing
//


// from https://en.cppreference.com/w/cpp/types/disjunction
template<class...> struct my_disjunction : std::false_type { };
template<class B1> struct my_disjunction<B1> : B1 { };
template<class B1, class... Bn>
struct my_disjunction<B1, Bn...> 
    : std::conditional<bool(B1::value), B1, my_disjunction<Bn...>>::type
{ };

// see https://stackoverflow.com/a/34099948/1694896
template<typename T, typename... Ts>
struct contains : my_disjunction<std::is_same<T, Ts>...> {};


/* Utility for getting keyword-style arguments as klfengine::process does.
 */
template<typename... A>
struct kwargs {
  template<typename T>
  using has_arg = contains<T, A...>;

  /** \warning take_arg() can only be called ONCE for each type, because its
   *           return value *has been std::move'ed
   */
  template<typename T, typename... Rest>
  static T && take_arg(T && t, Rest&&... ) {
    return t;
  }
  template<typename T, typename... Rest>
  static T && take_arg(T & t, Rest&&... ) {
    static_assert( ! kwargs<Rest...>::template has_arg<T>::value,
                   "You cannot specify the same keyword argument multiple times" );
    return std::move(t);
  }
  template<typename T, typename O, typename... Rest>
  static auto take_arg(O && , Rest&&... a) -> 
    typename std::enable_if<
      !std::is_same<T,typename std::remove_reference<O>::type>::value,
      T &&>::type
  {
    return take_arg<T>(std::forward<Rest>(a)...);
  }
  template<typename T>
  static T && take_arg()
  {
    throw std::invalid_argument(
        std::string("Expected argument ") + get_type_name<T>() + "{}"
        );
  }
};




// ---------------------------------------------------------




inline
std::string dbl_to_string(double dval)
{
  // // to_string gives "8.000000", and I'm not a fan of stringstream...
  // return std::to_string(dval);
  std::vector<char> s;
  s.resize(128);
  int n = std::snprintf(s.data(), s.size(), "%.8g", dval);
  s.resize(n);
  return std::string{s.begin(), s.end()};
}




// ---------------------------------------------------------



inline
void dump_cstr_to_file(const std::string & fname, const char * c_buffer)
{
  std::FILE * fp;
  fp = std::fopen(fname.c_str(), "w");
  if (fp == NULL) {
    throw std::system_error{errno, std::generic_category()};
  }
  int res = std::fputs(c_buffer, fp);
  if (res == EOF) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::fclose(fp);
}

inline
void dump_binary_data_to_file(const std::string & fname, const binary_data & data)
{
  std::FILE * fp;
  fp = std::fopen(fname.c_str(), "w");
  if (fp == NULL) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::size_t res = std::fwrite( data.data(), 1, data.size(), fp);
  if (res != data.size()) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::fclose(fp);
}

inline
binary_data load_file_data(const std::string & fname)
{
  std::size_t fsiz = fs::file_size(fs::path{fname});
  binary_data data;
  data.resize(fsiz);

  std::FILE * fp;
  fp = std::fopen(fname.c_str(), "rb");
  if (fp == NULL) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::size_t res = std::fread(&data[0], 1, fsiz, fp);
  if (res < fsiz) {
    throw std::system_error{errno, std::generic_category()};
  }
  // check that all data was read
  // if ( ! std::feof(fp) ) {
  //   throw std::runtime_error{"Expected that we were at end of stream after reading "
  //                            + std::to_string(fsiz) + " bytes from " + fname};
  // }
  std::fclose(fp);

  return data;
}




// ---------------------------------------------------------


inline std::string to_lowercase(std::string x)
{
  std::transform(x.begin(), x.end(), x.begin(), ::tolower);
  return x;
}


//
// thanks https://stackoverflow.com/a/25385766/1694896
//

constexpr char ws[] = " \t\n\r\f\v";

// trim from end of string (right)
inline void str_rtrim(std::string & s, const std::string & t = ws)
{
  s.erase(s.find_last_not_of(t) + 1);
}

// trim from beginning of string (left)
inline void str_ltrim(std::string & s, const std::string & t = ws)
{
  s.erase(0, s.find_first_not_of(t));
}

// trim from both ends of string (right then left)
inline void str_trim(std::string & s, const std::string & t = ws)
{
  str_rtrim(s, t);
  str_ltrim(s, t);
}




inline
std::vector<std::string> str_split_rx(
    std::string::const_iterator a, std::string::const_iterator b,
    const std::regex & rx_sep, bool skip_empty = false
    )
{
  std::vector<std::string> items;
  auto last_sep_end = a;

  for (std::sregex_iterator it = std::sregex_iterator(a, b, rx_sep);
       it != std::sregex_iterator{}; ++it) {
    // iterating over matches of the separator regex
    std::smatch sep_match = *it;
    if (!skip_empty || sep_match[0].first != last_sep_end) {
      items.push_back(std::string{last_sep_end,sep_match[0].first});
    }
    last_sep_end = sep_match[0].second;
  }
  if (!skip_empty || last_sep_end != b) {
    items.push_back(std::string{last_sep_end,b});
  }
  return items;
}






} // namespace utils
} // namespace detail
} // namespace klfengine
