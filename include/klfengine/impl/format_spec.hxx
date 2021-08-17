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

#include <klfengine/format_spec>

#include <nlohmann/json.hpp>

namespace klfengine {


_KLFENGINE_INLINE
std::string format_spec::as_string() const
{
  std::string s = format;
  if (!parameters.empty()) {
    nlohmann::json j = nlohmann::json::object();
    j = parameters;
    s += ":"+ j.dump();
  }
  return s;
}

_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const format_spec & v)
{
  j = nlohmann::json{
    {"format", v.format},
    {"parameters", v.parameters}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, format_spec & v)
{
  try {
    j.at("format").get_to(v.format);
    j.at("parameters").get_to(v.parameters);
  } catch (nlohmann::json::exception & e) {
    throw invalid_json_value{"klfengine::format_spec", j, e.what()};
  }
}




_KLFENGINE_INLINE
void to_json(nlohmann::json & j, const format_description & v)
{
  j = nlohmann::json{
    {"format_spec", v.format_spec},
    {"title", v.title},
    {"description", v.description}
  };
}
_KLFENGINE_INLINE
void from_json(const nlohmann::json & j, format_description & v)
{
  try {
    j.at("format_spec").get_to(v.format_spec);
    j.at("title").get_to(v.title);
    j.at("description").get_to(v.description);
  } catch (nlohmann::json::exception & e) {
    throw invalid_json_value{"klfengine::format_description", j, e.what()};
  }
}



// ---------------------------------------------------------



_KLFENGINE_INLINE
bool  format_provider::has_format(std::string format)
{
  return has_format( format_spec{ std::move(format) });
}

_KLFENGINE_INLINE
bool  format_provider::has_format(const format_spec & format)
{
  try {
    (void) internal_canonical_format(format, true);
  } catch (no_such_format & /*exc*/) {
    return false;
  }
  return true;
}

_KLFENGINE_INLINE
format_spec  format_provider::canonical_format(const format_spec & format)
{
  return internal_canonical_format(format, false);
}

_KLFENGINE_INLINE
format_spec  format_provider::canonical_format_or_empty(const format_spec & format)
{
  try {
    return internal_canonical_format(format, false);
  } catch (no_such_format & ex) {
    return format_spec{};
  }
}

_KLFENGINE_INLINE
format_spec  format_provider::internal_canonical_format(
    const format_spec & format,
    bool check_available_only
)
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


_KLFENGINE_INLINE
std::vector<format_description> format_provider::available_formats()
{
  return impl_available_formats();

}






} // namespace klfengine
