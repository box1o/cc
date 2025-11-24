#pragma once

#include "../detail/arithmetic.hpp"

#include <numbers>
#include <limits>

namespace cc {

template <floating_point T>
inline constexpr T pi = std::numbers::pi_v<T>;

template <floating_point T>
inline constexpr T two_pi = std::numbers::pi_v<T> * T{2};

template<floating_point T>
inline constexpr T epsilon = std::numeric_limits<T>::epsilon();

template<floating_point T>
inline constexpr T infinity = std::numeric_limits<T>::infinity();

}
