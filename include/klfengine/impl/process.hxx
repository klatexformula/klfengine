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

#include <exception>

#include <klfengine/basedefs>

#include <klfengine/process>


namespace klfengine {

_KLFENGINE_INLINE
process_exit_error::process_exit_error(std::string msg)
  : exception(msg)
{}


_KLFENGINE_INLINE
process_exit_error::~process_exit_error()
{}



namespace detail {

// static
_KLFENGINE_INLINE
std::string suffix_out_and_err(const binary_data & out, const binary_data & err)
{
  if (!out.empty() && !err.empty()) {
    return ":\n*** output: ***\n" + std::string{out.begin(), out.end()}
    + "\n*** error: ***\n" + std::string{err.begin(), err.end()};
  }
  if (!out.empty()) {
    return ":\n" + std::string{out.begin(), out.end()};
  }
  if (!err.empty()) {
    return ":\n" + std::string{err.begin(), err.end()};
  }
  return " [no output]";
}

} // namespace detail

} // namespace klfengine


// -----------------------------------------------------------------------------


namespace klfengine {
namespace detail {

struct exit_cleaner
{
  std::function<void()> fn;

  template<typename Fn>
  explicit exit_cleaner(Fn && f) : fn(std::forward<Fn>(f)) {}

  inline ~exit_cleaner() {
    cleanup();
  }

  inline void cleanup() {
    if (fn) {
      fn();
    }
    fn = std::function<void()>{};
  }

  inline void release() {
    fn = std::function<void()>{};
  }

  exit_cleaner(const exit_cleaner & ) = delete;
  exit_cleaner & operator=(const exit_cleaner & ) = delete;

  exit_cleaner(exit_cleaner && ) = default;
  exit_cleaner & operator=(exit_cleaner && ) = default;

};

} // namespace detail
} // namespace klfengine



// -----------------------------------------------------------------------------

// my low-level implementation of executing a process, feeding input, and
// getting output.  Separate implementations for POSIX and Windows.

#if defined(__unix__) || defined(__APPLE__)

#include <chrono> // std::milliseconds()
#include <thread> // std::this_thread::sleep_for()
#include <system_error>

#include <cstdlib>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


