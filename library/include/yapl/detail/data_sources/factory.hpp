#pragma once

#include "yapl/detail/data_sources/file.hpp"
#include "yapl/detail/data_sources/http.hpp"

#include <memory>
#include <string_view>
#include <variant>

namespace yapl::data_sources {

// Compile-time case-insensitive prefix check (avoids runtime string allocation)
constexpr bool starts_with_icase(std::string_view str, std::string_view prefix) noexcept {
    if (str.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        char c1 = str[i];
        char c2 = prefix[i];
        // ASCII lowercase conversion
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return false;
    }
    return true;
}

// Compile-time URL scheme detection
constexpr bool is_http_url(std::string_view url) noexcept {
    return starts_with_icase(url, "http://") || starts_with_icase(url, "https://");
}

// Variant type for polymorphic data sources with static dispatch
using data_source_variant = std::variant<
    std::unique_ptr<file>,
    std::unique_ptr<http>
>;

// Factory function to create appropriate data source based on URL
inline data_source_variant create(std::string_view url) {
    if (is_http_url(url)) {
        return std::make_unique<http>(std::string(url));
    }
    return std::make_unique<file>(std::filesystem::path(url));
}

// Helper to visit data source operations (non-const)
template <typename Func>
auto visit(data_source_variant& ds, Func&& func) {
    return std::visit([&func](auto& ptr) { return func(*ptr); }, ds);
}

// Helper to visit data source operations (const)
template <typename Func>
auto visit(const data_source_variant& ds, Func&& func) {
    return std::visit([&func](const auto& ptr) { return func(*ptr); }, ds);
}

} // namespace yapl::data_sources
