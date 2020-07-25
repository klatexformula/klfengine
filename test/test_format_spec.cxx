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
#include <klfengine/format_spec>


#include <catch2/catch.hpp>


//
// GCC/Clang warn about initializations like format_spec{"PNG"} because there is
// no initializer for the `parameters` field.  The warning is enabled via
// -Wall/-Wextra and we turn warnings into errors to help catch errors. But the
// standard allows this initialization, so we deactivate this warning entirely
// for these tests.  See also https://stackoverflow.com/a/13373951/1694896
//
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


TEST_CASE( "struct format_spec can be initialized and has the right fields", "[format_spec]" )
{
  { const klfengine::format_spec f{"PNG"};
    REQUIRE( f.format == "PNG" );
    REQUIRE( f.parameters == klfengine::value::dict{} );
  }

  { const klfengine::format_spec f{"PNG", {}};
    REQUIRE( f.format == "PNG" );
    REQUIRE( f.parameters == klfengine::value::dict{} );
  }

  { const klfengine::format_spec f{"PNG", {{"raw", klfengine::value{true}}}};
    REQUIRE( f.format == "PNG" );
    REQUIRE( f.parameters.at("raw") == klfengine::value{true} );
  }
}



TEST_CASE( "format_spec converts to string", "[format_spec]" )
{
  { const klfengine::format_spec f{"PNG", {{"raw", klfengine::value{true}}}};
    REQUIRE( f.as_string() == "PNG:{\"raw\":true}" );
  }
}


TEST_CASE( "struct format_description can be initialized and has the right fields",
           "[format_spec]" )
{
  { const klfengine::format_description f{
      {"PNG"},
      "Portable Graphics Format",
      "Standard image format with transparency"
    };
    REQUIRE( f.format_spec.format == "PNG" );
    REQUIRE( f.format_spec.parameters == klfengine::value::dict{} );
    REQUIRE( f.title == "Portable Graphics Format" );
    REQUIRE( f.description == "Standard image format with transparency" );
  }

  { const klfengine::format_description f{
      {"PNG", {{"use_transparency", klfengine::value{false}}}},
      "Portable Graphics Format (opaque)",
      "Standard PNG image format but without transparency"
    };
    REQUIRE( f.format_spec.format == "PNG" );
    REQUIRE( f.format_spec.parameters.at("use_transparency") == klfengine::value{false} );
    REQUIRE( f.title == "Portable Graphics Format (opaque)" );
    REQUIRE( f.description == "Standard PNG image format but without transparency" );
  }
}


TEST_CASE( "can throw no_such_format exception", "[format_spec]" )
{
  CHECK_THROWS_AS(
      throw klfengine::no_such_format("SVG"),
      klfengine::no_such_format
      );

  CHECK_THROWS_AS(
      throw klfengine::no_such_format(
          "SVG",
          "Your ghostscript doesn't support the svg device"
      ),
      klfengine::no_such_format
      );

}
