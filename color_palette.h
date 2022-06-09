#pragma once
#include "SCBW/api.h"

struct Recolor {
	f32 r;
	f32 g;
	f32 b;
	f32 gamma;
};

Recolor lerp_recolor(f32 t, Recolor a, Recolor b);
void set_palette_recolor(Recolor shadows, Recolor highlights);