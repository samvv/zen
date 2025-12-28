#ifndef ZEN_CONFIG_HPP
#define ZEN_CONFIG_HPP

#include <stdio.h>
#include <cstdlib>

#ifndef ZEN_NAMESPACE
#define ZEN_NAMESPACE zen
#endif

#ifndef ZEN_NAMESPACE_START
#define ZEN_NAMESPACE_START namespace zen {
#endif

#ifndef ZEN_NAMESPACE_END
#define ZEN_NAMESPACE_END }
#endif

#define ZEN_CAT(arg) #arg
#define ZEN_CAT2(arg) ZEN_CAT(arg)

#define ZEN_ASSERT(test)                                                                      \
  if (!(test)) {                                                                                \
    ZEN_PANIC("The invariant " #test " failed to hold. See the stack trace for more details."); \
  }

#if ZEN_ENABLE_DEBUG_ASSERTIONS
#define ZEN_DEBUG_ASSERT(test) ZEN_ASSERT(test)
#else
#define ZEN_DEBUG_ASSERT(test)
#endif

#define ZEN_PANIC(message, ...)                                            \
  fprintf(stderr, __FILE__ ":" ZEN_CAT2(__LINE__) ": " message "\n" __VA_OPT__(,) __VA_ARGS__); \
  std::abort();

#define ZEN_UNREACHABLE \
  ZEN_PANIC("Code that should have been unreachable was executed. This is a bug.");

#define ZEN_AUTO_SIZE (-1)

#if __cplusplus >= 201703L
#define ZEN_NODISCARD [[nodiscard]]
#else
#define ZEN_NODISCARD
#endif

#if __cplusplus >= 201402L
#define ZEN_DEPRECATED [[deprecated]]
#else
#define ZEN_DEPRECATED
#endif

#if __has_builtin(__builtin_expect) || defined(__GNUC__)
#define ZEN_LIKELY(expr) __builtin_expect((bool)(expr), true)
#define ZEN_UNLIKELY(expr) __builtin_expect((bool)(expr), false)
#else
#define ZEN_LIKELY(expr) (expr)
#define ZEN_UNLIKELY(expr) (expr)
#endif

#define ZEN_NOEXCEPT noexcept

#endif // of #ifndef ZEN_CONFIG_HPP

