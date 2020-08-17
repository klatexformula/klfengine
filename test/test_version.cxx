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
#include <klfengine/version>


#include <catch2/catch.hpp>



TEST_CASE( "version() and implementation_version() match component macros", "[version]" )
{
  REQUIRE( klfengine::version().major == KLFENGINE_VERSION_MAJOR ) ;
  REQUIRE( klfengine::version().minor == KLFENGINE_VERSION_MINOR ) ;
  REQUIRE( klfengine::version().release == KLFENGINE_VERSION_RELEASE ) ;
  REQUIRE( klfengine::version().suffix == KLFENGINE_VERSION_SUFFIX ) ;

  REQUIRE( klfengine::implementation_version().major == KLFENGINE_VERSION_MAJOR ) ;
  REQUIRE( klfengine::implementation_version().minor == KLFENGINE_VERSION_MINOR ) ;
  REQUIRE( klfengine::implementation_version().release == KLFENGINE_VERSION_RELEASE ) ;
  REQUIRE( klfengine::implementation_version().suffix == KLFENGINE_VERSION_SUFFIX ) ;
}

TEST_CASE( "_KLFENGINE_CONCAT_VER_4_j works for different suffixes/combinations", "[version]" )
{
  const char va[] = _KLFENGINE_CONCAT_VER_4_j(14,0,3, "beta-4.d", ".");
  REQUIRE( std::string( va ) == "14.0.3beta-4.d" );

  const char vb[] = _KLFENGINE_CONCAT_VER_4_j(0,0,0, "", ".");
  REQUIRE( std::string( vb ) == "0.0.0" );

  const char va2[] = _KLFENGINE_CONCAT_VER_4_j(14,0,3, "beta-4.d", "xzzx");
  REQUIRE( std::string( va2 ) == "14xzzx0xzzx3beta-4.d" );

  const char vb2[] = _KLFENGINE_CONCAT_VER_4_j(0,0,0, "", "/");
  REQUIRE( std::string( vb2 ) == "0/0/0" );
}


TEST_CASE( "KLFENGINE_VERSION_STRING matches individual components", "[version]" )
{
  // std::cerr << "VERSION IS '" << KLFENGINE_VERSION_STRING << "'\n";
  REQUIRE(
      std::string(KLFENGINE_VERSION_STRING) ==
      (std::to_string(KLFENGINE_VERSION_MAJOR) + "." +
       std::to_string(KLFENGINE_VERSION_MINOR) + "." +
       std::to_string(KLFENGINE_VERSION_RELEASE) + KLFENGINE_VERSION_SUFFIX)
      ) ;
}

TEST_CASE( "KLFENGINE_VERSION_STRING is usable in a constexpr expression", "[version]" )
{
  constexpr char the_version[] = KLFENGINE_VERSION_STRING;

  // actually we only need to check that this test compiles
  REQUIRE( std::string(the_version) == std::string(KLFENGINE_VERSION_STRING) ) ;
}
