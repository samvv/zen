
#pragma once

#include <iterator>
#include <type_traits>

#include "zen/config.hpp"

ZEN_NAMESPACE_START

template<typename IterT, typename F>
class mapped_iterator {

  IterT iterator;
  F func;

  using traits = std::iterator_traits<IterT>;

public:

  // TODO mirror traits::iterator_category but without output_iterator
  using iterator_category = std::input_iterator_tag;

  using value_type = typename std::invoke_result<F, typename traits::value_type>::type;

  /// It does not make sense to reference a return type that should be owned, so
  /// that is why a reference will always be a plain value_type.
  using reference = value_type;

  using difference_type = traits::difference_type;

  using pointer = void;

  mapped_iterator(IterT iterator, F func):
    iterator(iterator), func(func) {}

  // TODO make this work with arbitrary F
  bool operator==(const mapped_iterator& other) const {
    return iterator == other.iterator;
  }

  // TODO make this work with arbitrary F
  bool operator!=(const mapped_iterator& other) const {
    return iterator != other.iterator;
  }

  mapped_iterator& operator++() requires(std::incrementable<IterT>) {
    ++iterator;
    return *this;
  }

  mapped_iterator& operator--() requires(std::bidirectional_iterator<IterT>) {
    --iterator;
    return *this;
  }

  mapped_iterator operator++(int) requires(std::incrementable<IterT>) {
    return mapped_iterator { iterator++, func };
  }

  mapped_iterator operator--(int) requires(std::bidirectional_iterator<IterT>) {
    return mapped_iterator { iterator--, func };
  }

  IterT& operator+(difference_type offset) requires(std::random_access_iterator<IterT>) {
    iterator += offset;
    return *this;
  }

  IterT& operator-(difference_type offset) requires(std::random_access_iterator<IterT>) {
    iterator -= offset;
    return *this;
  }

  reference operator*() const requires(std::indirectly_readable<IterT>) {
    return func(*iterator);
  }

  // IterT& operator=(value_type&& value) requires(std::indirectly_writable<IterT, value_type>) {
  //   iterator = value;
  //   return *this;
  // }

};

template<std::input_iterator IterT, typename F>
auto map(IterT iterator, F func) {
  return mapped_iterator { iterator, func };
}

ZEN_NAMESPACE_END

namespace std {

template <typename IterT, typename F>
struct iterator_traits<zen::mapped_iterator<IterT, F>>
{
    using difference_type = ZEN_NAMESPACE::mapped_iterator<IterT, F>::difference_type;
    using value_type = ZEN_NAMESPACE::mapped_iterator<IterT, F>::value_type;
    using pointer = ZEN_NAMESPACE::mapped_iterator<IterT, F>::pointer;
    using reference = ZEN_NAMESPACE::mapped_iterator<IterT, F>::reference;
    using iterator_category = ZEN_NAMESPACE::mapped_iterator<IterT, F>::iterator_category;
};

}

