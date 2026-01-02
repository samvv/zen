#ifndef ZEN_META_HPP
#define ZEN_META_HPP

#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <memory>

#include "zen/config.hpp"

ZEN_NAMESPACE_START

namespace meta {

  /// Make an template compatible with binders.
  ///
  /// @see _1
  /// @see _2
  /// @see _3
  /// @see _4
  /// @see _5
  /// @see _6
  /// @see _7
  /// @see _8
  /// @see _9
  // template<template<class... Params> class F, class... Args>
  // struct lift : F<Args...> {};

  struct _ {};
  struct _1 {};
  struct _2 {};
  struct _3 {};
  struct _4 {};
  struct _5 {};
  struct _6 {};
  struct _7 {};
  struct _8 {};
  struct _9 {};

  template<std::size_t I>
  using sz = std::integral_constant<std::size_t, I>;

  using u0 = std::integral_constant<unsigned, 0>;
  using u1 = std::integral_constant<unsigned, 1>;
  using u2 = std::integral_constant<unsigned, 2>;
  using u3 = std::integral_constant<unsigned, 3>;
  using u4 = std::integral_constant<unsigned, 4>;
  using u5 = std::integral_constant<unsigned, 5>;
  using u6 = std::integral_constant<unsigned, 8>;
  using u7 = std::integral_constant<unsigned, 7>;
  using u8 = std::integral_constant<unsigned, 8>;
  using u9 = std::integral_constant<unsigned, 9>;

  template<bool Value>
  using bool_ = std::bool_constant<Value>;
  using false_ = bool_<false>;
  using true_ = bool_<true>;

  template<typename T>
  struct thunk {
    using type = T;
  };

  template<typename T1, typename T2>
  struct less;

  template<typename I, I T1, I T2>
  struct less<std::integral_constant<I, T1>, std::integral_constant<I, T2>> {
    static constexpr const bool value = T1 < T2;
  };

  template<typename I, typename T>
  struct get;

  template<typename I, typename ...Ts>
  struct get<I, std::tuple<Ts...>> {
    using type = std::tuple_element_t<I::value, std::tuple<Ts...>>;
  };

  template<typename T1, typename T2>
  struct get<u0, std::pair<T1, T2>> {
    using type = T1;
  };

  template<typename T1, typename T2>
  struct get<u1, std::pair<T1, T2>> {
    using type = T2;
  };

  template<typename T>
  struct head;

  template<typename ...Ts>
  struct head<std::tuple<Ts...>> {
    using type = std::tuple_element_t<0, std::tuple<Ts...>>;
  };

  template<typename T1, typename T2>
  struct head<std::pair<T1, T2>> {
    using type = T1;
  };

  template<typename T>
  using head_t = typename head<T>::type;

  template<typename T>
  struct rest;

  template<typename T1, typename ...Ts>
  struct rest<std::tuple<T1, Ts...>> {
    using type = std::tuple<Ts...>;
  };

  template<typename T>
  using rest_t = typename rest<T>::type;

  template<typename T>
  struct is_null;

  template<>
  struct is_null<std::tuple<>> : std::true_type {};

  template<typename T1, typename ...Ts>
  struct is_null<std::tuple<T1, Ts...>> : std::false_type {};

  template<typename T>
  using is_null_v = is_null<T>::value;

  template<bool, typename Then, typename Else>
  struct if_;

  template<typename Then, typename Else>
  struct if_<true, Then, Else> : Then {};

  template<typename Then, typename Else>
  struct if_<false, Then, Else> : Else {};

  // template<bool Test, typename Then, typename Else>
  // using if_ = std::conditional<Test, Then, Else>;

  template<bool Test, typename Then, typename Else>
  using if_t = if_<Test, Then, Else>::type;

  template<typename FnT, typename ...Ts>
  struct apply {
    using type = FnT::template apply<Ts...>;
  };

  template<typename FnT, typename ...Ts>
  using apply_t = apply<FnT, Ts...>::type;

