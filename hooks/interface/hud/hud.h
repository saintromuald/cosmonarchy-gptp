#pragma once
#include "BW/structures.h"
#include "BW/enumerations.h"
#include "BW/ui.h"
#include "hud_manager.h"
#include "storm/sfile.h"

// Only used for other players' races in the statres dialog
// TODO: refactor statres to reflect individual values as defined in huds.yml -Jesse
inline u16 default_race_icon_index(RaceId::Enum race) {
    switch (race) {
        case RaceId::Zerg: return 0;
        case RaceId::Terran: return 1;
        case RaceId::Protoss: return 2;
        case RaceId::Askosi: return 3;
    }
    return 1;
}

inline HudIcons g_hud_icons{}; // TODO: make this a member of ConsoleManager during statres refactor -Jesse

void load_statres(); // 004e5990

namespace hooks {
void load_race_grp_and_sounds(); // 004ee2d0

void load_progressbar(BinDlg* dialog, BinDlgProc interaction_function, Bitmap& empty, Bitmap& full); // 004596a0 & 00458050
void __stdcall conover_filename(char(&buffer)[260]); // 0045f0b0
void set_text_string(BinDlg* a1);       // 004263E0

void load_hud();            // 004c3bb0
void unload_hud();          // 004c3780

void inject_hud_hooks();
}