#include <iostream>

#include "zen/alloc.hpp"
#include "zen/bump_ptr_pool.hpp"

int main() {
  zen::bump_ptr_pool p;
  int* one_in_pool = zen::construct<int>(p, 42);
  std::cerr << "42 == " << *one_in_pool << "\n";
}
