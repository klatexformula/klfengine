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


#include <klfengine/basedefs>
#include <klfengine/input>
#include <klfengine/settings>


namespace klfengine
{

class run;
class engine_run_implementation;

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

  /** \brief The name of this engine implementation
   *
   * This simply returns the name that the subclass specified to the constructor
   * of this base class.
   */
  inline const std::string & name() const { return _name; }

  void set_settings(klfengine::settings settings_);
  inline klfengine::settings settings() const { return _settings; }

  std::unique_ptr<klfengine::run> run( input input_ );

private:
  const std::string _name;
  klfengine::settings _settings;

  /** \brief Called immediately after new settings were set
   *
   * This is called from set_settings(), after saving the new settings.
   * Subclasses can reimplement this method to adjust internal objects (a gs
   * interface, or instance) to the new settings.
   *
   * Subclasses can also directly alter the settings, if required.
   *
   * The default implementation does nothing.
   */
  virtual void adjust_for_new_settings(klfengine::settings & settings);

  /** \brief Create new engine run implementation instance for a new klf job
   *
   */
  virtual klfengine::engine_run_implementation *
  impl_create_engine_run_implementation(
      input input_,
      klfengine::settings settings_
      ) = 0;
};


}


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/engine.hxx>
#endif
