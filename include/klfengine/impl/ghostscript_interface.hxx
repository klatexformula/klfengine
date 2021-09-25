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

#include <regex>
#include <string>

#include <klfengine/process>
#include <klfengine/h/detail/utils.h>
#include <klfengine/h/ghostscript_interface.h>


#if defined(KLFENGINE_USE_LINKED_GHOSTSCRIPT) || defined(KLFENGINE_USE_LOAD_GHOSTSCRIPT)
#include <ghostscript/iapi.h>
#include <ghostscript/ierrors.h>
#endif


namespace klfengine {



_KLFENGINE_INLINE
ghostscript_error::ghostscript_error(std::string msg)
  : exception(msg)
{}

_KLFENGINE_INLINE
ghostscript_error::~ghostscript_error()
{}



struct ghostscript_interface_private
{
  ghostscript_interface::method method;
  std::string gs_path;

  std::vector<std::string> construct_gs_argv(
    std::string argv0,
    std::vector<std::string> gs_args,
    bool add_standard_batch_flags
  );

  void impl_run_gs_process(
    std::vector<std::string> gs_argv,
    const binary_data * stdin_data,
    bool add_standard_batch_flags,
    binary_data * capture_stdout,
    binary_data * capture_stderr
  );
  void impl_run_gs_linkedlibgs(
    std::vector<std::string> gs_argv,
    const binary_data * stdin_data,
    bool add_standard_batch_flags,
    binary_data * capture_stdout,
    binary_data * capture_stderr
  );
  void impl_run_gs_loadlibgs(
    std::vector<std::string> gs_argv,
    const binary_data * stdin_data,
    bool add_standard_batch_flags,
    binary_data * capture_stdout,
    binary_data * capture_stderr
  );
};


_KLFENGINE_INLINE
ghostscript_interface::ghostscript_interface(method method_, std::string gs_path)
{
  d = new ghostscript_interface_private{method_, std::move(gs_path)};
}

_KLFENGINE_INLINE
ghostscript_interface::ghostscript_interface(std::string method_s, std::string gs_path)
{
  d = new ghostscript_interface_private{parse_method(method_s), std::move(gs_path)};
}

_KLFENGINE_INLINE
ghostscript_interface::~ghostscript_interface()
{
  if (d != nullptr) {
    delete d;
  }
  d = nullptr;
}



_KLFENGINE_INLINE
ghostscript_interface::method ghostscript_interface::gs_method() const
{
  return d->method;
}
_KLFENGINE_INLINE
const std::string & ghostscript_interface::gs_path() const
{
  return d->gs_path;
}



// static
_KLFENGINE_INLINE
ghostscript_interface::method
ghostscript_interface::parse_method(const std::string & method_s)
{
  if (method_s == "none") {
    return method::None;
  } else if (method_s == "process") {
    return method::Process;
  } else if (method_s == "linked-libgs") {
    return method::LinkedLibgs;
  } else if (method_s == "load-libgs") {
    return method::LoadLibgs;
  }
  throw std::invalid_argument("Invalid gs interface method: " + method_s);
}




_KLFENGINE_INLINE
ghostscript_interface::gs_version_t ghostscript_interface::get_gs_version()
{
#if defined(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  if (d->method == method::LinkedLibgs) {

    gsapi_revision_t r;
    if (gsapi_revision(&r, sizeof(r)) == 0) {
      //fprintf(stderr, "DEBUG: Got gsapi_revision: revision = %ld\n", r.revision);
      int rev = int(r.revision);
      if (rev < 1000) {
        // revision is e.g. 9.19 -> 919
        return { int(r.revision)/100, int(r.revision)%100 };
      } else {
        // revision is e.g. 9540 -> 9.54.0
        return { int(r.revision)/1000, (int(r.revision)%1000)/10 };
      }
    }
    // if this failed, continue alternative way below -->
  }
#endif
  binary_data out_buf;

  run_gs( {"--version"}, capture_stdout_data{&out_buf} );

  std::string out{out_buf.begin(), out_buf.end()};

  std::regex rx_ver("^(\\d+)[.](\\d+)");

