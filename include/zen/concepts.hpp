#ifndef ZEN_CONCEPTS_HPP
#define ZEN_CONCEPTS_HPP

#include <concepts>
#include <iterator>
#include <type_traits>
#include <memory>

#include "zen/config.hpp"
#include "zen/compat.hpp"

ZEN_NAMESPACE_START

template<typename T>
concept range = requires (T& a) {
  { a.begin() } -> std::input_iterator;
  { a.end() } -> std::input_iterator;
};

template<typename T>
concept const_range = range<T>
  && std::same_as<iter_const_reference_t<T>, std::iter_reference_t<T>>;

template<typename T>
concept container = range<T> && requires (T& t) {
  T::value_type;
};

template<range T>
using range_iterator_t = decltype(std::declval<T&>().begin());

template<range T>
using range_reference_t = std::iter_reference_t<range_iterator_t<T>>;

template<typename T>
struct is_pointer : std::false_type {};

template<typename T>
struct is_pointer<T*> : std::true_type {};

template<typename T>
struct is_pointer<std::unique_ptr<T>> : std::true_type {};

template<typename T>
struct is_pointer<std::shared_ptr<T>> : std::true_type {};

template<typename T>
concept pointer = is_pointer<T>::value;

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_CONCEPTS_HPP
