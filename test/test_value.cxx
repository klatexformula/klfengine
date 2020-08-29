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
#include <klfengine/value>

//#include <iostream> // DEBUG
//#include <iomanip>
//#include <stdexcept>

#include <nlohmann/json.hpp>

#include <catch2/catch.hpp>


TEST_CASE( "variant_type can store an int or a string", "[variants]" )
{

  // ### TODO: should we expose a public klfengine-wide API for std::get vs
  // ### mpark::get (instead of the non-public _KLFENGINE_VARIANT_GET ?).  On
  // ### the other hand, I don't think it's important because the only thing
  // ### that should matter is klfengine::value, which has its own public .get()
  // ### method.

  REQUIRE(
      _KLFENGINE_VARIANT_GET<int>(klfengine::detail::variant_type<int, std::string>{3})
      == 3
      );
  REQUIRE(
      _KLFENGINE_VARIANT_GET<std::string>(
          klfengine::detail::variant_type<int, std::string>{"hello"})
      == std::string("hello")
      );
}


TEST_CASE( "value can get different data types correctly", "[variants]" )
{
  REQUIRE( klfengine::value{nullptr}.get<std::nullptr_t>() == nullptr );
  REQUIRE( klfengine::value{true}.get<bool>() == true );
  REQUIRE( klfengine::value{3}.get<int>() == 3 );
  REQUIRE( klfengine::value{42.5}.get<double>() == 42.5 );
  REQUIRE( klfengine::value{std::string("yo")}.get<std::string>() == std::string("yo") );
}


TEST_CASE( "value can store different data types recursively", "[variants]" )
{
  // const char * gets converted to bool (no, really, gotta be kidding me C++)
  //klfengine::value x{"one"};
  //std::cerr << "x.data-index() == " << x._data.index() << "\n"; // !!!!

  {
    klfengine::value d{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")},
        klfengine::value{klfengine::value::array{klfengine::value{3}, klfengine::value{4},
                                                 klfengine::value{5}}},
        klfengine::value{klfengine::value::dict{
          {"key1",
           klfengine::value{std::string("value1")}},
          {"key2",
           klfengine::value{222}}}}
      }
    };
    REQUIRE(
        d.get<klfengine::value::array>()[0].get<std::string>() == std::string("one")
        );
    REQUIRE(
        d.get<klfengine::value::array>()[2].get<klfengine::value::array>()[1].get<int>()
        == 4
        );
    REQUIRE(
        d.get<klfengine::value::array>()[3].get<klfengine::value::dict>()["key2"].get<int>()
        == 222
        );
  }

#if defined(__cpp_lib_string_udls) && __cpp_lib_string_udls >= 201304L
  {
    using namespace std::literals; // "xxx"s -> std::string  (C++ >= 14)

    klfengine::value d{
      klfengine::value::array{
        klfengine::value{"one"s},
        klfengine::value{"two"s},
        klfengine::value{klfengine::value::array{klfengine::value{3}, klfengine::value{4},
                                                 klfengine::value{5}}},
        klfengine::value{klfengine::value::dict{{"key1", klfengine::value{"value1"s}},
                                                {"key2", klfengine::value{222}}}}
      }
    };
    REQUIRE(
        d.get<klfengine::value::array>()[0].get<std::string>() == std::string("one")
        );
    REQUIRE(
        d.get<klfengine::value::array>()[2].get<klfengine::value::array>()[1].get<int>()
        == 4
        );
    REQUIRE(
        d.get<klfengine::value::array>()[3].get<klfengine::value::dict>()["key2"].get<int>()
        == 222
        );
  }
#endif
}



