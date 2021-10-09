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

#include <string>
#include <vector>
#include <map>
#include <functional>


#include <klfengine/basedefs>


#include <nlohmann/json.hpp>


#ifdef KLFENGINE_USE_MPARK_VARIANT
// Use mpark/variant as variant type instead of C++17 std::variant.  Set this
// e.g. as fallback variant type for C++11 & C++14.
#include <mpark/variant.hpp>

#else

// simply use C++17 std::variant
#include <variant>

#endif



namespace klfengine {


/** \brief String to boolean parser
 *
 * Recognizes values such as "T" or "1" or "on" for true and "F", "false", "0",
 * etc. for false.  Whitespace is stripped.
 *
 * Throws \ref std::invalid_argument if a clean boolean couldn't be parsed.
 */
bool parse_boolean(std::string str);




namespace detail {

#ifdef KLFENGINE_USE_MPARK_VARIANT
// fallback variant library for C++ < C++17
template<typename... Args>
using variant_type = mpark::variant<Args...>;
#define _KLFENGINE_VARIANT_GET mpark::get
#define _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE mpark::holds_alternative
#define _KLFENGINE_VARIANT_VISIT mpark::visit
#else
template<typename... Args>
using variant_type = std::variant<Args...>;
#define _KLFENGINE_VARIANT_GET std::get
#define _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE std::holds_alternative
#define _KLFENGINE_VARIANT_VISIT std::visit
#endif


// -- helpers for casting variant types --

struct dummy_type {};

template<typename ToType>
struct variant_cast_helper
{
  // provide at least one operator() so that "using Base::operator()" works
  inline void operator()(dummy_type, dummy_type) {}
};
template<>
struct variant_cast_helper<int>
{
  inline int operator()(const std::string & value_s)
  {
    std::size_t ppos;
    int value = std::stoi(value_s, &ppos);
    while (ppos < value_s.size() && std::isspace(value_s[ppos])) {
      ++ppos;
    }
    if (ppos < value_s.size()) {
      throw std::invalid_argument{ "Invalid integer: `" + value_s + "'" };
    }
    return value;
  }
};
template<>
struct variant_cast_helper<double>
{
  inline double operator()(int value) { return static_cast<double>(value); }
  inline double operator()(const std::string & value_s)
  {
    std::size_t ppos;
    double value = std::stod(value_s, &ppos);
    while (ppos < value_s.size() && std::isspace(value_s[ppos])) {
      ++ppos;
    }
    if (ppos < value_s.size()) {
      throw std::invalid_argument{ "Invalid real number: `" + value_s + "'" };
    }
    return value;
  }
};
template<>
struct variant_cast_helper<bool>
{
  inline bool operator()(int value) { return static_cast<bool>(value); }
  inline bool operator()(std::string str) { return parse_boolean(std::move(str)); }
};


// -- helpers for the helpers for casting variant types --


template<typename X>
struct dummy_internal_int { using dummy_type = int; };

template<typename ToType>
struct variant_has_castable_helper_visitor
{
private:
  // adapted from https://stackoverflow.com/a/31539364/1694896
  template<typename FromTypeNoQual, typename T>
  static std::true_type test_signature(ToType (T::*)(FromTypeNoQual) );
  template<typename FromTypeNoQual, typename T>
  static std::true_type test_signature(ToType (T::*)(const FromTypeNoQual &) );

  template<typename FromTypeNoQual, typename T>
  static auto test(std::nullptr_t)
    -> decltype(test_signature<FromTypeNoQual, variant_cast_helper<ToType> >(&T::operator()));

  template<typename FromTypeNoQual, typename T>
  static std::false_type test(...);

  template<typename FromTypeNoQual>
  using _is_castable_type =
    decltype(test<FromTypeNoQual, variant_cast_helper<ToType> >(nullptr));

public:
  template<typename FromTypeNoQual>
  using is_castable_type = _is_castable_type<FromTypeNoQual>;

