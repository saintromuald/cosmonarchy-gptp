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
#include <optional>
#include <string>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <vector>

constexpr u32 TFontCopyCount = 48;  // 6 font styles * 8 shades
constexpr u16 ScreenWidth = 640;
constexpr u16 ScreenHeight = 480;

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

template<typename T>
concept LinkedListNode = requires(T* node) {
    { node->next } -> std::convertible_to<T*>;
};

template<LinkedListNode T>
class linked_list_view : public std::ranges::view_interface<linked_list_view<T>> {
public:
    class iterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = T*;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        explicit iterator(T* node) : node_(node) {}

        T* operator*() const { return node_; }
        iterator& operator++() { node_ = node_->next; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }

        bool operator==(const iterator& other) const = default;
        bool operator==(std::default_sentinel_t) const { return node_ == nullptr; }

    private:
        T* node_ = nullptr;
    };

    linked_list_view() = default;
    explicit linked_list_view(T* head) : head_(head) {}

    iterator begin() const { return iterator(head_); }
    std::default_sentinel_t end() const { return {}; }

private:
    T* head_ = nullptr;
};

inline linked_list_view<bw::Dialog> dialog_children(bw::Dialog* parent) {
    if (!parent) return {};
    if (parent->type != DialogControlType::DialogBoxDC) {
        parent = parent->parent;
        if (!parent) return {};
    }
    return linked_list_view<bw::Dialog>(*parent->child<bw::Dialog*>());
}
inline linked_list_view<BinDlg> dialog_children(BinDlg* parent) {
    if (!parent) return {};
    if (parent->control_type != DialogControlType::DialogBoxDC) {
        parent = parent->parent;
        if (!parent) return {};
    }
    return linked_list_view<BinDlg>(parent->child_dialog);
}

inline auto get_children_by_index(bw::Dialog* parent, s16 index) {
    return dialog_children(parent) | std::views::filter([index](bw::Dialog* child) { return child->index == index; });
}

inline auto get_children_by_index(BinDlg* parent, s16 index) {
    return dialog_children(parent) | std::views::filter([index](BinDlg* child) { return child->index == index; });
}

template<EnumS16 T>
auto get_children_by_index(bw::Dialog* parent, T index) {
    return get_children_by_index(parent, std::to_underlying(index));
}

template<EnumS16 T>
auto get_children_by_index(BinDlg* parent, T index) {
    return get_children_by_index(parent, std::to_underlying(index));
}

bw::Dialog* get_first_child_by_index(bw::Dialog* parent, s16 index);
inline BinDlg* get_first_child_by_index(BinDlg* parent, s16 index) { return get_first_child_by_index(parent->to_dialog(), index)->to_bindlg(); }

template<EnumS16 T>
bw::Dialog* get_first_child_by_index(bw::Dialog* parent, T index) {
    return get_first_child_by_index(parent, std::to_underlying(index));
}

template<EnumS16 T>
BinDlg* get_first_child_by_index(BinDlg* parent, T index) {
    return get_first_child_by_index(parent, std::to_underlying(index));
}

void show_dialogs_with_index(bw::Dialog* dialog, s16 index);
inline void show_dialogs_with_index(BinDlg* dialog, s16 index) { show_dialogs_with_index(dialog->to_dialog(), index); };

template<EnumS16 T>
void show_dialogs_with_index(bw::Dialog* dialog, T index) {
    show_dialogs_with_index(dialog, std::to_underlying(index));
}

template<EnumS16 T>
void show_dialogs_with_index(BinDlg* dialog, T index) {
    show_dialogs_with_index(dialog, std::to_underlying(index));
}

void hide_dialogs_with_index(bw::Dialog* dialog, s16 index);
inline void hide_dialogs_with_index(BinDlg* dialog, s16 index) { hide_dialogs_with_index(dialog->to_dialog(), index); }

template<EnumS16 T>
void hide_dialogs_with_index(bw::Dialog* dialog, T index) {
    hide_dialogs_with_index(dialog, std::to_underlying(index));
}

template<EnumS16 T>
void hide_dialogs_with_index(BinDlg* dialog, T index) {
    hide_dialogs_with_index(dialog, std::to_underlying(index));
}

void update_dialog_visibility(bw::Dialog* dialog, s16 index, bool show);
inline void update_dialog_visibility(BinDlg* dialog, s16 index, bool show) { update_dialog_visibility(dialog->to_dialog(), index, show); }

template<EnumS16 T>
void update_dialog_visibility(bw::Dialog* dialog, T index, bool show) {
    update_dialog_visibility(dialog, std::to_underlying(index), show);
}

template<EnumS16 T>
void update_dialog_visibility(BinDlg* dialog, T index, bool show) {
    update_dialog_visibility(dialog, std::to_underlying(index), show);
}

void enable_dialogs_with_index(bw::Dialog* dialog, s16 index);
inline void enable_dialogs_with_index(BinDlg* dialog, s16 index) { enable_dialogs_with_index(dialog->to_dialog(), index); }

template<EnumS16 T>
void enable_dialogs_with_index(bw::Dialog* dialog, T index) {
    enable_dialogs_with_index(dialog, std::to_underlying(index));
}
template<EnumS16 T>
void enable_dialogs_with_index(BinDlg* dialog, T index) {
    enable_dialogs_with_index(dialog, std::to_underlying(index));
}

void disable_dialogs_with_index(bw::Dialog* dialog, s16 index);
inline void disable_dialogs_with_index(BinDlg* dialog, s16 index) { disable_dialogs_with_index(dialog->to_dialog(), index); }

template<EnumS16 T>
void disable_dialogs_with_index(bw::Dialog* dialog, T index) {
    disable_dialogs_with_index(dialog, std::to_underlying(index));
}

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
void bindlg_load_interaction_functions(bw::Dialog* dialog, const std::map<T, bw::DialogInteraction*>& functions) {
    for (auto& [index, function] : functions) {
        auto children = get_children_by_index(dialog, index);
        for (bw::Dialog* child : children) {
            child->interaction_function = function;
        }
    }
};

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

void bindlg_load_background_pcx(bw::Dialog* dialog, const char* filename, bool apply_dynamic_theme = false);

void change_menu(bool force_refresh = false);

// For dialogs loaded from a .bin file
void initialize_bin_dialog(BinDlg* dialog, BinDlgProc dialogproc);
inline void initialize_bin_dialog(bw::Dialog* dialog, bw::DialogInteraction* proc) {
    initialize_bin_dialog(dialog->to_bindlg(), reinterpret_cast<BinDlgProc>(proc));
}

// For dialogs built with the widget system: bounds are already in absolute
void initialize_widget_dialog(bw::Dialog* dialog, std::optional<bw::DialogInteraction*> interaction_function = std::nullopt);

void quick_send_event(bw::Dialog* dialog, DialogUserType user_type, u16 param = 0);
inline void quick_send_event(BinDlg* dialog, DialogUserType user_type, u16 param = 0) {
    quick_send_event(dialog->to_dialog(), user_type, param);
}

void show_bin_dialog(bw::Dialog* dialog);
inline void show_bin_dialog(BinDlg* dialog) { show_bin_dialog(dialog->to_dialog()); };

void hide_bin_dialog(bw::Dialog* dialog);
inline void hide_bin_dialog(BinDlg* dialog) { hide_bin_dialog(dialog->to_dialog()); };

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

smem::ptr<bw::Dialog> load_bin_file(const char* filename);

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
