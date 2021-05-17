#pragma once
//
#include <hook_tools.h>
#include <SCBW/api.h>
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	u32 realTopSpeed(Flingy* flingy, u32 flingyId);
	u16 realAcceleration(Flingy* flingy, u32 flingyId);
	bool is_high_templar_conduit(u16 unitId);
	u32 weapon_flingy(u32 weaponId, u8 playerId);
	u32 getFlingyHaltDistanceHook(Flingy* flingy);
	bool is_unused_order(u8 orderId);
	void setupSpiderMines(Unit* unit);
	u32 get_mine_id(Unit* unit);
	u32 extend_tech_target_check(u32 orig_result, Unit* caster, Unit* target, u8 techId);
	void order_PhaseLink(Unit* unit);
	bool canAttackTarget_EndChunk(Unit* source, u32 target_flags);
	void createDamageOverlay(Sprite* sprite);
	bool debugDamageOverlay(Sprite* sprite);

	u32 checkUnitDatRequirements(u8 playerId, u16 unit_id, Unit* parent);
	s32 isTileBlockedBy(Unit** unitArray, Unit* builder, s32 x_tile, s32 y_tile,
		s8 dontIgnoreReacting, s8 alsoInvisible);													// 00473300
	s32 gameLoop();																					// 004D94B0
	Unit* createUnit(u16 unitType, s32 x, s32 y, s32 playerId);									// 004A09D0
}

namespace hooks
{
	void injectCosmonarchySpecific();

}