#pragma once

#include "../core/registry.hpp"
#include "../storage/component_storage.hpp"

#include <tuple>
#include <type_traits>
#include <cassert>
#include <limits>

namespace cc::ecs {

template<typename... Ts>
struct first_type;

template<typename T, typename... Ts>
struct first_type<T, Ts...> {
    using type = T;
};

template<typename... Ts>
using first_type_t = typename first_type<Ts...>::type;

//NOTE: helper that can see Registry internals via friend
namespace detail {

struct ViewAccess {
    template<typename T>
    [[nodiscard]] static const ComponentStorage<T>* GetStorage(const Registry& registry) {
        return registry.template GetStorage<T>();
    }
};

//NOTE: pick smallest component storage as driver
template<typename... Components>
[[nodiscard]] const std::vector<EntityIndex>* SelectDriver(const Registry& registry) {
    const std::vector<EntityIndex>* driverDense = nullptr;
    std::size_t                     driverSize  = std::numeric_limits<std::size_t>::max();

    auto consider = [&](auto typeTag) {
        using Comp = decltype(typeTag);
        using T    = std::remove_cvref_t<Comp>;

        const auto* storage = ViewAccess::GetStorage<T>(registry);
        if (!storage) {
            return;
        }

        const auto& dense = storage->DenseEntities();
        if (dense.size() < driverSize) {
            driverSize  = dense.size();
            driverDense = &dense;
        }
    };

    (consider(std::type_identity<Components>{}), ...);

    return driverDense;
}

} // namespace detail

template<typename... Components>
class BasicView {
public:
    using RegistryType = Registry;

    explicit BasicView(RegistryType& registry) noexcept
        : registry_(&registry) {}

    struct Iterator {
        RegistryType*                   registry{nullptr};
        const std::vector<EntityIndex>* entities{nullptr};
        std::size_t                     index{0};

        void AdvanceUntilValid() {
            if (!registry || !entities) {
                return;
            }

            const auto count = entities->size();
            while (index < count) {
                const EntityIndex idx = (*entities)[index];

                if (idx >= registry->versions_.size()) {
                    ++index;
                    continue;
                }

                const EntityVersion ver = registry->versions_[idx];
                Entity e{idx, ver};

                if (!registry->IsValid(e)) {
                    ++index;
                    continue;
                }

                const bool hasAll = (registry->template Has<Components>(e) && ...);
                if (hasAll) {
                    break;
                }

                ++index;
            }
        }

        auto operator*() const {
            assert(registry && entities);
            const EntityIndex idx = (*entities)[index];
            const EntityVersion ver = registry->versions_[idx];
            Entity e{idx, ver};

            return std::forward_as_tuple(
                e,
                registry->template Get<Components>(e)...
            );
        }

        Iterator& operator++() {
            ++index;
            AdvanceUntilValid();
            return *this;
        }

        [[nodiscard]] bool operator==(const Iterator& other) const noexcept {
            return registry == other.registry &&
                   entities == other.entities &&
                   index == other.index;
        }

        [[nodiscard]] bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }
    };

    [[nodiscard]] Iterator begin() {
        if constexpr (sizeof...(Components) == 0) {
            return Iterator{registry_, nullptr, 0};
        }

        auto* dense = detail::SelectDriver<Components...>(*registry_);
        if (!dense) {
            return Iterator{registry_, nullptr, 0};
        }

        Iterator it{registry_, dense, 0};
        it.AdvanceUntilValid();
        return it;
    }

    [[nodiscard]] Iterator end() {
        if constexpr (sizeof...(Components) == 0) {
            return Iterator{registry_, nullptr, 0};
        }

        auto* dense = detail::SelectDriver<Components...>(*registry_);
        if (!dense) {
            return Iterator{registry_, nullptr, 0};
        }

        return Iterator{registry_, dense, dense->size()};
    }

private:
    RegistryType* registry_{nullptr};
};

} // namespace cc::ecs
