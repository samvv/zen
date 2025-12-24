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

template<typename T>
concept ConstRangeLike = requires (T& a) {
  { a.cbegin() } -> std::input_iterator;
  { a.cend() } -> std::input_iterator;
};

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

template<RangeLike ...Ts>
auto zip(Ts& ...args) {
  return make_iterator_range(
    zip(std::begin(args)...),
    zip(std::end(args)...)
  );
}

template<ConstRangeLike ...Ts>
auto czip(const Ts& ...args) {
  return make_iterator_range(
    zip(std::cbegin(args)...),
    zip(std::cend(args)...)
  );
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ITERATOR_RANGE_HPP
