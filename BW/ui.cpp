#include "ui.h"

// For g_tfontgame_image_data
#include "hooks/interface/mainmenu/glucustom.h"
#include "storm/sfile.h"

#include <algorithm>

bw::Dialog* get_first_child_by_index(bw::Dialog* parent, s16 index) {
    for (auto child : dialog_children(parent)) {
        if (child->index == index) return child;
    }
    return nullptr;
}

void show_dialogs_with_index(bw::Dialog* dialog, s16 index) {
    for (auto child : get_children_by_index(dialog, index)) {
        show_bin_dialog(child);
    }
}

void hide_dialogs_with_index(bw::Dialog* dialog, s16 index) {
    for (auto child : get_children_by_index(dialog, index)) {
        hide_bin_dialog(child);
    }
}

void update_dialog_visibility(bw::Dialog* dialog, s16 index, bool show) {
    auto child = get_first_child_by_index(dialog, index);
    if (!child) return;
    if (show) {
        show_bin_dialog(child);
    } else {
        hide_bin_dialog(child);
    }
}

void enable_dialogs_with_index(bw::Dialog* dialog, s16 index) {
    for (auto child : get_children_by_index(dialog, index)) {
        helper::enable_dialog(child);
    }
}

void disable_dialogs_with_index(bw::Dialog* dialog, s16 index) {
    for (auto child : get_children_by_index(dialog, index)) {
        helper::disable_dialog(child);
    }
}

void bindlg_load_background_palette(BinDlg* dialog) {
    s8 id = background_id(MenuId::Enum(g_gl_glues_mode));
    g_dialog_fade_info._unk1 = true;
    g_dialog_fade_info._unk2 = true;
    helper::bw_refresh(g_dialog_fade_info.mouse_y, g_dialog_fade_info.pointer_height - 1,
        g_dialog_fade_info.mouse_x, g_dialog_fade_info.pointer_width - 1);
    if (g_glue_background_id != id) {
        std::memcpy(game_palette, init_palette, sizeof(Palette));
        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
        helper::glu_update_palette(3);
        g_glue_background_id = id;
    }
    helper::set_callback_timer(dialog, 0x2e, 100, (TimerCallback)helper::show_queued_network_error);
}

void bindlg_load_image(const char* filename, Palette* palette, BinDlg* dialog, u8* (__stdcall* malloc_func)(u32)) {
    u32 width{};
    u32 height{};
    if (!SBmpAllocLoadImage(filename, palette, &dialog->bitmap.buffer, &width, &height, 0, 0, malloc_func)) {
        auto error = SErrGetLastError();
        helper::fatal_file_not_found(error, filename);
    }
    dialog->bitmap.width = width;
    dialog->bitmap.height = height;
}

// Ensure text remains a valid pointer for the life of the dialog
// Except DialogControlType::TextInput see: 004dcf30
void update_dialog_text(BinDlg* dialog, s16 index, const char* text) {
    if (!dialog || !text) return;

    for (auto child : get_children_by_index(dialog, index)) {
        if (child->control_type == DialogControlType::TextInput) {
            safe_copy(*child->textbox_buffer, text);
            child->TextBox0x36.unkFlags36 |= 1;
            if (!(child->flags & DialogFlags::Visible)) {
                continue;
            }
        } else {
            child->text = text;
        }
        if (child->flags & DialogFlags::UpdateNeeded) continue;
        child->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(child);
    }
}

s32 background_id(MenuId::Enum menu_id) {
    if (menu_id < MenuId::OriginalCount) {
        return g_menu_backgrounds[menu_id].background_id;
    } else if (menu_id == MenuId::GlueOptions) {
        return background_id(MenuId::GlueSelConn);
    } else if (menu_id == MenuId::GlueJoinCrownlink) {
        return background_id(MenuId::GlueGameSel);
    }
    return background_id(MenuId::GlueSelConn);
}

void bindlg_load_background(BinDlg* dialog) {
    // This function preps the background and palette in addition to registering functions
    // Instead use bindlg_load_interaction_functions for functions, but still call this one
    // for loading the background
    auto original_glues = g_gl_glues_mode;
    g_gl_glues_mode = g_gl_glues_mode > 25 ? 2 : g_gl_glues_mode; // TODO: Improve this
    helper::register_menu_functions(0, dialog, 0, 0);
    g_gl_glues_mode = original_glues;
}

