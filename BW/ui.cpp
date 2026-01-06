#include "ui.h"
#include "snp.h"

std::vector<BinDlg*> get_children_by_index(BinDlg* parent, s16 index) {
    std::vector<BinDlg*> output;
    if (!parent) return output;

    if (parent->control_type != DialogControlTypes::DialogBoxDC) {
        parent = parent->parent;
        if (!parent) return output;
    }
    for (auto child = parent->child_dialog; child; child = child->next) {
        if (child->index == index) output.push_back(child);
    }
    return output;
}

void show_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) helper::show_dialog(child);
    }
}

void hide_dialogs_with_index(BinDlg* dialog, s16 index) {
    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (child) helper::hide_dialog(child);
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
    helper::alloc_init_dialog_data(bin_dlg(), helper::alloc_background_image, __FILE__, __LINE__);
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
    helper::set_callback_timer(dialog, 0x2e, 100, 0x004de140);
}

void bindlg_load_image(const char* filename, Palette* palette, BinDlg* dialog, u8* (__stdcall* malloc_func)(u32)) {
    u32 width{};
    u32 height{};
    if (!SBmpAllocLoadImage(filename, palette, &dialog->buffer, &width, &height, 0, 0, malloc_func)) {
        auto error = SErrGetLastError();
        helper::fatal_file_not_found(error, filename);
    }
    dialog->bounds.width = width;
    dialog->bounds.height = height;
}

// Ensure text remains a valid pointer for the life of the dialog
// Except DialogControlTypes::TextBox see: 004dcf30
void update_dialog_text(BinDlg* dialog, s16 index, const char* text) {
    if (!dialog || !text) return;

    auto children = get_children_by_index(dialog, index);
    for (auto child : children) {
        if (!child) continue;
        if (child->control_type == DialogControlTypes::TextBox) {
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
    }
    return background_id(MenuId::GlueSelConn);
}

GrpHead* load_graphic(const char* filename, u32* bytes_read, const char* log_filename, u32 log_line) {
    HANDLE file;
    auto result = SFileOpenFile(filename, &file);
    if (!result) {
        helper::fatal_file_not_found(SErrGetLastError(), filename);
    }

    auto file_size = SFileGetFileSize(file, 0);
    if (file_size == -1) {
        helper::file_fatal(&file, SErrGetLastError());
        return nullptr;
    }

    auto buffer = (GrpHead*)SMemAlloc(file_size, log_filename, log_line, 0);
    if (!buffer) {
        SFileCloseFile(file);
        return nullptr;
    }

    u32 bytes = 0;
    result = SFileReadFile(file, buffer, file_size, &bytes, 0);
    if (!result) {
        auto error = GetLastError();
        SFileCloseFile(file);
        SMemFree(buffer, __FILE__, __LINE__, 0);
        if (error == ERROR_HANDLE_EOF) helper::file_fatal(&file, ERROR_BAD_LENGTH);
        helper::file_fatal(&file, error);
        return nullptr;
    }
    if (bytes != file_size) {
        SFileCloseFile(file);
        SMemFree(buffer, log_filename, log_line, 0);
        helper::file_fatal(&file, ERROR_BAD_LENGTH);
        return nullptr;
    }

    if (bytes_read) *bytes_read = bytes;

    SFileCloseFile(file);
    return buffer;
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

void change_menu() {
    if (g_glue_background_id != background_id((MenuId::Enum)g_gl_glues_mode)) {
        if (g_background_buffer) {
            SMemFree(g_background_buffer, __FILE__, __LINE__, 0);
        }
        g_background_size = {};
        g_background_buffer = 0;
        g_background_palette = {};
        helper::glu_change_menu_palette(3);
        helper::refresh_cursor_0();
    }
    helper::refresh_region();

    g_menu_alloc_counter = 0;
}
