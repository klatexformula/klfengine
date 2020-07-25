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




// TODO/FIXME: implement possible separate hxx compilation into separate
// translation unit
#define _KLFENGINE_INLINE inline




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



} // namespace klfengine
