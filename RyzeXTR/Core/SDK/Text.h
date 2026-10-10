#pragma once
#include <algorithm>
#include <string_view>

namespace text {
    inline std::size_t BoundedUtf8Length(std::string_view value, std::size_t maxBytes) {
        auto length = (std::min)(value.size(), maxBytes);
        // The first excluded byte reveals whether the boundary split a character.
        if (length < value.size()) {
            while (length > 0 && (static_cast<unsigned char>(value[length]) & 0xc0) == 0x80)
                --length;
        }
        return length;
    }
}
