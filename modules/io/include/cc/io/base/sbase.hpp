#pragma once

#include <cc/core/core.hpp>
#include <span>
#include <array>
#include <cstring>


namespace cc::io{

//NOTE: implements a CRTP base class for derived classes writer and reader ... from core concepts
template <typename Derived>
class sbase{
protected:
    constexpr Derived& self() noexcept {
        return static_cast<Derived&>(*this);
    }

    constexpr const Derived& self()const noexcept {
        return static_cast<Derived&>(*this);
    }
public:

    [[nodiscard]] size_t read(std::span<std::byte> buffer) {
        return self().read_impl(buffer);
    }

    [[nodiscard]] size_t write(std::span<const std::byte> data) {
        return self().write_impl(data);
    }

    void flush() {
        self().flush_impl();
    }

    [[nodiscard]] bool is_open() const noexcept {
        return self().is_open_impl();
    }

    template<cc::trivially_copyable T>
    [[nodiscard]] cc::result<T> read_binary() {
        std::array<std::byte, sizeof(T)> buffer;
        size_t bytes_read = read(buffer);

        if (bytes_read != sizeof(T)) {
            return cc::err(cc::error_code::file_eof, "Incomplete read");
        }

        T value;
        std::memcpy(&value, buffer.data(), sizeof(T));
        return value;
    }

    template<cc::trivially_copyable T>
    [[nodiscard]] cc::result<void> write_binary(const T& value) {
        auto data = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(&value), sizeof(T));

        if (write(data) != sizeof(T)) {
            return cc::err(cc::error_code::file_write_error, "Write failed");
        }

        return cc::ok();
    }

};



}
