#pragma once
#pragma pack(1)
#include <vector>
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

class salamanderImpact {
public:
	Point32 position;
	u8 elevation;
	u32 timer;
	u32 repeatsLeft;
};
class pariahHit {
public:
	CUnit* source;
	CUnit* target;
};

class globals {
	std::string output;
public:
	//serializable data
	u32 extended_unit_table;
	u32 extended_sfx;
	bool tbl_extender;
	bool unitid_extender;
	//non-serializable data (exists only temporarily in memory in wrapper/hook flow)
	int flingy_init_bullet_id;
	u8 flingy_init_player_id;
	std::vector<salamanderImpact> salamanderImpacts;
	std::vector<pariahHit> pariahHits;
	//

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
	void remove_unit_from_pariah_table(CUnit* unit, bool sourceOnly);
	bool was_hit_by_pariah(CUnit* source, CUnit* target);
	void register_pariah_hit(CUnit* source, CUnit* target);
	void createSalamanderFire(int x, int y, u8 player, u8 elevation, bool addImpactValue);
};

void injectGlobalsHooks();

extern globals Globals;