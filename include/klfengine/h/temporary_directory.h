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

#include <klfengine/h/detail/filesystem.h>

namespace klfengine {


namespace detail {

constexpr int temp_dir_default_max_tries = 100;
constexpr int temp_dir_default_num_rand_chars = 16;

} // namespace detail


/** \brief Create and automatically remove a temporary directory
 *
 * The temporary directory is created at construction time according to the
 * given name templates and base location.  If \a auto_delete is set (\ref
 * set_auto_delete(), which is on by default), then the directory is removed
 * when this object instance goes out of scope and the destructor gets called.
 *
 * <b>A note on C++11 move semantics:</b>
 * This class provies a C++11 move constructor which behaves as you'd expect,
 * i.e., it takes over the ownership of the temporary_directory.  That is, you
 * can move a temporary_directory to create another temporary_directory, and the
 * temporary directory will not be removed after the first object goes out of
 * scope (but don't access that first object any longer if it's been moved).
 * However there is no move \em assignment operator, because if you create a
 * temporary_directory instance the temporary directory is also automatically
 * physically created; we'd like to discourage creating a useless temporary
 * directory while creating a \a temporary_directory only to delete it
 * immediatly when we move another \a temporary_directory into this instance.
 */
class temporary_directory {
public:
  /** \brief Create temporary directory in a system default location
   *
   * The system temporary directory is obtained from the filesystem library, and
   * an empty dir name prefix is used.
   */
  temporary_directory();

  /** \brief Create temporary directory in the given location with given prefix etc.
   *
   * \param temp_dir  The location where to create the temporary directory (e.g.,
   *                  "/tmp").  You can specify a default-constructed \a
   *                  fs::path to use the system default temporary directory.
   *
   * \param name_prefix  A prefix to use for the name of the temporary
   *                     directory. The directory name will be created by
   *                     concatenating the prefix with a random sequence of
   *                     characters.
   *
   * \param num_rand_chars  The number of random alphanumeric characters to use
   *                        in the random part of the directory name (after the
   *                        prefix)
   *
   * This constructor throws exceptions on failures.
   */
  explicit temporary_directory(
      fs::path temp_dir,
      std::string name_prefix = std::string(),
      int num_rand_chars = detail::temp_dir_default_num_rand_chars
      );
  ~temporary_directory();

  /** \brief The path to the temporary directory that was created
   */
  inline fs::path path() const { return _path; }

  /** \brief Query whether or not auto-delete was enabled
   *
   * See \ref set_auto_delete().
   */
  inline bool auto_delete() { return _auto_delete; }
  /** \brief Set automatic removal of temporary directory on instance destruction
   *
   * If auto-delete is set, then the temporary directory (i.e., including any
   * subfolders and files) is deleted from the filesystem after this object goes
   * out of scope and the destructor gets called.  Use
   * <code>set_auto_delete(false)</code> to inhibit this behavior.
   *
   * See class documentation about C++11 move semanics.  If a \a
   * temporary_directory instance is moved to create another
   * temporary_directory, the former object's destructor will do nothing and it
   * is the latter object that will delete the directory upon destruction (if
   * auto-delete was set; the auto-delete property is inherited by the move).
   */
  inline void set_auto_delete(bool auto_delete) { _auto_delete = auto_delete; }

  // default move constructor
  temporary_directory(temporary_directory &&) = default;
  // but NO move assignment, because we wouldn't know what to do with the
  // existing temporary directory we created in the constructor.  (We could
  // delete it here; but we want to discourage creating useless temporary
  // directories.)
  temporary_directory & operator=(temporary_directory &&) = delete;

  // no copy semantics
  temporary_directory(const temporary_directory &) = delete;
  temporary_directory & operator=(const temporary_directory &) = delete;

private:
  // an empty path specified to temp_dir will be replaced by the default
  // obtained from std::filesystem::temp_directory_path()
  static fs::path create_temporary_dir(
    fs::path temp_dir = fs::path(),
    std::string name_prefix = std::string(),
    int num_rand_chars = detail::temp_dir_default_num_rand_chars,
    int max_tries = detail::temp_dir_default_max_tries
  );

  fs::path _path;
  bool _auto_delete;
};




} // namespace klfengine




#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/temporary_directory.hxx>
#endif
