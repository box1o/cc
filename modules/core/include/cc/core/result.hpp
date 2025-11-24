#pragma once
#include "error.hpp"
#include <expected>
#include <utility>

namespace cc {

template<typename T>
using result = std::expected<T, error>;

template<typename T>
constexpr auto ok(T&& value) noexcept -> result<std::decay_t<T>> {
    return result<std::decay_t<T>>(std::forward<T>(value));
}

constexpr auto ok() noexcept -> result<void> {
    return result<void>();
}

inline auto err(error_code code, std::string_view msg,
                std::source_location loc = std::source_location::current()) noexcept 
    -> std::unexpected<error> {
    return std::unexpected<error>(error(code, msg, loc));
}

inline auto err(error_code code,
                std::source_location loc = std::source_location::current()) noexcept
    -> std::unexpected<error> {
    return std::unexpected<error>(error(code, "", loc));
}

inline auto err(const error& e) noexcept -> std::unexpected<error> {
    return std::unexpected<error>(e);
}

} // namespace cc
