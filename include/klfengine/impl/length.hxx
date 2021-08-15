/* This file is part of the klfengine library, which is distributed under the
 * terms of the MIT license.
 *
 *     https://github.com/klatexformula/klfengine
 *
 * The MIT License (MIT)
 *
 * Copyright 2021 Philippe Faist
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

#include <cstdlib>
#include <cctype> // std::isspace
#include <string>

#include <klfengine/length>
#include <klfengine/h/detail/utils.h>

#include <nlohmann/json.hpp>


namespace klfengine {



_KLFENGINE_INLINE
length::length(const std::string & s)
{
  std::size_t parse_pos;
  value = std::stod(s, &parse_pos);
  unit = s.substr(parse_pos);
  detail::utils::str_trim(unit);
}
_KLFENGINE_INLINE
length::length(const char *s)
{
  if (s == nullptr) {
    throw std::invalid_argument("Cannot initialize klfengine::length from nullptr");
  }
  char * str_rest;
  value = std::strtod(s, &str_rest);
  unit = std::string{str_rest};
  detail::utils::str_trim(unit);
}

// _KLFENGINE_INLINE
// length::length(const length & other)
//   : value(other.value), unit(other.unit)
// {
// }
// _KLFENGINE_INLINE
// length::length(length && other) noexcept
//   : value(other.value), unit(std::move(other.unit))
// {
// }
// _KLFENGINE_INLINE
// length::~length()
// {
// }
// _KLFENGINE_INLINE
// length & length::operator=(const length & other)
// {
//   value = other.value;
//   unit = other.unit;
//   return *this;
// }
// _KLFENGINE_INLINE
// length & length::operator=(length && other) noexcept
// {
//   value = other.value;
//   std::swap(unit, other.unit);
//   return *this;
// }



_KLFENGINE_INLINE
std::string length::to_string() const
{
  return detail::utils::dbl_to_string(value) + unit;
}


_KLFENGINE_INLINE
double length::to_value_as_pt() const
{
  // 'p','t' for "pt"
  return detail::texlength_to_value_as_unit<65536L, 1,  'p','t'>( value, unit );
}

_KLFENGINE_INLINE
double length::to_value_as_bp() const
{
  // 'b','p' for "bp"
  return detail::texlength_to_value_as_unit<65536L*7227, 72*100,  'b','p'>( value, unit );
}





_KLFENGINE_INLINE
bool operator==(const length & a, const length & b)
{
  return a.value == b.value && a.unit == b.unit;
}
_KLFENGINE_INLINE
bool operator!=(const length & a, const length & b)
{
  return ! (a == b);
}

_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const length & v)
{
  j = nlohmann::json{
    {"value", v.value},
    {"unit", v.unit}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, length & v)
{
  if (j.is_object()) {
    j.at("value").get_to(v.value);
    j.at("unit").get_to(v.unit);
    return;
  }
  if (j.is_string()) {
    v = length{j.get<std::string>()};
    return;
  }
  if (j.is_number()) {
    v = length{j.get<double>()};
    return;
  }
  throw invalid_json_value{"klfengine::length", j};
}



} // namespace klfengine
