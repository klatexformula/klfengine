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
#include <map>
//#include <iostream>

#include <klfengine/basedefs>
#include <klfengine/h/detail/filesystem.h>


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
    static_assert( ! get_kwargs<Rest...>::template has_arg<T>::value,
                   "You cannot specify the same keyword argument multiple times" );
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


/** \brief A mapping object of names with values, representing a process environment
 */
using environment = std::map<std::string, std::string>;

/** \brief Get the current process environment
 */
environment current_environment();
/** \brief Get an \ref klfengine::environment instance from an C-style environ pointer
 */
environment parse_environment(char ** env_ptr);

/** \brief Instruct \ref set_environment() to include variables if not already defined
 */
struct provide_environment_variables {
  environment variables;
};
/** \brief Instruct \ref set_environment() to set variables to new values
 */
struct set_environment_variables {
  environment variables;
};
/** \brief Instruct \ref set_environment() to remove some variables
 */
struct remove_environment_variables {
  std::vector<std::string> variable_names;
};
/** \brief Instruct \ref set_environment() to prepend paths to some variables
 */
struct prepend_path_environment_variables {
  environment variables;
};
/** \brief Instruct \ref set_environment() to append paths to some variables
 */
struct append_path_environment_variables {
  environment variables;
};

namespace detail {
inline void do_set_environment(environment & env, set_environment_variables && a) {
  for (environment::iterator it = a.variables.begin(); it != a.variables.end(); ++it) {
    // set this variable
    env[it->first] = std::move(it->second);
  }
}
inline void do_set_environment(environment & env, provide_environment_variables && a) {
  for (environment::iterator it = a.variables.begin(); it != a.variables.end(); ++it) {
    if (env.find(it->first) == env.end()) {
      // provide this variable if not defined already
      env[it->first] = std::move(it->second);
    }
  }
}
inline void do_set_environment(environment & env, remove_environment_variables && a) {
  for (std::vector<std::string>::iterator it = a.variable_names.begin();
       it != a.variable_names.end(); ++it) {
    auto find_it = env.find(*it);
    if (find_it != env.end()) {
      // remove this key
      env.erase(find_it);
    }
  }
}
inline void do_set_environment_manip_path(environment & env, environment && variables,
                                          bool prepend) {
  for (environment::iterator it = variables.begin(); it != variables.end(); ++it) {
    if (env.find(it->first) == env.end()) {
      // if not defined, set the value
      env[it->first] = std::move(it->second);
    } else if (prepend) {
      env[it->first] = it->second + detail::path_separator + std::move(env[it->first]);
    } else {
      env[it->first] = std::move(env[it->first]) + detail::path_separator + it->second;
    }
  }
}
inline void do_set_environment(environment & env, prepend_path_environment_variables && a) {
  do_set_environment_manip_path(env, std::move(a.variables), true);
}
inline void do_set_environment(environment & env, append_path_environment_variables && a) {
  do_set_environment_manip_path(env, std::move(a.variables), false);
}
} // namespace detail

#ifdef _KLFENGINE_PROCESSED_BY_DOXYGEN
/** \brief Change the given environment with the specified operations
 *
 * The given \a environment is modified according to the keyword-like arguments
 * \a arg0, \a arg1, ..., which are processed in the given order.  The args must
 * be temporary instances that are of type \ref set_environment_variables, \ref
 * provide_environment_variables, \ref remove_environment_variables, \ref
 * prepend_path_environment_variables, or \ref
 * append_path_environment_variables.  See the documentation of those objects
 * for more details about their effect.
 *
 * Example:
 * \code
 *   klfe::environment e{{"VAR1", "value 1"}, {"MY_PATH_VAR", "/usr/bin:/bin"}};
 *   klfe::set_environment(
 *       e,
 *       klfe::set_environment_variables{ {{"VAR2", "value 2"}} },
 *       klfe::remove_environment_variables{ {"VAR1"} },
 *       klfe::prepend_path_environment_variables{
 *         {{"MY_PATH_VAR", "/usr/local/bin:/opt/bin"}}
 *       },
 *   );
 *   // e == { {"VAR2", "value 2"},
 *   //        {"MY_PATH_VAR", "/usr/local/bin:/opt/bin:/usr/bin:/bin"} }
 * \endcode
 */
