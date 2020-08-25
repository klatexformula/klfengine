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

// header we are testing gets included first (helps detect missing #include's)
#include <klfengine/klfengine>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace pybind11::literals;



PYBIND11_MODULE(_cxx_pyklf, m)
{
  auto json_module = py::module::import("json");

  py::class_<klfengine::input>(m, "input")
    .def(py::init<>())
    .def_readwrite("latex", &klfengine::input::latex)
    //.def_readwrite("math_mode" &klfengine::input::math_mode)
    .def_readwrite("preamble", &klfengine::input::preamble)
    .def_readwrite("latex_engine", &klfengine::input::latex_engine)
    .def_readwrite("font_size", &klfengine::input::font_size)
    //.def_readwrite("fg_color", &klfengine::input::fg_color)
    //.def_readwrite("bg_color", &klfengine::input::bg_color)
    //.def_readwrite("margins", &klfengine::input::margins)
    .def_readwrite("dpi", &klfengine::input::dpi)
    .def_readwrite("scale", &klfengine::input::scale)
    .def_readwrite("outline_fonts", &klfengine::input::outline_fonts)
    .def_property(
        "parameters",
        [json_module](klfengine::input & obj) {
          nlohmann::json j;
          j = obj.parameters;
          return json_module.attr("loads")( j.dump() );
        },
        [json_module](klfengine::input & obj, py::dict dic) {
          nlohmann::json::parse( json_module.attr("dumps")(dic).cast<std::string>() ).get_to(obj);
        }
        )
    .def("to_json", [](klfengine::input & obj) {
          nlohmann::json j;
          j = obj;
          return j.dump();
        })
    ;
  
  auto klfimplpkg_engine_m = m.def_submodule("klfimplpkg_engine", "klfimplpkg engine (...)");

  klfimplpkg_engine_m.def(
      "compile_to",
      [](klfengine::input input, std::string format) {
        klfengine::klfimplpkg_engine::engine e;

        e.set_settings(klfengine::settings::detect_settings());

        auto r = e.run(input);
        r->compile();
        auto data = r->get_data(klfengine::format_spec{format});

        std::string data_str{reinterpret_cast<const char*>(&data[0]), data.size()};

        return py::bytes{data_str};
      }
      );
}