MenuId::Enum menu_with_theme(GluTheme theme) {
    switch (theme) {
        case GluTheme::PalMm: return MenuId::GlueMainMenu;
        case GluTheme::PalCs: return MenuId::GlueExCampaign;
        case GluTheme::PalRt: return MenuId::GlueReadyT;
        case GluTheme::PalRz: return MenuId::GlueReadyZ;
        case GluTheme::PalRp: return MenuId::GlueReadyP;
        case GluTheme::PalPd: return MenuId::GlueScorePd;
        case GluTheme::PalPv: return MenuId::GlueScorePv;
        case GluTheme::PalZd: return MenuId::GlueScoreZd;
        case GluTheme::PalZv: return MenuId::GlueScoreZv;
        case GluTheme::PalTd: return MenuId::GlueScoreTd;
        case GluTheme::PalTv: return MenuId::GlueScoreTv;
        case GluTheme::PalNl: return MenuId::GlueChatroom;
    }
    return MenuId::GlueChatroom;
}

void bindlg_load_background_pcx(bw::Dialog* dialog, const char* filename, bool apply_dynamic_theme) {
    if (dialog->bitmap.buffer) return;
    load_bitmap(filename, dialog->bitmap, &g_init_palette);
    g_background = dialog->bitmap;
    g_background_palette = g_init_palette;
    auto original_glues = g_gl_glues_mode;
    g_gl_glues_mode = g_gl_glues_mode >= MenuId::OriginalCount ? MenuId::GlueSelConn : g_gl_glues_mode;
    helper::register_menu_functions(0, dialog->to_bindlg(), 0, 0);
    g_gl_glues_mode = original_glues;
    if (apply_dynamic_theme) {
        DynamicTheme::instance().apply();
    }
}

void change_menu(bool force_refresh) {
    if (force_refresh || g_glue_background_id != background_id((MenuId::Enum)g_gl_glues_mode)) {
        if (g_background.buffer) {
            SMemFree(g_background.buffer);
        }
        g_background = {};
        g_background_palette = {};
        helper::glu_change_menu_palette(3);
        helper::refresh_cursor_0();
    }
    helper::refresh_region();

    g_menu_z_index = 0;
}

void initialize_bin_dialog(BinDlg* dialog, BinDlgProc dialogproc) { // 00419d20
    dialog->flags |= (DialogFlags::DrawActiveState | DialogFlags::VerticalAlignmentBottom);
    dialog->onInitDialog0x46.unk_function = nullptr;
    if (dialogproc) {
        dialog->interaction_function = dialogproc;
    }

    const auto area_size = dialog->DialogBoxDC_0x36.bitmap.width * dialog->DialogBoxDC_0x36.bitmap.height;
    dialog->DialogBoxDC_0x36.bitmap.buffer = reinterpret_cast<u8*>(SMemAlloc(area_size));

    dialog->index = 0;

    quick_send_event(dialog, DialogUserType_Generic);
    
    // Convert bounds from x/y/w/h to left/top/right/bottom
    dialog->bounds.right += dialog->bounds.left;
    dialog->bounds.bottom += dialog->bounds.top;

    quick_send_event(dialog, DialogUserType_InitChildren);
    quick_send_event(dialog, DialogUserType_Create);

    helper::set_callback_timer(dialog, 12, 100, nullptr);

    dialog->flags &= ~DialogFlags::VerticalAlignmentBottom;
}

void initialize_widget_dialog(bw::Dialog* dialog, std::optional<bw::DialogInteraction*> interaction_function) { // 00419d20
    dialog->flags |= (DialogFlags::DrawActiveState | DialogFlags::VerticalAlignmentBottom);
    dialog->to_bindlg()->onInitDialog0x46.unk_function = nullptr; // TODO: Figure out what this is actually nulling and clean this up -Jesse
    if (interaction_function) {
        dialog->interaction_function = *interaction_function;
    }

    auto& area = dialog->data<bw::MenuData>()->backup_surface;
    area.buffer = reinterpret_cast<u8*>(SMemAlloc(area.width * area.height));

    dialog->index = 0;

    quick_send_event(dialog, DialogUserType_Generic);
    quick_send_event(dialog, DialogUserType_InitChildren);
    quick_send_event(dialog, DialogUserType_Create);

    helper::set_callback_timer(dialog->to_bindlg(), 12, 100, nullptr);

    dialog->flags &= ~DialogFlags::VerticalAlignmentBottom;
}

void quick_send_event(bw::Dialog* dialog, DialogUserType user_type, u16 param) {
    if (!dialog->interaction_function) return;
    DialogEvent event{
        .user = user_type,
        .param = param,
        .number_order = DialogEventType_Dialog,
        .current = {static_cast<u16>(g_mouse->x), static_cast<u16>(g_mouse->y)},
    };
    dialog->interaction_function(dialog, &event);
}

