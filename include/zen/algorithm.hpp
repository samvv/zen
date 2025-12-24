#ifndef ZEN_ALGORITHM_HPP
#define ZEN_ALGORITHM_HPP

#include "zen/meta.hpp"

ZEN_NAMESPACE_START

#define ZEN_CONSTEXPR constexpr

template<typename FnT, std::size_t ...J, typename ...Ts>
ZEN_CONSTEXPR auto map_tuple_impl(std::tuple<Ts...>& value, std::integer_sequence<std::size_t, J...>, FnT&& f) {
  return std::make_tuple(f(std::get<J>(value))...);
}

template<typename FnT, std::size_t ...J, typename ...Ts>
ZEN_CONSTEXPR auto map_const_tuple_impl(const std::tuple<Ts...>& value, std::integer_sequence<std::size_t, J...>, FnT&& f) {
  return std::make_tuple(f(std::get<J>(value))...);
}

template<typename FnT, typename ...Ts>
ZEN_CONSTEXPR auto cmap(const std::tuple<Ts...>& value, FnT op) {
  using seq = std::make_index_sequence<std::tuple_size_v<std::tuple<Ts...>>>;
  return map_const_tuple_impl(value, seq(), std::forward<FnT>(op));
}

template<typename FnT, typename ...Ts>
auto map(std::tuple<Ts...>& value, FnT op) {
  using seq = std::make_index_sequence<std::tuple_size_v<std::tuple<Ts...>>>;
  return map_tuple_impl(value, seq(), std::forward<FnT>(op));
}

template<typename T1, typename T2, typename FnT>
ZEN_CONSTEXPR auto cmap(const std::pair<T1, T2>& value, FnT fn) {
  return std::make_pair<T1, T2>(fn(value.first), fn(value.second));
}

template<typename T1, typename T2, typename FnT>
auto map(std::pair<T1, T2>& value, FnT fn) {
  return std::make_pair<T1, T2>(fn(value.first), fn(value.second));
}

template<typename T>
auto decrement(T& value) {
  --value;
}

template<typename T>
auto increment(T& value) {
  ++value;
}

template<typename T>
T prev_n(T value, meta::difference_t<T> n) {
  auto curr = value;
  for (std::ptrdiff_t i = 0; i < n; i++) {
    decrement(curr);
  }
  return curr;
}

template<typename T, typename = void>
struct next_n_impl {
  static T apply(T& value, meta::difference_t<T> n) {
    auto curr = value;
    for (std::ptrdiff_t i = 0; i < n; i++) {
      increment(curr);
    }
    return curr;
  }
};

template<typename T>
T next_n(T& value, meta::difference_t<T> n) {
  return next_n_impl<T>::apply(value, n);
}

template<class T, std::size_t N>
concept has_tuple_element =
  requires(T t) {
    typename std::tuple_element_t<N, std::remove_const_t<T>>;
    { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
  };

template<class T>
concept tuple_like = !std::is_reference_v<T> 
  && requires(T t) {
      typename std::tuple_size<T>::type;
      requires std::derived_from<
        std::tuple_size<T>,
        std::integral_constant<std::size_t, std::tuple_size_v<T>>
      >;
    }
  && []<std::size_t... N>(std::index_sequence<N...>) {
    return (has_tuple_element<T, N> && ...);
  }(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename F, typename Tuple, std::size_t... I>
auto tuple_map_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
  return std::make_tuple(f(std::get<I>(t))...);
}

template<typename RangeT, typename FnT, typename OutIterT>
void transform(const RangeT& range, FnT func, OutIterT iter) {
  for (auto& element: range) {
    iter = func(element);
  }
}

template<typename RangeT>
typename meta::get_element_t<RangeT> last(RangeT range) {
  auto prev = range.begin();
  auto curr = prev;
  auto end = range.end();
  for (;;) {
    curr++;
    if (curr == end) {
      break;
    }
    prev = curr;
  }
  return *prev;
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ALGORITHM_HPP