  template<typename T, typename F>
  struct map;

  template<typename ...Ts, typename F>
  struct map<std::tuple<Ts...>, F> {
    using type = std::tuple<apply_t<F, Ts>...>;
  };

  template<typename T, typename F>
  using map_t = typename map<T, F>::type;

  template<typename KeyT, typename T1, typename ...Ts>
  struct assoc_helper {
    // FIXME should probably inherit
    using type = if_t<
      std::is_same<typename T1::first_type, KeyT>::value,
      typename T1::second_type,
      typename assoc_helper<KeyT, Ts...>::type
    >;
  };

  template<typename KeyT, typename ListT>
  struct assoc;

  template<typename KeyT, typename ...Ts>
  struct assoc<KeyT, std::tuple<Ts...>> {
    using type = typename assoc_helper<KeyT, Ts...>::type;
  };

  template<typename Env, typename T>
  struct eval {
    using type = T;
  };

  // template<typename Env, template <typename...> class F, typename ...ArgTs>
  // struct eval<Env, lift<F, ArgTs...>> {
  //   using type = F<typename eval<Env, ArgTs>::type...>;
  // };

  template<typename Env>
  struct eval<Env, _1> {
    using type = std::tuple_element_t<0, Env>;
  };

  template<typename Env>
  struct eval<Env, _2> {
    using type = std::tuple_element_t<1, Env>;
  };

  template<typename Env>
  struct eval<Env, _3> {
    using type = std::tuple_element_t<2, Env>;
  };

  template<typename Env>
  struct eval<Env, _4> {
    using type = std::tuple_element_t<3, Env>;
  };

  template<typename Env>
  struct eval<Env, _5> {
    using type = std::tuple_element_t<4, Env>;
  };

  template<typename Env>
  struct eval<Env, _6> {
    using type = std::tuple_element_t<5, Env>;
  };

  template<typename Env>
  struct eval<Env, _7> {
    using type = std::tuple_element_t<6, Env>;
  };

  template<typename Env>
  struct eval<Env, _8> {
    using type = std::tuple_element_t<7, Env>;
  };

  template<typename Env>
  struct eval<Env, _9> {
    using type = std::tuple_element_t<8, Env>;
  };

  template<typename Env, typename T>
  using eval_t = typename eval<Env, T>::type;

  template<typename T>
  using eval0 = eval_t<std::tuple<>, T>;

  // template<typename Env, template <typename ...> class F, typename ...Ts>
  // struct eval<Env, F<Ts...>> {
  //   using type = typename F<eval_t<Env, Ts>...>::type;
  // };

  template<typename T>
  struct bind {
    template<typename ...ArgTs>
    using apply = eval_t<std::tuple<ArgTs...>, T>;
  };

  template<typename T, typename = void>
  struct get_element;

  template<std::input_iterator IterT>
  struct get_element<IterT> {
    using type = typename std::remove_reference_t<IterT>::value_type;
  };

  template<typename T>
  struct get_element<T*> {
    using type = T;
  };

  template<typename T>
  struct get_element<T*&> {
    using type = T;
  };

  template<typename T>
  struct get_element<T*&&> {
    using type = T;
  };

  template<typename T>
  using get_element_t = typename get_element<T>::type;

  /// Calculate the type that is used to represent the difference between two
  /// instances of the given type.
  template<typename T>
  struct difference {
    // TODO should require iterator concept
    using type = typename std::remove_reference_t<T>::difference_type;
  };

  template<typename T>
  struct difference<T*> {
    using type = std::ptrdiff_t;
  };

  template<typename T>
  using difference_t = typename difference<T>::type;

  template<std::size_t N, typename FnT, typename ...Ts>
  struct andmap_impl;

  template<typename FnT>
  struct andmap_impl<0, FnT> {
    static constexpr const bool value = true;
  };

