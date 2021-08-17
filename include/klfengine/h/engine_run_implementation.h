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

#include <klfengine/basedefs>
#include <klfengine/input>
#include <klfengine/settings>
#include <klfengine/format>


namespace klfengine {



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





/** \brief An engine's implementation of a compilation run of some latex code
 *
 * This abstract class should be subclassed by specific engines to implement the
 * core of their functionality.  The object is constructed with some \ref
 * klfengine::input and \ref klfengine::settings objects (these should remain
 * constant throughout the lifetime of this instance).
 *
 * .. note:: You should never have to construct engine_run_implementation
 *           instances directly when you want to use an engine.  When
 *           implementing a new engine, you should only create a
 *           engine_run_implementation instance when reimplementing \ref
 *           klfengine::impl_create_engine_run_implementation().
 *  
 *
 * Subclasses are responsible for the following tasks:
 *
 * - Perform any initial compilation steps that are common for all formats that
 *   can be requested later (e.g., run latex).  For this reimplement \ref
 *   impl_compile().
 *
 * - Report which formats are available, and which format specification
 *   parameters actually represent the same data.  Methods that need to be
 *   reimplemented are \ref impl_available_formats() and \ref
 *   impl_make_canonical()
 *
 * - Produce the data in a given format.  Reimplement \ref
 *   impl_produce_data().
 *
 * It might be assumed by users that the initial compilation step (implemented
 * in \ref impl_compile()) will be more computationally intensive than
 * converting the result to a specific requested format (\ref
 * impl_produce_data()).  That is, avoid leaving all the work to
 * impl_produce_data().
 *
 * Methods of \ref run instances will typically directly call the corresponding
 * methods here.  Subclasses of this class should prefer calling public members
 * of this base class (such as get_format_cref() or canonical_format()) rather
 * than recursively calling their own implementation methods such as
 * impl_produce_data().
 *
 * <b>Private, protected, or public virtual?</b>
 *
 * - the virtual methods \a impl_ should in principle have been declared
 *   private: See <a href="https://stackoverflow.com/a/3978552/1694896">this
 *   answer on SO</a> (among many others) and <a
 *   href="http://www.gotw.ca/publications/mill18.htm">this article by Herb
 *   Sutter</a> on why private virtual is a good thing.
 *
 * - but the issue is that many of these functions, such as impl_compile(), need
 *   to be called directly by \ref run instances.  Exposing a public non-virtual
 *   \ref compile() method here would not solve the issue because the
 *   compilation should only happen once anyways
 *
 */
class engine_run_implementation : public format_provider
{
public:
  /** \brief Constructor */
  engine_run_implementation(klfengine::input input_,
                            klfengine::settings settings_);
  /** \brief Destructor */
  virtual ~engine_run_implementation();

  /** \brief The klfengine::input data associated with this run instance
   *
   * Returns the \a input data that was provided to the constructor.  The input data
   * remains unchanged during the lifetime of this instance.
   */
  const klfengine::input & input() const { return _input; }
  /** \brief The klfengine::settings data associated with this run instance
   *
   * Returns the \a settings data that was provided to the constructor.  The
   * settings data remains unchanged during the lifetime of this instance.
   */
  const klfengine::settings & settings() const { return _settings; }

  /** \brief Perform any initial compilation steps.
   *
   * Subclasses should \b NOT call this.  The caller (a \ref run instance) is
   * responsible for ensuring that this method is called once, and once only,
   * before calling any other method of this class.
   */
  void compile();


  /** \brief Get result data associated with the given format
   *
   * This method checks the internal cache to see if the specified format was
   * already produced.  If so, a reference to the data is returned.  If not, the
   * format is produced and stored to the cache, and then a reference to the
   * corresponding data is returned.
   *
   * If the specified format is not available, a \ref no_such_format exception
   * is thrown.
   *
   * The \a format is not assumed to be in canonical form.
   *
   * The lifetime of the returned reference is the same as the lifetime of the
   * current class instance.
   *
   * \warning A const reference is returned to an internal data structure.  The
   *          caller is responsible for not modifying the data pointed by the
   *          returned const reference.  The reference becomes invalid once this
   *          \ref run object instance is destroyed.
   *
   */
  const binary_data & get_data_cref(const format_spec & format);


private:

  /** \brief Do the initial compilation of the equation
   *
   * Engines should reimplement this member to actually perform any initial
   * compilation required for the given input.  (E.g., run \a latex.)
   */
  virtual void impl_compile() = 0;


  /** \brief Return the data associated with the given canonical format
   *
   * Subclasses should here process whatever needs to be processed to obtain the
   * given \a format.  (E.g., run ghostscript, ...)
   *
   * The returned data is automatically stored into the cache for the given \a
   * format.  If intermediary formats were created while producing the specified
   * \a format, the intermediary format data can be stored to cache by calling
   * \ref store_to_cache().
   *
   * Subclasses can assume that \a canon_format is in canonical form (see \ref
   * make_canonical()).
   */
  virtual binary_data impl_produce_data(const format_spec & canon_format) = 0;


protected:

  /** \brief Store the given format data to cache
   *
   * Future calls to get_data_cref() will simply return the value stored in
   * cache.
   *
   * This function assumes that the given format is specified in canonical form
   * (see \ref impl_make_canonical()).
   *
   * The data is \em moved to the internal cache, and a const reference to it is
   * returned.  This is to avoid inadvertently creating copies of the binary
   * data.  That is, whoever calls this method should prepare the \ref
   * binary_data object, and by calling this method they yield ownerwhip of the
   * \ref binary_data object to the cache (they should use std::move for this).
   * The caller can use the returned reference to maintain (const) access to the
   * data.
   */
  const binary_data &
  store_to_cache(const format_spec & canonical_format, binary_data && data);


private:
  const klfengine::input _input;
  const klfengine::settings _settings;

  std::unordered_map<detail::fmtspec_cache_key_type,binary_data> _cache;

};





} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/engine_run_implementation.hxx>
#endif
