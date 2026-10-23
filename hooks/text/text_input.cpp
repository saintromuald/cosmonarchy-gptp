#include "text_input.h"

#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/structures.h"
#include "aise_signal.h"
#include "defer.h"
#include "logger.h"
#include "systems/display/display.h"
#include "systems/text/measure.h"
#include "systems/text/text.h"
#include "systems/text/utf8.h"
#include "text_print.h"
#include "text_width.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <utility>

namespace {

constexpr u16 ImeInjectedModifiers = 0xA0; // Vanilla IME marker
constexpr u16 AutorepeatModifier =   0x100;
constexpr u32 InputDisabledMask =    0x2A;   // Mouse buttons held
constexpr LONG CandidateBottomMargin = 24;

// Filename fields must stay ASCII unless the process's ANSI file APIs accept UTF-8
bool filenames_take_utf8() {
    static const bool utf8 = GetACP() == CP_UTF8;
    return utf8;
}

void log_once(bool& logged, std::string_view message) {
    if (std::exchange(logged, true)) {
        return;
    }
    gptp::g_logger << std::format("text_input: {}\n", message);
}

bool input_disabled() {
    return (g_mouse_button_state & InputDisabledMask) != 0;
}

bool console_owns_typing() {
    // Aidebug's post-hide capture flags aren't exposed yet
    return g_aise_state.console_visible;
}

bw::TextInputData* field_data(bw::Dialog* field) {
    return field ? field->data<bw::TextInputData>() : nullptr;
}

char* field_buffer(bw::Dialog* field) {
    if (!field) {
        return nullptr;
    }
    std::array<char, 256>* const* buffer = field->text<std::array<char, 256>*>();
    return buffer && *buffer ? (*buffer)->data() : nullptr;
}

size_t field_length(bw::Dialog* field) {
    const char* const text = field_buffer(field);
    return text ? strnlen(text, 256) : 0;
}

bw::Dialog* root_of(bw::Dialog* dialog) {
    while (dialog && dialog->parent) {
        dialog = dialog->parent;
    }
    return dialog;
}

bool takes_keyboard_input(const bw::Dialog* field) {
    return field
        && field->type == DialogControlType::TextInput
        && (field->flags & 0x1A) == (DialogFlags::Visible | DialogFlags::RespondsToEvents);
}

void mark_updated(bw::Dialog* field, bw::TextInputData& data) {
    data.flags |= bw::ScrollToCursorOnDraw;
    if (!(field->flags & DialogFlags::UpdateNeeded)) {
        field->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(field);
    }
}

// Rollback must delete bytes, not whole codepoints
void delete_bytes(bw::Dialog* field, size_t start, size_t count) {
    char* const text = field_buffer(field);
    bw::TextInputData* const data = field_data(field);
    if (!text || !data) {
        return;
    }
    const size_t length = std::min<size_t>(strnlen(text, 256), 255);
    start = std::min(start, length);
    count = std::min(count, length - start);
    if (count) {
        std::move(text + start + count, text + length + 1, text + start);
        text[length - count] = '\0';
    }
    data->cursor_position = u8(start);
    if (count) {
        mark_updated(field, *data);
    }
}

void revert_inserted_bytes(bw::Dialog* owner, size_t count) {
    if (!count) {
        return;
    }
    const bw::TextInputData* const data = field_data(owner);
    const u8 cursor = data ? data->cursor_position : 0;
    delete_bytes(owner, cursor >= count ? cursor - count : 0, count);
}

DialogEvent make_char_event(u16 value, u16 modifiers) {
    DialogEvent event{};
    event.virtual_key = value;
    event.virtual_key_modifier = modifiers;
    event.number_order = DialogEventType_Spacebar;
    return event;
}

s32 dispatch_char_event(u16 value, u16 modifiers) {
    DialogEvent event = make_char_event(value, modifiers);
    return helper::send_input_to_all_dialogs(&event);
}

bw::Dialog* in_game_chat_field() {
    for (bw::Dialog* root = g_current_main_dlg->to_dialog(); root; root = root->next) {
        if (bw::Dialog** const first_child = root->child<bw::Dialog*>()) {
            for (bw::Dialog* child = *first_child; child; child = child->next) {
                if (child->index == 6 && child->interaction_function == helper::game_chat_textbox_interact) {
                    return child;
                }
            }
        }
    }
    return nullptr;
}

// Destroyed dialogs leave the root chain before being freed; check membership before dereferencing
bool in_root_chain(const bw::Dialog* dialog) {
    if (!g_current_main_dlg) {
        return false;
    }
    for (bw::Dialog* root = g_current_main_dlg->to_dialog(); root; root = root->next) {
        if (root == dialog) {
            return true;
        }
        if (bw::Dialog** const first_child = root->child<bw::Dialog*>()) {
            for (bw::Dialog* child = *first_child; child; child = child->next) {
                if (child == dialog) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::wstring read_composition_string(HIMC context, DWORD flag) {
    const LONG byte_count = ImmGetCompositionStringW(context, flag, nullptr, 0);
    if (byte_count <= 0) {
        return {};
    }
    std::wstring result(size_t(byte_count) / 2, L'\0');
    const LONG read = ImmGetCompositionStringW(context, flag, result.data(), byte_count);
    if (read <= 0) {
        return {};
    }
    result.resize(size_t(read) / 2);
    return result;
}

std::string to_utf8(std::wstring_view text) {
    if (const auto converted = text::utf16_to_utf8(text)) {
        return *converted;
    }
    return {};
}

std::optional<size_t> caret_byte_offset(std::wstring_view composition, LONG cursor_units) {
    if (cursor_units < 0 || size_t(cursor_units) > composition.size()) {
        return std::nullopt;
    }
    const auto prefix = text::utf16_to_utf8(composition.substr(0, size_t(cursor_units)));
    if (!prefix) {
        return std::nullopt;
    }
    return prefix->size();
}

u8 font_slot_of(const bw::Dialog* dialog) {
    switch (dialog->flags & 0x4C00) {
        case DialogFlags::SmallestFontSize10: return 0;
        case DialogFlags::LargerFontSize16:   return 2;
        case DialogFlags::LargestFontSize16x: return 3;
        default:                              return 1;
    }
}

// Route a late composition result to its owner even after focus moves
class ReroutedFocus {
public:
    explicit ReroutedFocus(bw::Dialog* field) {
        if (!field) {
            return;
        }
        bw::Dialog* const root = root_of(field);
        m_menu = root ? root->data<bw::MenuData>() : nullptr;
        if (m_menu && m_menu->focused_control != field) {
            m_saved = m_menu->focused_control;
            m_menu->focused_control = field;
            m_borrowed = true;
        }
    }

    ~ReroutedFocus() {
        if (m_borrowed) {
            m_menu->focused_control = m_saved;
        }
    }

private:
    bw::MenuData* m_menu = nullptr;
    bw::Dialog* m_saved = nullptr;
    bool m_borrowed = false;
};

// Match cnc-ddraw's centered letterbox
POINT surface_to_client(POINT surface, LONG client_width, LONG client_height) {
    const f32 surface_width = g_display.surface_width();
    const f32 surface_height = g_display.surface_height();
    const f32 scale = std::min(client_width / surface_width, client_height / surface_height);
    const f32 origin_x = (client_width - surface_width * scale) / 2;
    const f32 origin_y = (client_height - surface_height * scale) / 2;
    return POINT{LONG(origin_x + surface.x * scale), LONG(origin_y + surface.y * scale)};
}


void call_unhandled_char_callback(DialogEvent& event) {
    if (g_unhandled_char_callback) {
        g_unhandled_char_callback(&event);
    }
}


void dispatch_char_or_pass_to_gameplay(u16 value, u16 modifiers) {
    DialogEvent event = make_char_event(value, modifiers);
    if (!helper::send_input_to_all_dialogs(&event)) {
        call_unhandled_char_callback(event);
    }
}

} // namespace

void TextInput::ensure_installed() {
    if (!m_alive || m_installed || !g_main_window) {
        return;
    }
    m_previous_wndproc = (WNDPROC)SetWindowLongPtrA(g_main_window, GWLP_WNDPROC,
        (LONG_PTR)subclass_proc);
    if (!m_previous_wndproc) {
        return;
    }
    m_installed = true;
    // Keep gameplay keys out of the IME until a field gains focus
    if (HIMC const context = ImmGetContext(g_main_window)) {
        m_ime_open = ImmGetOpenStatus(context);
        ImmReleaseContext(g_main_window, context);
    }
    m_default_context = ImmAssociateContext(g_main_window, nullptr);
    m_associated = false;
    gptp::g_logger << std::format("text_input: installed, default himc {:#x}, code page {}",
        u32(m_default_context), GetACP()) << std::endl;
    reconcile_association();
}

void TextInput::teardown() {
    m_alive = false;
    if (!m_installed || !g_main_window) {
        return;
    }
    // Aidebug may have replaced the subclass; restore only if ours is still head
    if ((WNDPROC)GetWindowLongPtrA(g_main_window, GWLP_WNDPROC) == subclass_proc) {
        SetWindowLongPtrA(g_main_window, GWLP_WNDPROC, (LONG_PTR)m_previous_wndproc);
        m_installed = false;
        m_previous_wndproc = nullptr;
    }
    // Keep the forward target if our proc is still chained
    associate_default_context();
}

// Korean startup may leave no captured context; fall back to the window's default
void TextInput::associate_default_context() {
    if (m_default_context) {
        ImmAssociateContext(g_main_window, m_default_context);
        HIMC const context = ImmGetContext(g_main_window);
        if (context) {
            ImmReleaseContext(g_main_window, context);
            return;
        }
    }
    ImmAssociateContextEx(g_main_window, nullptr, IACE_DEFAULT);
}

void TextInput::reconcile_association() {
    if (!m_alive || !m_installed || !g_main_window) {
        return;
    }
    bw::Dialog* const focused = focused_text_input();
    const bool want = focused != nullptr;
    if (!want) {
        HIMC const context = ImmGetContext(g_main_window);
        if (context) {
            m_ime_open = ImmGetOpenStatus(context); // Preserve the user's input mode
            if (m_composition.owner) {
                if (in_root_chain(m_composition.owner)) {
                    // Completion commits synchronously before disassociation
                    ImmNotifyIME(context, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                } else {
                    // Never redirect a destroyed owner's composition to the next field
                    reset_composition();
                    ImmNotifyIME(context, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                }
                ImmReleaseContext(g_main_window, context);
            } else {
                ImmReleaseContext(g_main_window, context);
            }
        } else if (m_composition.owner) {
            reset_composition();
        }
        // Vanilla may have associated the context independently
        ImmAssociateContext(g_main_window, nullptr);
        m_associated = false;
        return;
    }
    if (!m_associated) {
        associate_default_context();
        m_associated = true;
        HIMC const context = ImmGetContext(g_main_window);
        gptp::g_logger << std::format(
            "text_input: associated the IME (saved {:#x}, now {:#x}, open {})",
            u32(m_default_context), u32(context), context ? ImmGetOpenStatus(context) != 0 : false)
            << std::endl;
        if (context) {

            ImmSetOpenStatus(context, m_ime_open);
            ImmReleaseContext(g_main_window, context);
        }
    }
}

bw::Dialog* TextInput::focused_text_input() {
    if (!g_current_main_dlg) {
        return nullptr;
    }
    if (*g_chat_message_type != 0) {
        if (bw::Dialog* const field = in_game_chat_field()) {
            return field;
        }
        log_once(m_chat_walk_logged, "chat is open but the field walk found nothing");
    }
    // A focused popup takes precedence over fields beneath it
    for (bw::Dialog* root = g_current_main_dlg->to_dialog(); root; root = root->next) {
        if (root->flags & DialogFlags::UseAlternateStyle) {
            bw::Dialog* const field = m_recorded_field;
            if (field && in_root_chain(field) && takes_keyboard_input(field)
                && root_of(field) == root) {
                return field;
            }
            // These roots never store focused_control; match the dispatcher's first eligible field
            if (bw::Dialog** const first_child = root->child<bw::Dialog*>()) {
                for (bw::Dialog* child = *first_child; child; child = child->next) {
                    if (child->type == DialogControlType::TextInput
                        && takes_keyboard_input(child)) {
                        return child;
                    }
                }
            }
            log_once(m_flagged_root_logged, "a UseAlternateStyle root holds no text input");
            continue;
        }
        if (bw::MenuData* const menu = root->data<bw::MenuData>()) {
            if (!menu->focused_control) {
                continue;
            }
            if (takes_keyboard_input(menu->focused_control)) {
                return menu->focused_control;
            }
            log_once(m_focus_non_text_logged,
                std::format("focus is on control type {}", u16(menu->focused_control->type)));
            return nullptr;
        }
    }
    return nullptr;
}

void TextInput::note_focus_gained(bw::Dialog* field) {
    m_recorded_field = field;
}

void TextInput::note_focus_lost(bw::Dialog* field) {
    if (m_recorded_field == field) {
        m_recorded_field = nullptr;
    }
    complete_composition(field);
}

bool TextInput::insert_bytes_verified(bw::Dialog* owner, std::string_view bytes, u16 modifiers,
    bool* handled_elsewhere) {
    if (bytes.empty()) {
        return true;
    }
    char* const text = field_buffer(owner);
    bw::TextInputData* const data = field_data(owner);
    if (!text || !data) {
        return false;
    }

    // Name wrappers clear placeholders on the first byte, resetting the cursor to 1
    const bool placeholder_present = owner->graphic != 0;
    size_t inserted = 0;
    u8 previous_cursor = data->cursor_position;
    for (const char byte : bytes) {
        g_ime_insert_blocked = 0;
        const s32 handled = dispatch_char_event(u8(byte), modifiers);
        const u8 cursor = data->cursor_position;
        const bool moved = cursor == previous_cursor + 1
            || (inserted == 0 && cursor == 1 && placeholder_present && owner->graphic == 0);
        if (!moved || cursor == 0 || (u8)text[cursor - 1] != (u8)byte) {
            if (handled_elsewhere) {
                *handled_elsewhere = handled != 0;
            }
            revert_inserted_bytes(owner, inserted);
            return false;
        }
        previous_cursor = cursor;
        ++inserted;
    }
    return true;
}

void TextInput::complete_composition(bw::Dialog* field) {
    if (!m_composition.owner || m_composition.owner != field) {
        return;
    }
    if (m_composition.busy || !g_main_window) {
        reset_composition();
        return;
    }
    HIMC const context = ImmGetContext(g_main_window);
    if (!context) {
        reset_composition();
        return;
    }
    ImmNotifyIME(context, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
    ImmReleaseContext(g_main_window, context);
    reset_composition();
}

void TextInput::complete_active_composition() {
    if (m_composition.owner) {
        complete_composition(m_composition.owner);
    }
}

BOOL WINAPI TextInput::translate_message(const MSG* message) {
    const BOOL translated = ::TranslateMessage(message);
    TextInput& self = instance();
    if (self.m_alive && translated && g_main_window && message->hwnd == g_main_window) {
        MSG char_message;
        while (PeekMessageW(&char_message, g_main_window, WM_CHAR, WM_CHAR, PM_REMOVE)) {
            self.on_char(char_message);
        }
    }
    self.reconcile_association();
    return translated;
}

LRESULT CALLBACK TextInput::subclass_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    TextInput& self = instance();
    if (!self.m_alive || !self.m_installed) {
        return CallWindowProcA(self.m_previous_wndproc, window, message, wparam, lparam);
    }
    const bool console = console_owns_typing();
    switch (message) {
        case WM_IME_COMPOSITION:
            if (!console) {
                return self.handle_ime_composition(window, lparam);
            }
            break;
        case WM_IME_ENDCOMPOSITION:
            if (!console) {
                return self.handle_ime_endcomposition();
            }
            break;
        case WM_IME_STARTCOMPOSITION:
            if (!console) {
                return 1; // Suppress the default composition window
            }
            break;
        case WM_IME_CHAR:
            if (!console) {
                return 1; // Already handled by WM_IME_COMPOSITION
            }
            break;
        case WM_IME_SETCONTEXT:
            if (!console) {
                log_once(self.m_ime_setcontext_logged,
                    std::format("WM_IME_SETCONTEXT (active {})", wparam != 0));
                // Keep OS candidates, but render composition in-field
                return CallWindowProcA(self.m_previous_wndproc, window, message, wparam,
                    lparam & ~ISC_SHOWUICOMPOSITIONWINDOW);
            }
            break;
        case WM_CHAR:
            // Untranslated ANSI bytes would corrupt UTF-8 fields
            if (!console && wparam >= 0x80) {
                log_once(self.m_ansi_char_dropped_logged, "dropped an ANSI WM_CHAR >= 0x80");
                return 1;
            }
            break;
    }
    return CallWindowProcA(self.m_previous_wndproc, window, message, wparam, lparam);
}

LRESULT TextInput::handle_ime_composition(HWND window, LPARAM lparam) {
    if (m_composition.busy) {
        return 1; // Ignore synchronous IMM reentry
    }
    if (input_disabled()) {
        return 1;
    }
    HIMC const context = ImmGetContext(window);
    if (!context) {
        return 1;
    }
    m_composition.busy = true;
    const auto busy_guard = utils::defer([this] { m_composition.busy = false; });

    if (m_composition.owner && !in_root_chain(m_composition.owner)) {
        reset_composition();
    }

    if (lparam == 0) {
        // Some IMEs signal cancellation with no flags
        revert_recorded_range();
        ImmReleaseContext(window, context);
        return 1;
    }

    std::wstring result16;
    std::wstring comp16;
    if (lparam & GCS_RESULTSTR) {
        result16 = read_composition_string(context, GCS_RESULTSTR);
    }
    if (lparam & GCS_COMPSTR) {
        comp16 = read_composition_string(context, GCS_COMPSTR);
    }
    const LONG cursor_units = (lparam & GCS_CURSORPOS)
        ? ImmGetCompositionStringW(context, GCS_CURSORPOS, nullptr, 0)
        : -1;

    if (!(lparam & (GCS_RESULTSTR | GCS_COMPSTR))) {

        if (m_composition.owner && cursor_units >= 0) {
            if (const auto offset = caret_byte_offset(m_composition.text, cursor_units)) {
                m_composition.caret_offset = *offset;
            }
            update_candidates(context);
        }
        ImmReleaseContext(window, context);
        return 1;
    }


    bw::Dialog* const owner = m_composition.owner ? m_composition.owner : focused_text_input();
    revert_recorded_range();
    if (!owner) {
        // Cancel unowned composition so IME mode toggles remain usable
        log_once(m_no_field_composition_logged, "composition arrived with no field to feed");
        ImmNotifyIME(context, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        ImmReleaseContext(window, context);
        return 1;
    }

    const ReroutedFocus reroute(owner);
    const std::string result8 = to_utf8(result16);
    const std::string comp8 = to_utf8(comp16);
    if (result8.size() + comp8.size() >= 2) {
        g_ime_input_active = 1; // Enable field-cap rejection signals
    }

    bool rejected = !feed_composition_bytes(owner, result8);
    const size_t comp_start = field_data(owner) ? field_data(owner)->cursor_position : 0;
    if (!rejected && !comp8.empty()) {
        if (feed_composition_bytes(owner, comp8)) {
            m_composition.owner = owner;
            m_composition.start = u16(comp_start);
            m_composition.bytes = u16(comp8.size());
            m_composition.recorded_cursor = field_data(owner)->cursor_position;
            m_composition.recorded_length = u16(field_length(owner));
            m_composition.caret_offset = caret_byte_offset(comp16, cursor_units)
                .value_or(comp8.size());
            m_composition.text = comp16;
        } else {
            // Keep the committed result; the IME will not resend it
            rejected = true;
        }
    } else if (!rejected) {
        reset_composition();
    }

    if (rejected) {
        // Clear rather than cancel to preserve the IME's open state and mode toggles
        ImmSetCompositionString(context, SCS_SETSTR, nullptr, 0, nullptr, 0);
        reset_composition();
    } else {
        update_candidates(context);
    }
    ImmReleaseContext(window, context);
    return 1;
}

LRESULT TextInput::handle_ime_endcomposition() {
    if (m_composition.busy) {
        return 1;
    }
    // Results precede END; any remaining preedit was cancelled
    revert_recorded_range();
    return 1;
}

void TextInput::on_char(MSG& message) {
    if (!m_alive) {
        return;
    }
    if (console_owns_typing()) {
        DispatchMessageW(&message);
        return;
    }
    if (input_disabled()) {
        return;
    }
    g_ime_input_active = 0;

    u16 modifiers = u16(helper::check_modifier_keys());
    if (message.lParam & 0x40000000) {
        modifiers |= AutorepeatModifier;
    }
    const std::optional<char32_t> codepoint = take_codepoint(u16(message.wParam));
    if (!codepoint) {
        return;
    }
    if (*codepoint < 0x20 || *codepoint == 0x7F) {
        dispatch_char_or_pass_to_gameplay(u16(message.wParam), modifiers);
        return;
    }

    u8 encoded[4] = {};
    const size_t count = text::encode_utf8(*codepoint, encoded);
    if (!count) {
        return;
    }
    const std::string_view bytes{(const char*)encoded, count};
    if (bw::Dialog* const field = focused_text_input()) {
        bool handled_elsewhere = false;
        if (insert_bytes_verified(field, bytes, modifiers, &handled_elsewhere)) {
            return;
        }

        if (!handled_elsewhere && encoded[0] < 0x80) {
            DialogEvent event = make_char_event(encoded[0], modifiers);
            call_unhandled_char_callback(event);
        }
        return;
    }
    // Preserve menu hotkeys and auto-focus without dispatching partial UTF-8 sequences
    if (encoded[0] < 0x80) {
        dispatch_char_or_pass_to_gameplay(encoded[0], modifiers);
    }
}

std::optional<char32_t> TextInput::take_codepoint(u16 unit) {
    const bool high = unit >= 0xD800 && unit <= 0xDBFF;
    const bool low = unit >= 0xDC00 && unit <= 0xDFFF;
    if (high) {
        if (m_pending_surrogate) {
            log_once(m_lone_surrogate_logged, "dropped a lone surrogate half");
        }
        m_pending_surrogate = unit;
        return std::nullopt;
    }
    if (low) {
        if (!m_pending_surrogate) {
            log_once(m_lone_surrogate_logged, "dropped a lone surrogate half");
            return std::nullopt;
        }
        const u16 high_half = std::exchange(m_pending_surrogate, 0);
        return char32_t(0x10000 + ((high_half - 0xD800) << 10) + (unit - 0xDC00));
    }
    if (m_pending_surrogate) {
        m_pending_surrogate = 0;
        log_once(m_lone_surrogate_logged, "dropped a lone surrogate half");
    }
    return char32_t(unit);
}

void TextInput::reset_composition() {
    m_composition = Composition{};
}


void TextInput::revert_recorded_range() {
    if (!m_composition.owner) {
        return;
    }
    if (!in_root_chain(m_composition.owner)) {
        reset_composition();
        return;
    }
    bw::Dialog* const owner = m_composition.owner;
    const bool buffer_matches = field_data(owner)
        && field_data(owner)->cursor_position == m_composition.recorded_cursor
        && field_length(owner) == m_composition.recorded_length;
    if (takes_keyboard_input(owner) && buffer_matches) {
        delete_bytes(owner, m_composition.start, m_composition.bytes);
    }
    reset_composition();
}

// Overwrite would destroy text that rollback cannot restore
bool TextInput::feed_composition_bytes(bw::Dialog* owner, std::string_view bytes) {
    if (bytes.empty()) {
        return true;
    }
    bw::TextInputData* const data = field_data(owner);
    if (!data) {
        return false;
    }
    const u8 saved_flags = data->flags;
    data->flags |= bw::InsertMode;
    const bool accepted = insert_bytes_verified(owner, bytes, ImeInjectedModifiers);
    data->flags = (data->flags & ~bw::InsertMode) | (saved_flags & bw::InsertMode);
    return accepted;
}

POINT TextInput::field_caret_surface_position(bw::Dialog* field) {
    const bw::TextInputData* const data = field_data(field);
    const char* const text = field_buffer(field);
    POINT caret{};
    if (!data || !text) {
        return caret;
    }
    size_t caret_offset = data->cursor_position;
    if (m_composition.owner == field) {
        caret_offset = std::min<size_t>(m_composition.start + m_composition.caret_offset,
            field_length(field));
    }
    const size_t scroll = std::min<size_t>(data->scroll_offset, caret_offset);
    const u8 slot = font_slot_of(field);
    const s32 width = text::line_width(slot, std::string_view(text + scroll, caret_offset - scroll));
    const bw::Dialog* const parent = field->parent;
    caret.x = (parent ? parent->bounds.left() : 0) + field->bounds.left()
        + data->horizontal_margin + width;
    caret.y = (parent ? parent->bounds.top() : 0) + field->bounds.top()
        + data->vertical_offset + text::TextSystem::instance().line_advance(slot);
    return caret;
}


void TextInput::update_candidates(HIMC context) {
    RECT client{};
    if (!context || !g_main_window || !GetClientRect(g_main_window, &client)) {
        return;
    }
    POINT caret{};
    bw::Dialog* const field = m_composition.owner ? m_composition.owner : focused_text_input();
    if (field && field_data(field)) {
        caret = surface_to_client(field_caret_surface_position(field), client.right, client.bottom);
    } else {
        caret = {client.right / 2, client.bottom - CandidateBottomMargin};
    }
    CANDIDATEFORM form{};
    form.dwIndex = 0;
    form.dwStyle = CFS_CANDIDATEPOS;
    form.ptCurrentPos = caret;
    ImmSetCandidateWindow(context, &form);
}


namespace hooks {
namespace {


void update_dialog_if_needed(bw::Dialog* dlg) {
    if (!(dlg->flags & DialogFlags::UpdateNeeded)) {
        dlg->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(dlg);
    }
}

constexpr u16 BlinkTimerIndex = 2;
constexpr u32 BlinkIntervalMs = 500;
const TimerCallback CursorBlinkTick = (TimerCallback)0x00416600;

// 0x00416790 with start = true
void start_blink(bw::Dialog* dlg) {
    dlg->flags |= DialogFlags::DrawActiveState;
    helper::set_callback_timer(dlg, BlinkTimerIndex, BlinkIntervalMs, CursorBlinkTick);
    update_dialog_if_needed(dlg);
}

// Hide the OS cursor while typing until the mouse moves
void arm_mouse_slots(bw::Dialog* dlg, bw::TextInputData& data) {
    if (!data.holds_mouse_slots) {
        helper::refresh_cursor_0();
        helper::set_active_widget_for_event(dlg, 3);
        helper::set_active_widget_for_event(dlg, 4);
        helper::set_active_widget_for_event(dlg, 7);
        data.holds_mouse_slots = true;
    }
}

void release_mouse_slots(bw::Dialog* dlg, bw::TextInputData& data) {
    if (data.holds_mouse_slots) {
        helper::cursor_refresh();
        helper::clear_active_widget_slots(dlg);
        data.holds_mouse_slots = false;
    }
}


void set_field_font(const bw::Dialog* dlg) {
    set_font_handle_impl(font_base[font_slot_of(dlg)]);
}


void insert_byte(bw::Dialog* dlg, u8 byte) {
    char* const text = field_buffer(dlg);
    bw::TextInputData* const data = field_data(dlg);
    if (!text || !data) {
        return;
    }
    const size_t len = field_length(dlg);
    if (len + 1 >= 0x100) {
        return;
    }
    const size_t cursor = data->cursor_position;
    if (cursor >= len) {
        text[len] = char(byte);
        text[len + 1] = '\0';
    } else if (data->flags & bw::InsertMode) {
        std::move(text + cursor, text + len + 1, text + cursor + 1);
        text[cursor] = char(byte);
    } else if (!text::is_continuation(byte)) {
        size_t rest = 0;
        if (text::is_continuation(text[cursor])) {
            while (cursor + 1 + rest < len && text::is_continuation(text[cursor + 1 + rest])) {
                ++rest;
            }
        } else {
            rest = text::sequence_length_at({text, len}, cursor) - 1;
        }
        if (rest) {
            std::move(text + cursor + 1 + rest, text + len + 1, text + cursor + 1);
            text[len - rest] = '\0';
        }
        text[cursor] = char(byte);
    } else {
        // Continuations extend the newly written lead rather than overwrite the next codepoint
        std::move(text + cursor, text + len + 1, text + cursor + 1);
        text[cursor] = char(byte);
    }
    data->cursor_position = u8(cursor + 1);
    data->flags |= bw::ScrollToCursorOnDraw;
    update_dialog_if_needed(dlg);
}


void delete_text(bw::Dialog* dlg, size_t at, size_t count) {
    char* const text = field_buffer(dlg);
    bw::TextInputData* const data = field_data(dlg);
    if (!text || !data || !count) {
        return;
    }
    const size_t len = field_length(dlg);
    at = std::min(at, len);
    count = std::min(count, len - at);
    if (!count) {
        return;
    }
    std::move(text + at + count, text + len + 1, text + at);
    text[len - count] = '\0';
    data->cursor_position = u8(at);
    if (data->scroll_offset != 0) {
        data->flags |= bw::ScrollToCursorOnDraw;
    }
    update_dialog_if_needed(dlg);
}

bw::Dialog* child_by_index(bw::Dialog* root, u16 index) {
    if (bw::Dialog** const first_child = root->child<bw::Dialog*>()) {
        for (bw::Dialog* child = *first_child; child; child = child->next) {
            if (child->index == index) {
                return child;
            }
        }
    }
    return nullptr;
}

void (__fastcall* const refresh_save_name_list)(bw::Dialog*) = (void(__fastcall*)(bw::Dialog*))0x004DF520;
void (__fastcall* const refresh_save_game_list)(bw::Dialog*) = (void(__fastcall*)(bw::Dialog*))0x00461A90;


void refresh_ok_button_and_file_list(bw::Dialog& dlg, void (__fastcall* refresh_list)(bw::Dialog*)) {
    bw::Dialog* const root = root_of(&dlg);
    if (bw::Dialog* const ok_button = child_by_index(root, 0xFFFE)) {
        const char* const text = field_buffer(&dlg);
        if (text && text[0] != '\0') {
            helper::enable_dialog(ok_button);
        } else {
            helper::disable_dialog(ok_button);
        }
    }
    bw::Dialog* const combo = child_by_index(root, 1);
    bw::ListOrComboExtra* const extra = combo ? combo->extra<bw::ListOrComboExtra>() : nullptr;
    if (extra && extra->item_count != 0 && extra->selection_index != -1) {
        DialogEvent selection{};
        selection.user = DialogUserType_SetSelectionIndex;
        selection.param = 0xFF;
        selection.number_order = DialogEventType_Dialog;
        combo->interaction_function(combo, &selection);
        update_dialog_if_needed(combo);
        refresh_list(combo);
    }
}

// Only the replay-name wrapper signals IME rejection
void save_name_char(DialogEvent& event, bw::Dialog& dlg, bool signal_blocked,
    void (__fastcall* refresh_list)(bw::Dialog*)) {
    const u8 byte = u8(event.virtual_key);
    if (byte && std::strchr(g_network_table.get_string(0x11), byte)) {
        return;
    }

    bool insert = true;
    bool blocked_reject = false;
    if (byte >= 0x80 && !filenames_take_utf8()) {
        insert = false;
        blocked_reject = true;
    } else if (byte != VK_BACK) {
        const size_t len = field_length(&dlg);
        if (len > 26) {
            insert = false;
            blocked_reject = true;
        } else if (byte == ' ' && len == 0) {
            insert = false;
        }
    }
    if (blocked_reject && signal_blocked && g_ime_input_active) {
        g_ime_insert_blocked = 1;
    }
    if (insert) {
        textbox_interact(&dlg, &event);
    }
    refresh_ok_button_and_file_list(dlg, refresh_list);
}

} // namespace

u32 __fastcall textbox_interact(bw::Dialog* dlg, DialogEvent* evt) {
    TextInput::instance().ensure_installed();
    bw::TextInputData* const data = field_data(dlg);
    if (!data) {
        return 0;
    }

    switch (evt->number_order) {
        case DialogEventType_KeyDown:
            if (evt->virtual_key == VK_INSERT && (evt->virtual_key_modifier & 0xF) == 0) {
                data->flags ^= bw::InsertMode;
                return 1;
            }
            [[fallthrough]];
        case DialogEventType_KeyRepeat:
            switch (evt->virtual_key) {
                case VK_END:
                    data->cursor_position = u8(field_length(dlg));
                    update_dialog_if_needed(dlg);
                    return 1;
                case VK_HOME:
                    data->cursor_position = 0;
                    update_dialog_if_needed(dlg);
                    return 1;
                case VK_LEFT: {
                    const u8 cursor = data->cursor_position;
                    if (cursor != 0) {
                        data->cursor_position = u8(text::prev_sequence_start(
                            {field_buffer(dlg), field_length(dlg)}, cursor));
                        update_dialog_if_needed(dlg);
                    }
                    return 1;
                }
                case VK_RIGHT: {
                    const std::string_view text{field_buffer(dlg), field_length(dlg)};
                    const u8 cursor = data->cursor_position;
                    if (cursor < text.size()) {
                        data->cursor_position = u8(cursor + text::sequence_length_at(text, cursor));
                        update_dialog_if_needed(dlg);
                    }
                    return 1;
                }
                case VK_DELETE: {
                    const std::string_view text{field_buffer(dlg), field_length(dlg)};
                    const u8 cursor = data->cursor_position;
                    if (cursor < text.size()) {
                        delete_text(dlg, cursor, text::sequence_length_at(text, cursor));
                    }
                    return 1;
                }
                default:
                    return 0;
            }
        case DialogEventType_MouseMove:
            release_mouse_slots(dlg, *data);
            return 0;
        case DialogEventType_LButtonDown:
        case DialogEventType_LButtonDouble:

            TextInput::instance().complete_active_composition();
            start_blink(dlg);
            release_mouse_slots(dlg, *data);
            click_cursor_pos_update(dlg, evt);
            return 1;
        case DialogEventType_Dialog:
            switch (evt->user) {
                case DialogUserType_Destroy:
                    TextInput::instance().note_focus_lost(dlg);
                    helper::textbox_destroy(dlg);
                    break;
                case DialogUserType_RequestFocus:
                    TextInput::instance().note_focus_gained(dlg);
                    start_blink(dlg);
                    break;
                case DialogUserType_AcceptFocus:
                    TextInput::instance().note_focus_lost(dlg);
                    release_mouse_slots(dlg, *data);
                    dlg->flags &= ~DialogFlags::DrawActiveState;
                    helper::clear_callback_timer(dlg, BlinkTimerIndex);
                    update_dialog_if_needed(dlg);
                    break;
                case DialogUserType_InitChildren:
                    helper::textbox_init(dlg);
                    break;
                default:
                    break;
            }
            return helper::generic_common_interact(dlg->to_bindlg(), evt);
        case DialogEventType_Spacebar: {
            start_blink(dlg);
            if (evt->virtual_key == VK_BACK && data->cursor_position != 0) {
                arm_mouse_slots(dlg, *data);
                const std::string_view text{field_buffer(dlg), field_length(dlg)};
                const u8 cursor = data->cursor_position;
                const size_t start = text::prev_sequence_start(text, cursor);
                delete_text(dlg, start, cursor - start);
                return 1;
            }
            set_field_font(dlg);
            const bool accepted = evt->virtual_key >= 0x20 && evt->virtual_key != 0x7F
                && (evt->virtual_key < 0x80 || filenames_take_utf8()
                    || dlg->interaction_function != helper::char_name_popup_textbox_interact);
            if (accepted) {
                arm_mouse_slots(dlg, *data);
                insert_byte(dlg, u8(evt->virtual_key));
                TextInput::instance().note_focus_gained(dlg);
            }
            set_font_handle_impl(nullptr);
            return accepted;
        }
        default:
            return 0;
    }
}

u32 __stdcall copy_game_name(char* to, const char* from, u32 capacity) {
    return u32(text::copy_truncated(to, capacity, from));
}

void save_name_insert_char_impl(DialogEvent& event, bw::Dialog& dlg) {
    save_name_char(event, dlg, true, refresh_save_name_list);
}

void save_game_insert_char_impl(DialogEvent& event, bw::Dialog& dlg) {
    save_name_char(event, dlg, false, refresh_save_game_list);
}


void click_cursor_pos_update(bw::Dialog* dlg, DialogEvent* evt) {
    if (!dlg || !dlg->parent || !evt) {
        return;
    }
    bw::Dialog* root = dlg->parent;
    if (root->type != DialogControlType::DialogBoxDC) {
        root = root->parent;
    }
    if (!root) {
        return;
    }
    const s16 rel_x = s16(evt->current.x) - root->bounds.left();
    const s16 rel_y = s16(evt->current.y) - root->bounds.top();
    if (rel_x < dlg->bounds.left() || rel_x > dlg->bounds.right()
        || rel_y < dlg->bounds.top() || rel_y > dlg->bounds.bottom()) {
        return;
    }
    const s32 click_x = rel_x - dlg->bounds.left();

    char* const text = field_buffer(dlg);
    bw::TextInputData* const data = field_data(dlg);
    if (!text || !data) {
        return;
    }
    set_field_font(dlg);
    const std::string_view view{text, field_length(dlg)};
    size_t caret = view.size();
    if (click_x < 1) {
        caret = 0;
    } else {
        const s32 width = font_max_line_width_impl((const u8*)text);
        if (width > 0 && click_x < width) {
            caret = view.size() * size_t(click_x) / size_t(width);
        }
    }
    data->cursor_position = u8(text::snap_to_sequence_start(view, std::min(caret, view.size())));
}


void draw_textbox_content(bw::Dialog* dlg) {
    char local[256];
    bw::TextInputData* const data = field_data(dlg);
    char* const text = field_buffer(dlg);
    if (!dlg || !data || !text) {
        return;
    }

    set_field_font(dlg);


    DialogFlags::Enum flags = dlg->flags;
    const bool can_highlight = !(flags & DialogFlags::Disabled)
        && (flags & DialogFlags::Visible);
    bw::MenuData* const menu = dlg->parent ? dlg->parent->data<bw::MenuData>() : nullptr;
    const bw::Dialog* const focused = menu ? menu->focused_control : nullptr;
    if (can_highlight
        && (dlg == focused
            || (!focused && dlg->type == DialogControlType::DefaultButton
                && dlg->parent && !(dlg->parent->flags & DialogFlags::UseAlternateStyle)))) {
        flags |= DialogFlags::RenderWithFocusHighlight;
    } else {
        flags &= ~DialogFlags::RenderWithFocusHighlight;
    }
    dlg->flags = flags;
    
    if ((flags & DialogFlags::RenderWithFocusHighlight)
        && !(flags & DialogFlags::RemoveStyles)) {
        *g_draw_color_index = *palette_index_ref;
        helper::bw_draw(dlg->bounds.top(), dlg->bounds.left(),
            dlg->bounds.right() - dlg->bounds.left() + 1,
            dlg->bounds.bottom() - dlg->bounds.top() + 1);
    }

    if (data->flags & bw::ScrollToCursorOnDraw) {
        textbox_scroll_to_cursor_impl(dlg->to_bindlg());
    }

    const size_t length = field_length(dlg);
    const size_t scroll = std::min<size_t>(data->scroll_offset, length);
    const size_t copied = text::truncate_to_boundary({text + scroll, length - scroll}, 252);
    std::copy_n(text + scroll, copied, local);
    local[copied] = '\0';

    if (flags & DialogFlags::DrawActiveState) {
        size_t caret = data->cursor_position;
        caret = caret > scroll ? caret - scroll : 0;
        caret = std::min(caret, copied);
        // Cursor triple: color select, cursor glyph, color restore
        std::move(local + caret, local + copied + 1, local + caret + 3);
        local[caret] = 0x04;
        local[caret + 1] = 0xA6;
        local[caret + 2] = 0x01;
    }

    u8 color = data->flags >> 4;
    if (!color) {
        color = 2;
    }
    helper::get_color_handle(TextColor(color));

    g_draw_text = dlg->bounds;
    g_word_wrap_mode = 0;
    g_print_xy_alignment = 0x11;
    bw_print_xy_impl(local, dlg->bounds.left() + data->horizontal_margin,
        dlg->bounds.top() + data->vertical_offset);
    g_word_wrap_mode = 1;
    set_font_handle_impl(nullptr);
}

} // namespace hooks