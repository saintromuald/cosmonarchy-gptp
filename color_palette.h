#pragma once
#include "BW/api.h"

#pragma pack(push, 1)
struct RGBA {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

#define PaletteSize 256
struct Palette {
	RGBA colors[PaletteSize];
};
#pragma pack(pop)

Palette* const init_palette = (Palette*)0x005994E0;
Palette* const game_palette = (Palette*)0x006CEB40;

struct Recolor {
	f32 r;
	f32 g;
	f32 b;
	f32 gamma;
	f32 saturation;
};

#define RecolorCap 16
struct RecolorSet {
	u8 count;
	Recolor recolors[RecolorCap];
};

Recolor default_recolor();
RecolorSet default_recolor_set();
Recolor lerp_recolor(f32 t, Recolor a, Recolor b);
Recolor lerpn_recolor(f32 t, Recolor* recolors, u32 count);
void set_palette_recolor(RecolorSet set);