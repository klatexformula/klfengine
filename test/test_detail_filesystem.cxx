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
#include <klfengine/h/detail/filesystem.h>

#include <iostream>

#include <catch2/catch.hpp>


TEST_CASE( "fs exposes a std::filesystem interface", "[detail-filesystem]" )
{
  klfengine::fs::path p = klfengine::fs::current_path();
  REQUIRE( klfengine::fs::exists( p ) );
}


// forward declaration of non-public functions
namespace klfengine{ namespace detail {
fs_w_part compile_wildcard(const std::string & s);
std::string compile_wildcard_rx_pattern(const std::string & s);
} }


TEST_CASE( "compile_wildcard compiles fixed or wildcard patterns to fs_w_part",
           "[detail-filesystem]" )
{
  REQUIRE(
      _KLFENGINE_VARIANT_GET<std::string>(
          klfengine::detail::compile_wildcard("ABCd-fe.txt")
          )
      == std::string{"ABCd-fe.txt"} );
  REQUIRE(
      _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<std::regex>(
          klfengine::detail::compile_wildcard("test-?-[debug]*")
          )
      );
  REQUIRE(
      klfengine::detail::compile_wildcard_rx_pattern("test-?-[debug]*")
      == std::string{"^test\\-.\\-\\[debug\\].*$"}
      );
}


TEST_CASE( "find_wildcard_path works for relative paths", "[detail-filesystem]" )
{
  std::vector<klfengine::fs::path> results =
    klfengine::detail::find_wildcard_path({
        "test/test_detail_*",
        "test_detail_*",
      });
  std::cout << "[test] find_wildcard_path results =";
  for (auto & p: results) {
    std::cout << " " << p;
  }
  std::cout << ".\n";

  // we should find our own executable test_detail_filesystem(.exe)?
  auto it = std::find_if(
      results.begin(), results.end(),
      [](const klfengine::fs::path & p) {
        return p.filename().string().rfind("test_detail_filesystem",0)==0;
      });
  REQUIRE( it != results.end() );
  REQUIRE( klfengine::fs::exists(*it) );
}




TEST_CASE( "find_wildcard_path works for absolute paths", "[detail-filesystem]" )
{
  klfengine::fs::path cwd = klfengine::fs::current_path();
  std::vector<klfengine::fs::path> results =
    klfengine::detail::find_wildcard_path({
        cwd.string() + "/test/test_detail_*",
        cwd.string() + "/test_detail_*",
      });
  std::cout << "[test] cwd = " << cwd << "; find_wildcard_path results =";
  for (auto & p: results) {
    std::cout << " " << p;
  }
  std::cout << "\n";

  // we should find our own executable test_detail_filesystem(.exe)?
  auto it = std::find_if(
      results.begin(), results.end(),
      [](const klfengine::fs::path & p) {
        return p.filename().string().rfind("test_detail_filesystem",0)==0;
      });
  REQUIRE( it != results.end() );
  REQUIRE( klfengine::fs::exists(*it) );
}



TEST_CASE( "get_environment_PATH() returns a list with /bin in it",
           "[detail-filesystem]" )
{
  auto v = klfengine::detail::get_environment_PATH();
  REQUIRE( std::find(v.begin(), v.end(), "/bin") != v.end() );
}
