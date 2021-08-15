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
#include <functional>


#include <klfengine/basedefs>


#include <nlohmann/json.hpp>


#ifdef KLFENGINE_USE_MPARK_VARIANT
// Use mpark/variant as variant type instead of C++17 std::variant.  Set this
// e.g. as fallback variant type for C++11 & C++14.
#include <mpark/variant.hpp>

#else

// simply use C++17 std::variant
#include <variant>

#endif



namespace klfengine {

namespace detail {

#ifdef KLFENGINE_USE_MPARK_VARIANT
// fallback variant library for C++ < C++17
template<typename... Args>
using variant_type = mpark::variant<Args...>;
#define _KLFENGINE_VARIANT_GET mpark::get
#define _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE mpark::holds_alternative
#define _KLFENGINE_VARIANT_VISIT mpark::visit
#else
template<typename... Args>
using variant_type = std::variant<Args...>;
#define _KLFENGINE_VARIANT_GET std::get
#define _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE std::holds_alternative
#define _KLFENGINE_VARIANT_VISIT std::visit
#endif


// see https://stackoverflow.com/a/43309497/1694896
template<typename... PrimaryDataTypes>
struct recursive_variant_with_vector_and_map
{
  using this_type = recursive_variant_with_vector_and_map<PrimaryDataTypes...>;

  using array = std::vector<this_type>;
  using dict = std::map<std::string,this_type>;

  // !!! CAUTION !!! ANY CHANGE TO THE ORDER HERE MUST BE REFLECTED IN THE
  // FUNCTION to_json() IN value.hxx !!!
  variant_type<PrimaryDataTypes..., array, dict> _data;

  template<typename GetValueType>
  inline const GetValueType & get() const {
    return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
  }
  template<typename GetValueType>
  inline GetValueType & get() {
    return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
  }
  
  template<typename GetValueType>
  inline bool has_type() const {
    return _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<GetValueType>(_data);
  }

  template<typename FnVisitor>
  inline void visit(FnVisitor && fn) const
  {
    _KLFENGINE_VARIANT_VISIT(fn, _data);
  }
  template<typename FnVisitor>
  inline auto transform(FnVisitor && fn) const
    -> decltype( _KLFENGINE_VARIANT_VISIT(fn, _data) )
  {
    return _KLFENGINE_VARIANT_VISIT(fn, _data);
  }

  // template<typename RhsType>
  // inline recursive_variant_with_vector_and_map
  // operator=(RhsType && rhs) {
  //   return _data.operator=(std::forward<RhsType>(rhs));
  // }

  inline bool operator==(const this_type& rhs) const {
    return _data == rhs._data;
  }
  inline bool operator!=(const this_type& rhs) const {
    return _data != rhs._data;
  }
};

// !!! CAUTION !!! ANY CHANGE TO THESE PRIMARY TYPES OR THEIR ORDER MUST BE
// REFLECTED IN THE FUNCTION to_json() IN value.hxx !!!
using value = recursive_variant_with_vector_and_map<
  bool,
  int,
  double,
  std::nullptr_t,
  std::string
>;

// HAVING 'value' IN 'detail' NAMESPACE MAKES IT MORE CLEAR WHY to_json() AND
// from_json() SHOULD LIVE HERE, TOO.

void to_json(nlohmann::json & j, const value & v);
void from_json(const nlohmann::json & j, value & v);


} // namespace detail


/** \brief Store standard JSON-like types, including arrays and maps
 *
 * This is a std::variant type that can store ints, bools, doubles, strings,
 * as well as arrays and maps of such types (maps always have strings as keys).
 *
 * (If KLFENGINE_USE_MPARK_VARIANT is set, the <a
 * href="https://github.com/mpark/variant" target="_blank">mpark/variant</a>
 * library is used instead of std::variant.  See "Using the klfengine library".)
 *
 * You can construct arbitrary values with initializer lists:
 * \code
 *   klfengine::value{
 *     klfengine::value::array{
 *       klfengine::value{v1},
 *       klfengine::value{klfengine::value::dict{
 *         {std::string{"key1"}, dictvalue1},
 *         {std::string{"key2"}, dictvalue2},
 *         (...)
 *       }},
 *       (...)
 *     }
 *  }
 * \endcode
 *
 * \warning Always use explicit std::string's when constructing
 *    klfengine::value's, and not const char * constants, because <a
 *    href="https://stackoverflow.com/a/60683920/1694896">const char * values
 *    get converted to bool</a>.
 *
 * Important members:
 *
 * <b>value::array</b>: a typedef of <code>std::vector<klfengine::value></code>
 *
 * <b>value::dict</b>: a typedef of <code>std::map<std::string,
 * klfengine::value></code>
 */
using value = detail::value;



/** \brief Fetch a value in a map by key
 *
 * This function throws \a std::out_of_range if the key is not in the
 * dictionary.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X = value>
inline X dict_get(const value::dict & dict, const std::string & key)
{
  return dict_get<value>(dict, key).get<X>();
}
template<>
inline value dict_get<value>(const value::dict & dict, const std::string & key)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    throw std::out_of_range("No such key in dictionary: " + key);
  }
  return it->second;
}
/** \brief Fetch a value in a map by key, possibly with a default if the key
 *         does not exist.
 *
 * If the given \a key cannot be found in the dictionary, then \a dflt is
 * returned instead.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X>
inline X dict_get(const value::dict & dict, const std::string & key, X dflt)
{
  return dict_get<value>(dict, key, value{std::move(dflt)}).get<X>();
}
template<>
inline value dict_get<value>(const value::dict & dict, const std::string & key, value dflt)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return dflt; // std::move redundant
  }
  return it->second;
}

/** \brief Execute the given callback if a key exists in the dictionary
 *
 * If the given \a key is found in the dictionary \a dict, then \a fn is
 * executed with the value associated with that key.  If \a key was not found,
 * then nothing happens.
 *
 * Returns \a true if the key was found (and therefore \a fn was executed),
 * otherwise returns \a false.
 */
template<typename X>
inline bool dict_do_if(const value::dict & dict, const std::string & key,
                       std::function<void(const X&)> fn)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return false;
  }
  fn(it->second.get<X>());
  return true;
}
template<>
inline bool dict_do_if<value>(const value::dict & dict, const std::string & key,
                              std::function<void(const value&)> fn)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return false;
  }
  fn(it->second);
  return true;
}





} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/value.hxx>
#endif
