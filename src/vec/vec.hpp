#ifndef SHAPEZX_VEC_VEC_HPP
#define SHAPEZX_VEC_VEC_HPP

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <nlohmann/json_fwd.hpp>
#include <type_traits>

namespace shapezx::vec {

using nlohmann::json;

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

template <Arithmetic T = std::size_t> struct Vec2 {
  std::array<T, 2> data{0, 0};

  Vec2() = default;
  Vec2(const T a1, const T a2) : data({a1, a2}) {}

  auto &&operator[](this auto &&self, std::integral auto i) {
    return self.data.at(i);
  }

  template <typename U = T>
  Vec2<T> operator+(const Vec2<U> &rhs) const
    requires Arithmetic<T, U>
  {
    return {this->data[0] + rhs.data[0], this->data[1] + rhs.data[1]};
  }

  Vec2<T> &operator+=(const Vec2<T> &rhs) {
    *this = *this + rhs;
    return *this;
  };

  template <typename U = T>
  Vec2<T> operator-(const Vec2<U> &rhs) const
    requires Arithmetic<T, U>
  {
    return {this->data[0] - rhs.data[0], this->data[1] - rhs.data[1]};
  }

  Vec2<T> &operator-=(const Vec2<T> &rhs) {
    *this = *this - rhs;
    return *this;
  };

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

  template <typename U = T>
  T operator*(const Vec2 &rhs) const
    requires Arithmetic<T, U>
  {
    return this->data[0] * rhs.data[0] + this->data[1] * rhs.data[1];
  }

  bool operator==(const Vec2 &rhs) const { return this->data == rhs.data; }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vec2<size_t>, data);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vec2<std::make_signed_t<size_t>>, data);

} // namespace shapezx::vec

namespace std {
template <typename T> struct std::formatter<shapezx::vec::Vec2<T>> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const shapezx::vec::Vec2<T> &v, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "({}, {})", v[0], v[1]);
  }
};

} // namespace std

#endif