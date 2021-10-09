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



TEST_CASE( "parse_boolean can parse a valid boolean", "[value]" )
{
  REQUIRE( klfengine::parse_boolean("true") );
  REQUIRE( klfengine::parse_boolean("True") );
  REQUIRE( klfengine::parse_boolean("T") );
  REQUIRE( klfengine::parse_boolean("Y") );
  REQUIRE( klfengine::parse_boolean("y") );
  REQUIRE( klfengine::parse_boolean("yes") );
  REQUIRE( klfengine::parse_boolean("on") );
  REQUIRE( klfengine::parse_boolean("1") );
  REQUIRE( klfengine::parse_boolean("+2") );
  REQUIRE( klfengine::parse_boolean("-3849") );

  REQUIRE( klfengine::parse_boolean(" T") );
  REQUIRE( klfengine::parse_boolean("T  ") );
  REQUIRE( klfengine::parse_boolean("\t\ntrUe  \r\f\n\v") );
  REQUIRE( klfengine::parse_boolean("\n  \r+134  ") );

  REQUIRE( ! klfengine::parse_boolean("false") );
  REQUIRE( ! klfengine::parse_boolean("faLsE") );
  REQUIRE( ! klfengine::parse_boolean("f") );
  REQUIRE( ! klfengine::parse_boolean("F") );
  REQUIRE( ! klfengine::parse_boolean("N") );
  REQUIRE( ! klfengine::parse_boolean("n") );
  REQUIRE( ! klfengine::parse_boolean("no") );
  REQUIRE( ! klfengine::parse_boolean("off") );
  REQUIRE( ! klfengine::parse_boolean("0") );
  REQUIRE( ! klfengine::parse_boolean("+0") );

  REQUIRE( ! klfengine::parse_boolean(" F") );
  REQUIRE( ! klfengine::parse_boolean("f  ") );
  REQUIRE( ! klfengine::parse_boolean("\t\nFalse  \r\f\n\v") );
  REQUIRE( ! klfengine::parse_boolean("\n  \r 000  ") );

  REQUIRE_THROWS_AS( klfengine::parse_boolean(" invalid") , std::invalid_argument);
  REQUIRE_THROWS_AS( klfengine::parse_boolean("1invalid") , std::invalid_argument);
  REQUIRE_THROWS_AS( klfengine::parse_boolean("0  blabla") , std::invalid_argument);

  REQUIRE_THROWS_AS( klfengine::parse_boolean("tru") , std::invalid_argument);
  REQUIRE_THROWS_AS( klfengine::parse_boolean("fa") , std::invalid_argument);
  REQUIRE_THROWS_AS( klfengine::parse_boolean("ye") , std::invalid_argument);

  REQUIRE_THROWS_AS( klfengine::parse_boolean("o") , std::invalid_argument);
}


// -------


TEST_CASE( "variant_type can store an int or a string", "[value]" )
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


TEST_CASE( "value can get different data types correctly", "[value]" )
{
  REQUIRE( klfengine::value{nullptr}.get<std::nullptr_t>() == nullptr );
  REQUIRE( klfengine::value{true}.get<bool>() == true );
  REQUIRE( klfengine::value{3}.get<int>() == 3 );
  REQUIRE( klfengine::value{42.5}.get<double>() == 42.5 );
  REQUIRE( klfengine::value{std::string("yo")}.get<std::string>() == std::string("yo") );
}


TEST_CASE( "value can store different data types recursively", "[value]" )
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

  {
    //using namespace std::literals; // "xxx"s -> std::string  (C++ >= 14)

    klfengine::value d{
      klfengine::value::array{
        klfengine::value{std::string{"one"}},
        klfengine::value{std::string{"two"}},
        klfengine::value{klfengine::value::array{klfengine::value{3}, klfengine::value{4},
                                                 klfengine::value{5}}},
        klfengine::value{klfengine::value::dict{{"key1",
                                                 klfengine::value{std::string{"value1"}}},
                                                {"key2", klfengine::value{222}}}}
      }
    };
    REQUIRE(
        d.get<klfengine::value::array>()[0].get<std::string>() == std::string{"one"}
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
}


TEST_CASE( "value offers get_type_name", "[value]" )
{
  REQUIRE( klfengine::value{true}.get_type_name() == "bool" );
  REQUIRE( klfengine::value{1}.get_type_name() == "int" );
  REQUIRE( klfengine::value{1.5}.get_type_name() == "double" );
  REQUIRE( klfengine::value{nullptr}.get_type_name() == "null" );
}


TEST_CASE( "value offers has_type", "[value]" )
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

struct simple_visitor_with_return_value
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


TEST_CASE( "value can be visited", "[value]" )
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


TEST_CASE( "value can be visited with return value", "[value]" )
{
  { auto v = klfengine::value{nullptr};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "null" ); }

  { auto v = klfengine::value{true};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "bool" ); }

  { auto v = klfengine::value{3};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "int" ); }

  { auto v = klfengine::value{42.5};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "double" ); }

  { auto v = klfengine::value{std::string{"hello"}};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "string" ); }

  { auto v = klfengine::value{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")}
      }
    };
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "array" ); }

  { auto v = klfengine::value{klfengine::value::dict{
      {"key1",
       klfengine::value{std::string("value1")}},
      {"key2",
       klfengine::value{222}}
    }};
    REQUIRE( v.visit(simple_visitor_with_return_value()) == "dict" ); }
}




