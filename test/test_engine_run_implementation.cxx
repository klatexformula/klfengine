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
#include <klfengine/engine_run_implementation>

#include <catch2/catch.hpp>

#include "dummy_engine/dummy_engine.hxx"


//
// GCC/Clang warn about initializations like ``format_spec{"PNG"}`` because
// there is no initializer for the `parameters` field.  The warning is enabled
// via -Wall/-Wextra and we turn warnings into errors to help catch errors. But
// the standard allows this initialization, so we deactivate this warning
// entirely for these tests.  See also
// https://stackoverflow.com/a/13373951/1694896
//
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"



TEST_CASE( "engine_run_implementation.compile() calls impl_compile",
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_run_impl x{ klfengine::input{}, klfengine::settings{} };
  x.compile();
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_compile()"
    }) ;
}


TEST_CASE( "engine_run_implementation returns references to input and settings",
           "[engine_run_implementation]" )
{
  const klfengine::input in{"a + b = c"};
  const klfengine::settings sett{"/tmp", "/Library/TeX/texbin/", "none", "", {}};

  dummy_engine::dummy_run_impl x{ in, sett };

  REQUIRE( x.input() == in ) ;
  REQUIRE( x.settings() == sett ) ;
}




TEST_CASE( "engine_run_implementation.canonical_format() calls impl_make_canonical "
           "and behaves correctly" ,
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_run_impl x{ klfengine::input{}, klfengine::settings{} };

  x.compile();

  REQUIRE(
      x.canonical_format(klfengine::format_spec{"TEX", {}})
      == klfengine::format_spec{"TEX", {}}
      ) ;

  REQUIRE(
      x.canonical_format(
          klfengine::format_spec{"HTML",
                                 {{"italic", klfengine::value{false}},
                                  {"bold", klfengine::value{true}}}}
          )
      == klfengine::format_spec{"HTML", {{"bold", klfengine::value{true}}}}
      ) ;

  CHECK_THROWS_AS(
      x.canonical_format(klfengine::format_spec{"PNG", {}}),
      klfengine::no_such_format
      ) ;

  // test enforces "JPEG" instead of "JPG"
  CHECK_THROWS_AS(
      x.canonical_format(klfengine::format_spec{"JPG", {}}),
      klfengine::no_such_format
      ) ;

  CHECK_THROWS_AS(
      x.canonical_format(
          klfengine::format_spec{"HTML",
                                 {{"invalid_option", klfengine::value{false}}}}
          ),
      klfengine::no_such_format
      ) ;

  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_compile()",
      "impl_make_canonical(TEX, 0)",
      "impl_make_canonical(HTML:{\"bold\":true,\"italic\":false}, 0)",
      "impl_make_canonical(PNG, 0)",
      // JPG (instead of JPEG) caught by base class immediately -- so not in this log
      "impl_make_canonical(HTML:{\"invalid_option\":false}, 0)",
    }) ;
}



TEST_CASE( "engine_run_implementation.canonical_format_or_empty() calls impl_make_canonical "
           "and behaves correctly" ,
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_run_impl x{ klfengine::input{}, klfengine::settings{} };

  x.compile();

  REQUIRE(
      x.canonical_format_or_empty(klfengine::format_spec{"TEX", {}})
      == klfengine::format_spec{"TEX", {}}
      ) ;

  REQUIRE(
      x.canonical_format_or_empty(
          klfengine::format_spec{"HTML",
                                 {{"italic", klfengine::value{false}},
                                  {"bold", klfengine::value{true}}}}
          )
      == klfengine::format_spec{"HTML", {{"bold", klfengine::value{true}}}}
      ) ;

  REQUIRE(
      x.canonical_format_or_empty(klfengine::format_spec{"PNG", {}})
      == klfengine::format_spec{}
      ) ;

  // test enforces "JPEG" instead of "JPG"
  REQUIRE(
      x.canonical_format_or_empty(klfengine::format_spec{"JPG", {}})
      == klfengine::format_spec{}
      ) ;

  REQUIRE(
      x.canonical_format_or_empty(
          klfengine::format_spec{"HTML",
                                 {{"invalid_option", klfengine::value{false}}}}
          )
      == klfengine::format_spec{}
      ) ;

  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_compile()",
      "impl_make_canonical(TEX, 0)",
      "impl_make_canonical(HTML:{\"bold\":true,\"italic\":false}, 0)",
      "impl_make_canonical(PNG, 0)",
      // JPG (instead of JPEG) caught by base class immediately -- so not in this log
      "impl_make_canonical(HTML:{\"invalid_option\":false}, 0)",
    }) ;
}




