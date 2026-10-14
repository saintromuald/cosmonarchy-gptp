#pragma once
#include "types.h"
#include "BW/structures/common.h"

#include <optional>
#include <span>

// Frame header as stored in the file; defined in BW/structures.h.
struct GrpFrame;

namespace graphics {

struct Bitmap;

enum class FrameFormat {
    Rle,
    Raw,
};

struct FrameSize {
    u32 width = 0;
    u32 height = 0;
};

FrameSize fit_height(FrameSize frame, u32 target_height);

class GrpFile {
public:
    u16 frame_count() const { return m_frame_count; }
    s16 width() const { return m_width; }
    s16 height() const { return m_height; }

    std::span<const GrpFrame> frame_headers() const;

    const u8* get_frame_pixel_data(size_t frame_index) const;

    FrameSize frame_size(size_t frame_index) const;

    struct Blit {
        s32 x = 0;
        s32 y = 0;
        FrameFormat format = FrameFormat::Rle;
        u32 width = 0;    // 0 = native
        u32 height = 0;   // 0 = native

        // Nullopt: only the bitmap's own bounds clip the blit.
        std::optional<Box32> clip;

        const u8* remap = nullptr;
        u32 remap_length = 0;
    };

    void blit_frame(size_t frame_index, Bitmap& bitmap, const Blit& options) const;

private:
    u16 m_frame_count;
    s16 m_width;
    s16 m_height;
    u8 m_raw_data[];
};

// --- Inline unit icon resolution -------------------------------------------

struct UnitIcon {
    const GrpFile* grp;
    u16 frame;
};

std::optional<UnitIcon> unit_icon(u16 unit_id);
s32 unit_icon_advance(u16 unit_id, u32 pixel_height, s32 spacing);

} // namespace graphics