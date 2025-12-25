#include <utility>
#include <iostream>
#include <unordered_map>

#include "zen/range.hpp"

int main() {
  std::unordered_multimap<int, int> map { { 1, 1 }, { 2, 2 }, { 2, 3} };
  for (auto [k, v]: zen::make_iterator_range(std::as_const(map).equal_range(2))) {
    std::cerr << k << " = " << v << "\n";
  }
}
