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

#include <klfengine/basedefs>
#include <klfengine/settings>
#include <klfengine/process>
#include <klfengine/format>

namespace klfengine {

struct ghostscript_interface_private;


/** \brief Represents an error reported by Ghostcript
 *
 */
class ghostscript_error : public klfengine::exception
{
public:
  ghostscript_error(std::string msg);
  virtual ~ghostscript_error();
};

/** \brief Simple interface for calling Ghostscript
 * 
 * Supports running Ghostscript as an external process (\c "gs"), via its C
 * library API (either linked at compile-time or by loading \c "libgs.so" at
 * run-time).  The way Ghostscript is invoked is specified as the \a method,
 * which can be either \a ghostscript_interface::method::Process (or "process"),
 * \a ghostscript_interface::method::LinkedLibgs (or "linked-libgs"), or \a
 * ghostscript_interface::method::LoadLibgs (or "load-libgs").
 *
 * Depending on the \a method, the corresponding specified \a gs_path should
 * either be the path to the \c "gs" executable (for the \a Process method) or
 * the path to the \c "libgs.so" library (for the \a LoadLibgs method).  The
 * argument \a gs_path is unused if the method is \a LinkedLibgs.
 *
 * The \a method and \a gs_path are fixed once and for all for the lifetime of
 * this class instance.  See \ref klfengine::ghostscript_interface_engine_tool
 * if you'd like to be able to change the settings of how to run Ghostscript.
 *
 * To actually run Ghostscript, use the \ref run_gs() method.
 *
 * \todo The "load-libgs" method is not yet implemented.
 */
class ghostscript_interface
{
public:
  enum struct method {
    None,
    Process,
    LinkedLibgs,
    LoadLibgs
  };
  /** \brief Return the method enum member associated to the given method name
   *
   * Parses the strings "none", "process", "linked-libgs", and "load-libgs" to
   * their corresponding \ref method enum members.
   */
  static method parse_method(const std::string & method_s);

  /** \brief Constructor, providing \a method and an associated \a gs_path
   */
  explicit ghostscript_interface(method method_, std::string gs_path = std::string());

  /** \brief Constructor, providing \a method and an associated \a gs_path
   *
   * This overload is provided for convenience and enables you to specify the
   * method as a string. See \ref parse_method().
   */
  explicit ghostscript_interface(std::string method_s, std::string gs_path = std::string());
  ~ghostscript_interface();

  /** \brief Returns the method being used to run Ghostscript
   */
  method gs_method() const;

  /** \brief Returns the gs_path being used to run Ghostscript
   *
   * The meaning of \a gs_path depends on the current method (\a gs_method()).
   * See the class documentation above for more details.
   */
  const std::string & gs_path() const;

  // gs information

  /** \brief Store information about Ghostscript
   *
   * This information is typically obtained by running <code>gs --help</code>.
   *
   * This struct includes information about which devices are available (\a
   * devices member).
   */
  struct gs_info_t {
    std::string head;
    std::vector<std::string> devices;
    std::vector<std::string> search_path;
  };

  /** \brief Store version information about Ghostscript
   *
   * This information is typically obtained by running <code>gs --version</code>.
   */
  struct gs_version_t { int major; int minor; };

  /** \brief Store general information and version information about Ghostscript
   *
   */
  struct gs_version_and_info_t {
    gs_version_t version;
    gs_info_t info;
  };

  /** \brief Run Ghostscript to obtain its version information
   *
   */
  gs_version_t get_gs_version();

  /** \brief Run Ghostscript to obtain its general information
   *
   */
  gs_info_t get_gs_info();

  /** \brief Run Ghostscript to obtain its general information and version information
   *
   */
  gs_version_and_info_t get_gs_version_and_info();

  //

  /** \brief Instruct run_gs() to add standard batch-interaction Ghoscript flags
   *
   * By including <code>add_standard_batch_flags{true}</code> or
   * <code>add_standard_batch_flags{false}</code> in a call to \ref run_gs(),
   * you instruct run_gs() to add (or not) the standard Ghostscript flags
   * <code>-q -dBATCH -dNOPAUSE -dSAFER</code> that are commonly used to
   * run Ghostscript for noninteractive processing of files.
   */
  struct add_standard_batch_flags {
    bool _add_flags;
  };

