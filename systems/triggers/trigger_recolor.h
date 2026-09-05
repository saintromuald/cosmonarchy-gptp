#pragma once
#include "trigger_system.h"

struct TimedRecolor {
	RecolorSet set;
	f32 time;
};

#define MaxRecolors 32
struct RecolorList {
	u32 index;
	u32 count;
	TimedRecolor recolors[MaxRecolors];
};

struct RecolorSystem {
	f32 recolor_time;
	TimedRecolor source_recolor;
	TimedRecolor current_recolor;
	TimedRecolor target_recolor;
	RecolorList recolor_list;
};

extern RecolorSystem recolor_system;

void init_recolors();
void update_recolors();