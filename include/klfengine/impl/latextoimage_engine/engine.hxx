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

#include <klfengine/latextoimage_engine>


namespace klfengine {
namespace latextoimage_engine {

_KLFENGINE_INLINE
engine::engine()
  : klfengine::engine("latextoimage_engine")
{
  _gs_iface_tool = std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool>{
    new klfengine::detail::simple_gs_interface_engine_tool{}
  };
}

_KLFENGINE_INLINE
engine::~engine()
{
}

_KLFENGINE_INLINE
void engine::adjust_for_new_settings(klfengine::settings & settings_)
{
  _gs_iface_tool->set_settings(settings_);
}

// reimplemented from klfengine::engine
_KLFENGINE_INLINE
klfengine::engine_run_implementation *
engine::impl_create_engine_run_implementation( klfengine::input input_,
                                               klfengine::settings settings_ )
{
  return new run_implementation(_gs_iface_tool, std::move(input_), std::move(settings_));
}




} // namespace latextoimage_engine
} // namespace klfengine
