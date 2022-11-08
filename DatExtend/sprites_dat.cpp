#include <SCBW/api.h>
#include <hook_tools.h>
#include "datext.h"

namespace {
    // List of address reference replacements.
    s32 sprites_r[] = {/* image */   0,    0x00463A87 +4, 0x004683D5 +4, 0x0048D765 +4, 0x0048DA9F +4, 0x0049907D +4, 0x004D7A58 +4, 0x004D7ABB +4, 0x004E4CC5 +4, 0x004E4FDB +4, 0x004E5018 +4, 0x004E6640 +4,  0, 0,
                       /* health */  /*-130*/0, 0x0047A871 +2, 0x0047A90D +2, 0x0047AA6B +3, 0x004D6043 +2,  0, 0,
                       /* unknown */ 0,    0, 0,
                       /* visible */ 0,    0x0049906B +2,  0, 0,
                       /* sel img */ /*-130*/0, 0x004D601E +3, 0x004D681A +4,  0, 0,
                    /* sel ofs */ /*-130*/0, 0x004D6032 +2, 0x004D6840 +2,  0, 0};

    u32 sprite_is_valid[] = { 0x0042D515 +2, 0x00497157 +2, 0x0049717E +2, 0x004971A5 +2, 0x004971CC +2, 0x004971F3 +2, 0x0049721A +2, 0x00497C64 +2, 0 };
}

namespace DatExt {
    void sprites_dat_doPatch(u32 new_count) {
        datTablePatch((DatLoad*)LoadTable::Sprites_Dat, sprites_r);

        u32 corrected_count = (new_count + 5) / 6; // has to be divisible by 6, because loop at 0x00497110 iterates 6 entries at a time
        g_is_valid_sprite = (u8*)calloc(corrected_count, sizeof(u8));
        simple_replace(sprite_is_valid, (u32)g_is_valid_sprite);
        memory_patch(0x00497223 +1, corrected_count);
    }

    void sprites_dat_unpatch() {
        if (g_is_valid_sprite) {
            free(g_is_valid_sprite);
        }
    }
}