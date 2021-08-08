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

#include <klfengine/convert/gs_image>


namespace klfengine {
namespace convert {

_KLFENGINE_INLINE
binary_data gs_image::convert(
  const klfengine::format_spec & to_format,
  const klfengine::format_spec & /*from_format*/,
  const binary_data & data,
  const klfengine::settings & settings
)
{
  using namespace klfengine::detail::utils;

  std::string gs_device;
  int dpi = 600;

  std::vector<std::string> gs_device_options;

  // By default, we'll ask ghostscript to write to its stdout stream.  In case
  // we'd like to output to a file for certain devices, we might create a
  // temporary file in a temporary dir:
  std::unique_ptr<klfengine::temporary_directory> output_temp_dir = nullptr;

  // cf https://ghostscript.com/doc/current/Devices.htm
  if (to_format.format == "PNG") {
    //std::string device = (transparent ? "pngalpha" : "png16m");
    gs_device = "pngalpha";
    // TODO: use png16m if no transparency ?  Actually pngalpha seems to work in all cases ...
  } else if (to_format.format == "JPEG") {
    gs_device = "jpeg";
    // TODO: add -dJPEGQ=... or -dQFactor options in gs_device_options,
    // according to to_format options?
  } else if (to_format.format == "BMP") {
    gs_device = "bmp16m";
  } else if (to_format.format == "PNM") {
    gs_device = "pnm";
  } else if (to_format.format == "TIFF") {
    gs_device = "tiff24nc";
    // check docs -- gs needs seekable output device (i.e., not stdout)
    output_temp_dir = new klfengine::temporary_directory{ settings.temporary_directory };
  } else {
    throw no_such_format("klfengine::convert::gs_image can't convert to "
                         + to_format.format);
  }

  auto it_keydpi = to_format.parameters.find("dpi");
  if (it_keydpi != to_format.parameters.end()) {
    dpi = it_keydpi->second.get<int>();
  }

  std::string outfname{"-"};
  if (output_temp_dir != nullptr) {
    outfname = ( output_temp_dir.path() /
                 ("gs_image_out." + to_lowercase(to_format.format)) ).native();
  }

  std::string gs_stderr;

  std::vector<std::string> gs_args{
    "-sDEVICE=" + device,
    "-r" + std::to_string(dpi),
    "-dGraphicsAlphaBits=4", "-dTextAlphaBits=4",
    "-sOutputFile=" + outfname,
    "-" // take input from stdin
  };

  auto gsiface = _gs_iface_tool->gs_interface();

  auto gs_stdout = gsiface->run_gs(
    gs_args,
    data,
    true,
    &gs_stderr
  );

  if (outfname == "-") {
    // gs wrote result data to stdout
    return gs_stdout;
  }
  return load_file_data(outfname);
}
    




} // namespace convert
} // namespace klfengine
