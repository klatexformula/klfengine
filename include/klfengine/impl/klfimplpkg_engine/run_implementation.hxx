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

#include <cstdio> // FILE, fopen, fwrite
#include <system_error>
#include <cerrno>

#include <klfengine/klfimplpkg_engine>
#include <klfengine/temporary_directory>
#include <klfengine/h/detail/simple_gs_interface.h>
#include <klfengine/version>


namespace klfengine {
namespace klfimplpkg_engine {


namespace detail {

// klfimpl_sty_data.h expands to ` const char * klfimpl_sty_data = R"(...)"; '
// make the declaration 'static' to avoid errors if this file was included
// multiple times
static
#include <klfengine/impl/klfimplpkg_engine/klfimpl_sty_data.h>
} // namespace detail


struct run_implementation_private
{
  temporary_directory temp_dir;

  std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> gs_iface_tool;
};


_KLFENGINE_INLINE
run_implementation::run_implementation(
    std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> gs_iface_tool_,
    klfengine::input input_,
    klfengine::settings settings_
    )
  : klfengine::engine_run_implementation(std::move(input_), std::move(settings_))
{

  d = new run_implementation_private{
    // temp_dir
    temporary_directory{
      settings().temporary_directory,
      std::string{"klfeimplstytmp"} +
      _KLFENGINE_CONCAT_VER_3_j(
          KLFENGINE_VERSION_MAJOR,
          KLFENGINE_VERSION_MINOR,
          KLFENGINE_VERSION_RELEASE,
          "x"
          )
    },
    // gs_iface_tool
    std::move(gs_iface_tool_) // move the shared pointer, not the actual object (!)
  };

}
_KLFENGINE_INLINE
run_implementation::~run_implementation()
{
  delete d;
  d = nullptr;
}


static void dump_cstr_to_file(const std::string & fname, const char * c_buffer)
{
  std::FILE * fp;
  fp = std::fopen(fname.c_str(), "w");
  if (fp == NULL) {
    throw std::system_error{errno, std::generic_category()};
  }
  int res = std::fputs(c_buffer, fp);
  if (res == EOF) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::fclose(fp);
}

static binary_data load_file_data(const std::string & fname)
{
  std::size_t fsiz = fs::file_size(fs::path{fname});
  binary_data data;
  data.resize(fsiz);

  std::FILE * fp;
  fp = std::fopen(fname.c_str(), "rb");
  if (fp == NULL) {
    throw std::system_error{errno, std::generic_category()};
  }
  std::size_t res = std::fread(&data[0], 1, fsiz, fp);
  if (res < fsiz) {
    throw std::system_error{errno, std::generic_category()};
  }
  // check that all data was read
  // if ( ! std::feof(fp) ) {
  //   throw std::runtime_error{"Expected that we were at end of stream after reading "
  //                            + std::to_string(fsiz) + " bytes from " + fname};
  // }
  std::fclose(fp);

  return data;
}

_KLFENGINE_INLINE
void run_implementation::impl_compile()
{
  const klfengine::input & in = input();
  const klfengine::settings & sett = settings();

  // write our style file into the temp dir (TODO: maybe we could have a fixed
  // temp dir per engine instance, so we only do this once?) (OR TODO: maybe we
  // could have another different engine-wide temp dir to store the klfimpl.sty
  // file and use a TEXINPUTS=... when invoking latex)
  std::string klfimplsty_fname = (d->temp_dir.path() / "klfimpl.sty").native();
  dump_cstr_to_file(klfimplsty_fname, detail::klfimpl_sty_data);

  // prepare latex template
  std::string tempfname = d->temp_dir.path() / "klfetmp";

  std::string latex_str;
  latex_str += "\\documentclass[11pt]{article}\n";
  latex_str += "\\usepackage[" + in.latex_engine + "]{klfimpl}\n";
  
  latex_str += "\\klfSetTopMargin{" + std::to_string(in.margins.top) + "pt}\n";
  latex_str += "\\klfSetRightMargin{" + std::to_string(in.margins.right) + "pt}\n";
  latex_str += "\\klfSetBottomMargin{" + std::to_string(in.margins.bottom) + "pt}\n";
  latex_str += "\\klfSetLeftMargin{" + std::to_string(in.margins.left) + "pt}\n";

  //\klfSetFixedWidth{4cm}
  //\klfSetFixedHeight{1cm}
  //\klfSetFixedWidth{10pt}
  //\klfSetFixedHeight{8pt}

  if (in.scale != 1) {
    latex_str += "\\klfSetScale{" + std::to_string(in.scale) + "}\n";
    //\klfSetXScale{5}
    //\klfSetYScale{5}
  }

  //\klfSetXAlignCoeff{0.1}
  //\klfSetYAlignCoeff{0.8}

  //\klfSetBottomAlignment{bbox} % default
  //\klfSetBottomAlignment{baseline}
  //\klfSetTopAlignment{bbox} % default
  //\klfSetTopAlignment{Xheight}

  //\klfSetBaselineRuleType{line}
  //\renewcommand\klfBaselineRuleLineSetup{\color{blue}}
  //\renewcommand\klfBaselineRuleLineThickness{0.2pt}

  latex_str += in.preamble;

  latex_str +=
    "\n"
    "\\pagestyle{empty}\n"
    "\\begin{document}%\n";

  std::string font_cmds;
  if (in.font_size > 0) {
    font_cmds = "\\fontsize{" + std::to_string(in.font_size) + "}{" +
      std::to_string(in.font_size * 1.25) + "}\\selectfont";
  }

  latex_str += "\\begin{klfcontent}{\\hbox}{" + font_cmds + "}%\n";

  latex_str += in.math_mode.first;
  latex_str += "%\n";
  latex_str += in.latex;
  latex_str += "%\n";
  latex_str += in.math_mode.second;
  latex_str += "%\n";

  latex_str += "\\end{klfcontent}\n\\end{document}\n";

  dump_cstr_to_file(tempfname + ".tex", latex_str.c_str());


  fprintf(stderr, "LATEX DOCUMENT IS =\n%s\n", latex_str.c_str());

  binary_data out;
  binary_data err;

  // run {|pdf|xe|lua}latex
  process::run_and_wait(
      { // argv
        sett.get_tex_executable_path(in.latex_engine),
        "-file-line-error",
        "-interaction=nonstopmode",
        tempfname
      },
      process::run_in_directory{ d->temp_dir.path().native() },
      process::capture_stdout_data{out},
      process::capture_stderr_data{err}
      );

  binary_data pdf_data_obj;
  pdf_data_obj = load_file_data( (tempfname + ".pdf").c_str() );
  value::dict rawtrue{{"raw", value{true}}};
//  const binary_data & pdf_data =
    store_to_cache(format_spec{"PDF", rawtrue}, std::move(pdf_data_obj));

}

_KLFENGINE_INLINE
std::vector<format_description> run_implementation::impl_available_formats()
{
  return {}; //format_description{"PDF"}}; // for now
}

_KLFENGINE_INLINE
klfengine::format_spec run_implementation::impl_make_canonical(
    const klfengine::format_spec & format, bool /*check_only*/
    )
{
  if (format.format == "PDF") {
    return {"PDF", value::dict{{"raw", value{true}}}};
  }
  return {};
}

_KLFENGINE_INLINE
klfengine::binary_data run_implementation::impl_produce_data(
    const klfengine::format_spec &
    )
{
  // todo ...
  return {};
}




} // namespace klfimplpkg_engine
} // namespace klfengine
