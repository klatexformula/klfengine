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


#include <cstdint> // uint8_t

#include <string>
#include <vector>
#include <map>


#include <klfengine/basedefs>
#include <klfengine/value>

#include <nlohmann/json.hpp>


namespace klfengine
{

/** \brief Store a LaTeX length (for now, only in points)
 *
 * For now, this is a double floating-point value, with units of LaTeX points
 * ("1pt").
 *
 * \bug  In the future, we should have a fp-value associated with an explicit
 *       latex unit, without normalizing everything to points, so that we can
 *       specify font-dependent lengths such as "0.3ex".
 */
using length = double;

/** \brief Store an RGBA color with transparency
 *
 * The color is stored as a tuple of R,G,B,A values, each ranging 0-255.
 */
struct color {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  std::uint8_t alpha;
};


/** \brief Margins around a box
 */
struct margins {
  length top;
  length right;
  length bottom;
  length left;
};

/** \brief Description of a piece of LaTeX code and how to compile it
 *
 */
struct input
{
  // initialize with some reasonable default values
  input();

  // default copy & move semantics
  input(const input & copy) = default;
  input(input && move) = default;
  input & operator=(const input & copy) = default;
  input & operator=(input && move) = default;


  /** \brief The LaTeX code to process.
   *
   * This is typically the equation code, without <code>\\begin{equation}
   * ... \\end{equation}</code> or any other equation markers ("math mode
   * delimiters").
   */
  std::string latex;

  /** \brief Which LaTeX math mode delimiters to use
   *
   * For instance, <code>("\[", "\]")</code>.
   */
  std::pair<std::string,std::string> math_mode;

  /** \brief Code to include in the LaTeX preamble
   *
   * This LaTeX code is included in the LaTeX document preamble, that is,
   * between <code>\\documentstyle{...}</code> and
   * <code>\\begin{document}</code>.
   */
  std::string preamble;

  std::string latex_engine;

  /** \brief Font size in which to typeset the LaTeX code (in LaTeX points)
   *
   *
   */
  length font_size;

  color fg_color;
  color bg_color;

  klfengine::margins margins;

  int dpi;

  double scale;

  bool outline_fonts;

  /** \brief Custom, implementation-specific parameters
   *
   */
  value::dict parameters;
};


bool operator==(const color & a, const color & b);
bool operator!=(const color & a, const color & b);
void to_json(nlohmann::json & j, const color & v);
void from_json(const nlohmann::json & j, color & v);

bool operator==(const margins & a, const margins & b);
bool operator!=(const margins & a, const margins & b);
void to_json(nlohmann::json & j, const margins & v);
void from_json(const nlohmann::json & j, margins & v);

bool operator==(const input & a, const input & b);
bool operator!=(const input & a, const input & b);
void to_json(nlohmann::json & j, const input & v);
void from_json(const nlohmann::json & j, input & v);


} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/input.hxx>
#endif