// struct simple_visit_transformer
// {
//   std::string operator()(std::nullptr_t) {
//     return "null";
//   }
//   std::string operator()(bool) {
//     return "bool";
//   }
//   std::string operator()(int) {
//     return "int";
//   }
//   std::string operator()(double) {
//     return "double";
//   }
//   std::string operator()(std::string) {
//     return "string";
//   }
//   std::string operator()(klfengine::value::array) {
//     return "array";
//   }
//   std::string operator()(klfengine::value::dict) {
//     return "dict";
//   }
// };


// TEST_CASE( "value can be transformed", "[value]" )
// {
//   { auto v = klfengine::value{nullptr};
//     REQUIRE( v.transform(simple_visit_transformer()) == "null" ); }

//   { auto v = klfengine::value{true};
//     REQUIRE( v.transform(simple_visit_transformer()) == "bool" ); }

//   { auto v = klfengine::value{3};
//     REQUIRE( v.transform(simple_visit_transformer()) == "int" ); }

//   { auto v = klfengine::value{42.5};
//     REQUIRE( v.transform(simple_visit_transformer()) == "double" ); }

//   { auto v = klfengine::value{std::string{"hello"}};
//     REQUIRE( v.transform(simple_visit_transformer()) == "string" ); }

//   { auto v = klfengine::value{
//       klfengine::value::array{
//         klfengine::value{std::string("one")},
//         klfengine::value{std::string("two")}
//       }
//     };
//     REQUIRE( v.transform(simple_visit_transformer()) == "array" ); }

//   { auto v = klfengine::value{klfengine::value::dict{
//       {"key1",
//        klfengine::value{std::string("value1")}},
//       {"key2",
//        klfengine::value{222}}
//     }};
//     REQUIRE( v.transform(simple_visit_transformer()) == "dict" ); }
// }



TEST_CASE( "value supports equality comparision", "[value]" )
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




TEST_CASE( "value offers has_castable_to", "[value]" )
{
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<int>() );
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<double>() );
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<bool>() );
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<std::string>() );
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{nullptr}.has_castable_to<klfengine::value::dict>() );
  REQUIRE( klfengine::value{nullptr}.has_castable_to<std::nullptr_t>() );

  REQUIRE( klfengine::value{true}.has_castable_to<bool>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<int>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<double>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<std::string>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<std::nullptr_t>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{true}.has_castable_to<klfengine::value::dict>() );

  REQUIRE( klfengine::value{3}.has_castable_to<int>() );
  REQUIRE( klfengine::value{3}.has_castable_to<double>() );
  REQUIRE( klfengine::value{3}.has_castable_to<bool>() );
  REQUIRE( ! klfengine::value{3}.has_castable_to<std::nullptr_t>() );
  REQUIRE( ! klfengine::value{3}.has_castable_to<std::string>() );
  REQUIRE( ! klfengine::value{3}.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{3}.has_castable_to<klfengine::value::dict>() );

  REQUIRE( klfengine::value{42.5}.has_castable_to<double>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<int>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<bool>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<std::nullptr_t>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<std::string>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{42.5}.has_castable_to<klfengine::value::dict>() );

  // strings are in principle convertible to bool, int, and double, regardless
  // of actual value.
  REQUIRE( klfengine::value{std::string("yo")}.has_castable_to<bool>() );
  REQUIRE( klfengine::value{std::string("yo")}.has_castable_to<int>() );
  REQUIRE( klfengine::value{std::string("yo")}.has_castable_to<double>() );
  REQUIRE( ! klfengine::value{std::string("yo")}.has_castable_to<std::nullptr_t>() );
  REQUIRE( klfengine::value{std::string("yo")}.has_castable_to<std::string>() );
  REQUIRE( ! klfengine::value{std::string("yo")}.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! klfengine::value{std::string("yo")}.has_castable_to<klfengine::value::dict>() );

  auto va = klfengine::value{
      klfengine::value::array{
        klfengine::value{std::string("one")},
        klfengine::value{std::string("two")}
      }
    };

  REQUIRE( va.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! va.has_castable_to<klfengine::value::dict>() );
  REQUIRE( ! va.has_castable_to<std::nullptr_t>() );
  REQUIRE( ! va.has_castable_to<int>() );
  REQUIRE( ! va.has_castable_to<bool>() );


  auto vd = klfengine::value{klfengine::value::dict{
    {"key1",
     klfengine::value{std::string("value1")}},
    {"key2",
     klfengine::value{222}}
  }};


  REQUIRE( vd.has_castable_to<klfengine::value::dict>() );
  REQUIRE( ! vd.has_castable_to<klfengine::value::array>() );
  REQUIRE( ! vd.has_castable_to<std::nullptr_t>() );
  REQUIRE( ! vd.has_castable_to<int>() );
  REQUIRE( ! vd.has_castable_to<bool>() );
}


