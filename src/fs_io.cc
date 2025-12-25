
#ifdef __unix__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#else
#error "unsupported platform"
#endif

#include <string.h>

#include <limits>

#include "zen/fs/io.hpp"

ZEN_NAMESPACE_START

#define ZEN_FILE_BUFFER_SIZE 4096

namespace fs {

static std::error_code wrap_system_error(int code) {
  return std::error_code { code, std::system_category() };
}
                                                                                          ;
either<std::error_code, bytestring> read_file(const path& filename) {

  auto fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    return left(wrap_system_error(errno));
  }

  struct stat s;

  if (fstat(fd, &s) == -1) {
    close(fd);
    return left(wrap_system_error(errno));
  }

  if (s.st_size >= std::numeric_limits<std::size_t>::max()) {
    close(fd);
    return left(wrap_system_error(EOVERFLOW));
  }

  char buffer[ZEN_FILE_BUFFER_SIZE];

  auto chars = (char*)malloc(s.st_size);
  if (chars == NULL) {
    close(fd);
    return left(wrap_system_error(ENOMEM));
  }

  auto ptr = chars;

  for (;;) {
    ssize_t count = read(fd, buffer, sizeof(buffer));
    if (count == -1) {
      close(fd);
      return left(wrap_system_error(errno));
    }
    if (count == 0) {
      break;
    }
    memcpy(ptr, buffer, count);
    ptr += count;
  }

  close(fd);

  return right(bytestring { chars, static_cast<unsigned long>(ptr - chars) });
}

}

ZEN_NAMESPACE_END
