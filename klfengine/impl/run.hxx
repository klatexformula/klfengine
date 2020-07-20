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

#include <algorithm>

namespace klfengine {


namespace detail {


struct cache_entry_already_exists
  : klfengine::exception
{
  cache_entry_already_exists()
    : klfengine::exception(
        "Implementation error: you called store_to_cache() twice with the same format_spec"
        )
  {
  }
};

/** \brief Get the cache key for the given format
 *
 * \internal
 *
 * Produces a condensed form of the given \a format object into an internal
 * object type (this might be a std::string) used as the key to the map.
 *
 * \warning Assumes that \a format is in canonical form.
 *
 */
fmtspec_cache_key_type formatspec_cache_key(const format_spec & format)
{
  if (format.parameters.empty()) {
    return format.format;
  }
  return format.format + std::string("\0", 1) +
    nlohmann::json::json{format.parameters}.dump();
}

} // namespace detail


_KLFENGINE_INLINE run::run(
    std::shared_ptr<engine> engine_,
    input input_,
    settings settings_
    )
  : _engine(std::move(engine_)),
    _input(std::move(input_)),
    _settings(std::move(settings_)),
    _compiled(false)
{
}

_KLFENGINE_INLINE virtual run::~run()
{
}

_KLFENGINE_INLINE void run::compile()
{
  if (_compiled) { throw dont_call_compile_twice(); }

  std::lock_guard<std::mutex> lckgrd(_mutex);

  impl_compile();

  _compiled = true;
}

_KLFENGINE_INLINE bool run::has_format(const format_spec & format) const
{
  _ensure_compiled();

  std::lock_guard<std::mutex> lckgrd(_mutex);

  auto canon_fmt = canonical_format_nolock(format);

  auto ckey = detail::formatspec_cache_key(canon_fmt);
  if (!_cache.empty()) {
    if (_cache.find(ckey) != _cache.end()) {
      return true;
    }
  }

  return impl_has_format(canon_fmt);
}

_KLFENGINE_INLINE std::vector<std::string> run::available_formats() const
{
  return impl_available_formats();
}

_KLFENGINE_INLINE format_spec canonical_format(const format_spec & format)
{
  std::lock_guard<std::mutex> lckgrd(_mutex);
  return canonical_format_nolock(format);
}

_KLFENGINE_INLINE format_spec canonical_format_nolock(const format_spec & format)
{
  if (format.format == "JPG") {
    throw no_such_format(
        "JPG",
        "You misspelled format name ‘JPEG’ as ‘JPG’ (use the former exclusively please)"
        );
  }

  auto canon_fmt = impl_make_canonical(format);
  return canon_fmt;
}


_KLFENGINE_INLINE binary_data
run::get_data(const format_spec & format)
{
  std::lock_guard<std::mutex> lckgrd(_mutex);

  return get_data_cref_nolock(format);
}

_KLFENGINE_INLINE const binary_data &
run::get_data_cref(const format_spec & format)
{
  std::lock_guard<std::mutex> lckgrd(_mutex);

  return get_data_cref_nolock(format);
}

_KLFENGINE_INLINE const binary_data &
run::get_data_cref_nolock(const format_spec & format)
{
  _ensure_compiled();

  auto canon_fmt = canonical_format_nolock(format);
  auto ckey = detail::formatspec_cache_key(canon_fmt);

  auto cache_it = _cache.find(ckey);
  if (cache_it != _cache.end()) {
    // format exists in cache
    return cache_it->second;
  }

  // format does not yet exist, we need to produce it
  binary_data data = impl_produce_format(canon_fmt);

  auto result = _cache.insert(
      std::pair<detail::fmtspec_cache_key_type,binary_data>(std::move(ckey),
                                                            std::move(data))
      );

  // NOTE: ckey is invalidated by the above std::move(), use result.first->first

  if (!result.second) {
    // It is a error if the subclass registered the data for canon_fmt already.
    // This should not happen.
    throw detail::cache_entry_already_exists();
  }

  const binary_data & dref = result.first->second;
  return dref;
}


_KLFENGINE_INLINE bool run::impl_has_format(const format_spec & format) const
{
  auto avail_fmts = available_formats();

  return (
      std::find(avail_fmts.begin(), avail_fmts.end(), format.format)
      == avail_fmts.end()
      ) ;
}

_KLFENGINE_INLINE const binary_data &
run::store_to_cache(const format_spec & canon_fmt, binary_data && data)
{
  auto ckey = detail::formatspec_cache_key(canon_fmt);

  auto result = _cache.insert(
      // `data` is already an rvalue-reference, so no std::move(data) here:
      std::pair<detail::fmtspec_cache_key_type,binary_data>(std::move(ckey), data)
      );

  if (!result.second) {
    // insert() failed, entry already exists. This should not happen.
    throw detail::cache_entry_already_exists();
  }

  const binary_data & dref = result.first->second;
  return dref;
}

_KLFENGINE_INLINE const binary_data *
run::get_cached_or_null(const format_spec & canonical_format) const
{
  auto ckey = detail::formatspec_cache_key(canonical_format);
  const auto it = _cache.find(ckey);
  if (it == _cache.end()) {
    return nullptr;
  }
  return & it->second;
}



_KLFENGINE_INLINE void run::_ensure_compiled() const
{
  if (!_compiled) {
    throw forgot_to_call_compile();
  }
}







} // namespace klfengine


