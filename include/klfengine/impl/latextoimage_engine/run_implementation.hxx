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

#include <regex>

#include <klfengine/latextoimage_engine>
#include <klfengine/temporary_directory>
#include <klfengine/h/detail/utils.h>
#include <klfengine/h/detail/simple_gs_interface.h>
#include <klfengine/version>


namespace klfengine {
namespace latextoimage_engine {

namespace detail {

struct bbox {
  bbox(double x1_ = 0, double y1_ = 0, double x2_ = 0, double y2_ = 0)
    : x1(x1_), y1(y1_), x2(x2_), y2(y2_)
  {}

  double x1;
  double y1;
  double x2;
  double y2;

  template<typename ScaleFactor>
  bbox scaled_by(ScaleFactor a) const {
    return bbox{a*x1, a*y1, a*x2, a*y2};
  }
};

} // namespace detail



struct run_implementation_private
{
  temporary_directory temp_dir;

  std::shared_ptr<klfengine::detail::simple_gs_interface_engine_tool> gs_iface_tool;

  struct filenames_t
  {
    fs::path base, tex, dvi, ps, pdf, gs_input;

    inline void set(fs::path base_, bool via_dvi)
    {
      base = base_;
      tex = base_; tex.replace_extension(".tex");
      dvi = base_; dvi.replace_extension(".dvi");
      ps = base_; ps.replace_extension(".dvi");
      pdf = base_; pdf.replace_extension(".pdf");

      gs_input = (via_dvi ? ps : pdf);
    }

  };
  filenames_t fn;

  bool via_dvi;

  detail::bbox rawbbox;

  detail::bbox bbox;
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
      std::string{"klfelatextoimgtmp"} +
      _KLFENGINE_CONCAT_VER_3_j(
          KLFENGINE_VERSION_MAJOR,
          KLFENGINE_VERSION_MINOR,
          KLFENGINE_VERSION_RELEASE,
          "x"
          )
    },
    // gs_iface_tool
    std::move(gs_iface_tool_), // move the shared pointer, not the actual object (!)
    // fn
    {},
    // via_dvi
    (input_.latex_engine == "latex"),
    // rawbbox
    {},
    // bbox
    {}
  };
  
  d->fn.set(d->temp_dir.path() / "klfetemp", d->via_dvi);
  
}
_KLFENGINE_INLINE
run_implementation::~run_implementation()
{
  delete d;
  d = nullptr;
}


_KLFENGINE_INLINE
std::string run_implementation::assemble_latex_template(const klfengine::input & in)
{
  using namespace klfengine::detail::utils;

  // set up latex document

  bool use_latex_template = dict_get<bool>(in.parameters, "use_latex_template", true);

  if ( ! use_latex_template ) {
    // no need to go further, the user has prepared everything for us already
    return in.latex;
  }

  std::string docclass{ dict_get<std::string>(in.parameters, "document_class", "article") };
  // note, docoptions don't include [] argument wrapper
  std::string docoptions{ dict_get<std::string>(in.parameters, "document_class_options", "") };

  std::string ltxcolorpkg{ dict_get<std::string>(in.parameters, "latex_color_package", "color") };

  // tape together latex document
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

  bool need_fg_color = (in.fg_color != color{0,0,0,255});

  if (need_fg_color) {
    latex_str += "\\usepackage{";
    latex_str += ltxcolorpkg;
    latex_str += "}\n";
    latex_str += "\\definecolor{klffgcolor}{rgb}{" + dbl_to_string(in.fg_color.red/255.0) + "," +
      dbl_to_string(in.fg_color.green/255.0) + "," + dbl_to_string(in.fg_color.blue/255.0) + "}\n";
  }

  latex_str += in.preamble;
  latex_str += "\n";

  latex_str += "\\begin{document}\n"
    "\\thispagestyle{empty}\n";

  if (in.font_size > 0) {
    latex_str += "\\fontsize{" + dbl_to_string(in.font_size) + "}{"
      + dbl_to_string(in.font_size*1.25) + "}\\selectfont\n";
  }

  if (need_fg_color) {
    latex_str += "{\\color{klffgcolor}%\n";
  }

  // begin math mode
  latex_str += in.math_mode.first;
  latex_str += "%\n";

  // main latex content to compile
  latex_str += in.latex;
  latex_str += "%\n";

  // end math mode
  latex_str += in.math_mode.second;
  latex_str += "%\n";

  if (need_fg_color) {
    latex_str += "}%\n";
  }

  latex_str += "\\end{document}\n";

  return latex_str;
};


