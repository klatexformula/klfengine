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



#include <cstdint>

#include <klfengine/basedefs>

#include <nlohmann/json.hpp>


namespace klfengine {

class cannot_convert_length_units : public klfengine::exception
{
public:
  cannot_convert_length_units(std::string msg)
    : klfengine::exception(std::move(msg)) { }
};


/** \brief Store a LaTeX length with a value and a unit
 *
 * Lengths are stored as a pair of a floating-point value and a unit (one of
 * "pt", "mm", "em", etc.).  We don't directly store values in a standardized
 * unit because some units depend on the context (e.g. "ex", "em").
 *
 * \note Two length objects compare as equal only if they have the same units!
 *       E.g. "2cm" and "20mm" are considered different "length objects".
 */
struct length {
  double value;
  std::string unit;

  /** \brief Construct length from value and TeX unit (by default "pt")
   */
  inline length(double value_ = 0.0, std::string unit_ = "pt")
    : value(value_), unit(unit_) { }
  /** \brief Read length from a string
   *
   * The string must be of the form "<value><unit>", where value is a real
   * number.  Whitespace is allowed before value, and it is trimmed from the
   * unit.
   */
  explicit length(const std::string & s);  
  /** \brief Read length from a string
   *
   * Behaves in the same way as \ref length(const std::string &).
   */
  explicit length(const char *s);

  // // rule of 5
  // length(const length &);
  // length(length &&) noexcept;
  // ~length();
  // length & operator=(const length &);
  // length & operator=(length &&) noexcept;

  /** \brief String representation of the length
   *
   * Returns the value as a string, concatenated with the unit.
   */
  std::string to_string() const;

  /** \brief Convert length to TeX point
   *
   * Return the length value converted into TeX points.  Recall 72.27pt == 1in.
   *
   * Usual valid TeX units are accepted (e.g., "mm", "cm", "in", "pt", "bp",
   * "sp").
   *
   * An exception (\ref klfengine::exception) is thrown if the unit is unknown
   * or if the unit depends on the context (e.g. "em", "ex").
   *
   */
  double to_value_as_pt() const;

  /** \brief Convert length to PostScript point
   *
   * Return the length value converted into PostScipt points ("bp").  Recall 72bp == 1in.
   *
   * Behaves otherwise similarly to \ref to_value_as_pt().
   */
  double to_value_as_bp() const;
};



namespace detail {
//
// this needs to be in the header file because we'd like to test this routine
// directly in test_length.cxx
//
// 1 custom unit ("unitchar12")  ==  newunit_sp / newunit_sp_denom  scaled points ("sp")
template<int64_t newunit_sp, int64_t newunit_sp_denom, char unitchar1, char unitchar2>
inline double texlength_to_value_as_unit(double value, const std::string & unit)
{
  // TeXbook, p. 57
  if (unit == "pt") {
    return value * (double(newunit_sp_denom*65536) / newunit_sp);
  } else if (unit == "pc") {
    return value * (double(newunit_sp_denom*12*65536) / newunit_sp);
  } else if (unit == "in") {
    return value * (double(newunit_sp_denom*7227*65536) / (newunit_sp*100));
  } else if (unit == "bp") {
    return value * (double(newunit_sp_denom*7227*65536) / (newunit_sp*7200));
  } else if (unit == "cm") {
    return value * (double(newunit_sp_denom*7227*65536) / (newunit_sp*254));
  } else if (unit == "mm") {
    return value * (double(newunit_sp_denom*7227*65536) / (newunit_sp*2540));
  } else if (unit == "dd") {
    return value * (double(newunit_sp_denom*1238*65536) / (newunit_sp*1157));
  } else if (unit == "cc") {
    return value * (double(newunit_sp_denom*12*1238*65536) / (newunit_sp*1157));
  } else if (unit == "sp") {
    return value * double(newunit_sp_denom) / newunit_sp;
  }
  throw klfengine::cannot_convert_length_units(
    "klfengine::length: Cannot convert from unsupported unit `" + unit + "' "
    "to `" + std::string(1,unitchar1) + std::string(1,unitchar2) + "'"
  );
}
} // namespace detail



} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/length.hxx>
#endif