  std::smatch ver_match;
  bool ok = std::regex_search(out, ver_match, rx_ver);
  if (!ok) {
    throw std::runtime_error("Couldn't parse gs version output: " + out);
  }
  return{ std::stoi(ver_match[1].str()), std::stoi(ver_match[2].str()) };
}



namespace detail {

inline std::pair<std::string::const_iterator,std::string::const_iterator>
get_gs_help_section(const std::string & out, const char * sec_name)
{
  std::regex rx{
    std::string("[\\s\\n]+(") + sec_name + "):\\s*(" // e.g. "^Available devices: "
    "(?:.|(?:\\n|\\r|\\r\\n)\\s+)*" // content is any non-newline char (.) or a
                                    // newline followed by space
    ")"
    ,
    std::regex::ECMAScript | std::regex::icase
  };

  std::smatch m;
  if ( ! std::regex_search(out, m, rx) ) {
    throw std::runtime_error(
        std::string("Couldn't parse output (") + sec_name + ") of gs --help :\n"
        + out
        );
  }

  // m[0] is full match
  // m[1] is the section heading ("Available devices")
  // m[2] is the section contents (including newlines and additional space etc.)

  return{ m[2].first, m[2].second };
}

} // namespace detail




// static
_KLFENGINE_INLINE
ghostscript_interface::gs_info_t ghostscript_interface::get_gs_info()
{
  using namespace klfengine::detail::utils;

  binary_data out_buf;

  run_gs( {"--help"}, capture_stdout_data{&out_buf} );

  std::string out{out_buf.begin(), out_buf.end()};

  constexpr auto rxflg = std::regex::ECMAScript | std::regex::icase;

  std::smatch usage_match;
  if ( ! std::regex_search(out, usage_match, std::regex{"[\\s\\n]+Usage:", rxflg}) ) {
    throw std::runtime_error("Couldn't parse output of gs --help :\n" + out);
  }

  // const cast needed because string constructor needs both iterator
  // arguments to have same "constness" -->
  std::string head{
    const_cast<const std::string&>(out).begin(),
    usage_match[0].first
  };

  auto avail_devices_it = detail::get_gs_help_section(out, "Available devices");

  std::vector<std::string> devices = str_split_rx(
      avail_devices_it.first, avail_devices_it.second,
      std::regex("[\\s\\r\\n]+"),
      true // skip empty items
      );

  auto search_path_it = detail::get_gs_help_section(out, "Search path");

  std::vector<std::string> search_paths = str_split_rx(
      search_path_it.first, search_path_it.second,
      std::regex("[\\s\\r\\n]+:[\\s\\r\\n]+"),
      true // skip empty items
      );

  return{ std::move(head), std::move(devices), std::move(search_paths) };
}

_KLFENGINE_INLINE
ghostscript_interface::gs_version_and_info_t ghostscript_interface::get_gs_version_and_info()
{
  // I'm a bit wary of parsing the version information from the --help heading,
  // because the heading might change.  In contrast --version is specifically
  // for this purpose so we should rely on that.  The second process call
  // shouldn't add a big overhead, especially since --version should return
  // really quickly.
  return { get_gs_version(), get_gs_info() };
}




_KLFENGINE_INLINE
void ghostscript_interface::impl_run_gs(
  std::vector<std::string> gs_args,
  const binary_data * stdin_data,
  bool add_standard_batch_flags,
  binary_data * capture_stdout,
  binary_data * capture_stderr
)
{
  switch (d->method) {
  case method::None: {
     throw std::runtime_error("Can't run ghostscript, method was set to ‘None’");
     return;
  }
  case method::Process: {
    d->impl_run_gs_process(std::move(gs_args), stdin_data, add_standard_batch_flags,
                           capture_stdout, capture_stderr);
    return;
  }
  case method::LinkedLibgs: {
    d->impl_run_gs_linkedlibgs(std::move(gs_args), stdin_data, add_standard_batch_flags,
                               capture_stdout, capture_stderr);
    return;
  }
  case method::LoadLibgs: {
    d->impl_run_gs_loadlibgs(std::move(gs_args), stdin_data, add_standard_batch_flags,
                             capture_stdout, capture_stderr);
    return;
  }
  default: {
    throw std::runtime_error("Can't run ghostscript, invalid method ‘"
                             + std::to_string(int(d->method)) + "’");
  }
  } // switch
}