  /** \brief Instruct run_gs() to send data to Ghostscript's standard input
   *
   * Works like klfengine::process::send_stdin_data.
   */
  using send_stdin_data = klfengine::process::send_stdin_data;

  /** \brief Instruct run_gs() to capture Ghostscript's standard output
   *
   * Works like klfengine::process::capture_stdout_data.
   *
   * \warning When the \a LoadLibgs or \a LinkedLibgs methods are used, we
   *          cannot Ghostscript's PostScript-related output, which is always
   *          sent to the process stdout.  So you can't use "-sOutputFile=-" and
   *          recover the data with <code>capture_stdout_data{...}</code>.
   *          Instead, you can instruct Ghostscript to output to a temporary
   *          file with "-sOutputFile=..." and read that file once Ghostscript
   *          completes.
   */
  using capture_stdout_data = klfengine::process::capture_stdout_data;

  /** \brief Instruct run_gs() to capture Ghostscript's standard error output
   *
   * Works like klfengine::process::capture_stderr_data.
   */
  using capture_stderr_data = klfengine::process::capture_stderr_data;

#ifdef _KLFENGINE_PROCESSED_BY_DOXYGEN
  /** \brief Run Ghostscript with the given arguments and flags
   *
   * Run ghostscript with the given argument vector, as you'd run Ghostscript
   * from the command line.  Do NOT include "gs" or the executable name in the
   * first argument, you can directly start with Ghostscript flags like
   * "-sDEVICE=..." etc.
   *
   * Example:
   * \code
   *   klfengine::ghostscript_interface my_gs_interface{
   *     "process",
   *     "/usr/local/bin/gs"
   *   }; // alternatively, {"load-libgs", "/usr/lib/libgs.so"}, or etc.
   *
   *   std::vector<std::string> gs_args{
   *     "-sDEVICE=pngalpha",
   *     "-r120",
   *     "-sOutputFile=my_gs_output.png",
   *     "-"
   *   };
   *
   *   // prepare data to send to Ghostscript's standard input 
   *   klfengine::binary_data gs_input_data = ....;
   *
   *   klfengine::binary_data gs_stdout_data;
   *   klfengine::binary_data gs_stderr_data;
   *
   *   my_gs_interface.run_gs(
   *     gs_args,
   *     klfengine::ghostscript_interface::add_standard_batch_flags{true},
   *     klfengine::ghostscript_interface::send_stdin_data{gs_input_data},
   *     klfengine::ghostscript_interface::capture_stdout_data{&gs_stdout_data},
   *     klfengine::ghostscript_interface::capture_stderr_data{&gs_stderr_data}
   *   );
   *   
   *   // read output from file `my_gs_output.png`
   * \endcode
   *
   * By default, or if you pass the modifier argument
   * <code>add_standard_batch_flags{true}</code>, then the flags <code>-q
   * -dBATCH -dSAFER -dNOPAUSE</code> are automatically included.  Use
   * <code>add_standard_batch_flags{false}</code> to inhibit the addition of
   * these flags.
   *
   * If ghostscript exists with an error, then a \ref
   * klfengine::ghostscript_error exception is thrown.
   *
   * \warning If you are using the "load-libgs" or "linked-libgs" methods, we
   *          cannot capture Ghostscript's postscript-related standard output,
   *          as the latter is always sent to the process' stdout.  Instead, you
   *          can instruct Ghostscript to output to a temporary file with
   *          "-sOutputFile=xxx".
   */
  void run_gs(std::vector<std::string> gs_args,
              GhostscriptRunManipArg arg0,
              GhostscriptRunManipArg arg1, ...)
  {}
#else
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
#endif // _KLFENGINE_PROCESSED_BY_DOXYGEN

private:
  ghostscript_interface_private *d;

