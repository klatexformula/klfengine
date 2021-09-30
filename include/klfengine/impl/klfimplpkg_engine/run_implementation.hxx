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

#include <klfengine/klfimplpkg_engine>
#include <klfengine/temporary_directory>
#include <klfengine/h/detail/utils.h>
#include <klfengine/ghostscript_interface>
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

  std::shared_ptr<klfengine::ghostscript_interface_engine_tool> gs_iface_tool;

  klfengine::gs_device_args_format_provider gs_args_provider;

  bool via_dvi;

  fs::path fn_base;
  fs::path fn_tex;
  fs::path fn_pdfout;

  inline run_implementation_private(
      const klfengine::input & in, const klfengine::settings & sett,
      std::shared_ptr<klfengine::ghostscript_interface_engine_tool> gs_iface_tool_
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
    gs_iface_tool{ std::move(gs_iface_tool_) },
    gs_args_provider{
      gs_iface_tool.get(),
      value::dict{ // default parameter values:
        {"transparency", value{ (in.bg_color.alpha < 255) }},
        {"outline_fonts", value{in.outline_fonts}},
        {"dpi", value{in.dpi}},
        {"antialiasing", value{true}}
      }
    }
  {

    via_dvi = ((in.latex_engine == "latex") ? true : false);

    fn_base = temp_dir.path() / "klfetmp";
    fn_tex = fn_base;
    fn_tex.replace_extension(".tex");
    fn_pdfout = fn_base;
    fn_pdfout.replace_extension(".pdf");
  }
};


