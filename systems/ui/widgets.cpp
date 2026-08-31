#include "widgets.h"
#include "BW/bwhelper.h"
#include "systems/ui/palette_fader.h"

#include <algorithm>
#include <ranges>
#include <cctype>
#include <cassert>

namespace ui {
namespace defaults {
constexpr DialogFlags::Enum Dialog        = DialogFlags::Visible;
constexpr DialogFlags::Enum Button        = DialogFlags::Visible | DialogFlags::RespondsToEvents | DialogFlags::LargerFontSize16;
constexpr DialogFlags::Enum LightUp       = DialogFlags::Visible | DialogFlags::RespondsToEvents | DialogFlags::LargestFontSize16x
    | DialogFlags::HorizontalAlignmentLeft | DialogFlags::VerticalAlignmentMiddle;
constexpr DialogFlags::Enum CheckBox      = DialogFlags::Visible | DialogFlags::RespondsToEvents;
constexpr DialogFlags::Enum OptionButton  = DialogFlags::Visible | DialogFlags::RespondsToEvents;
// VerticalAlignMiddle makes it a slider vs a scrollbar
constexpr DialogFlags::Enum Slider        = DialogFlags::Visible | DialogFlags::RespondsToEvents | DialogFlags::VerticalAlignmentMiddle;
constexpr DialogFlags::Enum ListBox       = DialogFlags::Visible | DialogFlags::RespondsToEvents | DialogFlags::RemoveStyles;
constexpr DialogFlags::Enum ComboBox      = DialogFlags::Visible | DialogFlags::RespondsToEvents | DialogFlags::SmallestFontSize10;
constexpr DialogFlags::Enum TextBox       = DialogFlags::Visible | DialogFlags::RespondsToEvents;
constexpr DialogFlags::Enum Label         = DialogFlags::Visible;
constexpr DialogFlags::Enum Image         = DialogFlags::Visible;
}

static constexpr DialogFlags::Enum HorizontalAlignments
    = DialogFlags::HorizontalAlignmentLeft
    | DialogFlags::HorizontalAlignmentCenter
    | DialogFlags::HorizontalAlignmentRight;

static constexpr DialogFlags::Enum VerticalAlignments
    = DialogFlags::VerticalAlignmentBottom
    | DialogFlags::VerticalAlignmentMiddle
    | DialogFlags::VerticalAlignmentTop;

static constexpr DialogFlags::Enum FontSizes
    = DialogFlags::SmallestFontSize10
    | DialogFlags::SmallerFontSize14
    | DialogFlags::LargerFontSize16
    | DialogFlags::LargestFontSize16x;

static constexpr u16 OptionButtonHeight = 20; // Todo: Read this from g_theme_radiobutton_size instead
static constexpr size_t TextInputBufferSize = 256;

static Widget& set_flag(Widget& widget, DialogFlags::Enum flag, bool value) {
    bw::Dialog* dialog = widget.dialog();
    if (value) dialog->flags |= flag;
    else       dialog->flags &= ~flag;
    return widget;
}

static Widget& set_exclusive_flag(Widget& widget, DialogFlags::Enum group_mask, DialogFlags::Enum set) {
    bw::Dialog* dialog = widget.dialog();
    dialog->flags = (dialog->flags & ~group_mask) | set;
    return widget;
}

// --------------------------------
// Event handling
// --------------------------------
static u32 default_interaction_function(bw::Dialog& dialog, DialogEvent* event) {
    auto bindlg = dialog.to_bindlg();
    switch (dialog.type) {
        case DialogControlType::DialogBoxDC:           return helper::default_interaction_dialog(bindlg, event);
        case DialogControlType::Button:                return helper::default_interaction_button(bindlg, event);
        case DialogControlType::DefaultButton:         return helper::default_interaction_button(bindlg, event);
        case DialogControlType::OptionButton:          return helper::default_interaction_option(bindlg, event);
        case DialogControlType::CheckBox:              return helper::default_interaction_checkbox(bindlg, event);
        case DialogControlType::Image:                 return helper::default_interaction_image(bindlg, event);
        case DialogControlType::HorizontalScrollBar:   return helper::default_interaction_slider(bindlg, event);
        case DialogControlType::VerticalScrollBar:     return helper::default_interaction_slider(bindlg, event);
        case DialogControlType::TextInput:             return helper::default_interaction_textinput(bindlg, event);
        case DialogControlType::LabelLeftAlign:        return helper::default_interaction_label(bindlg, event);
        case DialogControlType::LabelCenterAlign:      return helper::default_interaction_label(bindlg, event);
        case DialogControlType::LabelRightAlign:       return helper::default_interaction_label(bindlg, event);
        case DialogControlType::ListBox:               return helper::default_interaction_listbox(bindlg, event);
        case DialogControlType::ComboBox:              return helper::default_interaction_combobox(bindlg, event);
        case DialogControlType::LightUpButton:         return helper::default_interaction_lightup(bindlg, event);
    }
    assert(!"Incorrect control type");
    return 1;
}

static u32 option_button_handle_event(Widget& widget, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_Activate: {
                widget.select();
            } break;
            case DialogUserType_SetSelectionIndex: {
                if (!event->param) return 1;
                widget.select();
            } break;
        }
    }
    return helper::button_shared_interact(widget.dialog()->to_bindlg(), event);
}

