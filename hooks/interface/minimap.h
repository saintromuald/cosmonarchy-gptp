#pragma once
#include "BW/bwdata.h"

extern u16 box_width;
extern u16 box_height;

namespace hooks {
	void minimapVisionUpdate_256();																//0x004A3A40
	void minimapVisionUpdate_128();																//0x004A3B30
	void minimapVisionUpdate_64();																//0x004A3C00
	void minimapVisionUpdate_staircase();
	void minimapSurfaceUpdate_256();															//0x004A41B0
	void minimapSurfaceUpdate_128();															//0x004A4240
	void minimapSurfaceUpdate_64();																//0x004A42B0
	void minimapSurfaceUpdate_staircase();
	void __cdecl setMapSizeConstants();															//0x004A4400
	void minimap_ping(s32 posX, s32 posY, u8 color);											//0x004A34C0
	void __fastcall MinimapImageUpdate(BinDlg* minimap, int dummy0, int dummy1, int dummy2);	//0x004A3EE0
	void draw_unit_box(s32 posX, u8 color, s32 posY, u32 sizeX, u32 sizeY, bool isBuilding);	//0x004A3FD0
	void minimapGameMouseUpdate(BinDlg* minimap);												//0x004A4D20

	void inject_minimap_hooks();
}