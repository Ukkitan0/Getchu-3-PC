#pragma once
#include <string_view>

namespace ae3 {
// Source 0x25182C resource registration. At 0x2519D4 a null A3
// selects the unconditional branch. At 0x2519D8..0x2519E0,
// the first byte of a nonnull A3 is compared with ASCII '*' (0x2A);
// if equal, source skips 0x42C44C string-name comparison.
// Otherwise 0x2519FC invokes the original source string-equality helper.
// Only this verified selection boundary is translated; original
// registration traversal, ownership and record insertion are not.
inline bool originalResourceAcceptSelector_2519d4(
    std::string_view existingSourceName,
    const char* originalSelector) noexcept {
    return originalSelector == nullptr || originalSelector[0] == '*' ||
           existingSourceName == std::string_view(originalSelector);
}
} // namespace ae3
