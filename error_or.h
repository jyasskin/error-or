#ifndef ERROR_VARIANT_H
#define ERROR_VARIANT_H

#include <system_error>
#include <type_traits>
#include <new>

namespace std {
namespace beta {

// Stores either an error_code or an instance of ValueType.  Users are
// expected to check for the presence of an error before retrieving
// the value, but if they forget, the value() method will throw
// system_error.
template<typename ValueType>
class error_or {
 public:
  /*implicit*/ error_or(error_code ec) noexcept : ec_(ec) {}
  /*implicit*/ error_or(ValueType value) noexcept(is_nothrow_move_constructible<ValueType>())
      : ec_() {
    new(&value_) ValueType(move(value));
  }

  error_or(const error_or& other) : ec_(other.ec_) {
    if (!ec_)
      new(&value_) ValueType(other.value());
  }
  error_or(error_or&& other) : ec_(other.ec_) {
    if (!ec_)
      new(&value_) ValueType(move(other).value());
  }
  ~error_or() {
    if (!ec_)
      value_.~ValueType();
  }

  error_or& operator=(error_or other) {
    swap(other);
    return *this;
  }

  void swap(error_or& other) {
    using std::swap;
    if (!*this && !other) {
      swap(ec_, other.ec_);
    } else if (bool(*this) && bool(other)) {
      swap(ec_, other.ec_);
      swap(value_, other.value_);
    } else if (bool(*this) && !other) {
      swap(ec_, other.ec_);
      new(&other.value_) ValueType(move(value_));
      value_.~ValueType();
    } else /* !*this && bool(other) */ {
      swap(ec_, other.ec_);
      new(&value_) ValueType(move(other.value_));
      other.value_.~ValueType();
    }
  }

  // True if the error_or contains a value.
  explicit operator bool() const noexcept {
    return !ec_;
  }

  error_code error() const noexcept {
    return ec_;
  }

  const ValueType& value() const & {
    if (ec_)
      throw system_error(ec_);
    return value_;
  }

  // Use as "std::move(the_error_or).value()".
  // Moving from error_or twice returns a moved-from ValueType; does
  // not throw an exception.
  ValueType value() && {
    if (ec_)
      throw system_error(ec_);
    return move(value_);
  }

 private:
  error_code ec_;
  union {
    ValueType value_;
  };
};

template<typename T>
void swap(error_or<T>& lhs, error_or<T>& rhs) {
  lhs.swap(rhs);
}

}
}

#endif  // ERROR_VARIANT_H