TEST_CASE( "value offers has_type", "[variants]" )
{
  REQUIRE( klfengine::value{nullptr}.has_type<std::nullptr_t>() );
  REQUIRE( ! klfengine::value{nullptr}.has_type<int>() );
  REQUIRE( ! klfengine::value{nullptr}.has_type<bool>() );
  REQUIRE( ! klfengine::value{nullptr}.has_type<std::string>() );
  REQUIRE( ! klfengine::value{nullptr}.has_type<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{nullptr}.has_type<klfengine::value::dict>() );

  REQUIRE( klfengine::value{true}.has_type<bool>() );
  REQUIRE( ! klfengine::value{true}.has_type<int>() );
  REQUIRE( ! klfengine::value{true}.has_type<std::string>() );
  REQUIRE( ! klfengine::value{true}.has_type<std::nullptr_t>() );

  REQUIRE( klfengine::value{3}.has_type<int>() );
  REQUIRE( ! klfengine::value{3}.has_type<double>() );
  REQUIRE( ! klfengine::value{3}.has_type<std::nullptr_t>() );
  REQUIRE( ! klfengine::value{3}.has_type<std::string>() );

  REQUIRE( klfengine::value{42.5}.has_type<double>() );
  REQUIRE( ! klfengine::value{42.5}.has_type<int>() );
  REQUIRE( ! klfengine::value{42.5}.has_type<bool>() );

  REQUIRE( klfengine::value{std::string("yo")}.has_type<std::string>() );
  REQUIRE( ! klfengine::value{std::string("yo")}.has_type<std::nullptr_t>() );

  auto va = klfengine::value{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")}
      }
    };

  REQUIRE( va.has_type<klfengine::value::array>() );
  REQUIRE( ! va.has_type<klfengine::value::dict>() );
  REQUIRE( ! va.has_type<std::nullptr_t>() );
  REQUIRE( ! va.has_type<int>() );
  REQUIRE( ! va.has_type<bool>() );


  auto vd = klfengine::value{klfengine::value::dict{
    {"key1",
     klfengine::value{std::string("value1")}},
    {"key2",
     klfengine::value{222}}
  }};


  REQUIRE( vd.has_type<klfengine::value::dict>() );
  REQUIRE( ! vd.has_type<klfengine::value::array>() );
  REQUIRE( ! vd.has_type<std::nullptr_t>() );
  REQUIRE( ! vd.has_type<int>() );
  REQUIRE( ! vd.has_type<bool>() );
}



struct simple_visitor
{
  std::string result;

  void operator()(std::nullptr_t) {
    result = "null";
  }
  void operator()(bool) {
    result = "bool";
  }
  void operator()(int) {
    result = "int";
  }
  void operator()(double) {
    result = "double";
  }
  void operator()(std::string) {
    result = "string";
  }
  void operator()(klfengine::value::array) {
    result = "array";
  }
  void operator()(klfengine::value::dict) {
    result = "dict";
  }
};


TEST_CASE( "value can be visited", "[variants]" )
{
  { auto v = klfengine::value{nullptr};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "null" ); }

  { auto v = klfengine::value{true};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "bool" ); }

  { auto v = klfengine::value{3};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "int" ); }

  { auto v = klfengine::value{42.5};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "double" ); }

  { auto v = klfengine::value{std::string{"hello"}};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "string" ); }

  { auto v = klfengine::value{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")}
      }
    };
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "array" ); }

  { auto v = klfengine::value{klfengine::value::dict{
      {"key1",
       klfengine::value{std::string("value1")}},
      {"key2",
       klfengine::value{222}}
    }};
    simple_visitor vis;
    v.visit(vis);
    REQUIRE( vis.result == "dict" ); }
}


struct simple_visit_transformer
{
  std::string operator()(std::nullptr_t) {
    return "null";
  }
  std::string operator()(bool) {
    return "bool";
  }
  std::string operator()(int) {
    return "int";
  }
  std::string operator()(double) {
    return "double";
  }
  std::string operator()(std::string) {
    return "string";
  }
  std::string operator()(klfengine::value::array) {
    return "array";
  }
  std::string operator()(klfengine::value::dict) {
    return "dict";
  }
};


TEST_CASE( "value can be transformed", "[variants]" )
{
  { auto v = klfengine::value{nullptr};
    REQUIRE( v.transform(simple_visit_transformer()) == "null" ); }

  { auto v = klfengine::value{true};
    REQUIRE( v.transform(simple_visit_transformer()) == "bool" ); }

  { auto v = klfengine::value{3};
    REQUIRE( v.transform(simple_visit_transformer()) == "int" ); }

  { auto v = klfengine::value{42.5};
    REQUIRE( v.transform(simple_visit_transformer()) == "double" ); }

  { auto v = klfengine::value{std::string{"hello"}};
    REQUIRE( v.transform(simple_visit_transformer()) == "string" ); }

  { auto v = klfengine::value{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")}
      }
    };
    REQUIRE( v.transform(simple_visit_transformer()) == "array" ); }

  { auto v = klfengine::value{klfengine::value::dict{
      {"key1",
       klfengine::value{std::string("value1")}},
      {"key2",
       klfengine::value{222}}
    }};
    REQUIRE( v.transform(simple_visit_transformer()) == "dict" ); }
}



