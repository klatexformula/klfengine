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


#include <klfengine/basedefs>


#include <nlohmann/json.hpp>

// todo later: support fallback variant library for C++-standard < C++17
#include <variant>


namespace klfengine {

namespace detail {

// todo later: support fallback variant library for C++ < C++17
template<typename... Args>
using variant_type = std::variant<Args...>;


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
    return std::get<GetValueType>(_data);
  }
  template<typename GetValueType>
  inline GetValueType & get() {
    return std::get<GetValueType>(_data);
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
 * This is a variant type that can store ints, bools, doubles, strings, as well
 * as arrays and maps of such types (maps always have strings as keys).
 *
 * You can construct arbitrary values with initializer lists:
 * \code
 *   klfengine::value{
 *     klfengine::value::array{
 *       klfengine::value{v1},
 *       klfengine::value{klfengine::value::dict{
 *         {std::string("key1"), dictvalue1},
 *         {std::string("key2"), dictvalue2},
 *         (...)
 *       }},
 *       (...)
 *     }
 *  }
 * \endcode
 *
 * \warning Always use explicit std::string's, not const char * constants,
 *    because otherwise <a
 *    href="https://stackoverflow.com/a/60683920/1694896">they get converted to
 *    \a bool</a>.
 *
 */
using value = detail::value;





} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/value.hxx>
#endif