// -------------------------------------
// tool to add standard batch flags, etc.
// -------------------------------------
inline
std::vector<std::string>
ghostscript_interface_private::construct_gs_argv(
  std::string argv0,
  std::vector<std::string> gs_args,
  bool add_standard_batch_flags
)
{
  if (add_standard_batch_flags) {
    std::vector<std::string> init_args{
      argv0,
      "-dNOPAUSE",
      "-dBATCH",
      "-dSAFER",
      "-q"
    };
    gs_args.insert(gs_args.begin(), init_args.begin(), init_args.end());
  } else {
    gs_args.insert(gs_args.begin(), argv0);
  }

  return gs_args;
}

// -------------------------------------
// run_gs - "process" method
// -------------------------------------

inline
void ghostscript_interface_private::impl_run_gs_process(
  std::vector<std::string> gs_args,
  const binary_data * stdin_data,
  bool add_standard_batch_flags,
  binary_data * capture_stdout,
  binary_data * capture_stderr
)
{
  if ( ! fs::exists(gs_path) ) {
    throw std::runtime_error("Invalid gs path: " + gs_path) ;
  }

  std::vector<std::string> gs_argv = construct_gs_argv(
    gs_path,
    std::move(gs_args),
    add_standard_batch_flags
  );

  try {
    process::run_and_wait(
      gs_argv,
      process::send_stdin_data{stdin_data},
      process::capture_stdout_data{capture_stdout},
      process::capture_stderr_data{capture_stderr}
    );
  } catch (process_exit_error & e) {
    throw ghostscript_error(e.what());
  }
}


// -------------------------------------


#if defined(KLFENGINE_USE_LINKED_GHOSTSCRIPT) || defined(KLFENGINE_USE_LOAD_GHOSTSCRIPT)

namespace detail {

// callbacks for gs' input and output
struct GhostscriptCallbacks {
  GhostscriptCallbacks(
    const binary_data * stdin_data_ptr,
    binary_data * stdout_data_ptr,
    binary_data * stderr_data_ptr
  )
    : _stdin_data_ptr(stdin_data_ptr),
      _stdout_data_ptr(stdout_data_ptr),
      _stderr_data_ptr(stderr_data_ptr),
      in_pos(0), out_pos(0), err_pos(0)
  {}
  const binary_data * _stdin_data_ptr;
  binary_data * _stdout_data_ptr;
  binary_data * _stderr_data_ptr;

  std::size_t in_pos, out_pos, err_pos;

  int handle_stdin(char * buf, int len)
  {
    if (_stdin_data_ptr == nullptr) {
      return 0; // no data to read
    }
    if (len < 0) {
      return -1;
    }
    std::size_t slen{ static_cast<size_t>(len) };
    if (in_pos >= _stdin_data_ptr->size()) {
      // past end of stream
      return 0;
    }
    if (in_pos + slen < _stdin_data_ptr->size()) {
      // there are at least len bytes available to read
      std::copy( _stdin_data_ptr->begin()+in_pos, _stdin_data_ptr->begin()+(in_pos+slen),
                 buf );
      in_pos += slen;
      return len;
    }
    // there are fewer than len bytes left to be read.  Read them all.
    int numread = _stdin_data_ptr->size() - in_pos;
    std::copy( _stdin_data_ptr->begin()+in_pos, _stdin_data_ptr->end(), buf );
    in_pos = _stdin_data_ptr->size();
    return numread;
  }

