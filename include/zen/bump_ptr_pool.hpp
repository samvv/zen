/// @file
/// @brief Implementation of a bump-pointer allocator.
///
/// This header defines @ref bump_ptr_pool, a kind of allocator that moves a pointer
/// to the right each time a new allocation is done.
///
/// @include bump_ptr_construct.cc

#ifndef ZEN_BUMP_PTR_POOL_HPP
#define ZEN_BUMP_PTR_POOL_HPP

#include <cstddef>
#include <cstring>
#include <list>
#include <memory>
#include <new>

#include "zen/config.hpp"
#include "zen/math.hpp"
#include "zen/alloc.hpp"

#define ZEN_DEFAULT_POOL_CHUNK_SIZE 65536

#define ZEN_DEFAULT_GROWING_BUMP_PTR_POOL_MIN_SIZE 4084

ZEN_NAMESPACE_START

/// @brief A chunk of memory for storing variable-width objects which get destroyed all
/// at once.
///
/// The objects are stored in a continuous byte array with a pointer keeping
/// track of the free space at the end. The pointer is bumped with each
/// allocation, hence the name.
///
/// @see construct
/// @see DynamicAllocator
/// @see growing_bump_ptr_pool
class bump_ptr_pool {

  std::size_t _data_sz;

  std::byte* _data_start;
  std::byte* _data_end;
  std::byte* _prev_slot = nullptr;

  std::byte* get_slot_next_at(const std::byte* ptr) {
    std::byte* out;
    std::memcpy(&out, ptr, sizeof(void*));
    return out;
  }

  destroy_fn get_slot_destroy_at(const std::byte* ptr) {
    destroy_fn out;
    std::memcpy(&out, ptr + sizeof(void*), sizeof(destroy_fn));
    return out;
  }

  void* get_slot_data_at(std::byte* ptr) {
    return ptr + sizeof(void*) + sizeof(destroy_fn);
  }

  void set_slot_next_at(std::byte* ptr, void* next) {
    std::memcpy(ptr, &next, sizeof(void*));
  }

  void set_slot_destroy_at(std::byte* ptr, destroy_fn fn) {
    std::memcpy(ptr + sizeof(void*), &fn, sizeof(destroy_fn));
  }

  std::byte* first_slot() {
    return _data_end == _data_start ? nullptr : _data_start;
  }

public:

  bump_ptr_pool(std::size_t sz = ZEN_DEFAULT_POOL_CHUNK_SIZE) {
    _data_sz = sz;
    _data_start = new std::byte[_data_sz + sizeof(void*)];
    memset(_data_start, 0, _data_sz + sizeof(void*));
    _data_end = _data_start;
  }

  bool empty() const {
    return _data_start == _data_end;
  }

  std::size_t max_bytes_free() const {
    auto free = _data_sz - (_data_end - _data_start);
    auto header_size = sizeof(void*) + sizeof(destroy_fn);
    return free < header_size ? 0 : free - header_size;
  }

  std::size_t capacity() const {
    return _data_sz;
  }

  void* allocate(std::size_t sz, std::size_t alignment, destroy_fn destroy) {
    void* data = get_slot_data_at(_data_end);
    std::size_t free = max_bytes_free();
    if (!std::align(alignment, sz, data, free)) {
      return nullptr;
    }
    set_slot_destroy_at(_data_end, destroy);
    if (_prev_slot) {
      set_slot_next_at(_prev_slot, _data_end);
    }
    _prev_slot = _data_end;
    _data_end = static_cast<std::byte*>(data) + sz;
    return data;
  }

  ~bump_ptr_pool() {
    auto curr_slot = first_slot();
    while (curr_slot) {
      get_slot_destroy_at(curr_slot)(get_slot_data_at(reinterpret_cast<std::byte*>(curr_slot)));
      curr_slot = get_slot_next_at(curr_slot);
    }
    delete _data_start;
  }

  static std::size_t min_size_for(std::size_t sz) {
    return power_of_2_ceil(sz + sizeof(void*) + sizeof(destroy_fn));
  }

};

/// @brief A growing collection of memory chunks for storing variable-width objects
/// which get destroyed all at once.
///
/// This pool keeps allocating memory using a given pool until it is full, at
/// which point another pool will be created.
///
/// The pools managed by this pool grow exponentially. That is, when allocating
/// a new pool the size of the new pool will be at least double that of the old
/// pool.
///
/// @see construct
/// @see bump_ptr_pool
class growing_bump_ptr_pool {

  std::list<bump_ptr_pool*> chunks;

public:

  growing_bump_ptr_pool(std::size_t min_size = ZEN_DEFAULT_GROWING_BUMP_PTR_POOL_MIN_SIZE) {
    chunks.push_back(new bump_ptr_pool { power_of_2_ceil(min_size) });
  }

  growing_bump_ptr_pool(growing_bump_ptr_pool&& other):
    chunks(std::move(other.chunks)) {}

  void* allocate(std::size_t sz, std::size_t alignment, destroy_fn destroy) {
    auto last_chunk = chunks.back();
    void* ptr = last_chunk->allocate(sz, alignment, destroy);
    if (ZEN_LIKELY(ptr)) {
      return ptr;
    }
    auto next_chunk = new (std::nothrow) bump_ptr_pool {
      std::max(
        bump_ptr_pool::min_size_for(sz),
        next_power_of_2(last_chunk->capacity())
      )
    };
    if (ZEN_UNLIKELY(next_chunk == nullptr)) {
      return nullptr;
    }
    chunks.push_back(next_chunk);
    return next_chunk->allocate(sz, alignment, destroy);
  }

  ~growing_bump_ptr_pool() {
    for (auto chunk: chunks) {
      delete chunk;
    }
  }
};

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_BUMP_PTR_POOL_HPP
