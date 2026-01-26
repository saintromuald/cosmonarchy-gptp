#pragma once

#include "structures.h"
#include "enumerations.h"
#include "color_palette.h"
#include "bwhelper.h"
#include "Storm.h"

#include <functional>
#include <map>
#include <concepts>
#include <string>
#include <initializer_list>
#include <vector>

template<typename T>
concept EnumS16 = std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, s16>;

template<typename T>
concept NotVoid = !std::is_void_v<T>;

std::vector<BinDlg*> get_children_by_index(BinDlg* parent, s16 index);
template<EnumS16 T>
std::vector<BinDlg*> get_children_by_index(BinDlg* parent, T index) {
    return get_children_by_index(parent, std::to_underlying(index));
}

BinDlg* get_first_child_by_index(BinDlg* parent, s16 index);
template<EnumS16 T>
BinDlg* get_first_child_by_index(BinDlg* parent, T index) {
    return get_first_child_by_index(parent, std::to_underlying(index));
}

void show_dialogs_with_index(BinDlg* dialog, s16 index);
template<EnumS16 T>
void show_dialogs_with_index(BinDlg* dialog, T index) {
    show_dialogs_with_index(dialog, std::to_underlying(index));
}

void hide_dialogs_with_index(BinDlg* dialog, s16 index);
template<EnumS16 T>
void hide_dialogs_with_index(BinDlg* dialog, T index) {
    hide_dialogs_with_index(dialog, std::to_underlying(index));
}

void update_dialog_visibility(BinDlg* dialog, s16 index, bool show);
template<EnumS16 T>
void update_dialog_visibility(BinDlg* dialog, T index, bool show) {
    update_dialog_visibility(dialog, std::to_underlying(index), show);
}

void enable_dialogs_with_index(BinDlg* dialog, s16 index);
template<EnumS16 T>
void enable_dialogs_with_index(BinDlg* dialog, T index) {
    enable_dialogs_with_index(dialog, std::to_underlying(index));
}

void disable_dialogs_with_index(BinDlg* dialog, s16 index);
template<EnumS16 T>
void disable_dialogs_with_index(BinDlg* dialog, T index) {
    disable_dialogs_with_index(dialog, std::to_underlying(index));
}

void update_dialog_text(BinDlg* dialog, s16 index, const char* text); // 004dcf30
template<EnumS16 T>
void update_dialog_text(BinDlg* dialog, T index, const char* text) {
    update_dialog_text(dialog, std::to_underlying(index), text);
}

template<EnumS16 T>
void bindlg_load_interaction_functions(BinDlg* dialog, std::map<T, BinDlgProc>& functions) {
    for (auto& [index, function] : functions) {
        auto children = get_children_by_index(dialog, index);
        for (auto child : children) {
            child->interaction_function = function;
        }
    }
};

s32 background_id(MenuId::Enum menu_id);

// Performs an Alloc
// Alternative to helper::alloc_background_image
void load_pcx_to_bitmap(const char* filename, Bitmap& bitmap, Palette* palette);
void bindlg_load_background_palette(BinDlg* dialog); // See dialog_swish_in 0x004de150
void bindlg_load_image(const char* filename, Palette* palette, BinDlg* dialog, u8* (__stdcall* malloc_func)(u32));
void bindlg_load_background(BinDlg* dialog);
void change_menu(bool force_refresh = false);
void initialize_dialog(BinDlg* dialog, BinDlgProc dialogproc); // 00419d20
void quick_send_event(BinDlg* dialog, DialogUserType user_type, u16 param = 0);

void show_bin_dialog(BinDlg* dialog);
void hide_bin_dialog(BinDlg* dialog);

template<EnumS16 T>
class BinDlgLabels {
public:
    BinDlgLabels(std::initializer_list<T> controls) {
        for (auto control : controls) {
            labels[control] = "";
        }
    }

    const char* get_label(T control) const {
        auto it = labels.find(control);
        return (it != labels.end()) ? it->second.c_str() : g_string_empty;
    }

    void set_label(T control, const std::string& text) {
        labels[control] = text;
    }

    void set_label(T control, const char* text) {
        labels[control] = text;
    }

    void update(BinDlg* dialog, T control, const char* text) {
        set_label(control, text);
        update_dialog_text(dialog, control, get_label(control));
    }

    void update(BinDlg* dialog, T control, const std::string& text) {
        update(dialog, control, text.c_str());
    }

private:
    std::map<T, std::string> labels;
};

