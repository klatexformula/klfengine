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
#include <klfengine/run>


#include <catch2/catch.hpp>

#include "dummy_engine/dummy_engine.hxx"


//
// test helpers
//
// this is defined at the end of this file
dummy_engine::dummy_run_impl * make_dummy_run_impl_ptr(
    std::string tex_input = "a + b = c"
    );

template<typename... A>
inline std::unique_ptr<klfengine::engine_run_implementation> make_dummy_run_impl(A&&... a)
{
  return std::unique_ptr<klfengine::engine_run_implementation>{
    make_dummy_run_impl_ptr(std::forward<A>(a)...)
  };
}





TEST_CASE( "run constructor does not accept a null pointer as argument", "[run]" )
{
  CHECK_THROWS_AS( klfengine::run{nullptr},
                   std::invalid_argument ) ;
}

TEST_CASE( "run ensures compile() is called exactly once", "[run]" )
{
  { klfengine::run r{make_dummy_run_impl()};
    REQUIRE( r.compiled() == false ) ;
    r.compile();
    REQUIRE( r.compiled() == true ) ;
    CHECK_THROWS_AS( r.compile(),
                     klfengine::dont_call_compile_twice ) ;
  }
  // check the different methods
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.has_format(klfengine::format_spec{"PNG", {}}),
                     klfengine::forgot_to_call_compile ) ;
  }
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.available_formats(),
                     klfengine::forgot_to_call_compile ) ;
  }
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.canonical_format(klfengine::format_spec{"PNG", {}}),
                     klfengine::forgot_to_call_compile ) ;
  }
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.find_format(std::vector<std::string>{"PNG", "JPEG"}),
                     klfengine::forgot_to_call_compile ) ;
  }
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.get_data(klfengine::format_spec{"PNG", {}}),
                     klfengine::forgot_to_call_compile ) ;
  }
  { klfengine::run r{make_dummy_run_impl()};
    CHECK_THROWS_AS( r.get_data_cref(klfengine::format_spec{"PNG", {}}),
                     klfengine::forgot_to_call_compile ) ;
  }

}


TEST_CASE( "run methods call engine_run_implementation methods", "[run]" )
{
  dummy_engine::dummy_run_impl * dimpl = make_dummy_run_impl_ptr("hello world");
  klfengine::run r{std::unique_ptr<dummy_engine::dummy_run_impl>(dimpl)};

  r.compile();

  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_compile()"
    } );

  const std::string data_tex =
    "<compiled data! input was `hello world'>";
  const std::string data_html =
    "&lt;compiled data! input was `hello world'&gt;";
  const std::string data_tex_b =
    "\\textbf{<compiled data! input was `hello world'>}";

  const std::string data_xmlembtex_b =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<dummy-data><![CDATA["
    "\\textbf{<compiled data! input was `hello world'>}"
    "]]></dummy-data>\n";

  dimpl->record_calls.clear();
  REQUIRE( r.has_format("TEX") == true );
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX, 1)"
    } );

  dimpl->record_calls.clear();
  REQUIRE( r.has_format("SVG") == false );
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(SVG, 1)"
    } );

  dimpl->record_calls.clear();
  (void) r.available_formats();
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_available_formats()"
    } );

  dimpl->record_calls.clear();
  REQUIRE( r.canonical_format({"TEX", {{"italic", klfengine::value{false}}}})
           == klfengine::format_spec{"TEX"} ) ;
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX:{\"italic\":false}, 0)"
    } );

  dimpl->record_calls.clear();
  REQUIRE( r.canonical_format_or_empty({"TEX", {{"italic", klfengine::value{false}}}})
           == klfengine::format_spec{"TEX"} ) ;
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX:{\"italic\":false}, 0)"
    } );

  // get_data_cref() works
  dimpl->record_calls.clear();
  REQUIRE( r.get_data_cref({"TEX", {}})
           == klfengine::binary_data(data_tex.begin(), data_tex.end()) ) ;
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX, 0)",
      "impl_produce_data(TEX)"
    } );

  // get_data() works too (note data is not produced a second time in recorded
  // function calls)
  dimpl->record_calls.clear();
  REQUIRE( r.get_data({"TEX", {}})
           == klfengine::binary_data(data_tex.begin(), data_tex.end()) ) ;
  REQUIRE( dimpl->record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX, 0)"
    } );
}



TEST_CASE( "find_format finds acceptable formats", "[run]" )
{
  klfengine::run r{make_dummy_run_impl()};

  r.compile();
  
  // finds a format in a string list
  REQUIRE( r.find_format(std::vector<std::string>{"SVG", "JPEG", "TEX"})
           == klfengine::format_spec{"TEX"} ) ;

  // finds a format in a format_spec list
  REQUIRE( r.find_format(std::vector<klfengine::format_spec>{
        klfengine::format_spec{"SVG"},
        klfengine::format_spec{"JPEG"},
        klfengine::format_spec{"TEX", {{"bubbly_level", klfengine::value{1.34}}}},
        klfengine::format_spec{"TXT", {{"italic", klfengine::value{false}}}},
        klfengine::format_spec{"TXT"}
      }) == klfengine::format_spec{"TXT"} ) ;

}





//
// GCC/Clang warn about initializations like ``klfengine::input{"a+b=c"}``
// because there is no initializer for the remaining fields.  The warning is
// enabled via -Wall/-Wextra and we turn warnings into errors to help catch
// errors. But the standard allows this initialization, so we deactivate this
// warning entirely for these tests.  See also
// https://stackoverflow.com/a/13373951/1694896
//
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


inline dummy_engine::dummy_run_impl * make_dummy_run_impl_ptr(
    std::string tex_input
    )
{
  klfengine::input in;
  in.latex = tex_input;
  return new dummy_engine::dummy_run_impl{
    in,
    klfengine::settings{"/tmp", "/Library/TeX/texbin/", "none", "", {}}
  };
}