  int handle_stdout(const char * buf, int len)
  {
    if (len < 0) {
      return 0;
    }
    if (_stdout_data_ptr == nullptr) {
      return len; // ignore data
    }
    _stdout_data_ptr->insert(_stdout_data_ptr->end(), buf, buf+len);
    return len;
  }
  int handle_stderr(const char * buf, int len)
  {
    if (len < 0) {
      return 0;
    }
    if (_stderr_data_ptr == nullptr) {
      return len; // ignore data
    }
    _stderr_data_ptr->insert(_stderr_data_ptr->end(), buf, buf+len);
    return len;
  }
};

static int _klfengine_gs_callback_stdin_fn(void * caller_handle, char * buf, int len)
{
  //fprintf(stderr, "STDIN CALLBACK!!! len=%d\n", len);
  return reinterpret_cast<GhostscriptCallbacks*>(caller_handle)->handle_stdin(buf, len);
  // (void)caller_handle; (void)buf; return 0;
}
static int _klfengine_gs_callback_stdout_fn(void * caller_handle, const char * buf, int len)
{
  //fprintf(stderr, "STDOUT CALLBACK!!! len=%d\n", len);
  return reinterpret_cast<GhostscriptCallbacks*>(caller_handle)->handle_stdout(buf, len);
  //(void)caller_handle; (void)buf; return 0;
}
static int _klfengine_gs_callback_stderr_fn(void * caller_handle, const char * buf, int len)
{
  //fprintf(stderr, "STDERR CALLBACK!!! len=%d\n", len);
  return reinterpret_cast<GhostscriptCallbacks*>(caller_handle)->handle_stderr(buf, len);
  //(void)caller_handle; (void)buf; return 0;
}

} // namespace detail

#endif

// -------------------------------------
// run_gs - "linked-libgs" method
// -------------------------------------

inline
void ghostscript_interface_private::impl_run_gs_linkedlibgs(
  std::vector<std::string> gs_args,
  const binary_data * stdin_data,
  bool add_standard_batch_flags,
  binary_data * capture_stdout,
  binary_data * capture_stderr
)
{
#if defined(KLFENGINE_USE_LINKED_GHOSTSCRIPT)
  // prepare the terrain

  //  - first, warn the user if we happen to notice that they are using
  //    ghostscript's stdout for device output.  It looks like there is no way
  //    for us to capture it and that ghostscript always sends it to the process
  //    stdout.
  if (
      std::find_if( gs_args.begin(), gs_args.end(), [](const std::string & s) {
        return (s == "-sOutputFile=-") || (s == "-sOUTPUTFILE=-") || (s == "-o-");
      } )
      != gs_args.end() ) {
    warn("klfengine::ghostscript_interface",
         "It looks ghostscript device output is stdout. There is no way for "
         "us to capture this output with a libgs-based method. Please switch "
         "to the 'process' method or change your ghostscript to write to a "
         "temporary file.");
  }


  //  - prepare argc & argv
  std::vector<std::string> gs_argv = construct_gs_argv(
    "gs", // dummy
    std::move(gs_args),
    add_standard_batch_flags
  );

  std::vector<char*> gs_cstrings;
  gs_cstrings.reserve(gs_argv.size());
  for (const auto & s : gs_argv) {
    gs_cstrings.push_back( const_cast<char*>(s.c_str()) );
  }

  //  - prepare stdio callbacks
  detail::GhostscriptCallbacks gs_cb{stdin_data, capture_stdout, capture_stderr};

  // see Example 1 at https://ghostscript.com/doc/current/API.htm#Example_usage

  void *gs_minst = NULL;
  int gs_ret_code = 0;
  int gs_ret_code_2 = 0;
  
  // second argument is our "custom callback handle"
  gs_ret_code = gsapi_new_instance(&gs_minst, reinterpret_cast<void*>(&gs_cb));
  if (gs_ret_code < 0) {
    throw std::runtime_error{"Failed to create ghostscript instance, code = "
                             + std::to_string(gs_ret_code)};
  }

  // gs stdio callbacks
  gsapi_set_stdio(
    gs_minst,
    detail::_klfengine_gs_callback_stdin_fn,
    detail::_klfengine_gs_callback_stdout_fn,
    detail::_klfengine_gs_callback_stderr_fn
  );

  //fprintf(stderr, "CALLBACKS HAVE BEEN SET!\n");

  gs_ret_code = gsapi_set_arg_encoding(gs_minst, GS_ARG_ENCODING_UTF8);
  if (gs_ret_code == 0) {
    gs_ret_code = gsapi_init_with_args(gs_minst, gs_cstrings.size(), gs_cstrings.data());
  }
  gs_ret_code_2 = gsapi_exit(gs_minst);
  if ((gs_ret_code == 0) || (gs_ret_code == gs_error_Quit) || (gs_ret_code == gs_error_Info)) {
    gs_ret_code = gs_ret_code_2;
  }

  gsapi_delete_instance(gs_minst);

  if ((gs_ret_code == 0) || (gs_ret_code == gs_error_Quit) || (gs_ret_code == gs_error_Info)) {
    // all ok
    return;
  }

  // error
  throw ghostscript_error{"Ghostscript error! code = " + std::to_string(gs_ret_code)};

#else

  throw std::runtime_error("GS method 'LinkedLibgs' is not available because it "
                           "was not enabled during compilation.");

#endif
}


