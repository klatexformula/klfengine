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
#include <pybind11/eval.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace pybind11::literals;


template<typename ClassType, typename MemberType>
void set_if_in_kwargs(ClassType & x, MemberType ClassType::* member, std::string member_name,
                      py::kwargs & kwargs)
{
  auto member_name_s = py::str(member_name);
  if (kwargs.attr("__contains__")(member_name_s).cast<bool>()) {
    x.*member = kwargs.attr("pop")(member_name_s).cast<MemberType>();
  }
}


// py::object value_to_pyobj(const klfengine::value & value);
// klfengine::value pyobj_to_value(const py::object & obj);
//
//
// py::dict valuedict_to_pydict(const klfengine::value::dict & d)
// {
//   py::dict pyd{};
//   for (const auto & item : d) {
//     pyd[item.first] = value_to_pyobj(item.second);
//   }
//   return pyd;
// }
// klfengine::value::dict pydict_to_valuedict(const py::dict & pyd)
// {
//   klfengine::value::dict d;
//   for (auto item : pyd) {
//     d[item.first] = pyobj_to_value(item.second);
//   }
//   return d;
// }
//
// py::list valuearray_to_pylist(const klfengine::value::array & d)
// {
//   py::list pyd{};
//   for (const auto & item : d) {
//     pyd.append(value_to_pyobj(item));
//   }
//   return pyd;
// }
// template<typename PyListOrTuple>
// klfengine::value::array pylist_to_valuearray(const PyListOrTuple & pyd)
// {
//   klfengine::value::array d;
//   for (auto item : pyd) {
//     d.push_back(pyobj_to_value(item));
//   }
//   return d;
// }
//
// struct value_to_pyobj_transformer
// {
//   py::object operator()(const klfengine::value::array & a)
//   {
//     return valuearray_to_pylist(a);
//   }
//   py::object operator()(const klfengine::value::dict & d)
//   {
//     return valuedict_to_pylist(d);
//   }
//   template<typename SimpleType,
//            typename SimpleTypeNoRef = typename std::remove_reference<SimpleType>::type,
//            typename std::enable_if<
//              !(std::is_same<SimpleTypeNoRef, value::array>::value ||
//                std::is_same<SimpleTypeNoRef, value::dict>::value),
//            bool>::type = true>
//   py::object operator()(SimpleType x) {
//     return py::cast(x);
//   }
// };
//
// py::object value_to_pyobj(const klfengine::value & value)
// {
//   return value.transform(value_to_pyobj_transformer{});
// }
// klfengine::value pyobj_to_value(py::object obj)
// {
//   if ( py::isinstance<py::list>(obj) ) {
//     return klfengine::value{pylist_to_valuearray(py::list{obj})};
//   }
//   if ( py::isinstance<py::tuple>(obj) ) {
//     return klfengine::value{pylist_to_valuearray(py::tuple{obj})};
//   }
//   if ( py::isinstance<py::dict>(obj) ) {
//     return klfengine::value{pydict_to_valuedict(py::dict{obj})};
//   }
//   return 
// }



PYBIND11_MODULE(pyklf, m)
{
  auto json_module = py::module::import("json");
  
  //
  // input
  //

  m.attr("length") = py::eval("float"); // the "float" python type object

  py::class_<klfengine::color>(m, "color")
    .def(py::init(
             [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
               return std::unique_ptr<klfengine::color>{ new klfengine::color{
                   r, g, b, a
                 } };
             }),
         "red"_a = 0,
         "green"_a = 0,
         "blue"_a = 0,
         "alpha"_a = 255
        )
    .def_readwrite("red", &klfengine::color::red)
    .def_readwrite("green", &klfengine::color::green)
    .def_readwrite("blue", &klfengine::color::blue)
    .def_readwrite("alpha", &klfengine::color::alpha)
    .def("__repr__",
         [](const klfengine::color & c) {
           nlohmann::json j;
           j = c;
           return "pyklf.color(*" + j.dump() + ")";
         })
    ;

  py::class_<klfengine::margins>(m, "margins")
    .def(py::init(
             [json_module](py::dict d) {
               nlohmann::json j =
                 nlohmann::json::parse( json_module.attr("dumps")(d).cast<std::string>() );
               auto m = std::unique_ptr<klfengine::margins>{ new klfengine::margins{} };
               j.get_to(*m);

               return m;
             }
             ))
    .def(py::init(
             [](klfengine::length top, klfengine::length right,
                klfengine::length bottom, klfengine::length left) {
               return std::unique_ptr<klfengine::margins>{ new klfengine::margins{
                   top, right, bottom, left
                 } };
             }),
         "top"_a = 0.0,
         "right"_a = 0.0,
         "bottom"_a = 0.0,
         "left"_a = 0.0
        )
    .def_readwrite("top", &klfengine::margins::top)
    .def_readwrite("right", &klfengine::margins::right)
    .def_readwrite("bottom", &klfengine::margins::bottom)
    .def_readwrite("left", &klfengine::margins::left)
    .def("__repr__",
         [](const klfengine::margins & m) {
           nlohmann::json j;
           j = m;
           return "pyklf.margins(*" + j.dump() + ")";
         })
    ;

  py::class_<klfengine::input>(m, "input")
    .def(py::init(
             [json_module](py::kwargs kwargs) {
               auto inptr = std::unique_ptr<klfengine::input>{new klfengine::input{}};

               klfengine::input & in = *inptr;

               set_if_in_kwargs(in, &klfengine::input::latex, "latex", kwargs);
               set_if_in_kwargs(in, &klfengine::input::math_mode, "math_mode", kwargs);
               set_if_in_kwargs(in, &klfengine::input::preamble, "preamble", kwargs);
               set_if_in_kwargs(in, &klfengine::input::latex_engine, "latex_engine", kwargs);
               set_if_in_kwargs(in, &klfengine::input::font_size, "font_size", kwargs);
               set_if_in_kwargs(in, &klfengine::input::fg_color, "fg_color", kwargs);
               set_if_in_kwargs(in, &klfengine::input::bg_color, "bg_color", kwargs);
               set_if_in_kwargs(in, &klfengine::input::margins, "margins", kwargs);
               set_if_in_kwargs(in, &klfengine::input::dpi, "dpi", kwargs);
               set_if_in_kwargs(in, &klfengine::input::scale, "scale", kwargs);
               set_if_in_kwargs(in, &klfengine::input::outline_fonts, "outline_fonts", kwargs);
               set_if_in_kwargs(in, &klfengine::input::parameters, "parameters", kwargs);

               if (py::len(kwargs) > 0) {
                 throw py::value_error("Invalid keyword argument for pyklf.input: "
                                       + py::repr(kwargs).cast<std::string>());
               }

               return inptr;
             }))
    .def_readwrite("latex", &klfengine::input::latex)
    .def_readwrite("math_mode", &klfengine::input::math_mode)
    .def_readwrite("preamble", &klfengine::input::preamble)
    .def_readwrite("latex_engine", &klfengine::input::latex_engine)
    .def_readwrite("font_size", &klfengine::input::font_size)
    .def_readwrite("fg_color", &klfengine::input::fg_color)
    .def_readwrite("bg_color", &klfengine::input::bg_color)
    .def_readwrite("margins", &klfengine::input::margins)
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
          nlohmann::json::parse(
              json_module.attr("dumps")(dic).cast<std::string>()
              ).get_to(obj.parameters);
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
