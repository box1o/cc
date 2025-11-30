#pragma once

#include "fwd.hpp"
#include "vec2.hpp" // IWYU pragma: keep
#include "../detail/arithmetic.hpp"
#include "../common/functions.hpp"

#include <array>
#include <cassert>
#include <cstddef>

namespace cc {

template<arithmetic T>
class vec<3, T> {
public:
    static constexpr std::size_t size = 3;
    using value_type = T;

    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        std::array<T, 3> data_;
    };

    constexpr vec() noexcept : data_{T{}, T{}, T{}} {}
    explicit constexpr vec(T scalar) noexcept : data_{scalar, scalar, scalar} {}
    constexpr vec(T x_, T y_, T z_) noexcept : data_{x_, y_, z_} {}
    constexpr vec(const vec<2, T>& xy_, T z_) noexcept : data_{xy_.x, xy_.y, z_} {}

    template<arithmetic U>
    constexpr explicit vec(const vec<3, U>& other) noexcept
        : data_{static_cast<T>(other.x),
                static_cast<T>(other.y),
                static_cast<T>(other.z)} {}

    [[nodiscard]] constexpr T& operator[](std::size_t i) noexcept {
        assert(i < 3);
        return data_[i];
    }

    [[nodiscard]] constexpr const T& operator[](std::size_t i) const noexcept {
        assert(i < 3);
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
        return *this;
    }

    constexpr vec& operator-=(const vec& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr vec& operator*=(const vec& other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    constexpr vec& operator*=(T scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr vec& operator/=(const vec& other) noexcept {
        assert(other.x != T{} && other.y != T{} && other.z != T{});
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    constexpr vec& operator/=(T scalar) noexcept {
        assert(scalar != T{});
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    [[nodiscard]] friend constexpr vec operator+(const vec& a, const vec& b) noexcept {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    [[nodiscard]] friend constexpr vec operator-(const vec& a, const vec& b) noexcept {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    [[nodiscard]] friend constexpr vec operator*(const vec& a, const vec& b) noexcept {
        return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    [[nodiscard]] friend constexpr vec operator*(const vec& v, T scalar) noexcept {
        return {v.x * scalar, v.y * scalar, v.z * scalar};
    }

    [[nodiscard]] friend constexpr vec operator*(T scalar, const vec& v) noexcept {
        return v * scalar;
    }

    [[nodiscard]] friend constexpr vec operator/(const vec& a, const vec& b) noexcept {
        assert(b.x != T{} && b.y != T{} && b.z != T{});
        return {a.x / b.x, a.y / b.y, a.z / b.z};
    }

    [[nodiscard]] friend constexpr vec operator/(const vec& v, T scalar) noexcept {
        assert(scalar != T{});
        return {v.x / scalar, v.y / scalar, v.z / scalar};
    }

    [[nodiscard]] constexpr vec operator-() const noexcept {
        return {-x, -y, -z};
    }

    [[nodiscard]] friend constexpr bool operator==(const vec& a, const vec& b) noexcept {
        if constexpr (floating_point<T>) {
            return approx_equal(a.x, b.x) &&
                   approx_equal(a.y, b.y) &&
                   approx_equal(a.z, b.z);
        }
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    [[nodiscard]] constexpr bool operator!=(const vec& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr T dot(const vec& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr vec cross(const vec& other) const noexcept {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
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
};

} // namespace cc
