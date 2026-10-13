#include "cursor_cache.h"

#include "../../BW/bwdata.h"
#include "display.h"

#include <algorithm>
#include <cmath>
#include <vector>

CursorCache& CursorCache::instance() {
    static CursorCache cache;
    return cache;
}

CursorCache::~CursorCache() {
    clear();
}

void CursorCache::clear() {
    for (const auto& [key, cursor] : m_cursors) {
        if (cursor) {
            DestroyIcon(cursor);
        }
    }
    m_cursors.clear();
}

HCURSOR CursorCache::get_cursor(const graphics::GrpFile* grp, u16 frame_index) {
    if (!grp || grp->frame_count() == 0) {
        return nullptr;
    }

    const u16 frame = frame_index % grp->frame_count();

    f32 scale = 1.0f;
    if (g_main_window) {
        RECT client = {};
        if (GetClientRect(g_main_window, &client)) {
            const s32 surface_w = s32(g_display.surface_width());
            const s32 surface_h = s32(g_display.surface_height());
            const s32 client_w = client.right - client.left;
            if (surface_w > 0 && surface_h > 0 && client_w > 0) {
                scale = f32(client_w) / f32(surface_w);
            }
        }
    }
    if (scale <= 0.1f || std::abs(scale - 1.0f) < 0.001f) {
        scale = 1.0f;
    }

    // Baked from g_target_palette rather than faded g_display_palette to avoid caching black during fades.
    if (m_palette != g_target_palette) {
        clear();
        m_palette = g_target_palette;
    }

    const u32 scale_fixed = u32(std::round(scale * 10000.0f));
    if (scale_fixed != m_scale_fixed) {
        clear();
        m_scale_fixed = scale_fixed;
    }

    const CursorKey key{ grp, frame, scale_fixed };

    const auto it = m_cursors.find(key);
    if (it != m_cursors.end()) {
        return it->second;
    }

    HCURSOR cursor = create_cursor(grp, frame, scale);
    m_cursors.emplace(key, cursor);
    return cursor;
}

