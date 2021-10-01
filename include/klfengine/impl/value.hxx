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
#include <klfengine/value>

#include <nlohmann/json.hpp>

namespace klfengine {

_KLFENGINE_INLINE
bool parse_boolean(std::string str)
{
  std::size_t i = 0;
  while (i < str.size() && std::isspace(str[i])) {
    ++i;
  }
  str.erase( str.begin(), str.begin() + i );
  std::size_t j = str.size();
  while (j > 0 && std::isspace(str[j-1])) {
    --j;
  }
  str.erase( str.begin() + j, str.end() );

  std::transform(str.begin(), str.end(), str.begin(), ::tolower);

  if (str == "t" || str == "true" || str == "y" || str == "yes" || str == "on") {
    return true;
  }
  if (str == "f" || str == "false" || str == "n" || str == "no" || str == "off") {
    return false;
  }

  // try to parse an integer
  try {

    int result = std::stoi(str, &i);
    if (i == str.size()) {
      // consumed entire string -> got integer
      return static_cast<bool>(result);
    }

  } catch (std::invalid_argument & e) {
    // ignore this exception, we'll throw our own exception
  }

  throw std::invalid_argument("Invalid boolean value: `" + str + "'");
}



namespace detail {

// !!: The to/from_json() functions need to be in the detail namespace, because
//     that's where 'recursive_variant_with_vector_and_map' is defined.  To
//     avoid additional ambiguity, we also define the 'value' type in the detail
//     namespace and then "import" value in the klfengine namespace.  This way
//     it makes sense in all cases to define to/from_json() in the detail
//     namespace.

struct value_to_json_visitor {
  nlohmann::json & j;

  void operator()(const value::array & a);
  void operator()(const value::dict & a);

  template<typename SimpleType,
           typename SimpleTypeNoRef = typename std::remove_reference<SimpleType>::type,
           typename std::enable_if<
             !(std::is_same<SimpleTypeNoRef, value::array>::value ||
               std::is_same<SimpleTypeNoRef, value::dict>::value),
           bool>::type = true>
  void operator()(SimpleType x) {
    j = x;
  }
};

inline void to_json(nlohmann::json & j, const value & v)
{
  v.visit( value_to_json_visitor{j} );
}

inline void value_to_json_visitor::operator()(const value::array & a)
{
  nlohmann::json ja = nlohmann::json::array();
  for (value::array::const_iterator it = a.begin(); it != a.end(); ++it) {
    nlohmann::json jj;
    to_json(jj, *it);
    ja.push_back(std::move(jj));
  }
  j = std::move(ja);
}
inline void value_to_json_visitor::operator()(const value::dict & d)
{
  nlohmann::json jd = nlohmann::json::object();
  for (value::dict::const_iterator it = d.begin(); it != d.end(); ++it) {
    nlohmann::json jv;
    to_json(jv, it->second);
    jd[it->first] = std::move(jv);
  }
  j = std::move(jd);
}


inline void from_json(const nlohmann::json & j, value & v)
{
  using nlohmann::json;

  // from nlohmann/json README -->
  // // convenience type checkers
  // j.is_null();
  // j.is_boolean();
  // j.is_number();
  // j.is_object();
  // j.is_array();
  // j.is_string();

  v = value{};

  const auto jtype = j.type();

  switch (jtype) {
  case json::value_t::null:
    v._data = nullptr;
    return;

  case json::value_t::boolean:
    v._data = j.get<bool>();
    return;

  case json::value_t::number_integer: // fallthrough ->
  case json::value_t::number_unsigned:
    v._data = j.get<int>();
    return;

  case json::value_t::number_float:
    v._data = j.get<double>();
    return;

  case json::value_t::string: // fallthrough ->
  case json::value_t::binary:
    v._data = j.get<std::string>();
    return;
    
  case json::value_t::array:
    {
      value::array varray;
      for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
        value item_val;
        from_json(*it, item_val);
        varray.push_back(std::move(item_val));
      }
      v._data = std::move(varray);
      return;
    }
  case json::value_t::object:
    {
      value::dict vdict;
      for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
        value item_val;
        from_json(it.value(), item_val);
        vdict[it.key()] = std::move(item_val);
      }
      v._data = std::move(vdict);
      return;
    }
  default:
    {
      throw invalid_json_value{"klfengine::value", j};
    }
  }
}



struct value_hasher_visitor
{
  std::size_t * seed_ptr;

  void operator()(const value::array & a);
  void operator()(const value::dict & d);

  template<typename SimpleType,
           typename SimpleTypeNoRef = typename std::remove_reference<SimpleType>::type,
           typename std::enable_if<
             !(std::is_same<SimpleTypeNoRef, value::array>::value ||
               std::is_same<SimpleTypeNoRef, value::dict>::value),
           bool>::type = true>
  void operator()(SimpleType x)
  {
    assert(seed_ptr != nullptr) ;
    hash_combine(*seed_ptr, std::hash<SimpleType>{}(x));
  }
};

inline
void value_hasher_visitor::operator()(const value::array & a)
{
  assert(seed_ptr != nullptr) ;
  for (auto const& x : a) {
    x.visit(value_hasher_visitor{seed_ptr});
  }
}
inline
void value_hasher_visitor::operator()(const value::dict & d)
{
  assert(seed_ptr != nullptr) ;
  for (auto const& x : d) {
    hash_combine(*seed_ptr, std::hash<value::dict::key_type>{}(x.first));
    x.second.visit(value_hasher_visitor{seed_ptr});
  }
}



_KLFENGINE_INLINE
std::size_t hash_value(const value & v)
{
  std::size_t seed = 0;
  v.visit(value_hasher_visitor{&seed});
  return seed;
}

_KLFENGINE_INLINE
std::size_t hash_value_dict(const value::dict & d)
{
  std::size_t seed = 0;
  value_hasher_visitor{&seed}(d);
  return seed;
}


} // namespace detail

} // namespace klfengine
