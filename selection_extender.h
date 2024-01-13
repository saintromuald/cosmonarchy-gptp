//Header
#pragma once
#include "scbw/structures.h"
#include "hooks/limits/ccmu.h"
#include "graphics/Font.h"

extern graphics::Font* grp_wire_Font;
void init_grp_wire_font(const char* font_name);

namespace hooks {
	void inject_selection_extender();
}