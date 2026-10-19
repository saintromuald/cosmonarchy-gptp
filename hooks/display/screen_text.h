#pragma once
#include "../../types.h"

namespace hooks {

constexpr s32 MessageLogLeft = 10;
constexpr s32 MessageLogTop = 250;
constexpr s32 MessageLogRight = display::SurfaceWidth / 2;

constexpr s32 DisplayCenter = display::SurfaceWidth / 2;
constexpr s32 CenterTextLeft = DisplayCenter - 200;
constexpr s32 CenterTextTop = 690;
constexpr s32 CenterTextRight = DisplayCenter + 200;

constexpr s32 TopRightTextLeft = 420;
constexpr s32 TopRightTextTop = 24;
constexpr s32 TopRightTextRight = 620;

constexpr u8 MessageLogPrependByte = std::to_underlying(TextColor::Orange);
constexpr u8 CenterTextAlignment = std::to_underlying(TextColor::AlignRight);
constexpr u8 TopRightTextAlignment = 0x14;

constexpr u32 TextSlotSize = 218;
constexpr u32 MessageLogSlotCount = 11;
constexpr u32 TopRightTextSlot = 11;  // Never populated by vanilla
constexpr u32 CenterTextSlot = 12;    // Aliases draw_out_text_left

// 0048CD30 reimplemented to use new word-wrap color logic
void display_text_message(const char* text, u16 packed_color, u32 ticks, u8 from_trigger);

// 0048CE90, jump-patched. Runs once at game start and sizes the message log around the
// current font: each line's clip band is the font height plus line spacing, and the
// per-line step is padded so the log fills the span from MessageLogTop to CenterTextTop.
void __cdecl init_screen_text_layout();

// 004BD614 (call to 0048CF60). Draws or defers the engine's screen text pass - the message
// log ring, the center notification line, the top-right line, and the UI features hooked
// onto its exit - so text is drawn post-resample rather than magnified/cropped with the
// world. The 0048CF60 body is reimplemented in screen_text.cpp using the layout constants.
void draw_screen_text(); // 004BD614, the game layer's call into 0048CF60

// Draws deferred screen text between world layers and UI, after resampling.
void draw_deferred_screen_text();

void inject_screen_text_hooks();

} // hooks