// -------------------------------------
// run_gs - "load-libgs" method
// -------------------------------------

inline
void ghostscript_interface_private::impl_run_gs_loadlibgs(
  std::vector<std::string> ,//gs_argv,
  const binary_data * ,//stdin_data,
  bool ,//add_standard_batch_flags,
  binary_data * ,//capture_stdout,
  binary_data * //capture_stderr
)
{

  throw std::runtime_error("Can't run ghostscript, method LoadLibgs not yet implemented.");

  // WRITE ME !

}



// -----------------------------------------------------------------------------


_KLFENGINE_INLINE
ghostscript_interface_engine_tool::ghostscript_interface_engine_tool()
{
}

_KLFENGINE_INLINE
void ghostscript_interface_engine_tool::set_settings(const settings & settings)
{
  struct gs_sett_pair_t {
    ghostscript_interface::method method;
    const std::string * gs_path_ptr;
  };

  gs_sett_pair_t cursett;
  if (_gs_interface) {
    cursett = gs_sett_pair_t{_gs_interface->gs_method(), & _gs_interface->gs_path()};
  }

  const std::string emptystr;
  gs_sett_pair_t newsett{ghostscript_interface::parse_method(settings.gs_method), &emptystr};
  if (newsett.method == ghostscript_interface::method::Process) {
    newsett.gs_path_ptr = & settings.gs_executable_path;
  } else if (newsett.method == ghostscript_interface::method::LoadLibgs) {
    newsett.gs_path_ptr = & settings.gs_libgs_path;
  }

  if (_gs_interface) {
    if (cursett.method == newsett.method) {
      if (cursett.method == ghostscript_interface::method::Process) {
        if (settings.gs_executable_path == *cursett.gs_path_ptr) {
          return; // no changes to gs method / path
        }
      } else if (cursett.method == ghostscript_interface::method::LoadLibgs) {
        if (settings.gs_libgs_path == *cursett.gs_path_ptr) {
          return; // no changes to gs method / path
        }
      } else {
        // current method is neither Process nor LoadLibgs, and it isn't to be changed
        return;
      }
    }
  }

  // changes, need to create new ghostscript_interface object.  Being a
  // std::unique_ptr, this will delete any old instance, if any.
  _gs_interface = std::unique_ptr<ghostscript_interface>{
    new ghostscript_interface{newsett.method, *newsett.gs_path_ptr}
  };

  _gs_version_and_info = _gs_interface->get_gs_version_and_info();
}





// =============================================================================