  template<std::size_t N, typename FnT, typename T, typename ...Ts>
  struct andmap_impl<N, FnT, T, Ts...> {
    static constexpr const bool value = apply_t<FnT, T>::value && andmap_impl<N-1, FnT, Ts...>::value;
  };

  template<typename FnT, typename T>
  struct andmap;

  template<typename FnT, typename ...Ts>
  struct andmap<FnT, std::tuple<Ts...>> {
    static constexpr const bool value = andmap_impl<std::tuple_size_v<std::tuple<Ts...>>, FnT, Ts...>::value;
  };

  template<typename FnT, typename T>
  constexpr const bool andmap_v = andmap<FnT, T>::value;

  /// Reduce a sequence of elements to a specific value using a function.
  ///
  /// The function (also known as an accumulator) must accept exactly two
  /// parameters. The first parameter will receive the current element, which
  /// the second parameter will hold the resulting value as it has been reduced
  /// thus far.
  ///
  /// @param FnT The accumulator
  /// @param T The sequence of elements
  /// @param InitT The initial value that the accumulator must receive
  template <typename FnT, typename T, typename InitT>
  struct fold;

  template<typename FnT, typename InitT>
  struct fold<FnT, std::tuple<>, InitT> : InitT {};

  template<typename FnT, typename InitT, typename T1, typename ...Ts>
  struct fold<FnT, std::tuple<T1, Ts...>, InitT> : apply_t<FnT, T1, typename fold<FnT, std::tuple<Ts...>, InitT>::type> {};

  template<typename FnT, typename T, typename InitT>
  using fold_t = typename fold<FnT, T, InitT>::type;

  template<typename FnT, typename T>
  struct fold1 : fold<FnT, rest_t<T>, head_t<T>> {};

  template<typename FnT, typename T>
  using fold1_t = typename fold1<FnT, T>::type;

  template<typename I1, typename I2>
  struct add;

  template<typename I, I K, I L>
  struct add<std::integral_constant<I, K>, std::integral_constant<I, L>> {
    using type = std::integral_constant<I, K+L>;
  };

  template<typename I1, typename I2>
  using add_t = add<I1, I2>::type;

  template<typename I1, typename I2>
  struct sub;

  template<typename I, I K, I L>
  struct sub<std::integral_constant<I, K>, std::integral_constant<I, L>> {
    using type = std::integral_constant<I, K-L>;
  };

  template<typename I1, typename I2>
  using sub_t = sub<I1, I2>::type;

  template<typename FnT, typename ListT, typename InitT>
  struct min_by : fold<
    bind<
      if_t<
        apply_t<FnT, _1>::value < apply_t<FnT, _2>::value,
        _1,
        _2
      >
    >,
    ListT,
    InitT
  > {};

  template<typename FnT, typename ListT, typename InitT>
  using min_by_t = typename min_by<FnT, ListT, InitT>::type;

  template<typename Seq, std::size_t I>
  struct iter {};

  template<typename T>
  struct begin;

  template<typename ...Ts>
  struct begin<std::tuple<Ts...>> {
    using type = iter<std::tuple<Ts...>, 0>;
  };

  template<typename T>
  using begin_t = typename begin<T>::type;

  template<typename IterT>
  struct next;

  template<std::size_t I, typename ...Ts>
  struct next<iter<std::tuple<Ts...>, I>> {
    using type = iter<std::tuple<Ts...>, I+1>;
  };

  template<typename IterT>
  using next_t = typename next<IterT>::type;

  template<typename IterT>
  struct deref;

  template<std::size_t I, typename ...Ts>
  struct deref<iter<std::tuple<Ts...>, I>> {
    using type = std::tuple_element_t<I, std::tuple<Ts...>>;
  };

  template<typename IterT>
  using deref_t = typename deref<IterT>::type;