void set_environment(environment & environment,
                     EnvManipArg0 arg0, EnvManipArg1 arg1, ...);
#else
inline void set_environment(environment & ) { }
template<typename EnvManipArg, typename... EnvManipArgs>
inline void set_environment(environment & environment_, EnvManipArg && a1,
                            EnvManipArgs && ... rest)
{
  detail::do_set_environment(environment_, std::forward<EnvManipArg>(a1));
  set_environment(environment_, std::forward<EnvManipArgs>(rest)...);
}
#endif

namespace detail {

template<typename Arg, typename ArgNoRef = typename std::remove_reference<Arg>::type,
         typename... Args>
inline typename std::enable_if<
  (std::is_same<ArgNoRef,set_environment_variables>::value ||
   std::is_same<ArgNoRef,provide_environment_variables>::value ||
   std::is_same<ArgNoRef,remove_environment_variables>::value ||
   std::is_same<ArgNoRef,append_path_environment_variables>::value ||
   std::is_same<ArgNoRef,prepend_path_environment_variables>::value),
  void
  >::type
do_set_environment_or_ignore_args(environment & e, Arg && a1, Args && ... rest)
{
  do_set_environment(e, std::move(a1), std::forward<Args>(rest)...);
}

template<typename Arg, typename ArgNoRef = typename std::remove_reference<Arg>::type,
         typename... Args>
inline typename std::enable_if<
  !(std::is_same<ArgNoRef,set_environment_variables>::value ||
    std::is_same<ArgNoRef,provide_environment_variables>::value ||
    std::is_same<ArgNoRef,remove_environment_variables>::value ||
    std::is_same<ArgNoRef,append_path_environment_variables>::value ||
    std::is_same<ArgNoRef,prepend_path_environment_variables>::value),
  void
  >::type
do_set_environment_or_ignore_args(environment & , Arg && , Args && ...)
{
}

inline void set_environment_or_ignore_args(environment & ) { }
template<typename EnvManipArg, typename... EnvManipArgs>
inline void set_environment_or_ignore_args(environment & environment_,
                                           EnvManipArg && a1,
                                           EnvManipArgs && ... rest)
{
  do_set_environment_or_ignore_args(environment_, std::forward<EnvManipArg>(a1));
  set_environment_or_ignore_args(environment_, std::forward<EnvManipArgs>(rest)...);
}

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
    environment * process_environment,
    int * capture_exit_code
    );

} // namespace detail


/** Interface to run an external process and capture its output
 *
 * \todo The API might still change.  I'm thinking about having running
 *       processes and pipes, etc.
 */
class process {
public:

  /** \brief Specifies the executable file that should be run
   *
   * If you don't specify this argument to \ref run_and_wait(), then the first
   * item in \a argv is used as the executable file name (but the executable is
   * not searched in the system \a $PATH)
   */
  struct executable {
    const std::string & _data_ref;
  };
  struct run_in_directory {
    const std::string & _data_ref;
  };

  /** \brief Instruction to send data to process' standard input
   *
   * After executing the process, \ref run_and_wait() will write the given data
   * to its standard input.  Example usage:
   * \code
   *   binary_data in_data = ...;
   *   klfe::process::run_and_wait(
   *       ...,
   *       klfe::process::send_stdin_data{in_data},
   *       ...
   *   );
   * \endcode
   */
  struct send_stdin_data {
    const binary_data & _data_ref;
  };

  /** \brief Instruct to capture stdout data to the given buffer
   *
   * Instructs \ref run_and_wait() to capture the standard output of the process
   * into the referenced buffer.  Use as follows:
   * \code
   *   binary_data out_buffer;
   *   klfe::process::run_and_wait(
   *       ...,
   *       klfe::process::capture_stdout_data{out_buffer},
   *       ...
   *   );
   * \endcode
   */
  struct capture_stdout_data {
    binary_data & _data_ref;
  };
  /** \brief Instruct to capture stderr data to the given buffer 
   *
   * Same as \ref capture_stdout_data, but for standard error output.
   */
  struct capture_stderr_data {
    binary_data & _data_ref;
  };
  /** \brief Dynamically control whether stdout capture is to be activated
   *
   * Only works in conjuction with \ref capture_stdout_data.  If
   * <code>capture_stdout_if{false}</code> is provided to \ref run_and_wait(),
   * then the effect of any <code>capture_stderr_data</code> is inhibited.
   */
  struct capture_stdout_if{
    bool _capture;
  };
  /** \brief Same as \ref capture_stdout_if, but for stderr
   */
  struct capture_stderr_if{
    bool _capture;
  };

