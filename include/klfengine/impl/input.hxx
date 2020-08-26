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

#include <klfengine/input>

#include <nlohmann/json.hpp>


namespace klfengine {



_KLFENGINE_INLINE
bool operator==(const color & a, const color & b)
{
  return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}
_KLFENGINE_INLINE
bool operator!=(const color & a, const color & b)
{
  return ! (a == b);
}

_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const color & v)
{
  j = nlohmann::json{
    {"red", v.red},
    {"green", v.green},
    {"blue", v.blue},
    {"alpha", v.alpha}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, color & v)
{
  j.at("red").get_to(v.red);
  j.at("green").get_to(v.green);
  j.at("blue").get_to(v.blue);
  j.at("alpha").get_to(v.alpha);
}




_KLFENGINE_INLINE
bool operator==(const margins & a, const margins & b)
{
  return a.top == b.top && a.right == b.right &&
    a.bottom == b.bottom && a.left == b.left;
}
_KLFENGINE_INLINE
bool operator!=(const margins & a, const margins & b)
{
  return ! (a == b);
}

_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const margins & v)
{
  j = nlohmann::json{
    {"top", v.top},
    {"right", v.right},
    {"bottom", v.bottom},
    {"left", v.left}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, margins & v)
{
  j.at("top").get_to(v.top);
  j.at("right").get_to(v.right);
  j.at("bottom").get_to(v.bottom);
  j.at("left").get_to(v.left);
}



_KLFENGINE_INLINE
bool operator==(const input & a, const input & b)
{
  return (
      a.latex == b.latex &&
      a.math_mode == b.math_mode &&
      a.preamble == b.preamble &&
      a.latex_engine == b.latex_engine &&
      a.font_size == b.font_size &&
      a.fg_color == b.fg_color &&
      a.bg_color == b.bg_color &&
      a.margins == b.margins &&
      a.dpi == b.dpi &&
      a.scale == b.scale &&
      a.outline_fonts == b.outline_fonts &&
      a.parameters == b.parameters
      );
}

_KLFENGINE_INLINE
bool operator!=(const input & a, const input & b)
{
  return ! (a == b);
}



_KLFENGINE_INLINE
input::input()
  : latex{},
    math_mode{"\\(", "\\)"},
    preamble{""},
    latex_engine{"pdflatex"},
    font_size{11.0},
    fg_color{0,0,0,255},
    bg_color{255,255,255,0},
    margins{0, 0, 0, 0},
    dpi{600},
    scale{1.0},
    outline_fonts{true},
    parameters{}
{
}



_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const input & v)
{
  j = nlohmann::json{
    {"latex", v.latex},
    {"math_mode", v.math_mode},
    {"preamble", v.preamble},
    {"latex_engine", v.latex_engine},
    {"font_size", v.font_size},
    {"fg_color", v.fg_color},
    {"bg_color", v.bg_color},
    {"margins", v.margins},
    {"dpi", v.dpi},
    {"scale", v.scale},
    {"outline_fonts", v.outline_fonts},
    {"parameters", v.parameters}
  };
}

_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, input & v)
{
  j.at("latex").get_to(v.latex);
  j.at("math_mode").get_to(v.math_mode);
  j.at("preamble").get_to(v.preamble);
  j.at("latex_engine").get_to(v.latex_engine);
  j.at("font_size").get_to(v.font_size);
  j.at("fg_color").get_to(v.fg_color);
  j.at("bg_color").get_to(v.bg_color);
  j.at("margins").get_to(v.margins);
  j.at("dpi").get_to(v.dpi);
  j.at("scale").get_to(v.scale);
  j.at("outline_fonts").get_to(v.outline_fonts);
  j.at("parameters").get_to(v.parameters);
}



} // namespace klfengine
