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



//#include <subprocess.hpp>

#include <klfengine/engine>
#include <klfengine/engine_run_implementation>

#include <regex>


namespace dummy_engine
{

class dummy_engine : public klfengine::engine
{
public:
  dummy_engine()
    : klfengine::engine("dummy-engine")
  {
  }

  std::vector<std::string> record_calls;

private:
  klfengine::engine_run_implementation *
  impl_create_engine_run_implementation( klfengine::input input_,
                                         klfengine::settings settings_ );
};


struct dummy_run_impl : klfengine::engine_run_implementation
{
  dummy_run_impl(
      klfengine::input input_,
      klfengine::settings settings_
      )
    : klfengine::engine_run_implementation(std::move(input_), std::move(settings_))
  {
  }


  std::vector<std::string> record_calls;

private:
  std::string _thedata;

  void impl_compile()
  {
    record_calls.push_back("impl_compile()");

    // "compile" the data
    _thedata = "<compiled data! input was `" + input().latex + "'>";
  }
  std::vector<klfengine::format_description> impl_available_formats()
  {
    record_calls.push_back("impl_available_formats()");
    return {
      {{"TXT", {}}, "TXT format", "TXT format description"},
      {{"TEX", {}}, "TEX format", "TEX format description"},
      {{"HTML", {}}, "HTML format", "HTML format description"},
      {{"XML-emb-TEX", {}}, "XML-emb-TEX format", "XML-emb-TEX format description"}
    };
  }

  virtual klfengine::format_spec impl_make_canonical(
      const klfengine::format_spec & format, bool check_only
      )
  {
    record_calls.push_back("impl_make_canonical(" + format.as_string()
                           + ", " + std::to_string(check_only) + ")");

    auto check_it_bf_params = [format]() {
      klfengine::value::dict p;
      std::for_each(
          format.parameters.begin(), format.parameters.end(),
          [&p,format](const std::pair<std::string,klfengine::value> & v) {
            if (v.first == "italic") {
              if (v.second.get<bool>()) {
                p[v.first] = klfengine::value{true};
              }
            } else if (v.first == "bold") {
              if (v.second.get<bool>()) {
                p[v.first] = klfengine::value{true};
              }
            } else {
              throw klfengine::no_such_format(format.format,
                                              "Invalid format parameter key: " + v.first);
            }
          });
      return p;
    };

    if (format.format == "TXT") {
      if (!format.parameters.empty()) {
        return klfengine::format_spec{}; // invalid -- shouldn't have any parameters
      }
      return format;
    } else if (format.format == "TEX" || format.format == "HTML" ||
               format.format == "XML-emb-TEX") {
      return klfengine::format_spec{format.format, check_it_bf_params()};
    } else {
      return klfengine::format_spec{};
    }
  }

  virtual klfengine::binary_data impl_produce_data(const klfengine::format_spec & format)
  {
    record_calls.push_back("impl_produce_data(" + format.as_string() + ")");

    if (format.format == "XML-emb-TEX") {
      // special case: XML with embedded TEX
      //
      // this should recursively call impl_produce_data(TEX...) and store that to cache.
      //
      const klfengine::binary_data & tex_data = get_data_cref({"TEX", format.parameters});
      
      const std::string xml_start =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dummy-data><![CDATA[";
      const std::string xml_end = "]]></dummy-data>\n";

      klfengine::binary_data d;
      d.insert(d.end(), xml_start.begin(), xml_start.end());
      d.insert(d.end(), tex_data.begin(), tex_data.end());
      d.insert(d.end(), xml_end.begin(), xml_end.end());

      return d;
    }

    if (format.format == "TXT") {

      return {_thedata.begin(), _thedata.end()};

    }

    // here we simulate a process in which TEX & HTML get created simultaneously
    // by the same process (e.g., getting two output files from a single run of
    // an external process).  This means we should call store_to_cache for the
    // format that wasn't requested.

    bool italic = false;
    bool bold = false;

    std::string before_tex;
    std::string after_tex;
    std::string before_html;
    std::string after_html;

    { auto it = format.parameters.find("italic");
      if (it != format.parameters.end() && it->second.get<bool>()) {
        italic = true;
        before_tex += "\\textit{";
        after_tex += "}";
        before_html += "<i>";
        after_html += "</i>";
      }
    }
    { auto it = format.parameters.find("bold");
      if (it != format.parameters.end() && it->second.get<bool>()) {
        bold = true;
        before_tex += "\\textbf{";
        after_tex += "}";
        before_html += "<b>";
        after_html += "</b>";
      }
    }

    std::string full_html = _thedata;
    full_html = std::regex_replace(full_html, std::regex("\\<"), "&lt;");
    full_html = std::regex_replace(full_html, std::regex("\\>"), "&gt;");

    full_html = before_html + full_html + after_html;
    klfengine::binary_data d_html{full_html.begin(), full_html.end()};
    klfengine::binary_data d_tex;
    d_tex.insert(d_tex.end(), before_tex.begin(), before_tex.end());
    d_tex.insert(d_tex.end(), _thedata.begin(), _thedata.end());
    d_tex.insert(d_tex.end(), after_tex.begin(), after_tex.end());

    klfengine::value::dict p;
    if (italic) {
      p["italic"] = klfengine::value{true};
    }
    if (bold) {
      p["bold"] = klfengine::value{true};
    }

    if (format.format == "HTML") {
      /*const auto& d_tex_cref = */ store_to_cache({"TEX", p}, std::move(d_tex));
      return d_html;
    }
    // requested TEX format
    /*const auto& d_html_cref =*/ store_to_cache({"HTML", p}, std::move(d_html));
    return d_tex;
  }

};





inline klfengine::engine_run_implementation *
dummy_engine::impl_create_engine_run_implementation( klfengine::input input_,
                                                     klfengine::settings settings_ )
{
  record_calls.push_back("impl_create_engine_run_implementation(...)");
  return new dummy_run_impl(std::move(input_), std::move(settings_));
}



} // namespace dummy_engine
