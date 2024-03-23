#pragma once
#include "cm.h"

// Note: No namespace, since those are pretty generic

Box32 box32_rect(Point16 pos, u32 w, u32 h);
Box32 box32_square(Point16 pos, u32 w);

bool is_hostile_or_neutral(u32 player, u32 other_player);
void replay_witness_event(Unit* unit, SpectatorEvent event_type);
u32 get_construction_graphic(Unit* unit);
void init_omnipresence_state();
void disconnect_addon_no_event(Unit* unit);
u32 get_bullet_damage(Bullet* bullet, u32 weapon_id);