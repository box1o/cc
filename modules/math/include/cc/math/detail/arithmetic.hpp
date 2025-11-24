#pragma once

#include <cstdint>
#include <type_traits>

namespace cc{

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept floating_point= std::is_floating_point_v<T>;

template<typename T>
concept integral = std::is_integral_v<T>;


template<typename T>
concept signed_arithmetic = arithmetic<T>&& std::is_signed_v<T>;

template<typename T>
concept unsigned_arithmetic = arithmetic<T>&& std::is_unsigned_v<T>;


enum class layout : std::uint16_t{
    rowm,
    colm
};

}