  void impl_run_gs(
    std::vector<std::string> gs_args,
    const binary_data * stdin_data,
    bool add_standard_batch_flags,
    binary_data * capture_stdout,
    binary_data * capture_stderr
  );
};






/** \brief An "engine tool" for handling ghostscript_interface objects
 *
 * Manages a \ref klfengine::ghostscript_interface instance and re-creates a new
 * instance whenever you'd like to change how Ghostscript is run.  The way
 * Ghostscript is invoked is read from a \ref klfengine::settings object.
 *
 * This class also queries once the version information and general information
 * and stores this information for later retrieval in gs_version(), gs_info(),
 * and gs_version_and_info() without having to run Ghostscript again.  This
 * information is updated when the settings are changed.
 *
 * \bug DESIGN DECISION NEEDED: It might happen that one thread changes the
 *      settings (new gs_interface gets created) while another thread is using
 *      the gs_interface?  Should the ghostscript_interface_engine_tool have a
 *      mutex lock (but then how to know when run_instance finished using the
 *      gs_interface() object?)
 */
class ghostscript_interface_engine_tool
{
public:
  ghostscript_interface_engine_tool();

  /** \brief 
   *
   */
  void set_settings(const settings & settings);

  ghostscript_interface * gs_interface() { return _gs_interface.get(); }

  inline ghostscript_interface::gs_version_t
  gs_version() const { return _gs_version_and_info.version; };

  inline ghostscript_interface::gs_info_t
  gs_info() const { return _gs_version_and_info.info; };

  inline ghostscript_interface::gs_version_and_info_t
  gs_version_and_info() const { return _gs_version_and_info; };

private:
  std::unique_ptr<ghostscript_interface> _gs_interface;
  ghostscript_interface::gs_version_and_info_t _gs_version_and_info;
};



// -------------------------------------



/** \brief Construct Ghostscript arguments for multiple vector and image formats
 *
 * This is a \ref klfengine::format_provider -derived class that provides the
 * command-line arguments you should specify to a Ghostscript call to set the
 * device appropriately to obtain the desired output given by a \ref
 * klfengine::format_spec object.
 *
 * You can query available formats with the (inherited) method \ref
 * klfengine::format_provider::available_formats() "available_formats()", you
 * can check whether a format is available with the (inherited) method \ref
 * klfengine::format_provider::has_format() "has_format()", and you can make
 * parameter dictionaries canonical with the (inherited) method \ref
 * klfengine::format_provider::canonical_format() "canonical_format()".
 *
 * This class handles format parameters such as "dpi" and "outline_fonts", with
 * defaults that can be specified to the constructor.
 *
 * Here are the possible formats that we can produce Ghostscript flags for.  For
 * each format, we look for parameters that can influence Ghostscript's output
 * as documented below:
 *
 * Raster Formats: "PNG", "JPEG", "BMP", "TIFF".  Accepted parameters:
 * \code
 * {
 *   "dpi": <int>,
 *   "antialiasing": true|false|{"text_bits": 1|2|4, "graphics_alpha_bits": 1|2|4}
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
      ghostscript_interface_engine_tool * gs_iface_tool_,
      value::dict param_defaults_
  )
    : _gs_iface_tool(gs_iface_tool_), _param_defaults(std::move(param_defaults_)) { }

  /** \brief Get Ghostscript arguments to produce the required format
   *
   * Returns a list of switches to include towards the beginning of your
   * Ghostscript argument list in order to produce output in the desired format.
   * This includes "-sDEVICE=...", as well as options that set the DPI
   * resolution etc. as specified by the format spec \a format.
   */
  std::vector<std::string> get_device_args_for_format(
      const format_spec & format
  );

private:
  virtual std::vector<format_description> impl_available_formats();
  virtual format_spec impl_make_canonical(const format_spec & format,
                                          bool check_available_only);

  ghostscript_interface_engine_tool * _gs_iface_tool;
  value::dict _param_defaults;
};





} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/ghostscript_interface.hxx>
#endif
