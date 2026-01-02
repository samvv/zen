#ifndef ZEN_HANA_HPP
#define ZEN_HANA_HPP

#include "boost/hana.hpp"
#include "zen/config.hpp"

ZEN_NAMESPACE_START

namespace hana = boost::hana;

template<typename T, typename Fn, typename I>
constexpr auto min_by(T&& seq, Fn&& get, I&& init) {
  return hana::fold(
    seq,
    init,
    [&](auto a, auto b) {
      return hana::if_(
        hana::less(get(a), get(b)),
        a,
        b
      );
    }
  );
}

template<typename T>
struct to_hana_type_tuple;

template<typename ...Ts>
struct to_hana_type_tuple<std::tuple<Ts...>> {
  static constexpr auto value = hana::make_tuple(hana::type_c<Ts>...);
};

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_HANA_HPP
