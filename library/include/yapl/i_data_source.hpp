#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

namespace yapl {

// Concept for data sources - enables static dispatch without virtual function overhead
template <typename T>
concept data_source = requires(T& t, const T& ct, size_t size, std::span<uint8_t> buffer) {
    { t.open() } -> std::same_as<void>;
    { t.close() } -> std::same_as<void>;
    { t.read_data(size, buffer) } -> std::same_as<size_t>;
    { ct.available() } -> std::same_as<size_t>;
    { ct.is_open() } -> std::same_as<bool>;
    { t.reset() } -> std::same_as<void>;
};

} // namespace yapl
