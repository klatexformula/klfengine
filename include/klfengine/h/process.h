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

#include <stdexcept>
#include <type_traits>
#include <vector>
#include <string>


#include <klfengine/basedefs>


namespace klfengine {


struct process_exit_error : exception
{
  process_exit_error(std::string msg);
  virtual ~process_exit_error();
};


namespace detail {

// from https://en.cppreference.com/w/cpp/types/disjunction
template<class...> struct my_disjunction : std::false_type { };
template<class B1> struct my_disjunction<B1> : B1 { };
template<class B1, class... Bn>
struct my_disjunction<B1, Bn...> 
    : std::conditional<bool(B1::value), B1, my_disjunction<Bn...>>::type
{ };

// see https://stackoverflow.com/a/34099948/1694896
template<typename T, typename... Ts>
struct contains : my_disjunction<std::is_same<T, Ts>...> {};


template<typename T>
struct arg_name_to_str
{
  static inline std::string get() { return "<unknown>"; }
};

template<typename... A>
struct get_kwargs {
  template<typename T>
  using has_arg = contains<T, A...>;

  // warning: get_arg() can only be called ONCE for each type, because its
  // return value *has been std::move'ed*

  template<typename T, typename... Rest>
  static T && get_arg(T && t, Rest&&... ) {
    return t;
  }
  template<typename T, typename... Rest>
  static T && get_arg(T & t, Rest&&... ) {
    return std::move(t);
  }
  template<typename T, typename O, typename... Rest>
  static auto get_arg(O && , Rest&&... a) -> 
    typename std::enable_if<
      !std::is_same<T,typename std::remove_reference<O>::type>::value,
      T &&>::type
  {
    return get_arg<T>(std::forward<Rest>(a)...);
  }
  template<typename T>
  static T && get_arg()
  {
    throw std::invalid_argument(
        std::string("Expected argument ") + get_type_name<T>() + "{}"
        );
  }
};

} // namespace detail


namespace detail {

std::string suffix_out_and_err(const binary_data * out, const binary_data * err);

void run_process_impl(
    const std::string & executable,
    const std::vector<std::string> & argv,
    const std::string & run_cwd,
    const binary_data * stdin_data,
    binary_data * capture_stdout,
    binary_data * capture_stderr,
    int * capture_exit_code
    );

} // namespace detail


struct process {

  struct send_stdin_data {
    const binary_data & _data_ref;
  };
  struct executable {
    const std::string & _data_ref;
  };
  struct run_in_directory {
    const std::string & _data_ref;
  };
  struct capture_stdout_data {
    binary_data & _data_ref;
  };
  struct capture_stderr_data {
    binary_data & _data_ref;
  };
  //struct fetch_file {
  //  std::string _filename;
  //  std::string & _data_ref;
  //};
  //struct environment {
    //    ........
    // std::vector<std::string> e
  //};

  template<typename... Args>
  static void run(const std::vector<std::string> & argv,
                  Args && ... args)
  {
    using namespace detail;

    if (argv.empty()) {
      throw std::invalid_argument("klfengine::process::run(): cannot have empty argv");
    }

    std::string ex;

    if (get_kwargs<Args...>::template has_arg<executable>::value) {
      executable d{get_kwargs<Args...>::template get_arg<executable>(args...)};
      ex = d._data_ref;
    } else {
      ex = argv.front();
    }

    std::string run_cwd;

    if (get_kwargs<Args...>::template has_arg<run_in_directory>::value) {
      run_in_directory d{
        get_kwargs<Args...>::template get_arg<run_in_directory>(args...)
      };
      run_cwd = d._data_ref;
    }

    const binary_data * stdin_d = nullptr;

    if (get_kwargs<Args...>::template has_arg<send_stdin_data>::value) {
      send_stdin_data d{
        get_kwargs<Args...>::template get_arg<send_stdin_data>(args...)
      };
      stdin_d = & d._data_ref;
    }

    binary_data * capture_stdout = nullptr;

    if (get_kwargs<Args...>::template has_arg<capture_stdout_data>::value) {
      capture_stdout_data d{
        get_kwargs<Args...>::template get_arg<capture_stdout_data>(args...)
      };
      capture_stdout = & d._data_ref;
    }

    binary_data * capture_stderr = nullptr;

    if (get_kwargs<Args...>::template has_arg<capture_stderr_data>::value) {
      capture_stderr_data d{
        get_kwargs<Args...>::template get_arg<capture_stderr_data>(args...)
      };
      capture_stderr = & d._data_ref;
    }

    int exit_code = -1;

    run_process_impl(
        ex,
        argv,
        run_cwd,
        stdin_d,
        capture_stdout,
        capture_stderr,
        &exit_code
    );

    if (exit_code != 0) {
      throw process_exit_error{
          "Process " + ex + " exited with code " + std::to_string(exit_code)
          + suffix_out_and_err(capture_stdout, capture_stderr)
      };
    }

    // normal exit, ok.
  }
};




} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/process.hxx>
#endif
