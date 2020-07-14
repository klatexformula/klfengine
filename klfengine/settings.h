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


namespace klfengine
{


struct settings
{
  settings(std::string temporary_directory_ = std::string(),
           std::string texbin_directory_ = std::string(),
           std::string gs_executable_path_ = std::string(),
           std::map<std::string, std::string> subprocess_add_environment_
           = std::map<std::string, std::string>())
    : temporary_directory(std::move(temporary_directory_)),
      texbin_directory(std::move(texbin_directory_)),
      gs_executable_path(std::move(gs_executable_path_)),
      subprocess_add_environment(std::move(subprocess_add_environment_))
  {
  }

  std::string temporary_directory;

  std::string texbin_directory;

  std::string gs_executable_path;

  std::map<std::string, std::string> subprocess_add_environment;


  std::string get_tex_executable_path(const std::string & exe_name) const
  {
    std::string s = texbin_directory + "/" + exe_name;
    // check that s points to a valid executable
    ...
    return s;
  }


  static std::string detect_temporary_directory()
  {
    return ... ;
  }

  static std::string detect_texbin_directory()
  {
    return ... ;
  }

  static std::string detect_gs_executable_path()
  {
    return ... ;
  }

  static std::map<std::string, std::string> detect_subprocess_add_environment()
  {
    return ... ;
  }

  static settings detect_settings()
  {
    return settings{
      detect_temporary_directory(),
        detect_texbin_directory(),
        detect_gs_executable_path(),
        detect_subprocess_add_environment()
    };
  }

};

};