  /** \brief Instruction for executed subprocess to not inherit the current environment
   *
   * It is your responsibility to provide any values for standard environment
   * variables (\a HOME, \a PATH, etc.) the child process might expect.
   */
  struct clear_environment {};

  /** \brief Execute a process and wait until it terminates
   *
   * The first parameter is the standard \a argv list for the new process, with
   * the first element being the process name (or what you want it to think it's
   * named).  Specify any other arguments as pseudo-keyword arguments using the
   * following helper classes:
   *
   * - <code>executable{"/path/to/exectuable"}</code> -- see \ref executable
   *
   * - <code>capture_stdout_data{buffer}, capture_stderr_data{buffer},
   *   capture_stdout_if{??}, capture_stderr_if{??}</code> -- capture the
   *   process output to a given buffer.  See \ref capture_stdout_data, \ref
   *   capture_stderr_data, \ref capture_stdout_if, \ref capture_stderr_if
   *
   * - any acceptable argument to \ref klfengine::set_environment(),
   *   possibly in combination with <code>clear_environment{}</code> -- specify
   *   how to set the subprocess' environment.
   *
   * Errors are reported by throwing suitable execptions.  If the process exits
   * with a nonzero return code this is considered an error and an exception is
   * thrown.  In case of errors and if you're capturing stdout and/or stderr,
   * the buffers will contain the data that has been received so far.
   */
  template<typename... Args>
  static void run_and_wait(const std::vector<std::string> & argv, Args && ... args)
  {
    using namespace detail;

    if (argv.empty()) {
      throw std::invalid_argument("klfengine::process::run_and_wait(): cannot have empty argv");
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
    if (get_kwargs<Args...>::template has_arg<capture_stdout_if>::value) {
      capture_stdout_if d{
        get_kwargs<Args...>::template get_arg<capture_stdout_if>(args...)
      };
      if (d._capture == false) {
        capture_stdout = nullptr;
      }
    }

    binary_data * capture_stderr = nullptr;

    if (get_kwargs<Args...>::template has_arg<capture_stderr_data>::value) {
      capture_stderr_data d{
        get_kwargs<Args...>::template get_arg<capture_stderr_data>(args...)
      };
      capture_stderr = & d._data_ref;
    }
    if (get_kwargs<Args...>::template has_arg<capture_stderr_if>::value) {
      capture_stderr_if d{
        get_kwargs<Args...>::template get_arg<capture_stderr_if>(args...)
      };
      if (d._capture == false) {
        capture_stderr = nullptr;
      }
    }

    environment env;
    bool want_clear_env = false;
    bool want_env = false;
    if (get_kwargs<Args...>::template has_arg<clear_environment>::value) {
      want_env = true;
      want_clear_env = true;
    }
    if (get_kwargs<Args...>::template has_arg<set_environment_variables>::value ||
        get_kwargs<Args...>::template has_arg<provide_environment_variables>::value ||
        get_kwargs<Args...>::template has_arg<remove_environment_variables>::value ||
        get_kwargs<Args...>::template has_arg<append_path_environment_variables>::value ||
        get_kwargs<Args...>::template has_arg<prepend_path_environment_variables>::value) {
      want_env = true;
      if (!want_clear_env) {
        env = current_environment();
      }
      detail::set_environment_or_ignore_args(env, args...);
      // using json = nlohmann::json;
      // std::cerr << "child environ will be set to:\n" << json{env}.dump(4) << "\n";
    }

    int exit_code = -1;

    run_process_impl(
        ex,
        argv,
        run_cwd,
        stdin_d,
        capture_stdout,
        capture_stderr,
        want_env ? &env : nullptr,
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
