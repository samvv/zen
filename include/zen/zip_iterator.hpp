/// @file
/// @brief Definitions for an iterator that merges several other iteerators.

#ifndef ZEN_ZIP_ITERATOR_HPP
#define ZEN_ZIP_ITERATOR_HPP

#include <algorithm>
#include <boost/hana/fwd/fold.hpp>
#include <boost/hana/fwd/index_if.hpp>
#include <boost/hana/fwd/tuple.hpp>
#include <boost/hana/fwd/type.hpp>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>

#include "zen/config.hpp"
#include "zen/hana.hpp"
#include "zen/algorithm.hpp"

ZEN_NAMESPACE_START

template<typename IterT>
using iterator_category_t = std::iterator_traits<IterT>::iterator_category;

constexpr auto _iterator_categories = to_hana_tuple_t<std::tuple<
  std::input_iterator_tag
, std::forward_iterator_tag
, std::bidirectional_iterator_tag
, std::random_access_iterator_tag
, std::contiguous_iterator_tag
>>::value;

// template<typename T>
// struct common_iterator_category : meta::fold1<
//     meta::bind<
//       meta::min_by_t<
//         meta::index_t<tags, meta::_1>,
//         meta::map_t<T, meta::bind<iterator_category_t<meta::_1>>>,
//         std::contiguous_iterator_tag
//       >
//     >,
//     T
//   > {};

// template<typename T>
// using common_iterator_category_t = common_iterator_category<T>::type;
//

template<typename T>
constexpr auto _iterator_category_tag_index(T element) {
  return hana::index_if(
    _iterator_categories,
    [&](const auto& x) { return hana::equal(x, element); }
  ).value();
}

/// An iterator that merges multiple other iterators.
template<typename T>
class zip_iterator {

  T iterators;

public:

  using value_type = decltype(
    +hana::unpack(
      hana::transform(
        to_hana_type_tuple<T>::value,
        [](auto t) {
          using It = typename decltype(t)::type;
          return hana::type_c<std::iter_value_t<It>>;
        }
      ),
      hana::template_<std::tuple>
    )
  )::type;

  /**
   * Because we always return a freshly constructed tuple (an rvalue), we cannot
   * return a reference to something that has no definitive memory location. 
   * Therefore, a reference is always equal to a plain old value.
   */
  using reference = value_type;

  using pointer = value_type*;

  using difference_type = std::ptrdiff_t;

  using iterator_category = decltype(
    +min_by(
      hana::transform(
        to_hana_type_tuple<T>::value,
        [](auto el) { return hana::type_c<iterator_category_t<typename decltype(+el)::type>>; }
      ),
      [](auto el) {
        return _iterator_category_tag_index(el);
      },
      hana::type_c<std::random_access_iterator_tag>
    )
  )::type;

  // using iterator_category = std::random_access_iterator_tag;
  // using iterator_category = common_iterator_category<T>;

  zip_iterator(T iterators):
    iterators(iterators) {}

  zip_iterator(const zip_iterator& other):
    iterators(other.iterators) {}

  zip_iterator(zip_iterator&& other):
    iterators(std::move(other.iterators)) {}

  zip_iterator& operator=(zip_iterator&& other) {
    iterators = std::move(other.iterators);
    return *this;
  }

  zip_iterator copy() {
    return { iterators };
  }

  bool operator==(const zip_iterator& other) const {
    return std::get<0>(iterators) == std::get<0>(other.iterators);
  }

  bool operator!=(const zip_iterator& other) const {
    return std::get<0>(iterators) != std::get<0>(other.iterators);
  }

  zip_iterator& operator++() {
    std::apply([](auto& ...args) { ((++args),...); }, iterators);
    return *this;
  }

  zip_iterator operator++(int) {
    return map(iterators, [&] (auto& iter) { return iter++; });
  }

  void operator--() {
    std::apply([&](auto& ...args) { ((--args),...); }, iterators);
  }

  zip_iterator operator+(std::ptrdiff_t offset) {
    return cmap(iterators, [&] (auto& iter) { return iter + offset; });
  }

  zip_iterator operator-(std::ptrdiff_t offset) {
    return cmap(iterators, [&] (auto& iter) { return iter - offset; });
  }

  zip_iterator& operator=(const zip_iterator& other) {
    iterators = other.iterators;
    return *this;
  }

  reference operator*() {
    return cmap(iterators, [] (const auto& iter) { return *iter; });
  }

};

template<std::input_iterator...Ts>
auto zip(Ts...args) {
  return zip_iterator<std::tuple<Ts...>>(std::tuple<Ts...>(std::forward<Ts>(args)...));
}

ZEN_NAMESPACE_END

namespace std {

  template <typename T>
  struct iterator_traits<zen::zip_iterator<T>>
  {
      using difference_type = ZEN_NAMESPACE::zip_iterator<T>::difference_type;
      using value_type = ZEN_NAMESPACE::zip_iterator<T>::value_type;
      using pointer = ZEN_NAMESPACE::zip_iterator<T>::pointer;
      using reference = ZEN_NAMESPACE::zip_iterator<T>::reference;
      using iterator_category = ZEN_NAMESPACE::zip_iterator<T>::iterator_category;
  };

}

#endif // #ifndef ZEN_ZIP_ITERATOR_HPP
