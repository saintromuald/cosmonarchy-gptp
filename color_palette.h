#pragma once
#include "SCBW/api.h"

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
Recolor lerp3_recolor(f32 t, Recolor a, Recolor b, Recolor c);
Recolor lerpn_recolor(f32 t, Recolor* recolors, u32 count);
void set_palette_recolor(RecolorSet set);