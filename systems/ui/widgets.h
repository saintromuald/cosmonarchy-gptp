#pragma once

#include "../BW/ui.h"
#include "palette_fader.h"

#include <concepts>
#include <functional>
#include <initializer_list>
#include <list>
#include <deque>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ui {
constexpr auto DefaultFadeMs = 400;

class BackgroundState {
public:
    static BackgroundState& instance() {
        static BackgroundState state;
        return state;
    }

    BackgroundState(const BackgroundState&) = delete;
    BackgroundState& operator=(const BackgroundState&) = delete;

    const std::string& current() const { return m_current; }
    void set_current(std::string path) { m_current = std::move(path); }
    const Palette& default_palette();

private:
    BackgroundState() = default;

    std::string m_current;
    Palette m_default_palette{};
    bool m_default_loaded = false;
};

enum class Align {
    Left,
    Right,
    Center
};

enum class FontSize {
    Small10,
    Medium14,
    Large16,
    Large16x,
};

enum class ActivateResult {
    ContinueWithDefault,
    ConsumeActivation,
};

class Group;

class Widget {
public:
    struct Callbacks {
        std::move_only_function<ActivateResult(Widget&, DialogEvent*)> on_nondlg_event;

        std::move_only_function<void(Widget&)>           on_create;
        std::move_only_function<void(Widget&)>           on_destroy;
        std::move_only_function<ActivateResult(Widget&)> on_activate;
        std::move_only_function<void(Widget&, u16)>      on_selection;
    };

    // Any copy or move would leave dangling pointers
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(Widget&&) = delete;

    // Builder Methods
    Widget* horizontal_align(Align alignment);
    // Todo: vertical_align
    Widget* fontsize(FontSize size);
    Widget* enable_hotkey();
    Widget* translucent(bool value = true);
    Widget* transparent(bool value = true);
    Widget* hide();
    Widget* show();
    Widget* disable();
    Widget* enable();
    Widget* set_default();
    Widget* set_cancel();

    // Callbacks
    Widget* on_create(std::move_only_function<void(Widget&)> callback);
    Widget* on_destroy(std::move_only_function<void(Widget&)> callback);
    Widget* on_activate(std::move_only_function<ActivateResult(Widget&)> callback);
    Widget* on_selection(std::move_only_function<void(Widget&, u16)> callback);
    Widget* on_nondlg_event(std::move_only_function<ActivateResult(Widget&, DialogEvent*)> callback);

    // Non-Builder methods
    void set_text(std::string text);
    const std::string& text() const { return m_text; };

    // Listbox / Combobox
    u8 add_item(std::string string);
    void clear_items();
    u8 selected_index() const;
    std::string_view list_string(u8 index);
    u8 count() const;

    // Optionbutton / Checkbox
    void select(bool value = true);

    u32 handle_event(DialogEvent* event);
    Callbacks& callbacks() { return m_callbacks; };
    bw::Dialog* dialog() { return &m_dialog; };
    u16 index() const { return m_dialog.index; };
    Group* parent() const { return m_group; };

private:
    friend class Menu;
    friend class Modal;
    friend class Group;

    Widget() = default;
    Widget(DialogBounds bounds, DialogControlType type, u16 index);

private:
    Group* m_group = nullptr;
    bw::Dialog m_dialog;
    std::string m_text; // Display text or image path
    std::deque<std::string> m_strings; // For listbox and combobox
    Callbacks m_callbacks;
    smem::ptr<u8> m_image_buffer;
};

class Group {
public:
    using Member = std::variant<std::unique_ptr<Widget>, std::unique_ptr<Group>>;

    // Any copy or move would leave dangling pointers
    Group(const Group&) = delete;
    Group& operator=(const Group&) = delete;
    Group(Group&&) = delete;
    Group& operator=(Group&&) = delete;