namespace klfengine {

namespace detail {

inline void set_read_nonblock(int fd)
{
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    throw std::system_error{errno, std::generic_category()};
  }
  int retval = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (retval == -1) {
    throw std::system_error{errno, std::generic_category()};
  }
}

_KLFENGINE_INLINE
void run_process_impl(
    const std::string & executable,
    const std::vector<std::string> & argv,
    const std::string & run_cwd,
    const binary_data & stdin_data,
    binary_data & capture_stdout,
    binary_data & capture_stderr,
    int & capture_exit_code
    )
{
  // pipe for stdout
  int stdout_fds[2];
  if ( pipe(stdout_fds) != 0 ) {
    throw std::system_error(errno, std::generic_category());
  }
  
  // in case of error before fork(), close the pipe we just opened.
  detail::exit_cleaner clean_stdout_pipes{
    [stdout_fds]() {
      // close the stdout pipes we opened earlier
      close(stdout_fds[0]);
      close(stdout_fds[1]);
    }
  };
      
  set_read_nonblock(stdout_fds[0]);

  // pipe for stderr
  int stderr_fds[2];
  if ( pipe(stderr_fds) != 0 ) {
    throw std::system_error{errno, std::generic_category()};
  }

  // in case of error before fork(), close the pipe we just opened.
  detail::exit_cleaner clean_stderr_pipes{
    [stderr_fds]() {
      // close the stdout pipes we opened earlier
      close(stderr_fds[0]);
      close(stderr_fds[1]);
    }
  };


  // pipe for stderr
  int stdin_fds[2];
  if ( pipe(stdin_fds) != 0 ) {
    throw std::system_error{errno, std::generic_category()};
  }

  // in case of error before fork(), close the pipe we just opened.
  detail::exit_cleaner clean_stdin_pipes{
    [stdin_fds]() {
      // close the stdout pipes we opened earlier
      close(stdin_fds[0]);
      close(stdin_fds[1]);
    }
  };

  const pid_t pid = fork();
  if (pid < 0) {
    throw std::system_error{errno, std::generic_category()};
  }
  // release our pipe cleaners -- we'll close them ourselves from this point on
  clean_stdout_pipes.release();
  clean_stderr_pipes.release();
  clean_stdin_pipes.release();

  if (pid == 0) {
    // this is the child process

    close(stdin_fds[1]);
    dup2(stdin_fds[0], 0);
    close(stdin_fds[0]);

    close(stdout_fds[0]);
    dup2(stdout_fds[1], 1);
    close(stdout_fds[1]);

    close(stderr_fds[0]);
    dup2(stderr_fds[1], 2);
    close(stderr_fds[1]);

    // change working directory
    if ( !run_cwd.empty() ) {
      if ( chdir(run_cwd.c_str()) == -1 ) {
        std::error_code syserr{errno, std::generic_category()};
        std::string errstr{ "chdir error " };
        errstr += std::to_string(syserr.value()) + ": " + syserr.message();
        // & write this to stderr ->
        write(2, errstr.c_str(), errstr.size());
        std::abort();
      }
    }

    std::vector<char*> vc(argv.size() + 1, 0);
    for (std::size_t i = 0; i < argv.size(); ++i) {
      vc[i] = const_cast<char*>(argv[i].c_str());
    }

    // execution! this shouldn't return.
    execv(executable.c_str(), &vc[0]);

    // error calling execv
    std::error_code syserr{errno, std::generic_category()};
    std::string errstr{ "execv error " };
    errstr += std::to_string(syserr.value()) + ": " + syserr.message();
    // & write this to stderr ->
    write(2, errstr.c_str(), errstr.size());
    std::abort();
  }

  close(stdin_fds[0]);
  close(stdout_fds[1]);
  close(stderr_fds[1]);

  ssize_t r_written = 0;
  while (r_written < (ssize_t)stdin_data.size()) {
    ssize_t r = write(stdin_fds[1], &stdin_data[r_written],
                      stdin_data.size()-r_written);
    if (r >= 0) {
      // success
      r_written += r;
      continue;
    }
    // error (r < 0)
    if (errno == EINTR) {
      continue; // try again
    }
    throw std::system_error{errno, std::generic_category()};
  }

  close(stdin_fds[1]);

  exit_cleaner clean_read_fds{
    [stdout_fds,stderr_fds]() {
      close(stdout_fds[0]);
      close(stderr_fds[0]);
    }
  };

  binary_data out;
  binary_data err;

  bool out_done[2] {false, false};
  binary_data * out_p[2]{ &out, &err };
  int read_fds[2] {stdout_fds[0], stderr_fds[0]};

  constexpr int buf_size = 4096;

  do {

    // read stdout & stderr, filling buffers as we read data

    for (int which = 0; which < 2; ++which) { // for which in [0, 1]
      auto & this_out_done = out_done[which];
      auto & this_buf = * out_p[which];
      int this_fd = read_fds[which];
      if (this_out_done) {
        continue;
      }
      std::size_t pos = this_buf.size();
      this_buf.resize(pos + buf_size);
      ssize_t r = read(this_fd, &this_buf[pos], buf_size);
      if (r > 0) {
        this_buf.resize(pos + r);
        continue;
      }
      // didn't read anything ->
      this_buf.resize(pos);
      if (r == 0) {
        this_out_done = true;
        continue;
      }
      if (errno == EAGAIN) {
        // no data available at the moment, continue trying later
        continue;
      } else if (errno == EINTR) {
        // interrupted by signal (for some reason), try again later
        continue;
      } else {
        throw std::system_error{errno, std::generic_category()};
      }
    }

    // wait a little bit before next read()
    std::this_thread::sleep_for(std::chrono::milliseconds{20});

  } while (!out_done[0] || !out_done[1]);

  clean_read_fds.cleanup();

  int ret_status;
  for (;;) {
    pid_t r = waitpid(pid, &ret_status, 0);
    if (r == -1) {
      if (errno == EINTR) {
        // try again
        continue;
      }
      throw std::system_error{errno, std::generic_category()};
    }
    break;
  }

  if (WIFEXITED(ret_status)) {
    int exit_code = WEXITSTATUS(ret_status);

    capture_stdout = std::move(out);
    capture_stderr = std::move(err);
    capture_exit_code = exit_code;

    if (exit_code == 0) {
      return;
    }
    throw process_exit_error{
      "Process " + executable + " exited with code " + std::to_string(exit_code)
      + suffix_out_and_err(out, err)
    };
  }

  if (WIFSIGNALED(ret_status)) {
    auto signo = WTERMSIG(ret_status);
    throw process_exit_error{
      "Process " + executable + " terminated with signal "
      + std::to_string(signo)
#if (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700) || \
  (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
      + strsignal(signo)
#endif
      + suffix_out_and_err(out, err)
    };
  }

  throw process_exit_error{
    "Process " + executable + " did not exit normally; waitpid() reported status "
    + std::to_string(ret_status)
  };
}

} // namespace detail

} // namespace klfengine