_KLFENGINE_INLINE
std::vector<klfengine::format_description>
gs_device_args_format_provider::impl_available_formats()
{
  // parameter specification = 
  // {
  //   "type": "bool"|"int"|"double"|"string"|"array"|"dict"|"color"|"length"|"margins",
  //   "null_ok": true|false,
  //   "validate_regex": <regex>, // applicable to bool,int,double,string
  //   "default": <default value>
  //   "dict_keys": { ... } // recursive
  // }
  value::dict vector_format_spec{
    {"outline_fonts", value{value::dict{
        {"type", value{std::string{"bool"}}},
        {"default", dict_get(_param_defaults, "outline_fonts", value{true})}
      }}}
  };
  value::dict raster_format_spec{
    {"dpi", value{value::dict{
        {"type", value{std::string{"int"}}},
        {"default", dict_get(_param_defaults, "dpi", value{600})}
      }}},
    {"antialiasing", value{value::dict{
        {"type", value{std::string{"bool|dict"}}},
        {"default", dict_get(_param_defaults, "antialiasing", value{true})},
        {"dict_keys", value{value::dict{
            {"text_alpha_bits", value{value::dict{
                {"type", value{std::string{"int"}}},
                {"validate_regex", value{std::string{"^1|2|4$"}}}
              }}},
            {"graphics_alpha_bits", value{value::dict{
                {"type", value{std::string{"int"}}},
                {"validate_regex", value{std::string{"^1|2|4$"}}}
              }}}
          }}}
      }}}
  };
  value::dict png_format_spec{ raster_format_spec };
  png_format_spec["transparency"] = value{value::dict{
    {"type", value{std::string{"bool"}}},
    {"default", dict_get(_param_defaults, "transparency", value{true})}
  }};

  // none of the devices used seem to be devices that aren't always installed,
  // so there doesn't appear to be much use to check which GS devices are
  // actually available.
  return std::vector<klfengine::format_description>{
    {
      {"PNG", png_format_spec},
      "PNG Image",
      "Portable Network Graphics Image, widely compatible raster "
      "image format with transparency"
    },
    {
      {"PDF", vector_format_spec},
      "PDF Document",
      "Portable Document Format, a widely compatible vector graphics format"
    },
    {
      {"JPEG", raster_format_spec},
      "JPEG Image",
      "Standard JPEG Image (does not have transparency)"
    },
    {
      {"PS", vector_format_spec},
      "PostScript (PS) Document",
      "Vector PostScript Drawing Document"
    },
    {
      {"EPS", vector_format_spec},
      "Encapsulated PostScript (EPS) Document",
      "Vector Encapsulated PostScript Drawing Document"
    },
    {
      {"TIFF", raster_format_spec},
      "TIFF Image",
      "Standard uncompressed TIFF Image (does not have transparency)"
    },
    {
      {"BMP", raster_format_spec},
      "BMP Image",
      "Uncompressed BMP Image (does not have transparency)"
    },
  };
}

_KLFENGINE_INLINE
format_spec gs_device_args_format_provider::impl_make_canonical(
  const format_spec & format,
  bool // check_available_only
)
{
  parameter_taker param{
    format.parameters,
    "klfengine::ghostscript_interface::gs_args_canonical_format_parameters"
  };

  if (format.format == "PDF" || format.format == "PS" || format.format == "EPS") {

    format_spec f{format.format, {}};

    bool outline_fonts = dict_get<bool>(_param_defaults, "outline_fonts", true);
    f.parameters["outline_fonts"] = value{param.take<bool>("outline_fonts", outline_fonts)};

    param.finished();
    return f;
  }

  if (format.format == "PNG" || format.format == "JPEG" || format.format == "TIFF"
      || format.format == "BMP") {

    format_spec f{format.format, {}};
    
    if (format.format == "PNG") {
      bool transparency = dict_get<bool>(_param_defaults, "transparency", true);
      transparency = param.take("transparency", transparency);
      f.parameters["transparency"] = value{transparency};
    }

    int dpi = dict_get<int>(_param_defaults, "dpi", true);
    f.parameters["dpi"] = value{param.take<int>("dpi", dpi)};
    
    value::dict aadic;
    value antialiasing = dict_get<value>(_param_defaults, "antialiasing", value{true});
    antialiasing = param.take("antialiasing", antialiasing);
    if (antialiasing.has_type<bool>()) {
      if (antialiasing.get<bool>()) {
        aadic["graphics_alpha_bits"] = value{4};
        aadic["text_alpha_bits"] = value{4};
      } else {
        // no antialiasing
        aadic["graphics_alpha_bits"] = value{1};
        aadic["text_alpha_bits"] = value{1};
      }
    } else if (antialiasing.has_type<value::dict>()) {
      parameter_taker paa{
        antialiasing.get<value::dict>(),
        "klfengine::ghostscript_interface::gs_args_set_device_for_format (antialiasing)"
      };
      aadic["graphics_alpha_bits"] = value{paa.take("graphics_alpha_bits", 4)};
      aadic["text_alpha_bits"] = value{paa.take("text_alpha_bits", 4)};
      paa.finished();
    } else {
      param.disable_check();
      throw invalid_parameter{param.what(), "invalid value for antialiasing="};
    }
    f.parameters["antialiasing"] = value{aadic};
    
    param.finished();
    return f;
  }

  param.disable_check();

  // no such format
  return format_spec{};
}

