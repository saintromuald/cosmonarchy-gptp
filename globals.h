#pragma once
#pragma pack(1)
#include <vector>
#include <unordered_map>
#include <sstream>
#include <ostream>
#include <istream>
#include "SCBW\api.h"
#include "hooks\limits\attack_extender.h"

class Globals;

struct SaveState {
	uint32 len;
	const char* ptr;

	SaveState() {
		ptr = 0;
		len = 0;
	}
};
namespace ImpactId {
	enum {
		SalamanderFire,
		GriefOfAllGods,
		IonField,
		Criticality,
		CausticDischarge,
	};
}

class PersistImpact {
public:
	uint8 impactId;
	Point32 position;
	uint32 mergeRadius;
	uint32 time;
	int32 spriteId;
	uint8 elevation;
	uint32 internalTimeMax;
	CThingy* internalThingy;
};

class PariahHit {
public:
	CUnit* source;
	CUnit* target;
};

class MultihitBullet {
public:
	CBullet* bullet;
	int divider;
	std::vector<CUnit*> already_hit;
	MultihitBullet() {
		divider = 0;
	}
	MultihitBullet(CBullet* bullet, std::vector<CUnit*> already_hit, uint32 divider) : bullet(bullet), already_hit(already_hit), divider(divider) {}
};

class ApostleAgent {
public:
	Point32 position;
	uint32 timer;
	uint32 unitId;
	uint8 playerId;
	uint32 energy;
	CUnit* creator;
};


class AttackExtender {
public:
	//	std::vector<std::vector<uint16>> attacks;
	uint16 attacks[PLAYABLE_PLAYER_COUNT][ATTACK_LIMIT];
	AttackExtender() {

		for (int i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
			for (int j = 0; j < ATTACK_LIMIT; j++) {
				attacks[i][j] = 0;
			}
		}
	}
};
struct CustomUnitStruct {
	std::unordered_map<uint32, int> genericValues;
	std::unordered_map<uint32, int> genericTimers;
	std::unordered_map<uint32, CUnit*> genericUnits;
	std::unordered_map<uint32, std::vector<int>> genericValueVectors;
	std::unordered_map<uint32, std::vector<int>> genericTimerVectors;
	std::unordered_map<uint32, std::vector<CUnit*>> genericUnitVectors;
	std::unordered_map<uint32, std::unordered_map<CUnit*, int>>	genericUnitTimerMap;

	void setGenericValue(uint32 variable, uint32_t value) {
		genericValues[variable] = value;
	}
	void addGenericValue(uint32 variable, uint32_t value) {
		genericValues[variable] += value;
	}
	void setGenericTimer(uint32 variable, uint32_t value) {
		genericTimers[variable] = value;
	}
	void setGenericUnit(uint32 variable, CUnit* unit) {
		genericUnits[variable] = unit;
	}

	uint32_t getGenericValue(uint32 variable) {
		auto it = genericValues.find(variable);
		if (it != genericValues.end()) {
			return it->second;
		}
		return 0;
	}

	uint32_t getGenericTimer(uint32 variable) {
		auto it = genericTimers.find(variable);
		if (it != genericTimers.end()) {
			return it->second;
		}
		return 0;
	}

