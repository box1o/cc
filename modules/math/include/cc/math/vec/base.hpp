#pragma once

#include "../detail/arithmetic.hpp"
#include "../common/functions.hpp"
#include <array>
#include <cassert>
#include <cstddef>


namespace cc {

template<std::size_t N , arithmetic T >
requires(N >= 2)
class vec{
public:
    static constexpr std::size_t size = N;
    using value_type = T;

    constexpr vec() = default;

    explicit constexpr vec(T scalar) noexcept{ 
        data_.fill(scalar);
    }

    template<arithmetic... Args>
    requires(sizeof...(Args) == N)
    constexpr vec(Args... args) noexcept
    : data_{static_cast<T>(args)...} {}

    template<arithmetic U>
    constexpr vec(const vec<N, U>& other) noexcept{
        for (std::size_t i = 0 ; i < N ; i++){
            data_[i] = static_cast<T>(other[i]);
        }
    }

    constexpr T& operator[](std::size_t i) noexcept {
        assert(i < N);
        return data_[i];
    }

    constexpr const T& operator[](std::size_t i) const noexcept {
        assert(i < N);
        return data_[i];
    }

    constexpr T* data() noexcept { return data_.data();}
    constexpr const T* data() const noexcept { return data_.data();}

    constexpr vec& operator+=(const vec& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data_[i] += other.data_[i];
        }
        return *this;
    }

    constexpr vec& operator-=(const vec& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data_[i] -= other.data_[i];
        }
        return *this;
    }

    constexpr vec& operator*=(const vec& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data_[i] *= other.data_[i];
        }
        return *this;
    }

    constexpr vec& operator*=(T scalar) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data_[i] *= scalar;
        }
        return *this;
    }

    constexpr vec& operator/=(const vec& other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            assert(other.data_[i] != T{});
            data_[i] /= other.data_[i];
        }
        return *this;
    }

    constexpr vec& operator/=(T scalar) noexcept {
        assert(scalar != T{});
        for (std::size_t i = 0; i < N; ++i) {
            data_[i] /= scalar;
        }
        return *this;
    }

    friend constexpr vec operator+(const vec& a, const vec& b) noexcept {
        vec result = a;
        result += b;
        return result;
    }

    friend constexpr vec operator-(const vec& a, const vec& b) noexcept {
        vec result = a;
        result -= b;
        return result;
    }

    friend constexpr vec operator*(const vec& a, const vec& b) noexcept {
        vec result = a;
        result *= b;
        return result;
    }

    friend constexpr vec operator*(const vec& v, T scalar) noexcept {
        vec result = v;
        result *= scalar;
        return result;
    }

    friend constexpr vec operator*(T scalar, const vec& v) noexcept {
        return v * scalar;
    }

    friend constexpr vec operator/(const vec& a, const vec& b) noexcept {
        vec result = a;
        result /= b;
        return result;
    }

    friend constexpr vec operator/(const vec& v, T scalar) noexcept {
        vec result = v;
        result /= scalar;
        return result;
    }

    constexpr vec operator-() const noexcept {
        vec result;
        for (std::size_t i = 0; i < N; ++i) {
            result.data_[i] = -data_[i];
        }
        return result;
    }

    friend constexpr bool operator==(const vec& a, const vec& b) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            if constexpr (floating_point<T>) {
                if (!approx_equal(a.data_[i], b.data_[i])) return false;
            } else {
                if (a.data_[i] != b.data_[i]) return false;
            }
        }
        return true;
    }

    constexpr T dot(const vec& other) const noexcept {
        T result = T{};
        for (std::size_t i = 0; i < N; ++i) {
            result += data_[i] * other.data_[i];
        }
        return result;
    }

    constexpr T len_sq() const noexcept {
        return dot(*this);
    }

    T len() const noexcept {
        return cc::sqrt(len_sq());
    }

    vec norm() const noexcept {
        T l = len();
        return l > T{} ? *this / l : vec{};
    }

private:
    std::array<T, N> data_{};
};

}
