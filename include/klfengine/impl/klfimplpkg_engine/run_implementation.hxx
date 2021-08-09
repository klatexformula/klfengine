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

#include <klfengine/klfimplpkg_engine>
#include <klfengine/temporary_directory>
#include <klfengine/h/detail/utils.h>
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

  fs::path fn_base;
  fs::path fn_tex;
  fs::path fn_pdfout;

  inline run_implementation_private(
      const klfengine::input & /*in*/, const klfengine::settings & sett,
      std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> gs_iface_tool_
  )
    :
    temp_dir{
      sett.temporary_directory,
      std::string{"klfeimplpkgtmp"} +
          _KLFENGINE_CONCAT_VER_3_j(
            KLFENGINE_VERSION_MAJOR,
            KLFENGINE_VERSION_MINOR,
            KLFENGINE_VERSION_RELEASE,
            "x"
          )
    },
    gs_iface_tool{ std::move(gs_iface_tool_) }
  {
    fn_base = temp_dir.path() / "klfetmp";
    fn_tex = fn_base;
    fn_tex.replace_extension(".tex");
    fn_pdfout = fn_base;
    fn_pdfout.replace_extension(".pdf");
  }
};


_KLFENGINE_INLINE
run_implementation::run_implementation(
    std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> gs_iface_tool_,
    klfengine::input input_,
    klfengine::settings settings_
    )
  : klfengine::engine_run_implementation(std::move(input_), std::move(settings_))
{

  d = new run_implementation_private{input(), settings(), std::move(gs_iface_tool_)};
}
_KLFENGINE_INLINE
run_implementation::~run_implementation()
{
  delete d;
  d = nullptr;
}

