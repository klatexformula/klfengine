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
// #include <functional>

//#include <iostream> // DEBUG

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
std::string suffix_out_and_err(const binary_data * out, const binary_data * err)
{
  if (out != nullptr && !out->empty() && err != nullptr && !err->empty()) {
    return ":\n*** output: ***\n" + std::string{out->begin(), out->end()}
    + "\n*** error: ***\n" + std::string{err->begin(), err->end()};
  }
  if (out != nullptr && !out->empty()) {
    return ":\n" + std::string{out->begin(), out->end()};
  }
  if (err != nullptr && !err->empty()) {
    return ":\n" + std::string{err->begin(), err->end()};
  }
  return " [no output or output not captured]";
}

} // namespace detail

} // namespace klfengine


// -----------------------------------------------------------------------------





// -----------------------------------------------------------------------------

// my low-level implementation of executing a process, feeding input, and
// getting output.  Separate implementations for POSIX and Windows.

#if defined(__unix__) || defined(__APPLE__)

#include <thread>
#include <system_error>

#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


namespace klfengine {

namespace detail {


inline void child_process_errno_abort(std::string what)
{
  std::error_code syserr{errno, std::generic_category()};
  std::string errstr{ std::move(what) };
  errstr += " error " + std::to_string(syserr.value()) + ": " + syserr.message();
  // & write this to stderr ->
  write(2, errstr.c_str(), errstr.size());
  std::abort();
}

inline void throw_from_errno()
{
  throw std::system_error{errno, std::generic_category()};
}

// inline void set_read_nonblock(int fd)
// {
//   int flags = fcntl(fd, F_GETFL);
//   if (flags < 0) {
//     throw_from_errno();
//   }
//   int retval = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
//   if (retval == -1) {
//     throw_from_errno();
//   }
// }

class pipe_handler
{
private:
  int _fds[2];
  bool _pipe_open;
  bool _pipe_closed[2];

public:
  pipe_handler()
    : _fds{0, 0},
      _pipe_open{false},
      _pipe_closed{false,false}
  {
  }

  ~pipe_handler()
  {
    if (_pipe_open) {
      if (!_pipe_closed[0]) {
        close(_fds[0]);
      }
      if (!_pipe_closed[1]) {
        close(_fds[1]);
      }
    }
  }

  bool pipe_open() { return _pipe_open; }

  void create()
  {
    if (_pipe_open) {
      throw std::runtime_error("pipe_handler::create(): pipe is already open");
    }

    if ( pipe(_fds) != 0 ) {
      throw_from_errno();
    }
    _pipe_open = true;
    _pipe_closed[0] = false;
    _pipe_closed[1] = false;
  }

  int read_fd()
  {
    if (!_pipe_open || _pipe_closed[0]) {
      throw std::runtime_error("pipe_handler::read_fd(): pipe not open");
    }
    return _fds[0];
  }
  int write_fd()
  {
    if (!_pipe_open || _pipe_closed[1]) {
      throw std::runtime_error("pipe_handler::write_fd(): pipe not open");
    }
    return _fds[1];
  }

  void close_read()
  {
    if (!_pipe_open || _pipe_closed[0]) {
      throw std::runtime_error("pipe_handler::close_read(): pipe not open");
    }
    close(_fds[0]);
  }

  void close_write()
  {
    if (!_pipe_open || _pipe_closed[1]) {
      throw std::runtime_error("pipe_handler::close_write(): pipe not open");
    }
    close(_fds[1]);
  }

  void child_process_setup_dup_read_to(int dupfd)
  {
    close(_fds[1]);
    if ( dup2(_fds[0], dupfd) == -1 ) {
      child_process_errno_abort("dup2");
    }
    close(_fds[0]);

    _pipe_closed[0] = true;
    _pipe_closed[1] = true;
  }
  void child_process_setup_dup_write_to(int dupfd)
  {
    close(_fds[0]);
    if ( dup2(_fds[1], dupfd) == -1 ) {
      child_process_errno_abort("dup2");
    }
    close(_fds[1]);

    _pipe_closed[0] = true;
    _pipe_closed[1] = true;
  }
  
  // allow move semantics
  pipe_handler(pipe_handler&& m)
    : _fds{m._fds[0], m._fds[1]},
      _pipe_open{m._pipe_open},
      _pipe_closed{m._pipe_closed[0], m._pipe_closed[1]}
  {
    m._fds[0] = 0;
    m._fds[1] = 0;
    m._pipe_open = false;
    m._pipe_closed[0] = false;
    m._pipe_closed[1] = false;
  }

