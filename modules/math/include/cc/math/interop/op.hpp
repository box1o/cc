#pragma once

#include "../mat/fwd.hpp"
#include "../vec/fwd.hpp"
#include "../detail/arithmetic.hpp"
#include <cstddef>

namespace cc {

template<std::size_t N, arithmetic T>
constexpr vec<N, T> operator*(const mat<N, N, T>& m, const vec<N, T>& v) noexcept {
    vec<N, T> result{};
    for (std::size_t i = 0; i < N; ++i) {
        T sum = T{};
        for (std::size_t j = 0; j < N; ++j) {
            sum += m(i, j) * v[j];
        }
        result[i] = sum;
    }
    return result;
}

template<std::size_t N, arithmetic T>
constexpr vec<N, T> operator*(const vec<N, T>& v, const mat<N, N, T>& m) noexcept {
    vec<N, T> result{};
    for (std::size_t j = 0; j < N; ++j) {
        T sum = T{};
        for (std::size_t i = 0; i < N; ++i) {
            sum += v[i] * m(i, j);
        }
        result[j] = sum;
    }
    return result;
}

template<arithmetic T>
constexpr vec<3, T> operator*(const mat<3, 3, T>& m, const vec<3, T>& v) noexcept {
    return vec<3, T>(
        m(0, 0) * v[0] + m(0, 1) * v[1] + m(0, 2) * v[2],
        m(1, 0) * v[0] + m(1, 1) * v[1] + m(1, 2) * v[2],
        m(2, 0) * v[0] + m(2, 1) * v[1] + m(2, 2) * v[2]
    );
}

template<arithmetic T>
constexpr vec<3, T> operator*(const vec<3, T>& v, const mat<3, 3, T>& m) noexcept {
    return vec<3, T>(
        v[0] * m(0, 0) + v[1] * m(1, 0) + v[2] * m(2, 0),
        v[0] * m(0, 1) + v[1] * m(1, 1) + v[2] * m(2, 1),
        v[0] * m(0, 2) + v[1] * m(1, 2) + v[2] * m(2, 2)
    );
}

template<arithmetic T>
constexpr vec<4, T> operator*(const mat<4, 4, T>& m, const vec<4, T>& v) noexcept {
    return vec<4, T>(
        m(0, 0) * v[0] + m(0, 1) * v[1] + m(0, 2) * v[2] + m(0, 3) * v[3],
        m(1, 0) * v[0] + m(1, 1) * v[1] + m(1, 2) * v[2] + m(1, 3) * v[3],
        m(2, 0) * v[0] + m(2, 1) * v[1] + m(2, 2) * v[2] + m(2, 3) * v[3],
        m(3, 0) * v[0] + m(3, 1) * v[1] + m(3, 2) * v[2] + m(3, 3) * v[3]
    );
}

template<arithmetic T>
constexpr vec<4, T> operator*(const vec<4, T>& v, const mat<4, 4, T>& m) noexcept {
    return vec<4, T>(
        v[0] * m(0, 0) + v[1] * m(1, 0) + v[2] * m(2, 0) + v[3] * m(3, 0),
        v[0] * m(0, 1) + v[1] * m(1, 1) + v[2] * m(2, 1) + v[3] * m(3, 1),
        v[0] * m(0, 2) + v[1] * m(1, 2) + v[2] * m(2, 2) + v[3] * m(3, 2),
        v[0] * m(0, 3) + v[1] * m(1, 3) + v[2] * m(2, 3) + v[3] * m(3, 3)
    );
}

template<std::size_t Rows, std::size_t Cols, arithmetic T>
constexpr vec<Rows, T> operator*(const mat<Rows, Cols, T>& m, const vec<Cols, T>& v) noexcept {
    vec<Rows, T> result{};
    for (std::size_t i = 0; i < Rows; ++i) {
        T sum = T{};
        for (std::size_t j = 0; j < Cols; ++j) {
            sum += m(i, j) * v[j];
        }
        result[i] = sum;
    }
    return result;
}

template<std::size_t Rows, std::size_t Cols, arithmetic T>
constexpr vec<Cols, T> operator*(const vec<Rows, T>& v, const mat<Rows, Cols, T>& m) noexcept {
    vec<Cols, T> result{};
    for (std::size_t j = 0; j < Cols; ++j) {
        T sum = T{};
        for (std::size_t i = 0; i < Rows; ++i) {
            sum += v[i] * m(i, j);
        }
        result[j] = sum;
    }
    return result;
}

}
