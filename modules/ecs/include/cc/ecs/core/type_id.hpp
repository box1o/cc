#pragma once

#include <cstddef>
#include <type_traits>
#include <atomic>

namespace cc::ecs {

using TypeID = std::size_t;

namespace detail {

inline TypeID NextTypeID() noexcept {
    static std::atomic<TypeID> counter{0};
    return counter++;
}

template<typename T>
TypeID TypeIDImpl() noexcept {
    static const TypeID id = NextTypeID();
    return id;
}

} // namespace detail

template<typename T>
[[nodiscard]] inline TypeID GetTypeID() noexcept {
    using Decayed = std::remove_cvref_t<T>;
    return detail::TypeIDImpl<Decayed>();
}

} // namespace cc::ecs
