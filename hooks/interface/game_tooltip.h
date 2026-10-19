#pragma once
#include "types.h"

namespace graphics { struct Bitmap; }
namespace bw { struct Dialog; }

// BW called original in-game tooltips "context help" from CtxtHelp.cpp
namespace hooks {

void show_game_tooltip(s32 x, s32 y, const char* text);                                  // 00481510

// Called by the command button hover handlers (00459770, 004597C0, 00459860) with the control under the mouse
void update_command_tooltip(bw::Dialog* buttonset, bw::Dialog* hovered);                 // 004593A0

// present_dialog_control (0041C810) steps for dialog bitmaps overlapping the tooltip
void save_under_game_tooltip(graphics::Bitmap* dialog_bitmap, s32 dialog_left, s32 dialog_top); // 00481260
void __stdcall stamp_game_tooltip(graphics::Bitmap* dialog_bitmap);                      // 004811E0
void __stdcall restore_under_game_tooltip(graphics::Bitmap* dialog_bitmap);              // 00481160

void inject_game_tooltip_hooks();

} // namespace hooks