  template<typename FromType>
  inline bool operator()(FromType && )
  {
    using FromTypeNoQual =
      typename std::remove_cv<
        typename std::remove_reference<FromType>::type
      >::type
      ;
    // fprintf(stderr, "DEBUG!!! can cast '%s' to '%s' ? -> same=%d, castable=%d\n",
    //         get_type_name<FromTypeNoQual>().c_str(),
    //         get_type_name<ToType>().c_str(),
    //         (int)std::is_same<FromTypeNoQual, ToType>::value,
    //         (int)is_castable_type<FromTypeNoQual>::value);
            
    return
      std::is_same<FromTypeNoQual, ToType>::value ||
      is_castable_type<FromTypeNoQual>::value;
  }

  // template<typename FromType,
  //          typename std::enable_if<decltype(is_castable_type<FromType>())::value, int>::type* = nullptr
  //          >
  // bool operator()(FromType && )
  // {
  //   return true;
  // }
  // template<typename FromType,
  //          typename std::enable_if<! decltype(is_castable_type<FromType>())::value, int>::type* = nullptr
  //          >
  // bool operator()(FromType && )
  // {
  //   return false;
  // }
};

// // DEBUGGGGG
// static_assert(
// //  decltype(variant_has_castable_helper_visitor<nullptr_t>::is_castable_type<nullptr_t>())::value
//   std::is_same<
//     decltype(variant_has_castable_helper_visitor<nullptr_t>().operator()(nullptr)),
//     int
//   >::value
// ,
// "");
//static_assert( std::is_same<std::nullptr_t, std::nullptr_t>::value , "");
// static_assert(
//   decltype(variant_has_castable_helper_visitor<double>::test_signature<int, variant_cast_helper<double> >(&variant_cast_helper<double>::operator()))
// ,
// ""
// )

template<typename ToType>
struct variant_cast_helper_full_visitor
  : public variant_cast_helper<ToType>
{
  inline ToType operator()(ToType value) { return value; }

  using variant_cast_helper<ToType>::operator();

  // template<
  //   typename FromType,
  //   typename std::enable_if<
  //     variant_has_castable_helper_visitor<FromType>::template is_castable_type<ToType>::value,
  //     int
  //   > = 1
  // >
  // inline ToType operator()(FromType && value) {
  //   return variant_cast_helper<ToType>()(std::forward<FromType>(value));
  // }

  template<
    typename FromType
    //,
    // typename std::enable_if<
    //   ! variant_has_castable_helper_visitor<FromType>::template is_castable_type<ToType>::value,
    //   int
    // > = 1
  >
  inline ToType operator()(FromType &&) {
    throw invalid_value{
      "Cannot convert klfengine::value from `" + get_type_name<FromType>() + "' "
      "to `" + get_type_name<ToType>() + "'"
    };
  }
};

template<typename T>
struct simplified_type_name
{
  static inline std::string the_name() { return get_type_name<T>(); }
};
// some simplified types, too
template<>
struct simplified_type_name<std::nullptr_t>
{
  static inline std::string the_name() {
    return "null";
  }
};
template<>
struct simplified_type_name<std::string>
{
  static inline std::string the_name() {
    return "string";
  }
};
template<typename... T>
struct simplified_type_name<std::vector<T...> >
{
  static inline std::string the_name() {
    return "array";
  }
};
template<typename... T>
struct simplified_type_name<std::map<T...> >
{
  static inline std::string the_name() {
    return "dict";
  }
};

struct simplified_type_name_visitor
{
  template<typename HeldType>
  inline std::string operator()(const HeldType &)
  {
    return simplified_type_name<HeldType>::the_name();
  }
};

// --

// see https://stackoverflow.com/a/43309497/1694896
template<typename... PrimaryDataTypes>
struct recursive_variant_with_vector_and_map
{
  using this_type = recursive_variant_with_vector_and_map<PrimaryDataTypes...>;

  using array = std::vector<this_type>;
  using dict = std::map<std::string,this_type>;

