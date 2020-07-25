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


#include <klfengine/value>

#include <nlohmann/json.hpp>

namespace klfengine {

namespace detail {

// !!!?!?! THESE FUNCTIONS need to be in the "detail" namespace ??! prolly
// !!!bc. the original class recursive_variant_with_array_and_dict is there.
//
// TODO - FIXME: template these functions and write them for general
// recursive_variant_with_array_and_dict objects, with variant visitors
// etc. (??? ### - but then how do we resolve types to JSON types reliably?)

inline void to_json(nlohmann::json & j, const value & v)
{
  // types order is:
  // bool,
  // int,
  // double,
  // std::nullptr_t,
  // std::string,
  // array,
  // dict

  // TODO:FIXME: ......... this should really be implemented with a template
  // visitor.  Let's worry about this a bit later after everything runs
  // smoothly, to ease debugging

  const auto index = v._data.index();

  switch (index) {
  case 0: // bool
    j = v.get<bool>();
    return;
  case 1: // int
    j = v.get<int>();
    return;
  case 2: // double
    j = v.get<double>();
    return;
  case 3: // nullptr_t
    j = nullptr;
    return;
  case 4: // std::string
    j = v.get<std::string>();
    return;
  case 5: // array
    {
      const value::array & a = v.get<value::array>();
      nlohmann::json ja = nlohmann::json::array();
      for (value::array::const_iterator it = a.begin(); it != a.end(); ++it) {
        nlohmann::json jj;
        to_json(jj, *it);
        ja.push_back(std::move(jj));
      }
      j = std::move(ja);
      return;
    }
  case 6: // dict
    {
      const value::dict & d = v.get<value::dict>();
      nlohmann::json jd = nlohmann::json::object();
      for (value::dict::const_iterator it = d.begin(); it != d.end(); ++it) {
        nlohmann::json jv;
        to_json(jv, it->second);
        jd[it->first] = std::move(jv);
      }
      j = std::move(jd);
      return;
    }
  default:
    throw std::runtime_error("Invalid klfengine::value variant index: "
                             + std::to_string(index));
  }
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
    throw std::runtime_error(std::string("Unknown JSON type to convert: ")
                             + j.dump());
  }
}

} // namespace detail

} // namespace klfengine
