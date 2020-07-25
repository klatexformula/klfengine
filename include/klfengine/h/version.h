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

#include <string>


#include <klfengine/h/version_number_raw.h>

/** \def KLFENGINE_VERSION_MAJOR
 *
 * \brief Expands to an integer representing the current major version of the
 *        klfengine library
 *
 * See \ref klfengine::version().
 */
/** \def KLFENGINE_VERSION_MINOR
 *
 * \brief Expands to an integer representing the current minor version of the
 *        klfengine library
 *
 * See \ref klfengine::version().
 *
 */
/** \def KLFENGINE_VERSION_RELEASE
 *
 * \brief Expands to an integer representing the current release version of the
 *        klfengine library
 *
 * See \ref klfengine::version().
 *
 */
/** \def KLFENGINE_VERSION_SUFFIX
 *
 * \brief Expands to a (double-quoted) C string literal representing the current
 *        suffix version of the klfengine library
 *
 * See \ref klfengine::version().
 *
 */



#define _KLFENGINE_STRINGIFY(x)  #x
#define _KLFENGINE_CONCAT_VER_3(a, b, c)        \
  _KLFENGINE_STRINGIFY(a.b.c)
#define _KLFENGINE_CONCAT_VER_4(a, b, c, dstr)  \
  ( _KLFENGINE_CONCAT_VER_3(a, b, c)   dstr )

/** \brief String representing the current klfengine version
 *
 * This macro expands to a C string literal (or an equivalent expression which
 * might be parenthesized).  You can use this in a \a constexpr context, for
 * instance.
 */
#define KLFENGINE_VERSION_STRING                \
  _KLFENGINE_CONCAT_VER_4(                      \
      KLFENGINE_VERSION_MAJOR,                  \
      KLFENGINE_VERSION_MINOR,                  \
      KLFENGINE_VERSION_RELEASE,                \
      KLFENGINE_VERSION_SUFFIX )


namespace klfengine {

/** \brief Version information with major, minor, release, suffix parts
 *
 */
struct version_info {
  int major;
  int minor;
  int release;
  std::string suffix;
};

// can't have constexpr version_info because of std::string [ :-( ] and having
// individual consts like this seems too redundant given that we already have
// the KLFENGINE_VERSION_XXXXX macros:
//
//constexpr static int  version_major = KLFENGINE_VERSION_MAJOR;
//constexpr static int  version_minor = KLFENGINE_VERSION_MINOR;
//constexpr static int  version_release = KLFENGINE_VERSION_RELEASE;
//constexpr static char version_suffix[] = KLFENGINE_VERSION_SUFFIX;




/** \brief Return the current library version
 *
 * The version is the one that is reported by the headers themselves.  That is
 * if you're compiling headers & library separately, depending on your setup
 * this might differ from implementation_version().
 *
 * That is, this function is guaranteed to be implemented inline and will
 * coincide with the values reported by the macros \ref KLFENGINE_VERSION_MAJOR,
 * \ref KLFENGINE_VERSION_MINOR, \ref KLFENGINE_VERSION_RELEASE, and \ref
 * KLFENGINE_VERSION_SUFFIX.
 */
inline version_info version()
{
  return version_info{  KLFENGINE_VERSION_MAJOR,
                        KLFENGINE_VERSION_MINOR,
                        KLFENGINE_VERSION_RELEASE,
                        KLFENGINE_VERSION_SUFFIX  };
}

/** \brief Return the current library implementation version
 *
 * This method, like \ref version(), returns a version_info instance
 * representing the current library version.  However, this method is guaranteed
 * to be implemented in the implementation part of the library
 * (<code>klfengine/impl/version.hxx</code>).  In case you decide to compile the
 * implementation separately into a different compilation unit, the return value
 * of implementation_version() could be different than that of \ref version() if
 * you compiled the implementation with a different version of klfengine.
 *
 * \warning Don't compile the implementation with a different version of
 *          klfengine, that's just asking for trouble.
 *
 * \bug  FIXME/TODO: Determine what level of compatibility we want to guarantee
 *       between headers and implementation.  Standard libraries guarantee
 *       binary compatibility for same major versions.  We might simply bump the
 *       major version pretty often.
 */
version_info implementation_version();


} // namespace klfengine


#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/version.hxx>
#endif
