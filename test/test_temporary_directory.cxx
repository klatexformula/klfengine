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
#include <klfengine/temporary_directory>
#include <klfengine/settings>

#include <iostream>

#include <catch2/catch.hpp>



TEST_CASE( "can create a temporary directory & it gets deleted upon destruction",
           "[temporary_directory]" )
{
  klfengine::fs::path the_path;
  { klfengine::temporary_directory td{};
    the_path = td.path();
    std::cout << "Created temporary directory, path = " << the_path << "\n";
    REQUIRE( klfengine::fs::exists(the_path) ) ;
  } // destruction here
  REQUIRE( ! klfengine::fs::exists(the_path) ) ;
}


TEST_CASE( "temp dir gets deleted exactly when auto-delete is set on",
           "[temporary_directory]" )
{
  klfengine::fs::path the_path;

  { klfengine::temporary_directory td{};
    td.set_auto_delete(true);
    the_path = td.path();
    std::cout << "Created temporary directory, path = " << the_path << "\n";
    REQUIRE( klfengine::fs::exists(the_path) ) ;
  } // destruction here
  REQUIRE( ! klfengine::fs::exists(the_path) ) ;

  { klfengine::temporary_directory td{};
    td.set_auto_delete(false);
    the_path = td.path();
    std::cout << "Created temporary directory, path = " << the_path << "\n";
    REQUIRE( klfengine::fs::exists(the_path) ) ;
  } // destructor here -- dir should not be removed
  REQUIRE( klfengine::fs::exists(the_path) ) ; // still exists after destruction

  // now actually delete the path so we don't pollute our disk
  klfengine::fs::remove_all(the_path) ;
  REQUIRE( ! klfengine::fs::exists(the_path) ) ;
}


TEST_CASE( "temp dir uses correct base directory and template",
           "[temporary_directory]" )
{
  const std::string temp_dir_sub_path{ "test-AAA" };
  const klfengine::fs::path our_temp_dir_path =
    klfengine::fs::temp_directory_path() / temp_dir_sub_path;

  klfengine::fs::create_directories(our_temp_dir_path);

  REQUIRE( klfengine::fs::exists(our_temp_dir_path) ) ; // sanity check

  { 
    const std::string prefix{ "pr3f1x_" };
    klfengine::temporary_directory td{our_temp_dir_path, prefix, 20};
    klfengine::fs::path p{ td.path() };

    std::cout << "Created temporary directory, path = " << p << "\n";

    // ensure we used the right path
    REQUIRE( p.parent_path().filename() == temp_dir_sub_path ) ;

    std::string fn{ p.filename() };

    // no std::string::startswith(), see https://stackoverflow.com/a/40441240/1694896
    REQUIRE( fn.rfind(prefix,0) == 0 );

    REQUIRE( fn.size() == prefix.size() + 20 );
  }

  // remove our testing junk directories
  klfengine::fs::remove_all( our_temp_dir_path );
}
