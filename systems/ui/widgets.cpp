#include "widgets.h"
#include "BW/bwhelper.h"
#include "systems/audio/audio_system.h"
#include "systems/graphics/translucency.h"
#include "systems/ui/palette_fader.h"
#include "systems/ui/tooltip.h"

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

static constexpr size_t TextInputBufferSize = 256;

static constexpr u16 DefaultRadioButtonHeight = 20;
static constexpr u16 RadioButtonRowSpacing = 8;

// Checked-listbox row state. flags_array first 4 bits is used by font style but the rest is unused
static constexpr u8 CheckedRowFlagBit = 1 << 4;
// Sprite indices in the theme UI GRP. From generic_checkbox_update @ 0x004efa80:
static constexpr u8 CheckboxSpriteUnchecked = 0x0C;
static constexpr u8 CheckboxSpriteChecked = 0x0E;

static constexpr u8 CheckedListboxRowPadding = 3;

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
    return helper::g_default_dialog_interaction_functions[static_cast<size_t>(dialog.type)](&dialog, event);
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

static bool is_keyboard_event(const DialogEvent* event) {
    switch (event->number_order) {
        case DialogEventType_KeyDown:
        case DialogEventType_KeyRepeat:
        case DialogEventType_KeyUp:
        case DialogEventType_Spacebar:
        case DialogEventType_SystemKey: return true;
        default: return false;
    }
}

static u32 top_dialog_handle_event(Widget& widget, DialogEvent* event) {
    Group* group = widget.parent();
    if (group) {
        if (event->number_order == DialogEventType_Dialog) {
            switch (event->user) {
                case DialogUserType_InitChildren: {
                    group->on_top_dialog_init_children(widget);
                } break;
                case DialogUserType_Create: {
                    group->on_top_dialog_create(widget);
                } break;
                case DialogUserType_Destroy: {
                    group->on_top_dialog_destroy(widget);
                } break;
                case DialogUserType_MouseOver: {
                    if (group->passthrough_mouse()) return 0;
                } break;
            }
        } else if (group->passthrough_keyboard() && is_keyboard_event(event)) {
            return 0;
        }
    }

    auto* dialog = widget.dialog();
    // Vanilla's Create handler and arrow-key focus cycling (00418A60) pick the first
    // focusable child from child_dlg and dereference it unchecked, so childless roots
    // must not reach them. Create still needs its invalidation.
    if (!dialog->child<bw::Dialog*>()) {
        if (event->number_order == DialogEventType_Dialog && event->user == DialogUserType_Create) {
            dialog->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(dialog->to_bindlg());
            return 0;
        }
        if ((event->number_order == DialogEventType_KeyDown || event->number_order == DialogEventType_KeyRepeat)
            && (event->virtual_key == VK_LEFT || event->virtual_key == VK_UP
                || event->virtual_key == VK_RIGHT || event->virtual_key == VK_DOWN)) {
            return 0;
        }
    }

    return default_interaction_function(*widget.dialog(), event);
}

static u32 __fastcall widget_event_forward(bw::Dialog* dialog, DialogEvent* event) {
    assert(dialog->user && "widget_event_forward called without a user pointer");

    auto widget = static_cast<Widget*>(dialog->user);
    return widget->handle_event(event);
}

bool is_widget_dialog(const bw::Dialog* dialog) {
    return dialog->interaction_function == &widget_event_forward;
}

// -------------------------------------
// Tooltip helpers
// -------------------------------------
static TooltipController* tooltip_controller_for(Widget& widget) {
    Group* group = widget.parent();
    if (!group) return nullptr;
    return group->root()->tooltip_controller();
}

// For routing mousewheel scroll events to hovered listbox
static Widget* find_scrollable_under_mouse(Group& group) {
    for (auto const& member : group.members()) {
        Widget* result = std::visit(Overloaded{
            [](const std::unique_ptr<Widget>& owned) -> Widget* {
                Widget& widget = *owned;
                const auto* dialog = widget.dialog();
                if (dialog->type == DialogControlType::ListBox
                    && (dialog->flags & DialogFlags::Visible)
                    && widget.screen_bounds().contains_mouse()) {
                    return &widget;
                }
                return nullptr;
            },
            [](const std::unique_ptr<Group>& owned) -> Widget* {
                return find_scrollable_under_mouse(*owned);
            },
        }, member);
        if (result) return result;
    }
    return nullptr;
}

