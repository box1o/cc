#pragma once
#include "base.hpp"
#include "scoped_connection.hpp"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <atomic>
#include <chrono>

namespace cc::gfx::events {

enum class Priority : u8 {
    Lowest  = 0,
    Low     = 64,
    Normal  = 128,
    High    = 192,
    Highest = 255
};

class EventBus {
public:
    static ref<EventBus> Create() {
        return createRef<EventBus>();
    }

    template<Event E, typename Fn>
    requires std::invocable<Fn, const E&>
    [[nodiscard]] ScopedConnection On(Fn&& fn, Priority priority = Priority::Normal) {
        auto id   = nextId_++;
        auto type = E::GetStaticType();

        auto wrapper = [fn = std::forward<Fn>(fn)](const EventBase& event) -> bool {
            const auto& typed = static_cast<const E&>(event);
            if constexpr (std::is_same_v<std::invoke_result_t<Fn, const E&>, bool>) {
                return fn(typed);
            } else {
                fn(typed);
                return false;
            }
        };

        auto& listeners = listeners_[type];
        listeners.emplace_back(ListenerData{
            .id       = id,
            .priority = static_cast<u8>(priority),
            .callback = std::move(wrapper)
        });

        std::ranges::sort(listeners, [](const auto& a, const auto& b) {
            return a.priority > b.priority;
        });

        return ScopedConnection(id, [this, type](ConnectionID connId) {
            Disconnect(type, connId);
        });
    }

    template<Event E>
    void Emit(E& event) {
        const auto type = event.GetEventType();
        auto it = listeners_.find(type);
        if (it == listeners_.end()) [[unlikely]] return;

        event.timestamp = GetTime();

        for (auto& listener : it->second) {
            if (listener.callback(event)) [[unlikely]] {
                event.handled = true;
                break;
            }
        }
    }

    template<Event E>
    void Emit(const E& event) {
        auto& mutableEvent = const_cast<E&>(event);
        Emit(mutableEvent);
    }

    void Emit(EventBase& event) {
        const auto type = event.GetEventType();
        auto it = listeners_.find(type);
        if (it == listeners_.end()) [[unlikely]] return;

        event.timestamp = GetTime();

        for (auto& listener : it->second) {
            if (listener.callback(event)) [[unlikely]] {
                event.handled = true;
                break;
            }
        }
    }

    void Clear() {
        listeners_.clear();
    }

    void Clear(EventType type) {
        listeners_.erase(type);
    }

private:
    struct ListenerData {
        ConnectionID id;
        u8           priority;
        std::function<bool(const EventBase&)> callback;
    };

    void Disconnect(EventType type, ConnectionID id) {
        auto it = listeners_.find(type);
        if (it != listeners_.end()) {
            auto& vec = it->second;
            std::erase_if(vec, [id](const auto& data) { return data.id == id; });
        }
    }

    static f64 GetTime() noexcept {
        static auto start = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<f64>(now - start).count();
    }

    std::unordered_map<EventType, std::vector<ListenerData>> listeners_;
    std::atomic<ConnectionID> nextId_{1};
};

} // namespace cc::gfx::events
