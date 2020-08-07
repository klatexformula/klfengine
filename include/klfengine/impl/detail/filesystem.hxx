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

#include <algorithm> // std::transform
#include <regex>

#include <klfengine/basedefs>
#include <klfengine/h/detail/filesystem.h>

namespace klfengine {

namespace detail {


/**
 * \internal
 *
 * Returns the number of hits.
 */
template<typename FnStoreHit>
int find_wildcard_path_impl(
    const fs::path & base,
    std::vector<fs_w_part>::const_iterator wildcard_expressions_begin,
    std::vector<fs_w_part>::const_iterator wildcard_expressions_end,
    int limit,
    FnStoreHit && store_hit
    )
{
  // std::cerr << "DEBUG: base=" << base << "; wildcard_expressions=";
  // for (std::vector<fs_w_part>::const_iterator it = wildcard_expressions_begin;
  //      it != wildcard_expressions_end; ++it) {
  //   if (_KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<std::string>(*it)) {
  //     std::cerr << " " << _KLFENGINE_VARIANT_GET<std::string>(*it);
  //   } else if (_KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<std::regex>(*it)) {
  //     std::cerr << " " << "<regex>";
  //   }
  // }
  // std::cerr << " limit=" << limit << "\n";

  // recursively explore base's children, finding matching wildcards.
  if (limit == 0) {
    // already exceeded limit
    return 0;
  }
  
  if (wildcard_expressions_begin == wildcard_expressions_end) {
    // base is what we're looking for
    store_hit(base);
    return 1;
  }

  const auto & w = *wildcard_expressions_begin;

  ++wildcard_expressions_begin;

  if ( _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<std::string>(w) ) {
    const std::string & item = _KLFENGINE_VARIANT_GET<std::string>(w);
    // next path item is fixed string -- descend into it
    fs::path next = base / item;
    if ( ! fs::exists(next) ) {
      // path doesn't exist
      return 0;
    }
    int n = find_wildcard_path_impl(
        next,
        wildcard_expressions_begin,
        wildcard_expressions_end,
        limit,
        store_hit
        );
    return n;
  }

  if ( _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<std::regex>(w) ) {

    int num_hits = 0;

    const std::regex & rx = _KLFENGINE_VARIANT_GET<std::regex>(w);

    for (auto && dir_item : fs::directory_iterator(base)) {

      std::string fn = dir_item.path().filename().string();

      if ( ! std::regex_match(fn, rx) ) {
        continue;
      }

      // This dir_item matches our wildcard.  Search recursively in this path
      // item -->
      int n = find_wildcard_path_impl(
          dir_item,
          wildcard_expressions_begin,
          wildcard_expressions_end,
          (limit < 0) ? -1 : std::max(0,limit-num_hits),
          store_hit
          );
      num_hits += n;
      if (limit >= 0 && num_hits >= limit) {
        return num_hits;
      }
    }

    return num_hits;
  }

  assert( false );
  return -1;
}


inline
std::string compile_wildcard_rx_pattern(const std::string & s)
{
  // inspiration from
  // https://github.com/Kogia-sima/cppglob/blob/master/src/fnmatch.cpp
  std::string needs_escape{ R"**([]-{}()*+?.\^$|)**" };

  std::string pat;
  pat = "^";
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '?') {
      pat += ".";
    } else if (s[i] == '*') {
      pat += ".*";
    } else {
      if (needs_escape.find(s[i]) == std::string::npos) {
        // no need for escape
        pat += s[i];
      } else {
        // need escape
        pat += '\\';
        pat += s[i];
      }
    }
  }
  pat += "$";
  return pat;
}

inline
fs_w_part compile_wildcard(const std::string & s)
{
  if (s.find_first_of("*?") == std::string::npos) {
    // no wildcards -- hard-coded dir name
    return {s};
  }

  // translate into std::regex pattern
  return {std::regex(compile_wildcard_rx_pattern(s))};
}




_KLFENGINE_INLINE
std::vector<fs::path>
find_wildcard_path(const std::vector<std::string> & wildcard_expressions,
                   int limit)
{
  return find_wildcard_path(wildcard_expressions,
                            std::function<bool(const fs::path&)>{},
                            limit);
}

_KLFENGINE_INLINE
std::vector<fs::path>
find_wildcard_path(const std::vector<std::string> & wildcard_expressions,
                   const std::function<bool(const fs::path&)> & predicate,
                   int limit)
{
  std::vector<fs::path> hits;

  for (const std::string & wild_expr : wildcard_expressions) {

    // parse the wildcard expression as a path, to start off with.  This will
    // take care of drive letters, etc.

    fs::path wild_path_parts{ wild_expr };

    // compile wildcard expression into list of parts

    fs::path base =
      wild_path_parts.has_root_path() ? wild_path_parts.root_path() : ".";
    fs::path rest = wild_path_parts.relative_path();

    std::vector<fs_w_part> parts;

    std::transform(rest.begin(), rest.end(),
                   std::back_inserter(parts), compile_wildcard);

    (void) find_wildcard_path_impl(
        base,
        parts.begin(),
        parts.end(),
        limit - (int)hits.size(),
        [&hits,&predicate](const fs::path & hit) {
          if (predicate && !predicate(hit)) {
            return;
          }
          if (std::find(hits.begin(), hits.end(), hit) == hits.end()) {
            hits.push_back(hit);
          }
        }
        );
  }

  return hits;
}


_KLFENGINE_INLINE
std::vector<std::string> get_environment_PATH(const char * varname)
{
  std::string env_path{ std::getenv(varname) };
  return
    detail::str_split_rx(env_path.begin(), env_path.end(),
                         std::regex(std::string("\\")+KLFENGINE_PATH_SEP), true);
}



} // namespace detail

} // namespace klfengine
