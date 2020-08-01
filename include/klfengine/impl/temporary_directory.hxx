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

#pragma once


#include <random>

#include <klfengine/temporary_directory>



namespace klfengine {



_KLFENGINE_INLINE
temporary_directory::temporary_directory()
  : _path{ create_temporary_dir() },
    _auto_delete{true}
{
  fs::permissions(_path, fs::perms::owner_all);
}

_KLFENGINE_INLINE
temporary_directory::temporary_directory(
    fs::path temp_dir,
    std::string name_prefix,
    int num_rand_chars
    )
  : _path{
      create_temporary_dir(std::move(temp_dir),
                           std::move(name_prefix),
                           num_rand_chars)
    },
    _auto_delete{true}
{
  fs::permissions(_path, fs::perms::owner_all);
}


_KLFENGINE_INLINE
temporary_directory::~temporary_directory()
{
  if (_auto_delete) {
    fs::remove_all(_path) ;
  }
}



// static
_KLFENGINE_INLINE
fs::path temporary_directory::create_temporary_dir(
    fs::path temp_dir, std::string name_prefix, int num_rand_chars, int max_tries
    )
{
  if (temp_dir.empty()) {
    temp_dir = fs::temp_directory_path();
  }

  const char ok_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";
  constexpr std::size_t ok_chars_len = sizeof(ok_chars) - 1; // \0 terminator
  
  std::mt19937 rnd{ std::random_device{}() };
  std::uniform_int_distribution<std::string::size_type> rnddist{0, ok_chars_len-1};

  while (max_tries > 0) {

    std::string sss;
    sss.reserve(num_rand_chars+1);

    for (int i = 0; i < num_rand_chars; ++i) {
      sss += ok_chars[ rnddist(rnd) ];
    }

    fs::path the_path{ temp_dir / (name_prefix + sss) };

    if (fs::create_directory(the_path)) {
      // the directory was successfully created.
      return the_path;
    }
    
    --max_tries;
  }

  throw std::runtime_error(
      "Exhausted max_tries attempting to create temporary directory in " + temp_dir.string()
      );
}




} // namespace klfengine