HCURSOR CursorCache::create_cursor(const graphics::GrpFile* grp, u16 frame_index, f32 scale) {
    if (!grp || frame_index >= grp->frame_count()) {
        return nullptr;
    }

    const GrpFrame& frame = grp->frame_headers()[frame_index];
    if (frame.w == 0 || frame.h == 0 || frame.data_offset == 0) {
        return nullptr;
    }

    // 004D1560 rewrites frame data_offset into an absolute pointer as the GRP loads.
    const u8* grp_bytes = reinterpret_cast<const u8*>(grp);
    const u32 raw_offset = frame.data_offset & 0x7FFFFFFFu;
    const u8* frame_base = (raw_offset >= reinterpret_cast<uintptr_t>(grp_bytes))
        ? reinterpret_cast<const u8*>(raw_offset)
        : (grp_bytes + raw_offset);
    const u16* row_offsets = reinterpret_cast<const u16*>(frame_base);

    std::vector<u32> decoded_pixels(size_t(frame.w) * frame.h, 0);

    for (u16 row = 0; row < frame.h; ++row) {
        if (row_offsets[row] == 0) {
            continue;
        }

        const u8* stream = frame_base + row_offsets[row];
        u32* row_out = &decoded_pixels[size_t(row) * frame.w];
        u16 x = 0;

        while (x < frame.w) {
            const u8 opcode = *stream++;
            if (opcode & 0x80) {
                const u8 count = opcode & 0x7F;
                if (count == 0) {
                    break;
                }
                x = std::min<u16>(frame.w, x + count);
            } else if (opcode & 0x40) {
                const u8 count = opcode & 0x3F;
                if (count == 0) {
                    break;
                }
                const u8 color_index = *stream++;
                const u32 pixel = (color_index == 0) ? 0 : (0xFF000000u |
                    (u32(m_palette.colors[color_index].r) << 16) |
                    (u32(m_palette.colors[color_index].g) << 8) |
                    u32(m_palette.colors[color_index].b));

                for (u8 i = 0; i < count && x < frame.w; ++i) {
                    row_out[x++] = pixel;
                }
            } else {
                const u8 count = opcode;
                if (count == 0) {
                    break;
                }
                for (u8 i = 0; i < count; ++i) {
                    const u8 color_index = *stream++;
                    if (x < frame.w) {
                        const u32 pixel = (color_index == 0) ? 0 : (0xFF000000u |
                            (u32(m_palette.colors[color_index].r) << 16) |
                            (u32(m_palette.colors[color_index].g) << 8) |
                            u32(m_palette.colors[color_index].b));
                        row_out[x++] = pixel;
                    }
                }
            }
        }
    }

    const s32 hotspot_x = CursorHotspotBase - s32(frame.x);
    const s32 hotspot_y = CursorHotspotBase - s32(frame.y);

    s32 scaled_w = frame.w;
    s32 scaled_h = frame.h;
    s32 scaled_hotspot_x = hotspot_x;
    s32 scaled_hotspot_y = hotspot_y;
    const u32* final_pixels = decoded_pixels.data();
    std::vector<u32> scaled_pixel_storage;

    if (scale > 0.1f && std::abs(scale - 1.0f) >= 0.001f) {
        scaled_w = std::max(1, s32(std::round(f32(frame.w) * scale)));
        scaled_h = std::max(1, s32(std::round(f32(frame.h) * scale)));
        scaled_hotspot_x = s32(std::round(f32(hotspot_x) * scale));
        scaled_hotspot_y = s32(std::round(f32(hotspot_y) * scale));

        scaled_pixel_storage.resize(size_t(scaled_w) * scaled_h);
        for (s32 y = 0; y < scaled_h; ++y) {
            const s32 src_y = std::min(s32(frame.h) - 1, (y * s32(frame.h)) / scaled_h);
            for (s32 x = 0; x < scaled_w; ++x) {
                const s32 src_x = std::min(s32(frame.w) - 1, (x * s32(frame.w)) / scaled_w);
                scaled_pixel_storage[size_t(y) * scaled_w + x] = decoded_pixels[size_t(src_y) * frame.w + src_x];
            }
        }
        final_pixels = scaled_pixel_storage.data();
    }

    scaled_hotspot_x = std::clamp(scaled_hotspot_x, 0, std::max(0, scaled_w - 1));
    scaled_hotspot_y = std::clamp(scaled_hotspot_y, 0, std::max(0, scaled_h - 1));

    BITMAPV5HEADER header = {};
    header.bV5Size = sizeof(BITMAPV5HEADER);
    header.bV5Width = scaled_w;
    header.bV5Height = -scaled_h;
    header.bV5Planes = 1;
    header.bV5BitCount = 32;
    header.bV5Compression = BI_BITFIELDS;
    header.bV5RedMask = 0x00FF0000;
    header.bV5GreenMask = 0x0000FF00;
    header.bV5BlueMask = 0x000000FF;
    header.bV5AlphaMask = 0xFF000000;

    HDC screen_dc = GetDC(nullptr);
    void* dib_pixels = nullptr;
    HBITMAP color_bitmap = CreateDIBSection(screen_dc, reinterpret_cast<const BITMAPINFO*>(&header), DIB_RGB_COLORS, &dib_pixels, nullptr, 0);
    ReleaseDC(nullptr, screen_dc);

    if (!color_bitmap || !dib_pixels) {
        if (color_bitmap) {
            DeleteObject(color_bitmap);
        }
        return nullptr;
    }

    std::copy_n(final_pixels, size_t(scaled_w) * scaled_h, static_cast<u32*>(dib_pixels));

    const size_t row_stride = ((size_t(scaled_w) + 15) / 16) * 2;
    std::vector<u8> mask_bits(row_stride * scaled_h, 0);
    for (s32 y = 0; y < scaled_h; ++y) {
        for (s32 x = 0; x < scaled_w; ++x) {
            if ((final_pixels[size_t(y) * scaled_w + x] & 0xFF000000u) == 0) {
                mask_bits[size_t(y) * row_stride + (x / 8)] |= u8(0x80 >> (x % 8));
            }
        }
    }

    HBITMAP mask_bitmap = CreateBitmap(scaled_w, scaled_h, 1, 1, mask_bits.data());
    if (!mask_bitmap) {
        DeleteObject(color_bitmap);
        return nullptr;
    }

    ICONINFO icon_info = {};
    icon_info.fIcon = FALSE;
    icon_info.xHotspot = DWORD(scaled_hotspot_x);
    icon_info.yHotspot = DWORD(scaled_hotspot_y);
    icon_info.hbmMask = mask_bitmap;
    icon_info.hbmColor = color_bitmap;

    HCURSOR cursor = CreateIconIndirect(&icon_info);

    DeleteObject(color_bitmap);
    DeleteObject(mask_bitmap);

    return cursor;
}
