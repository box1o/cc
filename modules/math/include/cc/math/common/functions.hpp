#pragma once

#include "../detail/arithmetic.hpp"
#include "./constants.hpp"
#include <cmath>


namespace cc {

template <floating_point T>
constexpr T rad(T degrees) noexcept {
    return degrees * (pi<T>) / T{180};
}

template <floating_point T>
constexpr T degrees(T rad) noexcept {
    return rad * (T{180} / pi<T>);
}

template <arithmetic T>
constexpr T lerp(T a, T b, T t) noexcept {
    return a + t * (b - a);
}

template<arithmetic T>
constexpr T sign(T value) noexcept {
    return value < T{0} ? T{-1} : (value > T{0} ? T{1} : T{0});
}

template<arithmetic T>
constexpr T abs(T value) noexcept {
    return value < T{0} ? -value : value;
}

template<arithmetic T>
constexpr T min(T a, T b) noexcept {
    return a < b ? a : b;
}

template<arithmetic T>
constexpr T max(T a, T b) noexcept {
    return a > b ? a : b;
}

template<arithmetic T>
constexpr T clamp(T value, T lo, T hi) noexcept {
    return min(max(value, lo), hi);
}

template<arithmetic T>
T sqrt(T value) noexcept {
    return std::sqrt(value);
}

template<floating_point T>
constexpr bool approx_equal(T a, T b, T tolerance = epsilon<T>) noexcept {
    return abs(a - b) <= tolerance;
}

}
