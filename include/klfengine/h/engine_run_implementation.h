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
#include <klfengine/format_spec>


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
 * Subclasses are responsible for the following tasks:
 *
 * - Report which formats are available, and which format specification
 *   parameters actually represent the same data.  Methods that need to be
 *   reimplemented are \ref impl_available_formats() and \ref
 *   impl_make_canonical()
 *
 * - Perform any initial compilation steps that are common for all formats that
 *   can be requested later (e.g., run latex).  For this reimplement \ref
 *   impl_compile().
 *
 * - Produce the data in a given format.  Reimplement \ref
 *   impl_produce_format().
 *
 * It might be assumed by users that the initial compilation step (implemented
 * in \ref impl_compile()) will be more computationally intensive than
 * converting the result to a specific requested format (\ref
 * impl_produce_format()).  That is, avoid leaving all the work to
 * impl_produce_format().
 *
 * Methods of \ref run instances will typically directly call the corresponding
 * methods here.  Subclasses of this class should prefer calling public members
 * of this base class (such as get_format_cref() or canonical_format()) rather
 * than recursively calling their own implementation methods such as
 * impl_produce_format().
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
class engine_run_implementation
{
public:
  engine_run_implementation(klfengine::input input_,
                            klfengine::settings settings_);
  virtual ~engine_run_implementation();

  const klfengine::input & input() const { return _input; }
  const klfengine::settings & settings() const { return _settings; }

  /** \brief Perform any initial compilation steps.
   *
   * Subclasses should \b NOT call this.  The caller (a \ref run instance) is
   * responsible for ensuring that this method is called once, and once only,
   * before calling any other method of this class.
   */
  void compile();

  /** \brief Return the format specification in canonical form
   *
   * Throws \ref no_such_format if the given format is invalid or is not
   * available.
   */
  format_spec canonical_format(const format_spec & format);

  /** \brief Return the format specification in canonical form
   *
   * This method never throws \ref no_such_format; if the given format is
   * invalid or is not available, it returns a default-constructed \ref
   * format_spec (with an empty format string).
   */
  format_spec canonical_format_or_empty(const format_spec & format);

  /** \brief Return a list of available formats
   *
   * Returns a list of formats that this run instance can produce, as a vector
   * of \ref format_description objects.  Each format description should include
   * a short title and a brief description for each format specification (see
   * \ref format_description).
   *
   * Subclasses should reimplement \ref impl_available_formats().
   */
  std::vector<format_description> available_formats();


  /** \brief Check if a given format is available.
   *
   * Returns TRUE if the given format can be produced, or FALSE otherwise.
   *
   * (The \a format need not be in canonical form.  Actually, a format in
   * canonical form that is non-empty is a format that is available.)
   */
  bool has_format(const format_spec & format);

  /** \brief Check if a given format is available.
   *
   * Overloaded method, provided for convenience.
   */
  bool has_format(std::string format);


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


  /** \brief Get a list of available formats
   *
   * Returns a list of formats that this run instance can produce, as a vector
   * of \ref format_description objects.  Each format description should include
   * a short title and a brief description for each format specification (see
   * \ref format_description).
   *
   * \todo BUG/TODO/FIXME: We cannot ask subclasses to return ALL possible
   * format_spec's.  Also, a UI would probably better ask for a format first,
   * and then present an UI for the parameters. ........... NEED DESIGN DECISION
   * HERE .......
   */
  virtual std::vector<format_description> impl_available_formats() = 0;

  /** \brief Canonicalize the format specification
   *
   * Sometimes different \a format_spec instances are in fact equivalent.  For
   * instance, the <code>format_spec{"PNG"}</code> might be equivalent to
   * <code>format_spec{"PNG", {value::dict{{"raw", value{false}}}}}</code>.  The
   * engine should know of such equivalences of format_spec's, because if say
   * the latter format_spec is requested when we have already compiled the
   * former, we don't need to compile anything again and simply return the
   * cached data.
   *
   * To inform our engine of such equivalences between \a format_spec s, this
   * function should return a \em canonical \ref format_spec for the given \a
   * format.  That is, the return value of this function should be the same for
   * any two equivalent \a format_spec s but different for any two nonequivalent
   * ones.
   *
   * If the format is invalid, or cannot be delivered, this implementation may
   * choose to:
   *
   * - either throw \ref no_such_format with an optional description of why
   *   this format is not available
   *
   * - or return an empty (default-constructed) \ref format_spec.  In this case
   *   \ref canonical_format() will automatically detect this and throw a \ref
   *   no_such_format exception.
   *
   * If \a check_available_only is \a true, then the subclass doesn't actually
   * have to compute the canonical form of \a format, it only needs to check
   * that the format is available.  In this case, the subclass should return any
   * (arbitrary) non-empty format_spec if the format is available, and do either
   * of the above two points if the format is unavailable (i.e., return an empty
   * format_spec or raise \ref no_such_format).
   */
  virtual format_spec impl_make_canonical(const format_spec & format,
                                          bool check_available_only) = 0;

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

  /**
   * \internal
   *
   * Like canonical_format() if check_available_only is false.
   *
   * If check_available_only is set, then:
   *
   *  - the return value is undefined and is to be discarded
   *
   *  - if the format is not available, \ref no_such_format is always raised
   *
   *  - if the format is available, no exception is raised.
   */
  format_spec internal_canonical_format(const format_spec & format,
                                        bool check_available_only);

};





} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/engine_run_implementation.hxx>
#endif
