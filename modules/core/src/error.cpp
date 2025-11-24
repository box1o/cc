#include <cc/core/error.hpp>
#include <cc/core/logger.hpp>
#include <format>

namespace cc {

const char* error::code_string() const noexcept {
    switch (code_) {
        case error_code::success: return "Success";

        case error_code::file_not_found: return "FileNotFound";
        case error_code::file_access_denied: return "FileAccessDenied";
        case error_code::file_read_error: return "FileReadError";
        case error_code::file_write_error: return "FileWriteError";
        case error_code::file_eof: return "FileEOF";

        case error_code::network_connection_failed: return "NetworkConnectionFailed";
        case error_code::network_connection_closed: return "NetworkConnectionClosed";
        case error_code::network_timeout: return "NetworkTimeout";
        case error_code::network_host_unreachable: return "NetworkHostUnreachable";
        case error_code::network_send_failed: return "NetworkSendFailed";
        case error_code::network_receive_failed: return "NetworkReceiveFailed";

        case error_code::parse_invalid_format: return "ParseInvalidFormat";
        case error_code::parse_unexpected_token: return "ParseUnexpectedToken";
        case error_code::parse_missing_field: return "ParseMissingField";
        case error_code::parse_type_mismatch: return "ParseTypeMismatch";

        case error_code::validation_null_value: return "ValidationNullValue";
        case error_code::validation_out_of_range: return "ValidationOutOfRange";
        case error_code::validation_invalid_state: return "ValidationInvalidState";

        case error_code::graphics_init_failed: return "GraphicsInitFailed";
        case error_code::graphics_device_lost: return "GraphicsDeviceLost";
        case error_code::graphics_invalid_format: return "GraphicsInvalidFormat";
        case error_code::graphics_framebuffer_incomplete: return "GraphicsFramebufferIncomplete";
        case error_code::graphics_shader_compilation_failed: return "GraphicsShaderCompilationFailed";
        case error_code::graphics_texture_creation_failed: return "GraphicsTextureCreationFailed";
        case error_code::graphics_buffer_creation_failed: return "GraphicsBufferCreationFailed";
        case error_code::graphics_unsupported_api: return "GraphicsUnsupportedAPI";

        case error_code::unknown_error: return "UnknownError";
        default: return "Unknown";
    }
}

const char* error::category_string() const noexcept {
    switch (category()) {
        case error_category::core: return "Core";
        case error_category::file_system: return "FileSystem";
        case error_category::network: return "Network";
        case error_category::parse: return "Parse";
        case error_category::validation: return "Validation";
        case error_category::graphics: return "Graphics";
        default: return "Unknown";
    }
}

std::string error::format() const {
    return std::format("{}:{} in {} - [{}::{}] {}",
                       location_.file_name(),
                       location_.line(),
                       location_.function_name(),
                       category_string(),
                       code_string(),
                       message_);
}

void error::log() const {
    switch (category()) {
        case error_category::file_system:
            log::Warn("[FileSystem] {}", format());
            break;
        case error_category::network:
            log::Error("[Network] {}", format());
            break;
        case error_category::parse:
            log::Warn("[Parse] {}", format());
            break;
        case error_category::validation:
            log::Warn("[Validation] {}", format());
            break;
        case error_category::graphics:
            log::Error("[Graphics] {}", format());
            break;
        default:
            log::Error("{}", format());
            break;
    }
}

}
