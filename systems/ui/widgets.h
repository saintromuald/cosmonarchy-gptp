#pragma once

#include "../BW/ui.h"
#include "palette_fader.h"
#include "tooltip.h"

#include <concepts>
#include <format>
#include <functional>
#include <initializer_list>
#include <deque>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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

// common_process_mouseover @ 0x00418030 skips hit-testing dialogs with this index
constexpr u16 SkipHitTestIndex = 0xFFFF;

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
        std::move_only_function<void(Widget&, u8, bool)> on_check_toggle;
        std::move_only_function<void(Widget&, u8)>       on_row_hover;
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
    Widget* enable_hotkey(std::optional<char> hotkey = {});
    Widget* translucent(bool value = true);
    Widget* transparent(bool value = true);
    Widget* hide();
    Widget* show();
    Widget* disable();
    Widget* enable();
    Widget* set_default();
    Widget* set_cancel();
    Widget* row_padding(u8 extra_pixels);

    // Callbacks
    Widget* on_create(std::move_only_function<void(Widget&)> callback);
    Widget* on_destroy(std::move_only_function<void(Widget&)> callback);
    Widget* on_activate(std::move_only_function<ActivateResult(Widget&)> callback);
    Widget* on_selection(std::move_only_function<void(Widget&, u16)> callback);
    Widget* on_nondlg_event(std::move_only_function<ActivateResult(Widget&, DialogEvent*)> callback);
    Widget* on_check_toggle(std::move_only_function<void(Widget&, u8, bool)> callback);
    Widget* on_row_hover(std::move_only_function<void(Widget&, u8)> callback);

    // Custom draw function replacing the control type's default draw. The callback
    // runs in the dialog repaint pass (0x0041C080): the target is the parent
    // container's surface and coordinates are parent-relative, matching the
    // widget's bounds - the same context bw_print_xy / draw_box already use.
    Widget* on_draw(std::move_only_function<void(Widget&)> callback);

    // Non-Builder methods
    void set_text(std::string text);
    const std::string& text() const { return m_text; };

    // Listbox / Combobox
    u8 add_item(std::string string);
    void clear_items();
    u8 selected_index() const;
    void select_row(u8 index);   // Highlight a listbox/combobox row (sets selection_index)
    void clear_selection();      // Remove any row highlight (selection_index = -1)
    std::string_view list_string(u8 index);
    u8 count() const;

    // Optionbutton / Checkbox
    void select(bool value = true);

    // Checked listbox row state (bit 4 of flags_array[index])
    void set_checked(u8 index, bool checked);
    bool is_checked(u8 index) const;
    void select_all();
    void deselect_all();
    // Scroll a listbox/combobox by `rows` (negative = up). Clamps to bounds.
    // Also syncs the scrollbar's handle if one exists.
    void scroll_by(s32 rows);

    Widget* tooltip(std::string text);
    Widget* tooltip(Tooltip content);
    template <typename F>
        requires std::invocable<F, Tooltip&>
    Widget* tooltip(F&& builder) {
        Tooltip tt;
        std::forward<F>(builder)(tt);
        return tooltip(std::move(tt));
    }

    Widget* tooltip_delay_ms(u16 ms);
    Widget* tooltip_placement(Placement placement);
    Widget* tooltip_hide_on_click(bool value = true);

    // Accessors for the TooltipController
    const Tooltip* tooltip_content() const;
    u16 tooltip_delay_ms() const { return m_tooltip_delay_ms; }
    Placement tooltip_placement() const { return m_tooltip_placement; }
    bool tooltip_hides_on_click() const { return m_tooltip_hides_on_click; }

    u32 handle_event(DialogEvent* event);
    Callbacks& callbacks() { return m_callbacks; };
    bw::Dialog* dialog() { return &m_dialog; };
    const bw::Dialog* dialog() const { return &m_dialog; };
    u16 index() const { return m_dialog.index; };
    Group* parent() const { return m_group; };

    // Relative to screen-space helpers
    v2s16 screen_top_left() const;
    DialogBounds screen_bounds() const;
    void set_screen_bounds(DialogBounds area);

    // Mark this widget dirty so the compositor calls our draw fn again
    // Use when updating a widget
    void invalidate(bool force_refresh = false);
    // Mark the parent dialog dirty so the compositor redraws this widget's area
    // Use when hiding or moving a widget
    void invalidate_parent(bool force_refresh = false);
private:
    friend class Menu;
    friend class Modal;
    friend class Hud;
    friend class Group;

    Widget() = default;
    Widget(DialogBounds bounds, DialogControlType type, u16 index);

    static void __fastcall draw_forward(bw::Dialog* dialog, DialogEvent* event, u32 user, void* draw_context);

