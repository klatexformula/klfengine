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


#include <nlohmann/json.hpp>


namespace klfengine {


void to_json(nlohmann::json::json & j, const value & v)
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

  switch (v.index()) {
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
    j = v.get<value::array>();
    return;
  case 6: // dict
    j = v.get<value::dict>();
    return;
  default:
    throw std::runtime_error("Invalid klfengine::value variant index: "
                             + std::to_string(v.index()));
  }
}
void from_json(const nlohmann::json::json & j, value & v)
{
  using namespace nlohmann::json;

  // from nlohmann/json README -->
  // // convenience type checkers
  // j.is_null();
  // j.is_boolean();
  // j.is_number();
  // j.is_object();
  // j.is_array();
  // j.is_string();

  if (j.is_null()) {
    v = nullptr;
  } else if (j.is_boolean()) {
    v = j.get<bool>();
  } else if (j.is_number()) {
    v = j.get<int>();
  } else if (j.is_object()) {
    value::dict vdict;
    for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
      value item_val;
      from_json(it.value(), item_val);
      vdict[it.key()] = std::move(item_val);
    }
    v = std::move(vdict);
  } else if (j.is_array()) {
    value::array varray;
    for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
      value item_val;
      from_json(*it, item_val);
      varray.push_back(std::move(item_val));
    }
    v = std::move(varray);
  } else if (j.is_string()) {
    v = j.get<std::string>();
  } else {
    throw std::runtime_error(std::string("Unknown JSON type to convert: ")
                             + j.type());
  }
}



} // namespace klfengine
