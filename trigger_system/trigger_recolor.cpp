#include "trigger_recolor.h"
#include "color_palette.h"
#include "maths.h"

RecolorSystem recolor_system;

void init_recolors() {
	memset(&recolor_system, 0, sizeof(recolor_system));
	recolor_system.current_recolor.set = default_recolor_set();
	set_palette_recolor(recolor_system.current_recolor.set);
}

void update_recolors() {
	if (recolor_system.recolor_time < recolor_system.target_recolor.time) {
		recolor_system.recolor_time += 1.0 / 24; // Tick Time for Brood War

		f32 t = recolor_system.recolor_time / recolor_system.target_recolor.time;
		if (t > 1) {
			t = 1;
		}

		for (u32 i = 0; i < recolor_system.current_recolor.set.count; ++i) {
			recolor_system.current_recolor.set.recolors[i] = lerp_recolor(ease_in_out(t), recolor_system.source_recolor.set.recolors[i], recolor_system.target_recolor.set.recolors[i]);
		}

		set_palette_recolor(recolor_system.current_recolor.set);
	} else if (recolor_system.recolor_list.count) {
		recolor_system.recolor_list.index %= recolor_system.recolor_list.count; // In case the list got smaller

		recolor_system.recolor_time = 0;
		recolor_system.source_recolor = recolor_system.current_recolor;
		recolor_system.target_recolor = recolor_system.recolor_list.recolors[recolor_system.recolor_list.index];

		recolor_system.recolor_list.index += 1;
		recolor_system.recolor_list.index %= recolor_system.recolor_list.count;
	}
}