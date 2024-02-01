#include "color_palette.h"
#include "maths.h"

Recolor default_recolor() {
	return {
		.r = 1,
		.g = 1,
		.b = 1,
		.gamma = 1,
		.saturation = 1,
	};
}

RecolorSet default_recolor_set() {
	return {
		.count = 1,
		.recolors = { default_recolor() },
	};
}

Recolor lerp_recolor(f32 t, Recolor a, Recolor b) {
	a.r          = lerp(t, a.r,			 b.r);
	a.g          = lerp(t, a.g,			 b.g);
	a.b          = lerp(t, a.b,			 b.b);
	a.gamma      = lerp(t, a.gamma,		 b.gamma);
	a.saturation = lerp(t, a.saturation,   b.saturation);
	return a;
}

Recolor lerpn_recolor(f32 t, Recolor* recolors, u32 count) {
	if (count == 0) {
		return default_recolor();
	}
	if (count == 1) {
		return recolors[0];
	}
	t = ease_in_out(t);
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
