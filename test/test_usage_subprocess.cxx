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


// this is not testing part of klfengine, but checking how we should use
// https://github.com/arun11299/cpp-subprocess


#include <subprocess/subprocess.hpp>

#include <klfengine/settings>


#include <catch2/catch.hpp>




TEST_CASE( "usage test of subprocess: can launch ghostscript and get result", "[usage-test]" )
{
  klfengine::settings settings = klfengine::settings::detect_settings();

  std::string ps_code{
    "%!PS\n"
    "<< /PageSize [36 36] >> setpagedevice 0.4 setlinewidth 2 2 newpath "
    "moveto 5 5 lineto 10 0 lineto 10 10 lineto closepath "
    "0.4 0 0 setrgbcolor stroke showpage\n"
  };

  std::vector<std::string> argv{
    settings.gs_executable_path,
    "-sDEVICE=pdfwrite",
    "-dBATCH",
    "-q",
    "-dNOPAUSE",
    "-sOutputFile=-"
  };

  auto p = subprocess::Popen(
      argv,
      subprocess::input{subprocess::PIPE},
      subprocess::output{subprocess::PIPE},
      subprocess::error{subprocess::PIPE}
      );
  auto pbufs = p.communicate(ps_code.c_str(), ps_code.size());

  auto retcode = p.poll();
  REQUIRE(retcode == 0);

  const auto & out_buf = pbufs.first;
  const auto & err_buf = pbufs.second;

  klfengine::binary_data out_data{out_buf.buf.data(), out_buf.buf.data()+out_buf.length};
  klfengine::binary_data err_data{err_buf.buf.data(), err_buf.buf.data()+err_buf.length};

  REQUIRE( err_data == klfengine::binary_data{} ) ;

  REQUIRE( std::string{out_data.begin(),out_data.end()}.rfind("%PDF-",0) == 0 ) ;
}


TEST_CASE( "usage test of subprocess: can get output in case of gs errors", "[usage-test]" )
{
  klfengine::settings settings = klfengine::settings::detect_settings();

  std::vector<std::string> argv{
    settings.gs_executable_path,
    "-sDEVICE=pdfwrite",
    "-dBATCH",
    //"-q",
    "-dNOPAUSE"
    //"-sOutputFile=-" // missing -sOutputFile=...
  };

  auto p = subprocess::Popen(
      argv,
      subprocess::input{subprocess::PIPE},
      subprocess::output{subprocess::PIPE},
      subprocess::error{subprocess::PIPE}
      );
  auto pbufs = p.communicate();

  auto retcode = p.retcode();
  REQUIRE( retcode != 0 );

  const auto & out_buf = pbufs.first;
  const auto & err_buf = pbufs.second;

  std::string out_str{out_buf.buf.data(), out_buf.buf.data()+out_buf.length};
  std::string err_str{err_buf.buf.data(), err_buf.buf.data()+err_buf.length};

  REQUIRE( err_str.find("Device 'pdfwrite' requires an output file")
           != std::string::npos ) ;
}
