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

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <klfengine/input.h>


#include <catch2/catch.hpp>


TEST_CASE( "variant_type can store an int or a string", "[variants]" )
{
  REQUIRE( std::get<int>(klfengine::variant_type<int, std::string>{3}) == 3 );
  REQUIRE(
      std::get<std::string>(klfengine::variant_type<int, std::string>{"hello"})
      == std::string("hello")
      );
}


TEST_CASE( "value can store different data types recursively", "[variants]" )
{
  REQUIRE( klfengine::value{nullptr}.get<std::nullptr_t>() == nullptr );
  REQUIRE( klfengine::value{true}.get<bool>() == true );
  REQUIRE( klfengine::value{3}.get<int>() == 3 );
  REQUIRE( klfengine::value{42.5}.get<double>() == 42.5 );
  REQUIRE( klfengine::value{std::string("yo")}.get<std::string>() == std::string("yo") );

  // const char * gets converted to bool (no, really, gotta be kidding me C++)
  //klfengine::value x{"one"};
  //std::cerr << "x.data-index() == " << x._data.index() << "\n"; // !!!!

  using namespace std::literals; // "xxx"s -> std::string  (C++ >= 14)

  klfengine::value d{
        klfengine::value::array{
          klfengine::value{"one"s},
          klfengine::value{"two"s},
          klfengine::value{klfengine::value::array{klfengine::value{3}, klfengine::value{4},
                                                   klfengine::value{5}}},
          klfengine::value{klfengine::value::dict{{"key1", klfengine::value{"value1"}},
                                                  {"key2", klfengine::value{222}}}}
        }
      };
  REQUIRE(
      d.get<klfengine::value::array>()[0].get<std::string>() == std::string("one")
      );
  REQUIRE(
      d.get<klfengine::value::array>()[2].get<klfengine::value::array>()[1].get<int>()
      == 4
      );
  REQUIRE(
      d.get<klfengine::value::array>()[3].get<klfengine::value::dict>()["key2"].get<int>()
      == 222
      );
}