TEST_CASE( "value supports equality comparision", "[variants]" )
{
  REQUIRE( klfengine::value{12} == klfengine::value{12} );
  REQUIRE( klfengine::value{12} != klfengine::value{13} );
  REQUIRE( klfengine::value{12} != klfengine::value{12.0} );

  klfengine::value::array a{
    klfengine::value{12}, klfengine::value{14}, klfengine::value{15}
  };
  klfengine::value::array b{
    klfengine::value{12}, klfengine::value{14}, klfengine::value{15}
  };

  REQUIRE( a == b );

  klfengine::value::array c{
    klfengine::value{12}, klfengine::value{14}, klfengine::value{15}, klfengine::value{15}
  };
  REQUIRE( a != c) ;


  klfengine::value::dict x{{"a",klfengine::value{14}}};
  klfengine::value::dict x2{{"a",klfengine::value{14}}};
  klfengine::value::dict y{{"a",klfengine::value{14.0}}};
  klfengine::value::dict z{{"a",klfengine::value{14}},
                           {"b",klfengine::value{15}}};

  REQUIRE( x == x2 );
  REQUIRE( x != y );
  REQUIRE( x != z );
}




/*
namespace ns {
    // a simple struct to model a person
    struct person {
        std::string name;
        std::string address;
        int age;
    };
}


namespace ns {
    void to_json(nlohmann::json& j, const person& p) {
        j = nlohmann::json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
    }

    void from_json(const nlohmann::json& j, person& p) {
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
    }
} // namespace ns


TEST_CASE( "debugging json stuff") 
{
  // create a person
  ns::person p {"Ned Flanders", "744 Evergreen Terrace", 60};

  // conversion: person -> json
  nlohmann::json j = p;

  std::cout << j << std::endl;
  // {"address":"744 Evergreen Terrace","age":60,"name":"Ned Flanders"}

  // conversion: json -> person
  auto p2 = j.get<ns::person>();

  // that's it
  assert(p.name == p2.name);
}
*/



TEST_CASE( "value can be converted to JSON", "[variants]" )
{
  klfengine::value d{
        klfengine::value::array{
          klfengine::value{std::string("one")},
          klfengine::value{std::string("two")},
          klfengine::value{klfengine::value::array{klfengine::value{3}, klfengine::value{4},
                                                   klfengine::value{5}}},
          klfengine::value{klfengine::value::dict{
            {"key1", klfengine::value{std::string("value1")}},
            {"key2", klfengine::value{222}}
          }}
        }
      };

  nlohmann::json j = d;

  // std::cout << j.dump(4) << "\n\n";

  const auto json_ok = nlohmann::json::parse( R"(
    [ "one", "two", [3, 4, 5], { "key1": "value1", "key2": 222 } ]
)" );

  REQUIRE( j == json_ok ) ;
}



TEST_CASE( "value can be converted from JSON", "[variants]" )
{
  const nlohmann::json j = nlohmann::json::parse(R"xx( {
      "A": 1,
      "B": ["b", false],
      "C": {
          "d": 0.25,
          "e": [null]
      }
  } )xx");

  // std::cerr << "JSON object is " << j.dump() << "\n";

  klfengine::value v = j.get<klfengine::value>();

  using array = klfengine::value::array;
  using dict = klfengine::value::dict;

  // std::cerr << "DEBUG: "
  //   << "v.index() == " << v._data.index() << " "
  //   << "v['A'].index() == " << v.get<dict>()["A"]._data.index() << "\n";

  REQUIRE(
      v.get<dict>()["A"].get<int>() == 1
      );
  REQUIRE(
      v.get<dict>()["B"].get<array>().size() == 2
      );
  REQUIRE(
      v.get<dict>()["B"].get<array>()[0].get<std::string>() == "b"
      );
  REQUIRE(
      v.get<dict>()["B"].get<array>()[1].get<bool>() == false
      );
  REQUIRE(
      v.get<dict>()["C"].get<dict>()["d"].get<double>() == 0.25
      );
  REQUIRE(
      v.get<dict>()["C"].get<dict>()["e"].get<array>()[0].get<std::nullptr_t>() == nullptr
      );

}
