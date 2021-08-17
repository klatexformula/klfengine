/* This file is part of the klfengine library, which is distributed under the
 * terms of the MIT license.
 *
 *     https://github.com/klatexformula/klfengine
 *
 * The MIT License (MIT)
 *
 * Copyright 2021 Philippe Faist
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

#include <klfengine/engine>
#include <klfengine/run>
#include <klfengine/engine_run_implementation>

#include <klfengine/h/latextoimage_engine/run_implementation.h>
#include <klfengine/h/detail/simple_gs_interface.h>

namespace klfengine {
namespace latextoimage_engine {


class engine : public klfengine::engine {
public:
  engine();
  virtual ~engine();

private:
  // reimplemented from klfengine::engine
  void adjust_for_new_settings(klfengine::settings & settings);
  klfengine::engine_run_implementation *
  impl_create_engine_run_implementation( klfengine::input input_,
                                         klfengine::settings settings_ );

  std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> _gs_iface_tool;
};



} // namespace latextoimage_engine
} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/latextoimage_engine/engine.hxx>
#endif
