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

// Todo: move those functions to maths file
f32 clamp(f32 value, f32 min, f32 max);
f32 lerp(f32 t, f32 a, f32 b);

Recolor default_recolor();
RecolorSet default_recolor_set();
Recolor lerp_recolor(f32 t, Recolor a, Recolor b);
Recolor lerpn_recolor(f32 t, Recolor* recolors, u32 count);
void set_palette_recolor(RecolorSet set);