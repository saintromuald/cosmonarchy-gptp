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
	set.shadows = default_recolor();
	set.midtones = default_recolor();
	set.highlights = default_recolor();
	return set;
}

// Todo: move those static function to maths file
static f32 clamp(f32 value, f32 min, f32 max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static f32 lerp(f32 t, f32 a, f32 b) {
	return a * (1 - t) + b * t;
}

static f32 lerp3(f32 t, f32 a, f32 b, f32 c) {
	if (t < 0.5) {
		return lerp(t * 2, a, b);
	}
	return lerp((t - 0.5) * 2, b, c);
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r          = lerp(t, a.r,			 b.r);
	a.g          = lerp(t, a.g,			 b.g);
	a.b          = lerp(t, a.b,			 b.b);
	a.gamma      = lerp(t, a.gamma,		 b.gamma);
	a.saturation = lerp(t, a.saturation, b.saturation);
	return a;
}

Recolor lerp3_recolor(f32 t, Recolor a, Recolor b, Recolor c) {
	a.r          = lerp3(t, a.r,		  b.r,          c.r);
	a.g          = lerp3(t, a.g,		  b.g,          c.g);
	a.b          = lerp3(t, a.b,		  b.b,          c.b);
	a.gamma      = lerp3(t, a.gamma,	  b.gamma,      c.gamma);
	a.saturation = lerp3(t, a.saturation, b.saturation, c.saturation);
	return a;
}

static u8 mix_color(f32 base, f32 blend, f32 saturation, f32 brightness) {
	base = lerp(saturation, brightness, base);
	return clamp(base * blend, 0, 1) * 255;
}

void set_palette_recolor(RecolorSet set) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		f32 brightness = (init_palette->colors[i].r + init_palette->colors[i].g + init_palette->colors[i].b) / 3.0 / 255.0;
		Recolor recolor = lerp3_recolor(brightness, set.shadows, set.midtones, set.highlights);
		game_palette->colors[i].r = mix_color(init_palette->colors[i].r / 255.0, recolor.r * recolor.gamma, recolor.saturation, brightness);
		game_palette->colors[i].g = mix_color(init_palette->colors[i].g / 255.0, recolor.g * recolor.gamma, recolor.saturation, brightness);
		game_palette->colors[i].b = mix_color(init_palette->colors[i].b / 255.0, recolor.b * recolor.gamma, recolor.saturation, brightness);
	}
	helper::update_palette();
}