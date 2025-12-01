#include <cc/ecs/core/registry.hpp>

namespace cc::ecs {

Entity Registry::Create() { 
    const EntityIndex idx = AllocateIndex();
    const EntityVersion ver = versions_[idx];
    return Entity{idx, ver};
}

void Registry::Destroy(Entity e) { 
    if (!IsValid(e)) {
        return;
    }

    ++versions_[e.index];
    RecycleIndex(e.index);

    //NOTE: component cleanup remains explicit; views filter by IsValid
}

bool Registry::IsValid(Entity e) const noexcept { 
    return e.index < versions_.size() &&
           e.version != 0 &&
           versions_[e.index] == e.version;
}

EntityIndex Registry::AllocateIndex() { 
    if (!freeList_.empty()) {
        const EntityIndex idx = freeList_.back();
        freeList_.pop_back();
        return idx;
    }

    const EntityIndex idx = static_cast<EntityIndex>(versions_.size());
    versions_.push_back(1);
    return idx;
}

void Registry::RecycleIndex(EntityIndex index) { 
    freeList_.push_back(index);
}

} // namespace cc::ecs
