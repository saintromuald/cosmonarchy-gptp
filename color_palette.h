#pragma once
#include "SCBW/api.h"

struct Recolor {
	f32 r;
	f32 g;
	f32 b;
	f32 gamma;
	f32 saturation;
};

struct RecolorSet {
	Recolor shadows;
	Recolor midtones;
	Recolor highlights;
};

Recolor default_recolor();
RecolorSet default_recolor_set();
Recolor lerp_recolor(f32 t, Recolor a, Recolor b);
Recolor lerp3_recolor(f32 t, Recolor a, Recolor b, Recolor c);
void set_palette_recolor(RecolorSet set);