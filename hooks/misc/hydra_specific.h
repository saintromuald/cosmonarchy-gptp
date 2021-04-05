#pragma once
//
#include <hook_tools.h>
#include <SCBW/api.h>
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>

namespace hooks {
	uint32 realTopSpeed(CFlingy* flingy, uint32 flingyId);
	uint16 realAcceleration(CFlingy* flingy, uint32 flingyId);
	bool is_high_templar_conduit(uint16 unitId);
	uint32 weapon_flingy(uint32 weaponId, uint8 playerId);
	uint32 getFlingyHaltDistance(CFlingy* flingy);
	bool is_unused_order(uint8 orderId);
	void setupSpiderMines(CUnit* unit);
	uint32 get_mine_id(CUnit* unit);
	uint32 extend_tech_target_check(uint32 orig_result, CUnit* caster, CUnit* target, uint8 techId);
	void order_PhaseLink(CUnit* unit);
	bool canAttackTarget_EndChunk(CUnit* source, uint32 target_flags);
	void createDamageOverlay(CSprite* sprite);
	uint32 checkUnitDatRequirements(uint8 playerId, uint16 unit_id, CUnit* parent);
	int32 isTileBlockedBy(CUnit** unitArray, CUnit* builder, int32 x_tile, int32 y_tile, int8 dontIgnoreReacting, int8 alsoInvisible); // 00473300
	int32 gameLoop(); // 004D94B0
}

namespace hooks {
	void injectHydraSpecific();

}