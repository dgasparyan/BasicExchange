#ifndef FIXED_STRING_H
#define FIXED_STRING_H

#include <array>
#include <string_view>
#include <compare>
#include <type_traits>
#include <cstring>

template<std::size_t Capacity, bool NullTerminated = true>
class FixedString {
    static_assert(Capacity > 0);
    static constexpr std::size_t max_chars_ =
        NullTerminated ? (Capacity - 1) : Capacity;

    std::array<char, Capacity> buf_{};   // zero-inited tail
    unsigned char len_ = 0;

public:
    // default
    constexpr FixedString() noexcept = default;

    // from string literal (compile-time length check)
    template<std::size_t N>
    requires (N > 0 && (N - 1) <= max_chars_)
    constexpr explicit FixedString(const char (&lit)[N]) noexcept {
        for (std::size_t i = 0; i < N - 1; ++i) buf_[i] = lit[i];
        len_ = static_cast<unsigned char>(N - 1);
        if constexpr (NullTerminated) buf_[len_] = '\0';
    }

    // from string_view (runtime length capped)
    constexpr explicit FixedString(std::string_view s) noexcept {
        const std::size_t n = s.size() <= max_chars_ ? s.size() : max_chars_;
        if (n) std::memcpy(buf_.data(), s.data(), n);
        len_ = static_cast<unsigned char>(n);
        if constexpr (NullTerminated) buf_[len_] = '\0';
    }

    explicit FixedString(const std::string& s) noexcept
    : FixedString(std::string_view{s}) {}

    // read-only access
    constexpr std::string_view view() const noexcept { return {buf_.data(), len_}; }
    constexpr std::size_t size() const noexcept { return len_; }
    static constexpr std::size_t capacity() noexcept { return max_chars_; }
    constexpr const char* data() const noexcept { return buf_.data(); }
    constexpr const char* c_str() const noexcept requires (NullTerminated) { return buf_.data(); }

    // comparisons with string semantics (ignore zeroed tail)
    friend constexpr bool operator==(const FixedString& a, const FixedString& b) noexcept {
        return a.len_ == b.len_ &&
               std::char_traits<char>::compare(a.buf_.data(), b.buf_.data(), a.len_) == 0;
    }
    friend constexpr std::strong_ordering operator<=>(const FixedString& a, const FixedString& b) noexcept {
        return std::string_view{a.buf_.data(), a.len_} <=> std::string_view{b.buf_.data(), b.len_};
    }
};

static_assert(std::is_trivially_copyable_v<FixedString<32>>);

#endif