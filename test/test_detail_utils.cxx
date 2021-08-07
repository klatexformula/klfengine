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
#include <klfengine/h/detail/utils.h>


#include <catch2/catch.hpp>



struct string_arg_test {
  std::string _data;
};

struct another_arg_test {
  std::string & _data_ref;
};

// test our tools for parsing the arguments to klfengine::process::run_and_wait()
template<typename... Args>
static std::string test_run_args(Args && ... args)
{
  using namespace klfengine;
  using namespace klfengine::detail;
  using namespace klfengine::detail::utils;

  if (kwargs<Args...>::template has_arg<string_arg_test>::value) {
    string_arg_test d{
      kwargs<Args...>::template take_arg<string_arg_test>(args...)
    };
    return "yes! string_arg_test._data is: " + d._data;
  }
  return "no.";
}

TEST_CASE( "argument handling with detail::get_kwargs", "[process]" )
{
  { auto res = test_run_args(string_arg_test{"hello world"});
    REQUIRE( res == "yes! string_arg_test._data is: hello world" ) ; }
  { auto res = test_run_args(string_arg_test{"hello world"});
    REQUIRE( res == "yes! string_arg_test._data is: hello world" ) ; }
  { auto res = test_run_args();
    REQUIRE( res == "no." ) ; }
  { std::string x;
    auto res = test_run_args(another_arg_test{x});
    REQUIRE( res == "no." ) ; }
  { std::string x;
    auto res = test_run_args(another_arg_test{x},
                             string_arg_test{"hello world"});
    REQUIRE( res == "yes! string_arg_test._data is: hello world" ) ; }
  { std::string x;
    auto res = test_run_args(string_arg_test{"hello world"},
                             another_arg_test{x});
    REQUIRE( res == "yes! string_arg_test._data is: hello world" ) ; }
}


