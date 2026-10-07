#include "translucent_image.h"

#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../BW/enumerations.h"
#include "../../graphics/Bitmap.h"
#include "../../systems/graphics/translucency.h"

#include <algorithm>
#include <cstddef>

namespace hooks {
namespace {

void translucent_blit(bw::Dialog* dlg, s32 source_x, s32 source_y, const Box16* clip) {
    graphics::Bitmap* surface = g_compositor_active_draw_target;
    if (!surface || !surface->data) return;

    const Bitmap& bitmap = dlg->bitmap;
    const s32 bitmap_width = bitmap.width;
    const s32 bitmap_height = bitmap.height;

    // Clip and offset math mirrored from 004EF440 and 004172F0 (render_bitmap).
    s32 dest_left = clip->left;
    s32 dest_top = clip->top;
    s32 src_x = source_x;
    s32 src_y = source_y;
    if (src_x < 0) {
        dest_left = clip->left - src_x;
        src_x = 0;
    }
    if (src_y < 0) {
        dest_top = clip->top - src_y;
        src_y = 0;
    }

    const s32 right_extent = std::min<s32>(clip->left + bitmap_width, clip->right + 1);
    const s32 bottom_extent = std::min<s32>(clip->top + bitmap_height, clip->bottom + 1);

    const s32 src_right = std::min((right_extent - dest_left) + 1 + src_x, bitmap_width);
    const s32 src_bottom = std::min((bottom_extent - dest_top) + 1 + src_y, bitmap_height);

    s32 copy_width = std::min(src_right - src_x, right_extent - dest_left);
    s32 copy_height = std::min(src_bottom - src_y, bottom_extent - dest_top);
    if (copy_width <= 0 || copy_height <= 0) return;

    // Clamp to surface bounds.
    const s32 pitch = surface->width;
    const s32 surface_height = surface->height;
    if (dest_left < 0) {
        const s32 delta = -dest_left;
        src_x += delta;
        dest_left = 0;
        copy_width -= delta;
    }
    if (dest_top < 0) {
        const s32 delta = -dest_top;
        src_y += delta;
        dest_top = 0;
        copy_height -= delta;
    }
    copy_width = std::min(copy_width, pitch - dest_left);
    copy_height = std::min(copy_height, surface_height - dest_top);
    if (copy_width <= 0 || copy_height <= 0) return;

    const u8* blend_row = graphics::active_translucency_row();
    const u8 blend_index = graphics::translucent_index();
    const u8* source_pixels = bitmap.buffer;
    u8* dest_pixels = surface->data;
    for (s32 row = 0; row < copy_height; ++row) {
        const u8* source_row = source_pixels + static_cast<size_t>(src_y + row) * bitmap_width + src_x;
        u8* dest_row = dest_pixels + static_cast<size_t>(dest_top + row) * pitch + dest_left;
        for (s32 col = 0; col < copy_width; ++col) {
            const u8 source_index = source_row[col];
            if (source_index == 0) continue; // transparent colorkey
            // 004172F0: blend the theme's translucent index, copy remainder opaque.
            dest_row[col] = source_index == blend_index
                ? blend_row[dest_row[col]]
                : source_index;
        }
    }
}

} // namespace

u32 __fastcall image_draw(bw::Dialog* dlg, s32 source_x, s32 source_y, const Box16* clip) {
    if (!dlg->bitmap.buffer) return 0;
    if (!(dlg->flags & DialogFlags::ApplyTranslucency)) {
        // Vanilla non-translucent path uses uncapped SBltROP3.
        helper::blit_bitmap_with_flags(clip, &dlg->bitmap, source_x, source_y, static_cast<u32>(dlg->flags));
        return 1;
    }
    translucent_blit(dlg, source_x, source_y, clip);
    return 1;
}

} // hooks
