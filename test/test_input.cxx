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

// header we are testing gets included first (helps detect missing #include's)
#include <klfengine/input>



#include <catch2/catch.hpp>

//#include <iostream> // DEBUG





TEST_CASE( "struct input has the right fields of the right type", "[input]" )
{
  klfengine::input in;
  in.latex = std::string("latex input");
  in.math_mode = std::make_pair("\\[", "\\]");
  in.preamble = std::string("latex preamble");
  in.latex_engine = std::string("pdflatex");
  in.font_size = 16.0;
  in.fg_color = klfengine::color{22, 80, 127, 255};
  in.bg_color = klfengine::color{250, 252, 253, 0};
  in.margins = klfengine::margins{1.0, 2.0, 3.0, 4.0};
  in.dpi = 1200;
  in.scale = 2.0;
  in.outline_fonts = true;
  in.parameters = klfengine::value::dict{
    {"use_documentclass", klfengine::value{std::string{"article"}}}
  };

  REQUIRE( in.latex == std::string("latex input") );
  REQUIRE( in.math_mode == std::pair<std::string,std::string>("\\[", "\\]") );
  REQUIRE( in.preamble == std::string("latex preamble") );
  REQUIRE( in.latex_engine == std::string("pdflatex") );
  REQUIRE( in.font_size == 16.0 );
  REQUIRE( in.fg_color == klfengine::color{22, 80, 127, 255} );
  REQUIRE( in.bg_color == klfengine::color{250, 252, 253, 0} );
  REQUIRE( in.margins == klfengine::margins{1.0, 2.0, 3.0, 4.0} );
  REQUIRE( in.dpi == 1200 );
  REQUIRE( in.scale == 2.0 );
  REQUIRE( in.outline_fonts == true );
  REQUIRE( in.parameters == klfengine::value::dict{
    {"use_documentclass", klfengine::value{std::string{"article"}}}
  } );
}


// TEST_CASE( "struct input initializes the fields in the correct order", "[input]" )
// {
//   klfengine::input in{
//     "latex input", // latex
//     {"\\[", "\\]"}, // math_mode
//     "latex preamble", // preamble
//     "pdflatex", // latex_engine
//     16.0, // font_size
//     {22, 80, 127, 255}, // fg_color
//     {250, 252, 253, 0}, // bg_color
//     {1.0, 2.0, 3.0, 4.0}, // margins
//     1200, // dpi
//     2.0, // scale
//     true, // outline_fonts
//     {{"use_documentclass", klfengine::value{std::string{"article"}}}} // parameters
//   };
//
//   REQUIRE( in.latex == std::string("latex input") );
//   REQUIRE( in.math_mode == std::pair<std::string,std::string>("\\[", "\\]") );
//   REQUIRE( in.preamble == std::string("latex preamble") );
//   REQUIRE( in.latex_engine == std::string("pdflatex") );
//   REQUIRE( in.font_size == 16.0 );
//   REQUIRE( in.fg_color == klfengine::color{22, 80, 127, 255} );
//   REQUIRE( in.bg_color == klfengine::color{250, 252, 253, 0} );
//   REQUIRE( in.margins == klfengine::margins{1.0, 2.0, 3.0, 4.0} );
//   REQUIRE( in.dpi == 1200 );
//   REQUIRE( in.scale == 2.0 );
//   REQUIRE( in.outline_fonts == true );
//   REQUIRE( in.parameters == klfengine::value::dict{
//     {"use_documentclass", klfengine::value{std::string{"article"}}}
//   } );
// }

klfengine::input test_make_input(
    std::string latex,
    std::pair<std::string,std::string> math_mode,
    std::string preamble,
    std::string latex_engine,
    double font_size,
    klfengine::color fg_color,
    klfengine::color bg_color,
    klfengine::margins margins,
    int dpi,
    double scale,
    bool outline_fonts,
    klfengine::value::dict parameters)
{
  klfengine::input in;
  in.latex = std::move(latex);
  in.math_mode = std::move(math_mode);
  in.preamble = std::move(preamble);
  in.latex_engine = std::move(latex_engine);
  in.font_size = std::move(font_size);
  in.fg_color = std::move(fg_color);
  in.bg_color = std::move(bg_color);
  in.margins = std::move(margins);
  in.dpi = std::move(dpi);
  in.scale = std::move(scale);
  in.outline_fonts = std::move(outline_fonts);
  in.parameters = std::move(parameters);
  return in;
}



TEST_CASE( "struct input compares for (in)equality", "[input]" )
{
  klfengine::input in = test_make_input(
    "latex input", // latex
    {"\\[", "\\]"}, // math_mode
    "latex preamble", // preamble
    "pdflatex", // latex_engine
    16.0, // font_size
    {22, 80, 127, 255}, // fg_color
    {250, 252, 253, 0}, // bg_color
    {1.0, 2.0, 3.0, 4.0}, // margins
    1200, // dpi
    2.0, // scale
    true, // outline_fonts
    {{"use_documentclass", klfengine::value{std::string{"article"}}}} // parameters
  );

  klfengine::input in2 = test_make_input(
    "latex input", // latex
    {"\\[", "\\]"}, // math_mode
    "latex preamble", // preamble
    "pdflatex", // latex_engine
    16.0, // font_size
    {22, 80, 127, 255}, // fg_color
    {250, 252, 253, 0}, // bg_color
    {1.0, 2.0, 3.0, 4.0}, // margins
    1200, // dpi
    2.0, // scale
    true, // outline_fonts
    {{"use_documentclass", klfengine::value{std::string{"article"}}}} // parameters
  );

  klfengine::input in_x = test_make_input(
    "latex input", // latex
    {"\\[", "\\]xx"}, // math_mode    // <-- typo here
    "latex preamble", // preamble
    "pdflatex", // latex_engine
    16.0, // font_size
    {22, 80, 127, 255}, // fg_color
    {250, 252, 253, 0}, // bg_color
    {1.0, 2.0, 3.0, 4.0}, // margins
    1200, // dpi
    2.0, // scale
    true, // outline_fonts
    {{"use_documentclass", klfengine::value{std::string{"article"}}}} // parameters
  );

  REQUIRE( in == in2 );
  REQUIRE( !(in != in2) );

  REQUIRE( in != in_x );
  REQUIRE( !(in == in_x) );
  
}



TEST_CASE( "struct input converts to/from JSON", "[input]" )
{
  klfengine::input in = test_make_input(
    "latex input", // latex
    {"\\[", "\\]"}, // math_mode
    "latex preamble", // preamble
    "pdflatex", // latex_engine
    16.0, // font_size
    {22, 80, 127, 255}, // fg_color
    {250, 252, 253, 0}, // bg_color
    {1.0, 2.0, 3.0, 4.0}, // margins
    1200, // dpi
    2.0, // scale
    true, // outline_fonts
    {{"use_documentclass", klfengine::value{std::string{"article"}}}} // parameters
  );

  nlohmann::json j;
  j = in;

  //std::cerr << "DEBUG: " << j.dump(4) << "\n";

  klfengine::input in2;
  j.get_to(in2);

  REQUIRE( in == in2 );
}

