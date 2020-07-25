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


#include <memory>

#include <klfengine/basedefs.h>


namespace klfengine
{

class run;

/** \brief Base abstract class for implementations
 *
 * Implementations should subclass this class to provide a factory for
 * \ref engine_run_implementation instances.  See \ref run().
 *
 * Subclasses of this class sholud also provide information about the
 * implementation (currently only \ref name() but later TBD maybe also expected
 * available formats, etc.).
 * 
 */
class engine
{
public:
  explicit engine(std::string name_);
  virtual ~engine() = default;

  inline const std::string & name() const { return _name; }

  void set_settings(klfengine::settings settings_);
  inline klfengine::settings settings() const { return _settings; }

  std::unique_ptr<klfengine::run> run( input input_ );

protected:
  const std::string _name;
  klfengine::settings _settings;

private:
  virtual klfengine::run *
  impl_create_compilation( input input_, klfengine::settings settings_ ) = 0;
};


}


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/engine.hxx>
#endif
