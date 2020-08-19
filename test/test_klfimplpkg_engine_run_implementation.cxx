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
#include <klfengine/h/klfimplpkg_engine/run_implementation.h>


#include <klfengine/h/klfimplpkg_engine/engine.h>


#include <catch2/catch.hpp>

TEST_CASE( "something happens when this and that 89yt43urnj", "[keyword]" )
{
  klfengine::klfimplpkg_engine::engine e;

  e.set_settings(klfengine::settings::detect_settings());

  klfengine::input in;
  in.latex = std::string("\\int \\left[a + \\frac{b}{f(x)}\\right] dx =: Z[f]");
  in.math_mode = std::make_pair("$\\begin{aligned}", "\\end{aligned}$");
  in.preamble = std::string("\\usepackage{amsmath}\n\\usepackage{amssymb}");
  in.latex_engine = std::string("pdflatex");
  in.font_size = -1.0;
  in.margins = klfengine::margins{0.4, 0.4, 0.4, 0.4};
  in.dpi = 1200;
  in.scale = 1.0;
  in.outline_fonts = true;
  in.parameters = klfengine::value::dict{
    {"use_documentclass", klfengine::value{std::string{"article"}}}
  };

  auto r = e.run(in);

  r->compile();

  auto data = r->get_data(klfengine::format_spec{"PDF"});

  std::string data_str{reinterpret_cast<const char*>(&data[0]), data.size()};
  CAPTURE( data_str );
  // fprintf(stderr, "PDF DATA IS:\n");
  // fwrite(&data[0], 1, data.size(), stderr);   fprintf(stderr, "\n");

  // also write to /tmp/ for my own testing...
  // FILE * fp_debug_out = fopen("/tmp/mytestoutput.pdf", "w");
  // fwrite(&data[0], 1, data.size(), fp_debug_out);
  // fclose(fp_debug_out);

  // write tests here
  REQUIRE( false ) ;
}
