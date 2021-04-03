#pragma once
#pragma pack(1)
#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <ostream>
#include <istream>
#include "SCBW\api.h"
#include "hooks\limits\attack_extender.h"

class globals;

struct SaveState {
	u32 len;
	const char* ptr;

	SaveState() {
		ptr = 0;
		len = 0;
	}
};
namespace ImpactId {
	enum {
		Salamander,
		Planetcracker,
		IonField,
	};
}

class persistImpact {
public:
	Point32 position;
	u8 elevation;
	u32 timer;
	u32 repeatsLeft;
	u8 impactId;
};
class pariahHit {
public:
	CUnit* source;
	CUnit* target;
};
class multihitBullet {
public:
	CBullet* bullet;
	int divider;
	std::vector<CUnit*> already_hit;
	multihitBullet(){
		divider = 0;
	}
	multihitBullet(CBullet* bullet,std::vector<CUnit*> already_hit,u32 divider): bullet(bullet),already_hit(already_hit),divider(divider){}
};

class apostleAgent {
public:
	Point32 position;
	u32 timer;
	u32 unitId;
	u8 playerId;
	u32 energy;
	CUnit* creator;
};


class AttackExtender {
public:
//	std::vector<std::vector<u16>> attacks;
	u16 attacks[PLAYABLE_PLAYER_COUNT][ATTACK_LIMIT];
	AttackExtender() {
		
		for (int i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
			for (int j = 0; j < ATTACK_LIMIT; j++) {
				attacks[i][j] = 0;
			}
		}
	}
};
struct CustomUnitStruct {
	// Timers
	std::unordered_map<int, int> genericTimers;

	void setGenericTimer(int variable, int value) {
		genericTimers[variable] = value;
	}

	int getGenericTimer(int variable) {
		auto it = genericTimers.find(variable);
		if (it != genericTimers.end()) {
			return it->second;
		}
		return 0;
	}

	// Generics
	template <typename T>
	std::unordered_map<int, T>& getGenericMap() {
		static std::unordered_map<int, T> map;
		return map;
	}

	template <typename T>
	T* getGeneric(int variable) {
		auto& map = getGenericMap<T>();
		auto it = map.find(variable);
		if (it != map.end()) {
			return &it->second;
		}
		return nullptr;
	}

	template <typename T>
	T* setGeneric(int variable, T value) {
		auto& map = getGenericMap<T>();
		map[variable] = value;
		return &map[variable];
	}

	// Legacy Functions
	void setGenericValue(int variable, int value) {
		setGeneric<int>(variable, value);
	}

	void addGenericValue(int variable, int value) {
		setGeneric<int>(variable, *getGeneric<int>(value) + 1);
	}

	void setGenericUnit(int variable, CUnit* unit) {
		setGeneric<CUnit*>(variable, unit);
	}

	int getGenericValue(int variable) {
		return *getGeneric<int>(variable);
	}

	CUnit* getGenericUnit(int variable) {
		return *getGeneric<CUnit*>(variable);
	}

	// Arrays
	template<typename T>
	std::vector<T>& getGenericVector(int variable) {
		auto vec = getGeneric<std::vector<T>>(variable);
		if (!vec) {
			vec = setGeneric<std::vector<T>>({});
		}
		return vec;
	}
};
class globals {
	std::string output;
public:
	//debugging data
/*	double unitSpeedDbg;
	double weaponDamageDbg;
	double gameHookDbg;
	double updateUnitStateDbg;
	double updateStatusEffectsDbg;*/
	//serializable data
	u32 extended_unit_table;
	u32 extended_sfx;
	bool tbl_extender;
	bool unitid_extender;

	std::vector<persistImpact> persistImpacts;
	std::vector<pariahHit> pariahHits;
	std::vector<multihitBullet> multihitBullets;
	std::vector<apostleAgent> apostleAgents;
	std::map<CUnit*, std::vector<CUnit*>> lurkerSpineHits;
	std::map<CUnit*, CustomUnitStruct> unitStructs;
	AttackExtender attackers;
	
	apostleAgent test;

	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	int flingy_init_bullet_id;
	u8 flingy_init_player_id;
	CBullet* bounce_bullet;
//	SaveState aise_save();
	void aise_save();
	void aise_load(const char* ptr, u32 len);
	globals() {
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
	void createSalamanderFire(int x, int y, u8 player, u8 elevation, bool addImpactValue);
	void createIonField(int x, int y, u8 player, u8 elevation, bool addImpactValue);
	void createPlanetCracker(int x, int y, u8 player, u8 elevation);
	bool was_already_hit_by_multihit_bullet(CBullet* bullet, CUnit* target);
	bool multihit_bullet_is_set(CBullet* bullet);
	u32 get_multihit_count(CBullet* bullet);
	void add_multihit_empty_bullet(CBullet* bullet, int divider);
	void add_bullet_multihit(CBullet* bullet, CUnit* target);
	void add_incremental_bullet_multihit(CBullet* bullet, CUnit* target);
	void initLazarusAgent(int x, int y, u8 player, u32 unitId, u32 energy, CUnit* unit);
	void clear_ptr_in_apostle_table(CUnit* unit);
	void remove_multihit_bullet(CBullet* bullet);
	int get_divider(CBullet* bullet);
	void clear_lurker(CUnit* lurker);
	void add_lurker_spine_pair(CUnit* lurker, CUnit* target);
	bool spine_pair_exists(CUnit* lurker, CUnit* target);

};

void injectGlobalsHooks();

static SaveState saveState;
extern globals Globals;

namespace scbw {
	CUnit* get_generic_unit(CUnit* unit, u32 id);
	u32 get_generic_value(CUnit* u, u32 val);
	u32 get_generic_timer(CUnit* u, u32 val);
	void set_generic_unit(CUnit* unit, u32 id, CUnit* targ);
	void set_generic_value(CUnit* u, u32 id, u32 val);
	void set_generic_timer(CUnit* u, u32 id, u32 val);
	void add_generic_value(CUnit* u, u32 id, int val);
	CBullet* setMissileSourcePosRet(CUnit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset);
	bool isSlowed(CUnit* unit);
	void refreshDebuffs(CUnit* unit);
	void inheritDebuffs(CUnit* parent, CUnit* child);
	bool isDebuffed(CUnit* unit);
	void restoreFunc(CUnit* unit);
	void applyParasite(CUnit* unit, u8 playerID, CUnit* queen);
	void doDeconstructionDamage(CUnit* unit, u16 weaponId, bool playSound);
	void setMissileSourcePos(CUnit* unit, u32 weaponId, u32 valueId, s32 rotateCount, u32 overlayType, u32 frameset_offset);
	CUnit** getSourceQueens(CUnit* unit);
}