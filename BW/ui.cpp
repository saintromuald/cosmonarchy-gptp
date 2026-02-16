#include "ui.h"

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

BinFile::BinFile(const char* filename) {
    load_file(filename);
}

void BinFile::load_file(const char* filename) {
    HANDLE file_handle;
    if (!SFileOpenFileEx(0, filename, 0, &file_handle)) {
        auto error = SErrGetLastError();
        helper::fatal_file_not_found(error, filename);
    }

    auto file_size = SFileGetFileSize(file_handle, 0);
    if (file_size == -1) {
        auto error = GetLastError();
        helper::file_fatal(&file_handle, error);
    }

    if (!file_size) {
        helper::fatal_file_not_found(ERROR_BAD_LENGTH, filename);
    }

    m_buffer.resize(file_size);

    u32 bytes_read = 0;
    if (!SFileReadFile(file_handle, m_buffer.data(), file_size, &bytes_read, 0)) {
        auto error = GetLastError();
        if (error == ERROR_HANDLE_EOF) {
            helper::file_fatal(&file_handle, ERROR_BAD_LENGTH);
        }
        helper::file_fatal(&file_handle, error);
    }

    if (bytes_read != file_size) {
        helper::file_fatal(&file_handle, ERROR_BAD_LENGTH);
    }

    SFileCloseFile(file_handle);
    
    bin_dlg()->flags |= 4;
    helper::fixup_and_init_dialog_data(bin_dlg(), helper::alloc_background_image, __FILE__, __LINE__);
}

void bindlg_load_background_palette(BinDlg* dialog) {
    s8 id = background_id(MenuId::Enum(g_gl_glues_mode));
    g_dialog_fade_info._unk1 = true;
    g_dialog_fade_info._unk2 = true;
    helper::bw_refresh(g_dialog_fade_info.mouse_y, g_dialog_fade_info.pointer_height - 1,
        g_dialog_fade_info.mouse_x, g_dialog_fade_info.pointer_width - 1);
    if (g_glue_background_id != id) {
        std::memcpy(game_palette, init_palette, sizeof(Palette));
        if (!(dialog->flags & 1)) {
            dialog->flags |= 1;
            helper::update_dialog(dialog);
        }
        helper::glu_update_palette(3);
        g_glue_background_id = id;
    }
    helper::set_callback_timer(dialog, 0x2e, 100, (TimerCallback)0x004de140);
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
// Except DialogControlType::TextBox see: 004dcf30
void update_dialog_text(BinDlg* dialog, s16 index, const char* text) {
    if (!dialog || !text) return;

    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (!child) continue;
        if (child->control_type == DialogControlType::TextBox) {
            safe_copy(*child->textbox_buffer, text);
            child->TextBox0x36.unkFlags36 |= 1;
            if (!(child->flags & BinDlgFlags::Visible)) {
                continue;
            }
        } else {
            child->text = text;
        }
        if (child->flags & BinDlgFlags::Highlighted) continue;
        child->flags |= BinDlgFlags::Highlighted;
        helper::update_dialog(child);
    }
}

s32 background_id(MenuId::Enum menu_id) {
    if (menu_id < 27) {
        return g_menu_backgrounds[menu_id].background_id;
    } else if (menu_id == MenuId::GlueOptions) {
        return background_id(MenuId::GlueSelConn);
    } else if (menu_id == MenuId::GlueJoinCrownlink) {
        return background_id(MenuId::GlueGameSel);
    }
    return background_id(MenuId::GlueSelConn);
}

void load_pcx_to_bitmap(const char* filename, Bitmap& bitmap, Palette* palette) {
    u32 width, height;
    if (!SBmpAllocLoadImage(filename, palette, &bitmap.buffer, &width, &height, 0, 0, helper::generic_malloc_wrapper)) {
        helper::fatal_file_not_found(SErrGetLastError(), filename);
    }
    bitmap.width = width;
    bitmap.height = height;
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

void bindlg_load_background(BinDlg* dialog, const char* filename) {
    if (dialog->bitmap.buffer) return;
    load_pcx_to_bitmap(filename, dialog->bitmap, &g_init_palette);
    g_background = dialog->bitmap;
    g_background_palette = g_init_palette;
    helper::register_menu_functions(0, dialog, 0, 0);
}

void change_menu(bool force_refresh) {
    if (force_refresh || g_glue_background_id != background_id((MenuId::Enum)g_gl_glues_mode)) {
        if (g_background.buffer) {
            SMemFree(g_background.buffer, __FILE__, __LINE__, 0);
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
    dialog->flags |= (BinDlgFlags::NoClickSound | BinDlgFlags::VerticalAlignmentBottom);
    dialog->onInitDialog0x46.unk_function = nullptr;
    if (dialogproc) {
        dialog->interaction_function = dialogproc;
    }

    const auto area_size = dialog->DialogBoxDC_0x36.bitmap.width * dialog->DialogBoxDC_0x36.bitmap.height;
    dialog->DialogBoxDC_0x36.bitmap.buffer = (u8*)SMemAlloc(area_size, __FILE__, __LINE__, 0);

    dialog->index = 0;

    quick_send_event(dialog, DialogUserType_Generic);
    
    // Convert bounds from x/y/w/h to left/top/right/bottom
    dialog->bounds.right += dialog->bounds.left;
    dialog->bounds.bottom += dialog->bounds.top;

    quick_send_event(dialog, DialogUserType_InitChildren);
    quick_send_event(dialog, DialogUserType_Create);

    helper::set_callback_timer(dialog, 12, 100, nullptr);

    dialog->flags &= ~BinDlgFlags::VerticalAlignmentBottom;
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
    if (dialog->flags & BinDlgFlags::Visible) return;
    DialogEvent event{
        .user = DialogUserType_Show,
        .number_order = DialogEventType_Dialog,
        .current = {(u16)g_mouse->x, (u16)g_mouse->y}
    };
    auto result = dialog->interaction_function(dialog, &event);
    if (result && (dialog->flags & BinDlgFlags::Highlighted) == 0) {
        dialog->flags |= BinDlgFlags::Highlighted;
        helper::update_dialog(dialog);
    }
}

void hide_bin_dialog(BinDlg* dialog) {
    if (!dialog) return;
    if ((dialog->flags & BinDlgFlags::Visible) == 0) return;
    DialogEvent event{
        .user = DialogUserType_Hide,
        .number_order = DialogEventType_Dialog,
        .current = {(u16)g_mouse->x, (u16)g_mouse->y}
    };
    auto result = dialog->interaction_function(dialog, &event);
    if (result) {
        DialogEvent next_event{
            .user = DialogUserType_SelectNext,
            .number_order = DialogEventType_Dialog,
            .current = {(u16)g_mouse->x, (u16)g_mouse->y}
        };
        if ((dialog->flags & BinDlgFlags::Highlighted) == 0) {
            dialog->flags |= BinDlgFlags::Highlighted;
            helper::update_dialog(dialog);
        }
    }
}