TEST_CASE( "value can get_cast()", "[value]" )
{
  REQUIRE( klfengine::value{true}.get_cast<bool>() == true );
  REQUIRE( klfengine::value{1}.get_cast<int>() == 1 );
  REQUIRE( klfengine::value{2.0}.get_cast<double>() == 2.0 );
  REQUIRE( klfengine::value{std::string{"xyZ"}}.get_cast<std::string>()
           == std::string{"xyZ"} );
  REQUIRE( klfengine::value{nullptr}.get_cast<std::nullptr_t>() == nullptr );

  // castable types
  REQUIRE( klfengine::value{1}.get_cast<bool>() == true );
  REQUIRE( klfengine::value{1}.get_cast<double>() == 1.0 );
  REQUIRE( klfengine::value{std::string{"2.0"}}.get_cast<double>() == 2.0 );
  REQUIRE( klfengine::value{std::string{"+3"}}.get_cast<int>() == 3 );
  REQUIRE( klfengine::value{std::string{"FaLsE"}}.get_cast<bool>() == false );

  // check that we throw an std::invalid_argument if necessary
  REQUIRE_THROWS_AS( klfengine::value{nullptr}.get_cast<bool>(), std::exception );
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



TEST_CASE( "value can be converted to JSON", "[value]" )
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



TEST_CASE( "value can be converted from JSON", "[value]" )
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



// -------------------------------------

// dict_get, dict_take

TEST_CASE("dict_get finds existing value in dict", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_get(d, "B") == klfengine::value{std::string{"value of B"}} );
}

TEST_CASE("dict_get finds existing value in dict and converts type", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_get<std::string>(d, "B") == std::string{"value of B"} );
}

TEST_CASE("dict_get throws error on nonexistent key", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE_THROWS_AS( klfengine::dict_get<std::string>(d, "D") , std::out_of_range );
}

TEST_CASE("dict_get with default value finds existing value in dict", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_get(d, "B", std::string{"XXXYYYZZZ"})
           == std::string{"value of B"} );
}

TEST_CASE("dict_get returns default on nonexistent key", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_get(d, "D", 1234) == 1234 );
}

// ---

TEST_CASE("dict_take finds existing value in dict", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_take(d, "B") == klfengine::value{std::string{"value of B"}} );

  REQUIRE( d.size() == 2 );
  REQUIRE( d == klfengine::value::dict{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  } );
}

TEST_CASE("dict_take finds existing value in dict and converts type", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_take<std::string>(d, "B") == std::string{"value of B"} );

  REQUIRE( d.size() == 2 );
  REQUIRE( d == klfengine::value::dict{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  } );
}

TEST_CASE("dict_take throws error on nonexistent key", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE_THROWS_AS( klfengine::dict_take(d, "D") , std::out_of_range );

  REQUIRE( d.size() == 3 );
  REQUIRE( d == klfengine::value::dict{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  } );
}

