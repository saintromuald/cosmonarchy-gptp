#pragma once
#include "BW/structures.h"

u8 g_tfontgame_image_data[328]; // Larger tfont.pcx buffer to fit the new colors.
u16 display_text_color[11];
namespace hooks {
    void display_establishing_shot();														// 004D9200, hooked by X405
    void cmdact_game_speed(BinDlg* control);												// 00481EF0
    void hide_dialog(BinDlg* dialog);														// 00418700
    void __cdecl create_main_window();														// 004E05B0
    void initialize_game_type_list(BinDlg* list);											// 004AF400

    void inject_glucustom_hooks();

    // DarkenedFantasies \\

    // Score screen player color fixes
    void sub_41D160();
    void sub_4EF600();
    void sub_4EF710();
    void loadMenu_gluScore_pIcons();

    // Player-colored font
    void get_color_handle();
    void writeFontApplyColour_patch();
    
    // CTRL+R restart game hotkey
    void RestartGameMenu();
    
    // Diplomacy
    void sub_490300_patch_903CA();
    void sub_4AABB0_new();
    void sub_490E90_patch_90EC3();
    void UpdateOKButton2();
    void minimap_style();
    void minimap_dlg_Create_patch_A5242();
    void getColourID_new();
    void set_display_text_color();
    void get_display_text_color();

    // Preview minimap with UMS
    void preview_minimap_patch_EF066();
}