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


namespace klfengine
{

template<Args...>
using variant_type = std::variant<Args...>;


using length = variant_type<double, std::string>;

// store color as RGBA or custom string
using color = variant_type<std::tuple<uint8,uint8,uint8,uint8>, std::string>;



struct input
{
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
  variant_type<std::string, std::pair<std::string,std::string>> math_mode;

  /** \brief Code to include in the LaTeX preamble
   *
   * This LaTeX code is included in the LaTeX document preamble, that is,
   * between <code>\\documentstyle{...}</code> and
   * <code>\\begin{document}</code>.
   */
  std::string preamble;

  std::string latex_engine;

  /** \brief LaTeX commands to set the font
   *
   * Specifying commands here (instead of a point size) is much more
   * flexible..... ........... ............ ????????? THINK!!!!
   * ........... MIGHT BE BETTER TO KEEP 'double' AFTER ALL, MUCH CLEARER
   * WHAT WE MEAN.
   *
   */
  std::string font;

  color fg_color;
  color bg_color;

  std::tuple<length, length, length, length> margins;

  int dpi;

  double vector_scale;

  bool outline_fonts;
};


};
