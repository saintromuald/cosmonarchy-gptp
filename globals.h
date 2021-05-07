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

class Globals;

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
namespace ImpactId
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

struct PersistImpact
{
	u8 impactId;
	Point32 position;
	u32 radius;
	u32 currentTime;
	s32 spriteId;
	u8 elevation;
	u8 player;
	u32 maxTime;
	CThingy* thingy;
};

class PariahHit
{
public:
	CUnit* source;
	CUnit* target;
};

class MultihitBullet
{
public:
	CBullet* bullet;
	s32 divider;
	std::vector<CUnit*> already_hit;
	MultihitBullet()
	{
		divider = 0;
	}
	MultihitBullet(CBullet* bullet, std::vector<CUnit*> already_hit, u32 divider) : bullet(bullet), already_hit(already_hit), divider(divider) {}
};

class ApostleAgent
{
public:
	Point32 position;
	u32 timer;
	u32 unitId;
	u8 playerId;
	u32 energy;
	u32 stacks;
	CUnit* creator;
};


class AttackExtender
{
public:
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

class Globals
{
	std::string output;
public:
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

	std::vector<PersistImpact> persistImpacts;
	std::vector<PariahHit> pariahHits;
	std::vector<MultihitBullet> multihitBullets;
	std::vector<ApostleAgent> apostleAgents;
	std::unordered_map<CUnit*, std::vector<CUnit*>> lurkerSpineHits;
	UnitExt unitExt[UNIT_ARRAY_LENGTH];

	inline UnitExt* GetUnitExt(CUnit* unit)
	{
		assert(unit);
		return &unitExt[unit->getIndex()];
	}

	AttackExtender attackers;
	ApostleAgent test;

	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	s32 flingy_init_bullet_id;
	u8 flingy_init_player_id;
	CBullet* bounce_bullet;
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
	void unitDeathHook(CUnit* unit);
	void remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly);
	bool was_hit_by_pariah(CUnit* source, CUnit* target);

	void register_pariah_hit(CUnit* source, CUnit* target);
	void createGenericPersistImpact(u8 impactId, u32 time, Point32 position, u32 radius, s32 spriteId, u8 elevation, u8 player);
	bool was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target);
	bool multihit_bullet_is_set(CBullet* bullet);
	u32 get_multihit_count(CBullet* bullet);
	void add_multihit_empty_bullet(CBullet* bullet, s32 divider);
	void add_bullet_multihit(CBullet* bullet, CUnit* target);
	void add_incremental_bullet_multihit(CBullet* bullet, CUnit* target);
	void initLazarusAgent(s32 x, s32 y, u8 player, u32 unitId, u32 energy, u32 stacks, CUnit* unit);
	void clear_ptr_in_apostle_table(CUnit* unit);
	void remove_multihit_bullet(CBullet* bullet);
	s32 get_divider(CBullet* bullet);
	void clear_lakizilisk(CUnit* lurker);
	void add_lurker_spine_pair(CUnit* lurker, CUnit* target);
	bool spine_pair_exists(CUnit* lurker, CUnit* target);

};

void ApplyIrradiate(CUnit* unit, u8 player, u32 time);

void injectGlobalsHooks();

static SaveState saveState;
extern Globals globals;

namespace hooks
{
	struct WeaponDamageData;
}

namespace scbw
{
	CBullet* setMissileSourcePos(CUnit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset);
	bool isSlowed(CUnit* unit);
	void refreshDebuffs(CUnit* unit);
	void inheritDebuffs(CUnit* parent, CUnit* child);
	bool isDebuffed(CUnit* unit);
	void restoreFunc(CUnit* unit);
	void applyParasite(CUnit* unit, u8 playerID, CUnit* queen);
	void doDeconstructionDamage(CUnit* unit, u16 weaponId, bool playSound = true);
	CUnit** getSourceQueens(CUnit* unit);

	void clearClarionLinks(CUnit* unit);
	void placeClarionLinks(CUnit* unit);
	bool hasClarionLinks(CUnit* unit);
	std::vector<CUnit*> getClarionLinks(CUnit* unit);
	void spreadClarionDamage(hooks::WeaponDamageData& data);
}