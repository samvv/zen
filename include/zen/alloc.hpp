#ifndef ZEN_ALLOC_HPP
#define ZEN_ALLOC_HPP

#include <concepts>
#include <utility>

#include "zen/config.hpp"

ZEN_NAMESPACE_START

/// The signature of a function that is called whenever an object inside an
/// allocator is going to be destroyed.
///
/// The function accepts a single `void*` parameter that contains a reference to
/// the memory of the object that is being destroyed.
///
/// @see construct
using destroy_fn = void (*)(void *);

/// The concept of all allocators that can allocate dynamic objects.
///
/// This type of allocators must keep track of destructors because the type is
/// not known upfront.
///
/// @see construct
template<typename T>
concept DynamicAllocator = requires (
  T& t,
  std::size_t size,
  std::size_t alignment,
  destroy_fn destroy
) {
  { t.allocate(size, alignment, destroy) } -> std::same_as<void*>;
};

/// Construct an object inside the memory provided by the given allocator.
///
/// ```
/// #include <iostream>
///
/// #include "zen/alloc.hpp"
/// #include "zen/bump_ptr_pool.hpp"
///
/// int main() {
///   zen::bump_ptr_pool p;
///   Foo* ptr = zen::construct<Foo>(1, 2);
///   if (!ptr) {
///     std::cerr << "out of memory!\n";
///     return 1;
///   }
/// }
/// ```
template<typename R, DynamicAllocator Alloc, typename ...Ts>
R* construct(Alloc& allocator, Ts&& ...args) {
  auto ptr = allocator.allocate(
    sizeof(R),
    alignof(R),
    [](void* ptr) { static_cast<R*>(ptr)->~R(); }
  );
  if (ZEN_UNLIKELY(!ptr)) {
    return nullptr;
  }
  ::new (ptr) R (std::forward<Ts>(args)...);
  return reinterpret_cast<R*>(ptr);
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ALLOC_HPP