private:
    Group* m_group = nullptr;
    bw::Dialog m_dialog;
    std::string m_text; // Display text or image path
    std::deque<std::string> m_strings; // For listbox and combobox
    Callbacks m_callbacks;
    std::move_only_function<void(Widget&)> m_draw_callback; // Replaces the default draw when set
    u8 m_row_padding = 0; // Extra per-row pixels for listbox/combobox
    std::unique_ptr<Tooltip> m_tooltip; // Optional; nullptr = no tooltip
    u16 m_tooltip_delay_ms = 300;
    Placement m_tooltip_placement = Placement::Auto;
    bool m_tooltip_hides_on_click = true;
    u8 m_last_hovered_row = 0xFF; // Used by on_row_hover state machine
};

class Group {
    friend class DialogTooltipSurface;
public:
    using Member = std::variant<std::unique_ptr<Widget>, std::unique_ptr<Group>>;

    virtual ~Group() = default;

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
    template <typename... Args>
    Widget* label(DialogBounds bounds, std::format_string<Args...> fmt, Args&&... args) {
        return label(bounds, std::format(fmt, std::forward<Args>(args)...));
    }

    Widget* listbox(DialogBounds bounds);
    Widget* combobox(DialogBounds bounds);
    // Listbox variant where each row has a leading checkbox
    Widget* checked_listbox(DialogBounds bounds);

    Group* group();
    Group* radio_group(DialogBounds bounds, std::span<const std::string_view> options,
        u8 initial_selection = 0, Align alignment = Align::Left);
    Group* on_radio_selection(std::move_only_function<void(u8)> callback);

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

    // Return true to pass mouse events that don't hit a child widget through to the game
    virtual bool passthrough_mouse() const { return false; }

    // Return true to pass keyboard events through to the game instead of consuming them
    virtual bool passthrough_keyboard() const { return false; }

    virtual TooltipController* tooltip_controller() { return nullptr; }

protected:
    Group() : m_root(this), m_parent(nullptr) {}

    static std::unique_ptr<Widget> make(DialogBounds bounds, DialogControlType type, u16 index) {
        return std::unique_ptr<Widget>(new Widget(bounds, type, index));
    }

    void connect_child(Widget* child);
    u16 next_index() { return m_root->m_next_index++; };
    // For tooltips
    void attach_external_widget(Widget* w);

    // The BW top level widget whose child pointer heads the linked list
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

    TooltipController* tooltip_controller() override { return &m_tooltip_controller; }

protected:
    void on_top_dialog_init_children(Widget& top) override;
    void on_top_dialog_create      (Widget& top) override;

private:
    PalettePreserve m_instant_colors;
    TooltipController m_tooltip_controller;
};

class Modal : public Group {
public:
    Modal(std::string background = "glue\\dynamic\\pDPopup.pcx");

    // Any copy or move would leave dangling pointers
    Modal(const Modal&) = delete;
    Modal& operator=(const Modal&) = delete;
    Modal(Modal&&) = delete;
    Modal& operator=(Modal&&) = delete;

    TooltipController* tooltip_controller() override { return &m_tooltip_controller; }

    virtual void on_init() {};
    virtual void on_exit(u32 last_control) {};

    u32 load();

protected:
    void on_top_dialog_init_children(Widget& top) override;
    void on_top_dialog_create      (Widget& top) override;
    void on_top_dialog_destroy     (Widget& top) override;

private:
    TooltipController m_tooltip_controller;
};

class Hud : public Group {
public:
    ~Hud() override;

    Hud(const Hud&) = delete;
    Hud& operator=(const Hud&) = delete;
    Hud(Hud&&) = delete;
    Hud& operator=(Hud&&) = delete;

    void show();
    void hide();
    bool is_hidden() const;

    // The container dialog that owns the HUD's compositing surface
    bw::Dialog* dialog() const { return m_main->dialog(); };

    bool passthrough_mouse() const override { return true; }
    bool passthrough_keyboard() const override { return true; }

    TooltipController* tooltip_controller() override { return g_game_tooltips.get(); }

protected:
    // Flags select the compositing style; see OverlayHud
    Hud(DialogBounds bounds, DialogFlags::Enum flags);

    // Links the dialog into the game's list and fires the InitChildren/Create events.
    // The derived class must add its children first: vanilla's Create handler walks
    // them to pick the initial keyboard focus
    void initialize();

    virtual void on_initialized() = 0;
    virtual void on_show() {}
    virtual void on_hide() {}
};

// Composited against the world in the game screen buffer (ReverseDialogDirection):
// the world presents through the container, child widgets draw over it, and the
// vanilla in-game repaint pass (FUN_0041C7B0) keeps it fresh when the world
// changes underneath. Clicks reach the map unless a child widget claims them.
class OverlayHud : public Hud {
public:
    explicit OverlayHud(DialogBounds bounds);

protected:
    void on_initialized() override;
};

// True for dialogs created by this framework (their interaction function is the
// widget event forwarder), e.g. to skip bin-dialog bounds fixups in vanilla init
bool is_widget_dialog(const bw::Dialog* dialog);

bool modal_ok_cancel(std::string_view prompt);
void modal_ok(std::string_view prompt);
bool modal_string_prompt(std::string_view prompt, std::string& value);
} // namespace ui