    Widget* button(DialogBounds bounds, std::string text);
    // Todo: imagebutton (load pcx or grp frame to dialog bitmap)
    Widget* lightup_button(DialogBounds bounds, std::string text);
    Widget* checkbox(DialogBounds bounds, std::string text, bool initial_value = false);
    Widget* image(DialogBounds bounds, std::string path);
    Widget* slider(DialogBounds bounds, u16 min, u16 max, u16 initial, bool vertical = false);
    Widget* textinput(DialogBounds bounds);
    Widget* label(DialogBounds bounds, std::string text);
    Widget* listbox(DialogBounds bounds);
    Widget* combobox(DialogBounds bounds);

    Group* group();
    // Todo: use Align instead of button_on_right
    Group* radio_group(DialogBounds bounds, std::initializer_list<std::string_view> options,
        u8 initial_selection = 0, bool button_on_right = false);

    // Builders broadcast to entire group
    Group* horizontal_align(Align alignment);
    Group* fontsize(FontSize size);
    Group* translucent(bool value = true);
    Group* transparent(bool value = true);
    Group* hide();
    Group* show();
    Group* disable();
    Group* enable();

    std::span<const Member> members() const { return m_members; };
    Group* parent() const { return m_parent; };
    Group* root()   const { return m_root; };

    virtual void on_top_dialog_init_children(Widget&) {}
    virtual void on_top_dialog_create       (Widget&) {}
    virtual void on_top_dialog_destroy      (Widget&) {}

protected:
    // Root Group constructor
    Group() : m_root(this), m_parent(nullptr) {}

    static std::unique_ptr<Widget> make(DialogBounds bounds, DialogControlType type, u16 index) {
        return std::unique_ptr<Widget>(new Widget(bounds, type, index));
    }

    void connect_child(Widget* child);
    u16 next_index() { return m_root->m_next_index++; };

    // The BW top level widget whose .child pointer heads the linked list
    std::unique_ptr<Widget> m_main;

private:
    // Subgroup constructor
    Group(Group* root, Group* parent) : m_root(root), m_parent(parent) {}

    Group* m_root;
    Group* m_parent;
    std::vector<Member> m_members;
    u16 m_next_index = 1;
};

class Menu : public Group {
public:
    Menu(std::string background = "", PalettePreserve instant_colors = PalettePreserve::None);

    // Any copy or move would leave dangling pointers
    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;
    Menu(Menu&&) = delete;
    Menu& operator=(Menu&&) = delete;

    virtual void on_init() {};
    virtual void on_exit(u32 last_control) {};

    u32 load();

    void change_background(std::string background,
        PalettePreserve preserve = PalettePreserve::FontShade | PalettePreserve::WidgetColors, u32 fade_ms = DefaultFadeMs);
    void fade_background_out(std::move_only_function<void()> on_complete = nullptr,
        PalettePreserve preserve = PalettePreserve::FontShade | PalettePreserve::WidgetColors, u32 fade_ms = DefaultFadeMs);
    void fade_background_in(std::string background = "",
        PalettePreserve preserve = PalettePreserve::FontShade | PalettePreserve::WidgetColors, u32 fade_ms = DefaultFadeMs);

protected:
    void on_top_dialog_init_children(Widget& top) override;
    void on_top_dialog_create      (Widget& top) override;

private:
    PalettePreserve m_instant_colors;
};

class Modal : public Group {
public:
    Modal(std::string background = "glue\\dynamic\\pDPopup.pcx");

    // Any copy or move would leave dangling pointers
    Modal(const Modal&) = delete;
    Modal& operator=(const Modal&) = delete;
    Modal(Modal&&) = delete;
    Modal& operator=(Modal&&) = delete;

    virtual void on_init() {};
    virtual void on_exit(u32 last_control) {};

    u32 load();

protected:
    void on_top_dialog_init_children(Widget& top) override;
    void on_top_dialog_create      (Widget& top) override;
    void on_top_dialog_destroy     (Widget& top) override;
};

bool modal_ok_cancel(std::string_view prompt);
void modal_ok(std::string_view prompt);
bool modal_string_prompt(std::string_view prompt, std::string& value);
} // namespace ui