_KLFENGINE_INLINE
void run_implementation::impl_compile()
{
  using namespace klfengine::detail::utils;
  using namespace klfengine::detail;

  const klfengine::input & in = input();
  const klfengine::settings & sett = settings();

  std::string latex_str = assemble_latex_template(in);
  
  dump_cstr_to_file(d->fn.tex.native(), latex_str.c_str());

  //fprintf(stderr, "LATEX DOCUMENT IS =\n%s\n", latex_str.c_str());
  (void) store_to_cache(format_spec{"LATEX", value::dict{{"raw", value{true}}}},
                        binary_data{latex_str.begin(), latex_str.end()});


  binary_data latex_out;
  binary_data latex_err;

  // run {|pdf|xe|lua}latex
  process::run_and_wait(
      { // argv
        sett.get_tex_executable_path(in.latex_engine),
        "-file-line-error",
        "-interaction=nonstopmode",
        d->fn.tex
      },
      process::run_in_directory{ d->temp_dir.path().native() },
      process::capture_stdout_data{&latex_out},
      process::capture_stderr_data{&latex_err}
      );


  if (d->via_dvi) {

    binary_data dvi_data_obj;
    dvi_data_obj = load_file_data( d->fn.dvi.native() );
    //  const binary_data & dvi_data =
      store_to_cache(format_spec{"DVI", value::dict{{"raw", value{true}}}},
                     std::move(dvi_data_obj));

    binary_data dvips_out;
    binary_data dvips_err;

    // run dvips
    process::run_and_wait(
      { // argv
        sett.get_tex_executable_path("dvips"),
        d->fn.dvi
      },
      process::run_in_directory{ d->temp_dir.path().native() },
      process::capture_stdout_data{&dvips_out},
      process::capture_stderr_data{&dvips_err}
      );

    binary_data ps_data_obj;
    ps_data_obj = load_file_data( d->fn.ps.native() );
    //  const binary_data & dvi_data =
      store_to_cache(format_spec{"PS", value::dict{{"raw", value{true}}}},
                     std::move(ps_data_obj));
      
  } else {
    binary_data pdf_data_obj;
    pdf_data_obj = load_file_data( d->fn.pdf.native() );
    ;
    //  const binary_data & pdf_data =
      store_to_cache(format_spec{"PDF", value::dict{{"raw", value{true}}}},
                     std::move(pdf_data_obj));
  }

  // in either case, we read out the (hi res) bounding box using ghostscript
  
  binary_data gsbbox_err_data;

  auto gs_iface = d->gs_iface_tool->gs_interface();

  gs_iface->run_gs(
    {
      "-sDEVICE=bbox",
      d->fn.gs_input.native()
    },
    simple_gs_interface::add_standard_batch_flags{true},
    simple_gs_interface::capture_stderr_data{&gsbbox_err_data}
  );

  std::string gsbbox_err{gsbbox_err_data.begin(), gsbbox_err_data.end()};

  std::regex rxgsbbox{
    "(?:^|\n)\\%\\%\\s*HiResBoundingBox\\s*:\\s*"
      "([0-9.e+-]+)\\s+([0-9.e+-]+)\\s+([0-9.e+-]+)\\s+([0-9.e+-]+)\\s*(\\n|$)"
  };

  std::smatch gsbbox_match;
  bool ok = std::regex_search(gsbbox_err, gsbbox_match, rxgsbbox);
  if (!ok) {
    throw std::runtime_error("Couldn't parse gs bounding box information: " + gsbbox_err);
  }

  d->rawbbox = detail::bbox{
    std::stod(gsbbox_match[1].str()),
    std::stod(gsbbox_match[2].str()),
    std::stod(gsbbox_match[3].str()),
    std::stod(gsbbox_match[4].str())
  };

  // compute bbox of scaled graphics, including margins etc.
  d->bbox = d->rawbbox;
  d->bbox.x1 -= in.margins.left.to_value_as_bp();
  d->bbox.y1 -= in.margins.bottom.to_value_as_bp();
  d->bbox.x2 += in.margins.right.to_value_as_bp();
  d->bbox.y2 += in.margins.top.to_value_as_bp();
  // also apply scale to margins
  d->bbox = d->bbox.scaled_by(in.scale);
}

_KLFENGINE_INLINE
std::vector<klfengine::format_description> run_implementation::impl_available_formats()
{
  return {}; // FIXME !!!
  // if (d->via_dvi) {
  //   return { {"DVI",{}}, {"EPS",{}}, {"PS",{}}, {"PDF",{}}, {"PNG",{}} };
  // } else {
  //   return { {"EPS",{}}, {"PS",{}}, {"PDF",{}}, {"PNG",{}}} };
  // }
}