static u32 top_dialog_handle_event(Widget& widget, DialogEvent* event) {
    Group* group = widget.parent();
    if (group && event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_InitChildren:
                group->on_top_dialog_init_children(widget);
                break;
            case DialogUserType_Create:
                group->on_top_dialog_create(widget);
                break;
            case DialogUserType_Destroy:
                group->on_top_dialog_destroy(widget);
                break;
        }
    }
    return default_interaction_function(*widget.dialog(), event);
}

static u32 __fastcall widget_event_forward(bw::Dialog* dialog, DialogEvent* event) {
    assert(dialog->user && "widget_event_forward called without a user pointer");

    auto widget = static_cast<Widget*>(dialog->user);
    return widget->handle_event(event);
}

// --------------------------------
// Widget builders
// --------------------------------
static DialogControlType label_align_type(Align alignment) {
    switch (alignment) {
        case Align::Left:   return DialogControlType::LabelLeftAlign;
        case Align::Center: return DialogControlType::LabelCenterAlign;
        case Align::Right:  return DialogControlType::LabelRightAlign;
    }
    return DialogControlType::LabelLeftAlign;
}

static DialogFlags::Enum horizontal_align_flag(Align alignment) {
    switch (alignment) {
        case Align::Left:   return DialogFlags::HorizontalAlignmentLeft;
        case Align::Center: return DialogFlags::HorizontalAlignmentCenter;
        case Align::Right:  return DialogFlags::HorizontalAlignmentRight;
    }
    return DialogFlags::HorizontalAlignmentLeft;
}

Widget* Widget::horizontal_align(Align alignment) {
    switch (m_dialog.type) {
        case DialogControlType::LabelLeftAlign:
        case DialogControlType::LabelCenterAlign:
        case DialogControlType::LabelRightAlign: {
            m_dialog.type = label_align_type(alignment);
        } break;
        case DialogControlType::CheckBox:
        case DialogControlType::OptionButton: {
            set_flag(*this, DialogFlags::ReverseDialogDirection, alignment == Align::Right);
        } break;
        default: {
            set_exclusive_flag(*this, HorizontalAlignments, horizontal_align_flag(alignment));
        } break;
    }
    return this;
}

static DialogFlags::Enum fontsize_flag(FontSize size) {
    switch (size) {
        case FontSize::Small10:  return DialogFlags::SmallestFontSize10;
        case FontSize::Medium14: return DialogFlags::SmallerFontSize14;
        case FontSize::Large16:  return DialogFlags::LargerFontSize16;
        case FontSize::Large16x: return DialogFlags::LargestFontSize16x;
    }
    return DialogFlags::SmallerFontSize14;
}

Widget* Widget::fontsize(FontSize size) {
    set_exclusive_flag(*this, FontSizes, fontsize_flag(size));
    return this;
}

Widget* Widget::enable_hotkey() {
    if (m_text.empty()) return this;
    m_text[0] = static_cast<char>(std::tolower(m_text[0]));
    set_flag(*this, DialogFlags::HasHotkey, true);
    return this;
}

Widget* Widget::translucent(bool value) {
    set_flag(*this, DialogFlags::ApplyTranslucency, value);
    return this;
}

Widget* Widget::transparent(bool value) {
    set_flag(*this, DialogFlags::TransparentHideColor, value);
    return this;
}

Widget* Widget::hide() {
    set_flag(*this, DialogFlags::Visible, false);
    return this;
}

Widget* Widget::show() {
    set_flag(*this, DialogFlags::Visible, true);
    return this;
}

Widget* Widget::disable() {
    set_flag(*this, DialogFlags::Disabled, true);
    return this;
}

Widget* Widget::enable() {
    set_flag(*this, DialogFlags::Disabled, false);
    return this;
}

Widget* Widget::set_default() {
    if (m_dialog.type == DialogControlType::Button) {
        m_dialog.type = DialogControlType::DefaultButton;
    } else {
        set_flag(*this, DialogFlags::DefaultButton, true);
    }
    return this;
}

