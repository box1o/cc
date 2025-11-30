#pragma once

#include "fwd.hpp"
#include "vec2.hpp" // IWYU pragma: keep
#include "vec3.hpp" // IWYU pragma: keep
#include "../detail/arithmetic.hpp"
#include "../common/functions.hpp"

#include <array>
#include <cassert>
#include <cstddef>

namespace cc {

template<arithmetic T>
class vec<4, T> {
public:
    static constexpr std::size_t size = 4;
    using value_type = T;

    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        std::array<T, 4> data_;
    };

    constexpr vec() noexcept : data_{T{}, T{}, T{}, T{}} {}
    explicit constexpr vec(T scalar) noexcept : data_{scalar, scalar, scalar, scalar} {}
    constexpr vec(T x_, T y_, T z_, T w_) noexcept : data_{x_, y_, z_, w_} {}
    constexpr vec(const vec<2, T>& xy_, T z_, T w_) noexcept : data_{xy_.x, xy_.y, z_, w_} {}
    constexpr vec(const vec<3, T>& xyz_, T w_) noexcept : data_{xyz_.x, xyz_.y, xyz_.z, w_} {}

    template<arithmetic U>
    constexpr explicit vec(const vec<4, U>& other) noexcept
        : data_{static_cast<T>(other.x),
                static_cast<T>(other.y),
                static_cast<T>(other.z),
                static_cast<T>(other.w)} {}

    [[nodiscard]] constexpr T& operator[](std::size_t i) noexcept {
        assert(i < 4);
        return data_[i];
    }

    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept {
        assert(i < 4);
        return data_[i];
    }

    [[nodiscard]] constexpr T* data() noexcept {
        return data_.data();
    }

    [[nodiscard]] constexpr const T* data() const noexcept {
        return data_.data();
    }

    constexpr vec& operator+=(const vec& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    constexpr vec& operator-=(const vec& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    constexpr vec& operator*=(const vec& other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    constexpr vec& operator*=(T scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr vec& operator/=(const vec& other) noexcept {
        assert(other.x != T{} && other.y != T{} && other.z != T{} && other.w != T{});
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    constexpr vec& operator/=(T scalar) noexcept {
        assert(scalar != T{});
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    [[nodiscard]] friend constexpr vec operator+(const vec& a, const vec& b) noexcept {
        return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    }

    [[nodiscard]] friend constexpr vec operator-(const vec& a, const vec& b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    }

    [[nodiscard]] friend constexpr vec operator*(const vec& a, const vec& b) noexcept {
        return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    }

    [[nodiscard]] friend constexpr vec operator*(const vec& v, T scalar) noexcept {
        return {v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar};
    }

    [[nodiscard]] friend constexpr vec operator*(T scalar, const vec& v) noexcept {
        return v * scalar;
    }

    [[nodiscard]] friend constexpr vec operator/(const vec& a, const vec& b) noexcept {
        assert(b.x != T{} && b.y != T{} && b.z != T{} && b.w != T{});
        return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    }

    [[nodiscard]] friend constexpr vec operator/(const vec& v, T scalar) noexcept {
        assert(scalar != T{});
        return {v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar};
    }

    [[nodiscard]] constexpr vec operator-() const noexcept {
        return {-x, -y, -z, -w};
    }

    [[nodiscard]] friend constexpr bool operator==(const vec& a, const vec& b) noexcept {
        if constexpr (floating_point<T>) {
            return approx_equal(a.x, b.x) &&
                   approx_equal(a.y, b.y) &&
                   approx_equal(a.z, b.z) &&
                   approx_equal(a.w, b.w);
        }
        return a.x == b.x &&
               a.y == b.y &&
               a.z == b.z &&
               a.w == b.w;
    }

    [[nodiscard]] constexpr bool operator!=(const vec& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr T dot(const vec& other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    [[nodiscard]] constexpr T length_squared() const noexcept {
        return dot(*this);
    }

    [[nodiscard]] T length() const noexcept {
        return cc::sqrt(length_squared());
    }

    [[nodiscard]] vec normalized() const noexcept {
        T l = length();
        if (l == T{}) {
            return vec{};
        }
        return *this / l;
    }

    constexpr vec& normalize() noexcept {
        T l = length();
        if (l != T{}) {
            *this /= l;
        }
        return *this;
    }

    [[nodiscard]] constexpr vec<2, T> xy() const noexcept { return {x, y}; }
    [[nodiscard]] constexpr vec<2, T> xz() const noexcept { return {x, z}; }
    [[nodiscard]] constexpr vec<2, T> yz() const noexcept { return {y, z}; }
    [[nodiscard]] constexpr vec<3, T> xyz() const noexcept { return {x, y, z}; }
};

} // namespace cc
