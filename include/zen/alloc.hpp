/// @file
/// @brief Generic tools and utilities for using and defining allocators.
///
/// Unless writing a custom allocator, the most essential declaration in this
/// header file is [zen::construct](@ref construct). It allows you to construct
/// any object inside the allocator of choice.
///
/// @include bump_ptr_construct.cc

#ifndef ZEN_ALLOC_HPP
#define ZEN_ALLOC_HPP

#include <concepts>
#include <new>
#include <utility>

#include "zen/config.hpp"

ZEN_NAMESPACE_START

/// The signature of a function that is called whenever an object inside an
/// allocator is going to be destroyed.
///
/// The function accepts a single `void*` parameter that contains a reference to
/// the memory of the object that is being destroyed.
///
/// ```
/// #include <iostream>
///
/// #include "zen/alloc.hpp"
///
/// struct Foo {
///
///   ~Foo() {
///     std::cerr << "Foo is being destroyed!\n";
///   }
///
/// }
///
/// int main() {
///   zen::bump_ptr_pool allocator;
///   auto ptr = allocator.allocate(
///     sizeof(Foo),
///     alignof(Foo),
///     [](void* ptr) { static_cast<Foo*>(ptr)->~Foo(); }
///   );
///   auto foo = std::launder(reinterpret_cast<Foo*>(ptr));
///   // Foo's destructor will run at the end of the program
/// }
/// ```
///
/// @see construct for automatically constructing objects using an allocator
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
/// @include construct.cc
///
/// @see bump_ptr_pool for a simple allocator for dynamic objects
/// @see growing_bump_ptr_pool for an allocator that grows in size when out of memory
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
  return std::launder(reinterpret_cast<R*>(ptr));
}

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_ALLOC_HPP