_KLFENGINE_INLINE
std::vector<std::string>
gs_device_args_format_provider::get_device_args_for_format( const format_spec & fmt )
{
  format_spec format = canonical_format(fmt);

  std::vector<std::string> gs_args;

  parameter_taker param{
    format.parameters,
    "klfengine::ghostscript_interface::gs_args_set_device_for_format"
  };
  param.disable_check();

  bool is_vector_format = true;

  // choose correct device
  if (format.format == "PNG") {
    is_vector_format = false;
    bool transparency = param.take<bool>("transparency");
    if (transparency) {
      gs_args.push_back("-sDEVICE=pngalpha");
      // gs starts rendering transparency poorly in larger images without the
      // following option -- https://stackoverflow.com/a/4907328/1694896
      gs_args.push_back("-dMaxBitmap=2147483647");
    } else {
      gs_args.push_back("-sDEVICE=png16m");
    }
  } else if (format.format == "JPEG") {
    is_vector_format = false;
    gs_args.push_back("-sDEVICE=jpeg");
    // TODO : parameter to set JPEG quality
  } else if (format.format == "TIFF") {
    is_vector_format = false;
    gs_args.push_back("-sDEVICE=tiff24nc");
  } else if (format.format == "BMP") {
    is_vector_format = false;
    gs_args.push_back("-sDEVICE=bmp16m");
  } else if (format.format == "PDF") {
    is_vector_format = true;
    gs_args.push_back("-sDEVICE=pdfwrite");
  } else if (format.format == "PS") {
    is_vector_format = true;
    gs_args.push_back("-sDEVICE=ps2write");
  } else if (format.format == "EPS") {
    is_vector_format = true;
    gs_args.push_back("-sDEVICE=eps2write");
  } else {
    throw std::invalid_argument{"Cannot produce Ghostscript flags for format "+format.format};
  }

  // outline fonts, if applicable
  if (is_vector_format) {
    bool outline_fonts = param.take<bool>("outline_fonts");
    if (outline_fonts) {
      auto gs_ver = _gs_iface_tool->gs_version();
      if (gs_ver.major < 9 || (gs_ver.major == 9 && gs_ver.minor < 15)) {
        error(
          "klfengine::ghostscript_interface::gs_args_set_device_for_format",
          "Requested outline_fonts=true, but "
          "you have ghostscript v" + std::to_string(gs_ver.major) + "."
          + std::to_string(gs_ver.minor) + ".  Please upgrade to gs>=9.15 for font outlines."
        );
      } else {
        gs_args.push_back("-dNoOutputFonts");
      }
    }
  }

  // dpi and anti-aliasing
  if ( ! is_vector_format ) {

    int dpi = param.take<int>("dpi");
    gs_args.push_back("-r" + std::to_string(dpi));

    value::dict antialiasing_dic = param.take<value::dict>("antialiasing");

    const int graphics_alpha_bits = dict_get<int>(antialiasing_dic, "graphics_alpha_bits");
    const int text_alpha_bits = dict_get<int>(antialiasing_dic, "text_alpha_bits");

    gs_args.push_back("-dGraphicsAlphaBits="+std::to_string(graphics_alpha_bits));
    gs_args.push_back("-dTextAlphaBits="+std::to_string(text_alpha_bits));

  }

  return gs_args;
}



} // namespace klfengine
