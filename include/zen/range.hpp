#ifndef ZEN_ITERATOR_RANGE_HPP
#define ZEN_ITERATOR_RANGE_HPP

#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "zen/meta.hpp"
#include "zen/mapped_iterator.hpp"
#include "zen/zip_iterator.hpp"

ZEN_NAMESPACE_START

template<typename IterT>
class iterator_range {

  IterT left;
  IterT right;

  using trait_type = std::iterator_traits<IterT>;

public:

  using value_type = typename trait_type::value_type;
  using reference = typename trait_type::reference;
  using iterator = IterT;
  using difference_type = typename trait_type::difference_type;

  iterator_range(IterT left, IterT right):
    left(left), right(right) {}

  IterT begin() {
    return left;
  }

  IterT end() {
    return right;
  }

  iterator_range& operator++() requires std::incrementable<IterT> {
    ++left;
    return *this;
  }

  iterator_range operator++(int) requires std::incrementable<IterT> {
    return iterator_range { left++, right };
  }

  iterator_range& operator--() requires std::incrementable<IterT> {
    --left;
    return *this;
  }

  iterator_range operator--(int) requires std::incrementable<IterT> {
    return iterator_range { left--, right };
  }

  template<typename F>
  auto map(F func) {
    using iter = mapped_iterator<IterT, F>;
    return iterator_range<iter> {
      iter { left, func },
      iter { right, func },
    };
  }

  auto map_first() {
    return map([](auto pair) { return pair.first; });
  }

  auto map_second() {
    return map([](auto pair) { return pair.second; });
  }

};

template<typename IterT>
auto make_iterator_range(IterT&& a, IterT&& b) {
  return iterator_range<IterT>(std::forward<IterT>(a), std::forward<IterT>(b));
}

/**
 * Construct an iterator range from a pair of iterators.
 *
 * This function is especially useful for methods in the standard library that
 * return such a pair of iterators, such as std::unordered_map::equal_range.
 */
template<typename IterT>
auto make_iterator_range(std::pair<IterT, IterT>&& pair) {
  return iterator_range<IterT>(std::forward<IterT>(pair.first), std::forward<IterT>(pair.second));
}

template<typename T>
concept RangeLike = requires (T& a) {
  { a.begin() } -> std::input_iterator;
  { a.end() } -> std::input_iterator;
};

/**
 * This type is defined in the standard library as
 * [std::iter_const_reference_t][1] but we want to support down to C++ 20.
 *
 * [1]: https://en.cppreference.com/w/cpp/iterator/iter_t.html
 */
template< std::indirectly_readable T >
using iter_const_reference_t = std::common_reference_t<
  const std::iter_value_t<T>&&,
  std::iter_reference_t<T>
>;

template<typename T>
concept ConstRangeLike = RangeLike<T> && std::same_as<iter_const_reference_t<T>, std::iter_reference_t<T>>;

template<RangeLike T>
auto make_iterator_range(T& container) {
  return make_iterator_range(
    std::begin(container),
    std::end(container)
  );
}

template<RangeLike T>
auto make_const_iterator_range(const T& container) {
  return make_iterator_range(
    std::cbegin(container),
    std::cend(container)
  );
}

/**
 * This specialization allows users to pass an rvalue directly to the zip_iterator.
 *
 * Eventually, we will want to apply this step to only a single argument.
 */
template<std::input_iterator ... IterT>
auto zip(iterator_range<IterT> ...range) {
  return make_iterator_range(
    zip(std::begin(range)...),
    zip(std::end(range)...)
  );
}

/**
 * Since a range might be a container, rvalues are not allowed.
 */
template<RangeLike ...Ts>
auto zip(Ts& ...args) {
  return make_iterator_range(
    zip(std::begin(args)...),
    zip(std::end(args)...)
  );
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ITERATOR_RANGE_HPP
