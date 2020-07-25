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

#include <string>
#include <vector>
#include <exception>


namespace klfengine {

/** \brief Storage type for (binary) data resulting from a compilation
 *
 * Currently we only guarantee that binary_data provides vector-like or
 * string-like C++ STL iterator access via \a begin() and \a end().  For
 * instance, we might change the typedef to \a std::string depending on what
 * works best in implementations.
 *
 * \fixme FIXME: Determine what interface binary_data allows.  
 *
 */
using binary_data = std::vector<std::uint8_t>;


/** \brief Exception class for klfengine
 *
 * Errors reported by the klfengine library are thrown as exceptions which
 * inherit \ref klfengine::exception.
 */
class exception : public std::exception
{
public:
  /** \brief Constructor with full error message
   *
   */
  explicit exception(std::string msg_)
    : _msg(std::move(msg_))
  {
  }
  /** \brief Constructor with full error message provided as C-style string
   *
   * The string is copied to an internal member so the pointed data does not
   * need to outlive the constructor call.
   */
  explicit exception(const char * msg_c_str)
    : _msg(msg_c_str)
  {
  }
  virtual ~exception() = default;

  /** \brief Returns the error message provided to the constructor
   */
  inline const char * what() const noexcept { return _msg.c_str(); }

private:
  std::string _msg;
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
