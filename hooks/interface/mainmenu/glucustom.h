#pragma once
#include "SCBW/structures.h"

u8 tFontGame_ImageData[320];//Larger tfont.pcx buffer to fit the new colors.
namespace hooks
{
	void display_establishing_shot();					// 004D9200, hooked by X405
	void cmdact_game_speed(BinDlg* control);			// 00481EF0

	void inject_glucustom_hooks();

	// DarkenedFantasies \\

	void sub_41D160();
	void sub_4EF600();
	void sub_4EF710();
	void loadMenu_gluScore_pIcons();
	void get_color_handle();
	void writeFontApplyColour_patch();
	void StatHangarCount_patch_25471();
	void RestartGameMenu();
	void sub_490E90_patch_90EC3();
	void sub_490E90_patch_90ED1();
	void UpdateOKButton2();
	void sub_490E90_patch_90EDF();
	void sub_4AABB0_new();
	void minimap_style();
}