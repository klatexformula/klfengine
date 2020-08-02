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
#include <string>

#include <iostream> // DEBUG

#include <klfengine/h/detail/simple_gs_interface.h>

#include <klfengine/process>

//#include <subprocess/subprocess.hpp>

namespace klfengine {

namespace detail {


_KLFENGINE_INLINE
simple_gs_interface::simple_gs_interface(method method_, std::string gs_path)
  : _method(method_), _gs_path(std::move(gs_path))
{
  _init();
}

_KLFENGINE_INLINE
simple_gs_interface::simple_gs_interface(std::string method_s, std::string gs_path)
  : _method(get_method(method_s)), _gs_path(std::move(gs_path))
{
  _init();
}

void simple_gs_interface::_init()
{
  if (_method != method::Process) {
    throw std::runtime_error("Only 'method::Process' method is supported for now.") ;
  }

  if ( ! fs::exists(_gs_path) ) {
    throw std::runtime_error("Invalid gs path: " + _gs_path) ;
  }
}


// static
_KLFENGINE_INLINE
simple_gs_interface::method
simple_gs_interface::get_method(const std::string & method_s)
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
std::pair<int,int> simple_gs_interface::gs_version()
{
  binary_data out_buf;
  process::run( {_gs_path, "--version"},
                process::capture_stdout_data{out_buf} );
  std::string out{out_buf.begin(), out_buf.end()};

  std::regex rx_ver("^(\\d+)[.](\\d+)");

  std::smatch ver_match;
  bool ok = std::regex_search(out, ver_match, rx_ver);
  if (!ok) {
    throw std::runtime_error("Couldn't parse gs version output: " + out);
  }
  return{ std::stoi(ver_match[1].str()), std::stoi(ver_match[2].str()) };
}

//namespace detail { // already in namespace detail
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
std::vector<std::string> str_split_rx(
    std::string::const_iterator a, std::string::const_iterator b,
    const std::regex & rx_sep, bool skip_empty = false
    )
{
  std::vector<std::string> items;
  auto last_sep_end = a;

  for (std::sregex_iterator it = std::sregex_iterator(a, b, rx_sep);
       it != std::sregex_iterator{}; ++it) {
    // iterating over matches of the separator regex
    std::smatch sep_match = *it;
    if (!skip_empty || sep_match[0].first != last_sep_end) {
      items.push_back(std::string{last_sep_end,sep_match[0].first});
    }
    last_sep_end = sep_match[0].second;
  }
  if (!skip_empty || last_sep_end != b) {
    items.push_back(std::string{last_sep_end,b});
  }
  return items;
}
//} // namespace detail


// static
_KLFENGINE_INLINE
simple_gs_interface::gs_info_t simple_gs_interface::gs_info()
{
  binary_data out_buf;
  process::run( {_gs_path, "--help"},
                process::capture_stdout_data{out_buf} );
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

// static
_KLFENGINE_INLINE
binary_data simple_gs_interface::run_gs(
    const std::vector<std::string> & gs_args,
    const binary_data & stdin_data,
    bool add_standard_batch_flags,
    std::string * set_stderr)
{
  std::vector<std::string> argv;
  argv.push_back(_gs_path);
  if (add_standard_batch_flags) {
    argv.push_back("-dNOPAUSE");
    argv.push_back("-dBATCH");
    argv.push_back("-dSAFER");
    argv.push_back("-q");
  }
  argv.insert(argv.end(), gs_args.begin(), gs_args.end());

  binary_data out;
  binary_data err;

  process::run(
      argv,
      process::send_stdin_data{stdin_data},
      process::capture_stdout_data{out},
      process::capture_stderr_data{err}
      );

  if (set_stderr != nullptr) {
    *set_stderr = std::string{err.begin(), err.end()};
  }

  return out;
}



} // namespace detail

} // namespace klfengine
