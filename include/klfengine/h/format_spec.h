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
 *          "JPEG", not as "JPG".  Engines will raise \ref no_such_format if you
 *          use the format name "JPG".
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
 */
struct format_description
{
  klfengine::format_spec format_spec;
  std::string title;
  std::string description;
};






/** \brief Raised when the requested format is invalid or not available
 *
 * The \a fmt argument will be reported as the format that was impossible to
 * deliver.  An optional \a message can be used to specify why the format was
 * not available.  Both these arguments are combined into the output of the
 * exception's standard \a what() method.
 */
struct no_such_format : exception
{
  no_such_format(std::string fmt)
    : exception("No such format: " + std::move(fmt))
  { }

  no_such_format(std::string fmt, std::string message)
    : exception("No such format: " + std::move(fmt) + ": " + std::move(message))
  { }
};







} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/format_spec.hxx>
#endif
