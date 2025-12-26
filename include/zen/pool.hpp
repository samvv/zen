#ifndef ZEN_POOL_HPP
#define ZEN_POOL_HPP

#include <array>
#include <istream>

#define ZEN_DEFAULT_POOL_CHUNK_SIZE 2048

ZEN_NAMESPACE_START

/// A pool of memory for storing fixed-width objects.
///
/// By default the pool will not allocate any memory.
template<typename T, std::size_t ChunkSize = ZEN_DEFAULT_POOL_CHUNK_SIZE>
class pool {

  union slot {
    alignas(T) std::array<std::byte, sizeof(T)> data;
    slot* next;
  };

  struct chunk {
    chunk() {
      for (std::size_t i = 0; i < ChunkSize-1; ++i) {
        elements[i].next = &elements[i+1];
      }
      elements[ChunkSize-1].next = nullptr;
    }
    slot elements[ChunkSize];
    slot* free_list = elements;
    chunk* next = nullptr;
  };

  unsigned chunk_count = 0;

  chunk* first = nullptr;
  chunk* last = nullptr;

public:

  pool() {}

  pool(pool&& other):
    first(std::move(other.first)),
    last(std::move(other.last)),
    chunk_count(std::move(chunk_count)) {
      other.chunk_count = 0;
      other.first = nullptr;
      other.last = nullptr;
    }

  template<typename ...Ts>
  T* construct(Ts&& ...args) {

    if (first == nullptr) {
      first = new chunk;
      last = first;
    }

    if (!last->free_list) {
      auto next = new chunk;
      last->next = next;
      chunk_count++;
      last = next;
    }

    auto this_slot = last->free_list;
    last->free_list = this_slot->next;
    ::new (&this_slot->data) T (std::forward<Ts>(args)...);
    return reinterpret_cast<T*>(this_slot);
  }

  ~pool() {
    auto curr = first;
    while (curr) {
      auto keep = curr->next;
      delete curr;
      curr = keep;
    }
    // chunk_count = 0;
  }

};

ZEN_NAMESPACE_END

#endif // of #ifndef ZEN_POOL_HPP
