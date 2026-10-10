#pragma once
#include <cstdlib>
#include <cstring>

namespace resolver {
enum class Mode : int { Off, Standard, Experimental };

inline Mode GetMode(int value) {
    return value >= 0 && value <= static_cast<int>(Mode::Experimental)
        ? static_cast<Mode>(value) : Mode::Off;
}

// An explicit new setting wins over the legacy checkbox, including Off.
inline int LoadMode(const char* value, const char* legacy, int current) {
    if (value && *value) {
        char* end = nullptr;
        const long parsed = std::strtol(value, &end, 10);
        return end != value && *end == '\0' && parsed >= 0 && parsed <= 2
            ? static_cast<int>(parsed) : static_cast<int>(Mode::Off);
    }
    if (legacy && *legacy)
        return std::strcmp(legacy, "true") == 0 ? 1 : 0;
    return static_cast<int>(GetMode(current));
}
}