_KLFENGINE_INLINE
std::string run_implementation::assemble_latex_template(
    const klfengine::input & in
)
{
  using namespace klfengine::detail::utils;

  bool use_latex_template = dict_get<bool>(in.parameters, "use_latex_template", true);

  if ( ! use_latex_template ) {
    // no need to go further, the user has prepared everything for us already
    return in.latex;
  }

  bool need_latex_color_package = false;

  std::string pre_preamble;
  std::string klf_preamble;

  std::string docclass{ dict_get<std::string>(in.parameters, "document_class", "article") };
  // note, docoptions don't include [] argument wrapper
  std::string docoptions{ dict_get<std::string>(in.parameters, "document_class_options", "") };

  std::string font_cmds;

  if (in.font_size > 0) {
    font_cmds += "\\fontsize{" + dbl_to_string(in.font_size) + "}{" +
      dbl_to_string(in.font_size * 1.25) + "}\\selectfont";
  }

  if (in.fg_color != color{0,0,0,255}) {
    need_latex_color_package = true;
    klf_preamble += "\\definecolor{klffgcolor}{rgb}{"
      + dbl_to_string(in.fg_color.red/255.0) + ","
      + dbl_to_string(in.fg_color.green/255.0) + ","
      + dbl_to_string(in.fg_color.blue/255.0) + "}\n";
    // TODO: treat alpha!=255 case correctly

    font_cmds += "\\color{klffgcolor}";
  }

  bool baseline_rule = false;
  std::string baseline_rule_type{"line"};
  std::string baseline_rule_setup{"\\color{blue}"};
  std::string baseline_rule_thickness{"0.2pt"};
  dict_do_if<value>(in.parameters, "baseline_rule",
                    [&need_latex_color_package,&baseline_rule,&baseline_rule_type,
                     &baseline_rule_setup,&baseline_rule_thickness](const value& br) {
    // specify baseline rule.
    if (br.has_type<bool>()) {
      baseline_rule = br.get<bool>();
      if (baseline_rule) {
        need_latex_color_package = true; // the default baseline_rule_setup uses \color{...}
      }
    } else {
      // it's a dict with values
      baseline_rule = true;
      auto d = br.get<value::dict>();
      baseline_rule_type = dict_get<std::string>(d, "type", baseline_rule_type);
      baseline_rule_setup = dict_get<std::string>(d, "setup", baseline_rule_setup);
      baseline_rule_thickness = dict_get<std::string>(d, "thickness", baseline_rule_thickness);
    }
  });

  if (baseline_rule) {
    pre_preamble += 
      "\\klfSetBaselineRuleType{" + baseline_rule_type + "}\n"
      "\\renewcommand\\klfBaselineRuleLineSetup{" + baseline_rule_setup + "}\n"
      "\\renewcommand\\klfBaselineRuleLineThickness{" + baseline_rule_thickness + "}\n"
      ;
  }

  //\klfSetFixedWidth{4cm}
  //\klfSetFixedHeight{1cm}
  //\klfSetFixedWidth{10pt}
  //\klfSetFixedHeight{8pt}

  //\klfSetXAlignCoeff{0.1}
  //\klfSetYAlignCoeff{0.8}

  dict_do_if<std::string>(in.parameters, "fixed_width", [&pre_preamble](const std::string & x) {
    pre_preamble += "\\klfSetFixedWidth{" + x + "}%\n";
  });
  dict_do_if<std::string>(in.parameters, "fixed_height", [&pre_preamble](const std::string & x) {
    pre_preamble += "\\klfSetFixedHeight{" + x + "}%\n";
  });
  dict_do_if<double>(in.parameters, "x_align_coefficient",
                     [&pre_preamble](double x) {
    pre_preamble += "\\klfSetXalignCoeff{" + dbl_to_string(x) + "}%\n";
  });
  dict_do_if<double>(in.parameters, "y_align_coefficient",
                     [&pre_preamble](double x) {
    pre_preamble += "\\klfSetYalignCoeff{" + dbl_to_string(x) + "}%\n";
  });

  if (in.scale <= 0) {
    // invalid scale
    throw std::invalid_argument("input.scale has invalid value " + dbl_to_string(in.scale));
  }

  //\klfSetXScale{5}, \klfSetYScale{5}, \klfSetScale{5}
  bool set_xy_scale = false;
  dict_do_if<double>(in.parameters, "x_scale",
                     [&pre_preamble,&set_xy_scale](double x) {
    pre_preamble += "\\klfSetXScale{" + dbl_to_string(x) + "}%\n";
    set_xy_scale = true;
  });
  dict_do_if<double>(in.parameters, "y_scale",
                     [&pre_preamble,&set_xy_scale](double x) {
    pre_preamble += "\\klfSetYScale{" + dbl_to_string(x) + "}%\n";
    set_xy_scale = true;
  });
  if (in.scale != 1) {
    if (set_xy_scale) {
      warn("klfengine::klfimplpkg_engine::run_implementation",
           "Scaling must be set either with the 'input.scale' property or using the "
           "'input.parameters[\"x_scale\"]'/'input.parameters[\"y_scale\"]' parameters, "
           "you can't mix.");
    } else {
      pre_preamble += "\\klfSetScale{" + dbl_to_string(in.scale) + "}\n";
    }
  }

  //\klfSetBottomAlignment{bbox} % default
  //\klfSetBottomAlignment{baseline}
  //\klfSetTopAlignment{bbox} % default
  //\klfSetTopAlignment{Xheight}

  dict_do_if<std::string>(in.parameters, "top_alignment",
                          [&pre_preamble](const std::string & x) {
    pre_preamble += "\\klfSetTopAlignment{" + x + "}%\n"; // one of 'bbox' or 'Xheight'
  });
  dict_do_if<std::string>(in.parameters, "bottom_alignment",
                          [&pre_preamble](const std::string & x) {
    pre_preamble += "\\klfSetBottomAlignment{" + x + "}%\n"; // one of 'bbox' or 'baseline'
  });

  // margins

  pre_preamble += "\\klfSetTopMargin{" + dbl_to_string(in.margins.top) + "pt}\n";
  pre_preamble += "\\klfSetRightMargin{" + dbl_to_string(in.margins.right) + "pt}\n";
  pre_preamble += "\\klfSetBottomMargin{" + dbl_to_string(in.margins.bottom) + "pt}\n";
  pre_preamble += "\\klfSetLeftMargin{" + dbl_to_string(in.margins.left) + "pt}\n";

  // background: solid color, frame

  if (in.bg_color.alpha > 0) {
    // have solid bg color
    pre_preamble += "\\klfSetBackgroundColor{" +
      std::to_string(in.bg_color.red) + "," +
      std::to_string(in.bg_color.green) + "," +
      std::to_string(in.bg_color.blue) + "}%\n";
    pre_preamble += "\\klfSetBackgroundColorOpacity{" +
      dbl_to_string(in.bg_color.alpha/255.0) + "}%\n";
  }

  dict_do_if<value>(in.parameters, "bg_frame",
                    [&pre_preamble](const value& bgf_v) {
    if (bgf_v.has_type<bool>()) {
      pre_preamble += "\\klfSetBackgroundFrameThickness{0.4pt}%\n";
      pre_preamble += "\\klfSetBackgroundFrameOffset{1pt}%\n";
    } else {
      value::dict bgfd = bgf_v.get<value::dict>();
      bool bg_frame_on = false;
      bool need_set_default_thickness = true;
      dict_do_if<std::string>(bgfd, "thickness", [&](const std::string & t) {
        pre_preamble += "\\klfSetBackgroundFrameThickness{" + t + "}%\n";
        bg_frame_on = true;
        need_set_default_thickness = false;
      });
      dict_do_if<std::string>(bgfd, "color", [&](const std::string & t) {
        pre_preamble += "\\klfSetBackgroundFrameColor{" + t + "}%\n";
        bg_frame_on = true;
      });
      dict_do_if<std::string>(bgfd, "x_offset", [&](const std::string & t) {
        pre_preamble += "\\klfSetBackgroundFrameXOffset{" + t + "}%\n";
        bg_frame_on = true;
      });
      dict_do_if<std::string>(bgfd, "y_offset", [&](const std::string & t) {
        pre_preamble += "\\klfSetBackgroundFrameYOffset{" + t + "}%\n";
        bg_frame_on = true;
      });
      dict_do_if<std::string>(bgfd, "offset", [&](const std::string & t) {
        pre_preamble += "\\klfSetBackgroundFrameOffset{" + t + "}%\n";
        bg_frame_on = true;
      });
      if (bg_frame_on && need_set_default_thickness) {
        pre_preamble += "\\klfSetBackgroundFrameThickness{0.4pt}%\n";
      }
    }
  });

  // ---

  std::string latex_str;
  latex_str += "\\documentclass";
  if (docoptions.size()) {
    latex_str += "[";
    latex_str += docoptions;
    latex_str += "]";
  }
  latex_str += "{";
  latex_str += docclass;
  latex_str += "}\n";

  // our main klfimpl class
  latex_str += "\\usepackage[" + in.latex_engine + "]{klfimpl}\n";

  latex_str += pre_preamble;

  latex_str += "%%% --- begin user preamble ---\n";
  latex_str += in.preamble;
  latex_str += "\n";
  latex_str += "%%% --- end user preamble ---\n";
  if (need_latex_color_package) {
    // ensure color or xcolor have been loaded
    latex_str += "\\klfEnsureColorPackageLoaded\n";
  }
  latex_str += klf_preamble; // klf_preamble ends with "\n" already, we built it ourselves

  latex_str +=
    "\\pagestyle{empty}\n"
    "\\begin{document}%\n";

  latex_str += "\\begin{klfcontent}{\\hbox}{" + font_cmds + "}%\n";

  latex_str += "%%% --- begin user math_mode and latex ---\n";
  latex_str += in.math_mode.first;
  latex_str += "%\n";
  latex_str += in.latex;
  latex_str += "%\n";
  latex_str += in.math_mode.second;
  latex_str += "%\n";
  latex_str += "%%% --- end user math_mode and latex ---\n";

  latex_str += "\\end{klfcontent}%\n\\end{document}\n";

  //fprintf(stderr, "DEBUG: latex_str = '%s'\n", latex_str.c_str());

  return latex_str;
}

