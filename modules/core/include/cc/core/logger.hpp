#pragma once
#include <memory>
#include <string_view>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace cc::log {

enum class Level {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
};

namespace detail {
inline std::shared_ptr<spdlog::logger>& GetLoggerInstance() noexcept {
    static std::shared_ptr<spdlog::logger> g_logger;
    return g_logger;
}

inline spdlog::level::level_enum ToSpdlogLevel(Level level) noexcept {
    switch (level) {
        case Level::Trace:    return spdlog::level::trace;
        case Level::Debug:    return spdlog::level::debug;
        case Level::Info:     return spdlog::level::info;
        case Level::Warn:     return spdlog::level::warn;
        case Level::Error:    return spdlog::level::err;
        case Level::Critical: return spdlog::level::critical;
        case Level::Off:      return spdlog::level::off;
        default:              return spdlog::level::info;
    }
}
}

inline void Init(const std::string_view &name) {
    spdlog::set_pattern("%T [%n] %v%$");
    detail::GetLoggerInstance() = spdlog::stdout_color_mt(std::string(name));
    detail::GetLoggerInstance()->set_level(spdlog::level::trace);
}

inline void SetLevel(Level level) noexcept {
    auto spdlog_level = detail::ToSpdlogLevel(level);
    auto& logger = detail::GetLoggerInstance();
    if (logger) {
        logger->set_level(spdlog_level);
    }
    spdlog::set_level(spdlog_level);
}

inline std::shared_ptr<spdlog::logger>& GetLogger() noexcept {
    return detail::GetLoggerInstance();
}

template<typename... Args>
inline void Trace(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->trace(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline void Debug(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->debug(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline void Info(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->info(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline void Warn(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->warn(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline void Error(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->error(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline void Critical(std::string_view fmt, Args&&... args) {
    auto& logger = GetLogger();
    if (logger) {
        logger->critical(fmt::runtime(fmt), std::forward<Args>(args)...);
    }
}

}
