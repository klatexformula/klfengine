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



#include <subprocess.hpp>

#include <klfengine/engine.h>


namespace klfengine
{

namespace dummy_engine
{

class dummy_engine : public engine
{
public:
  dummy_engine()
    : engine("dummy_engine")
  {
  }

private:
  klfengine::run *
  impl_create_compilation( input input_, klfengine::settings settings_ );
};


struct dummy_run : klfengine::run
{
  dummy_run(
      std::shared_ptr<klfengine::engine> engine,
      klfengine::input input_,
      klfengine::settings settings_
      )
    : klfengine::run(std::move(input_), std::move(settings_))
  {
  }

private:
  void impl_compile()
  {
    std::cerr << "\"compiling\" ... \n";
    std::cerr << "let's say it's done!\n";
  }
  std::vector<std::string> impl_available_formats() const
  {
    return {"TXT", "TEX"};
  }

  virtual bool impl_has_format(const char * format,
                               const value::dict & ) const
  {
    return impl_available_formats().contains(format);
  }
  virtual binary_data impl_get_format_data(const char * format,
                                           const value::dict & parameters)
  {
    std::string d = std::string("dummy output data in ") + format + " format.";

    if (parameters.find("raw") != parameters.end() &&
        parameters["raw"].get<bool>() == true) {
      d += " [it's raw!]";
    }
    d += "\n";
    
    return {d.begin(), d.end()};
  }

};





inline klfengine::run *
dummy_engine::impl_create_compilation( input input_, klfengine::settings settings_ );
{
  return new dummy_run(this, std::move(input_), std::move(settings_));
}



} // namespace dummy_engine
} // namespace klfengine
