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

#include <klfengine/run>
#include <klfengine/engine_run_implementation>


namespace klfengine {
namespace latex_dvips_gs_engine {

/*

struct run_implementation_private;

class run_implementation : public klfengine::engine_run_implementation
{
public:
  run_implementation(
      klfengine::input input_,
      klfengine::settings settings_
      );
  virtual ~run_implementation();

protected:
  void create_latex_template();
  void create_dvi();
  void create_eps_raw();
  void create_eps_processed();

private:
  run_implementation_private *d;

  virtual void impl_compile();
  virtual std::vector<klfengine::format_description> impl_available_formats();
  virtual klfengine::format_spec impl_make_canonical(
      const klfengine::format_spec & format, bool check_only
      );
  virtual klfengine::binary_data impl_produce_data(const klfengine::format_spec & format);
};
*/

} // namespace latex_dvips_gs_engine
} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/latex_dvips_gs_engine/run_implementation.hxx>
#endif
