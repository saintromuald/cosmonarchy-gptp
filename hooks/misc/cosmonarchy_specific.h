#pragma once
#include <hook_tools.h>
#include <SCBW/api.h>
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	u32 real_top_speed(Flingy* flingy, u32 flingy_id);
	u16 real_acceleration(Flingy* flingy, u32 flingy_id);
	bool is_high_templar_conduit(u16 unit_id);
	u32 weapon_flingy(u32 weaponId, u8 playerId);
	u32 get_flingy_halt_distance(Flingy* flingy);
	bool is_unused_order(u8 orderId);
	void set_up_spider_mines(Unit* unit);
	u32 get_mine_id(Unit* unit);
	u32 extend_tech_target_check(u32 orig_result, Unit* caster, Unit* target, u8 techId);
	void order_phase_link(Unit* unit);
	bool can_attack_target_end_chunk(Unit* source, u32 target_flags);
	void create_damage_overlay(Sprite* sprite);
	bool debug_damage_overlay(Sprite* sprite);

	u32 check_unit_dat_requirements(u8 playerId, u16 unit_id, Unit* parent);
	s32 game_loop();																				// 004D94B0
	Unit* create_unit(u16 unit_id, s32 x, s32 y, s32 playerId);										// 004A09D0
	void transform_unit_main(Unit* unit, u32 unit_id);												// 0049F170
	char ignore_iscript_and_go_to_idle(Unit* unit);													// 00476550
	void init_damage_overlay_counts();																// 004D5C20
}

namespace hooks
{
	void inject_cosmonarchy_specific();
}