static u8 listbox_row_at_mouse_y(const bw::Dialog& dialog, s16 mouse_y) {
    if (dialog.type != DialogControlType::ListBox && dialog.type != DialogControlType::ComboBox) {
        return 0xFF;
    }

    const auto* extra = dialog.extra<bw::ListOrComboExtra>();
    if (!extra || extra->element_height == 0) return 0xFF;
    
    const s16 top = dialog.bounds.top();
    if (mouse_y < top) return 0xFF;
    
    const u16 offset_within = static_cast<u16>(mouse_y - top);
    const u16 visible_row = static_cast<u16>(offset_within / extra->element_height);
    if (visible_row >= extra->visible_row_count) return 0xFF;
    
    const u16 absolute = static_cast<u16>(extra->selection_index_2 + visible_row);
    if (absolute >= extra->item_count) return 0xFF;
    
    return static_cast<u8>(absolute);
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

Widget* Widget::enable_hotkey(std::optional<char> hotkey) {
    // BW interprets the first character of the text buffer as the hotkey when the flag is set
    if (hotkey) {
        m_text.insert(m_text.begin(), static_cast<char>(std::tolower(static_cast<unsigned char>(*hotkey))));
        *m_dialog.text<const char*>() = m_text.c_str();
    } else {
        if (m_text.empty()) return this;
        m_text[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(m_text[0])));
    }
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

// Listbox spawns a scrollbar sibling dialog on init
static bw::Dialog* listbox_scrollbar(bw::Dialog& dialog) {
    if (dialog.type != DialogControlType::ListBox && dialog.type != DialogControlType::ComboBox) {
        return nullptr;
    }
    return dialog.data<bw::ListOrComboData>()->scrollbar;
}

Widget* Widget::hide() {
    set_flag(*this, DialogFlags::Visible, false);
    if (auto* scrollbar = listbox_scrollbar(m_dialog)) {
        scrollbar->flags &= ~DialogFlags::Visible;
    }
    invalidate_parent();
    return this;
}

Widget* Widget::show() {
    set_flag(*this, DialogFlags::Visible, true);
    if (auto* scrollbar = listbox_scrollbar(m_dialog)) {
        scrollbar->flags |= DialogFlags::Visible | DialogFlags::UpdateNeeded;
        helper::update_dialog(scrollbar->to_bindlg());
    }
    invalidate(true);
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

Widget* Widget::on_check_toggle(std::move_only_function<void(Widget&, u8, bool)> callback) {
    m_callbacks.on_check_toggle = std::move(callback);
    return this;
}

Widget* Widget::on_row_hover(std::move_only_function<void(Widget&, u8)> callback) {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    m_callbacks.on_row_hover = std::move(callback);
    return this;
}

void __fastcall Widget::draw_forward(bw::Dialog* dialog, DialogEvent*, u32, void*) {
    assert(dialog->user && "widget draw_forward called without a user pointer");

    auto widget = static_cast<Widget*>(dialog->user);
    if (widget->m_draw_callback) {
        widget->m_draw_callback(*widget);
    }
}

Widget* Widget::on_draw(std::move_only_function<void(Widget&)> callback) {
    m_draw_callback = std::move(callback);
    m_dialog.draw_function = &draw_forward;
    return this;
}

// --------------------------------
// Tooltips
// --------------------------------
Widget* Widget::tooltip(std::string text) {
    m_tooltip = std::make_unique<Tooltip>(std::move(text));
    return this;
}

Widget* Widget::tooltip(Tooltip tooltip) {
    m_tooltip = std::make_unique<Tooltip>(std::move(tooltip));
    return this;
}

Widget* Widget::tooltip_delay_ms(u16 ms) {
    m_tooltip_delay_ms = ms;
    return this;
}

Widget* Widget::tooltip_placement(Placement placement) {
    m_tooltip_placement = placement;
    return this;
}

Widget* Widget::tooltip_hide_on_click(bool value) {
    m_tooltip_hides_on_click = value;
    return this;
}

Widget* Widget::row_padding(u8 extra_pixels) {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    m_row_padding = extra_pixels;
    return this;
}

const Tooltip* Widget::tooltip_content() const {
    return m_tooltip.get();
}

// --------------------------------
// Widget non-builder methods
// --------------------------------
void Widget::set_text(std::string text) {
    assert(m_dialog.type != DialogControlType::DialogBoxDC
        && m_dialog.type != DialogControlType::Image
        && m_dialog.type != DialogControlType::HorizontalScrollBar
        && m_dialog.type != DialogControlType::VerticalScrollBar
        && m_dialog.type != DialogControlType::ListBox
        && m_dialog.type != DialogControlType::ComboBox);

    if (m_dialog.type == DialogControlType::TextInput) {
        // TextInput has an allocated char[256]
        auto text_array = m_dialog.text<std::array<char, TextInputBufferSize>*>();
        safe_copy(**text_array, text);
        return;
    }

    m_text = std::move(text);
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

void Widget::select_row(u8 index) {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    auto* extra = m_dialog.extra<bw::ListOrComboExtra>();
    if (index >= extra->item_count) return;
    extra->selection_index = static_cast<s8>(index);
    invalidate();
}

void Widget::clear_selection() {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    m_dialog.extra<bw::ListOrComboExtra>()->selection_index = -1;
    invalidate();
}

std::string_view Widget::list_string(u8 index) {
    if (index >= m_strings.size()) return {};
    return m_strings[index];
}

u8 Widget::count() const {
    assert((m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox)
        && "count called on unsupported control type");
    return m_dialog.extra<bw::ListOrComboExtra>()->item_count;
}

void Widget::set_checked(u8 index, bool checked) {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    auto* data = m_dialog.data<bw::ListOrComboData>();
    auto* extra = m_dialog.extra<bw::ListOrComboExtra>();
    if (!data->flags_array || index >= extra->item_count) return;
    if (checked) data->flags_array[index] |=  CheckedRowFlagBit;
    else         data->flags_array[index] &= ~CheckedRowFlagBit;
    invalidate();
}

bool Widget::is_checked(u8 index) const {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    const auto* data = m_dialog.data<bw::ListOrComboData>();
    const auto* extra = m_dialog.extra<bw::ListOrComboExtra>();
    if (!data->flags_array || index >= extra->item_count) return false;
    return (data->flags_array[index] & CheckedRowFlagBit) != 0;
}

static void set_all_checked(bw::Dialog& dialog, bool checked) {
    auto* data = dialog.data<bw::ListOrComboData>();
    auto* extra = dialog.extra<bw::ListOrComboExtra>();
    if (!data->flags_array) return;
    for (u8 i = 0; i < extra->item_count; ++i) {
        if (checked) data->flags_array[i] |=  CheckedRowFlagBit;
        else         data->flags_array[i] &= ~CheckedRowFlagBit;
    }
    static_cast<Widget*>(dialog.user)->invalidate();
}

void Widget::select_all() {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    set_all_checked(m_dialog, true);
}

void Widget::deselect_all() {
    assert(m_dialog.type == DialogControlType::ListBox
        || m_dialog.type == DialogControlType::ComboBox);
    set_all_checked(m_dialog, false);
}

void Widget::scroll_by(s32 rows) {
    if (m_dialog.type != DialogControlType::ListBox
        && m_dialog.type != DialogControlType::ComboBox) return;
    auto* extra = m_dialog.extra<bw::ListOrComboExtra>();
    if (!extra || extra->visible_row_count >= extra->item_count) return;

    const s32 max_top = static_cast<s32>(extra->item_count - extra->visible_row_count);
    const s32 current_top = extra->selection_index_2;
    const s32 new_top = std::clamp<s32>(current_top + rows, 0, max_top);
    if (new_top == current_top) return;

    extra->selection_index_2 = static_cast<u8>(new_top);
    // Keep scrollbar handle in sync when present
    if (auto* scrollbar = m_dialog.data<bw::ListOrComboData>()->scrollbar) {
        scrollbar->data<bw::SliderData>()->current = static_cast<u16>(new_top);
        scrollbar->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(scrollbar->to_bindlg());
    }
    invalidate();
}

void Widget::select(bool value) {
    switch (m_dialog.type) {
        case DialogControlType::CheckBox: {
            auto data = m_dialog.data<bw::SelectionButtonData>();
            data->selected = value;
            invalidate();
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
                    sibling_widget->invalidate();
                }
            }
            auto data = m_dialog.data<bw::SelectionButtonData>();
            data->selected = true;
            invalidate();
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
                if (m_callbacks.on_selection && event->param != 0xFFFF) m_callbacks.on_selection(*this, event->param);
            } break;
            case DialogUserType_MouseEnter: {
                if (auto* controller = tooltip_controller_for(*this)) controller->on_mouse_enter(this);
            } break;
            case DialogUserType_AcceptFocus: {
                // Dispatched when the mouse leaves the widget
                if (auto* controller = tooltip_controller_for(*this)) controller->on_mouse_leave(this);
                m_last_hovered_row = 0xFF;
            } break;
            case DialogUserType_MouseOver: {
                // Fires each frame while the cursor is over this widget
                // (from dialog_process_hit_detection @ 0x00418340)
                // Used for row-hover detection on listbox / combobox variants.
                if (m_callbacks.on_row_hover
                    && (m_dialog.type == DialogControlType::ListBox
                        || m_dialog.type == DialogControlType::ComboBox))
                {
                    const u8 row = listbox_row_at_mouse_y(m_dialog, event->current.y);
                    if (row != m_last_hovered_row) {
                        m_last_hovered_row = row;
                        if (row != 0xFF) m_callbacks.on_row_hover(*this, row);
                    }
                }
                // Reposition mouse-tracked tooltip so it follows the cursor.
                if (auto* controller = tooltip_controller_for(*this)) controller->on_mouse_over(this);
            } break;
        }
    } else {
        // Non-Dialog events.
        switch (event->number_order) {
            case DialogEventType_Idle: {
                // Top-dialog receives idle roughly every 100ms. Drive tooltip delay from it.
                if (m_dialog.type == DialogControlType::DialogBoxDC) {
                    if (auto* controller = tooltip_controller_for(*this)) controller->tick(GetTickCount());
                }
            } break;
            case DialogEventType_LButtonDown:
            case DialogEventType_RButtonDown:
            case DialogEventType_MButtonDown: {
                if (m_tooltip_hides_on_click) {
                    if (auto* controller = tooltip_controller_for(*this)) controller->hide();
                }
            } break;
            case DialogEventType_MWheelUp:
            case DialogEventType_MWheelDown: {
                // Wheel events land on the top dialog; route to whatever
                // listbox is under the cursor. Doesn't fire in windowed mode.
                if (m_dialog.type == DialogControlType::DialogBoxDC && m_group) {
                    if (auto* target = find_scrollable_under_mouse(*m_group)) {
                        const s32 delta = (event->number_order == DialogEventType_MWheelUp) ? -1 : 1;
                        target->scroll_by(delta);
                    }
                }
            } break;
        }

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

    const u32 result = default_interaction_function(m_dialog, event);

    // Post-default hook: bump listbox row height by m_row_padding once vanilla
    // listbox_init has set element_height from the font. Also shrinks
    // visible_row_count so we don't spill past the widget's visible area.
    if (m_row_padding > 0
        && event->number_order == DialogEventType_Dialog
        && event->user == DialogUserType_InitChildren
        && (m_dialog.type == DialogControlType::ListBox
            || m_dialog.type == DialogControlType::ComboBox)){
        auto* extra = m_dialog.extra<bw::ListOrComboExtra>();
        const u16 old_h = extra->element_height;
        const u16 new_h = old_h + m_row_padding;
        if (old_h > 0 && new_h <= 0xFF) {
            // Total_h is the exact visible height clamped by listbox_init.
            const u16 total_h = static_cast<u16>(extra->visible_row_count) * old_h;
            extra->element_height    = static_cast<u8>(new_h);
            extra->visible_row_count = static_cast<u8>(total_h / new_h);
        }
    }

    return result;
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

DialogBounds Widget::screen_bounds() const {
    const auto* parent = m_dialog.parent;
    if (!parent) return m_dialog.bounds;
    return m_dialog.bounds.offset(parent->bounds.left(), parent->bounds.top());
}

v2s16 Widget::screen_top_left() const {
    const auto bounds = screen_bounds();
    return v2s16{bounds.left(), bounds.top()};
}

void Widget::set_screen_bounds(DialogBounds area) {
    const auto* parent = m_dialog.parent;
    m_dialog.bounds = parent
        ? area.offset(static_cast<s16>(-parent->bounds.left()),
                      static_cast<s16>(-parent->bounds.top()))
        : area;
}

void Widget::invalidate(bool force_refresh) {
    if (!(m_dialog.flags & DialogFlags::UpdateNeeded) || force_refresh) {
        m_dialog.flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(m_dialog.to_bindlg());
    }
}

void Widget::invalidate_parent(bool force_refresh) {
    auto* parent = m_dialog.parent;
    if (!parent) return;
    if (!(parent->flags & DialogFlags::UpdateNeeded) || force_refresh) {
        parent->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(parent->to_bindlg());
    }
}

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
    auto owned = make(bounds, DialogControlType::Image, ui::SkipHitTestIndex);
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
    auto owned = make(bounds, DialogControlType::LabelLeftAlign, SkipHitTestIndex);
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

// --------------------------------
// Checked Listbox
// --------------------------------

// Per-row renderer: row_rect is in SCREEN coords (see listbox_update @ 0x0041B950).
static void __fastcall checked_listbox_item_renderer(
    bw::Dialog* dialog, u32 row_index, Box16* row_rect, s32 mode, s32 flags) {
    const auto* data  = dialog->data<bw::ListOrComboData>();
    const auto* extra = dialog->extra<bw::ListOrComboExtra>();
    const u8 row = static_cast<u8>(row_index);
    if (row >= extra->item_count) return;

    const bool checked = data->flags_array && (data->flags_array[row] & CheckedRowFlagBit);
    const char* label = data->strings_array ? data->strings_array[row] : nullptr;

    const s16 checkbox_size = g_theme_checkbox_size ? static_cast<s16>(g_theme_checkbox_size) : s16{10};
    const s16 row_h = static_cast<s16>((row_rect->bottom - row_rect->top) + 1);
    const s16 checkbox_x = static_cast<s16>(row_rect->left + 2);
    const s16 checkbox_y = static_cast<s16>(row_rect->top + (row_h - checkbox_size) / 2);

    const u8 sprite = checked ? CheckboxSpriteChecked : CheckboxSpriteUnchecked;
    helper::draw_clamped_dialog_image(row_rect, sprite, checkbox_x, checkbox_y, 0x2000);

    if (label) {
        helper::get_color_handle(LobbyTextColor::Green);
        const s16 text_x = static_cast<s16>(checkbox_x + checkbox_size + 4);
        helper::bw_print_xy(label, text_x, row_rect->top);
    }

    // Outline the highlighted row (set by a body click; drives move up/down).
    // helper::draw_box fills a rect, so draw four 1px edges to make a border.
    if (static_cast<s8>(row) == extra->selection_index) {
        const s16 x = row_rect->left;
        const s16 y = row_rect->top;
        s16 w = static_cast<s16>((row_rect->right - row_rect->left) + 1);
        // Rows span the full width; keep the border clear of the scrollbar when present.
        if (data->scrollbar && (data->scrollbar->flags & DialogFlags::Visible))
            w -= static_cast<s16>(g_theme_vslider_width);
        *g_draw_color_index = g_palette_index_reference[12];
        helper::draw_box(x, y, w, 1);                                    // top
        helper::draw_box(x, y, 1, row_h);                               // left
        helper::draw_box(static_cast<s16>(x + w - 1), y, 1, row_h);     // right
        helper::draw_box(x, static_cast<s16>(y + row_h - 1), w, 1);     // bottom
    }
}

static bool checked_listbox_toggle_clicked_row(bw::Dialog* dialog, bool commit_selection) {
    auto* data  = dialog->data<bw::ListOrComboData>();
    auto* extra = dialog->extra<bw::ListOrComboExtra>();
    const u8 clicked = extra->clicked_index;
    if (!data->flags_array || clicked >= extra->item_count) return false;

    data->flags_array[clicked] ^= CheckedRowFlagBit;
    const bool now = (data->flags_array[clicked] & CheckedRowFlagBit) != 0;

    if (commit_selection) {
        extra->selection_index = static_cast<s8>(clicked);
    }

    auto* widget = static_cast<Widget*>(dialog->user);
    widget->invalidate();
    if (widget->callbacks().on_check_toggle) {
        widget->callbacks().on_check_toggle(*widget, clicked, now);
    }
    return true;
}

static bool checked_listbox_select_clicked_row(bw::Dialog* dialog) {
    auto* extra = dialog->extra<bw::ListOrComboExtra>();
    const u8 clicked = extra->clicked_index;
    if (clicked >= extra->item_count) return false;

    extra->selection_index = static_cast<s8>(clicked);

    auto* widget = static_cast<Widget*>(dialog->user);
    widget->invalidate();
    if (widget->callbacks().on_selection) {
        widget->callbacks().on_selection(*widget, clicked);
    }
    return true;
}

static u32 __fastcall checked_listbox_event_forward(bw::Dialog* dialog, DialogEvent* event) {
    assert(dialog->user && "checked_listbox_event_forward called without a user pointer");

    if (event->number_order == DialogEventType_Dialog
        && event->user == DialogUserType_InitChildren){
        const u32 result = widget_event_forward(dialog, event);
        dialog->extra<bw::ListOrComboExtra>()->item_renderer = &checked_listbox_item_renderer;
        return result;
    }

    if (event->number_order == DialogEventType_LButtonUp) {
        // Split the click: the checkbox glyph toggles membership; the row body
        // highlights it (for move up/down). Run BW's default handler first so it
        // clears its click/drag-scroll tracking -- otherwise the listbox keeps
        // scroll-following the mouse after a click -- then apply our result.
        auto* extra = dialog->extra<bw::ListOrComboExtra>();
        const s16 checkbox_size = g_theme_checkbox_size
            ? static_cast<s16>(g_theme_checkbox_size) : s16{10};
        const s16 checkbox_left  = static_cast<s16>(dialog->bounds.left() + 2);
        const s16 checkbox_right = static_cast<s16>(checkbox_left + checkbox_size);
        const bool on_checkbox = event->current.x >= checkbox_left && event->current.x <= checkbox_right;

        const u8 clicked = extra->clicked_index;         // default handler may consume this
        const s8 prev_selection = extra->selection_index;
        const u32 result = widget_event_forward(dialog, event);
        extra->clicked_index = clicked;                  // restore for the helpers below
        if (on_checkbox) {
            extra->selection_index = prev_selection;     // a checkbox click must not move the highlight
            if (checked_listbox_toggle_clicked_row(dialog, false)) {
                AudioSystem::instance().play_sound(SoundId::glue_mouseover_wav,
                    PlaySoundSettings{.force_globally = true});
            }
        } else {
            checked_listbox_select_clicked_row(dialog);
        }
        return result;
    }

    if (event->number_order == DialogEventType_LButtonDouble) {
        // BW listbox handler treats doubleclick as activating the default
        // control, which we don't want. Swallow it (single clicks already handle
        // toggling) and just keep the clicked row highlighted.
        checked_listbox_select_clicked_row(dialog);
        return 1;
    }

    return widget_event_forward(dialog, event);
}

Widget* Group::checked_listbox(DialogBounds bounds) {
    Widget* widget = listbox(bounds);
    widget->m_dialog.interaction_function = &checked_listbox_event_forward;
    widget->m_row_padding = CheckedListboxRowPadding;
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
    Group* group = owned.get();
    m_members.emplace_back(std::move(owned));
    return group;
}

Group* Group::radio_group(DialogBounds bounds, std::span<const std::string_view> options,
    u8 initial_selection, Align alignment) {
    assert(!options.empty() && "Provide at least one option string");
    assert(alignment != Align::Center && "Center alignment is not available for radio buttons");

    auto owned_group = std::unique_ptr<Group>(new Group(m_root, this));
    Group* group = owned_group.get();
    m_members.emplace_back(std::move(owned_group));

    // g_theme_radiobutton_size is 0 until the theme GRP is parsed at menu load;
    // fall back so buttons don't stack when built in a Menu constructor.
    const u16 row_height = g_theme_radiobutton_size ? g_theme_radiobutton_size : DefaultRadioButtonHeight;
    // Pitch = button height + configurable gap; adjust RadioButtonRowSpacing to taste.
    const u16 row_pitch = static_cast<u16>(row_height + RadioButtonRowSpacing);
    auto current_bounds = bounds;
    current_bounds.set_xywh(current_bounds.x(), current_bounds.y(), current_bounds.w(), row_height);
    for (auto&& option : options) {
        auto owned = make(current_bounds, DialogControlType::OptionButton, next_index());
        Widget* widget = owned.get();
        widget->m_text.assign(option);
        *widget->m_dialog.text<const char*>() = widget->m_text.c_str();
        widget->m_dialog.flags = defaults::OptionButton;
        if (alignment == Align::Right) widget->m_dialog.flags |= DialogFlags::ReverseDialogDirection;
        widget->m_group = group;
        connect_child(widget);
        group->m_members.emplace_back(std::move(owned));
        current_bounds = current_bounds.offset(0, row_pitch);   // offset() is const, returns a new value
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

Group* Group::on_radio_selection(std::move_only_function<void(u8)> callback) {
    // These radio buttons signal a pick via Activate (click) -> widget.select();
    // they don't emit SetSelectionIndex, so hook on_activate rather than
    // on_selection. The single user handler is shared across every button
    // through one refcounted wrapper rather than duplicated.
    auto handler = std::make_shared<std::move_only_function<void(u8)>>(std::move(callback));

    u8 index = 0;
    for (auto& member : m_members) {
        auto* owned = std::get_if<std::unique_ptr<Widget>>(&member);
        if (!owned) continue;
        Widget* widget = owned->get();
        if (widget->dialog()->type != DialogControlType::OptionButton) continue;

        widget->on_activate([handler, index](Widget&) -> ActivateResult {
            if (*handler) (*handler)(index);
            // Continue so option_button_handle_event still runs select(),
            // which updates this button's (and its siblings') visual state.
            return ActivateResult::ContinueWithDefault;
        });
        ++index;
    }
    return this;
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
    each_widget(*this, [&](Widget& widget) { widget.horizontal_align(alignment); });
    return this;
}
Group* Group::fontsize(FontSize size) {
    each_widget(*this, [&](Widget& widget) { widget.fontsize(size); });
    return this;
}
Group* Group::translucent(bool value) {
    each_widget(*this, [&](Widget& widget) { widget.translucent(value); });
    return this;
}
Group* Group::transparent(bool value) {
    each_widget(*this, [&](Widget& widget) { widget.transparent(value); });
    return this;
}
Group* Group::hide() { each_widget(*this, [](Widget& widget) { widget.hide(); }); return this; }
Group* Group::show() { each_widget(*this, [](Widget& widget) { widget.show(); }); return this; }
Group* Group::disable() { each_widget(*this, [](Widget& widget) { widget.disable(); }); return this; }
Group* Group::enable() { each_widget(*this, [](Widget& widget) { widget.enable(); }); return this; }

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

void Group::attach_external_widget(Widget* widget) {
    assert(widget && "attach_external_widget requires a non-null widget");
    widget->m_group = this;

    bw::Dialog* dialog = widget->dialog();
    bw::Dialog* root_dialog = m_root->m_main->dialog();
    dialog->parent = root_dialog;

    auto* head_slot = root_dialog->child<bw::Dialog*>();
    if (!*head_slot) {
        *head_slot = dialog;
        dialog->next = nullptr;
        return;
    }
    bw::Dialog* cursor = *head_slot;
    while (cursor->next) cursor = cursor->next;
    cursor->next = dialog;
    dialog->next = nullptr;
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
    m_main = make(DialogBounds::from_xywh(0, 0, display::SurfaceWidth, display::SurfaceHeight), DialogControlType::DialogBoxDC, 0);
    m_main->m_text = std::move(background);
    m_main->dialog()->data<bw::MenuData>()->backup_surface.width = display::SurfaceWidth;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.height = display::SurfaceHeight;
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
        dialog->bitmap.width = display::SurfaceWidth;
        dialog->bitmap.height = display::SurfaceHeight;
        dialog->bitmap.buffer = static_cast<u8*>(SMemAlloc(display::SurfaceWidth * display::SurfaceHeight));
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
    g_gl_glues_mode = g_gl_glues_mode >= MenuId::OriginalCount ? MenuId::GlueSelConn : g_gl_glues_mode;
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
    top.invalidate();
    helper::glu_update_palette(3);
    // Set to -1 so a subsequent vanilla menu re-loads its background.
    g_glue_background_id = -1;
    helper::set_callback_timer(dialog->to_bindlg(), 0x2e, 100,
        reinterpret_cast<TimerCallback>(helper::show_queued_network_error));
}

// --------------------------------
// Modal
// --------------------------------
Modal::Modal(std::string background) : Group() {
    Bitmap popup_bitmap{};
    load_bitmap(background.c_str(), popup_bitmap, nullptr, SFileScope::DiskAndMpq, false);

    const s16 w = popup_bitmap.width ? static_cast<s16>(popup_bitmap.width) : static_cast<s16>(display::SurfaceWidth);
    const s16 h = popup_bitmap.height ? static_cast<s16>(popup_bitmap.height) : static_cast<s16>(display::SurfaceHeight);
    const s16 x = static_cast<s16>((display::SurfaceWidth - w) / 2);
    const s16 y = static_cast<s16>((display::SurfaceHeight - h) / 2);

    m_main = make(DialogBounds::from_xywh(x, y, w, h), DialogControlType::DialogBoxDC, 0);
    m_main->m_text = std::move(background);
    m_main->dialog()->bitmap = popup_bitmap;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.width = display::SurfaceWidth;
    m_main->dialog()->data<bw::MenuData>()->backup_surface.height = display::SurfaceHeight;
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
    g_gl_glues_mode = g_gl_glues_mode >= MenuId::OriginalCount ? MenuId::GlueSelConn : g_gl_glues_mode;
    helper::register_menu_functions(0, dialog->to_bindlg(), 0, 0);
    g_gl_glues_mode = original_glues;
}

void Modal::on_top_dialog_create(Widget& top) {
    top.invalidate();
}

void Modal::on_top_dialog_destroy(Widget& top) {
    helper::menu_pop_z_index(top.dialog()->to_bindlg());
}

// --------------------------------
// Hud
// --------------------------------
Hud::Hud(DialogBounds bounds, DialogFlags::Enum flags) : Group() {
    m_main = make(bounds, DialogControlType::DialogBoxDC, 0);
    auto* dialog = m_main->dialog();
    auto& surface = dialog->data<bw::MenuData>()->backup_surface;
    surface.width = bounds.w();
    surface.height = bounds.h();
    dialog->flags = flags;
    // The DialogBoxDC draw blits its background into the container with flags
    // hardcoded to 0 (opaque); draw like an Image so TransparentHideColor can
    // colorkey the container's blit
    dialog->draw_function = helper::g_default_dialog_draw_functions[std::to_underlying(DialogControlType::Image)];
    m_main->m_group = this;
}

Hud::~Hud() {
    helper::destroy_dialog(m_main->dialog()->to_bindlg());
}

void Hud::initialize() {
    initialize_widget_dialog(m_main->dialog());
    on_initialized();
}

void Hud::show() {
    auto dialog = m_main->dialog();
    dialog->flags |= DialogFlags::Visible | DialogFlags::UpdateNeeded;
    helper::update_dialog(dialog->to_bindlg());
    on_show();
}

void Hud::hide() {
    on_hide();
    auto dialog = m_main->dialog();
    dialog->flags &= ~DialogFlags::Visible;
    dialog->flags |= DialogFlags::UpdateNeeded;
    helper::update_dialog(dialog->to_bindlg());
}

bool Hud::is_hidden() const {
    return !(m_main->dialog()->flags & DialogFlags::Visible);
}

OverlayHud::OverlayHud(DialogBounds bounds)
    : Hud(bounds, defaults::Dialog | DialogFlags::ReverseDialogDirection) {}

void OverlayHud::on_initialized() {
    // No background: keep the container empty so the colorkey blit skips every
    // pixel and only child widgets composite over the world
    m_main->transparent();
    auto& surface = m_main->dialog()->data<bw::MenuData>()->backup_surface;
    std::fill_n(surface.buffer, size_t(surface.width) * surface.height, 0);

    // The container's 0x0C bitmap is the erase source for dirty regions: the
    // DialogBoxDC draw (generic_dialog_update @ 0x004EF590) blits it opaquely
    // into the surface before children redraw, clearing their stale pixels.
    // All-zero keeps the erased area on the colorkey so the world presents
    // through it. The Destroy path frees it (generic_common_interact @ 0x00418C40).
    auto& backdrop = m_main->dialog()->bitmap;
    backdrop.width = surface.width;
    backdrop.height = surface.height;
    backdrop.buffer = static_cast<u8*>(SMemAlloc(size_t(backdrop.width) * backdrop.height));
    std::fill_n(backdrop.buffer, size_t(backdrop.width) * backdrop.height, 0);

    // Hud's constructor draws the container like an Image so the backdrop blit
    // honors TransparentHideColor - which would skip every zero pixel and
    // erase nothing. The DialogBoxDC draw blits it opaque instead; the present
    // blit (0x0041C810) still colorkeys the surface with the dialog flags.
    m_main->dialog()->draw_function = helper::g_default_dialog_draw_functions[std::to_underlying(DialogControlType::DialogBoxDC)];
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
        modal.m_textinput->on_create([initial = value](Widget& widget) mutable {
            widget.set_text(std::move(initial));
        });
    }

    std::string captured;
    modal.m_textinput->on_destroy([&captured](Widget& widget) {
        auto* buffer = *widget.dialog()->text<std::array<char, TextInputBufferSize>*>();
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
    // Exclude preserved indices from translucency LUT during transition.
    graphics::g_translucency_preserve = preserve;

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
                new_palette, fade_ms, Ease::Out,
                [] { graphics::g_translucency_preserve = PalettePreserve::None; }, preserve);
        },
        preserve);
}

void Menu::fade_background_out(std::move_only_function<void()> on_complete,
    PalettePreserve preserve, u32 fade_ms) {
    graphics::g_translucency_preserve = preserve;
    const Palette black{};
    bw::Dialog* dialog = m_main->dialog();
    ui::g_palette_fader.start(dialog->to_bindlg(), black, fade_ms, Ease::In,
        [on_complete = std::move(on_complete)]() mutable {
            graphics::g_translucency_preserve = PalettePreserve::None;
            on_complete();
        }, preserve);
}

void Menu::fade_background_in(std::string background, PalettePreserve preserve, u32 fade_ms) {
    bw::Dialog* dialog = m_main->dialog();
    graphics::g_translucency_preserve = preserve;

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
        g_background_palette, fade_ms, Ease::Out,
        [] { graphics::g_translucency_preserve = PalettePreserve::None; }, preserve);
}
} // namespace ui
