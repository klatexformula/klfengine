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
#include <klfengine/engine>

#include <klfengine/run>

#include <catch2/catch.hpp>

#include "dummy_engine/dummy_engine.hxx"



TEST_CASE( "engine constructor works and saves name",
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_engine x{};

  REQUIRE( x.name() == "dummy-engine") ;
}


TEST_CASE( "engine saves and returns settings",
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_engine x{};

  const klfengine::settings s{
    "/tmp",
    "/usr/local/texlive/20xx/somewhere/bin/",
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
    }
  };

  x.set_settings(s);

  REQUIRE( x.settings() == s );

  const klfengine::settings s2{
    "/tmp/XXX/xtmp",
    "/opt/texlive/20xx/bin/",
    "none",
    "",
    {}
  };

  // can re-set settings again later

  x.set_settings(s2);

  REQUIRE( x.settings() == s2 );

}


TEST_CASE( "engine calls impl_create_engine_run_instance",
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_engine x{};

  klfengine::input in;
  in.latex = "a+b=c";

  std::unique_ptr<klfengine::run> r = x.run( in );

  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_create_engine_run_implementation(...)"
    }) ;
}

