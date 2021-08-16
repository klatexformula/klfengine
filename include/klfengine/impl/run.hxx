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

#include <stdexcept>
#include <algorithm>

#include <klfengine/run>

namespace klfengine {



_KLFENGINE_INLINE
void run::_ensure_compiled() const
{
  if (!_compiled) {
    throw forgot_to_call_compile();
  }
}



_KLFENGINE_INLINE run::run(
    std::unique_ptr<engine_run_implementation> e
    )
  : _e(std::move(e)),
    _compiled(false)
{
  if (!_e) {
    // don't allow a null pointer
    throw std::invalid_argument("Null pointer passed to klfengine::run() constructor");
  }
}

// _KLFENGINE_INLINE run::~run()
// {
// }



_KLFENGINE_INLINE void run::compile()
{
  if (_compiled) { throw dont_call_compile_twice(); }

  std::lock_guard<std::mutex> lckgrd(_mutex);

  _e->compile();

  _compiled = true;
}

_KLFENGINE_INLINE bool run::compiled() const
{
  return _compiled;
}


_KLFENGINE_INLINE bool run::has_format(const format_spec & format)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  return _e->has_format(format);
}

_KLFENGINE_INLINE bool run::has_format(std::string format)
{
  return has_format(format_spec{std::move(format)});
}

_KLFENGINE_INLINE std::vector<format_description>
run::available_formats()
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  return _e->available_formats();
}

_KLFENGINE_INLINE format_spec
run::canonical_format(const format_spec & format)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);
  return _e->canonical_format(format);
}

_KLFENGINE_INLINE format_spec
run::canonical_format_or_empty(const format_spec & format)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);
  return _e->canonical_format_or_empty(format);
}


_KLFENGINE_INLINE binary_data
run::get_data(const format_spec & format)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  // produces copy via copy constructor
  return binary_data{ _e->get_data_cref(format) };
}

_KLFENGINE_INLINE const binary_data &
run::get_data_cref(const format_spec & format)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  return _e->get_data_cref(format);
}









} // namespace klfengine


