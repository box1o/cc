#pragma once
#include "../base.hpp"

namespace cc::gfx::events {

struct AppTickEvent : EventBase {
    f32 deltaTime;

    explicit AppTickEvent(f32 dt)
        : deltaTime(dt) {}

    CC_GFX_EVENT_TYPE(AppTick)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

struct AppUpdateEvent : EventBase {
    f32 deltaTime;

    explicit AppUpdateEvent(f32 dt)
        : deltaTime(dt) {}

    CC_GFX_EVENT_TYPE(AppUpdate)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

struct AppRenderEvent : EventBase {
    AppRenderEvent() = default;

    CC_GFX_EVENT_TYPE(AppRender)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

struct AppShutdownEvent : EventBase {
    AppShutdownEvent() = default;

    CC_GFX_EVENT_TYPE(AppShutdown)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

struct AppSuspendEvent : EventBase {
    AppSuspendEvent() = default;

    CC_GFX_EVENT_TYPE(AppSuspend)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

struct AppResumeEvent : EventBase {
    AppResumeEvent() = default;

    CC_GFX_EVENT_TYPE(AppResume)
    CC_GFX_EVENT_CATEGORY(EventCategory::Application)
};

} // namespace cc::gfx::events
