#include "error_or.h"

#include <assert.h>
#include <stdlib.h>

int main() {
  std::error_code ec(3, std::system_category());
  std::beta::error_or<int> r = ec;
  assert(r.error() == ec);
  assert(!r);
  try {
    r.value();
    abort();
  } catch(const std::system_error& err) {
    assert(err.code() == ec);
  }
}
