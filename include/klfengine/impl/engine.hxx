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

#include <klfengine/engine>
#include <klfengine/engine_run_implementation>
#include <klfengine/run>


namespace klfengine {



_KLFENGINE_INLINE
engine::engine(std::string name_)
  : _name(std::move(name_))
{
}

_KLFENGINE_INLINE
void engine::set_settings(klfengine::settings settings_)
{
  _settings = std::move(settings_);
}


_KLFENGINE_INLINE
std::unique_ptr<klfengine::run>
engine::run( input input_ )
{
  std::unique_ptr<engine_run_implementation> impl_ptr{
    impl_create_engine_run_implementation(
        input_,
        settings()
        )
  };

  std::unique_ptr<klfengine::run> run_ptr{
    new klfengine::run{ std::move(impl_ptr) }
  };

  // don't use std::move() here explicitly, see
  // https://stackoverflow.com/a/19272035/1694896
  return run_ptr;
}




} // namespace klfengine
