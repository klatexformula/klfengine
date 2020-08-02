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
#include <klfengine/process>

#include <iostream>

#include <catch2/catch.hpp>


struct string_arg_test {
  std::string _data;
};

struct another_arg_test {
  std::string & _data_ref;
};

// test our tools for parsing the arguments to klfengine::process::run()
template<typename... Args>
static std::string test_run_args(Args && ... args)
{
  using namespace klfengine;
  using namespace klfengine::detail;

  if (get_kwargs<Args...>::template has_arg<string_arg_test>::value) {
    string_arg_test d{
      get_kwargs<Args...>::template get_arg<string_arg_test>(args...)
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




TEST_CASE( "can run basic process & detect error exit codes", "[process]" )
{
  CHECK_THROWS_AS(
      klfengine::process::run(
          {"bash", "-c", "exit 39;"},
          klfengine::process::executable{"/bin/bash"}
          ),
      klfengine::process_exit_error
      );

  CHECK_THROWS_WITH(
      klfengine::process::run(
          {"bash", "-c", "exit 39;"},
          klfengine::process::executable{"/bin/bash"}
          ),
      Catch::Contains("39")
      );

  // doesn't throw
  klfengine::process::run(
      {"bash", "-c", "exit 0;"},
      klfengine::process::executable{"/bin/bash"}
      );
}



TEST_CASE( "can capture process out/err", "[process]" )
{
  { klfengine::binary_data out;

    klfengine::process::run(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stdout_data{out}
        );

    REQUIRE( out == klfengine::binary_data{'o', 'u', 't', '\n'} );
  }
  { klfengine::binary_data err;

    klfengine::process::run(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stderr_data{err}
        );

    REQUIRE( err == klfengine::binary_data{'e', 'r', 'r', '\n'} );
  }
  { klfengine::binary_data out;
    klfengine::binary_data err;

    klfengine::process::run(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stdout_data{out},
        klfengine::process::capture_stderr_data{err}
        );

    REQUIRE( out == klfengine::binary_data{'o', 'u', 't', '\n'} );
    REQUIRE( err == klfengine::binary_data{'e', 'r', 'r', '\n'} );
  }
}


TEST_CASE( "can send process stdin", "[process]" )
{
  klfengine::binary_data out;
  klfengine::binary_data err;

  const std::string in = "echo 'out' && echo >&2 'err'";
  const klfengine::binary_data stdin_d{in.begin(), in.end()};

  klfengine::process::run(
      {"bash"},
      klfengine::process::executable{"/bin/bash"},
      klfengine::process::capture_stdout_data{out},
      klfengine::process::capture_stderr_data{err},
      klfengine::process::send_stdin_data{stdin_d}
      );

  REQUIRE( out == klfengine::binary_data{'o', 'u', 't', '\n'} );
  REQUIRE( err == klfengine::binary_data{'e', 'r', 'r', '\n'} );
}