  /// Return the 0-based index of an element in the sequence.
  ///
  /// This function will error if the requested element could not be found.
  ///
  /// @param ListT The sequence of elements to search
  /// @param NeedleT The element to search for
  /// @param I An optional index from which to start the search
  template<typename StartT, typename NeedleT, std::size_t I = 0>
  struct index : if_<std::is_same<NeedleT, deref_t<StartT>>::value, sz<I>, typename index<next_t<StartT>, NeedleT, I+1>::type> {};

  // template<typename NeedleT, std::size_t I>
  // struct index<std::tuple<>, NeedleT, I> {
  //   static_assert(false && "element not found in std::tuple");
  // };

  // template<typename NeedleT, std::size_t I, typename T, typename ...Ts>
  // struct index<std::tuple<T, Ts...>, NeedleT, I> requires (std::is_same<NeedleT, T>::value) : sz<I> {};

  // template<typename NeedleT, std::size_t I, typename T, typename ...Ts>
  // struct index<std::tuple<T, Ts...>, NeedleT, I, std::enable_if_t<!std::is_same<NeedleT, T>::value>> : index<std::tuple<Ts...>, NeedleT, I + 1> {};

  template<typename StartT, typename NeedleT, std::size_t I = 0>
  using index_t = typename index<StartT, NeedleT, I>::type;

  template<typename T>
  struct is_pointer : std::false_type {};

  template<typename T>
  struct is_pointer<T*> : std::true_type {};

  template<typename T>
  struct is_pointer<std::shared_ptr<T>> : std::true_type {};

  template<typename T>
  struct is_pointer<std::unique_ptr<T>> : std::true_type {};

  template<typename T>
  static constexpr const bool is_pointer_v = is_pointer<T>::value;

  template<typename T>
  struct pointer_element;

  template<typename T>
  struct pointer_element<T*> {
    using type = T;
  };

  template<typename T>
  struct pointer_element<std::shared_ptr<T>> {
    using type = T;
  };

  template<typename T>
  struct pointer_element<std::unique_ptr<T>> {
    using type = T;
  };

  template<typename T>
  using pointer_element_t = typename pointer_element<T>::type;

  template<typename T, typename U>
  auto static_pointer_cast(U& ptr);

  // TODO In a later stage these could be autogenerated

  template<typename Env, typename T1, typename T2>
  struct eval<Env, less<T1, T2>> : less<eval_t<Env, T1>, eval_t<Env, T2>> {};

  template<typename Env, typename T1, typename T2>
  struct eval<Env, add<T1, T2>> : add<eval_t<Env, T1>, eval_t<Env, T2>> {};

  template<typename Env, typename T>
  struct eval<Env, head<T>> : head<eval_t<Env, T>> {};

  template<typename Env, typename T>
  struct eval<Env, rest<T>> : rest<eval_t<Env, T>> {};

  template<typename Env, typename T1, typename T2>
  struct eval<Env, sub<T1, T2>> : sub<eval_t<Env, T1>, eval_t<Env, T2>> {};

  template<typename Env, bool T1, typename T2, typename T3>
  struct eval<Env, if_<T1, T2, T3>> : if_<T1, eval_t<Env, T2>, eval_t<Env, T3>> {};

  template<typename Env, typename FnT, typename ListT, typename InitT>
  struct eval<Env, fold<FnT, ListT, InitT>> : fold<eval_t<Env, FnT>, eval_t<Env, ListT>, eval_t<Env, InitT>> {};

  template<typename Env, typename FnT, typename ListT>
  struct eval<Env, index<FnT, ListT>> : index<eval_t<Env, FnT>, eval_t<Env, ListT>> {};

  template<typename Env, typename FnT, typename ListT, typename InitT>
  struct eval<Env, min_by<FnT, ListT, InitT>> : min_by<eval_t<Env, FnT>, eval_t<Env, ListT>, eval_t<Env, InitT>> {};

  template<typename Env, typename T1, typename T2>
  struct eval<Env, std::is_same<T1, T2>> : std::is_same<eval_t<Env, T1>, eval_t<Env, T2>> {};

}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_META_HPP
