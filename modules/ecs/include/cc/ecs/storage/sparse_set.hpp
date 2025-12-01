#pragma once

#include <cc/core/types.hpp>
#include <vector>
#include <cassert>

namespace cc::ecs {

//NOTE: Sparse-set for entity indices.
class SparseSet {
public:
    using Index = cc::u32;

    SparseSet() = default;

    [[nodiscard]] Index Size() const noexcept {
        return static_cast<Index>(dense_.size());
    }

    [[nodiscard]] bool Empty() const noexcept {
        return dense_.empty();
    }

    [[nodiscard]] const std::vector<Index>& Dense() const noexcept {
        return dense_;
    }

    [[nodiscard]] bool Contains(Index index) const noexcept {
        if (index >= sparse_.size()) {
            return false;
        }
        const Index pos = sparse_[index];
        return pos != Invalid && pos < dense_.size() && dense_[pos] == index;
    }

    Index Insert(Index index) {
        EnsureCapacity(index);
        if (Contains(index)) {
            return sparse_[index];
        }

        const Index pos = static_cast<Index>(dense_.size());
        dense_.push_back(index);
        sparse_[index] = pos;
        return pos;
    }

    void Erase(Index index) {
        if (!Contains(index)) {
            return;
        }

        const Index pos     = sparse_[index];
        const Index lastPos = static_cast<Index>(dense_.size() - 1);
        const Index moved   = dense_[lastPos];

        if (pos != lastPos) {
            dense_[pos]    = moved;
            sparse_[moved] = pos;
        }

        dense_.pop_back();
        sparse_[index] = Invalid;
    }

    [[nodiscard]] Index IndexOf(Index index) const noexcept {
        if (!Contains(index)) {
            return Invalid;
        }
        return sparse_[index];
    }

    static constexpr Index Invalid = static_cast<Index>(-1);

private:
    void EnsureCapacity(Index index) {
        if (index >= sparse_.size()) {
            sparse_.resize(static_cast<std::size_t>(index) + 1, Invalid);
        }
    }

    std::vector<Index> dense_;
    std::vector<Index> sparse_;
};

} // namespace cc::ecs
