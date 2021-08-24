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

  template<typename GetValueType,
           typename std::enable_if<!std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0 >
  inline const GetValueType & get() const {
    return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
  }
  template<typename GetValueType,
           typename std::enable_if<!std::is_same<GetValueType, this_type>::value,
                                   int>::type = 0>
  inline GetValueType & get() {
    return _KLFENGINE_VARIANT_GET<GetValueType>(_data);
  }

  // helper access function .get<value>() for dict_get, dict_take, parameter_taker, etc.
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

  template<typename FnVisitor>
  inline void visit(FnVisitor && fn) const
  {
    _KLFENGINE_VARIANT_VISIT(fn, _data);
  }
  template<typename FnVisitor>
  inline auto transform(FnVisitor && fn) const
    -> decltype( _KLFENGINE_VARIANT_VISIT(fn, _data) )
  {
    return _KLFENGINE_VARIANT_VISIT(fn, _data);
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
   * parameters in its destructor.
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

  inline bool has(std::string key)
  {
    auto it = _paramdict.find(key);
    return (it != _paramdict.end());
  }

  template<typename X = value>
  const X & take(std::string key) {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      throw std::out_of_range("No such key in dictionary: " + key);
    }
    const value * val = it->second;
    _paramdict.erase(it);
    return val->get<X>();
  }

  template<typename X>
  const X & take(std::string key, const X & dflt) {
    auto it = _paramdict.find(key);
    if (it == _paramdict.end()) {
      return dflt;
    }
    const value * val = it->second;
    _paramdict.erase(it);
    return val->get<X>();
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

} // namespace klfengine



// custom specialization of std::hash can be injected in namespace std
//
// hash of klfengine::value is used e.g. to allow format_spec to be a valid key
// type in an std::unordered_map
namespace std {
template<> struct hash<klfengine::value>
{
  std::size_t operator()(klfengine::value const & v) const noexcept
  {
    return klfengine::detail::hash_value(v);
  }
};
template<> struct hash<klfengine::value::dict>
{
  std::size_t operator()(klfengine::value::dict const & v) const noexcept
  {
    return klfengine::detail::hash_value_dict(v);
  }
};
} // namespace std



#ifndef _KLFENGINE_DONT_INCLUDE_IMPL_HXX
#include <klfengine/impl/value.hxx>
#endif