#elif defined(__WIN32) || defined(__WIN64)


// thanks https://stackoverflow.com/a/46348112/1694896

............................. need to go through this function .................
int SystemCapture(
    std::string         CmdLine,    //Command Line
    std::string         CmdRunDir,  //set to '.' for current directory
    std::string&        ListStdOut, //Return List of StdOut
    std::string&        ListStdErr, //Return List of StdErr
    std::uint32_t&      RetCode)    //Return Exit Code
{
    int                  Success;
    SECURITY_ATTRIBUTES  security_attributes;
    HANDLE               stdout_rd = INVALID_HANDLE_VALUE;
    HANDLE               stdout_wr = INVALID_HANDLE_VALUE;
    HANDLE               stderr_rd = INVALID_HANDLE_VALUE;
    HANDLE               stderr_wr = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION  process_info;
    STARTUPINFO          startup_info;
    thread               stdout_thread;
    thread               stderr_thread;

    security_attributes.nLength              = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle       = TRUE;
    security_attributes.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&stdout_rd, &stdout_wr, &security_attributes, 0) ||
            !SetHandleInformation(stdout_rd, HANDLE_FLAG_INHERIT, 0)) {
        return -1;
    }

    if (!CreatePipe(&stderr_rd, &stderr_wr, &security_attributes, 0) ||
            !SetHandleInformation(stderr_rd, HANDLE_FLAG_INHERIT, 0)) {
        if (stdout_rd != INVALID_HANDLE_VALUE) CloseHandle(stdout_rd);
        if (stdout_wr != INVALID_HANDLE_VALUE) CloseHandle(stdout_wr);
        return -2;
    }

    ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));

    startup_info.cb         = sizeof(STARTUPINFO);
    startup_info.hStdInput  = 0;
    startup_info.hStdOutput = stdout_wr;
    startup_info.hStdError  = stderr_wr;

    if(stdout_rd || stderr_rd)
        startup_info.dwFlags |= STARTF_USESTDHANDLES;

    // Make a copy because CreateProcess needs to modify string buffer
    char      CmdLineStr[MAX_PATH];
    strncpy(CmdLineStr, CmdLine.c_str(), MAX_PATH);
    CmdLineStr[MAX_PATH-1] = 0;

    Success = CreateProcess(
        nullptr,
        CmdLineStr,
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        CmdRunDir.c_str(),
        &startup_info,
        &process_info
    );
    CloseHandle(stdout_wr);
    CloseHandle(stderr_wr);

    if(!Success) {
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
        CloseHandle(stdout_rd);
        CloseHandle(stderr_rd);
        return -4;
    }
    else {
        CloseHandle(process_info.hThread);
    }

    if(stdout_rd) {
        stdout_thread=thread([&]() {
            DWORD  n;
            const size_t bufsize = 1000;
            char         buffer [bufsize];
            for(;;) {
                n = 0;
                int Success = ReadFile(
                    stdout_rd,
                    buffer,
                    (DWORD)bufsize,
                    &n,
                    nullptr
                );
                printf("STDERR: Success:%d n:%d\n", Success, (int)n);
                if(!Success || n == 0)
                    break;
                string s(buffer, n);
                printf("STDOUT:(%s)\n", s.c_str());
                ListStdOut += s;
            }
            printf("STDOUT:BREAK!\n");
        });
    }

    if(stderr_rd) {
        stderr_thread=thread([&]() {
            DWORD        n;
            const size_t bufsize = 1000;
            char         buffer [bufsize];
            for(;;) {
                n = 0;
                int Success = ReadFile(
                    stderr_rd,
                    buffer,
                    (DWORD)bufsize,
                    &n,
                    nullptr
                );
                printf("STDERR: Success:%d n:%d\n", Success, (int)n);
                if(!Success || n == 0)
                    break;
                string s(buffer, n);
                printf("STDERR:(%s)\n", s.c_str());
                ListStdOut += s;
            }
            printf("STDERR:BREAK!\n");
        });
    }

    WaitForSingleObject(process_info.hProcess,    INFINITE);
    if(!GetExitCodeProcess(process_info.hProcess, (DWORD*) &RetCode))
        RetCode = -1;

    CloseHandle(process_info.hProcess);

    if(stdout_thread.joinable())
        stdout_thread.join();

    if(stderr_thread.joinable())
        stderr_thread.join();

    CloseHandle(stdout_rd);
    CloseHandle(stderr_rd);

    return 0;
}

#endif
