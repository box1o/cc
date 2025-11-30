#pragma once
#include "../base.hpp"

namespace cc::gfx::events {

struct WindowCloseEvent : EventBase {
    WindowCloseEvent() = default;

    CC_GFX_EVENT_TYPE(WindowClose)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowResizeEvent : EventBase {
    u32 width;
    u32 height;

    WindowResizeEvent(u32 w, u32 h) : width(w), height(h) {}

    CC_GFX_EVENT_TYPE(WindowResize)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowFocusEvent : EventBase {
    WindowFocusEvent() = default;

    CC_GFX_EVENT_TYPE(WindowFocus)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowLostFocusEvent : EventBase {
    WindowLostFocusEvent() = default;

    CC_GFX_EVENT_TYPE(WindowLostFocus)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowMovedEvent : EventBase {
    i32 x;
    i32 y;

    WindowMovedEvent(i32 x_, i32 y_) : x(x_), y(y_) {}

    CC_GFX_EVENT_TYPE(WindowMoved)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowMinimizedEvent : EventBase {
    WindowMinimizedEvent() = default;

    CC_GFX_EVENT_TYPE(WindowMinimized)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowMaximizedEvent : EventBase {
    WindowMaximizedEvent() = default;

    CC_GFX_EVENT_TYPE(WindowMaximized)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

struct WindowRestoredEvent : EventBase {
    WindowRestoredEvent() = default;

    CC_GFX_EVENT_TYPE(WindowRestored)
    CC_GFX_EVENT_CATEGORY(EventCategory::Window)
};

} // namespace cc::gfx::events
