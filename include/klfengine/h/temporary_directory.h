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

#include <klfengine/basedefs>

#include <klfengine/h/detail/filesystem.h>

namespace klfengine {


namespace detail {

constexpr int temp_dir_default_max_tries = 100;
constexpr int temp_dir_default_num_rand_chars = 16;

} // namespace detail


class temporary_directory {
public:
  temporary_directory();
  explicit temporary_directory(
      fs::path temp_dir,
      std::string name_prefix = std::string(),
      int num_rand_chars = detail::temp_dir_default_num_rand_chars
      );
  ~temporary_directory();

  inline fs::path path() const { return _path; }

  inline bool auto_delete() { return _auto_delete; }
  inline void set_auto_delete(bool auto_delete) { _auto_delete = auto_delete; }

  // default move semantics
  temporary_directory(temporary_directory &&) = default;
  temporary_directory & operator=(temporary_directory &&) = default;

  // no copy semantics
  temporary_directory(const temporary_directory &) = delete;
  temporary_directory & operator=(const temporary_directory &) = delete;

private:
  static fs::path create_temporary_dir(
    fs::path temp_dir = fs::path(), // defaults to std::filesystem::temp_directory_path()
    std::string name_prefix = std::string(),
    int num_rand_chars = detail::temp_dir_default_num_rand_chars,
    int max_tries = detail::temp_dir_default_max_tries
  );

  fs::path _path;
  bool _auto_delete;
};




} // namespace klfengine




#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/temporary_directory.hxx>
#endif