Widget* Widget::set_cancel() {
    set_flag(*this, DialogFlags::CancelButton, true);
    return this;
}

// --------------------------------
// Widget callbacks
// --------------------------------
Widget* Widget::on_create(std::move_only_function<void(Widget&)> callback) {
    m_callbacks.on_create = std::move(callback);
    return this;
}

Widget* Widget::on_destroy(std::move_only_function<void(Widget&)> callback) {
    m_callbacks.on_destroy = std::move(callback);
    return this;
}

Widget* Widget::on_activate(std::move_only_function<ActivateResult(Widget&)> callback) {
    m_callbacks.on_activate = std::move(callback);
    return this;
}

Widget* Widget::on_selection(std::move_only_function<void(Widget&, u16)> callback) {
    assert(m_dialog.type == DialogControlType::CheckBox
        || m_dialog.type == DialogControlType::OptionButton
        || m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox
        || m_dialog.type == DialogControlType::VerticalScrollBar
        || m_dialog.type == DialogControlType::HorizontalScrollBar);
    m_callbacks.on_selection = std::move(callback);
    return this;
}

Widget* Widget::on_nondlg_event(std::move_only_function<ActivateResult(Widget&, DialogEvent*)> callback) {
    m_callbacks.on_nondlg_event = std::move(callback);
    return this;
}

// --------------------------------
// Widget non-builder methods
// --------------------------------
void Widget::set_text(std::string text) {
    assert(m_dialog.type != DialogControlType::DialogBoxDC
        || m_dialog.type != DialogControlType::Image
        || m_dialog.type != DialogControlType::HorizontalScrollBar
        || m_dialog.type != DialogControlType::VerticalScrollBar
        || m_dialog.type != DialogControlType::TextInput
        || m_dialog.type != DialogControlType::ListBox
        || m_dialog.type != DialogControlType::ComboBox);
    
    m_text = text;

    if (m_dialog.type == DialogControlType::TextInput) {
        // TextInput has an allocated char[256]
        auto text_array = m_dialog.text<std::array<char, 256>*>();
        safe_copy(**text_array, text);
        return;
    }
    
    *m_dialog.text<const char*>() = m_text.c_str();
    helper::update_dialog(m_dialog.to_bindlg());
}

u8 Widget::add_item(std::string string) {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    m_strings.emplace_back(std::move(string));
    return helper::combobox_add_item(m_dialog.to_bindlg(), BinDlgComboboxFlags::ProvideStrings, m_strings.back().c_str());
}

void Widget::clear_items() {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    helper::combobox_clear_items(m_dialog.to_bindlg());
    m_strings.clear();
}

u8 Widget::selected_index() const {
    switch (m_dialog.type) {
        case DialogControlType::ComboBox:
        case DialogControlType::ListBox: return m_dialog.extra<bw::ListOrComboExtra>()->selection_index;
        case DialogControlType::HorizontalScrollBar:
        case DialogControlType::VerticalScrollBar: return m_dialog.data<bw::SliderData>()->current;
    }
    assert(!"selected_index called on unsupported control type");
    return 0;
}

std::string_view Widget::list_string(u8 index) {
    if (index > m_strings.size()) return {};
    return m_strings[index];
}

u8 Widget::count() const {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox
        || !"count called on unsupported control type");
    return m_dialog.extra<bw::ListOrComboExtra>()->item_count;
}

void Widget::select(bool value) {
    switch (m_dialog.type) {
        case DialogControlType::CheckBox: {
            auto data = m_dialog.data<bw::SelectionButtonData>();
            data->selected = value;
            if (!(m_dialog.flags & DialogFlags::UpdateNeeded)) {
                m_dialog.flags |= DialogFlags::UpdateNeeded;
                helper::update_dialog(m_dialog.to_bindlg());
            }
        } break;
        case DialogControlType::OptionButton: {
            // Toggle off all other option buttons in the same group
            if (m_group) {
                for (auto& member : m_group->members()) {
                    auto* potential_sibling = std::get_if<std::unique_ptr<Widget>>(&member);
                    if (!potential_sibling) continue;
                    auto sibling_widget = potential_sibling->get();
                    if (sibling_widget == this) continue;
                    auto sibling_dialog = sibling_widget->dialog();
                    if (sibling_dialog->type != DialogControlType::OptionButton) continue;
                    auto data = sibling_dialog->data<bw::SelectionButtonData>();
                    data->selected = false;
                    if (!(sibling_dialog->flags & DialogFlags::UpdateNeeded)) {
                        sibling_dialog->flags |= DialogFlags::UpdateNeeded;
                        helper::update_dialog(sibling_dialog->to_bindlg());
                    }
                }
            }
            auto data = m_dialog.data<bw::SelectionButtonData>();
            data->selected = true;
            m_dialog.flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(m_dialog.to_bindlg());
        } break;
    }
}

