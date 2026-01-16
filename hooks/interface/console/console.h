#pragma once
#include "BW/structures.h"
#include "BW/enumerations.h"
#include "BW/ui.h"

#include <memory>

class ConsoleIcons {
public:
    GrpFile& mineral() { return m_mineral.get(); };
    GrpFile& gas() { return m_gas.get(); };
    GrpFile& supply() { return m_supply.get(); };
    GrpFile& general() { return m_general.get(); };
private:
    LoadedFile<GrpFile> m_mineral{"game\\mineralicons.grp"};
    LoadedFile<GrpFile> m_gas{"game\\gasicons.grp"};
    LoadedFile<GrpFile> m_supply{"game\\supplyicons.grp"};
    LoadedFile<GrpFile> m_general{"game\\icons.grp"};
};

inline u16 race_icon_offset(RaceId::Enum race) {
    switch (race) {
    case RaceId::Zerg: return 0;
    case RaceId::Terran: return 1;
    case RaceId::Protoss: return 2;
    case RaceId::Askosi: return 3;
    }
    return 1;
}

// TODO: Convert this to a unique_ptr and create/reset with console load/unload
inline ConsoleIcons g_console_icons{};


void load_statres();            // 004e5990


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

void load_wireframe_palette();  // 00456a90
void load_statf10();            // 004f5300
void load_console();            // 004c3bb0 WIP

void load_statfluf_mouse_regions();     // 004f4dc0
void unload_statfluf_mouse_regions();   // 004f4cf0

void __stdcall conover_filename(char(&buffer)[260]); // 0045f0b0

void set_text_string(BinDlg* a1);       // 004263E0
void inject_console_hooks();
}