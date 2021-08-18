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
#include <klfengine/value>


#include <nlohmann/json.hpp>


namespace klfengine {


/** \brief A format name and possible parameters
 *
 * The \a format name is conventinally an all-caps format name like "PNG" or
 * "PDF".  The \a parameters are implementation-defined arguments that can
 * change the "flavor" of the returned data, while still in the same format.
 * For instance, a "PNG" format could offer a <code>dpi=XXX</code> parameter
 * indicating the dots-per-inch resolution to which the vector output should be
 * rasterized into the resulting PNG image.
 *
 * \warning We fix the convention that the JPEG format name is spelled out as
 *          "JPEG", not as "JPG".  Format providers will raise \ref
 *          no_such_format if you use the format name "JPG".
 *
 * \warning Please use only upper case format names.  We fixed upper case naming
 *          by convention.
 */
struct format_spec
{
  format_spec()
    : format(), parameters()
  {}
  format_spec(const format_spec & other) = default;
  format_spec(format_spec && other) = default;
  explicit format_spec(std::string format_)
    : format(std::move(format_)), parameters()
  {}
  format_spec(std::string format_, value::dict parameters_)
    : format(std::move(format_)), parameters(std::move(parameters_))
  {}
  ~format_spec() { }

  std::string format;
  value::dict parameters;

  std::string as_string() const;

  format_spec & operator=(std::string other_) {
    format = std::move(other_);
    parameters = value::dict{};
    return *this;
  }
  format_spec & operator=(const format_spec &) = default;
  format_spec & operator=(format_spec &&) = default;
};


inline bool operator==(const format_spec & a, const format_spec & b)
{
  return (a.format == b.format && a.parameters == b.parameters);
}
inline bool operator!=(const format_spec & a, const format_spec & b)
{
  return !(a == b);
}


void to_json(nlohmann::json & j, const format_spec & v);
void from_json(const nlohmann::json & j, format_spec & v);


/** \brief A format specification along with a short title and description
 *
 * A combination of a \ref format_spec along with a short title and description.
 * This type is used by run::available_formats(), etc.
 *
 * The \a title is meant to describe the given \a format very briefly, so it can
 * be used for instance in a format selection drop-down menu.  The \a
 * description might include a little more information, and might perhaps be
 * suitable for a mouse-over tooltip or a separate widget that can display
 * additional text describing this format.
 *
 * The \a format_spec can contain a parameter map that describes the possible
 * parameters that can be set.  For each parameter, we expect the value in the
 * parameter map to have the special structure
 * \code
 *  {
 *    "type": "bool"|"int"|"double"|"string"|"array"|"dict"|"color"|"length"|"margins",
 *    "null_ok": true|false,
 *    "validate_regex": <regex>, // applicable to bool,int,double,string
 *    "default": <default value>
 *    "dict_keys": { ... } // recursive, if type=="dict"
 *  }
 * \endcode
 *
 * The parameter structure isn't validated in any way by \a klfengine base
 * functions.  They are an indication of what type of user input forms should be
 * presented to the user for them to specify the format parameters.
 */
struct format_description
{
  klfengine::format_spec format_spec;
  std::string title;
  std::string description;
};



void to_json(nlohmann::json & j, const format_description & v);
void from_json(const nlohmann::json & j, format_description & v);



/** \brief Raised when the requested format is invalid or not available
 *
 * The \a fmt argument will be reported as the format that was impossible to
 * deliver.  An optional \a message can be used to specify why the format was
 * not available.  Both these arguments are combined into the output of the
 * exception's standard \a what() method.
 */
class no_such_format : public klfengine::exception
{
public:
  no_such_format(std::string fmt)
    : exception("No such format: " + std::move(fmt))
  { }

  no_such_format(std::string fmt, std::string message)
    : exception("No such format: " + std::move(fmt) + ": " + std::move(message))
  { }
};






/** \brief Abstract base class for objects that provide format output
 *
 */
class format_provider
{
public:
  /** \brief Return the format specification in canonical form
   *
   * Throws \ref klfengine::no_such_format if the given format is invalid or is
   * not available.
   */
  format_spec canonical_format(const format_spec & format);