	CUnit* getGenericUnit(uint32 variable) {
		auto it = genericUnits.find(variable);
		if (it != genericUnits.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::vector<int>& getGenericValueVector(uint32 variable) {
		auto it = genericValueVectors.find(variable);
		if (it != genericValueVectors.end()) {
			return it->second;
		}
		genericValueVectors[variable] = {};
		return genericValueVectors[variable];
	}

	std::vector<int>& getGenericTimerVector(uint32 variable) {
		auto it = genericTimerVectors.find(variable);
		if (it != genericTimerVectors.end()) {
			return it->second;
		}
		genericTimerVectors[variable] = {};
		return genericTimerVectors[variable];
	}

	std::vector<CUnit*>& getGenericUnitVector(uint32 variable) {
		auto it = genericUnitVectors.find(variable);
		if (it != genericUnitVectors.end()) {
			return it->second;
		}
		genericUnitVectors[variable] = {};
		return genericUnitVectors[variable];
	}

	std::unordered_map<CUnit*, int>& getGenericUnitTimerMap(uint32 variable) {
		if (!genericUnitTimerMap.empty()) {
			auto it = genericUnitTimerMap.find(variable);
			if (it != genericUnitTimerMap.end()) {
				return it->second;
			}
		}
		genericUnitTimerMap[variable] = {};
		return genericUnitTimerMap[variable];
	}
};
class Globals {
	std::string output;
public:
	//debugging data
/*	double unitSpeedDbg;
	double weaponDamageDbg;
	double gameHookDbg;
	double updateUnitStateDbg;
	double updateStatusEffectsDbg;*/
	//serializable data
	uint32 extended_unit_table;
	uint32 extended_sfx;
	bool tbl_extender;
	bool unitid_extender;

	std::vector<PersistImpact> persistImpacts;
	std::vector<PariahHit> pariahHits;
	std::vector<MultihitBullet> multihitBullets;
	std::vector<ApostleAgent> apostleAgents;
	std::unordered_map<CUnit*, std::vector<CUnit*>> lurkerSpineHits;
	std::unordered_map<CUnit*, CustomUnitStruct> unitStructs;
	AttackExtender attackers;

	ApostleAgent test;

	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	int flingy_init_bullet_id;
	uint8 flingy_init_player_id;
	CBullet* bounce_bullet;
	//	SaveState aise_save();
	void aise_save();
	void aise_load(const char* ptr, uint32 len);
	Globals() {
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
	void unitFrameHook(CUnit* unit);
	void unitDeathHook(CUnit* unit);
	void remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly);
	bool was_hit_by_pariah(CUnit* source, CUnit* target);

	void register_pariah_hit(CUnit* source, CUnit* target);
	void createGenericPersistImpact(uint8 impactId, Point32 position, uint32 mergeRadius, uint32 time, int32 spriteId, uint8 elevation);
	bool was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target);
	bool multihit_bullet_is_set(CBullet* bullet);
	uint32 get_multihit_count(CBullet* bullet);
	void add_multihit_empty_bullet(CBullet* bullet, int divider);
	void add_bullet_multihit(CBullet* bullet, CUnit* target);
	void add_incremental_bullet_multihit(CBullet* bullet, CUnit* target);
	void initLazarusAgent(int x, int y, uint8 player, uint32 unitId, uint32 energy, CUnit* unit);
	void clear_ptr_in_apostle_table(CUnit* unit);
	void remove_multihit_bullet(CBullet* bullet);
	int get_divider(CBullet* bullet);
	void clear_lakizilisk(CUnit* lurker);
	void add_lurker_spine_pair(CUnit* lurker, CUnit* target);
	bool spine_pair_exists(CUnit* lurker, CUnit* target);

};

void injectGlobalsHooks();

static SaveState saveState;
extern Globals globals;

namespace hooks {
	struct WeaponDamageData;
}

namespace scbw {
	CUnit* get_generic_unit(CUnit* unit, uint32 id);
	uint32 get_generic_value(CUnit* u, uint32 val);
	uint32 get_generic_timer(CUnit* u, uint32 val);
	void set_generic_unit(CUnit* unit, uint32 id, CUnit* targ);
	void set_generic_value(CUnit* u, uint32 id, uint32 val);
	void set_generic_timer(CUnit* u, uint32 id, uint32 val);
	void add_generic_value(CUnit* u, uint32 id, int val);
	std::vector<int>& get_generic_value_vector(CUnit* u, uint32 id);
	std::vector<int>& get_generic_timer_vector(CUnit* u, uint32 id);
	std::vector<CUnit*>& get_generic_unit_vector(CUnit* u, uint32 id);
	std::unordered_map<CUnit*, int>& get_generic_unit_timer_map(CUnit* u, uint32 id);
	CBullet* setMissileSourcePosRet(CUnit* unit, uint32 weaponId, uint32 valueId, int32 rotateCount, uint32 overlayType, uint32 frameset_offset);
	bool isSlowed(CUnit* unit);
	void refreshDebuffs(CUnit* unit);
	void inheritDebuffs(CUnit* parent, CUnit* child);
	bool isDebuffed(CUnit* unit);
	void restoreFunc(CUnit* unit);
	void applyParasite(CUnit* unit, uint8 playerID, CUnit* queen);
	void doDeconstructionDamage(CUnit* unit, uint16 weaponId, bool playSound);
	void setMissileSourcePos(CUnit* unit, uint32 weaponId, uint32 valueId, int32 rotateCount, uint32 overlayType, uint32 frameset_offset);
	CUnit** getSourceQueens(CUnit* unit);

	void clearClarionLinks(CUnit* unit);
	void placeClarionLinks(CUnit* unit);
	bool hasClarionLinks(CUnit* unit);
	std::vector<CUnit*> getClarionLinks(CUnit* unit);
	void spreadClarionDamage(hooks::WeaponDamageData& data);
}