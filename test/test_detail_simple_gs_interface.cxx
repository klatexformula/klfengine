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
  return klfengine::settings::detect_settings().gs_executable_path;
}




inline void do_test_check_gs_version(klfengine::detail::simple_gs_interface & gs)
{
  auto ver = gs.get_gs_version();

  std::cout << "[test] You are running ghostscript version "
            << ver.major << "." << ver.minor << "\n";

  CAPTURE( ver.major );
  CAPTURE( ver.minor );

  REQUIRE( ver.major >= 8 );
  REQUIRE( ver.minor >= 0 );
}

TEST_CASE( "check gs version via Process", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  do_test_check_gs_version(gs);
}

TEST_CASE( "check gs version via LinkedLibgs", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LinkedLibgs,
    get_gs_path()
  };

  do_test_check_gs_version(gs);
}

TEST_CASE( "check gs version via LoadLibgs", "[detail-simple_gs_interface][!mayfail]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LoadLibgs,
    get_gs_path()
  };

  do_test_check_gs_version(gs);
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




inline void do_check_gs_information(klfengine::detail::simple_gs_interface & gs)
{
  auto info = gs.get_gs_info();

  std::cout << "[test] Ghostscript info:\n"
            << "Head: " << info.head << "\n"
            << "Possible output devices are: " << info.devices << "\n"
            << "Search paths are: " << info.search_path << "\n";

  REQUIRE( info.devices.size() > 0 );
  REQUIRE( std::find(info.devices.begin(), info.devices.end(), std::string{"pdfwrite"})
           != info.devices.end() );
}

TEST_CASE( "check gs information via Process", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  do_check_gs_information(gs);
}

TEST_CASE( "check gs information via LinkedLibgs", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LinkedLibgs,
    get_gs_path()
  };

  do_check_gs_information(gs);
}

TEST_CASE( "check gs information via LoadLibgs", "[detail-simple_gs_interface][!mayfail]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LoadLibgs,
    get_gs_path()
  };

  do_check_gs_information(gs);
}


inline void do_handle_gs_errors(klfengine::detail::simple_gs_interface & gs)
{
  klfengine::binary_data stderr_data;
  klfengine::binary_data output_data;

  CHECK_THROWS_AS(
      gs.run_gs(
          {"-sDEVICE=pdfwrite","-dBATCH", "-q", "-dNOPAUSE"}, // missing -sOutputFile=
          klfengine::detail::simple_gs_interface::send_stdin_data{klfengine::binary_data{}},
          klfengine::detail::simple_gs_interface::add_standard_batch_flags{false},
          klfengine::detail::simple_gs_interface::capture_stderr_data{&stderr_data},
          klfengine::detail::simple_gs_interface::capture_stdout_data{&output_data}
          ),
      klfengine::detail::ghostscript_error
      );

  std::string stderr_s{stderr_data.begin(), stderr_data.end()};

  // and reports an error on stderr
  REQUIRE( stderr_s.find("Device 'pdfwrite' requires an output file")
           != std::string::npos ) ;
}

TEST_CASE( "handle gs errors via Process", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  do_handle_gs_errors( gs );
}

TEST_CASE( "handle gs errors via LinkedLibgs", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LinkedLibgs,
    get_gs_path()
  };

  do_handle_gs_errors( gs );
}

TEST_CASE( "handle gs errors via LoadLibgs", "[detail-simple_gs_interface][!mayfail]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LoadLibgs,
    get_gs_path()
  };

  do_handle_gs_errors( gs );
}


inline void do_can_run_gs(klfengine::detail::simple_gs_interface & gs)
{
  std::string ps_code{
    "%!PS\n"
    "<< /PageSize [36 36] >> setpagedevice 0.4 setlinewidth 2 2 newpath moveto 5 5 lineto 10 0 lineto 10 10 lineto closepath 0.4 0 0 setrgbcolor stroke showpage"
  };

  klfengine::binary_data stdout_data;
  klfengine::binary_data stderr_data;

  gs.run_gs(
      {"-sDEVICE=pdfwrite","-dBATCH", "-q", "-dNOPAUSE", "-sOutputFile=-", "-"},
      // send some dummy PS input to stdin
      klfengine::detail::simple_gs_interface::send_stdin_data{
        klfengine::binary_data{ps_code.begin(), ps_code.end()}
      },
      // already have all necessary flags above
      klfengine::detail::simple_gs_interface::add_standard_batch_flags{false},
      // output & error streams
      klfengine::detail::simple_gs_interface::capture_stdout_data{&stdout_data},
      klfengine::detail::simple_gs_interface::capture_stderr_data{&stderr_data}
      );

  std::string stderr_s{stderr_data.begin(), stderr_data.end()};
  std::string output_s{stdout_data.begin(), stdout_data.end()};

  CAPTURE( stderr_s );
  CAPTURE( output_s );

  REQUIRE( stderr_data == klfengine::binary_data{} ) ;

  REQUIRE( output_s.rfind("%PDF-",0) == 0 ) ;
}

TEST_CASE( "can run gs via Process", "[detail-simple_gs_interface]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::Process,
    get_gs_path()
  };

  do_can_run_gs( gs );
}

// *** It looks like device output is *always* sent to stdout regardless of
//     callbacks!  We can't use libgs if we want gs to write to stdout!
//
// TEST_CASE( "can run gs via LinkedLibgs", "[detail-simple_gs_interface]" )
// {
//   klfengine::detail::simple_gs_interface gs{
//     klfengine::detail::simple_gs_interface::method::LinkedLibgs,
//     get_gs_path()
//   };
//   do_can_run_gs( gs );
// }

TEST_CASE( "can run gs via LoadLibgs", "[detail-simple_gs_interface][!mayfail]" )
{
  klfengine::detail::simple_gs_interface gs{
    klfengine::detail::simple_gs_interface::method::LoadLibgs,
    get_gs_path()
  };

  do_can_run_gs( gs );
}