void show_bin_dialog(bw::Dialog* dialog) {
    if (!dialog) return;
    if (dialog->flags & DialogFlags::Visible) return;
    DialogEvent event{
        .user = DialogUserType_Show,
        .number_order = DialogEventType_Dialog,
        .current = {(u16)g_mouse->x, (u16)g_mouse->y}
    };
    auto result = dialog->interaction_function(dialog, &event);
    if (result && !(dialog->flags & DialogFlags::UpdateNeeded)) {
        dialog->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dialog);
    }
}

void hide_bin_dialog(bw::Dialog* dialog) {
    if (!dialog) return;
    if (!(dialog->flags & DialogFlags::Visible)) return;
    DialogEvent event{
        .user = DialogUserType_Hide,
        .number_order = DialogEventType_Dialog,
        .current = {(u16)g_mouse->x, (u16)g_mouse->y}
    };
    auto result = dialog->interaction_function(dialog, &event);
    if (result) {
        DialogEvent next_event{
            .user = DialogUserType_AcceptFocus,
            .number_order = DialogEventType_Dialog,
            .current = {(u16)g_mouse->x, (u16)g_mouse->y}
        };
        if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog);
        }
    }
}

smem::ptr<bw::Dialog> load_bin_file(const char* filename) {
    auto output = sfile::load_file(filename).unsafe_cast<bw::Dialog>();
    output.get()->flags |= DialogFlags::HeapAllocated;
    helper::fixup_and_init_dialog_data(output.get()->to_bindlg(), helper::alloc_background_image);
    return output;
}

namespace ui {

void scale_bin(bw::Dialog& dialog, const BinScale& options) {
    const auto scale = options.scale;
    const auto offset = options.offset;
    if (scale == 1 && offset == v2s16{} && !options.fullscreen) {
        return;
    }

    auto resize_surface = [](Bitmap& surface, u16 width, u16 height) {
        surface.width = width;
        surface.height = height;
        if (surface.buffer) {
            SMemFree(surface.buffer);
            const size_t area = size_t(width) * height;
            surface.buffer = reinterpret_cast<u8*>(SMemAlloc(area));
            std::fill_n(surface.buffer, area, 0);
        }
    };

    // Fullscreen pins the root to the screen, so `offset` shifts the scaled
    // children into place on it instead of translating the root.
    const auto child_offset = options.fullscreen ? offset : v2s16{};

    auto scale_node = [scale, child_offset, &resize_surface](bw::Dialog& node) {
        auto& bounds = node.bounds;
        bounds.set_ltrb(s16(bounds.left() * scale + child_offset.x), s16(bounds.top() * scale + child_offset.y),
            s16(bounds.right() * scale + child_offset.x), s16(bounds.bottom() * scale + child_offset.y));

        // LightUpButton animation bounds were offset by the (unscaled) bounds at
        // fixup; multiplying after keeps them affine-correct.
        if (node.type == DialogControlType::LightUpButton) {
            auto& extra = node.extra<bw::LightUpButtonExtra>()->bounds;
            extra.set_ltrb(s16(extra.left() * scale + child_offset.x), s16(extra.top() * scale + child_offset.y),
                s16(extra.right() * scale + child_offset.x), s16(extra.bottom() * scale + child_offset.y));

            // response_bounds is local-space, so it scales without the offset
            auto& response = node.data<bw::LightUpButtonData>()->response_bounds;
            response.set_ltrb(s16(response.left() * scale), s16(response.top() * scale),
                s16(response.right() * scale), s16(response.bottom() * scale));

            // Animation smk render positions are relative to the button's top-left, so
            // they scale without the offset; their sizes are probed from the files
            for (auto* animation = *node.child<bw::LightUpButtonAnimation*>(); animation; animation = animation->next) {
                animation->render_pos = {
                    u16(animation->render_pos.x * scale),
                    u16(animation->render_pos.y * scale),
                };
            }
        }

        if (node.type != DialogControlType::DialogBoxDC) {
            return;
        }

        auto& surface = node.data<bw::MenuData>()->backup_surface;
        resize_surface(surface, u16(surface.width * scale), u16(surface.height * scale));
    };

    if (options.fullscreen) {
        dialog.bounds = DialogBounds::from_xywh(0, 0, display::SurfaceWidth, display::SurfaceHeight);
        if (dialog.type == DialogControlType::DialogBoxDC) {
            resize_surface(dialog.data<bw::MenuData>()->backup_surface, display::SurfaceWidth, display::SurfaceHeight);
        }
    } else {
        scale_node(dialog);

        // Child bounds are relative to the root DialogBoxDC, so translating the
        // root carries the whole tree.
        dialog.bounds = dialog.bounds.offset(offset.x, offset.y);
    }

    if (dialog.type != DialogControlType::DialogBoxDC) {
        return;
    }

    for (auto* node = *dialog.child<bw::Dialog*>(); node; node = node->next) {
        scale_node(*node);
    }
}

} // ui

