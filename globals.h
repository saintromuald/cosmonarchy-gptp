#pragma once
#pragma pack(1)
#include <vector>
#include <map>
#include <sstream>
#include <ostream>
#include <istream>
#include "SCBW\api.h"

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

class globals {
	std::string output;
public:
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
	
	apostleAgent test;

	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	int flingy_init_bullet_id;
	u8 flingy_init_player_id;
	CBullet* bounce_bullet;
	SaveState aise_save();
	void aise_load(const char* ptr, u32 len);
	globals() {
		extended_unit_table = 0;
		extended_sfx = 0;
		tbl_extender = false;
		unitid_extender = false;
		//
	}
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

extern globals Globals;