  /** \brief Return the format specification in canonical form
   *
   * This method never throws \ref klfengine::no_such_format; if the given
   * format is invalid or is not available, it returns a default-constructed
   * \ref format_spec (with an empty format string).
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
   * (The \a format need not be in canonical form.  Actually, the way \a
   * has_format() works is that it attempts to get the canonical form for the
   * given \a format, and any error means that the format is not available.)
   */
  bool has_format(const format_spec & format);

  /** \brief Check if a given format is available.
   *
   * Overloaded method, provided for convenience.
   */
  bool has_format(std::string format);


  /** \brief Find a suitable format
   *
   * Returns the first available format from the given list of \a formats.
   *
   * Formats might be a list of \a string s for \a format_spec s.  This function
   * works by considering each given format in order and attempting to get its
   * canonical form.  If the canonicalization succeeds, this means the format is
   * available and this function returns the canonical format.  If the
   * canonicalization fails, the format is unavailable and the next given format
   * is considered.
   *
   * If none of the given formats are available, \ref klfengine::no_such_format
   * is thrown.
   */
  template<typename IteratorInterfaceContainer>
  format_spec find_format(IteratorInterfaceContainer && formats);


private:

  /** \brief Get a list of available formats
   *
   * Returns a list of formats that this run instance can produce, as a vector
   * of \ref format_description objects.  Each format description should include
   * a short title and a brief description for each format specification, and if
   * possible a specification of accepted parameters (see \ref
   * format_description).
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
   * By convention, it's preferable to choose a 'canonical format' in which all
   * relevant parameter keys are present (e.g. select as canonical choices
   * <code>('PDF', {"raw": false}), ('PDF', {"raw": true})</code> instead of
   * <code>('PDF', {}), ('PDF', {"raw": true})</code>)  It's easer that way to
   * access the underlying value of these parameters and can allow the defaults
   * to change.
   *
   * If the format is invalid, or cannot be delivered, this implementation may
   * choose to:
   *
   * - either throw \ref klfengine::no_such_format with an optional description
   *   of why this format is not available
   *
   * - or return an empty (default-constructed) \ref format_spec.  In this case
   *   \ref canonical_format() will automatically detect this and throw a \ref
   *   klfengine::no_such_format exception.
   *
   * If \a check_available_only is \a true, then the subclass doesn't actually
   * have to compute the canonical form of \a format, it only needs to check
   * that the format is available.  In this case, the subclass should return any
   * (arbitrary) non-empty format_spec if the format is available, and do either
   * of the above two points if the format is unavailable (i.e., return an empty
   * format_spec or raise \ref klfengine::no_such_format).
   */
  virtual format_spec impl_make_canonical(const format_spec & format,
                                          bool check_available_only) = 0;



private:
  /*---*
   * \internal
   *
   * Internal call that implements canonical_format by wrapping the user's
   * canonical_format into some additional checks.
   *
   * This behaves like canonical_format() if check_available_only is false.
   *
   * If check_available_only is set, then:
   *
   *  - the return value is undefined and is to be discarded
   *
   * - if the format is not available, \ref klfengine::no_such_format is always
   *   raised
   *
   *  - if the format is available, no exception is raised.
   */
  format_spec internal_canonical_format(const format_spec & format,
                                        bool check_available_only);
};




// template members need to be in the .h, not in the .hxx which might be
// compiled separately in a single translation unit

template<typename IteratorInterfaceContainer>
inline format_spec format_provider::find_format(IteratorInterfaceContainer && formats)
{
  // note container value type can also be std::string, because you can
  // construct a format_spec from a std::string

  for (auto it = formats.begin(); it != formats.end(); ++it) {
    format_spec canon = canonical_format_or_empty( format_spec{*it} );
    if (!canon.format.empty()) {
      return canon;
    }
  }

  // reached the end, didn't find a suitable format
  throw no_such_format("<no suitable format found in given list>");
}

} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/format.hxx>
#endif
