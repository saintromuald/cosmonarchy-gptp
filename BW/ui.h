#pragma once

#include "structures.h"
#include "enumerations.h"
#include "color_palette.h"
#include "bwhelper.h"
#include "storm/storm.h"
#include "storm/sfile.h"
#include "storm/smem.hpp"

#include <functional>
#include <map>
#include <memory>
#include <concepts>
#include <string>
#include <initializer_list>
#include <vector>

constexpr u32 TFontCopyCount = 48;  // 6 font styles * 8 shades

// Loads an 8-bit PCX or BMP (auto-detected) into `bitmap`
// Allocs using SMemAlloc so either capture buffer in smem::ptr or ensure it's otherwise freed
bool load_bitmap(const char* filename, Bitmap& bitmap, Palette* palette = nullptr,
    SFileScope scope = SFileScope::DiskAndMpq, bool fatal_on_fail = true);

enum class GluTheme {
    PalMm, // Main Menu
    PalCs, // Campaign Select
    PalRt, // Ready Terran
    PalRz, // Ready Zerg
    PalRp, // Ready Protoss
    PalPd, // Protoss Defeat
    PalPv, // Protoss Victory
    PalZd, // Zerg Defeat
    PalZv, // Zerg Victory
    PalTd, // Terran Defeat
    PalTv, // Terran Victory
    PalNl, // Other Menus (Moon background)
};

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
void bindlg_load_interaction_functions(BinDlg* dialog, const std::map<T, BinDlgProc>& functions) {
    for (auto& [index, function] : functions) {
        auto children = get_children_by_index(dialog, index);
        for (auto child : children) {
            child->interaction_function = function;
        }
    }
};

s32 background_id(MenuId::Enum menu_id);

void bindlg_load_background_palette(BinDlg* dialog); // See dialog_swish_in 0x004de150
void bindlg_load_image(const char* filename, Palette* palette, BinDlg* dialog, u8* (__stdcall* malloc_func)(u32));
void bindlg_load_background(BinDlg* dialog);
MenuId::Enum menu_with_theme(GluTheme theme);
void bindlg_load_background_pcx(BinDlg* dialog, const char* filename, bool apply_dynamic_theme = false);

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

smem::ptr<BinDlg> load_bin_file(const char* filename);

std::vector<BinDlg*> splice_children_into(BinDlg* dest, BinDlg* source,
    s16 offset_x = 0, s16 offset_y = 0, s16 index_base = 0, bool bounds_are_ltrb = true);

// See usage in glu_options.cpp
// Make sure and add all tabs before calling glu_load_bin_dialog
class CompositeBinDlg {
public:
    struct Tab {
        std::vector<BinDlg*> controls;
        s16 index_base = 0;
        s16 offset_x = 0;
        s16 offset_y = 0;
    };

    explicit CompositeBinDlg(const char* main_filename);

    CompositeBinDlg(const CompositeBinDlg&) = delete;
    CompositeBinDlg& operator=(const CompositeBinDlg&) = delete;
    CompositeBinDlg(CompositeBinDlg&&) noexcept = default;
    CompositeBinDlg& operator=(CompositeBinDlg&&) noexcept = default;

    u32 add_tab(const char* tab_filename, s16 offset_x = 0, s16 offset_y = 0, s16 index_base = 0);

    BinDlg* bin_dlg() { return m_main.get(); }
    BinDlg* release() { return m_main.release(); }

    size_t tab_count() const { return m_tabs.size(); }
    const Tab& tab(u32 i) const { return m_tabs[i]; }

    void show_tab(u32 i);
    void hide_tab(u32 i);
    void show_only(u32 i);

private:
    smem::ptr<BinDlg> m_main;
    std::vector<smem::ptr<BinDlg>> m_tab_bins;
    std::vector<Tab> m_tabs;
};

class DynamicTheme {
public:
    static DynamicTheme& instance() {
        static DynamicTheme dynamic_theme;
        return dynamic_theme;
    }

    DynamicTheme(const DynamicTheme&) = delete;
    DynamicTheme(DynamicTheme&&) = delete;
    DynamicTheme& operator=(const DynamicTheme&) = delete;
    DynamicTheme& operator=(DynamicTheme&&) = delete;
    ~DynamicTheme();

    void apply();

private:
    DynamicTheme();
    DialogTheme* m_dialog_theme{};
    smem::ptr<GrpFile> m_cursor_grp;
    // Currently unsure what tile.grp is used for, if anything
    // Seems related to save games
    std::array<u8, TFontCopyCount> m_tfont_indices;
};
