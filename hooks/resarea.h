#pragma once
#include "BW/structures.h"
#include "ai_layouts.h"

namespace hooks {
u32 __fastcall get_unit_hp_or_resarea(Unit* unit, ChkUnitSettings* settings);
b32 __fastcall encode_start_location(ChkUnit* chk_unit, v2u16* position); // 004CB190
void create_resareas(); // 00446350

void inject_resarea_hooks();
} // namespace hooks