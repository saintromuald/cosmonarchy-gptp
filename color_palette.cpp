#include "color_palette.h"

#define PaletteSize 256
#pragma pack(push, 1)

struct RGBA {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};

struct Palette {
	RGBA colors[PaletteSize];
};

#pragma pack(pop)

static Palette* const init_palette = (Palette*)0x005994E0;
static Palette* const game_palette = (Palette*)0x006CEB40;

Recolor default_recolor() {
	Recolor recolor = {};
	recolor.r = 1;
	recolor.g = 1;
	recolor.b = 1;
	recolor.gamma = 1;
	recolor.saturation = 1;
	return recolor;
}

RecolorSet default_recolor_set() {
	RecolorSet set = {};
	set.count = 1;
	set.recolors[0] = default_recolor();
	return set;
}

f32 clamp(f32 value, f32 min, f32 max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

f32 lerp(f32 t, f32 a, f32 b) {
	return a * (1 - t) + b * t;
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r          = lerp(t, a.r,			 b.r);
	a.g          = lerp(t, a.g,			 b.g);
	a.b          = lerp(t, a.b,			 b.b);
	a.gamma      = lerp(t, a.gamma,		 b.gamma);
	a.saturation = lerp(t, a.saturation, b.saturation);
	return a;
}

Recolor lerpn_recolor(f32 t, Recolor* recolors, u32 count) {
	if (count == 0) {
		return default_recolor();
	}
	if (count == 1) {
		return recolors[0];
	}
	u32 index = clamp(t * count, 0, count - 2);
	t -= index * (1.0 / count);
	t *= count;
	return lerp_recolor(t, recolors[index], recolors[index + 1]);
}

static u8 mix_color(f32 base, f32 blend, f32 saturation, f32 brightness) {
	base = lerp(saturation, brightness, base);
	return clamp(base * blend, 0, 1) * 255;
}

void set_palette_recolor(RecolorSet set) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		f32 brightness = (init_palette->colors[i].r + init_palette->colors[i].g + init_palette->colors[i].b) / 3.0 / 255.0;
		Recolor recolor = lerpn_recolor(brightness, set.recolors, set.count);
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r / 255.0, recolor.r * recolor.gamma, recolor.saturation, brightness);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g / 255.0, recolor.g * recolor.gamma, recolor.saturation, brightness);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b / 255.0, recolor.b * recolor.gamma, recolor.saturation, brightness);
	}
	helper::update_palette();
}