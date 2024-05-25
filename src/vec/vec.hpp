#ifndef SHAPEZX_VEC_VEC_HPP
#define SHAPEZX_VEC_VEC_HPP

#include <concepts>
#include <cstddef>

namespace shapezx::vec {

// clang-format off
template <typename T, typename U = T>
concept Arithmetic = requires(T x, U y) {
  { x * y } -> std::same_as<T>;
  { x + y } -> std::same_as<T>;
  { x - y } -> std::same_as<T>;
};

template <typename K, typename T>
concept Scalar = Arithmetic<K> && requires(K c, T x) {
  { x * c } -> std::same_as<T>;
  { x / c } -> std::same_as<T>;
};
// clang-format on

template <Arithmetic T> struct Vec2 {
  T data[2];

  Vec2() = delete;
  explicit Vec2(const T a1, const T a2) : data({a1, a2}) {}

  template<typename Self>
  auto&& operator[](this Self&& self, size_t i) {
    return self.data[i];
  }

  template<typename U = T>
  Vec2<T> operator+(const Vec2<U> &rhs) const requires Arithmetic<T, U> {
    return {this->data[0] + rhs.data[0], this->data[1] + rhs.data[1]};
  }

  template<typename U = T>
  Vec2<T> operator-(const Vec2<U> &rhs) const requires Arithmetic<T, U> {
    return {this->data[0] - rhs.data[0], this->data[1] - rhs.data[1]};
  }

  template <typename K = T>
  Vec2 operator*(const K &rhs) const
    requires Scalar<K, T>
  {
    return {this->data[0] * rhs, this->data[1] * rhs};
  }

  template <typename K = T>
  Vec2 operator/(const K &rhs) const
    requires Scalar<K, T>
  {
    return {this->data[0] / rhs, this->data[1] / rhs};
  }

  template<typename U = T>
  T operator*(const Vec2 &rhs) const requires Arithmetic<T, U> {
    return this->data[0] * rhs.data[0] + this->data[1] * rhs.data[1];
  }
};
} // namespace shapezx::vec

#endif