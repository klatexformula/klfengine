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

#include <memory> // std::unique_ptr
#include <atomic>
#include <mutex>

#include <klfengine/engine_run_implementation>
#include <klfengine/format>


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
   *
   * This function does not lock any mutex and returns immediately.
   */
  bool compiled() const;

  bool has_format(const format_spec & format);

  bool has_format(std::string format);

  /** \brief Return a list of available output formats and their specifications
   *
   * See \ref format_description for more details on the precise information
   * provided.
   *
   * Even this method can only be called after compile() is called.
   */
  std::vector<format_description> available_formats();

  /** \brief Get canonical equivalent format specification
   *
   * See \ref format_provider::canonical_format().  Throws an exception if the
   * format is not available.
   *
   * Even this method can only be called after compile() is called.
   */
  format_spec canonical_format(const format_spec & format);


  /** \brief Get canonical equivalent format specification
   *
   * See \ref format_provider::canonical_format_or_empty().  Returns a
   * default-constructed empty \ref format_spec if the format is not available.
   *
   * Even this method can only be called after compile() is called.
   */
  format_spec canonical_format_or_empty(const format_spec & format);


  /** \brief Find the first availble format in the given list
   *
   * See \ref format_provider::find_format().
   *
   * This method can only be called after compile() is called.
   */
  template<typename IteratorInterfaceContainer>
  format_spec find_format(IteratorInterfaceContainer && formats);
  

  /** \brief Get output data for a requested format
   *
   * If the output data associated with \a format has not yet been computed, the
   * engine will run the necessary process to obtain the data and return it.
   * The data is cached such that future calls do not recompute the data.
   *
   * See also \ref engine_run_implementation::get_data_cref().
   *
   * The given \a format is transformed into canonical form before any
   * processing, so you don't have to worry which equivalent format_spec you
   * provide.
   *
   * If you only need read access to the data, consider using \ref
   * get_data_cref() which avoids an unnecessary copy.
   */
  binary_data get_data(const format_spec & format);

  /** \brief Get read-only output data for a requested format
   *
   * This function behaves exactly like \ref get_data(), except that it returns
   * a const reference to an internal data structure where the data is stored.
   * This method avoids an unecessary copy if you only need read-only access to
   * the data.  Do NOT attempt to modify the data please!
   */
  const binary_data & get_data_cref(const format_spec & format);


  // no copy, move, or assignment operators.
  run(const run &) = delete;
  run(run &&) = delete;
  run operator=(const run &) = delete;
  run & operator=(run &&) = delete;

private:

  std::unique_ptr<engine_run_implementation> _e;

  /**
   * \internal
   *
   * Having \a _compiled declared \a atomic enables some methods (like \ref
   * compiled()) to be declared const.  Otherwise if a member locks a mutex, it
   * must be non-const.
   */
  std::atomic<bool> _compiled;

  std::mutex _mutex;

  void _ensure_compiled() const;
};



// template members need to be in the .h, not in the .hxx which might be
// compiled separately in a single translation unit

template<typename IteratorInterfaceContainer>
inline format_spec run::find_format(IteratorInterfaceContainer && formats)
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  return _e->find_format(std::forward<IteratorInterfaceContainer>(formats));
}



} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/run.hxx>
#endif
