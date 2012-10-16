#include "error_or.h"

#include <assert.h>
#include <stdlib.h>
#include <string>

int main() {
  std::error_code ec(3, std::system_category());
  std::beta::error_or<std::string> r = ec;
  assert(r.error() == ec);
  assert(!r);
  try {
    r.value();
    abort();
  } catch(const std::system_error& err) {
    assert(err.code() == ec);
  }

  r = std::string("Hello World");
  assert(r);
  assert(r.value() == "Hello World");
}