  // !!! CAUTION !!! ANY CHANGE TO THE ORDER HERE MUST BE REFLECTED IN THE
  // FUNCTION to_json() IN value.hxx !!!
  using this_variant_type = variant_type<PrimaryDataTypes..., array, dict>;
  this_variant_type _data;

  // --

  inline std::string get_type_name() const
  {
    return _KLFENGINE_VARIANT_VISIT(simplified_type_name_visitor(), _data);
  }

  // get(), const version
  template<typename GetValueType,
           typename std::enable_if<!std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0 >
  inline const GetValueType & get() const {
    try {
      return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
    } catch (exception & e) {
      throw invalid_value{
        "Requested `" + klfengine::detail::get_type_name<GetValueType>()
        + "' but value contains a `"
        + get_type_name() + "'"
      };
    }
  }
  // get(), non-const version
  template<typename GetValueType,
           typename std::enable_if<!std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0>
  inline GetValueType & get() {
    try {
      return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
    } catch (exception & e) {
      throw invalid_value{
        "Requested `" + klfengine::detail::get_type_name<GetValueType>()
        + "' but value contains a `"
        + get_type_name() + "'"
      };
    }
  }
  // get<value>(), helper access function for dict_get, parameter_taker, etc.
  template<typename GetValueType,
           typename std::enable_if<std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0>
  inline const GetValueType & get() const {
    return *this;
  }
  template<typename GetValueType,
           typename std::enable_if<std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0 >
  inline GetValueType & get() {
    return *this;
  }

  // --
  
  template<typename GetValueType>
  inline bool has_type() const {
    return _KLFENGINE_VARIANT_HOLDS_ALTERNATIVE<GetValueType>(_data);
  }

  // template<typename FnVisitor>
  // inline void visit(FnVisitor && fn) const
  // {
  //   _KLFENGINE_VARIANT_VISIT(fn, _data);
  // }
  template<typename FnVisitor>
  inline auto visit(FnVisitor && fn) const ->
    typename std::enable_if<
        std::is_same<decltype(_KLFENGINE_VARIANT_VISIT(fn, _data)),void>::value,
        decltype(_KLFENGINE_VARIANT_VISIT(fn, _data))
    >::type
  {
    _KLFENGINE_VARIANT_VISIT(fn, _data);
  }

  template<typename FnVisitor>
  inline auto visit(FnVisitor && fn) const ->
    typename std::enable_if<
        ! std::is_same<decltype(_KLFENGINE_VARIANT_VISIT(fn, _data)),void>::value,
        decltype(_KLFENGINE_VARIANT_VISIT(fn, _data))
    >::type
  {
    return _KLFENGINE_VARIANT_VISIT(fn, _data);
  }

  // template<typename FnVisitor>
  // inline auto transform(FnVisitor && fn) const
  //   -> decltype( _KLFENGINE_VARIANT_VISIT(fn, _data) )
  // {
  //   return _KLFENGINE_VARIANT_VISIT(fn, _data);
  // }


  // like get() but with cast between certain compatible types
  
  template<typename GetValueType>
  GetValueType get_cast() const
  {
    return visit(variant_cast_helper_full_visitor<GetValueType>());
  }

  template<typename GetValueType>
  inline bool has_castable_to() const
  {
    return visit(variant_has_castable_helper_visitor<GetValueType>());
  }


  // template<typename RhsType>
  // inline recursive_variant_with_vector_and_map
  // operator=(RhsType && rhs) {
  //   return _data.operator=(std::forward<RhsType>(rhs));
  // }

