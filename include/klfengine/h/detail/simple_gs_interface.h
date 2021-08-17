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
#include <klfengine/settings>
#include <klfengine/process>
#include <klfengine/format_spec>

namespace klfengine {

namespace detail {

struct simple_gs_interface_private;

struct ghostscript_error : exception
{
  ghostscript_error(std::string msg);
  virtual ~ghostscript_error();
};

/**
 * \internal
 *
 * For now, this class is internal API because I'm not sure how I want to
 * structure it, and we can expect major API changes to this at any point.
 */
class simple_gs_interface
{
public:
  enum struct method {
    None,
    Process,
    LinkedLibgs,
    LoadLibgs
  };
  static method parse_method(const std::string & method_s);

  // constructors
  /**
   * The meaning of \a gs_path depends on the method.  If the method is \ref
   * method::Process, then the path should be the path to the \c gs executable.
   * If the method is \ref method::Loadlibgs, then \a gs_path is the path to the
   * \c "libgs.so" library.
   */
  explicit simple_gs_interface(method method_, std::string gs_path = std::string());
  explicit simple_gs_interface(std::string method_s, std::string gs_path = std::string());
  ~simple_gs_interface();

  method gs_method() const;
  const std::string & gs_path() const;

  // gs information

  struct gs_info_t {
    std::string head;
    std::vector<std::string> devices;
    std::vector<std::string> search_path;
  };

  struct gs_version_t { int major; int minor; };
  struct gs_version_and_info_t { gs_version_t version; gs_info_t info; };

  gs_version_t get_gs_version();

  gs_info_t get_gs_info();

  gs_version_and_info_t get_gs_version_and_info();

  //

  struct add_standard_batch_flags {
    bool _add_flags;
  };

  using send_stdin_data = klfengine::process::send_stdin_data;
  using capture_stderr_data = klfengine::process::capture_stderr_data;
  using capture_stdout_data = klfengine::process::capture_stdout_data;

  template<typename... Args>
  void run_gs(std::vector<std::string> gs_args, Args && ... args)
  {
    using namespace detail::utils;

    bool add_standard_batch_flags_yn = true;
    if (kwargs<Args...>::template has_arg<add_standard_batch_flags>::value) {
      add_standard_batch_flags d{
        kwargs<Args...>::template take_arg<add_standard_batch_flags>(args...)
      };
      add_standard_batch_flags_yn = d._add_flags;
    }

    // always capture stdout and stderr for error messages
    binary_data _default_stderr_buf;
    binary_data _default_stdout_buf;

    binary_data * capture_stderr_bufptr = & _default_stderr_buf;
    if (kwargs<Args...>::template has_arg<capture_stderr_data>::value) {
      capture_stderr_data d{
        kwargs<Args...>::template take_arg<capture_stderr_data>(args...)
      };
      capture_stderr_bufptr = d._data_ptr;
    }

    binary_data * capture_stdout_bufptr = & _default_stdout_buf;
    if (kwargs<Args...>::template has_arg<capture_stdout_data>::value) {
      capture_stdout_data d{
        kwargs<Args...>::template take_arg<capture_stdout_data>(args...)
      };
      capture_stdout_bufptr = d._data_ptr;
    }

    binary_data _default_stdin_data{};
    const binary_data * stdin_data_bufptr = & _default_stdin_data;
    if (kwargs<Args...>::template has_arg<send_stdin_data>::value) {
      send_stdin_data d{
        kwargs<Args...>::template take_arg<send_stdin_data>(args...)
      };
      stdin_data_bufptr = d._data_ptr;
    }


    impl_run_gs(std::move(gs_args),
                stdin_data_bufptr,
                add_standard_batch_flags_yn,
                capture_stdout_bufptr,
                capture_stderr_bufptr);
  }

private:
  simple_gs_interface_private *d;

  void impl_run_gs(
    std::vector<std::string> gs_args,
    const binary_data * stdin_data,
    bool add_standard_batch_flags,
    binary_data * capture_stdout,
    binary_data * capture_stderr
  );
};







class simple_gs_interface_engine_tool
{
public:
  simple_gs_interface_engine_tool();

  void set_settings(const settings & settings);

  simple_gs_interface * gs_interface() { return _gs_interface.get(); }

  inline simple_gs_interface::gs_version_t
  gs_version() const { return _gs_version_and_info.version; };

  inline simple_gs_interface::gs_info_t
  gs_info() const { return _gs_version_and_info.info; };

  inline simple_gs_interface::gs_version_and_info_t
  gs_version_and_info() const { return _gs_version_and_info; };

private:
  std::unique_ptr<simple_gs_interface> _gs_interface;
  simple_gs_interface::gs_version_and_info_t _gs_version_and_info;
};



// -------------------------------------


/** \brief Construct Ghostscript arguments for multiple vector and image formats
 *
 *
 * Here are the possible formats that we can produce Ghostscript flags for.  For
 * each format, we look for parameters that can influence Ghostscript's output
 * as documented below:
 *
 * Raster Formats: "PNG", "JPEG", "BMP", "TIFF".  Accepted parameters:
 * \code
 * {
 *   "dpi": <int>,
 *   "antialiasing": true|false|{"TextBits": 1|2|4, "GraphicsAlphaBits": 1|2|4}
 * }
 * \endcode
 *
 * Vector Formats: "PDF", "PS", "EPS".  Accepted parameters:
 * \code
 * {
 *   "outline_fonts": true|false
 * }
 * \endcode
 *
 */
class gs_device_args_format_provider : public format_provider
{
public:
  gs_device_args_format_provider(
      simple_gs_interface_engine_tool * gs_iface_tool_,
      value::dict param_defaults_
  )
    : _gs_iface_tool(gs_iface_tool_), _param_defaults(std::move(param_defaults_)) { }

  /** \brief Set -sDEVICE=... etc. for the required format spec.
   *
   * Returns a list of switches to include towards the beginning of your
   * Ghostscript argument list in order to produce output in the desired format.
   *
   */
  std::vector<std::string> get_device_args_for_format(
      const format_spec & format
  );

private:
  virtual std::vector<format_description> impl_available_formats();
  virtual format_spec impl_make_canonical(const format_spec & format,
                                          bool check_available_only);

  simple_gs_interface_engine_tool * _gs_iface_tool;
  value::dict _param_defaults;
};





} // namespace detail

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/detail/simple_gs_interface.hxx>
#endif
