#pragma once
#pragma pack(1)
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <ostream>
#include <istream>
#include <assert.h>
#include "BW\api.h"
#include "data_ext.h"

#define ATTACK_LIMIT 64

struct IquareGlobals;

struct SaveState
{
	u32 len;
	const char* ptr;

	SaveState()
	{
		ptr = 0;
		len = 0;
	}
};

enum EffectId {
	EffectId_MorningstarGroundZero,
	EffectId_DiademIonField,
	EffectId_SalamanderFire,
	EffectId_Irradiate,
	EffectId_AspirantCriticality,
	EffectId_HallucinatingField,
	EffectId_LanifectDisruptionField,
	EffectId_StarSovereignTheGriefOfAllGods,
	EffectId_KeskathalorCausticDischarge,
	EffectId_IsthrathalethPlagueField,
	EffectId_AionReclamationField,
	EffectId_VilgolethHeal,
	EffectId_CyclopsRecursionRound,

	EffectId_Count,
};

struct Effect {
	u32 id;
	Point16 position;
	u32 radius;
	u32 current_time;
	u32 max_time;
	s32 sprite_id;
	u32 elevation;
	u32 player;
	Thingy* thingy;
	Unit* source;
	Unit* origin;
};

struct LineAttackHit {
	Unit* source;
	Unit* target;
};

struct MultihitBullet {
	Bullet* bullet;
	s32 divider;
	std::vector<Unit*> already_hit;

	MultihitBullet() : bullet(0), already_hit({}), divider(0) {}
	MultihitBullet(Bullet* bullet, std::vector<Unit*> already_hit, u32 divider) : bullet(bullet), already_hit(already_hit), divider(divider) {}
};

struct AttackExtender {
	//	std::vector<std::vector<u16>> attacks;
	u16 attacks[PLAYABLE_PLAYER_COUNT][ATTACK_LIMIT];
	AttackExtender()
	{

		for (s32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
		{
			for (s32 j = 0; j < ATTACK_LIMIT; j++)
			{
				attacks[i][j] = 0;
			}
		}
	}
};

struct IquareGlobals {
	std::string output;
	// Debugging data
/*	f64 unitSpeedDbg;
	f64 weaponDamageDbg;
	f64 gameHookDbg;
	f64 updateUnitStateDbg;
	f64 updateStatusEffectsDbg;*/
	// Serializable data
	u32 extended_sfx;
	bool tbl_extender;
	bool unitid_extender;

	std::vector<Effect> effects;
	std::vector<LineAttackHit> line_attack_hits;
	std::vector<MultihitBullet> multihit_bullets;
	std::unordered_map<Unit*, std::vector<Unit*>> lakizilisk_spine_hits;
	AttackExtender attackers;

	IquareGlobals() {
		extended_sfx = 0;
		tbl_extender = false;
		unitid_extender = false;

		refresh_debugging_info();
	}

	// non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	s32 flingy_init_bullet_id;
	u8 flingy_init_player_id;
	Bullet* bounce_bullet;
//	SaveState aise_save();
	void aise_save();
	void aise_load(const char* ptr, u32 len);
	void refresh_debugging_info();
	void reset();
	void frame_hook();
	void unit_death(Unit* unit);
	void remove_unit_from_line_attack_table(Unit* unit, bool source_only);
	void remove_unit_from_flamethtower_table(Unit* unit, bool source_only);

	bool was_hit_by_line_attack(Unit* source, Unit* target);
	void register_line_attack_hit(Unit* source, Unit* target);
	
	bool was_hit_by_flamethrower(Unit* source, Unit* target);
	void register_flamethrower_hit(Unit* source, Unit* target);

	void create_effect(u8 id, u32 time, Point16 position, u32 radius, s32 sprite_id, u8 elevation, u8 player, Unit* source, Unit* parent = 0);
	bool was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target);
	bool multihit_bullet_is_set(Bullet* bullet);
	u32 get_multihit_count(Bullet* bullet);
	void add_multihit_empty_bullet(Bullet* bullet, s32 divider);
	void add_bullet_multihit(Bullet* bullet, Unit* target);
	void add_incremental_bullet_multihit(Bullet* bullet, Unit* target);
	void remove_multihit_bullet(Bullet* bullet);
	s32 get_divider(Bullet* bullet);
	void clear_lakizilisk(Unit* lakizilisk);
	void add_lakizilisk_spine_pair(Unit* lakizilisk, Unit* target);
	bool spine_pair_exists(Unit* lakizilisk, Unit* target);
};

void apply_irradiate(Unit* unit, u8 player, u32 time);

void inject_globals_hooks();

extern IquareGlobals iquare_globals;

namespace hooks
{
	struct WeaponDamageData;
}

namespace bw
{
	bool is_slowed(Unit* unit);
	void refresh_debuffs(Unit* unit);
	void inherit_debuffs(Unit* parent, Unit* child);
	bool is_debuffed(Unit* unit);
	void restore_func(Unit* unit);
	void do_deconstruction_damage(Unit* unit, u16 weapon_id, bool play_sound = true);

	void clear_clarion_links(Unit* unit);
	void place_clarion_links(Unit* unit);
	bool has_clarion_links(Unit* unit);
	std::vector<Unit*> get_clarion_links(Unit* unit);
	void spread_clarion_damage(hooks::WeaponDamageData& data);
}