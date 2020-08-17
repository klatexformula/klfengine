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

#include <klfengine/latex_dvips_gs_engine>


namespace klfengine {
namespace latex_dvips_gs_engine {



struct run_implementation_private
{
  temporary_directory temp_dir;

  struct filenames_t {
    std::string tex, dvi, rawEps, bboxEps, processedEps, rawPng,
      pdfMarks, pdf, gsSvg, svg;
  };
  filenames_t fn;
};


run_implementation::run_implementation(
    std::shared_ptr<detail::simple_gs_interface> gs_iface_,
    const detail::simple_gs_interface::gs_version_and_info_t & gs_version_and_info_,
    klfengine::input input_,
    klfengine::settings settings_
    )
  : klfengine::engine_run_implementation(std::move(input_), std::move(settings_)),
    _gs_iface{std::move(gs_iface_)},
    _gs_version_and_info{gs_version_and_info_}
{

  d = new run_implementation_private{
    { // temp_dir
     settings().temporary_directory_path,
      std::string{"klfetmp"} +
      _KLFENGINE_CONCAT_VER_3_j(
          KLFENGINE_VERSION_MAJOR,
          KLFENGINE_VERSION_MINOR,
          KLFENGINE_VERSION_RELEASE,
          "x"
          )
    },
    {} // fn's
  };
  
  std::string tempfname = d.temp_dir.path() / "klfetemp";

  d.fn.tex = tempfname + ".tex";
  d.fn.dvi = tempfname + ".dvi";
  d.fn.rawEps = tempfname + ".eps";
  d.fn.bboxEps = tempfname + "-bbox.eps";
  d.fn.processedEps = tempfname + "-processed.eps";
  d.fn.rawPng = tempfname + "-raw.png";
  d.fn.pdfMarks = tempfname + ".pdfmarks";
  d.fn.pdf = tempfname + ".pdf";
  d.fn.gsSvg = tempfname + "-gs.svg";
  d.fn.svg = tempfname + ".svg";

  ......... ..............
}
run_implementation::~run_implementation()
{
  delete d;
  d = nullptr;
}


run_implementation::impl_compile()
{
}

std::vector<klfengine::format_description> impl_available_formats()
{
  return {"EPS", "PDF", "PNG"}; // for now
}

klfengine::format_spec impl_make_canonical(
    const klfengine::format_spec & format, bool check_only
    )
{
  if (format.format == "EPS") {
    return {"EPS", {}};
  }
  if (format.format == "PDF") {
    return {"PDF", {}};
  }
  if (format.format == "PNG") {
    return {"PNG", {}};
  }
  return {};
}

klfengine::binary_data impl_produce_data(const klfengine::format_spec & format)
{
}




} // namespace latex_dvips_gs_engine
} // namespace klfengine