  inline bool operator==(const this_type& rhs) const {
    return _data == rhs._data;
  }
  inline bool operator!=(const this_type& rhs) const {
    return _data != rhs._data;
  }
};


// !!! CAUTION !!! ANY CHANGE TO THESE PRIMARY TYPES OR THEIR ORDER MUST BE
// REFLECTED IN THE FUNCTION to_json() IN value.hxx !!!
using value = recursive_variant_with_vector_and_map<
  bool,
  int,
  double,
  std::nullptr_t,
  std::string
>;

// HAVING 'value' IN 'detail' NAMESPACE MAKES IT MORE CLEAR WHY to_json() AND
// from_json() SHOULD LIVE HERE, TOO.

void to_json(nlohmann::json & j, const value & v);
void from_json(const nlohmann::json & j, value & v);



// provide some hashing functions
std::size_t hash_value(const value & v);
std::size_t hash_value_dict(const value::dict & d);


} // namespace detail


/** \brief Store standard JSON-like types, including arrays and maps
 *
 * This is a std::variant type that can store ints, bools, doubles, strings,
 * as well as arrays and maps of such types (maps always have strings as keys).
 *
 * (If KLFENGINE_USE_MPARK_VARIANT is set, the <a
 * href="https://github.com/mpark/variant" target="_blank">mpark/variant</a>
 * library is used instead of std::variant.  See "Using the klfengine library".)
 *
 * You can construct arbitrary values with initializer lists:
 * \code
 *   klfengine::value{
 *     klfengine::value::array{
 *       klfengine::value{v1},
 *       klfengine::value{klfengine::value::dict{
 *         {std::string{"key1"}, dictvalue1},
 *         {std::string{"key2"}, dictvalue2},
 *         (...)
 *       }},
 *       (...)
 *     }
 *  }
 * \endcode
 *
 * \warning Always use explicit std::string's when constructing
 *    klfengine::value's, and not const char * constants, because <a
 *    href="https://stackoverflow.com/a/60683920/1694896">const char * values
 *    get converted to bool</a>.
 *
 * Important members:
 *
 * <b>value::array</b>: a typedef of <code>std::vector<klfengine::value></code>
 *
 * <b>value::dict</b>: a typedef of <code>std::map<std::string,
 * klfengine::value></code>
 */
using value = detail::value;



/** \brief Fetch a value in a map by key
 *
 * This function throws \a std::out_of_range if the key is not in the
 * dictionary.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X = value>
inline X dict_get(const value::dict & dict, const std::string & key)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    throw std::out_of_range("No such key in dictionary: " + key);
  }
  return it->second.get<X>();
}

/** \brief Fetch a value in a map by key, possibly with a default if the key
 *         does not exist.
 *
 * If the given \a key cannot be found in the dictionary, then \a dflt is
 * returned instead.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X>
inline X dict_get(const value::dict & dict, const std::string & key, X dflt)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return dflt; // std::move redundant
  }
  return it->second.get<X>();
}

/** \brief Take a value in a map by key
 *
 * If the given key is found in the dictionary, the associated value is returned
 * and the key is removed from the dictionary.  This function throws \a
 * std::out_of_range if the key is not in the dictionary.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X = value>
inline X dict_take(value::dict & dict, const std::string & key)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    throw std::out_of_range("No such key in dictionary: " + key);
  }
  value val{ std::move(it->second) };
  dict.erase(it);
  return val.get<X>();
}

/** \brief Take a value from a map by key, possibly with a default if the key
 *         does not exist.
 *
 * If the given key is found in the dictionary, the associated value is returned
 * and the key is removed from the dictionary.  If the given \a key cannot be
 * found in the dictionary, then \a dflt is returned instead.
 *
 * If \a X is a standard klfengine::value type (bool, int, etc.), then the
 * corresponding value is returned.  If \a X is klfengine::value, the
 * klfengine::value object is returned.
 */
template<typename X>
inline X dict_take(value::dict & dict, const std::string & key, X dflt)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return dflt; // std::move redundant
  }
  value val{ std::move(it->second) };
  dict.erase(it);
  return val.get<X>();
}

/** \brief Execute the given callback if a key exists in the dictionary
 *
 * If the given \a key is found in the dictionary \a dict, then \a fn is
 * executed with the value associated with that key.  If \a key was not found,
 * then nothing happens.
 *
 * Returns \a true if the key was found (and therefore \a fn was executed),
 * otherwise returns \a false.
 */
template<typename X>
inline bool dict_do_if(const value::dict & dict, const std::string & key,
                       std::function<void(const X&)> fn)
{
  auto it = dict.find(key);
  if (it == dict.end()) {
    return false;
  }
  fn(it->second.get<X>());
  return true;
}






