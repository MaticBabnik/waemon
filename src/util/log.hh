#include <format>
#include <iostream>
#include <string_view>

namespace logger {

// ANSI escape codes for colors
constexpr std::string_view RESET_COLOR = "\033[0m";
constexpr std::string_view INFO_COLOR  = "\033[1;32m"; // Bright Green
constexpr std::string_view WARN_COLOR  = "\033[1;33m"; // Bright Yellow

// Define the info function
template <typename... Args> void info(std::string_view format, Args &&...args) {
    std::cout << INFO_COLOR << "[INFO] "
              << std::vformat(format, std::make_format_args(args...))
              << RESET_COLOR << "\n";
}

// Define the warn function
template <typename... Args> void warn(std::string_view format, Args &&...args) {
    std::cout << WARN_COLOR << "[WARN] "
              << std::vformat(format, std::make_format_args(args...))
              << RESET_COLOR << "\n";
}

}