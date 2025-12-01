#pragma once

#include "entity.hpp"
#include "type_id.hpp"
#include "../storage/component_storage.hpp"

#include <cc/core/types.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cassert>
#include <concepts>
#include <utility>

namespace cc::ecs {

template<typename... Components>
class BasicView;

//NOTE: helper for friendship of view internals
namespace detail {
    struct ViewAccess;
}

//NOTE: Registry manages entity lifetimes and component storages.
class Registry {
public:
    Registry() = default;
    ~Registry() = default;

    [[nodiscard]] Entity Create();

    void Destroy(Entity e);

    [[nodiscard]] bool IsValid(Entity e) const noexcept;

    template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    T& Emplace(Entity e, Args&&... args) {
        assert(IsValid(e));
        auto& storage = GetOrCreateStorage<T>();
        return storage.Emplace(e, std::forward<Args>(args)...);
    }

    template<typename T>
    [[nodiscard]] bool Has(Entity e) const {
        if (!IsValid(e)) {
            return false;
        }
        const auto* storage = GetStorage<T>();
        return storage ? storage->Has(e) : false;
    }

    template<typename T>
    [[nodiscard]] T& Get(Entity e) {
        assert(IsValid(e));
        auto* storage = GetStorage<T>();
        assert(storage && "ComponentStorage not found.");
        return storage->Get(e);
    }

    template<typename T>
    [[nodiscard]] const T& Get(Entity e) const {
        assert(IsValid(e));
        const auto* storage = GetStorage<T>();
        assert(storage && "ComponentStorage not found.");
        return storage->Get(e);
    }

    template<typename T>
    void Remove(Entity e) {
        if (!IsValid(e)) {
            return;
        }
        auto* storage = GetStorage<T>();
        if (storage) {
            storage->Remove(e);
        }
    }

    template<typename... Components>
    [[nodiscard]] BasicView<Components...> View() {
        return BasicView<Components...>(*this);
    }

private:
    struct IStorage {
        virtual ~IStorage() = default;
    };

    template<typename T>
    struct StorageImpl final : IStorage {
        ComponentStorage<T> storage;
    };

    std::vector<EntityVersion> versions_;
    std::vector<EntityIndex>   freeList_;
    std::unordered_map<TypeID, std::unique_ptr<IStorage>> storages_;

    [[nodiscard]] EntityIndex AllocateIndex();
    void RecycleIndex(EntityIndex index);

    template<typename T>
    [[nodiscard]] ComponentStorage<T>* GetStorage() {
        const auto it = storages_.find(GetTypeID<T>());
        if (it == storages_.end()) {
            return nullptr;
        }
        auto* base = static_cast<StorageImpl<T>*>(it->second.get());
        return &base->storage;
    }

    template<typename T>
    [[nodiscard]] const ComponentStorage<T>* GetStorage() const {
        const auto it = storages_.find(GetTypeID<T>());
        if (it == storages_.end()) {
            return nullptr;
        }
        auto* base = static_cast<const StorageImpl<T>*>(it->second.get());
        return &base->storage;
    }

    template<typename T>
    [[nodiscard]] ComponentStorage<T>& GetOrCreateStorage() {
        const auto id = GetTypeID<T>();
        auto it = storages_.find(id);
        if (it == storages_.end()) {
            auto storage = std::make_unique<StorageImpl<T>>();
            auto* ptr    = &storage->storage;
            storages_.emplace(id, std::move(storage));
            return *ptr;
        }
        auto* base = static_cast<StorageImpl<T>*>(storages_[id].get());
        return base->storage;
    }

    template<typename...>
    friend class BasicView;

    friend struct detail::ViewAccess; //NOTE: grants helper access to private GetStorage
};

} // namespace cc::ecs