  pipe_handler & operator=(pipe_handler&& m)
  {
    _fds[0] = m._fds[0];
    _fds[1] = m._fds[1];
    _pipe_open = m._pipe_open;
    _pipe_closed[0] = m._pipe_closed[0];
    _pipe_closed[1] = m._pipe_closed[1];
  
    m._fds[0] = 0;
    m._fds[1] = 0;
    m._pipe_open = false;
    m._pipe_closed[0] = false;
    m._pipe_closed[1] = false;

    return *this;
  }

  // no copy semantics
  pipe_handler(const pipe_handler&) = delete;
  pipe_handler & operator=(const pipe_handler &) = delete;
};



inline void write_data_from_buffer(int fd, const binary_data & buffer)
{
  // std::cerr << "write_data_from_buffer(" << fd << ", (buffer))\n";

  ssize_t r_written = 0;
  while (r_written < (ssize_t)buffer.size()) {
    ssize_t r = write(fd, &buffer[r_written], buffer.size() - r_written);
    if (r >= 0) {
      // success
      r_written += r;
      continue;
    }
    // error (r < 0)
    if (errno == EINTR) {
      continue; // try again
    }
    throw_from_errno();
  }

  // std::cerr << "write_data_from_buffer(" << fd << ", (buffer)) done.\n";
}

inline void read_data_to_buffer(int fd, binary_data & buffer)
{
  buffer.clear();

  // std::cerr << "read_data_to_buffer(" << fd << ", (buffer))\n";

  constexpr int read_buf_size = 4096;

  std::size_t pos = 0;

  for (;;) {
    buffer.resize(pos + read_buf_size);

    ssize_t r = read(fd, &buffer[pos], read_buf_size);
    // std::cerr << "primitive read(" << fd << ") returned " << r << "\n";
    if (r > 0) {
      buffer.resize(pos + r);
      pos += r;
      // std::cerr << "got some data, buffer is now '"
      //           << std::string{buffer.begin(), buffer.end()} << "'\n";
      continue;
    }
    // otherwise, we didn't read anything, so reset buffer size as before
    buffer.resize(pos);
    if (r == 0) {
      // end of stream reached
      // std::cerr << "read_data_to_buffer(" << fd << ", (buffer)) done. buffer = '"
      //           << std::string{buffer.begin(), buffer.end()} << "'\n";
      return;
    }
    if (errno == EAGAIN) {
      // no data available at the moment, continue trying
      continue;
    }
    if (errno == EINTR) {
      // interrupted by signal (for some reason), try again
      continue;
    }
    
    throw_from_errno();
  }
}

inline void setup_thread_capture_output(binary_data * buffer, int fd,
                                        std::exception_ptr * th_excptr)
{
  try {
    read_data_to_buffer(fd, *buffer);
    close(fd);
    // std::cerr << "finishing read thread here on fd=" << fd << ", buffer is '"
    //           << std::string{buffer.begin(), buffer.end()} << "'\n";
  } catch (...) {
    *th_excptr = std::current_exception();
  }
}



_KLFENGINE_INLINE
void run_process_impl(
    const std::string & executable,
    const std::vector<std::string> & argv,
    const std::string & run_cwd,
    const binary_data * stdin_data,
    binary_data * capture_stdout,
    binary_data * capture_stderr,
    int * capture_exit_code
    )
{
  // std::cerr << "run_process_impl(" << executable << ")\n";

  // pipe handlers -- whether we actually open the pipes or not
  pipe_handler p_in;
  pipe_handler p_out;
  pipe_handler p_err;

  if (stdin_data != nullptr) {
    p_in.create();
  }

  if (capture_stdout != nullptr) {
    p_out.create();
  }

  if (capture_stderr != nullptr) {
    p_err.create();
  }

  const pid_t pid = fork();
  if (pid < 0) {
    throw_from_errno();
  }

  if (pid == 0) {
    // this is the child process

    if (p_in.pipe_open()) {
      p_in.child_process_setup_dup_read_to(0);
    }
    if (p_out.pipe_open()) {
      p_out.child_process_setup_dup_write_to(1);
    }
    if (p_err.pipe_open()) {
      p_err.child_process_setup_dup_write_to(2);
    }

    // change working directory
    if ( !run_cwd.empty() ) {
      if ( chdir(run_cwd.c_str()) == -1 ) {
        child_process_errno_abort("chdir");
      }
    }

    std::vector<char*> vc(argv.size() + 1, 0);
    for (std::size_t i = 0; i < argv.size(); ++i) {
      vc[i] = const_cast<char*>(argv[i].c_str());
    }

    // execution! this shouldn't return.
    execv(executable.c_str(), vc.data());
    // error calling execv, since it returned to this program instead of
    // creating the new process -->
    child_process_errno_abort("execv");
  }

  if (p_in.pipe_open()) {
    p_in.close_read();
  }
  if (p_out.pipe_open()) {
    p_out.close_write();
  }
  if (p_err.pipe_open()) {
    p_err.close_write();
  }

  // threads to write to stdin & read from stdout/stderr
  std::unique_ptr<std::thread> th_in;
  std::unique_ptr<std::thread> th_out;
  std::unique_ptr<std::thread> th_err;

  std::exception_ptr th_in_excptr = nullptr;
  std::exception_ptr th_out_excptr = nullptr;
  std::exception_ptr th_err_excptr = nullptr;

  if (p_in.pipe_open()) {
    // create a thread that will write the input data
    int stdin_fd = p_in.write_fd();
    th_in = std::unique_ptr<std::thread>{ new std::thread{
      [stdin_data,stdin_fd,&th_in_excptr](){
        try {
          write_data_from_buffer(stdin_fd, *stdin_data);
          close(stdin_fd);
        } catch (...) {
          th_in_excptr = std::current_exception();
        }
      }
    } };
  }

  if (p_out.pipe_open()) {
    // std::cerr << "output pipe is open, creating thread for capture\n";
    th_out = std::unique_ptr<std::thread>{ new std::thread{
      std::bind( setup_thread_capture_output,
                 capture_stdout, p_out.read_fd(), &th_out_excptr )
    } };
  }
  if (p_err.pipe_open()) {
    // std::cerr << "stderr pipe is open, creating thread for capture\n";
    th_err = std::unique_ptr<std::thread>{ new std::thread{
      std::bind( setup_thread_capture_output,
                 capture_stderr, p_err.read_fd(), &th_err_excptr )
    } };
  }

  int ret_status;
  for (;;) {
    pid_t r = waitpid(pid, &ret_status, 0);
    if (r == -1) {
      if (errno == EINTR) {
        // try again
        continue;
      }
      throw_from_errno();
    }
    break;
  }

  // wait for any output to be collected, if necessary
  if (th_in != nullptr) { th_in->join(); }
  if (th_in_excptr != nullptr) { std::rethrow_exception(th_in_excptr); }
  if (th_out != nullptr) { th_out->join(); }
  if (th_out_excptr != nullptr) { std::rethrow_exception(th_out_excptr); }
  if (th_err != nullptr) { th_err->join(); }
  if (th_err_excptr != nullptr) { std::rethrow_exception(th_err_excptr); }

  // std::cerr << "run_process_impl(" << executable << ") process finished.\n";
  // if (capture_stdout != nullptr) {
  //   std::cerr << "\tcaptured stdout data = '"
  //             << std::string{capture_stdout->begin(),capture_stdout->end()} << "'\n";
  // }
  // if (capture_stderr != nullptr) {
  //   std::cerr << "\tcaptured stderr data = '"
  //             << std::string{capture_stderr->begin(),capture_stderr->end()} << "'\n";
  // }

  if (WIFEXITED(ret_status)) {
    int exit_code = WEXITSTATUS(ret_status);

    *capture_exit_code = exit_code;

    if (exit_code == 0) {
      return;
    }
    throw process_exit_error{
      "Process " + executable + " exited with code " + std::to_string(exit_code)
      + suffix_out_and_err(capture_stdout, capture_stderr)
    };
  }

  if (WIFSIGNALED(ret_status)) {
    auto signo = WTERMSIG(ret_status);
    throw process_exit_error{
      "Process " + executable + " terminated with signal "
      + std::to_string(signo) + " (" + strsignal(signo) + ")"
      + suffix_out_and_err(capture_stdout, capture_stderr)
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

// ............................. go through this code, convert to our function
// signature and conventions, and TEST!! .................

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
