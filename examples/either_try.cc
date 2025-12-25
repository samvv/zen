#include <vector>
#include <iostream>

#include "zen/either.hpp"

zen::either<std::string, char32_t>
decode_utf8_char(const unsigned char *& s) {
  char32_t c;
  if (s[0] < 0x80) {
    c = s[0];
    s += 1;
  } else if ((s[0] & 0xe0) == 0xc0) {
      c = ((long)(s[0] & 0x1f) <<  6) |
          ((long)(s[1] & 0x3f) <<  0);
      s += 2;
  } else if ((s[0] & 0xf0) == 0xe0) {
      c = ((long)(s[0] & 0x0f) << 12) |
          ((long)(s[1] & 0x3f) <<  6) |
          ((long)(s[2] & 0x3f) <<  0);
      s += 3;
  } else if ((s[0] & 0xf8) == 0xf0 && (s[0] <= 0xf4)) {
      c = ((long)(s[0] & 0x07) << 18) |
          ((long)(s[1] & 0x3f) << 12) |
          ((long)(s[2] & 0x3f) <<  6) |
          ((long)(s[3] & 0x3f) <<  0);
      s += 4;
  } else {
    s += 1; // skip this byte
    return zen::left("invalid UTF-8 byte sequence");
  }
  if (c >= 0xd800 && c <= 0xdfff) {
    return zen::left("invalid surrogate half");
  }
  return zen::right(c);
}

zen::either<std::string, std::vector<char32_t>> decode_utf8_string(const std::string_view& str) {
  std::vector<char32_t> out;
  const unsigned char* iter = reinterpret_cast<const unsigned char*>(str.data());
  const unsigned char* end = iter + str.size();
  for (; iter != end;)  {
    auto result = decode_utf8_char(iter);
    ZEN_TRY(result);
    out.push_back(*result); // may now dereference the result
  }
  return zen::right(std::move(out));
}

int main(int argc, const char* argv[]) {
  ZEN_ASSERT(argc >= 2);
  for (auto ch: decode_utf8_string(argv[1]).unwrap()) {
    std::cout << (int32_t)ch << " ";
  }
  std::cout << '\n';
  return 0;
}
