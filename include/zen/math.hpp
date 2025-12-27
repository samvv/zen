#ifndef ZEN_MATH_HPP
#define ZEN_MATH_HPP

#include <bit>
#include <cstdint>

ZEN_NAMESPACE_START

// Some definitions taken from the LLVM project

/// Return true if the argument is a power of two > 0.
/// Ex. isPowerOf2_32(0x00100000U) == true (32 bit edition.)
constexpr bool is_power_of_2_32(uint32_t Value) {
  return std::has_single_bit(Value);
}

/// Return true if the argument is a power of two > 0 (64 bit edition.)
constexpr bool is_power_of_2_64(uint64_t Value) {
  return std::has_single_bit(Value);
}

/// Return the floor log base 2 of the specified value, -1 if the value is zero.
/// (64 bit edition.)
inline unsigned log2_64(uint64_t Value) {
  return 63 - std::countl_zero(Value);
}

/// Return the ceil log base 2 of the specified value, 64 if the value is zero.
/// (64 bit edition.)
inline unsigned log2_64_ceil(uint64_t Value) {
  return 64 - std::countl_zero(Value - 1);
}

/// Returns the next power of two (in 64-bits) that is strictly greater than A.
/// Returns zero on overflow.
constexpr uint64_t next_power_of_2(uint64_t A) {
  A |= (A >> 1);
  A |= (A >> 2);
  A |= (A >> 4);
  A |= (A >> 8);
  A |= (A >> 16);
  A |= (A >> 32);
  return A + 1;
}

/// Returns the power of two which is greater than or equal to the given value.
/// Essentially, it is a ceil operation across the domain of powers of two.
inline uint64_t power_of_2_ceil(uint64_t A) {
  if (!A || A > UINT64_MAX / 2)
    return 0;
  return UINT64_C(1) << log2_64_ceil(A);
}

/// Fast integral exponentiation.
///
/// The type of the first argument will be the type returned by the function.
template<std::integral A, std::unsigned_integral B>
inline A ipow(A x, B p) {
  if (p == 0) return 1;
  if (p == 1) return x;
  int tmp = ipow(x, p / 2);
  if (p % 2 == 0) return tmp * tmp;
  else return x * tmp * tmp;
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_MATH_HPP
