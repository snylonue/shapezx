#include <concepts>

namespace shapezx::vec {

// clang-format off
template <typename T>
concept Arithmetic = requires(T x) {
  { x * x } -> std::same_as<T>;
  { x + x } -> std::same_as<T>;
  { x - x } -> std::same_as<T>;
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
  explicit Vec2(const T &a1, const T &a2) : data({a1, a2}) {}

  Vec2 operator+(const Vec2 &rhs) const {
    return {this->data[0] + rhs.data[0], this->data[1] + rhs.data[1]};
  }

  Vec2 operator-(const Vec2 &rhs) const {
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

  T operator*(const Vec2 &rhs) const {
    return this->data[0] * rhs.data[0] + this->data[1] * rhs.data[1];
  }
};
} // namespace shapezx::vec