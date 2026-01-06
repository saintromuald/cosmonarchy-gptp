#pragma once
#include "BW/structures.h"
#include "BW/bwhelper.h"
#include "BW/Storm.h"
#include "BW/ui.h"

#include <array>
#include <algorithm>
#include <numeric>
#include <format>
#include <string>
#include <unordered_map>
#include <span>

namespace hooks {
void load_racial_sounds(bool enable);   // 0048fb40
void load_race_grp_and_sounds();        // 004ee2d0
void load_console_image();              // 004c3950

static void load_progressbar(BinDlg* dialog, BinDlgProc interaction_function,
    Bitmap& empty, Bitmap& full, RaceId::Enum race); // 004596a0 & 00458050

void load_command_buttons();    // 00459b90
void load_textbox();            // 004f37c0
// void load_statport();        // 0045f320 Waiting to hook this due to datext changes
void load_statlb();             // 004bed70
void load_statres();            // 004e5990
void load_wireframe_palette();  // 00456a90
void load_statf10();            // 004f5300
void load_console();            // 004c3bb0 WIP

void load_statfluf_mouse_regions();     // 004f4dc0
void unload_statfluf_mouse_regions();   // 004f4cf0

void __stdcall conover_filename(char(&buffer)[260]); // 0045f0b0

void set_text_string(BinDlg* a1);       // 004263E0
void inject_console_hooks();
}