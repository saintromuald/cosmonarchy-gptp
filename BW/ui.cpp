#include "ui.h"

// For g_tfontgame_image_data
#include "hooks/interface/mainmenu/glucustom.h"
#include "storm/sfile.h"

std::vector<BinDlg*> get_children_by_index(BinDlg* parent, s16 index) {
    std::vector<BinDlg*> output;
    if (!parent) return output;

    if (parent->control_type != DialogControlType::DialogBoxDC) {
        parent = parent->parent;
        if (!parent) return output;
    }
    for (auto child = parent->child_dialog; child; child = child->next) {
        if (child->index == index) output.push_back(child);
    }
    return output;
}

BinDlg* get_first_child_by_index(BinDlg* parent, s16 index) {
    if (parent->control_type != DialogControlType::DialogBoxDC) {
        parent = parent->parent;
        if (!parent) return nullptr;
    }
    for (auto child = parent->child_dialog; child; child = child->next) {
        if (child->index == index) return child;
    }
    return nullptr;
}

void show_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) show_bin_dialog(child);
    }
}

void hide_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) hide_bin_dialog(child);
    }
}

void update_dialog_visibility(BinDlg* dialog, s16 index, bool show) {
    auto child = get_first_child_by_index(dialog, index);
    if (!child) return;
    if (show) {
        show_bin_dialog(child);
    } else {
        hide_bin_dialog(child);
    }
}

void enable_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) helper::enable_dialog(child);
    }
}

void disable_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) helper::disable_dialog(child);
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

    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (!child) continue;
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

void bindlg_load_background_pcx(BinDlg* dialog, const char* filename, bool apply_dynamic_theme) {
    if (dialog->bitmap.buffer) return;
    load_bitmap(filename, dialog->bitmap, &g_init_palette);
    g_background = dialog->bitmap;
    g_background_palette = g_init_palette;
    auto original_glues = g_gl_glues_mode;
    g_gl_glues_mode = g_gl_glues_mode >= MenuId::OriginalCount ? MenuId::GlueSelConn : g_gl_glues_mode;
    helper::register_menu_functions(0, dialog, 0, 0);
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

void initialize_dialog(BinDlg* dialog, BinDlgProc dialogproc) { // 00419d20
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

void quick_send_event(BinDlg* dialog, DialogUserType user_type, u16 param) {
    if (!dialog->interaction_function) return;
    DialogEvent event{
        .user = user_type,
        .param = param,
        .number_order = DialogEventType_Dialog,
        .current = {static_cast<u16>(g_mouse->x), static_cast<u16>(g_mouse->y)},
    };
    dialog->interaction_function(dialog, &event);
}

void show_bin_dialog(BinDlg* dialog) {
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

void hide_bin_dialog(BinDlg* dialog) {
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

smem::ptr<BinDlg> load_bin_file(const char* filename) {
    auto output = sfile::load_file(filename).unsafe_cast<BinDlg>();
    output.get()->flags |= DialogFlags::HeapAllocated;
    helper::fixup_and_init_dialog_data(output.get(), helper::alloc_background_image);
    return output;
}

std::vector<BinDlg*> splice_children_into(BinDlg* dest, BinDlg* source,
    s16 offset_x, s16 offset_y, s16 index_base, bool bounds_are_ltrb) {
    std::vector<BinDlg*> out;
    if (!dest || !source) return out;

    BinDlg* first = source->child_dialog;
    if (!first) return out;

    for (BinDlg* child = first; child; child = child->next) {
        child->parent = dest;
        if (bounds_are_ltrb) {
            child->bounds.left   += offset_x;
            child->bounds.right  += offset_x;
            child->bounds.top    += offset_y;
            child->bounds.bottom += offset_y;
        } else {
            child->bounds.x += offset_x;
            child->bounds.y += offset_y;
        }
        child->index = static_cast<u16>(child->index + index_base);
        out.push_back(child);
    }

    source->child_dialog = nullptr;

    if (dest->child_dialog) {
        BinDlg* tail = dest->child_dialog;
        while (tail->next) tail = tail->next;
        tail->next = first;
    } else {
        dest->child_dialog = first;
    }

    return out;
}

CompositeBinDlg::CompositeBinDlg(const char* main_filename)
    : m_main(load_bin_file(main_filename)) {
}

u32 CompositeBinDlg::add_tab(const char* tab_filename, s16 offset_x, s16 offset_y, s16 index_base) {
    auto tab_bin = load_bin_file(tab_filename);
    auto controls = splice_children_into(m_main.get(), tab_bin.get(),
        offset_x, offset_y, index_base, true);

    m_tab_bins.push_back(std::move(tab_bin));
    m_tabs.emplace_back(std::move(controls), index_base, offset_x, offset_y);
    return m_tabs.size() - 1;
}

void CompositeBinDlg::show_tab(u32 i) {
    if (i >= m_tabs.size()) return;
    for (auto* c : m_tabs[i].controls) show_bin_dialog(c);
}

void CompositeBinDlg::hide_tab(u32 i) {
    if (i >= m_tabs.size()) return;
    for (auto* c : m_tabs[i].controls) hide_bin_dialog(c);
}

void CompositeBinDlg::show_only(u32 i) {
    for (size_t t = 0; t < m_tabs.size(); ++t) {
        if (t == i) show_tab(t);
        else        hide_tab(t);
    }
}

DynamicTheme::DynamicTheme() {
    // Based on 0x004ddf90 load_cursor() - converts relative frame offsets to absolute pointers
    m_cursor_grp = sfile::load_file("glue\\dynamic\\arrow.grp").unsafe_cast<GrpFile>();

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
