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

#include <klfengine/basedefs>

#include <algorithm>

#include <catch2/catch.hpp>


struct MyTestType { };



TEST_CASE( "binary_data can store binary data", "[basedefs]" )
{
  const char * data = "Hell\0o";
  const std::size_t len = 6;

  klfengine::binary_data b(len);
  std::copy(data, data+len, b.begin());

  std::string s(b.begin(), b.end());
  REQUIRE( s == std::string("Hell\0o", len) ) ;
}


TEST_CASE( "klfe::exception can throw custom message", "[basedefs]" )
{
  CHECK_THROWS_AS(
      throw klfengine::exception("hello" + std::string(" world")),
      klfengine::exception
      );
  CHECK_THROWS_WITH(
      throw klfengine::exception("hello-world"),
      "hello-world"
      );
}


TEST_CASE( "klfe::detail::get_type_name", "[basedefs]" )
{
  REQUIRE( klfengine::detail::get_type_name<MyTestType>()
           == "MyTestType" ) ;
  REQUIRE( klfengine::detail::get_type_name<const int &>()
           == "int const&" ) ;
}



TEST_CASE( "klfe::detail::str_split_rx", "[basedefs]" )
{
  std::string x{"abcd;efghijkl;;m;nn;op;"};
  REQUIRE(
      klfengine::detail::str_split_rx(x.begin(), x.end(), std::regex{"\\;+"})
      == std::vector<std::string>{ "abcd", "efghijkl", "m", "nn", "op", "" }
    );
}
