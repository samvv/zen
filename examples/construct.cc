
#include <iostream>

#include "zen/alloc.hpp"
#include "zen/bump_ptr_pool.hpp"

struct A { };

struct B : public A {
  bool b;
  B(bool b): A(), b(b) {}
};

struct C : public A {
  int c;
  C(int c): A(), c(c) {}
};

int main() {
  zen::bump_ptr_pool p;
  C* ptr = zen::construct<C>(p, 2);
  if (!ptr) {
    std::cerr << "out of memory!\n";
    return 1;
  }
  std::cerr << "c = " << ptr->c << "\n";
}
