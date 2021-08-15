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
#include <klfengine/length>

#include <catch2/catch.hpp>


TEST_CASE( "struct length has the right fields of the right type", "[length]" )
{
  klfengine::length len;
  len.value = 2.5;
  len.unit = std::string("cm");
  
  REQUIRE( len.value == 2.5 );
  REQUIRE( len.unit == "cm" );
}


TEST_CASE( "struct length constructor parses std::string", "[length]" )
{
  { klfengine::length len{std::string{"2.5cm"}};
    REQUIRE( len.value == 2.5 );
    REQUIRE( len.unit == "cm" ); }

  { klfengine::length len{std::string{" 2.5\t cm "}};
    REQUIRE( len.value == 2.5 );
    REQUIRE( len.unit == "cm" ); }
}

TEST_CASE( "struct length constructor parses const char*", "[length]" )
{
  { klfengine::length len{"2.5cm"};
    REQUIRE( len.value == 2.5 );
    REQUIRE( len.unit == "cm" ); }

  { klfengine::length len{" 2.5\t cm "};
    REQUIRE( len.value == 2.5 );
    REQUIRE( len.unit == "cm" ); }
}


TEST_CASE( "struct length converts TeX length to bp and pt", "[input]" )
{
  using namespace Catch::literals;

  { klfengine::length len{2.5, "pt"};
    REQUIRE( len.to_value_as_pt() == 2.5_a );
    REQUIRE( len.to_value_as_bp() == Approx(2.5*72/72.27) ); }

  { klfengine::length len{1.0, "in"};
    REQUIRE( len.to_value_as_pt() == 72.27_a );
    REQUIRE( len.to_value_as_bp() == 72.0_a ); }

  { klfengine::length len{2.54, "cm"}; // == 1in
    REQUIRE( len.to_value_as_pt() == 72.27_a );
    REQUIRE( len.to_value_as_bp() == 72.0_a ); }

  { klfengine::length len{25.4, "mm"}; // == 1in
    REQUIRE( len.to_value_as_pt() == 72.27_a );
    REQUIRE( len.to_value_as_bp() == 72.0_a ); }

  { klfengine::length len{65536.0*72.27, "sp"}; // == 1in
    REQUIRE( len.to_value_as_pt() == 72.27_a );
    REQUIRE( len.to_value_as_bp() == 72.0_a ); }

}


// -----------------


TEST_CASE( "texlength_to_value_as_unit can convert TeX lengths accurately", "[length]" )
{
  using namespace Catch::literals;

  // 1pc == 12pt
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 1.0 , "pc" )
    ==
    12.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*12,1, 'p','c'>( 12.0 , "pt" )
    ==
    1.0_a
  );

  // 1in == 72.27pt
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 1.0 , "in" )
    ==
    72.27_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,100, 'i','n'>( 72.27 , "pt" )
    ==
    1.0_a
  );

  // 72bp == 1in
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,100, 'i','n'>( 72 , "bp" )
    ==
    1.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,100*72, 'b','p'>( 1.0 , "in" )
    ==
    72._a
  );

  // 2.54cm == 1in
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,100, 'i','n'>( 2.54 , "cm" )
    ==
    1.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,254, 'c','m'>( 1.0 , "in" )
    ==
    2.54_a
  );

  // 10mm == 1cm
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,254, 'c','m'>( 10 , "mm" )
    ==
    1._a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*7227,2540, 'm','m'>( 1 , "cm" )
    ==
    10._a
  );

  // 1157 dd == 1238pt
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 1157 , "dd" )
    ==
    1238.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*1238,1157, 'd','d'>( 1238 , "pt" )
    ==
    1157.0_a
  );

  // 1 cc == 12 dd
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*1238,1157, 'd','d'>( 1. , "cc" )
    ==
    12.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L*1238*12,1157, 'c','c'>( 12. , "dd" )
    ==
    1.0_a
  );

  // 65536 sp == 1 pt
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 65536. , "sp" )
    ==
    1.0_a
  );
  REQUIRE(
    klfengine::detail::texlength_to_value_as_unit<1,1, 's','p'>( 1.0 , "pt" )
    ==
    65536.0_a
  );

}


TEST_CASE( "texlength_to_value_as_unit rejects unknown/non-fixed-dimension TeX unit",
           "[length]" )
{
  using Catch::Contains;

  REQUIRE_THROWS_AS(
    (klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 1.4 , "em" )) ,
    klfengine::cannot_convert_length_units
  ) ;
  REQUIRE_THROWS_WITH(
    (klfengine::detail::texlength_to_value_as_unit<65536L,1, 'p','t'>( 1.4 , "zz" )) ,
    Contains("`pt'") && Contains("`zz'")
  ) ;
}