namespace detail {

using paramdict_type = std::map<std::string,const value*>;

class paramdict_citerator : public value::dict::const_iterator
{
public:
  paramdict_citerator(value::dict::const_iterator it)
    : value::dict::const_iterator(std::move(it))
  {}

  inline paramdict_type::value_type operator*() const
  {
    return std::make_pair(
        value::dict::const_iterator::operator->()->first,
        & value::dict::const_iterator::operator->()->second
    );
  }

private:
  inline paramdict_type::value_type operator->() const
  {
    throw std::runtime_error{"no operator-> for paramdict_citerator"};
  }
};

class paramdict_to_dict_citerator : public paramdict_type::const_iterator
{
public:
  paramdict_to_dict_citerator(paramdict_type::const_iterator it)
    : paramdict_type::const_iterator(std::move(it))
  {}

  inline value::dict::value_type operator*() const
  {
    return std::make_pair(
        paramdict_type::const_iterator::operator->()->first,
        * paramdict_type::const_iterator::operator->()->second
    );
  }

private:
  inline value::dict::value_type operator->() const
  {
    throw std::runtime_error{"no operator-> for paramdict_to_dict_citerator"};
  }
};


} // namespace detail




/** \brief Utility to parse parameters provided as a \ref klfengine::value::dict structure
 *
 * Use this helper class as follows:
 * \code
 * void myfunction(const value::dict & parameters)
 * {
 *   parameter_taker param(parameters, "myfunction");
 *   bool use_raster = param.take("raster", false); // false is default
 *   int dpi = -1;
 *   if (use_raster) {
 *     dpi = param.take("dpi", 600); // 600 is default
 *   }
 *   // check that all parameters were take()en and throw an exception
 *   // if that's not the case
 *   param.finished();
 *
 *   // ... do something ...
 *
 * }
 * \endcode
 *
 * The original dictionary object specified to the constructor is not modified.
 *
 * Call \ref finished() to check that all parameters have been consumed by calls
 * to \ref take().  (If you don't call finished(), then the destructor will also
 * perform this check, but will only produce a warning message without throwing
 * an exception.)
 *
 * \warning You must ensure that the dictionary reference provided to the
 *          constructor remains valid and constant during the entire lifetime of
 *          the parameter_taker instance.
 */
class parameter_taker
{
public:
  /** \brief Initialize the parameter_taker with a value::dict const reference
   *
   * The provided dictionary reference must be valid and unchanged during the
   * entire lifetime of the present \a parameter_taker instance.
   */
  explicit parameter_taker(const value::dict & dict_,
                           std::string what_ = std::string{})
    : _paramdict(
        detail::paramdict_citerator(dict_.begin()),
        detail::paramdict_citerator(dict_.end())
      ),
      _what(what_),
      _check_all_taken_called(false)
  {
    // for (const auto & p : dict_) {
    //   fprintf(stderr, "DEBUG: original dictionary got item = %s, ptr = %p\n",
    //           p.first.c_str(), & p.second);
    // }
    // for (const auto & p : _paramdict) {
    //   fprintf(stderr, "DEBUG: our own pointer dictionary got item = %s, ptr = %p\n",
    //           p.first.c_str(), p.second);
    // }
  }

  /** \brief Destructor checks that all parameters were "take()en"
   *
   * The destructor calls check_all_taken(), unless you have already called
   * check_all_taken() at least once.
   */
  ~parameter_taker()
  {
    if (!_check_all_taken_called) {
      _check_all_taken(false);
    }
  }

  /** \brief Check that all parameters were "take()en"
   *
   * If there are any remaining parameters for which \ref take() was not called,
   * an \a klfengine::invalid_parameter exception is thrown.
   */
  void finished()
  {
    _check_all_taken(true);
  }

  /** \brief Don't worry about remaining parameters
   *
   * Call this if you want the parameter_taker not to warn about un-take()-en
   * parameters in its destructor.  For instance, you could call this
   * immediately before throwing an exception to report another error.
   */
  void disable_check()
  {
    _check_all_taken_called = true;
  }

