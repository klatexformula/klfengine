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
#include <klfengine/h/engines/latextoimage/run_implementation.h>

#include <klfengine/engines/latextoimage>
#include <klfengine/h/engines/latextoimage/engine.h>

#include <catch2/catch.hpp>

#include "testutils.hxx"


TEST_CASE( "simple compilation with engines::latextoimage produces correct equation image",
           "[engines-latextoimage-run_implementation]" )
{
  klfengine::engines::latextoimage::engine e;

  e.set_settings(klfengine::settings::detect_settings());

  klfengine::input in;
  in.latex = std::string("\\int \\left[a + \\frac{b}{f(x)}\\right] dx =: Z[f]");
  in.math_mode = std::make_pair("\\begin{align*}", "\\end{align*}");
  in.preamble = std::string("\\usepackage{amsmath}\n\\usepackage{amssymb}");
  in.latex_engine = std::string("pdflatex");
  in.font_size = -1.0;
  in.margins = klfengine::margins{
    klfengine::length{"1bp"},
    klfengine::length{"1bp"},
    klfengine::length{"1bp"},
    klfengine::length{"1bp"}
  };
  in.dpi = 1200;
  in.scale = 1.0;
  in.outline_fonts = true;
  in.bg_color = klfengine::color{255,255,255,255};
  in.parameters = klfengine::value::dict{
    {"document_class", klfengine::value{std::string{"article"}}},
    {"document_class_options", klfengine::value{std::string{"11pt"}}}
  };

  auto r = e.run(in);

  r->compile();

  auto canon_format_spec = r->canonical_format(klfengine::format_spec{"PNG"});
  nlohmann::json canon_format_spec_j;
  canon_format_spec_j = canon_format_spec;
  CAPTURE( canon_format_spec_j.dump() );

  auto pngdata = r->get_data(klfengine::format_spec{"PNG"});

  klfengine::detail::utils::dump_binary_data_to_file("testoutf_931ieowf.png", pngdata);

  require_images_similar("testoutf_931ieowf.png",
                         KLFENGINE_TEST_DATA_DIR "engines_latextoimage_run_implementation_1.png");

}
