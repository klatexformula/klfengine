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
      std::string{"klfeimplpkgtmp"} +
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

_KLFENGINE_INLINE
std::string run_implementation::assemble_latex_template(
    const klfengine::input & in
)
{
  using namespace klfengine::detail::utils;

  bool use_latex_template = true;

  { auto it_use_latex_template = in.parameters.find("use_latex_template");
    if (it_use_latex_template != in.parameters.end()) {
      use_latex_template = it_use_latex_template->second.get<bool>();
    }
  }

  if ( ! use_latex_template ) {
    // no need to go further, the user has prepared everything for us already
    return in.latex;
  }

  bool need_latex_color_package = false;

  std::string pre_preamble;
  std::string klf_preamble;

  std::string docclass{"article"};
  std::string docoptions{}; // don't include [] argument wrapper

  { auto it_docclass = in.parameters.find("document_class");
    if (it_docclass != in.parameters.end()) {
      docclass = it_docclass->second.get<std::string>();
    }
  }

  { auto it_docoptions = in.parameters.find("document_class_options");
    if (it_docoptions != in.parameters.end()) {
      docoptions = it_docoptions->second.get<std::string>();
    }
  }

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
  { auto br_it = in.parameters.find("baseline_rule");
    if (br_it != in.parameters.end()) {
      // specify baseline rule.
      if (br_it->second.has_type<bool>()) {
        baseline_rule = br_it->second.get<bool>();
        if (baseline_rule) {
          need_latex_color_package = true; // the default baseline_rule_setup uses \color{...}
        }
      } else {
        // it's a dict with values
        baseline_rule = true;
        auto d = br_it->second.get<value::dict>();
        { auto it = d.find("type");
          if (it != d.end()) { baseline_rule_type = it->second.get<std::string>(); } }
        { auto it = d.find("setup");
          if (it != d.end()) { baseline_rule_setup = it->second.get<std::string>(); } }
        { auto it = d.find("thickness");
          if (it != d.end()) { baseline_rule_thickness = it->second.get<std::string>(); } }
      }
    }
  }

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

  if (in.scale <= 0) {
    // invalid scale
    throw std::invalid_argument("input.scale has invalid value " + dbl_to_string(in.scale));
  }

  if (in.scale != 1) {
    pre_preamble += "\\klfSetScale{" + dbl_to_string(in.scale) + "}\n";
    //\klfSetXScale{5}
    //\klfSetYScale{5}
  }

  //\klfSetXAlignCoeff{0.1}
  //\klfSetYAlignCoeff{0.8}

  //\klfSetBottomAlignment{bbox} % default
  //\klfSetBottomAlignment{baseline}
  //\klfSetTopAlignment{bbox} % default
  //\klfSetTopAlignment{Xheight}

  pre_preamble += "\\klfSetTopMargin{" + dbl_to_string(in.margins.top) + "pt}\n";
  pre_preamble += "\\klfSetRightMargin{" + dbl_to_string(in.margins.right) + "pt}\n";
  pre_preamble += "\\klfSetBottomMargin{" + dbl_to_string(in.margins.bottom) + "pt}\n";
  pre_preamble += "\\klfSetLeftMargin{" + dbl_to_string(in.margins.left) + "pt}\n";

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

  if (need_latex_color_package) {
    latex_str +=
      "\\makeatletter"
      "\\newcommand\\klfEnsureColorPackageLoaded{%\n"
      "  \\@ifpackageloaded{color}{}{\\@ifpackageloaded{xcolor}{}{\\RequirePackage{xcolor}}}}%"
      "\\makeatother\n"
      ;
  }
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
  std::string tempfname{ d->temp_dir.path() / "klfetmp" };

  std::string latex_str{ assemble_latex_template(in) };

  dump_cstr_to_file(tempfname + ".tex", latex_str.c_str());


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
        tempfname
      },
      process::run_in_directory{ d->temp_dir.path().native() },
      process::capture_stdout_data{&out},
      process::capture_stderr_data{&err}
      );


  // TODO: SUPPORT FOR "latex" (->DVI) ENGINE !! (or skip it? if the user wants
  // a latex->dvi workflow they can use latextoimage_engine)


  binary_data pdf_data_obj;
  pdf_data_obj = load_file_data( tempfname + ".pdf" );

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
