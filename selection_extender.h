//Header
#pragma once
#include "scbw/structures.h"
#include "hooks/limits/ccmu.h"
#include "graphics/Font.h"

extern graphics::Font* grpWireFont;
void init_grpWireFont(char* font_name);

namespace hooks {
	void inject_selection_extender();
}