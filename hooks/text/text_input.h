#pragma once
#include "types.h"

#include <optional>
#include <string>
#include <string_view>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imm.h>

struct DialogEvent;
namespace bw {
struct Dialog;
}

// Direct typing arrives as UTF-16 through the TranslateMessage IAT hook; IME messages arrive through the main window's ANSI subclass
class TextInput {
public:
    static TextInput& instance() {
        static TextInput instance;
        return instance;
    }

    TextInput(const TextInput&) = delete;
    TextInput& operator=(const TextInput&) = delete;

    void ensure_installed();
    void teardown();

    // Only associate IME when text input has focus so hotkeys still work
    void reconcile_association();


    bw::Dialog* focused_text_input();

    // Tracks focus for roots that don't store it
    void note_focus_gained(bw::Dialog* field);
    void note_focus_lost(bw::Dialog* field);

    // Rejected updates are reverted. handled_elsewhere reports bytes consumed without insertion, such as chat input at its cap
    bool insert_bytes_verified(bw::Dialog* owner, std::string_view bytes, u16 modifiers, bool* handled_elsewhere = nullptr);

    // Prevents the next IME update from reinserting the phrase into a newly focused field
    void complete_composition(bw::Dialog* field);

    // Field clicks must complete composition before moving the caret
    void complete_active_composition();

    static BOOL WINAPI translate_message(const MSG* message);

private:
    TextInput() = default;

    struct Composition {
        bw::Dialog* owner = nullptr;
        u16 start = 0;            // Byte offset in the owner's buffer
        u16 bytes = 0;
        u16 recorded_cursor = 0;  // Validation snapshot from the last update
        u16 recorded_length = 0;  // Validation snapshot from the last update
        size_t caret_offset = 0;  // Byte offset within the range, for candidate positioning
        std::wstring text;        // Retained for cursor-only updates
        bool busy = false;        // ImmNotifyIME calls reenter synchronously
    };

    static LRESULT CALLBACK subclass_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handle_ime_composition(HWND window, LPARAM lparam);
    LRESULT handle_ime_endcomposition();
    void on_char(MSG& message);
    std::optional<char32_t> take_codepoint(u16 unit);
    void reset_composition();
    void revert_recorded_range();
    bool feed_composition_bytes(bw::Dialog* owner, std::string_view bytes);
    POINT field_caret_surface_position(bw::Dialog* field);
    void update_candidates(HIMC context);
    void associate_default_context();

private:
    bool m_alive = true;
    bool m_installed = false;
    bool m_associated = false;        // Avoids ImmGetContext probes
    HIMC m_default_context = nullptr; // Captured before the first disassociation
    BOOL m_ime_open = TRUE;           // Korean installs start disassociated. Default to Hangul when a field first takes focus
    WNDPROC m_previous_wndproc = nullptr;
    bw::Dialog* m_recorded_field = nullptr;
    u16 m_pending_surrogate = 0;
    bool m_lone_surrogate_logged = false;
    bool m_ansi_char_dropped_logged = false;
    bool m_no_field_composition_logged = false;
    bool m_chat_walk_logged = false;
    bool m_flagged_root_logged = false;
    bool m_focus_non_text_logged = false;
    bool m_ime_setcontext_logged = false;
    Composition m_composition;
};

namespace hooks {

u32 __fastcall textbox_interact(bw::Dialog* dlg, DialogEvent* evt);

void click_cursor_pos_update(bw::Dialog* dlg, DialogEvent* evt);

void draw_textbox_content(bw::Dialog* dlg);

// Save names are ASCII-only unless the process uses the UTF-8 code page
void save_name_insert_char_impl(DialogEvent& event, bw::Dialog& dlg);
void save_game_insert_char_impl(DialogEvent& event, bw::Dialog& dlg);

// GameData's 24-byte name buffer must truncate at a codepoint boundary
u32 __stdcall copy_game_name(char* to, const char* from, u32 capacity);

void inject_text_input_hooks();
void teardown_text_input_hooks();
} // namespace hooks