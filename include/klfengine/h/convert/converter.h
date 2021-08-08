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
#include <klfengine/input>
#include <klfengine/settings>


namespace klfengine {

namespace convert {

class converter
{
public:

  inline
  binary_data convert(
    const klfengine::format_spec & to_format,
    const klfengine::format_spec & from_format,
    const binary_data & data,
    const klfengine::input & input,
    const klfengine::input & settings
  )
  {
    return impl_convert(to_format, from_format, data, input, settings);
  }

private:

  virtual binary_data impl_make_canonical(
    const klfengine::format_spec & to_format,
    bool check_available_only.........
  ) = 0;

  virtual binary_data impl_convert(
    const klfengine::format_spec & to_format,
    const klfengine::format_spec & from_format,
    const binary_data & data,
    const klfengine::input & input,
    const klfengine::input & settings
  ) = 0;
    
  std::shared_ptr<detail::simple_gs_interface_tool> _gs_iface_tool;
};


} // namespace convert
} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/convert/gs_image.hxx>
#endif
