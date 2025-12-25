#include <utility>
#include <iostream>
#include <unordered_map>

#include "zen/range.hpp"

int main() {
  std::unordered_map<int, int> map { { 1, 1 }, { 2, 2 } };
  for (auto [k, v]: zen::make_iterator_range(std::as_const(map).equal_range(2))) {
    std::cerr << k << " = " << v << "\n";
  }
}
