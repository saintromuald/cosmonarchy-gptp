#pragma once
#include "BW/bwdata.h"

extern u16 box_width;
extern u16 box_height;

namespace hooks {
	void minimapVisionUpdate_256();																// 004A3A40
	void minimapVisionUpdate_128();																// 004A3B30
	void minimapVisionUpdate_64();																// 004A3C00
	void minimapVisionUpdate_staircase();
	void minimapSurfaceUpdate_256();															// 004A41B0
	void minimapSurfaceUpdate_128();															// 004A4240
	void minimapSurfaceUpdate_64();																// 004A42B0

	void __cdecl set_map_size_constants();														// 004A4400
	void minimap_ping(s32 posX, s32 posY, u8 color);											// 004A34C0
	void __fastcall minimap_image_update(BinDlg* minimap, int dummy0, int dummy1, int dummy2);	// 004A3EE0
	void draw_unit_box(s32 posX, u8 color, s32 posY, u32 sizeX, u32 sizeY, bool is_building);	// 004A3FD0
	void minimap_game_mouse_update(BinDlg* minimap);											// 004A4D20

	void inject_minimap_hooks();
}