class BinFile {
public:
	BinFile(const char* filename);
	BinDlg* bin_dlg() { return reinterpret_cast<BinDlg*>(m_buffer.data()); };

private:
	void load_file(const char* filename);

private:
	std::vector<u8> m_buffer;
};

// Only use this for bins whose dialogproc frees the allocated heap
template<typename T = void>
T* load_file_smemalloc(const char* filename, const char* log_filename = __FILE__, u32 log_line = __LINE__) {
    void* file_handle;
    if (!SFileOpenFileEx(0, filename, 0, &file_handle)) {
        helper::fatal_file_not_found(SErrGetLastError(), filename);
    }

    auto file_size = SFileGetFileSize(file_handle, 0);
    if (file_size == -1) {
        auto error = GetLastError();
        SFileCloseFile(file_handle);
        helper::file_fatal(&file_handle, error);
        return nullptr;
    }

    if (!file_size) {
        SFileCloseFile(file_handle);
        helper::fatal_file_not_found(ERROR_BAD_LENGTH, filename);
        return nullptr;
    }

    T* buffer = static_cast<T*>(SMemAlloc(file_size, log_filename, log_line, '\0'));

    if (!buffer) {
        SFileCloseFile(file_handle);
        return nullptr;
    }

    u32 bytes_read = 0;
    if (!SFileReadFile(file_handle, buffer, file_size, &bytes_read, 0)) {
        auto error = GetLastError();
        SFileCloseFile(file_handle);
        SMemFree(buffer, __FILE__, __LINE__, 0);
        if (error == ERROR_HANDLE_EOF) {
            helper::file_fatal(&file_handle, ERROR_BAD_LENGTH);
            return nullptr;
        }
        helper::file_fatal(&file_handle, error);
        return nullptr;
    }

    return buffer;
}

template<typename T = void>
class LoadedFile {
public:
    explicit LoadedFile(const char* filename) {
        load_file(filename);
    }

    LoadedFile(const LoadedFile&) = delete;
    LoadedFile& operator=(const LoadedFile&) = delete;

    LoadedFile(LoadedFile&& other) noexcept: m_buffer(std::move(other.m_buffer)) {}

    LoadedFile& operator=(LoadedFile&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
        }
        return *this;
    }

    T* data() { return reinterpret_cast<T*>(m_buffer.data()); }
    const T* data() const { return reinterpret_cast<const T*>(m_buffer.data()); }

    template<typename U = T>
    U& get() requires (!std::is_void_v<U>) {
        return *reinterpret_cast<U*>(m_buffer.data());
    }

    template<typename U = T>
    const U& get() const requires (!std::is_void_v<U>) {
        return *reinterpret_cast<const U*>(m_buffer.data());
    }

    size_t size() const { return m_buffer.size(); }

    template<typename U = T>
    size_t count() const requires std::is_array_v<U> {
        return m_buffer.size() / sizeof(std::remove_extent_t<U>);
    }

    bool empty() const { return m_buffer.empty(); }
    explicit operator bool() const { return !m_buffer.empty(); }

    T* operator->() { return data(); }
    const T* operator->() const { return data(); }

    template<typename U = T>
    U& operator*() requires (!std::is_void_v<U>) {
        return get<U>();
    }

    template<typename U = T>
    const U& operator*() const requires (!std::is_void_v<U>) {
        return get<U>();
    }

private:
    void load_file(const char* filename) {
        void* file_handle;
        if (!SFileOpenFileEx(0, filename, 0, &file_handle)) {
            helper::fatal_file_not_found(SErrGetLastError(), filename);
        }

        auto file_size = SFileGetFileSize(file_handle, 0);
        if (file_size == -1) {
            auto error = GetLastError();
            SFileCloseFile(file_handle);
            helper::file_fatal(&file_handle, error);
            return;
        }

        if (!file_size) {
            SFileCloseFile(file_handle);
            helper::fatal_file_not_found(ERROR_BAD_LENGTH, filename);
            return;
        }

        m_buffer.resize(file_size);

        u32 bytes_read = 0;
        if (!SFileReadFile(file_handle, m_buffer.data(), file_size, &bytes_read, 0)) {
            auto error = GetLastError();
            SFileCloseFile(file_handle);
            if (error == ERROR_HANDLE_EOF) {
                helper::file_fatal(&file_handle, ERROR_BAD_LENGTH);
                return;
            }
            helper::file_fatal(&file_handle, error);
            return;
        }

        SFileCloseFile(file_handle);

        if (bytes_read != file_size) {
            helper::file_fatal(&file_handle, ERROR_BAD_LENGTH);
        }
    }

private:
    std::vector<u8> m_buffer;
};