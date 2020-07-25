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

#include <atomic>

#include <klfengine/engine.h>

namespace klfengine {


/** \brief Raised in the event of inconsistent calls to run::compile()
 *
 */
struct forgot_to_call_compile : klfengine::exception
{
  forgot_to_call_compile()
    : exception("You forgot to call klfengine::run::compile()")
  { }
};

/** \brief Raised in the event of inconsistent calls to run::compile()
 *
 */
struct dont_call_compile_twice : klfengine::exception
{
  dont_call_compile_twice()
    : exception("You called klfengine::run::compile() twice")
  { }
};




namespace detail
{
/**
 * \internal
 *
 * Type to use for cache keys (which represent format_spec's).  We'll store some
 * (binary?) string representing the format_spec (e.g. JSON? or BSON?)
 */
using fmtspec_cache_key_type = std::string;
}


class engine_run_implementation;




/** \brief A compiling run of some latex code
 *
 * Instances of klfengine::run are returned by \ref klfengine::engine::run().
 *
 * All members of this base class are thread-safe, protected by a
 * class-instance-wide mutex lock upon entry in each method.
 */
class run
{
public:
  run(std::unique_ptr<engine_run_implementation> engine_run_implementation_);

  virtual ~run();

  /** \brief Run any initial compilation steps
   *
   * Users \b must call \a compile() on this object before calling any of the
   * other member functions of this class (including \a has_format() etc.).
   *
   * See also \ref compiled().
   */
  void compile();

  /** \brief Check whether compile() has been called and has completed
   *
   * Returns \a true if this run has been "compiled" by calling \a compile(),
   * and if \a compile() was called from a different thread, also checks that
   * compile() has completed.
   *
   * See also \ref compile().
   */
  bool compiled() const;

  bool has_format(const format_spec & format) const;

  std::vector<format_description> available_formats() const;

  format_spec canonical_format(const format_spec & format);

  template<typename IteratorInterfaceContainer>
  format_spec find_format(const IteratorInterfaceContainer & formats);

  // no copy, move, or assignment operators.
  run(const run &) = delete;
  run(run &&) = delete;
  run operator=(const run &) = delete;
  run & operator=(run &&) = delete;


private:

  std::unique_ptr<engine_run_implementation> _e;

  bool _compiled;

  std::mutex _mutex;

  void _ensure_compiled() const;
};



// template members need to be in the .h, not in the .hxx which might be
// compiled separately in a single translation unit

template<typename IteratorInterfaceContainer>
inline format_spec find_format(const IteratorInterfaceContainer & formats)
{
  _ensure_compiled();

  // note container value type can also be std::string, because you can
  // construct a format_spec from a std::string (DOUBLE-CHECK THIS)

  auto result = std::find_if(
      formats.begin(),
      formats.end(),
      [this](const format_spec & f) {
        return impl_has_format(f);
      }
      );

  if (result == formats.end()) {
    throw no_such_format("<no suitable format found in list>");
  }

  return *result;
}



} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/run.hxx>
#endif
