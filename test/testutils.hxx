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

#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <regex>

#include <catch2/catch.hpp>

#include <klfengine/process>
#include <klfengine/h/detail/filesystem.h>
#include <klfengine/h/detail/simple_gs_interface.h>



#define KLFENGINE_TEST_DATA_DIR KLFENGINE_TEST_ROOT_SRC_DIR "/test_data/"


inline std::string find_test_helper_tool(std::string toolname)
{
  namespace klfe = klfengine;

  std::string varname{toolname};
  std::transform(varname.begin(), varname.end(), varname.begin(), ::toupper);

  const char * var_value_ptr = std::getenv(varname.c_str());
  if (var_value_ptr != nullptr) {
    return std::string{var_value_ptr};
  }

  // search in some standard dirs (/usr/bin, /usr/local/bin)
  std::vector<std::string> search_path{
    "/usr/bin",
    "/usr/local/bin"
  };
  for (auto pth : search_path) {
    klfe::fs::path candidate = klfe::fs::path{pth} / toolname;
    if (klfe::fs::exists(candidate)) {
      return candidate.native();
    }
  }

  throw std::runtime_error("Can't find tool " + toolname + ".  Try setting $" + varname
                           + " to the full path to the executable");
}



inline void require_images_similar(std::string fnimg1, std::string fnimg2)
                                   //,double rmse_normalized_threshold=0.04)
{
  namespace klfe = klfengine;

  std::string magick = find_test_helper_tool("magick");
  
  std::string devnull = "/dev/null";

  klfe::binary_data magick_err;

  int magick_compare_exit_code;

  klfe::process::run_and_wait({
      magick,
      "compare",
      "-define", "profile:skip=ICC",
      "-metric", "ncc",
      //"-fuzz", "5%",
      fnimg1,
      fnimg2,
      devnull
    },
    klfengine::process::capture_stderr_data{&magick_err},
    klfengine::process::capture_exit_code{magick_compare_exit_code},
    klfengine::process::check_exit_code{false}
    );
      
  std::string magick_s_err{magick_err.begin(), magick_err.end()};

  CAPTURE( magick_s_err );
  CAPTURE( magick_compare_exit_code );

  // std::regex rx_magick_mae{"^\\s*([0-9.e+-]+)\\s*\\(([0-9.e+-]+)\\)(?:\\s|\n)*$"};
  // std::smatch magick_mae_match;
  // bool ok = std::regex_search(magick_s_err, magick_mae_match, rx_magick_mae);
  // if (!ok) {
  //   throw std::runtime_error("Couldn't parse magick compare information: " + magick_s_err);
  // }
  // const double mae_value = std::stod(magick_mae_match[1].str());
  // //const double mae_norm_value = std::stod(magick_mae_match[2].str());
  // const double mae_threshold = 20; // # of pixels with values beyond fuzz threshold
  // REQUIRE( mae_value <= mae_threshold ) ;

  std::regex rx_magick_ncc{"^\\s*([0-9.e+-]+)(?:\\s|\n)*$"};
  std::smatch magick_ncc_match;
  bool ok = std::regex_search(magick_s_err, magick_ncc_match, rx_magick_ncc);
  if (!ok) {
    throw std::runtime_error("Couldn't parse magick compare information: " + magick_s_err);
  }
  const double ncc_value = std::stod(magick_ncc_match[1].str());
  const double ncc_threshold = 0.9997;
  REQUIRE( ncc_value >= ncc_threshold ) ;


  // std::regex rx_magick_pae{"^\\s*([0-9.e+-]+)\\s*\\(([0-9.e+-]+)\\)(?:\\s|\n)*$"};
  // std::smatch magick_pae_match;
  // bool ok = std::regex_search(magick_s_err, magick_pae_match, rx_magick_pae);
  // if (!ok) {
  //   throw std::runtime_error("Couldn't parse magick compare information: " + magick_s_err);
  // }
  // //const double pae_value = std::stod(magick_pae_match[1].str());
  // const double pae_norm_value = std::stod(magick_pae_match[2].str());
  // const double pae_normalized_threshold = 0.04;
  // REQUIRE( pae_norm_value <= pae_normalized_threshold ) ;
}


inline void pdfdata_to_pngfile(
  const klfengine::binary_data & pdfdata,
  std::string fnamepng,
  bool transparent,
  int dpi
)
{
  klfengine::binary_data gs_pdftopng_stderr;
  klfengine::binary_data gs_pdftopng_stdout;
  std::string gs_exec = find_test_helper_tool("gs");
  CAPTURE( gs_exec );

  std::string device = (transparent ? "pngalpha" : "png16m");

  std::vector<std::string> gs_pdftopng_args{
    "-sDEVICE=" + device,
    "-r" + std::to_string(dpi),
    "-dGraphicsAlphaBits=4", "-dTextAlphaBits=4",
    "-sOutputFile=" + fnamepng,
    "-" // take input from stdin
  };

  CAPTURE( gs_pdftopng_args );

  klfengine::detail::simple_gs_interface gsiface("process", gs_exec);
  gsiface.run_gs(
    gs_pdftopng_args,
    klfengine::detail::simple_gs_interface::send_stdin_data{pdfdata},
    klfengine::detail::simple_gs_interface::add_standard_batch_flags{true},
    klfengine::detail::simple_gs_interface::capture_stdout_data{&gs_pdftopng_stdout},
    klfengine::detail::simple_gs_interface::capture_stderr_data{&gs_pdftopng_stderr}
  );

  std::string gs_pdftopng_stdout_s{gs_pdftopng_stdout.begin(), gs_pdftopng_stdout.end()};
  std::string gs_pdftopng_stderr_s{gs_pdftopng_stderr.begin(), gs_pdftopng_stderr.end()};

  CAPTURE(gs_pdftopng_stdout_s) ;
  CAPTURE(gs_pdftopng_stderr_s) ;
}
