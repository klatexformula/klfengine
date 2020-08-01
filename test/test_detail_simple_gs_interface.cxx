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
#include <klfengine/h/detail/simple_gs_interface.h>

#include <klfengine/process>

#include <iostream>

#include <catch2/catch.hpp>


// -----------------------------------------------------------------------------

// some tests for subprocess.hpp --

// #include <subprocess/subprocess.hpp>

// TEST_CASE( "subprocess can handle stdin, stdout, & stderr, and detect process failure",
//            "[subprocess]" )
// {
//   using namespace subprocess;
//   Popen p{
//     std::vector<std::string>{"/bin/bash"},
//     input{PIPE},
//     output{PIPE},
//     error{PIPE}
//   };
//   std::string stdin_data{ "echo 'out' && echo >&2 'err' && exit 39;\n" };
//   auto res = p.communicate(stdin_data.c_str(), stdin_data.size());
//   auto retcode = p.poll();

//   REQUIRE( retcode == 39 );
//   REQUIRE( std::string{res.first.buf.begin(), res.first.buf.end()} == "out\n" );
//   REQUIRE( std::string{res.second.buf.begin(), res.second.buf.end()} == "err\n" );
// }


// -----------------------------------------------------------------------------


inline std::string get_gs_path()
{
  return "/usr/local/bin/gs";
}
TEST_CASE( "gs path not hard-coded", "[tests]" )
{
  REQUIRE( (false && "hard-coded gs path should not be used in test") );
}


TEST_CASE( "check gs version", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };
  auto ver = gs.gs_version();

  std::cout << "You are running ghostscript version "
            << ver.first << "." << ver.second << "\n";

  REQUIRE( ver.first >= 8 );
  REQUIRE( ver.second >= 0 );
}

template<typename T>
inline std::ostream & operator<<(std::ostream& s, const std::vector<T> & v)
{
  for (std::size_t i = 0; i < v.size(); ++i) {
    s << v[i];
    if (i < v.size()-1) {
      s << ", ";
    }
  }
  return s;
}


TEST_CASE( "check gs information", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };
  auto info = gs.gs_info();

  std::cout << "Ghostscript info:\n" << info.head << "\n"
            << "Possible output devices are: " << info.devices << "\n"
            << "Search paths are: " << info.search_path << "\n";

  REQUIRE( info.devices.size() > 0 );
  REQUIRE( std::find(info.devices.begin(), info.devices.end(), std::string{"pdfwrite"})
           != info.devices.end() );
}

TEST_CASE( "handle gs errors", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  std::string stderr_s;

  klfengine::binary_data output;

  CHECK_THROWS_AS(
      output = gs.run_gs(
          {"-sDEVICE=pdfwrite","-dBATCH", "-q", "-dNOPAUSE"}, // missing -sOutputFile=
          klfengine::binary_data{}, // gs stdin
          false, // already have all necessary flags above
          //
          &stderr_s
          ),
      klfengine::process_exit_error
      );
}



TEST_CASE( "can run gs", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  std::string stderr_s;

  std::string ps_code{
    "%!PS\n"
    "<< /PageSize [36 36] >> setpagedevice 0.4 setlinewidth 2 2 newpath moveto 5 5 lineto 10 0 lineto 10 10 lineto closepath 0.4 0 0 setrgbcolor stroke showpage"
  };

  auto output = gs.run_gs(
      {"-sDEVICE=pdfwrite","-dBATCH", "-q", "-dNOPAUSE", "-sOutputFile=-"},
      // some dummy PS input
      klfengine::binary_data{ps_code.begin(), ps_code.end()},
      false, // already have all necessary flags above
      //
      &stderr_s
      );

  REQUIRE( stderr_s == std::string() ) ;

  REQUIRE( std::string{output.begin(),output.end()}.rfind("%PDF-",0) == 0 ) ;
}