TEST_CASE( "engine_run_implementation.has_format() calls impl_make_canonical "
           "and behaves correctly" ,
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_run_impl x{ klfengine::input{}, klfengine::settings{} };

  x.compile();

  REQUIRE(
      x.has_format(klfengine::format_spec{"TEX", {}})
      ) ;

  REQUIRE(
      x.has_format(
          klfengine::format_spec{"HTML",
                                 {{"italic", klfengine::value{false}},
                                  {"bold", klfengine::value{true}}}}
          )
      ) ;

  REQUIRE(
      ! x.has_format(klfengine::format_spec{"PNG", {}})
      ) ;

  // test enforces "JPEG" instead of "JPG"
  REQUIRE(
      ! x.has_format(klfengine::format_spec{"JPG", {}})
      ) ;

  REQUIRE(
      ! x.has_format(
          klfengine::format_spec{"HTML",
                                 {{"invalid_option", klfengine::value{false}}}}
          )
      ) ;

  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_compile()",
      "impl_make_canonical(TEX, 1)",
      "impl_make_canonical(HTML:{\"bold\":true,\"italic\":false}, 1)",
      "impl_make_canonical(PNG, 1)",
      // JPG (instead of JPEG) caught by base class immediately -- so not in this log
      "impl_make_canonical(HTML:{\"invalid_option\":false}, 1)",
    }) ;
}


TEST_CASE( "engine_run_implementation.get_data_cref produces data and stores to cache",
           "[engine_run_implementation]" )
{
  dummy_engine::dummy_run_impl x{ klfengine::input{"hello world"},
                                  klfengine::settings{} };

  x.compile();

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

  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"TEX", {}})
           == klfengine::binary_data(data_tex.begin(), data_tex.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX, 0)",
      "impl_produce_data(TEX)"
    } );

  // repeated call should retrieve data from cache and not re-produce data
  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"TEX", {}})
           == klfengine::binary_data(data_tex.begin(), data_tex.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX, 0)",
    } );

  // same if format is specified in non-canonical format
  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"TEX", {{"italic", klfengine::value{false}}}})
           == klfengine::binary_data(data_tex.begin(), data_tex.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX:{\"italic\":false}, 0)"
    } );

  // our dummy engine produces TEX & HTML simultaneously, so again, this should
  // not trigger any data production, only a successful cache lookup.  This
  // tests also store_to_cache().
  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"HTML", {{"italic", klfengine::value{false}}}})
           == klfengine::binary_data(data_html.begin(), data_html.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(HTML:{\"italic\":false}, 0)"
    } );

  // now we test recursive calls to get_data_cref.
  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"XML-emb-TEX", {{"bold", klfengine::value{true}}}})
           == klfengine::binary_data(data_xmlembtex_b.begin(), data_xmlembtex_b.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(XML-emb-TEX:{\"bold\":true}, 0)",
      "impl_produce_data(XML-emb-TEX:{\"bold\":true})",
      "impl_make_canonical(TEX:{\"bold\":true}, 0)",
      "impl_produce_data(TEX:{\"bold\":true})"
    } );

  // this should find the data from the cache
  x.record_calls.clear();
  REQUIRE( x.get_data_cref({"TEX", {{"bold", klfengine::value{true}}}})
           == klfengine::binary_data(data_tex_b.begin(), data_tex_b.end()) ) ;
  REQUIRE( x.record_calls == std::vector<std::string>{
      "impl_make_canonical(TEX:{\"bold\":true}, 0)"
    } );

}








TEST_CASE( "engine_run_implementation calls impl_available_formats()",
           "[engine_run_implementation][!mayfail]" )
{
  // TODO/BUG: need to determine what available_formats() should return.  See
  // that class doc.
  REQUIRE( false ) ;
}
