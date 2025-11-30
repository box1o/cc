#pragma once
#include "type.hpp"
#include "category.hpp"
#include <string_view>

namespace cc::gfx::events {

template<typename T>
concept Event = requires(const T t) {
    { T::GetStaticType() } -> std::same_as<EventType>;
    { t.GetEventType() }   -> std::same_as<EventType>;
    { t.GetCategoryFlags() } -> std::same_as<u16>;
    { t.GetName() }        -> std::convertible_to<std::string_view>;
};

struct EventBase {
    bool handled{false};
    f64  timestamp{0.0};

    virtual ~EventBase() = default;
    virtual EventType GetEventType() const = 0;
    virtual u16 GetCategoryFlags() const = 0;
    virtual std::string_view GetName() const = 0;

    [[nodiscard]] bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & category;
    }
};

#define CC_GFX_EVENT_TYPE(type) \
    static ::cc::gfx::events::EventType GetStaticType() { \
        return ::cc::gfx::events::EventType::type; \
    } \
    ::cc::gfx::events::EventType GetEventType() const override { \
        return GetStaticType(); \
    } \
    std::string_view GetName() const override { \
        return #type; \
    }

#define CC_GFX_EVENT_CATEGORY(category) \
    u16 GetCategoryFlags() const override { \
        return static_cast<u16>(category); \
    }

} // namespace cc::gfx::events
