#pragma once

#include <array>
#include <string>
#include <utility>
#include <variant>
#include <cstdint>

namespace db {
    constexpr size_t INT_SIZE = sizeof(int);
    constexpr size_t DOUBLE_SIZE = sizeof(double);
    constexpr size_t CHAR_SIZE = 64;

    enum class type_t {
        INT, CHAR, DOUBLE
    };

    using field_t = std::variant<int, double, std::string>;

    struct PageId {
        std::string file;
        size_t page;

    public:
        bool operator==(const PageId &) const = default;
    };

    constexpr size_t DEFAULT_PAGE_SIZE = 4096;

    using Page = std::array<uint8_t, DEFAULT_PAGE_SIZE>;
} // namespace db

namespace std {

    template<>
    struct hash<db::PageId> {
        size_t operator()(db::PageId const &pid) const noexcept {
            auto h1 = std::hash<std::string>()(pid.file);
            auto h2 = std::hash<size_t>()(pid.page);

            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };

} // namespace std