  inline std::string what() const { return _what; }

  parameter_taker(const parameter_taker & copy) = delete;
  parameter_taker(parameter_taker && move) = delete;
  parameter_taker & operator=(const parameter_taker & copy) = delete;
  parameter_taker & operator=(parameter_taker && move) = delete;

  inline bool has(const std::string & key)
  {
    auto it = _paramdict.find(key);
    return (it != _paramdict.end());
  }
  template<typename X>
  inline bool has(const std::string & key)
  {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return false;
    }
    return it->second->has_type<X>();
  }
  template<typename X>
  inline bool has_castable_to(const std::string & key)
  {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return false;
    }
    return it->second->has_castable_to<X>();
  }

  template<typename X = value>
  const X & take(const std::string & key) {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      throw std::out_of_range("No such key in dictionary: " + key);
    }
    const value * val = it->second;
    _paramdict.erase(it);
    return val->get<X>();
  }

  template<typename X = value>
  X take_cast(const std::string & key) {
    return take<value>(key).get_cast<X>();
  }

  template<typename X>
  const X & take(const std::string & key, const X & dflt) {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return dflt;
    }
    const value * val = it->second;
    _paramdict.erase(it);
    return val->get<X>();
  }

  template<typename X>
  X take_cast(const std::string & key, const X & dflt) {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return dflt;
    }
    const value * val = it->second;
    _paramdict.erase(it);
    return val->get_cast<X>();
  }

  template<typename X = value>
  inline bool take_and_do_if(const std::string & key,
                             std::function<void(const X&)> fn)
  {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return false;
    }
    const value * val = it->second;
    _paramdict.erase(it);
    fn(val->get<X>());
    return true;
  }


  value::dict get_remaining() const
  {
    return value::dict{
      detail::paramdict_to_dict_citerator( _paramdict.begin() ),
      detail::paramdict_to_dict_citerator( _paramdict.end() )
    };
  }
  value::dict take_remaining()
  {
    value::dict rem = get_remaining();
    _paramdict.clear();
    return rem;
  }

private:
  detail::paramdict_type _paramdict;
  std::string _what;
  bool _check_all_taken_called;

  void _check_all_taken(bool throw_exception = true)
  {
    _check_all_taken_called = true;
    if (!_paramdict.empty()) {
      std::string msg = "superfluous key(s) ";
      bool first = true;
      for ( const std::pair<std::string, const value *> & p : _paramdict ) {
        if (!first) {
          msg += ",";
        } else {
          first = false;
        }
        msg += "\""+p.first+"\"";
      }

      if (throw_exception) {
        throw invalid_parameter{_what, msg};
      } else {
        warn(_what, "invalid parameters, " + msg);
      }
    }
  }
};



namespace detail {
// Support hashing klfengine::value e.g. to allow format_spec to be a valid key
// type in an std::unordered_map
template<> struct hash<klfengine::value>
{
  std::size_t operator()(klfengine::value const & v) const noexcept
  {
    return hash_value(v);
  }
};
template<> struct hash<klfengine::value::dict>
{
  std::size_t operator()(klfengine::value::dict const & v) const noexcept
  {
    return hash_value_dict(v);
  }
};
} // namespace detail

} // namespace klfengine



// // custom specialization of std::hash can be injected in namespace std
// //
// // hash of klfengine::value is used e.g. to allow format_spec to be a valid key
// // type in an std::unordered_map
// namespace std {
// template<> struct hash<klfengine::value>
// {
//   std::size_t operator()(klfengine::value const & v) const noexcept
//   {
//     return klfengine::detail::hash_value(v);
//   }
// };
// template<> struct hash<klfengine::value::dict>
// {
//   std::size_t operator()(klfengine::value::dict const & v) const noexcept
//   {
//     return klfengine::detail::hash_value_dict(v);
//   }
// };
// } // namespace std



#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/value.hxx>
#endif
