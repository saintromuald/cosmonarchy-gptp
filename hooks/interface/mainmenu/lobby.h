#pragma once
#include "BW/structures.h"

namespace hooks
{
    bool init_game();                                                           // 004EE110
    void start_game(s32 player);                                                // 00472060
    void init_singleplayer_custom(BinDlg* dialog);                              // 004AD850
    void on_player_join(u32 player_id, BinDlg* dialog, u32 data_1, u32 data_2); // 00470720

    void inject_lobby_hooks();
}