u32 Widget::handle_event(DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_Create: {
                if (m_callbacks.on_create) m_callbacks.on_create(*this);
            } break;
            case DialogUserType_Destroy: {
                if (m_callbacks.on_destroy) m_callbacks.on_destroy(*this);
            } break;
            case DialogUserType_Activate: {
                if (m_callbacks.on_activate) {
                    if (m_callbacks.on_activate(*this) == ActivateResult::ConsumeActivation) {
                        return 1;
                    }
                }
            } break;
            case DialogUserType_SetSelectionIndex: {
                if (m_callbacks.on_selection) m_callbacks.on_selection(*this, event->param);
            } break;
        }
    } else {
        if (m_callbacks.on_nondlg_event) {
            if (m_callbacks.on_nondlg_event(*this, event) == ActivateResult::ConsumeActivation) {
                return 1;
            }
        }
    }

    // Default behavior overrides
    if (m_dialog.type == DialogControlType::DialogBoxDC)  return top_dialog_handle_event(*this, event);
    if (m_dialog.type == DialogControlType::OptionButton) return option_button_handle_event(*this, event);
    if (m_dialog.type == DialogControlType::Image) return helper::generic_common_interact(m_dialog.to_bindlg(), event);

    return default_interaction_function(m_dialog, event);
}

Widget::Widget(DialogBounds bounds, DialogControlType type, u16 index)
    : m_dialog{
        .bounds = bounds,
        .index = index,
        .type = type,
        .user = this,
        .interaction_function = &widget_event_forward,
        .draw_function = helper::g_default_dialog_draw_functions[std::to_underlying(type)]
    } {}

