/// \brief Backports from later standard libraries to the current one.
///
/// This header contains some definitions that we use throughout the Zen
/// libraries. These definitions are inspired from, if not straightout copied
/// from, newer C++ standard libraries that the C++ standard library we use does
/// not support.
#ifndef ZEN_COMPAT_HPP
#define ZEN_COMPAT_HPP

#include <iterator>

ZEN_NAMESPACE_START

/// Compute the const reference type of T.
///
/// This type is defined in the standard library as
/// [std::iter_const_reference_t][1] but we want to support down to C++ 20.
///
/// [1]: https://en.cppreference.com/w/cpp/iterator/iter_t.html
template<std::indirectly_readable T>
using iter_const_reference_t = std::common_reference_t<
  const std::iter_value_t<T>&&,
  std::iter_reference_t<T>
>;

template<typename T, typename ... U>
concept neither = (!std::same_as<T, U> && ...);

template<typename T>
concept strict_unsigned_integral = std::unsigned_integral<T> &&
  neither<T, bool, char, char8_t, char16_t, char32_t, wchar_t>;

/// Checks if x is an integral power of two.
constexpr bool has_single_bit(strict_unsigned_integral auto x) noexcept {
  return x && !(x & (x - 1));
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_COMPAT_HPP