_KLFENGINE_INLINE
void run_implementation::impl_compile()
{
  using namespace klfengine::detail::utils;

  const klfengine::input & in = input();
  const klfengine::settings & sett = settings();

  // write our style file into the temp dir (TODO: maybe we could have a fixed
  // temp dir per engine instance, so we only do this once?) (OR TODO: maybe we
  // could have another different engine-wide temp dir to store the klfimpl.sty
  // file and use a TEXINPUTS=... when invoking latex) (OR TODO: We could inject
  // the code into the LaTeX template with some minimal boilerplate top/bottom
  // code as I do in my side project styexpress)
  std::string klfimplsty_fname{ (d->temp_dir.path() / "klfimpl.sty").native() };
  dump_cstr_to_file(klfimplsty_fname, detail::klfimpl_sty_data);

  // prepare latex template

  std::string latex_str{ assemble_latex_template(in) };

  dump_cstr_to_file( d->fn_tex.native(), latex_str.c_str() );


  //fprintf(stderr, "LATEX DOCUMENT IS =\n%s\n", latex_str.c_str());
  (void) store_to_cache(format_spec{"LATEX", value::dict{{"raw", value{true}}}},
                        binary_data{latex_str.begin(), latex_str.end()});

  // TODO: add meta-information to latex string!


  binary_data out;
  binary_data err;

  // run {|pdf|xe|lua}latex
  process::run_and_wait(
      { // argv
        sett.get_tex_executable_path(in.latex_engine),
        "-file-line-error",
        "-interaction=nonstopmode",
        d->fn_tex.native()
      },
      process::run_in_directory{ d->temp_dir.path().native() },
      process::capture_stdout_data{&out},
      process::capture_stderr_data{&err}
      );


  // TODO: SUPPORT FOR "latex" (->DVI) ENGINE !! (or skip it? if the user wants
  // a latex->dvi workflow they can use latextoimage_engine)


  binary_data pdf_data_obj;
  pdf_data_obj = load_file_data( d->fn_pdfout );

  (void) store_to_cache(format_spec{"PDF", value::dict{{"raw", value{true}}}},
                        std::move(pdf_data_obj));

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
  if (format.format == "LATEX") {
    return {"LATEX", value::dict{{"raw", value{true}}}};
  }

  if (format.format == "PDF") {
    bool want_raw = dict_get<bool>(format.parameters, "raw", false);
    return {"PDF", value::dict{{"raw", value{want_raw}}}};
  }
  
  if (format.format == "PNG" || format.format == "JPEG" ||
      format.format == "TIFF" || format.format == "BMP") {
    int dpi = dict_get<int>(format.parameters, "dpi", input().dpi);
    bool antialiasing = dict_get<bool>(format.parameters, "antialiasing", true);
    return {
      format.format,
      value::dict{
        {"dpi", value{dpi}},
        {"antialiasing", value{antialiasing}}
      }
    };
  }


  if (format.format == "PS") {
    return {"PS", {}};
  }
  if (format.format == "EPS") {
    return {"EPS", {}};
  }

  return {};
}

