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


#include <vector>
#include <string>
#include <functional>

#include <klfengine/basedefs>
#include <klfengine/value>

#include <klfengine/h/detail/provide_fs.h>


namespace klfengine {

namespace detail {


#ifdef _KLFENGINE_OS_WIN
constexpr char path_separator = ';';
#else
constexpr char path_separator = ':';
#endif



std::vector<fs::path>
find_wildcard_path(const std::vector<std::string> & wildcard_expressions,
                   const std::vector<std::string> & file_names
                     = std::vector<std::string>{},
                   int limit = -1);


std::vector<fs::path>
find_wildcard_path(const std::vector<std::string> & wildcard_expressions,
                   const std::vector<std::string> & file_names,
                   const std::function<bool(const fs::path&)> & predicate,
                   int limit = -1);


std::vector<std::string> get_environment_PATH(const char * varname = "PATH");


} // namespace detail

} // namespace klfengine



#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/detail/filesystem.hxx>
#endif
