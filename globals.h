#pragma once
#pragma pack(1)
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <ostream>
#include <istream>
#include <assert.h>
#include "SCBW\api.h"
#include "hooks\limits\attack_extender.h"

struct Globals;

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
namespace EffectId
{
	enum
	{
		SalamanderFire,
		GriefOfAllGods,
		IonField,
		Criticality,
		CausticDischarge,
		Irradiate,
		Hallucination,
		GroundZero,
	};
}

struct Effect
{
	u8 effectId;
	Point32 position;
	u32 radius;
	u32 currentTime;
	s32 spriteId;
	u8 elevation;
	u8 player;
	u32 maxTime;
	Thingy* thingy;
};

struct PariahHit
{
	Unit* source;
	Unit* target;
};

struct MultihitBullet
{
	Bullet* bullet;
	s32 divider;
	u32 refCount;
	std::vector<Unit*> already_hit;

	MultihitBullet() : bullet(0), already_hit({}), divider(0), refCount(1) {}
	MultihitBullet(Bullet* bullet, std::vector<Unit*> already_hit, u32 divider) : bullet(bullet), already_hit(already_hit), divider(divider), refCount(1) {}
};

struct ApostleAgent
{
	Point32 position;
	u32 timer;
	u32 unitId;
	u8 playerId;
	u32 energy;
	u32 stacks;
	Unit* creator;
};


struct AttackExtender
{
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

struct Globals
{
	std::string output;
	//debugging data
/*	f64 unitSpeedDbg;
	f64 weaponDamageDbg;
	f64 gameHookDbg;
	f64 updateUnitStateDbg;
	f64 updateStatusEffectsDbg;*/
	//serializable data
	u32 extended_unit_table;
	u32 extended_sfx;
	bool tbl_extender;
	bool unitid_extender;

	std::vector<Effect> effects;
	std::vector<PariahHit> pariahHits;
	std::vector<MultihitBullet> multihitBullets;
	std::vector<ApostleAgent> apostleAgents;
	std::unordered_map<Unit*, std::vector<Unit*>> lurkerSpineHits;
	UnitExt unitExt[UNIT_ARRAY_LENGTH];

	inline UnitExt* GetUnitExt(Unit* unit)
	{
		assert(unit);
		return &unitExt[unit->getIndex()];
	}

	AttackExtender attackers;
	ApostleAgent test;

	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	s32 flingy_init_bullet_id;
	u8 flingy_init_player_id;
	Bullet* bounce_bullet;
	//	SaveState aise_save();
	void aise_save();
	void aise_load(const char* ptr, u32 len);
	Globals()
	{
		extended_unit_table = 0;
		extended_sfx = 0;
		tbl_extender = false;
		unitid_extender = false;

		refreshDebuggingInfo();

		//
	}
	void refreshDebuggingInfo();
	void reset();
	void frameHook();
	void unitDeathHook(Unit* unit);
	void remove_unit_from_pariah_table(Unit* unit, bool sourceOnly);
	bool was_hit_by_pariah(Unit* source, Unit* target);

	void register_pariah_hit(Unit* source, Unit* target);
	void createEffect(u8 impactId, u32 time, Point32 position, u32 radius, s32 spriteId, u8 elevation, u8 player);
	bool was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target);
	bool multihit_bullet_is_set(Bullet* bullet);
	u32 get_multihit_count(Bullet* bullet);
	void add_multihit_empty_bullet(Bullet* bullet, s32 divider);
	void add_bullet_multihit(Bullet* bullet, Unit* target);
	void add_incremental_bullet_multihit(Bullet* bullet, Unit* target);
	void initLazarusAgent(s32 x, s32 y, u8 player, u32 unitId, u32 energy, u32 stacks, Unit* unit);
	void clear_ptr_in_apostle_table(Unit* unit);
	void remove_multihit_bullet(Bullet* bullet);
	void force_remove_multihit_bullet(Bullet* bullet);
	s32 get_divider(Bullet* bullet);
	void clear_lakizilisk(Unit* lurker);
	void add_lurker_spine_pair(Unit* lurker, Unit* target);
	bool spine_pair_exists(Unit* lurker, Unit* target);

};

void ApplyIrradiate(Unit* unit, u8 player, u32 time);

void injectGlobalsHooks();

static SaveState saveState;
extern Globals globals;

namespace hooks
{
	struct WeaponDamageData;
}

namespace scbw
{
	Bullet* setMissileSourcePos(Unit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset);
	bool isSlowed(Unit* unit);
	void refreshDebuffs(Unit* unit);
	void inheritDebuffs(Unit* parent, Unit* child);
	bool isDebuffed(Unit* unit);
	void restoreFunc(Unit* unit);
	void applyParasite(Unit* unit, u8 playerID, Unit* queen);
	void doDeconstructionDamage(Unit* unit, u16 weaponId, bool playSound = true);
	Unit** getSourceQueens(Unit* unit);

	void clearClarionLinks(Unit* unit);
	void placeClarionLinks(Unit* unit);
	bool hasClarionLinks(Unit* unit);
	std::vector<Unit*> getClarionLinks(Unit* unit);
	void spreadClarionDamage(hooks::WeaponDamageData& data);
}