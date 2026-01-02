/// @file
/// @brief Support for creating ranges over iterators.
#ifndef ZEN_ITERATOR_RANGE_HPP
#define ZEN_ITERATOR_RANGE_HPP

#include <boost/hana/fwd/tuple.hpp>
#include <istream>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "zen/config.hpp"
#include "zen/concepts.hpp"
#include "zen/compat.hpp"
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


  /// Get an iterator pointing to the first element in this range.
  IterT begin() {
    return left;
  }

  /// Get an iterator pointing just beyond the last element in this range.
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

/// Create an @ref iterator_range directly out of a start iterator and an end
/// iterator.
///
/// The resulting object has methods [begin()](@ref iterator_range::begin) and
/// [end()](@ref iterator_range::end), which makes it compatible with for-loops.
///
/// @see make_iterator_range(std::pair<IterT, IterT>&& )
template <typename IterT> auto make_iterator_range(IterT &&a, IterT &&b) {
  return iterator_range<IterT> { std::forward<IterT>(a), std::forward<IterT>(b) };
}

/// Construct an iterator range from a pair of iterators.
///
/// This function is especially useful for methods in the standard library that
/// return such a pair of iterators, such as [std::unordered_map::equal_range][1].
///
/// @include make_iterator_range_equal_range.cc
///
/// @see make_iterator_range(IterT&&, IterT&&)
///
/// [1]: https://en.cppreference.com/w/cpp/container/unordered_map/equal_range.html
template<typename IterT>
auto make_iterator_range(std::pair<IterT, IterT>&& pair) {
  return iterator_range<IterT> { std::forward<IterT>(pair.first), std::forward<IterT>(pair.second) };
}

template<range T>
auto make_iterator_range(T& container) {
  return make_iterator_range(
    std::begin(container),
    std::end(container)
  );
}

// By default, we don't allow rvalues.
//
// For example, a container passed to `zip` would be destroyed before the
// iterator can run.
template <typename T>
struct _zip_accept_rvalue : std::false_type {};

// An iterator_range should be passed by value.
template<typename IterT>
struct _zip_accept_rvalue<iterator_range<IterT>> : std::true_type {};

template<typename ...Ts>
struct is_zippable {
  static constexpr bool value = hana::all(
    hana::transform(
      hana::make_tuple(hana::type_c<Ts>...),
      [](auto t) {
        using T = decltype(+t)::type;
        return std::is_lvalue_reference_v<T>
            || _zip_accept_rvalue<std::remove_reference_t<T>>::value;
      }
    )
  );
};

template<typename ...Ts>
using is_zippable_v = is_zippable<Ts...>::value;

/// Create an @ref iterator_range that zips over the given arguments.
///
/// To create a zipper that only holds constant references, use [std::as_const][1]
///
/// [1]: https://en.cppreference.com/w/cpp/utility/as_const.html
template<range ...Ts>
auto zip(Ts&& ...args) {
  static_assert(
    is_zippable<Ts...>::value,
    "the provided value cannot be passed in as an rvalue"
  );
  return make_iterator_range(
    zip(std::begin(args)...),
    zip(std::end(args)...)
  );
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ITERATOR_RANGE_HPP
