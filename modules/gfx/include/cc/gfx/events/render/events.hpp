#pragma once
#include "../base.hpp"

namespace cc::gfx::events {

struct FrameBeginEvent : EventBase {
    f32 deltaTime;

    explicit FrameBeginEvent(f32 dt)
        : deltaTime(dt) {}

    CC_GFX_EVENT_TYPE(FrameBegin)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

struct FrameEndEvent : EventBase {
    f32 frameTime;

    explicit FrameEndEvent(f32 ft)
        : frameTime(ft) {}

    CC_GFX_EVENT_TYPE(FrameEnd)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

struct RenderBeginEvent : EventBase {
    RenderBeginEvent() = default;

    CC_GFX_EVENT_TYPE(RenderBegin)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

struct RenderEndEvent : EventBase {
    RenderEndEvent() = default;

    CC_GFX_EVENT_TYPE(RenderEnd)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

struct ViewportResizeEvent : EventBase {
    u32 width;
    u32 height;

    ViewportResizeEvent(u32 w, u32 h)
        : width(w), height(h) {}

    CC_GFX_EVENT_TYPE(ViewportResize)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

struct SwapBuffersEvent : EventBase {
    SwapBuffersEvent() = default;

    CC_GFX_EVENT_TYPE(SwapBuffers)
    CC_GFX_EVENT_CATEGORY(EventCategory::Render)
};

} // namespace cc::gfx::events
