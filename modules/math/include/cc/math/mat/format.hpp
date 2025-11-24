#pragma once
#include <cstddef>
#include <format>
#include "./base.hpp"

namespace std {
template <std::size_t Rows, std::size_t Cols, cc::arithmetic T>
struct formatter<cc::mat<Rows, Cols, T>> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const cc::mat<Rows, Cols, T>& m, std::format_context& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "mat{}x{}(\n", Rows, Cols);
        for (std::size_t i = 0; i < Rows; ++i) {
            out = std::format_to(out, "  [");
            for (std::size_t j = 0; j < Cols; ++j) {
                out = std::format_to(out, "{}", m(i, j));
                if (j + 1 < Cols) out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "]");
            if (i + 1 < Rows) out = std::format_to(out, ",");
            out = std::format_to(out, "\n");
        }
        return std::format_to(out, ")");
    }
};
} // namespace std
