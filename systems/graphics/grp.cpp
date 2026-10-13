#include "grp.h"

#include "BW/structures.h"
#include "DatExtend/datext.h"
#include "graphics/Bitmap.h"

#include <algorithm>
#include <cstdint>

namespace graphics {
namespace {

// Frame data_offset can be file-relative (sfile loads) or an absolute pointer
// once vanilla's loader rewrites it, and its high bit doubles as a "frame
// wider than 255px" flag. Resolved the way cursor_cache and stat_resource do.
struct FrameView {
    const u8* data;
    u32 width;
    u32 height;
};

FrameView frame_view(const GrpFile& file, const GrpFrame& header) {
    const u8* base = reinterpret_cast<const u8*>(&file);
    const uintptr_t offset = header.data_offset & 0x7FFFFFFFu;
    FrameView view;
    view.data = offset >= reinterpret_cast<uintptr_t>(base)
        ? reinterpret_cast<const u8*>(offset)
        : base + offset;
    view.width = header.w;
    if (header.data_offset & 0x80000000u) {
        view.width += 256;
    }
    view.height = header.h;
    return view;
}

// Frame width is bounded by the header: u8 w plus the "wider than 255px"
// flag's extra 256.
constexpr u32 MaxFrameWidth = 511;

// Decodes one frame row into out (view.width bytes, 0 = transparent).
void decode_row(const FrameView& view, FrameFormat format, u32 row, u8* out) {
    std::fill_n(out, view.width, u8(0));
    if (format == FrameFormat::Raw) {
        std::copy_n(view.data + size_t(row) * view.width, view.width, out);
        return;
    }
    // Rle: u16 row-offset table, then opcode lines - skip = 0x80|n transparent
    // pixels, run = 0x40|n repeats of the next color byte, plain n = n literal
    // color bytes. Same semantics cursor_cache and PyMS's GRP library decode
    // with; rows are independently addressed, so an overrun cannot desync
    // the rows after it.
    const u16* row_offsets = reinterpret_cast<const u16*>(view.data);
    if (row_offsets[row] == 0) {
        return; // empty line
    }
    const u8* stream = view.data + row_offsets[row];
    u32 col = 0;
    while (col < view.width) {
        const u8 opcode = *stream++;
        if (opcode & 0x80) {
            col += opcode & 0x7F;
        } else if (opcode & 0x40) {
            const u8 count = opcode & 0x3F;
            if (count == 0) {
                break;
            }
            const u8 color = *stream++;
            for (u32 i = 0; i < count && col < view.width; ++i, ++col) {
                out[col] = color;
            }
        } else {
            if (opcode == 0) {
                break;
            }
            for (u32 i = 0; i < opcode && col < view.width; ++i, ++col) {
                out[col] = *stream++;
            }
        }
    }
}

}

std::span<const GrpFrame> GrpFile::frame_headers() const {
    return {reinterpret_cast<const GrpFrame*>(m_raw_data), m_frame_count};
}

std::span<GrpFrame> GrpFile::frame_headers() {
    return {reinterpret_cast<GrpFrame*>(m_raw_data), m_frame_count};
}

const u8* GrpFile::get_frame_pixel_data(size_t frame_index) const {
    if (frame_index >= m_frame_count) {
        frame_index = 0;
    }
    return reinterpret_cast<const u8*>(this) + frame_headers()[frame_index].data_offset;
}

FrameSize GrpFile::frame_size(size_t frame_index) const {
    if (frame_index >= m_frame_count) {
        return {};
    }
    const GrpFrame& header = frame_headers()[frame_index];
    FrameSize size;
    size.width = header.w;
    if (header.data_offset & 0x80000000u) {
        size.width += 256;
    }
    size.height = header.h;
    return size;
}

FrameSize fit_height(FrameSize frame, u32 target_height) {
    if (!frame.width || !frame.height || !target_height) {
        return {};
    }
    FrameSize fitted;
    fitted.height = std::min(target_height, frame.height);
    fitted.width = frame.width * fitted.height / frame.height;
    return fitted.width ? fitted : FrameSize{};
}

void GrpFile::blit_frame(size_t frame_index, Bitmap& bitmap, const Blit& options) const {
    if (frame_index >= m_frame_count) {
        return;
    }
    const FrameView view = frame_view(*this, frame_headers()[frame_index]);
    if (!view.width || !view.height) {
        return;
    }
    const u32 dest_width = options.width ? options.width : view.width;
    const u32 dest_height = options.height ? options.height : view.height;
    const auto draw_pixel = [&](s32 x, s32 y, u8 color) {
        if (!color) {
            return; // pixel 0 is transparent
        }
        if (options.remap && color < options.remap_length) {
            color = options.remap[color];
        }
        bitmap.draw_dot(x, y, color);
    };

    // Nearest-neighbor sampling with the same integer math the cursor
    // scaler uses. Rows decode once into a buffer and are shared between
    // destination rows that land on the same source row.
    u8 row_pixels[MaxFrameWidth];
    u32 decoded_row = 0;
    bool have_row = false;
    for (u32 row = 0; row < dest_height; ++row) {
        const u32 source_row = row * view.height / dest_height;
        if (!have_row || source_row != decoded_row) {
            decode_row(view, options.format, source_row, row_pixels);
            decoded_row = source_row;
            have_row = true;
        }
        for (u32 col = 0; col < dest_width; ++col) {
            draw_pixel(options.x + s32(col), options.y + s32(row), row_pixels[col * view.width / dest_width]);
        }
    }
}

std::optional<UnitIcon> unit_icon(u16 unit_id) {
    const GrpHead* head = *DatExt::get_unit_icons_pointer();
    const GrpFile* grp = reinterpret_cast<const GrpFile*>(head);
    if (!grp || unit_id >= grp->frame_count()) {
        return std::nullopt;
    }
    return UnitIcon{grp, unit_id};
}

s32 unit_icon_advance(u16 unit_id, u32 pixel_height, s32 spacing) {
    const auto icon = unit_icon(unit_id);
    if (!icon) {
        return 0;
    }
    const FrameSize fitted = fit_height(icon->grp->frame_size(icon->frame), pixel_height);
    return fitted.height ? s32(fitted.width) + spacing : 0;
}

} // namespace graphics