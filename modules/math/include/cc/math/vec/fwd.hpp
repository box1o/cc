#pragma once
#include "../detail/arithmetic.hpp"
#include <cstddef>


namespace cc {

//NOTE: Forward declaration
template<std::size_t N , arithmetic T >
requires(N >= 2)
class vec;

}
