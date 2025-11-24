#pragma once
#include <string>
#include <string_view>
#include <source_location>
#include "types.hpp"

namespace cc {

enum class error_category : u8 {
    core = 0,
    file_system = 1,
    network = 2,
    parse = 3,
    validation = 4,
    graphics = 5
};

enum class error_code : u16 {
    success = 0,

    file_not_found = 100,
    file_access_denied = 101,
    file_read_error = 102,
    file_write_error = 103,
    file_eof = 104,

    network_connection_failed = 200,
    network_connection_closed = 201,
    network_timeout = 202,
    network_host_unreachable = 203,
    network_send_failed = 204,
    network_receive_failed = 205,

    parse_invalid_format = 300,
    parse_unexpected_token = 301,
    parse_missing_field = 302,
    parse_type_mismatch = 303,

    validation_null_value = 400,
    validation_out_of_range = 401,
    validation_invalid_state = 402,

    graphics_init_failed = 500,
    graphics_device_lost = 501,
    graphics_invalid_format = 502,
    graphics_framebuffer_incomplete = 503,
    graphics_shader_compilation_failed = 504,
    graphics_texture_creation_failed = 505,
    graphics_buffer_creation_failed = 506,
    graphics_unsupported_api = 507,

    unknown_error = 999
};

class error {
    error_code code_;
    std::string message_;
    std::source_location location_;

public:
    constexpr error(error_code code, std::string_view msg,
                    std::source_location loc = std::source_location::current()) noexcept
    : code_(code), message_(msg), location_(loc) {}

    [[nodiscard]] constexpr error_code code() const noexcept { return code_; }
    [[nodiscard]] constexpr error_category category() const noexcept {
        u16 code_val = static_cast<u16>(code_);
        if (code_val >= 100 && code_val < 200) return error_category::file_system;
        if (code_val >= 200 && code_val < 300) return error_category::network;
        if (code_val >= 300 && code_val < 400) return error_category::parse;
        if (code_val >= 400 && code_val < 500) return error_category::validation;
        if (code_val >= 500 && code_val < 600) return error_category::graphics;
        return error_category::core;
    }

    [[nodiscard]] constexpr std::string_view message() const noexcept { return message_; }
    [[nodiscard]] constexpr const std::source_location& location() const noexcept { return location_; }

    [[nodiscard]] bool is_file_error() const noexcept { return category() == error_category::file_system; }
    [[nodiscard]] bool is_network_error() const noexcept { return category() == error_category::network; }
    [[nodiscard]] bool is_parse_error() const noexcept { return category() == error_category::parse; }
    [[nodiscard]] bool is_validation_error() const noexcept { return category() == error_category::validation; }
    [[nodiscard]] bool is_graphics_error() const noexcept { return category() == error_category::graphics; }

    [[nodiscard]] std::string format() const;
    [[nodiscard]] const char* code_string() const noexcept;
    [[nodiscard]] const char* category_string() const noexcept;

    void log() const;
};

}
