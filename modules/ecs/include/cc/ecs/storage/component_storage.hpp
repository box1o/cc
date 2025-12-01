#pragma once

#include "../core/entity.hpp"
#include "sparse_set.hpp"

#include <vector>
#include <utility>
#include <cassert>

namespace cc::ecs {

template<typename T>
class ComponentStorage {
public:
    using Component = T;

    ComponentStorage() = default;

    template<typename... Args>
    T& Emplace(Entity e, Args&&... args) {
        const auto idx = e.index;
        const auto pos = sparse_.Insert(idx);

        if (pos == components_.size()) {
            components_.emplace_back(std::forward<Args>(args)...);
        } else {
            components_[pos] = T(std::forward<Args>(args)...);
        }

        return components_[pos];
    }

    void Remove(Entity e) {
        const auto idx = e.index;
        if (!sparse_.Contains(idx)) {
            return;
        }

        const auto pos  = sparse_.IndexOf(idx);
        const auto last = components_.size() - 1;
        assert(pos != SparseSet::Invalid);

        if (pos != last) {
            components_[pos] = std::move(components_[last]);
        }

        components_.pop_back();
        sparse_.Erase(idx);
    }

    [[nodiscard]] bool Has(Entity e) const noexcept {
        return sparse_.Contains(e.index);
    }

    [[nodiscard]] T& Get(Entity e) {
        const auto pos = sparse_.IndexOf(e.index);
        assert(pos != SparseSet::Invalid);
        return components_[pos];
    }

    [[nodiscard]] const T& Get(Entity e) const {
        const auto pos = sparse_.IndexOf(e.index);
        assert(pos != SparseSet::Invalid);
        return components_[pos];
    }

    [[nodiscard]] const std::vector<SparseSet::Index>& DenseEntities() const noexcept {
        return sparse_.Dense();
    }

    [[nodiscard]] const std::vector<T>& DenseComponents() const noexcept {
        return components_;
    }

    [[nodiscard]] std::vector<T>& DenseComponents() noexcept {
        return components_;
    }

    [[nodiscard]] std::size_t Size() const noexcept {
        return components_.size();
    }

    [[nodiscard]] bool Empty() const noexcept {
        return components_.empty();
    }

private:
    SparseSet      sparse_;
    std::vector<T> components_;
};

} // namespace cc::ecs
