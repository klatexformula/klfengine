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





TEST_CASE( "parse_environment parses environment variables", "[process]" )
{
  const char * env_vars[] = {
    "VAR1=some value goes here",
    "X=135",
    "AAA_ZZZ_83==13394=",
    NULL
  };

  klfengine::environment e = klfengine::parse_environment(
      const_cast<char**>(env_vars)
      );

  REQUIRE( e.at("VAR1") == "some value goes here" );
  REQUIRE( e.at("X") == "135" );
  REQUIRE( e.at("AAA_ZZZ_83") == "=13394=" );
}


TEST_CASE( "set_environment sets environment accordingly", "[process]" )
{
  klfengine::environment e{
    {"A", "value of A"},
    {"B", "value of B"},
    {"R1", "remove this"},
    {"R2", "remove this (2)"},
    {"C_PATH", std::string{"/path/to/A"} + klfengine::detail::path_separator +
               "C\\path\\to\\B" // w/o colon so it works on linux systems
    }
  };
  klfengine::set_environment(
      e,
      klfengine::provide_environment_variables{{
        {"A", "other value of A"},
        {"Z", "new value of Z"} 
      }},
      klfengine::set_environment_variables{{
        {"B", "set value of B"},
        {"D", "set value of D"}
      }},
      klfengine::remove_environment_variables{{"R1", "R2", "NON_EXISTENT"}},
      klfengine::prepend_path_environment_variables{{
        {"C_PATH", std::string{"/first/path"} + klfengine::detail::path_separator
            + "second/path"},
        {"X_PATH", std::string{"/some/path/X"} + klfengine::detail::path_separator
            + "/another/path/X"},
        {"Y_PATH", std::string{"/some/path"} + klfengine::detail::path_separator
            + "/another/path"},
      }},
      klfengine::append_path_environment_variables{{
        {"C_PATH", "/last/path"}
      }},
      klfengine::set_environment_variables{{ // can override earlier instruction
        {"X_PATH", "/some/X/path"}
      }}
      );
 
  // check the following variables
  // std::cerr << "New environment is:\n";
  // for (const auto & item : e) {
  //   std::cerr << "        " << item.first << ": " << item.second << "\n";
  // }
  // std::cerr << "\n";

  REQUIRE( e.find("A") != e.end() );
  REQUIRE( e["A"] == "value of A" );

  REQUIRE( e.find("B") != e.end() );
  REQUIRE( e["B"] == "set value of B" );

  REQUIRE( e.find("C_PATH") != e.end() );
  REQUIRE( e["C_PATH"] ==
           std::string{"/first/path"} + klfengine::detail::path_separator +
           "second/path" + klfengine::detail::path_separator +
           std::string{"/path/to/A"} + klfengine::detail::path_separator +
           "C\\path\\to\\B" + klfengine::detail::path_separator +
           "/last/path" );

  REQUIRE( e.find("D") != e.end() );
  REQUIRE( e["D"] == "set value of D" );


  REQUIRE( e.find("R1") == e.end() );
  REQUIRE( e.find("R2") == e.end() );

  REQUIRE( e.find("X_PATH") != e.end() );
  REQUIRE( e["X_PATH"] == "/some/X/path" );

  REQUIRE( e.find("Y_PATH") != e.end() );
  REQUIRE( e["Y_PATH"] ==
           std::string{"/some/path"} + klfengine::detail::path_separator
            + "/another/path" );

  REQUIRE( e.find("Z") != e.end() );
  REQUIRE( e["Z"] == "new value of Z" );
}





TEST_CASE( "can run basic process & detect error exit codes", "[process]" )
{
  CHECK_THROWS_AS(
      klfengine::process::run_and_wait(
          {"bash", "-c", "exit 39;"},
          klfengine::process::executable{"/bin/bash"}
          ),
      klfengine::process_exit_error
      );

  CHECK_THROWS_WITH(
      klfengine::process::run_and_wait(
          {"bash", "-c", "exit 39;"},
          klfengine::process::executable{"/bin/bash"}
          ),
      Catch::Contains("39")
      );

  // doesn't throw
  klfengine::process::run_and_wait(
      {"bash", "-c", "exit 0;"},
      klfengine::process::executable{"/bin/bash"}
      );
}



TEST_CASE( "can capture process out/err", "[process]" )
{
  { klfengine::binary_data out;

    klfengine::process::run_and_wait(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stdout_data{&out}
        );

    REQUIRE( out == klfengine::binary_data{'o', 'u', 't', '\n'} );
  }
  { klfengine::binary_data err;

    klfengine::process::run_and_wait(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stderr_data{&err}
        );

    REQUIRE( err == klfengine::binary_data{'e', 'r', 'r', '\n'} );
  }
  { klfengine::binary_data out;
    klfengine::binary_data err;

    klfengine::process::run_and_wait(
        {"bash", "-c", "echo 'out' && echo >&2 'err'"},
        klfengine::process::executable{"/bin/bash"},
        klfengine::process::capture_stdout_data{&out},
        klfengine::process::capture_stderr_data{&err}
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

  klfengine::process::run_and_wait(
      {"bash"},
      klfengine::process::executable{"/bin/bash"},
      klfengine::process::capture_stdout_data{&out},
      klfengine::process::capture_stderr_data{&err},
      klfengine::process::send_stdin_data{stdin_d}
      );

  REQUIRE( out == klfengine::binary_data{'o', 'u', 't', '\n'} );
  REQUIRE( err == klfengine::binary_data{'e', 'r', 'r', '\n'} );
}



TEST_CASE( "can launch process with inherited and modified environment", "[process]" )
{
  klfengine::binary_data out;

  setenv("PARENT_PROCESS_VARIABLE_A", "XXX", 1);

  klfengine::process::run_and_wait(
      {"bash", "-c", "echo \"|$MY_VARIABLE|$PARENT_PROCESS_VARIABLE_A|\""},
      klfengine::process::executable{"/bin/bash"},
      klfengine::process::capture_stdout_data{&out},
      klfengine::set_environment_variables{ {
        {"MY_VARIABLE", "ZZZ"}
      } }
      );

  REQUIRE( out == klfengine::binary_data{'|', 'Z', 'Z', 'Z', '|', 'X', 'X', 'X', '|', '\n'} );
}

TEST_CASE( "can launch process while clearing the parent's environment", "[process]" )
{
  klfengine::binary_data out;

  setenv("PARENT_PROCESS_VARIABLE_B", "here!", 1);

  klfengine::process::run_and_wait(
      {"bash", "-c", "echo \"|$MY_VARIABLE|$PARENT_PROCESS_VARIABLE_B|\""},
      klfengine::process::executable{"/bin/bash"},
      klfengine::process::capture_stdout_data{&out},
      klfengine::process::clear_environment{},
      klfengine::set_environment_variables{ {
        {"MY_VARIABLE", "ZZZ"}
      } }
      );

  REQUIRE( out == klfengine::binary_data{'|', 'Z', 'Z', 'Z', '|', '|', '\n'} );
}
