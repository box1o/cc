#pragma once
#include <cc/core/core.hpp>
#include <functional>

namespace cc::gfx::events {

using ConnectionID = u64;

class ScopedConnection {
public:
    ScopedConnection() = default;

    ScopedConnection(ConnectionID id, std::function<void(ConnectionID)> disconnectFn)
        : id_(id)
        , disconnectFn_(std::move(disconnectFn))
        , connected_(true) {}

    ~ScopedConnection() {
        Disconnect();
    }

    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    ScopedConnection(ScopedConnection&& other) noexcept
        : id_(other.id_)
        , disconnectFn_(std::move(other.disconnectFn_))
        , connected_(other.connected_) {
        other.connected_ = false;
    }

    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            Disconnect();
            id_ = other.id_;
            disconnectFn_ = std::move(other.disconnectFn_);
            connected_ = other.connected_;
            other.connected_ = false;
        }
        return *this;
    }

    void Disconnect() {
        if (connected_ && disconnectFn_) {
            disconnectFn_(id_);
            connected_ = false;
        }
    }

    [[nodiscard]] bool IsConnected() const { return connected_; }
    [[nodiscard]] ConnectionID GetID() const { return id_; }

private:
    ConnectionID id_{0};
    std::function<void(ConnectionID)> disconnectFn_{};
    bool connected_{false};
};

} // namespace cc::gfx::events
