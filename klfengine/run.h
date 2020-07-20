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


struct forgot_to_call_compile : klfengine::exception
{
  forgot_to_call_compile()
    : exception("You forgot to call klfengine::run::compile()")
  { }
};

struct dont_call_compile_twice : klfengine::exception
{
  dont_call_compile_twice()
    : exception("You called klfengine::run::compile() twice")
  { }
};



/** \brief A format name and possible parameters
 *
 * The \a format name is conventinally an all-caps format name like "PNG" or
 * "PDF".  The \a parameters are implementation-defined arguments that can
 * change the "flavor" of the returned data, while still in the same format.
 * For instance, a "PDF" format could offer a <code>rasterize=true|false</code>
 * parameter indicating whether the PDF data should contain a rasterized picture
 * or vector graphics.
 *
 * \note We fix the convention that the JPEG format name is spelled out as
 *       "JPEG", not as "JPG".  We'll raise \ref no_such_format() if you use the
 *       format "JPG".
 *
 * \warning Currently, we fix the \a format name to be case-sensitive, and
 *          conventionally we declare that lowercase letters are illegal.
 */
struct format_spec
{
  std::string format;
  value::dict parameters;
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




struct no_such_format : klfengine::exception
{
  no_such_format(std::string fmt)
    : exception("No such format: " + std::move(fmt))
  { }

  no_such_format(std::string fmt, std::string message)
    : exception("No such format: " + std::move(fmt) + ": " + std::move(message))
  { }
};





class run;





/** \brief Interface that the run instance presents to the engine run implementation
 *
 * Engine run implementations might want to call methods on klfengine::run
 * objects.  But we don't want to let it call methods directly, for instance, if
 * the run instance manages concurrency and locks etc.  Instead, those methods
 * are presented via this interface to avoid deadlocks etc.
 *
 * ...................
 *
 */
struct run_instance_interface
{
  const klfengine::input & input;
  const klfengine::settings & settings;

  std::function<format_spec (const format_spec &)> canonical_format;

  std::function<const binary_data & (const format_spec &)> get_data_cref;
};





class engine_run_implementation
{
public:
  engine_run_implementation(run_instance_interface run_iface_)
    : _run_iface(std::move(run_iface_))
  {
  }

  /** \brief Do the initial compilation of the equation
   *
   * Engines should reimplement this member to actually perform any initial
   * compilation required for the given input.  (E.g., run \a latex.)
   */
  virtual void impl_compile() = 0;

  /** \brief Get a list of available formats
   *
   */
  virtual std::vector<std::string> impl_available_formats() const = 0;

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
   * function should return a \em canonical format_spec for the given \a
   * format_spec.  That is, the return value of this function should be the same
   * for any two equivalent \a format_spec s but different for any two
   * nonequivalent ones.
   *
   * 
   * \fixme TODO/FIXME: ....... DETERMINE HOW TO HANDLE UNKNOWN PARAMETERS.
   *        Should they be actively rejected (throw an error), or silently
   *        removed (possibly return wrong data flavor)?
   */
  virtual format_spec impl_make_canonical(const format_spec & format) = 0;

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
  virtual binary_data impl_produce_format(const format_spec & canon_format) = 0;


  /** \brief Query whether we can obtain the given format_spec
   *
   * The default implementation checks whether or not \a format.format is in the
   * list returned by \ref impl_available_formats().  Subclasses may do
   * something more refined, such as checking whether the given format
   * parameters are well-formed and the corresponding format is available for
   * this run.  (For instance, maybe some features provided via format arguments
   * are only available for given latex environments or ghostscript version,
   * etc.)
   *
   * Subclasses can assume that \a canon_format is in canonical form (see \ref
   * make_canonical()).
   */
  virtual bool impl_has_format(const format_spec & canon_format) const;


protected:
  const run_instance_interface & run_iface() const { return _run_iface; }

private:
  const run_instance_interface _run_iface;
};




/** \brief A compiling run of some latex code
 *
 * Instances of klfengine::run are returned by \ref klfengine::engine::run().
 *
 * ..................... OLD: All members of this base class are thread-safe.
 * Implementation subclasses' methods don't have to worry about thread safety as
 * they are protected by a class-instance-wide mutex lock.
 */
class run
{
public:
  run(std::shared_ptr<engine> engine_,
      std::unique_ptr<engine_run_implementation> engine_run_implementation_);

  ......................... fix creation order of objects interface/implementation/...  ....................

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
  bool compiled() const { return _compiled; }

  bool has_format(const format_spec & format) const;

  std::vector<std::string> available_formats() const;

  format_spec canonical_format(const format_spec & format);

  template<typename IteratorInterfaceContainer>
  format_spec find_format(const IteratorInterfaceContainer & formats);

  /** \brief Get result data associated
   *
   * The correct data format will be produced, if necessary (and if available).
   *
   * ............. DOC ...............
   */
  binary_data get_data(const format_spec & format);

  /** \brief Get result data associated with the given format (as a const reference)
   *
   * The correct data format will be produced, if necessary (and if available).
   *
   * ............. DOC ...............
   *
   * \warning A const reference is returned to an internal data structure.  The
   *          caller is responsible for not modifying the data pointed by the
   *          returned const reference.  The reference becomes invalid once this
   *          \ref run object instance is destroyed.
   *
   */
  const binary_data & get_data_cref(const format_spec & format);

  // no copy, move, or assignment operators.
  compilation(const compilation &) = delete;
  compilation(compilation &&) = delete;
  compilation operator=(const compilation &) = delete;
  compilation & operator=(compilation &&) = delete;


private:

protected:
  /** \brief Store the given format data to cache
   *
   * Future calls to get_format() will simply return the value stored in cache.
   *
   * This function assumes that the given format is specified in canonical form
   * (see \ref make_canonical()).
   *
   * The data is \em moved to the internal cache, and a const reference to it is
   * returned.  This is to avoid inadvertently creating copies of the binary
   * data.  That is, whoever calls this method should prepare the \ref
   * binary_data object, and by calling this method they yield ownerwhip of the
   * \a binary_data object to the cache (they should use std::move for this).
   * The caller can use the returned reference to maintain (const) access to the
   * data.
   */
  const binary_data &
  store_to_cache(const format_spec & canonical_format, binary_data && data);

  format_spec canonical_format_nolock(const format_spec & format) const;

  const binary_data & get_data_cref_nolock(const format_spec & canonical_format);

  /** \brief Return binary data from the cache
   *
   * If the given canonical format exists in the cache, return a const pointer
   * to the corresponding data.  If there is no such cache entry, return \a
   * nullptr.
   */
  const binary_data * get_cached_or_null(const format_spec & canonical_format) const;


private:

  std::shared_ptr<klfengine::engine> _engine;

  const input _input;
  const settings _settings;

  std::atomic<bool> _compiled;

  std::unordered_map<detail::fmtspec_cache_key_type>,binary_data> _cache;
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