_KLFENGINE_INLINE
run_implementation::run_implementation(
    std::shared_ptr<klfengine::ghostscript_interface_engine_tool> gs_iface_tool_,
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
    pre_preamble += "\\klfSetXAlignCoeff{" + dbl_to_string(x) + "}%\n";
  });
  dict_do_if<double>(in.parameters, "y_align_coefficient",
                     [&pre_preamble](double x) {
    pre_preamble += "\\klfSetYAlignCoeff{" + dbl_to_string(x) + "}%\n";
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

  pre_preamble += "\\klfSetTopMargin{" + in.margins.top.to_string() + "}\n";
  pre_preamble += "\\klfSetRightMargin{" + in.margins.right.to_string() + "}\n";
  pre_preamble += "\\klfSetBottomMargin{" + in.margins.bottom.to_string() + "}\n";
  pre_preamble += "\\klfSetLeftMargin{" + in.margins.left.to_string() + "}\n";

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

  // one of "\\hbox", "\\vbox", "\\vtop", "\\vcenter", possibly followed by
  // TeX attribute such as "\\hbox to 2em"
  //
  // You CANNOT add arbitrary other TeX commands here, this will break the
  // template. (The contents of `content_tex_box_primitive` being inserted in a
  // construct like "\setbox\mybox=#1{content}")
  //
  // if you use one of the \vbox'es, it's up to you to make sure it has the
  // right width (set \hsize)
  bool content_tex_box_primitive =
    dict_get<std::string>(in.parameters, "content_tex_box_primitive", "\\hbox");

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

  latex_str += "\\begin{klfcontent}{" + content_tex_box_primitive
    + "}{" + font_cmds + "}%\n";

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

  // TODO: SUPPORT FOR "latex" (->DVI) ENGINE !! (or skip it? if the user wants
  // a latex->dvi workflow they can use latextoimage_engine)
  if ( d->via_dvi ) {
    throw std::runtime_error(
        "Running latex->DVI with klfimplpkg engine is not yet implemented!");
  }

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
  (void) store_to_cache(format_spec{"LATEX", value::dict{{"latex_raw", value{true}}}},
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


  binary_data pdf_data_obj;
  pdf_data_obj = load_file_data( d->fn_pdfout );

  (void) store_to_cache(format_spec{"PDF", value::dict{{"latex_raw", value{true}}}},
                        std::move(pdf_data_obj));

}

_KLFENGINE_INLINE
std::vector<format_description> run_implementation::impl_available_formats()
{
  std::vector<klfengine::format_description> fmtlist;

  // initialize with Ghostscript-based formats
  fmtlist = d->gs_args_provider.available_formats();

  value want_raw_spec{value::dict{
    { "type", value{std::string{"bool"}} }
  }};

  // find PDF, PS and add latex_raw parameter
  for (auto & x : fmtlist) {
    if (x.format_spec.format == "PDF" && !d->via_dvi) {
      x.format_spec.parameters["latex_raw"] = want_raw_spec;
      continue;
    }
    if (x.format_spec.format == "PS" && d->via_dvi) {
      x.format_spec.parameters["latex_raw"] = want_raw_spec;
      continue;
    }
  }

  // Add LATEX.  Don't even offer latex_raw option, cause it's not an option :) 
  fmtlist.push_back({
      { "LATEX", {} },
      "LaTeX document",
      "The full LaTeX document used to compile the equation",
  });

  return fmtlist;
}

_KLFENGINE_INLINE
klfengine::format_spec run_implementation::impl_make_canonical(
    const klfengine::format_spec & format, bool /*check_only*/
    )
{
  klfengine::format_spec canon_format;
  parameter_taker param{ format.parameters, "klfengine::klfimplpkg_engine" };

  if (format.format == "LATEX") {
    bool latex_raw = param.take("latex_raw", true);
    if (latex_raw == false) {
      param.disable_check();
      throw invalid_parameter{param.what(), "\"LATEX\" format requires latex_raw=true"};
    }
    param.finished();
    canon_format.format = "LATEX";
    canon_format.parameters["latex_raw"] = value{true};
    return canon_format;
  }
  if (format.format == "DVI") {
    if (d->via_dvi) {
      bool latex_raw = param.take("latex_raw", true);
      if (latex_raw == false) {
        param.disable_check();
        throw invalid_parameter{param.what(), "\"DVI\" format requires latex_raw=true"};
      }
      param.finished();
      canon_format.format = "DVI";
      canon_format.parameters["latex_raw"] = value{true};
      return canon_format;
    } else {
      // no DVI available, no such format
      param.disable_check();
      throw no_such_format{
        "There is no \"latex_raw\" DVI because the latex engine doesn't generate DVI output"
      };
    }
  }

  if (format.format == "PDF" || format.format == "PS") {
    // these formats can be the latex raw versions, enabled with latex_raw=true.
    bool want_latex_raw = param.take("latex_raw", false);

    if (want_latex_raw) {
      if (format.format == "PDF" && d->via_dvi) {
        param.disable_check();
        throw no_such_format{
          "There is no \"latex_raw\" PDF because the latex engine doesn't directly generate PDF"
        };
      }
      if (format.format == "PS" && !d->via_dvi) {
        param.disable_check();
        throw no_such_format{
          "There is no \"latex_raw\" PS because the latex engine doesn't generate DVI output"
        };
      }
    }

    if (want_latex_raw) {
      param.finished();
      canon_format.format = format.format;
      canon_format.parameters["latex_raw"] = value{true};
      return canon_format;
    }

    // get Ghostscript's canonical format
    
    format_spec gs_format{ format.format, param.take_remaining() };
    param.finished();
    canon_format = d->gs_args_provider.canonical_format( std::move(gs_format) );
    // fix canonical format to have this key regardless of whether raw version is available
    canon_format.parameters["latex_raw"] = value{false};
    return canon_format;
  }

  // Offer any remaining Ghostscript-based formats.  See if our Ghostscript
  // handler can handle them:
  canon_format = d->gs_args_provider.canonical_format_or_empty( format );
  if ( canon_format != format_spec{} ) {
    param.disable_check(); // already checked by gs_args_provider
    return canon_format;
  }

  // no such format
  param.disable_check();
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

  parameter_taker param{ format.parameters,
    "klfengine::klfimplpkg_engine::impl_produce_data" };

  // 
  bool latex_raw = param.take("latex_raw", false);
  if (latex_raw == true) {
    // All available RAW formats have all been stored in the cache at
    // compile-time.  If this function was called with a latex_raw=true
    // parameter, it means that there is no corresponding raw data.
    param.disable_check();
    throw invalid_parameter{param.what(),
        "No RAW format available for \"" + format.format + "\""};
  }

  if ( format.format == "PDF" && ! in.outline_fonts ) {
    // no further processing is needed.  We can use the raw PDF directly.
    auto raw_pdf_data = get_data_cref(format_spec{"PDF",
                                                  value::dict{{"latex_raw", value{true}}}});
    return raw_pdf_data;
  }

  auto gs_iface = d->gs_iface_tool->gs_interface();

  // don't use ghostscript STDOUT so that we can also use libgs-based methods in
  // ghostscript_interface
  fs::path outf = d->fn_base;
  outf.replace_filename(d->fn_base.filename().generic_string() + "-gs."
                        + to_lowercase(format.format));

  auto param_remaining = param.take_remaining();
  param.finished();

  std::vector<std::string> gs_process_args{
    d->gs_args_provider.get_device_args_for_format(
      format_spec{format.format, param_remaining}
    )
  };

  gs_process_args.push_back("-sOutputFile="+outf.native());

  // finally, the input file
  gs_process_args.push_back(d->fn_pdfout.native());

  // now, run the full ghostscript command.
  //binary_data gs_stderr;
  //binary_data gs_stdout;
  gs_iface->run_gs(
    gs_process_args,
    ghostscript_interface::add_standard_batch_flags{true}
    //ghostscript_interface::capture_stdout_data{&gs_stdout},
    //ghostscript_interface::capture_stderr_data{&gs_stderr}
  );

  binary_data gs_result_data{ load_file_data(outf.native()) };

  return gs_result_data;
}





} // namespace klfimplpkg_engine
} // namespace klfengine
