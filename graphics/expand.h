#pragma once
#include "../types.h"

#include <algorithm>

namespace graphics {

// Exact integer magnification: expands each source byte to a Factor block and
// each source row to Factor destination rows (the first row is expanded, the
// rest are copies). Nearest-neighbor — pixel-pure, matching the zoom midpoint
// LUT exactly at integer factors.
template <u32 Factor>
void expand_integer(const u8* source, u8* destination, u32 destination_pitch, u32 width, u32 height) {
    static_assert(Factor > 1);
    const size_t row_bytes = size_t(width) * Factor;
    for (u32 y = 0; y < height; ++y) {
        u8* row = destination + size_t(y) * Factor * destination_pitch;
        const u8* src = source + size_t(y) * width;
        for (u32 x = 0; x < width; ++x) {
            std::fill_n(row + size_t(x) * Factor, Factor, src[x]);
        }
        for (u32 r = 1; r < Factor; ++r) {
            std::copy_n(row, row_bytes, row + size_t(r) * destination_pitch);
        }
    }
}

} // graphics
