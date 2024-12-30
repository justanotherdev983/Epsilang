#pragma once

#include <string>
#include <iostream>
#include <format>
#include <chrono>

static size_t g_error_count = 0;

enum class log_level_e
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
};

inline std::string log_level_to_string(const log_level_e& level)
{
    switch (level)
    {
        case log_level_e::DEBUG: return "DEBUG";
        case log_level_e::INFO: return "INFO";
        case log_level_e::WARNING: return "WARNING";
        case log_level_e::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    char buffer[32];  // Create a char buffer with sufficient size, because c func
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return std::string(buffer);
}

template <typename... Args>
void log_message(log_level_e level, std::string_view fmt, Args&&... args) {
    try {
        if (level == log_level_e::ERROR) {
            ++g_error_count;
        }

        std::string formatted_msg = std::vformat(fmt, std::make_format_args(args...));
        std::cerr << std::format("[{}][{}]: {}\n",
                               get_timestamp(),
                               log_level_to_string(level),
                               formatted_msg);
    } catch (const std::exception& e) {
        std::cerr << "Formatting error: " << e.what() << std::endl;
    }
}

template <typename... Args>
void debug_msg(std::string_view fmt, Args&&... args) {
    log_message(log_level_e::DEBUG, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void info_msg(std::string_view fmt, Args&&... args) {
    log_message(log_level_e::INFO, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void warning_msg(std::string_view fmt, Args&&... args) {
    log_message(log_level_e::WARNING, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void error_msg(std::string_view fmt, Args&&... args) {
    log_message(log_level_e::ERROR, fmt, std::forward<Args>(args)...);
}

inline size_t get_error_count() { return g_error_count; }
inline void reset_error_count() { g_error_count = 0; }