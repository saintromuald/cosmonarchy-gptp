#pragma once
#include "BW\bwdata.h"

bool is_any_corpse(Thingy* thingy);
bool is_inactive_corpse(Thingy* thingy);
bool is_active_corpse(Thingy* thingy);
void consume_corpse(Thingy* corpse);
Thingy* get_closest_active_corpse(Point16 position, u32 radius);
Unit* reclaim_corpse(Thingy* corpse, u32 player);
void avenge_corpse(Thingy* corpse, u32 player);
void ascend_corpse(Thingy* corpse, u32 player);
void ascend_in_area(u32 player, Point16 position, u32 range);
void avenge_or_ascend_corpse(Thingy* corpse, u32 player);
void avenge_corpses_in_area(Point16 position, u32 radius, u32 player);
void ascend_corpses_in_area(Point16 position, u32 radius, u32 player);
void avenge_or_ascend_corpses_in_area(Point16 position, u32 radius, u32 player);
u32 unit_avenges_or_ascends(Unit* unit); // Returns (source_player + 1) if should avenge/ascend, returns 0 otherwise 
void incubate(Unit* target, u32 count, u32 unit_id, u32 player);
void set_custom_egg(Unit* egg, u32 unit_id, u32 time, u32 hp = 150 * 256);
void fire_secondary_melee_weapon_if_needed(Unit* unit, u32 weapon_id);
bool can_infest(u32 player, Unit* target);
void mark_infesting(u32 player, Unit* target);
void blissful_embrace_tick(Unit* target);
void blissful_embrace(Unit* target, u32 player);
void empress_zombify(u32 player, Unit* source, Unit* target, u32 stacks);
void mind_control(u32 player, Unit* source, Unit* target);
void recede_mind_control(Unit* target);
void spread_debuffs(Unit* from, Unit* to);