#pragma once
#include "error.hpp"            // IWYU pragma: keep
#include "result.hpp"
#include <concepts>
#include <span>
#include <string>
#include <vector>
#include <type_traits>

namespace cc {

template<typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

template<typename T>
concept reader = requires(T t, std::span<std::byte> buf) {
    { t.read(buf) } -> std::convertible_to<size_t>;
    { t.is_open() } -> std::convertible_to<bool>;
};

template<typename T>
concept writer = requires(T t, std::span<const std::byte> buf) {
    { t.write(buf) } -> std::convertible_to<size_t>;
    { t.flush() } -> std::same_as<void>;
    { t.is_open() } -> std::convertible_to<bool>;
};

template<typename T>
concept line_reader = reader<T> && requires(T t) {
    { t.read_line() } -> std::same_as<result<std::string>>;
};

template<typename T>
concept serializable = requires(T t) {
    { t.serialize() } -> std::same_as<std::vector<std::byte>>;
} || trivially_copyable<T>;

template<typename T>
concept deserializable = requires(T t, std::span<const std::byte> data) {
    { T::deserialize(data) } -> std::same_as<result<T>>;
};

} // namespace cc
