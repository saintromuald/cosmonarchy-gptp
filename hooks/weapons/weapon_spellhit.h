#pragma once
#include <BW/structures/Unit.h>
#include <BW/structures/Bullet.h>
#include "BW\api.h"

namespace hooks
{
	void irradiate_hit(Unit* attacker, Point16 target, u8 attacking_player_id);		// 00454E00
	void irradiate_hit_internal(Unit* attacker, Point16 target, u8 attacking_player_id);
	void optical_flare_hit(Unit* target, u32 attacking_player_id, Bullet* bullet);	// 00455170
	void restore_hit(Unit* target, Bullet* bullet);									// 00455230
	void lockdown_hit(Unit* target, u8 previousLockdownTimer, Bullet* bullet);		// 00455380
	void weapon_bullet_hit(Bullet* bullet, Unit* target, u32 damage_divisor);		// 00479AE0
	void iscript_attackmelee(Unit* attacker);										// 00479B40
	void emp_shockwave_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet);			// 00492BA0
	void corrosive_acid_hit(u32 attacking_player_id, s32 x, s32 y, Bullet* bullet);	// 004F4770
	void consume_hit(Unit* attacker, Unit* target);									// 004F47B0
	void ensnare_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet);					// 004F4830
	void dark_swarm_hit(s32 x, s32 y, u32 attacking_player_id, Bullet* bullet);		// 004F48A0
	void carapace_swarm_hit_internal(s32 x, s32 y, u32 attacking_player_id, u32 spell_unit_id);
	void broodling_hit(Unit* attacker, Unit* target, Bullet* bullet);				// 004F4940
	void plague_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet);					// 004F4B30
	void maelstrom_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet);				// 004F6850
	void mind_control_hit(Unit* attacker, Unit* target);							// 004F6910
	void stasis_field_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet);			// 004F6A80
	void stasis_field_hit_internal(Unit* attacker, Point16 position, Bullet* bullet);
	void disruption_web_hit(u32 attacking_player_id, s32 x, s32 y, Bullet* bullet);	// 004F6AF0
	Unit* create_disruption_web(u32 player, s32 x, s32 y);
	s32 ensnare_proc(Unit* target, Unit* attacker);									// 004F46C0
	void create_scanner(Unit* unit);												// 00464E40
	u8 stasis_field_overlay(Unit* unit, u8 timer);									// 004F67B0
	char disable_unit(Unit* unit);													// 00492CC0
	bool maelstrom_proc(Unit* target, Unit* source);								// 004F6760
	Unit* iterate_units_at_location_target_proc(Box16* location,					// 004E8280
		Unit*(__fastcall* proc)(Unit*, Unit*),
		Unit* unit);
	b32 give_unit_to_player(Unit* unit, u32 player);								// 004C8040
	void set_maelstrom_timer(Unit* unit, u8 duration);								// 004553F0

	void inject_weapon_spell_hit_hooks();
}

// Hooks could be confirmed thanks to 0048B770 CBullet_Damage that call almost all of them
// Hit features not supported by these hooks may be hardcoded in that function (like the Matraleth Parasite hit effect)