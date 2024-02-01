#include "color_palette.h"

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

// Todo: move those static functions to maths file
static f32 clamp(f32 value, f32 min, f32 max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static f32 lerp(f32 t, f32 a, f32 b) {
	return a * (1 - t) + b * t;
}

static f32 smoothstep(f32 t, f32 a, f32 b) {
	f32 ease_in = t * t;
	f32 ease_out = 1 - (1 - t) * (1 - t);
	f32 ease_in_out = lerp(t, ease_in, ease_out);
	return lerp(ease_in_out, a, b);
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r          = smoothstep(t, a.r,			 b.r);
	a.g          = smoothstep(t, a.g,			 b.g);
	a.b          = smoothstep(t, a.b,			 b.b);
	a.gamma      = smoothstep(t, a.gamma,		 b.gamma);
	a.saturation = smoothstep(t, a.saturation,   b.saturation);
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
	base = smoothstep(saturation, brightness, base);
	return clamp(base * blend, 0, 1) * 255;
}

void set_palette_recolor(RecolorSet set) {
	for (u32 i = 0; i < PaletteSize; ++i) {
		RGBA color = init_palette->colors[i];
		f32 brightness = (color.r + color.g + color.b) / 3.0 / 255.0;
		Recolor recolor = lerpn_recolor(brightness, set.recolors, set.count);

		RGBA* out = &game_palette->colors[i];
		out->r = mix_color(color.r / 255.0, recolor.r * recolor.gamma, recolor.saturation, brightness);
		out->g = mix_color(color.g / 255.0, recolor.g * recolor.gamma, recolor.saturation, brightness);
		out->b = mix_color(color.b / 255.0, recolor.b * recolor.gamma, recolor.saturation, brightness);
	}
	helper::update_palette();
}