TEST_CASE("dict_take with default value finds existing value in dict", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_take(d, "B", std::string{"XXX"})
           == std::string{"value of B"} );

  REQUIRE( d.size() == 2 );
  REQUIRE( d == klfengine::value::dict{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  } );
}

TEST_CASE("dict_take returns default value on nonexistent key in dict", "[value]")
{
  klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  REQUIRE( klfengine::dict_take(d, "D", std::string{"XXX"})
           == std::string{"XXX"} );

  REQUIRE( d.size() == 3 );
  REQUIRE( d == klfengine::value::dict{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  } );
}


// --


TEST_CASE("parameter_taker finds fields", "[value]")
{
  const klfengine::value::dict d{
    {"A", klfengine::value{1}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  // keep param within a C++ block scope
  {
    klfengine::parameter_taker param(d, "phase 1");
    param.disable_check();

    REQUIRE( param.has<int>("A") );
    REQUIRE( ! param.has<std::string>("A") );
    REQUIRE( param.has_castable_to<int>("A") );
    REQUIRE( param.has_castable_to<double>("A") );
    REQUIRE( ! param.has_castable_to<klfengine::value::dict>("A") );
  }

}

TEST_CASE("parameter_taker parses parameters as expected", "[value]")
{
  const klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  // keep param within a C++ block scope
  {
    klfengine::parameter_taker param(d, "phase 1");

    std::string value_A = param.take<std::string>("A");
    klfengine::value value_B = param.take<klfengine::value>("B");
    klfengine::value value_C = param.take("C");
    param.finished();

    REQUIRE(value_A == "value of A");
    REQUIRE(value_B == klfengine::value{std::string{"value of B"}});
    REQUIRE(value_C == klfengine::value{std::string{"value of C"}});
  }

}

TEST_CASE("parameter_taker can get parameters with casting", "[value]")
{
  const klfengine::value::dict d{
    {"A", klfengine::value{std::string{"True"}}},
    {"B", klfengine::value{std::string{"+1.60e+001"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  // keep param within a C++ block scope
  {
    klfengine::parameter_taker param(d, "phase 1");

    bool value_A = param.take_cast<bool>("A", false);
    double value_B = param.take_cast<double>("B");
    std::string value_C = param.take_cast<std::string>("C");
    param.finished();

    REQUIRE(value_A == true);
    REQUIRE(value_B == 16.0);
    REQUIRE(value_C == std::string{"value of C"});
  }

}


TEST_CASE("parameter_taker can do_if", "[value]")
{
  const klfengine::value::dict d{
    {"A", klfengine::value{true}},
    {"B", klfengine::value{std::string{"+1.60e+001"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  // keep param within a C++ block scope
  {
    klfengine::parameter_taker param(d, "phase 1");

    bool called_A = false;
    bool have_A = param.take_and_do_if<bool>("A", [&called_A](bool val) {
      REQUIRE(val == true);
      called_A = true;
    });
    REQUIRE(have_A);
    REQUIRE(called_A);

    bool have_D = param.take_and_do_if<klfengine::value>(
        "D",
        [](const klfengine::value & ) { }
    );
    REQUIRE(!have_D);

    bool have_E = param.take_and_do_if<int>("E", [](int) { });
    REQUIRE(!have_E);

    bool called_B = false;
    bool have_B = param.take_and_do_if<klfengine::value>(
        "B",
        [&](const klfengine::value & val) {
          REQUIRE( val.get_cast<double>() == 16.0 );
          called_B = true;
        }
    );
    REQUIRE(have_B);
    REQUIRE(called_B);

    (void) param.take_cast<std::string>("C");

    param.finished();
  }

}


TEST_CASE("parameter_taker.finished() checks that all parameters were take()en", "[value]")
{
  const klfengine::value::dict d{
    {"A", klfengine::value{std::string{"value of A"}}},
    {"B", klfengine::value{std::string{"value of B"}}},
    {"C", klfengine::value{std::string{"value of C"}}}
  };

  // keep param within a C++ block scope
  {
    klfengine::parameter_taker param(d, "phase 2");

    std::string value_A = param.take<std::string>("A");
    // forgot to take "B"
    klfengine::value value_C = param.take("C");

    REQUIRE(value_A == "value of A");
    REQUIRE(value_C == klfengine::value{std::string{"value of C"}});

    CHECK_THROWS_AS( param.finished(), klfengine::invalid_parameter ) ;
    CHECK_THROWS_WITH( param.finished(), Catch::Contains("\"B\"") ) ;
  };

}
