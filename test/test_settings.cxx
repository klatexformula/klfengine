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
#include <klfengine/settings>

#include <klfengine/h/detail/filesystem.h>

#include <catch2/catch.hpp>


TEST_CASE( "struct settings has the right fields of the right type", "[settings]" )
{
  klfengine::settings s;
  s.temporary_directory = "/tmp";
  s.texbin_directory = "/usr/local/texlive/20xx/somewhere/bin/";
  s.gs_method = "process";
  s.gs_executable_path = "/usr/local/bin/gs";
  s.subprocess_add_environment = std::map<std::string,std::string>{
    {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
    {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
  };

  REQUIRE( s.temporary_directory == "/tmp" );
  REQUIRE( s.texbin_directory == "/usr/local/texlive/20xx/somewhere/bin/" );
  REQUIRE( s.gs_method == "process" );
  REQUIRE( s.gs_executable_path == "/usr/local/bin/gs" );
  REQUIRE( s.subprocess_add_environment == std::map<std::string,std::string>{
    {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
    {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
  } );

}


TEST_CASE( "struct settings compares for (in)equality", "[settings]" )
{
  klfengine::settings s{
    "/tmp",
    "/usr/local/texlive/20xx/somewhere/bin/",
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
    }
  };

  klfengine::settings s2{
    "/tmp",
    "/usr/local/texlive/20xx/somewhere/bin/",
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
    }
  };

  klfengine::settings t{
    "/tmp",
    "/usr/local/texlive/20xy/somewhere/bin/", // single typo here
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
    }
  };

  klfengine::settings u{
    "/tmp",
    "/usr/local/texlive/20xx/somewhere/bin/",
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/filex"} // single typo here
    }
  };

  REQUIRE( s == s2 );
  REQUIRE( !(s != s2) );

  REQUIRE( s != t );
  REQUIRE( !(s == t) );

  REQUIRE( s != u );
  REQUIRE( !(s == u) );
}



TEST_CASE( "struct settings converts to/from JSON", "[settings]" )
{
  const klfengine::settings s{
    "/tmp",
    "/usr/local/texlive/20xx/somewhere/bin/",
    "process",
    "/usr/local/bin/gs",
    {
     {"TEXINPUTS", "/some/path/for/latex/to/look/for/files"},
     {"BIBINPUTS", "/some/path/for/bibtex/to/look/for/files"}
    }
  };

  nlohmann::json j;
  j = s;

  //std::cerr << "DEBUG: " << j.dump(4) << "\n";

  klfengine::settings s2;
  j.get_to(s2);

  REQUIRE( s == s2 );
}




// TEST_CASE( "can detect a temporary directory", "[settings]" )
// {
//   // found a dir & didn't throw an error
//   std::string tmpdir = klfengine::settings::detect_temporary_directory();
//   std::cout << "[test] Detected temporary dir = " << tmpdir << "\n";
//   REQUIRE( tmpdir != std::string() ) ;
// }

// TEST_CASE( "can find latex directory on this system", "[settings]" )
// {
//   settings s = klfengine::settings::detect_settings();
//   std::cout << "[test] detected texbin directory " << s.texbin_directory << "\n";
//   REQUIRE( s.texbin_directory.size() );
//   REQUIRE( s.texbin_directory.rfind("latex") != std::string::npos );
// }

// TEST_CASE( "can find latex directory in test directories, choosing latest version",
//            "[settings]" )
// {
//   settings s = klfengine::settings::detect_settings();
//   const auto & result = s.texbin_directory;
//   std::cout << "[test] detected texbin directory " << result << "\n";
//   REQUIRE( result.size() );
//   REQUIRE( result.rfind("latex") != std::string::npos );
// }

// TEST_CASE( "can find gs", "[settings]" )
// {
//   settings s = klfengine::settings::detect_settings();
//   const auto & result = s.gs_executable_path;
//   std::cout << "[test] detected gs exec = " << result << "\n";
//   REQUIRE( result.size() );
//   REQUIRE( result.rfind("gs") != std::string::npos );
// }


TEST_CASE( "settings::detect_settings() does its job", "[settings]" )
{
  klfengine::settings s = klfengine::settings::detect_settings(
      { KLFENGINE_TEST_ROOT_SRC_DIR "/some_path/*/bin",
        KLFENGINE_TEST_ROOT_SRC_DIR "/some_path/dummy_texlive/<texlive-year>/bin" }
      );

  using json = nlohmann::json;
  INFO( "detected settings = " << json{s}.dump(4) );

  REQUIRE( klfengine::fs::canonical(s.texbin_directory)
           == klfengine::fs::canonical(KLFENGINE_TEST_ROOT_SRC_DIR
                                       "/some_path/dummy_texlive/2030/bin") );

  REQUIRE( klfengine::fs::canonical(s.gs_executable_path)
           == klfengine::fs::canonical(KLFENGINE_TEST_ROOT_SRC_DIR
                                       "/some_path/dummy_gs/bin/gs") );
}