_KLFENGINE_INLINE
klfengine::binary_data run_implementation::impl_produce_data(
    const klfengine::format_spec & format
    )
{
  using namespace klfengine::detail::utils;
  using namespace klfengine::detail;

  const klfengine::input & in = input();
  //const klfengine::settings & sett = settings();

  if ( format.format == "PDF" && ! in.outline_fonts ) {
    // no further processing is needed.  We can use the raw PDF directly.
    auto raw_pdf_data = get_data_cref(format_spec{"PDF", value::dict{{"raw", value{true}}}});
    return raw_pdf_data;
  }

  auto gs_iface = d->gs_iface_tool->gs_interface();
  auto gs_ver = d->gs_iface_tool->gs_version();


  std::vector<std::string> gs_process_args;

  // don't use ghostscript STDOUT so that we can also use libgs-based methods in
  // simple_gs_interface
  fs::path outf = d->fn_base;
  outf.replace_filename(d->fn_base.filename().generic_string() + "-gs."
                        + to_lowercase(format.format));

  bool is_vector_format = true;

  // choose correct device
  if (format.format == "PNG") {
    is_vector_format = false;
    gs_process_args.push_back("-sDEVICE=pngalpha");
    // gs starts rendering transparency poorly in larger images without the
    // following option -- https://stackoverflow.com/a/4907328/1694896
    gs_process_args.push_back("-dMaxBitmap=2147483647");
  } else if (format.format == "JPEG") {
    is_vector_format = false;
    gs_process_args.push_back("-sDEVICE=jpeg");
  } else if (format.format == "TIFF") {
    is_vector_format = false;
    gs_process_args.push_back("-sDEVICE=tiff24nc");
  } else if (format.format == "BMP") {
    is_vector_format = false;
    gs_process_args.push_back("-sDEVICE=bmp16m");
  } else if (format.format == "PDF") {
    gs_process_args.push_back("-sDEVICE=pdfwrite");
  } else if (format.format == "PS") {
    gs_process_args.push_back("-sDEVICE=ps2write");
  } else if (format.format == "EPS") {
    gs_process_args.push_back("-sDEVICE=eps2write");
  }

  if ( ! is_vector_format ) {
    int dpi = dict_get<int>(format.parameters, "dpi");
    gs_process_args.push_back("-r" + std::to_string(dpi));
  }

  gs_process_args.push_back("-sOutputFile="+outf.native());

  // outline fonts, if applicable
  if (is_vector_format && in.outline_fonts) {
    if (gs_ver.major < 9 || (gs_ver.major == 9 && gs_ver.minor < 15)) {
      fprintf(stderr,
              "*** klfengine::latextoimage_engine warning: input requested outline_fonts=true, but "
              "you have ghostscript v%d.%d.  Please upgrade to gs>=9.15 for font outlines.\n",
              gs_ver.major, gs_ver.minor);
    } else {
      gs_process_args.push_back("-dNoOutputFonts");
    }
  }
  
  // anti-aliasing
  if ( ! is_vector_format ) {
    bool antialiasing = dict_get<bool>(format.parameters, "antialiasing");
    if (antialiasing) {
      gs_process_args.push_back("-dGraphicsAlphaBits=4");
      gs_process_args.push_back("-dTextAlphaBits=4");
    }
  }

  // finally, the input file
  gs_process_args.push_back(d->fn_pdfout.native());

  // now, run the full ghostscript command.
  //binary_data gs_stderr;
  //binary_data gs_stdout;
  gs_iface->run_gs(
    gs_process_args,
    simple_gs_interface::add_standard_batch_flags{true}
    //simple_gs_interface::capture_stdout_data{&gs_stdout},
    //simple_gs_interface::capture_stderr_data{&gs_stderr}
  );

  binary_data gs_result_data{ load_file_data(outf.native()) };

  return gs_result_data;
}





} // namespace klfimplpkg_engine
} // namespace klfengine