smem::ptr<bw::Dialog> load_bin_file_scaled(const char* filename, const ui::BinScale& options) {
    auto output = load_bin_file(filename);
    ui::scale_bin(*output.get(), options);
    return output;
}

DynamicTheme::DynamicTheme() {
    // Based on 0x004ddf90 load_cursor() - converts relative frame offsets to absolute pointers
    m_cursor_grp = sfile::load_file("glue\\dynamic\\arrow.grp").unsafe_cast<graphics::GrpFile>();

    u8* data = reinterpret_cast<u8*>(m_cursor_grp.get());
    u16 frame_count = *reinterpret_cast<u16*>(data) & 0x7fff;
    for (u16 i = 0; i < frame_count; ++i) {
        u32* data_off = reinterpret_cast<u32*>(data + 6 + i * 8 + 4);
        *data_off = reinterpret_cast<u32>(data) + (*data_off & 0x7fffffff);
    }
    Bitmap bitmap{};
    load_bitmap("glue\\dynamic\\tfont.pcx", bitmap, nullptr);
    if (bitmap.width * bitmap.height != TFontCopyCount) {
        helper::fatal_file_not_found(SErrGetLastError(), "glue\\dynamic\\tfont.pcx");
    }
    std::copy_n(bitmap.buffer, TFontCopyCount, m_tfont_indices.begin());
    SMemFree(bitmap.buffer);
}

DynamicTheme::~DynamicTheme() {
    if (m_dialog_theme && m_dialog_theme->grp_handle) {
        SMemFree(m_dialog_theme->grp_handle);
        m_dialog_theme->grp_handle = nullptr;
    }
    if (m_dialog_theme) {
        SMemFree(m_dialog_theme);
    }
}

void DynamicTheme::apply() {
    if (!m_dialog_theme) {
        m_dialog_theme = helper::dialog_grp_construct("glue\\dynamic\\dlg.grp", __FILE__, __LINE__);
    } else {
        helper::apply_dialog_theme(m_dialog_theme);
    }

    if (g_current_cursor != m_cursor_grp.get()) {
        g_current_cursor = m_cursor_grp.get();
        helper::refresh_cursor();
    }

    std::copy_n(m_tfont_indices.begin(), TFontCopyCount, g_tfontgame_image_data);
}

bool load_bitmap(const char* filename, Bitmap& bitmap,
    Palette* palette, SFileScope scope, bool fatal_on_fail) {
    auto file = fatal_on_fail
        ? sfile::load_file(filename, scope)
        : sfile::try_load_file(filename, scope);
    if (!file) return false;

    // Query dimensions and format.
    u32 width = 0, height = 0, bits_per_pixel = 0;
    if (!SBmpDecodeImage(0, file.data.get(), file.size, nullptr, nullptr, 0,
        &width, &height, &bits_per_pixel)) {

        if (fatal_on_fail) helper::fatal_file_not_found(sfile::ErrorInvalidData, filename);
        return false;
    }

    if (bits_per_pixel != 8) {
        if (fatal_on_fail) helper::fatal_file_not_found(sfile::ErrorInvalidData, filename);
        return false;
    }

    const u32 pixel_buffer_size = width * height;
    u8* pixels = static_cast<u8*>(SMemAlloc(pixel_buffer_size));
    if (!pixels) {
        if (fatal_on_fail) helper::fatal_file_not_found(sfile::ErrorInvalidData, filename);
        return false;
    }

    if (!SBmpDecodeImage(0, file.data.get(), file.size, palette,
        pixels, pixel_buffer_size, nullptr, nullptr, nullptr)) {

        SMemFree(pixels);
        if (fatal_on_fail) helper::fatal_file_not_found(sfile::ErrorInvalidData, filename);
        return false;
    }

    bitmap.width  = static_cast<u16>(width);
    bitmap.height = static_cast<u16>(height);
    bitmap.buffer = pixels;
    return true;
}