// --------------------------------
// Group widget factories
// --------------------------------
// Each factory creates a Widget, wires it into BW's linked list at the
// correct position and stores ownership in this group's m_members
Widget* Group::button(DialogBounds bounds, std::string text) {
    auto owned = make(bounds, DialogControlType::Button, next_index());
    Widget* widget = owned.get();
    widget->m_text = std::move(text);
    *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
    widget->m_dialog.flags = defaults::Button;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::lightup_button(DialogBounds bounds, std::string text) {
    auto owned = make(bounds, DialogControlType::LightUpButton, next_index());
    Widget* widget = owned.get();
    widget->m_text = std::move(text);
    *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
    widget->m_dialog.flags = defaults::LightUp;
    widget->m_dialog.data<bw::LightUpButtonData>()->response_bounds.set_xywh(0, 0, bounds.w(), bounds.h());
    widget->m_dialog.extra<bw::LightUpButtonExtra>()->bounds = bounds;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::checkbox(DialogBounds bounds, std::string text, bool initial_value) {
    auto owned = make(bounds, DialogControlType::CheckBox, next_index());
    Widget* widget = owned.get();
    widget->m_text = std::move(text);
    *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
    widget->m_dialog.flags = defaults::CheckBox;
    widget->m_dialog.data<bw::SelectionButtonData>()->selected = initial_value;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::image(DialogBounds bounds, std::string path) {
    auto owned = make(bounds, DialogControlType::Image, next_index());
    Widget* widget = owned.get();
    widget->m_text = std::move(path);
    *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
    widget->m_dialog.flags = defaults::Image;
    load_bitmap(widget->m_text.c_str(), widget->dialog()->bitmap);
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::slider(DialogBounds bounds, u16 min, u16 max, u16 initial, bool vertical) {
    auto owned = make(bounds, DialogControlType::HorizontalScrollBar, next_index());
    Widget* widget = owned.get();
    if (vertical) widget->m_dialog.type = DialogControlType::VerticalScrollBar;
    auto data = widget->m_dialog.data<bw::SliderData>();
    data->min = min;
    data->max = max;
    data->current = initial;
    widget->m_dialog.flags = defaults::Slider;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::textinput(DialogBounds bounds) {
    auto owned = make(bounds, DialogControlType::TextInput, next_index());
    Widget* widget = owned.get();
    // Note - don't use m_text for textinput.m_dialog.text as the interaction function mallocs its own buffer
    widget->m_dialog.flags = defaults::TextBox;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::label(DialogBounds bounds, std::string text) {
    auto owned = make(bounds, DialogControlType::LabelLeftAlign, next_index());
    Widget* widget = owned.get();
    widget->m_text = std::move(text);
    *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
    widget->m_dialog.flags = defaults::Label;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::listbox(DialogBounds bounds) {
    auto owned = make(bounds, DialogControlType::ListBox, next_index());
    Widget* widget = owned.get();
    widget->m_dialog.flags = defaults::ListBox;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

Widget* Group::combobox(DialogBounds bounds) {
    auto owned = make(bounds, DialogControlType::ComboBox, next_index());
    Widget* widget = owned.get();
    widget->m_dialog.flags = defaults::ComboBox;
    widget->m_group = this;
    connect_child(widget);
    m_members.emplace_back(std::move(owned));
    return widget;
}

// --------------------------------
// Group subgroups
// --------------------------------
Group* Group::group() {
    auto owned = std::unique_ptr<Group>(new Group(m_root, this));
    Group* g = owned.get();
    m_members.emplace_back(std::move(owned));
    return g;
}

Group* Group::radio_group(DialogBounds bounds, std::initializer_list<std::string_view> options,
    u8 initial_selection, bool button_on_right) {
    assert(options.size() && "Provide at least one option string");

    auto owned_group = std::unique_ptr<Group>(new Group(m_root, this));
    Group* group = owned_group.get();
    m_members.emplace_back(std::move(owned_group));

    auto current_bounds = bounds;
    current_bounds.set_xywh(current_bounds.x(), current_bounds.y(), current_bounds.w(), OptionButtonHeight);
    for (auto& option : options) {
        auto owned = make(current_bounds, DialogControlType::OptionButton, next_index());
        Widget* widget = owned.get();
        widget->m_text = option;
        *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
        widget->m_dialog.flags = defaults::OptionButton;
        if (button_on_right) widget->m_dialog.flags |= DialogFlags::ReverseDialogDirection;
        widget->m_group = group;
        connect_child(widget);
        group->m_members.emplace_back(std::move(owned));
        current_bounds.offset(0, OptionButtonHeight);
    }

    if (initial_selection >= options.size()) initial_selection = 0;
    u8 seen = 0;
    for (auto& member : group->m_members) {
        auto* widget = std::get_if<std::unique_ptr<Widget>>(&member);
        if (!widget) continue;
        if (seen == initial_selection) {
            (*widget)->m_dialog.data<bw::SelectionButtonData>()->selected = true;
            break;
        }
        ++seen;
    }
    return group;
}

// --------------------------------
// Group -- broadcast builders
// --------------------------------
static void each_widget(Group& group, auto&& fn) {
    for (auto& member : group.members()) {
        std::visit([&](auto& widget) {
            using P = std::remove_reference_t<decltype(*widget)>;
            if constexpr (std::is_same_v<P, Widget>) {
                fn(*widget);
            } else {
                each_widget(*widget, fn);
            }
        }, member);
    }
}

Group* Group::horizontal_align(Align alignment) {
    each_widget(*this, [&](Widget& w) { w.horizontal_align(alignment); });
    return this;
}
Group* Group::fontsize(FontSize size) {
    each_widget(*this, [&](Widget& w) { w.fontsize(size); });
    return this;
}
Group* Group::translucent(bool value) {
    each_widget(*this, [&](Widget& w) { w.translucent(value); });
    return this;
}
Group* Group::transparent(bool value) {
    each_widget(*this, [&](Widget& w) { w.transparent(value); });
    return this;
}
Group* Group::hide() { each_widget(*this, [](Widget& w) { w.hide(); }); return this; }
Group* Group::show() { each_widget(*this, [](Widget& w) { w.show(); }); return this; }
Group* Group::disable() { each_widget(*this, [](Widget& w) { w.disable(); }); return this; }
Group* Group::enable() { each_widget(*this, [](Widget& w) { w.enable(); }); return this; }

// --------------------------------
// Group linked list wiring
// --------------------------------
static Widget* last_widget_in_subtree(Group& group) {
    for (auto& member : group.members() | std::views::reverse) {
        Widget* result = nullptr;
        std::visit([&](auto& up) {
            using P = std::remove_reference_t<decltype(*up)>;
            if constexpr (std::is_same_v<P, Widget>) {
                result = up.get();
            } else {
                result = last_widget_in_subtree(*up);
            }
            }, member);
        if (result) return result;
    }
    return nullptr;
}

static Widget* first_widget_in_subtree(Group& group) {
    for (auto& member : group.members()) {
        Widget* result = nullptr;
        std::visit([&](auto& up) {
            using P = std::remove_reference_t<decltype(*up)>;
            if constexpr (std::is_same_v<P, Widget>) {
                result = up.get();
            } else {
                result = first_widget_in_subtree(*up);
            }
            }, member);
        if (result) return result;
    }
    return nullptr;
}

// Widget that should immediately precede a new insertion into groups's members in BW's linked list
static Widget* dfs_predecessor_for_append(Group& g_start) {
    if (auto* widget = last_widget_in_subtree(g_start)) return widget;
    // group is empty at this level so walk up. At each parent, take the DFS-last
    // widget among the members that come before group.
    auto group = &g_start;
    while (auto parent = group->parent()) {
        Widget* result = nullptr;
        for (auto& member : parent->members()) {
            // Stop when we reach group itself.
            if (auto* group_up = std::get_if<std::unique_ptr<Group>>(&member); group_up && group_up->get() == group) {
                break;
            }
            std::visit([&](auto& up) {
                using P = std::remove_reference_t<decltype(*up)>;
                if constexpr (std::is_same_v<P, Widget>) {
                    result = up.get();
                } else {
                    if (auto* widget = last_widget_in_subtree(*up)) result = widget;
                }
                }, member);
        }
        if (result) return result;
        group = parent;
    }
    return nullptr;
}

void Group::connect_child(Widget* child) {
    assert(child->parent() && "widget must have its m_group set before connect_child");

    Widget* previous = dfs_predecessor_for_append(*child->parent());
    bw::Dialog* dialog = child->dialog();
    bw::Dialog* root  = m_root->m_main->dialog();
    dialog->parent = root;

    if (previous) {
        dialog->next = previous->dialog()->next;
        previous->dialog()->next = dialog;
    } else {
        auto head_slot = root->child<bw::Dialog*>();
        dialog->next = *head_slot;
        *head_slot = dialog;
    }
}

// --------------------------------
// Menu
// --------------------------------
const Palette& BackgroundState::default_palette() {
    if (!m_default_loaded) {
        Bitmap bitmap{};
        if (load_bitmap("glue\\dynamic\\tfont.pcx", bitmap, &m_default_palette,
                SFileScope::DiskAndMpq, false)) {
            if (bitmap.buffer) SMemFree(bitmap.buffer);
            m_default_loaded = true;
        }
    }
    return m_default_palette;
}

Menu::Menu(std::string background, PalettePreserve instant_colors)
    : Group(), m_instant_colors(instant_colors) {
    m_main = make(DialogBounds::from_xywh(0, 0, 640, 480), DialogControlType::DialogBoxDC, 0);
    m_main->m_text = std::move(background);
    m_main->dialog()->data<bw::MenuData>()->backup_surface.width = 640;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.height = 480;
    m_main->dialog()->flags = defaults::Dialog;
    m_main->m_group = this;
}

u32 Menu::load() {
    on_init();
    auto dialog = m_main->dialog();
    auto last_control = helper::glu_load_bin_dialog(dialog->to_bindlg(), dialog->interaction_function);
    on_exit(last_control);
    change_menu(true);
    return last_control;
}

void Menu::on_top_dialog_init_children(Widget& top) {
    auto dialog = top.dialog();
    if (top.text().empty()) {
        constexpr u16 w = 640;
        constexpr u16 h = 480;
        dialog->bitmap.width = w;
        dialog->bitmap.height = h;
        dialog->bitmap.buffer = static_cast<u8*>(SMemAlloc(w * h));
        g_init_palette = Palette{};
        copy_preserved_indices(g_init_palette,
            BackgroundState::instance().default_palette(), m_instant_colors);
    } else {
        load_bitmap(top.text().c_str(), dialog->bitmap, &g_init_palette);
    }

    g_background = dialog->bitmap;
    g_background_palette = g_init_palette;
    BackgroundState::instance().set_current(top.text());

    auto original_glues = g_gl_glues_mode;
    g_gl_glues_mode = g_gl_glues_mode > 25 ? MenuId::GlueSelConn : g_gl_glues_mode;
    helper::register_menu_functions(0, dialog->to_bindlg(), 0, 0);
    g_gl_glues_mode = original_glues;

    DynamicTheme::instance().apply();
}

void Menu::on_top_dialog_create(Widget& top) {
    auto dialog = top.dialog();
    g_dialog_fade_info._unk1 = true;
    g_dialog_fade_info._unk2 = true;
    helper::bw_refresh(g_dialog_fade_info.mouse_y, g_dialog_fade_info.pointer_height - 1,
        g_dialog_fade_info.mouse_x, g_dialog_fade_info.pointer_width - 1);

    *game_palette = *init_palette;
    if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
        dialog->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dialog->to_bindlg());
    }
    helper::glu_update_palette(3);
    // Set to -1 so a subsequent vanilla menu re-loads its background.
    g_glue_background_id = -1;
    helper::set_callback_timer(dialog->to_bindlg(), 0x2e, 100,
        (TimerCallback)helper::show_queued_network_error);
}

// --------------------------------
// Modal
// --------------------------------
Modal::Modal(std::string background) : Group() {
    Bitmap popup_bitmap{};
    load_bitmap(background.c_str(), popup_bitmap, nullptr, SFileScope::DiskAndMpq, false);

    const s16 w = popup_bitmap.width ? static_cast<s16>(popup_bitmap.width) : s16{640};
    const s16 h = popup_bitmap.height ? static_cast<s16>(popup_bitmap.height) : s16{480};
    const s16 x = static_cast<s16>((640 - w) / 2);
    const s16 y = static_cast<s16>((480 - h) / 2);

    m_main = make(DialogBounds::from_xywh(x, y, w, h), DialogControlType::DialogBoxDC, 0);
    m_main->m_text = std::move(background);
    m_main->dialog()->bitmap = popup_bitmap;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.width = 640;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.height = 480;
    m_main->dialog()->flags = defaults::Dialog | DialogFlags::ApplyTranslucency;
    m_main->m_group = this;
}

u32 Modal::load() {
    on_init();
    auto dialog = m_main->dialog();
    auto last_control = helper::glu_load_bin_dialog(dialog->to_bindlg(), dialog->interaction_function);
    on_exit(last_control);
    return last_control;
}

void Modal::on_top_dialog_init_children(Widget& top) {
    auto dialog = top.dialog();
    auto original_glues = g_gl_glues_mode;
    g_gl_glues_mode = g_gl_glues_mode > 25 ? MenuId::GlueSelConn : g_gl_glues_mode;
    helper::register_menu_functions(0, dialog->to_bindlg(), 0, 0);
    g_gl_glues_mode = original_glues;
}

void Modal::on_top_dialog_create(Widget& top) {
    auto dialog = top.dialog();
    if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
        dialog->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dialog->to_bindlg());
    }
}

void Modal::on_top_dialog_destroy(Widget& top) {
    helper::menu_pop_z_index(top.dialog()->to_bindlg());
}

namespace {
class OkCancelModal : public Modal {
public:
    OkCancelModal(std::string prompt, std::string background)
        : Modal(std::move(background)) {
        m_prompt_label = label(DialogBounds::from_ltrb(18, 20, 340, 145), std::move(prompt))
            ->horizontal_align(Align::Center)
            ->fontsize(FontSize::Medium14);
        m_ok_button = lightup_button(DialogBounds::from_ltrb(18, 160, 174, 186), "o\x04" "O\x01" "k")
            ->horizontal_align(Align::Center)
            ->enable_hotkey()
            ->set_default();
        m_cancel_button = lightup_button(DialogBounds::from_ltrb(188, 160, 340, 186), "c\x04" "C\x01" "ancel")
            ->horizontal_align(Align::Center)
            ->enable_hotkey()
            ->set_cancel();
    }

    Widget* m_prompt_label;
    Widget* m_ok_button;
    Widget* m_cancel_button;
};
} // namespace

bool modal_ok_cancel(std::string_view prompt) {
    // Todo: use_dynamic_theme = false - load appropriate background pDPopup.pcx

    OkCancelModal modal{std::string{prompt}, "glue\\dynamic\\pDPopup.pcx"};
    const u32 last_control = modal.load();
    return last_control == modal.m_ok_button->index();
}

namespace {
class OkModal : public Modal {
public:
    OkModal(std::string prompt, std::string background)
        : Modal(std::move(background)) {
        m_prompt_label = label(DialogBounds::from_ltrb(18, 20, 340, 145), std::move(prompt))
            ->horizontal_align(Align::Center)
            ->fontsize(FontSize::Medium14);
        m_ok_button = lightup_button(DialogBounds::from_ltrb(104, 160, 260, 186), "o\x04" "O\x01" "k")
            ->horizontal_align(Align::Center)
            ->enable_hotkey()
            ->set_default()
            ->set_cancel();
    }

    Widget* m_prompt_label;
    Widget* m_ok_button;
};
} // namespace

void modal_ok(std::string_view prompt) {
    OkModal modal{std::string{prompt}, "glue\\dynamic\\pokpopup.pcx"};
    modal.load();
}

namespace {
class StringPromptModal : public Modal {
public:
    StringPromptModal(std::string prompt, std::string background)
        : Modal(std::move(background))
    {
        m_prompt_label = label(DialogBounds::from_ltrb(18, 20, 340, 80), std::move(prompt))
            ->horizontal_align(Align::Center)
            ->fontsize(FontSize::Medium14);
        m_textinput = textinput(DialogBounds::from_ltrb(23, 80, 344, 106));
        m_ok_button = lightup_button(DialogBounds::from_ltrb(18, 160, 174, 186), "o\x04" "O\x01" "k")
            ->horizontal_align(Align::Center)
            ->enable_hotkey()
            ->set_default();
        m_cancel_button = lightup_button(DialogBounds::from_ltrb(188, 160, 340, 186), "c\x04" "C\x01" "ancel")
            ->horizontal_align(Align::Center)
            ->enable_hotkey()
            ->set_cancel();
    }

    Widget* m_prompt_label;
    Widget* m_textinput;
    Widget* m_ok_button;
    Widget* m_cancel_button;
};
} // namespace

bool modal_string_prompt(std::string_view prompt, std::string& value) {
    StringPromptModal modal{std::string{prompt}, "glue\\dynamic\\pEPopup.pcx"};

    if (!value.empty()) {
        modal.m_textinput->on_create([initial = value](Widget& w) mutable {
            w.set_text(std::move(initial));
        });
    }

    std::string captured;
    modal.m_textinput->on_destroy([&captured](Widget& w) {
        auto* buffer = *w.dialog()->text<std::array<char, 256>*>();
        if (buffer) captured = buffer->data();
    });

    const u32 last_control = modal.load();
    if (last_control == modal.m_ok_button->index()) {
        value = std::move(captured);
        return true;
    }
    return false;
}

// --------------------------------
// Menu
// --------------------------------
void Menu::change_background(std::string background, PalettePreserve preserve, u32 fade_ms) {
    if (m_main->m_text == background) return;

    Bitmap  new_bitmap{};
    Palette new_palette{};
    if (!load_bitmap(background.c_str(), new_bitmap, &new_palette,
        SFileScope::DiskAndMpq, false)) return;

    m_main->m_text = background;

    smem::ptr<u8[]> new_buffer{new_bitmap.buffer};
    const Palette black{};
    bw::Dialog* dialog = m_main->dialog();
    ui::g_palette_fader.start(dialog->to_bindlg(), black, fade_ms, Ease::In,
        [this, new_bitmap, new_palette, new_buffer = std::move(new_buffer), preserve, fade_ms]() mutable {
            bw::Dialog* dialog = m_main->dialog();
            if (dialog->bitmap.buffer) {
                SMemFree(dialog->bitmap.buffer);
            }
            dialog->bitmap.width = new_bitmap.width;
            dialog->bitmap.height = new_bitmap.height;
            dialog->bitmap.buffer = new_buffer.release();

            copy_preserved_indices(new_palette, g_display_palette, preserve);

            g_background = dialog->bitmap;
            g_background_palette = new_palette;
            BackgroundState::instance().set_current(m_main->m_text);
            *init_palette = new_palette;
            *game_palette = new_palette;

            helper::setup_dialog_palette_effects(nullptr, init_palette);

            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog->to_bindlg());

            ui::g_palette_fader.start(dialog->to_bindlg(),
                new_palette, fade_ms, Ease::Out, nullptr, preserve);
        },
        preserve);
}

void Menu::fade_background_out(std::move_only_function<void()> on_complete,
    PalettePreserve preserve, u32 fade_ms) {
    const Palette black{};
    bw::Dialog* dialog = m_main->dialog();
    ui::g_palette_fader.start(dialog->to_bindlg(), black, fade_ms, Ease::In,
        std::move(on_complete), preserve);
}

void Menu::fade_background_in(std::string background, PalettePreserve preserve, u32 fade_ms) {
    bw::Dialog* dialog = m_main->dialog();

    // Swap in a new background (display is assumed to be black) before revealing it.
    if (!background.empty() && m_main->m_text != background) {
        Bitmap  new_bitmap{};
        Palette new_palette{};
        if (load_bitmap(background.c_str(), new_bitmap, &new_palette,
            SFileScope::DiskAndMpq, false)) {
            m_main->m_text = background;

            if (dialog->bitmap.buffer) {
                SMemFree(dialog->bitmap.buffer);
            }
            dialog->bitmap = new_bitmap;

            copy_preserved_indices(new_palette, g_display_palette, preserve);

            g_background = dialog->bitmap;
            g_background_palette = new_palette;
            BackgroundState::instance().set_current(background);
            *init_palette = new_palette;
            *game_palette = new_palette;

            helper::setup_dialog_palette_effects(nullptr, init_palette);

            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog->to_bindlg());
        }
    }

    ui::g_palette_fader.start(dialog->to_bindlg(),
        g_background_palette, fade_ms, Ease::Out, nullptr, preserve);
}
} // namespace ui