_KLFENGINE_INLINE
klfengine::format_spec run_implementation::impl_make_canonical(
    const klfengine::format_spec & format, bool /*check_available_only*/
    )
{
  if (format.format == "LATEX") {
    return {"LATEX", value::dict{{"raw", value{true}}}};
  }
  if (format.format == "DVI") {
    if (d->via_dvi) {
      return {"DVI", value::dict{{"raw", value{true}}}}; // always "raw"
    } else {
      // no DVI available, no such format
      return {};
    }
  }
  if (format.format == "EPS") {
    return {"EPS", {}};
  }
  if (format.format == "PS") {
    bool want_raw = false;
    if (d->via_dvi) {
      // could request either raw or non-raw PS
      want_raw = dict_get<bool>(format.parameters, "raw", want_raw);
    }
    return {"PS", value::dict{{"raw", value{want_raw}}}};
  }
  if (format.format == "PDF") {
    bool want_raw = false;
    if (!d->via_dvi) {
      // could request either raw or non-raw PDF
      want_raw = dict_get<bool>(format.parameters, "raw", want_raw);
    }
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

  return {};
}

_KLFENGINE_INLINE
klfengine::binary_data
run_implementation::impl_produce_data(const klfengine::format_spec & format)
{
  using namespace klfengine::detail::utils;
  using namespace klfengine::detail;

  const klfengine::input & in = input();
  //const klfengine::settings & sett = settings();

  const double bg_bleed_pt = 1.0; // draw bg rectangle extending 1 pt outside each margin side

  //
  // every data format we produce here (for now) is based on a ghostscript call
  // on the file 'd->fn.gs_input'. In each case we need to apply some
  // transformations to:
  //
  // - crop at the desired bounding box
  //
  // - apply scale
  //
  // - draw background rectangle, if applicable
  //
  // - outline fonts
  //

  bool bg_is_transparent = (in.bg_color.alpha < 255);

  std::vector<std::string> gs_process_args;

  // don't use ghostscript STDOUT so that we can also use libgs-based methods in
  // simple_gs_interface
  fs::path outf = d->fn.base;
  outf.replace_filename(d->fn.base.filename().generic_string() + "-gs."
                        + to_lowercase(format.format));

  bool is_vector_format = true;

  // choose correct device
  if (format.format == "PNG") {
    is_vector_format = false;
    if (bg_is_transparent) {
      gs_process_args.push_back("-sDEVICE=pngalpha");
      // gs starts rendering transparency poorly in larger images without the
      // following option -- https://stackoverflow.com/a/4907328/1694896
      gs_process_args.push_back("-dMaxBitmap=2147483647");
    } else {
      gs_process_args.push_back("-sDEVICE=png16m");
    }
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

  const double widthpt  = d->bbox.x2 - d->bbox.x1;
  const double heightpt = d->bbox.y2 - d->bbox.y1;

  auto gs_iface = d->gs_iface_tool->gs_interface();
  auto gs_ver = d->gs_iface_tool->gs_version();

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
  
  // output size
  gs_process_args.push_back("-dDEVICEWIDTHPOINTS=" + dbl_to_string(widthpt));
  gs_process_args.push_back("-dDEVICEHEIGHTPOINTS=" + dbl_to_string(heightpt));
  gs_process_args.push_back("-dFIXEDMEDIA");

  // anti-aliasing
  if ( ! is_vector_format ) {
    bool antialiasing = dict_get<bool>(format.parameters, "antialiasing");
    if (antialiasing) {
      gs_process_args.push_back("-dGraphicsAlphaBits=4");
      gs_process_args.push_back("-dTextAlphaBits=4");
    }
  }

  // PostScript page initialization code -- draw background color rectangle, then apply
  // translation & scaling
  std::string gs_ps_cmds;
  gs_ps_cmds +=
    "<< /BeginPage { ";

  if (!bg_is_transparent) {
    gs_ps_cmds +=
      "newpath " +
      dbl_to_string(-bg_bleed_pt) +  " " + dbl_to_string(-bg_bleed_pt) + " moveto " +
      dbl_to_string(widthpt+2*bg_bleed_pt) + " " + dbl_to_string(-bg_bleed_pt) + " lineto " +
      dbl_to_string(widthpt+2*bg_bleed_pt) + " "
          + dbl_to_string(heightpt+2*bg_bleed_pt) + " lineto " +
      dbl_to_string(-bg_bleed_pt) + " " + dbl_to_string(heightpt+2*bg_bleed_pt)+ " lineto " +
      "closepath "
      "gsave " +
      dbl_to_string(in.bg_color.red/255.0) + " " + dbl_to_string(in.bg_color.green/255.0)
          + " " + dbl_to_string(in.bg_color.blue/255.0) + " setrgbcolor "
      "fill "
      "grestore "
      ;
  }

  gs_ps_cmds += dbl_to_string(-d->bbox.x1) + " " + dbl_to_string(-d->bbox.y1)
    + " translate ";
  gs_ps_cmds += dbl_to_string(in.scale) + " " + dbl_to_string(in.scale) + " scale ";
  
  gs_ps_cmds +=
    "} >> setpagedevice ";
  
  gs_process_args.push_back("-c");
  gs_process_args.push_back(gs_ps_cmds);

  //fprintf(stderr, "DEBUG: gs_ps_cmds = %s\n", gs_ps_cmds.c_str());

  // finally, the input file
  gs_process_args.push_back("-f");
  gs_process_args.push_back(d->fn.gs_input.native());

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




} // namespace latextoimage_engine
} // namespace klfengine
