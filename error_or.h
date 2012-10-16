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
    new(&space_) ValueType(move(value));
  }

  error_or(const error_or& other) : ec_(other.ec_) {
    if (!ec_)
      new(&space_) ValueType(other.value());
  }
  error_or(error_or&& other) : ec_(other.ec_) {
    if (!ec_)
      new(&space_) ValueType(move(other).value());
  }
  ~error_or() {
    if (!ec_)
      unchecked_value().~ValueType();
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
      swap(unchecked_value(), other.unchecked_value());
    } else {
      std::swap(*this, other);
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
    return unchecked_value();
  }

  // Use as "std::move(the_error_or).value()".
  // Moving from error_or twice returns a moved-from ValueType; does
  // not throw an exception.
  ValueType value() && {
    if (ec_)
      throw system_error(ec_);
    return move(unchecked_value());
  }

 private:
  ValueType& unchecked_value() {
    return reinterpret_cast<ValueType&>(space_);
  }

  const ValueType& unchecked_value() const {
    return reinterpret_cast<const ValueType&>(space_);
  }

  error_code ec_;
  typename aligned_storage<sizeof(ValueType), alignof(ValueType)>::type space_;
};

template<typename T>
void swap(error_or<T>& lhs, error_or<T>& rhs) {
  lhs.swap(rhs);
}

}
}

#endif  // ERROR_VARIANT_H
