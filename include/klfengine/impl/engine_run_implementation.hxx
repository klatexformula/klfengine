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

#include <klfengine/engine_run_implementation>

#include <nlohmann/json.hpp>


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
_KLFENGINE_INLINE
fmtspec_cache_key_type formatspec_cache_key(const format_spec & format)
{
  if (format.parameters.empty()) {
    return format.format;
  }
  return format.format + std::string("\0", 1) +
    nlohmann::json{format.parameters}.dump();
}

} // namespace detail





_KLFENGINE_INLINE engine_run_implementation::engine_run_implementation(
    klfengine::input input_,
    klfengine::settings settings_
    )
  : _input(std::move(input_)),
    _settings(std::move(settings_))
{
}

_KLFENGINE_INLINE engine_run_implementation::~engine_run_implementation()
{
}


_KLFENGINE_INLINE
void engine_run_implementation::compile()
{
  // note: klfengine::run::compile() already checks that compile() isn't called
  // twice.

  impl_compile();
}


_KLFENGINE_INLINE bool
engine_run_implementation::has_format(std::string format)
{
  return has_format( format_spec{ std::move(format) });
}

_KLFENGINE_INLINE bool
engine_run_implementation::has_format(const format_spec & format)
{
  try {
    (void) internal_canonical_format(format, true);
  } catch (no_such_format & /*exc*/) {
    return false;
  }
  return true;
}

_KLFENGINE_INLINE format_spec
engine_run_implementation::canonical_format(const format_spec & format)
{
  return internal_canonical_format(format, false);
}

_KLFENGINE_INLINE format_spec
engine_run_implementation::canonical_format_or_empty(const format_spec & format)
{
  try {
    return internal_canonical_format(format, false);
  } catch (no_such_format & ex) {
    return format_spec{};
  }
}

_KLFENGINE_INLINE format_spec
engine_run_implementation::internal_canonical_format(const format_spec & format,
                                                     bool check_available_only)
{
  if (format.format == "JPG") {
    throw no_such_format(
        "JPG",
        "You misspelled format name ‘JPEG’ as ‘JPG’ (use the former exclusively please)"
        );
  }

  auto canon_fmt = impl_make_canonical(format, check_available_only);

  if (canon_fmt.format.empty()) {
    throw no_such_format(format.format, "format is unknown or is not available");
  }

  return canon_fmt;
}


_KLFENGINE_INLINE std::vector<format_description>
engine_run_implementation::available_formats()
{
  return impl_available_formats();
}

_KLFENGINE_INLINE const binary_data &
engine_run_implementation::get_data_cref(const format_spec & format)
{
  auto canon_fmt = canonical_format(format);
  auto ckey = detail::formatspec_cache_key(canon_fmt);

  auto cache_it = _cache.find(ckey);
  if (cache_it != _cache.end()) {
    // format exists in cache
    return cache_it->second;
  }

  // format does not yet exist, we need to produce it
  binary_data data = impl_produce_data(canon_fmt);

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


// _KLFENGINE_INLINE bool
// engine_run_implementation::impl_has_format(const format_spec & format) const
// {
// .....
// }


_KLFENGINE_INLINE const binary_data &
engine_run_implementation::store_to_cache(
    const format_spec & canon_fmt,
    binary_data && data
    )
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






} // namespace klfengine
