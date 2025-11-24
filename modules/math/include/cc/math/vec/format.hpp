#pragma once
#include <format>
#include "./base.hpp"

namespace std {
template <std::size_t N, cc::arithmetic T>
struct formatter<cc::vec<N, T>> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const cc::vec<N, T>& v, std::format_context& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "vec{}(", N);
        for (std::size_t i = 0; i < N; ++i) {
            out = std::format_to(out, "{}", v[i]);
            if (i + 1 < N) out = std::format_to(out, ", ");
        }
        return std::format_to(out, ")");
    }
};
}
