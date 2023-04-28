#include "api.h"
#include "scbwdata.h"
#include <SCBW/UnitFinder.h>
#include <hooks/cosmonarchy_finder.h>
#include <algorithm>
#include <cassert>
#include <utility>
#include <hooks/weapons/weapon_damage.h>
#include <globals.h>
#include "hooks/target_related.h"
#include "hooks/unit_ai.h"
#include "ai_layouts.h"
#include "config.h"
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <hooks\unit_movement.h>

namespace scbw {

	void artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, Point16 target_pt) {
		auto target_unit = source->order_target.unit;
		auto target_pos = source->order_target.pt;
		source->order_target.unit = target;
		source->order_target.pt = target_pt;
		Point16 pick;
		if (target && target_unit) {
			source->order_target.pt = target_unit->sprite->position;
		} else {
			source->order_target.pt = target_pt;
		}
		helper::create_bullet(weapon_id, source, source->sprite->position.x, source->sprite->position.y, player,
			get_angle(source->position.x, source->position.y, source->order_target.pt.x, source->order_target.pt.y));
		source->order_target.unit = target_unit;
		source->order_target.pt = target_pos;
	}

	void artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, u16 x, u16 y) {
		Point16 tgt;
		tgt.x = x;
		tgt.y = y;
		artificial_bullet_from(weapon_id, player, source, target, tgt);
	}

	void teleport_internal(Unit* unit, Point16 target, u16 sprite_id, bool preserve_order) {
		// Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
		scbw::prepare_unit_move(unit, false);
		if (!preserve_order) {
			unit->order_target.unit = 0;
		}
		scbw::set_unit_position(unit, target.x, target.y);
		scbw::refresh_unit_after_move(unit);

		// Stop some currently active orders
		if (unit->id != UnitId::ZergCocoon && !preserve_order) {
			unit->order_computer(units_dat::return_to_idle_order[unit->id]);
		}

		// Create the Recall animation over the unit
		if (sprite_id) {
			Thingy* recall_effect_sprite = helper::create_thingy(sprite_id, target.x, target.y, 0);
			if (recall_effect_sprite) {
				recall_effect_sprite->sprite->elevation = unit->sprite->elevation + 1;
				helper::set_thingy_visibility_flags(recall_effect_sprite);
			}

		}

		// Disconnect a nuke from a unit using it
		if (scbw::unit_can_nuke(unit)) {
			if (unit->connected_unit && unit->connected_unit->id == UnitId::TerranNuclearMissile) {
				unit->connected_unit->connected_unit = 0;
				unit->connected_unit = 0;
			}
		}
	}

	bool teleport_to_point(Unit* unit, Point16 target, u16 sprite_id, bool preserve_order) {
		Point16 new_pos = {};
		if (!scbw::check_unit_collision_pos(unit, &target, &new_pos, 0, false, 0)) {
			return false;
		}

		teleport_internal(unit, new_pos, sprite_id, preserve_order);
		return true;
	}

	bool teleport_to_unit(Unit* unit, Unit* target, u32 radius, u16 sprite_id, bool preserve_order) {
		Box16 move_area = {};
		move_area.left   = target->get_left()   - radius;
		move_area.top    = target->get_top()    - radius;
		move_area.right  = target->get_right()  + radius;
		move_area.bottom = target->get_bottom() + radius;
		move_area = clamp_box16(move_area);

		Point16 new_pos = {};
		if (!scbw::check_unit_collision_pos(unit, &unit->position, &new_pos, &move_area, false, 0)) {
			return false;
		}

		teleport_internal(unit, new_pos, sprite_id, preserve_order);
		return true;
	}

	Thingy* create_and_init_thingy(u32 spriteId, s16 x, s16 y, u32 player, s32 elevation_level) {
		Thingy* thingy = helper::create_thingy(spriteId, x, y, player);
		if (thingy) {
			thingy->sprite->elevation = elevation_level;
			helper::set_thingy_visibility_flags(thingy);
		}
		return thingy;
	}

	void print_string(std::string text, u32 color) {
		print(text.c_str(), color);
	}

	void print(const char* text, u32 color) {
		const u32 Func_PrintText = 0x0048CD30;
		if (!text) {
			return;
		}

		DWORD gtc = GetTickCount() + 7000;
		__asm {
			PUSHAD
			PUSH 0;//unknown
			MOV eax, text
			PUSH gtc
			PUSH color
			CALL Func_PrintText
			POPAD
		}
	}

	void printf(const char* format, ...) {
		char buf[256] = {};

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		__crt_va_end(_ArgList);

		print(buf);
	}

	void printf_color(const char* format, u32 color, ...) {
		char buf[256] = {};

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		va_end(_ArgList);

		print(buf, color);
	}

	void printf_console(const char* format, ...) {
		char buf[1025] = {};

		va_list _ArgList;
		va_start(_ArgList, format);
		vsnprintf(buf, sizeof(buf), format, _ArgList);
		va_end(_ArgList);

		OutputDebugStringA(buf);
		OutputDebugStringA("\n");
	}

	//-------- Unit checks --------//

	// Identical to function @ 0x00475CE0
	bool can_weapon_target_unit(u8 weapon_id, Unit* target, Unit* attacker) {
		if (weapon_id >= WeaponsIdLimit) {
			return false;
		}
		if (!target) {
			return weapons_dat::target_flags[weapon_id].terrain;
		}
		if (target->status & UnitStatus::Invincible) {
			return false;
		}

		const TargetFlag target_flags = weapons_dat::target_flags[weapon_id];
		const u32 targetProps = units_dat::base_property[target->id];
		if (target->status & UnitStatus::InAir ? !target_flags.air : !target_flags.ground) {
			return false;
		}
		if (target_flags.mechanical && !(targetProps & UnitProperty::Mechanical)) {
			return false;
		}
		if (target_flags.organic && !(targetProps & UnitProperty::Organic)) {
			return false;
		}
		if (target_flags.non_building && targetProps & UnitProperty::Building) {
			return false;
		}
		if (target_flags.non_robotic && targetProps & UnitProperty::RoboticUnit) {
			return false;
		}
		if (target_flags.organic_or_mechanical && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical))) {
			return false;
		}
		if (target_flags.player_owned && target->player != attacker->player) {
			return false;
		}
		return true;
	}

	bool is_under_dark_swarm(Unit* unit) {
		return !(unit->status & UnitStatus::InAir) && get_dark_swarm_unit(unit) != nullptr;
	}

	bool is_dark_swarm(Unit* unit) {
		return unit->id == UnitId::Spell_DarkSwarm;
	}

	Unit* get_dark_swarm_unit(Unit* unit) {
		static UnitFinder dark_swarm_finder;
		dark_swarm_finder.search(unit->get_left(), unit->get_top(), unit->get_right(), unit->get_bottom());
		return dark_swarm_finder.get_first(is_dark_swarm);
	}

	Unit* get_dark_swarm_from_position(u16 x, u16 y, u16 radius) {
		static UnitFinder dark_swarm_finder;
		dark_swarm_finder.search(x - radius, y - radius, x + radius, y + radius);
		return dark_swarm_finder.get_first(is_dark_swarm);
	}

	//-------- Graphics and geometry --------//

	// Improved code from BWAPI's include/BWAPI/Position.h: getApproxDistance()
	// Logically same as function @ 0x0040C360
	// x1 = ECX;x2 = EDX;y1 = [EBP+08];y2 = [EBP+0C];
	u32 get_distance_fast(s32 x1, s32 y1, s32 x2, s32 y2) {
		s32 dMax = abs(x1 - x2), dMin = abs(y1 - y2);
		if (dMax < dMin) {
			std::swap(dMax, dMin);
		}
		if (dMin <= (dMax >> 2)) {
			return dMax;
		}
		return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);
	}

	u32 get_distance_square(s32 x1, s32 y1, s32 x2, s32 y2) {
		return std::max(abs(x1 - x2), abs(y1 - y2));
	}

	// Identical to function @ 0x00494C10
	s32 arc_tangent(s32 slope) {
		const u32 tangentTable[] = {
				 7,	 13,	 19,	 26,	 32,	 38,	 45,	 51,
				58,	 65,	 71,	 78,	 85,	 92,	 99,	107,
			 114,	122,	129,	137,	146,	154,	163,	172,
			 181,	190,	200,	211,	221,	233,	244,	256,
			 269,	283,	297,	312,	329,	346,	364,	384,
			 405,	428,	452,	479,	509,	542,	578,	619,
			 664,	716,	775,	844,	926, 1023, 1141, 1287,
			1476, 1726, 2076, 2600, 3471, 5211, 10429, -1
		};

		bool isNegative = false;
		if (slope < 0) {
			isNegative = true;
			slope = -slope;
		}

		s32 min = 0, max = 64, angle = 32;
		do {
			if ((u32)slope <= tangentTable[angle]) {
				max = angle;
			} else {
				min = angle + 1;
			}
			angle = (min + max) / 2;
		} while (min != max);
		return (isNegative ? -angle : angle);
	}

	// Identical to function @ 0x00495300
	//					EDX		[EBP+08]	ECX			EAX
	s32 get_angle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd) {
		s32 dx = xEnd - xStart, dy = yEnd - yStart;
		if (dx == 0) {
			return dy > 0 ? 128 : 0;		
		}

		s32 angle = arc_tangent((dy << 8) / dx);
		if (dx < 0) {
			angle += 192;
			return angle == 256 ? 0 : angle;
		} else {
			return angle + 64;
		}
	}

	Point16 rotate_point(Point16 anchor, Point16 point, f64 angle_degrees) {
		f64 x1 = point.x - anchor.x;
		f64 y1 = point.y - anchor.y;
		auto radians = angle_degrees / 57.296;
		f64 x2 = x1 * cos(radians) - y1 * sin(radians);
		f64 y2 = x1 * sin(radians) + y1 * cos(radians);
		Point16 product = {};
		product.x = x2 + anchor.x;
		product.y = y2 + anchor.y;
		return product;
	}

	Vector32 rotate_point(Vector32 anchor, Vector32 point, f64 angle_degrees) {
		f64 x1 = point.x - anchor.x;
		f64 y1 = point.y - anchor.y;
		auto radians = angle_degrees / 57.296;
		f64 x2 = x1 * cos(radians) - y1 * sin(radians);
		f64 y2 = x1 * sin(radians) + y1 * cos(radians);
		Vector32 product = {};
		product.x = x2 + anchor.x;
		product.y = y2 + anchor.y;
		return product;
	}

	f64 bw_angle_to_normal(s32 bw_angle) {
		f64 angle = (f64)bw_angle * 1.40625;
		return angle;
	}

	s32 get_polar_x(s32 distance, u8 angle) {
		return distance * g_angle_distance[angle].x >> 8;
	}

	s32 get_polar_y(s32 distance, u8 angle) {
		return distance * g_angle_distance[angle].y >> 8;
	}

	//-------- Player information --------//

	s32 get_supply_remaining(u8 player, u8 raceId) {
		assert(raceId <= 2);
		assert(player < PLAYER_COUNT);

		s32 supply_provided;
		if (is_cheat_enabled(CheatFlags::FoodForThought)) {
			supply_provided = g_race_supply[RaceId::Terran].max[player];
		} else {
			supply_provided = std::min(g_race_supply[RaceId::Terran].max[player], g_race_supply[RaceId::Terran].provided[player]);
		}
		return supply_provided - g_race_supply[RaceId::Terran].used[player];
	}

	bool has_tech_researched(u8 player, u16 techId) {
		assert(player < PLAYER_COUNT);
		assert(techId != TechId::None);
		if (techId < TechId::Restoration) {
			return TechSc->is_researched[player][techId] != 0;
		} else {
			return TechBw->is_researched[player][techId - TechId::Restoration] != 0;
		}
	}

	bool has_tech_available(u8 player, u16 techId) {
		assert(player < PLAYER_COUNT);
		assert(techId != TechId::None);
		if (techId < TechId::Restoration) {
			return TechSc->is_enabled[player][techId] != 0;
		} else {
			return TechBw->is_enabled[player][techId - TechId::Restoration] != 0;
		}
	}

	void set_tech_research_state(u8 player, u16 techId, bool is_researched) {
		assert(player < PLAYER_COUNT);
		assert(techId != TechId::None);

		if (techId < TechId::Restoration) {
			TechSc->is_researched[player][techId] = is_researched;
		} else {
			TechBw->is_researched[player][techId - TechId::Restoration] = is_researched;
		}
	}

	const u32 Func_DoWeaponDamage = 0x00479930;
	void send_gptp_aise_command(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2, u32 v3) {
		_asm {
			PUSHAD
			MOV EAX, v3
			MOV EDI, unit2
			PUSH v2
			PUSH unit1
			PUSH 50000
			PUSH command
			PUSH v1
			CALL Func_DoWeaponDamage
			POPAD
		}
	}

	/*
	u32 get_flingy_data(u32 entry, u32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetFlingyData, entry, index);
	}

	u32 get_sprite_data(u32 entry, u32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetSpriteData, entry, index);
	}

	u32 get_image_data(u32 entry, u32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetImageData, entry, index);
	}

	u32 get_unit_data(u32 entry, u32 index) {
		return get_aise_value(nullptr, nullptr, AiseId::GetUnitData, entry, index);
	}
	*/

	bool is_larval_egg(Unit* unit) {
		if (unit->id == UnitId::ZergEgg) {
			u8 origin = unit->create_origin;
			if (origin == UnitId::ZergLarva || origin == UnitId::ZergOcsalLarva) {
				return true;
			}
		}
		return false;
	}

	bool is_zerg_egg_new(Unit* unit, bool allow_larval_eggs, bool allow_larvae, u16 explicit_unit_id) {
		u32 id = UnitId::None;
		if (explicit_unit_id != UnitId::None) {
			id = explicit_unit_id;
		} else if (unit) {
			id = unit->id;
		}
		switch (id) {
			case UnitId::ZergEgg: {
				if (unit && !allow_larval_eggs && (unit->create_origin == UnitId::ZergLarva || unit->create_origin == UnitId::ZergOcsalLarva)) {
					return false;
				}
				return true;
			}
			case UnitId::ZergCocoon: {
				return true;
			}
		}

		if (allow_larvae && (id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva)) {
			return true;
		}
		return false;
	}

	bool is_zerg_egg(u16 id, bool allow_normal_eggs, bool allow_larvae) {
		switch (id) {
			case UnitId::ZergCocoon: {
				return true; 
			}
		}
		if ((id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva) && allow_larvae) {
			return true;
		}
		if (id == UnitId::ZergEgg && allow_normal_eggs) {
			return true;
		}
		return false;
	}

	bool is_conventional_unit_morph_result(u16 id) {
		switch (id) {
			case UnitId::ZergGosvileth:
			case UnitId::ZergIroleth:
			case UnitId::ZergAlaszileth:
			case UnitId::ZergVorvrokor:
			case UnitId::ZergLiiralisk:
			case UnitId::ZergKalkalisk:
			case UnitId::ZergSovroleth:
			case UnitId::ZergGhitorokor:
			case UnitId::ZergBactalisk:
			case UnitId::ZergTethzorokor:
			case UnitId::ZergVithrilisk:
			case UnitId::ZergUltrokor:
			case UnitId::ZergZobriolisk:
			case UnitId::ZergZarcavrokor:
			case UnitId::ZergSelkerilisk:
			case UnitId::ZergAkistrokor:
			case UnitId::ZergAlkajelisk:
			case UnitId::ZergOgroszaleth:
			{
				return true;
			}
		}
		return false;
	}

	bool is_any_morph_result(u16 id) {
		if (is_conventional_unit_morph_result(id)) {
			return true;
		}
		return false;
	}

	bool is_conventional_unit_morph_source(u16 id, bool allow_larvae, bool allow_high_templar) {
		switch (id) {
			case UnitId::ZergDroleth:
			case UnitId::ZergOvileth:
			case UnitId::ZergOthstoleth:
			case UnitId::ZergZethrokor:
			case UnitId::ZergQuazilisk:
			case UnitId::ZergNathrokor:
			case UnitId::ZergCikralisk:
			case UnitId::ZergProtathalor:
			case UnitId::ZergMutalisk:
			case UnitId::ZergAxitrilisk:
			case UnitId::ZergMortothrokor:
			case UnitId::ZergHydralisk:
			case UnitId::ZergIzirokor:
			case UnitId::ZergSkithrokor:
			case UnitId::ZergKagralisk:
			case UnitId::ZergEvigrilisk:
			case UnitId::ZergAlmaksalisk:
			case UnitId::ZergKeskathalor:
			case UnitId::ZergGeszithalor:
			case UnitId::ZergZoryusthaleth:
			case UnitId::ZergLakizilisk:
			case UnitId::ZergGorgrokor:
			case UnitId::ZergKonvilisk:
			case UnitId::ZergVilgorokor:
			case UnitId::ZergMatraleth:
			case UnitId::ZergIsthrathaleth:
			case UnitId::ZergElthrazalisk:
			{
				return true; 
			}
		}
		if (id == UnitId::ProtossCantavis && allow_high_templar) {
			return true;
		}
		if ((id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva) && allow_larvae) {
			return true;
		}
		return false;
	}

	bool is_zerg_building_morph_result(u16 id) {
		switch (id) {
			case UnitId::ZergGathtelosk:
			case UnitId::ZergCaphrolosk:
			case UnitId::ZergEzigrisant:
			case UnitId::ZergIrulentIvoa:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergOrbothOmlia:
			case UnitId::ZergLarvoskColony:
			case UnitId::ZergSpraithColony:
			case UnitId::ZergSurkithColony:
			case UnitId::ZergSkortrithColony:
			case UnitId::ZergSpirtithColony:
			case UnitId::ZergLiivralPond:
			case UnitId::ZergVithrathHaunt:
			case UnitId::ZergGorgralSwamp:
			case UnitId::ZergAlmakisAntre:
			case UnitId::ZergAlkajChasm:
			case UnitId::ZergUltrokCavern: {
				return true;
			}
		}
		return false;
	}

	bool is_zerg_building_morph_source(u16 id) {
		switch (id) {
			case UnitId::ZergHachirosk:
			case UnitId::ZergExcisant:
			case UnitId::ZergIrolIris:
			case UnitId::ZergIrulentIvoa:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergCagrantColony:
			case UnitId::ZergSpraithColony:
			case UnitId::ZergQuazrokPool:
			case UnitId::ZergHydrokDen:
			case UnitId::ZergMuthrokSpire:
			case UnitId::ZergMatravilNest:
			case UnitId::ZergZorkizShroud:
			case UnitId::ZergGeszkathGrotto:
			case UnitId::ZergElthisthMound: {
				return true;
			}
		}
		return false;
	}

	std::string read_samase_file(char* name) {
		const char* data = 0;
		u32 len = 0;
		auto data_ptr = &data;
		auto len_ptr = &len;
		u32 val = reinterpret_cast<u32>(name);
		auto length = scbw::get_aise_value(0, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
		char* buf = (char*)scbw::get_aise_value(0, 0, AiseId::ReadSamaseFile_Ptr, 0, 0);
		std::string str;
		if (length == 0) {
			return str;
		} else {
			str = std::string(buf, length);
		}
		return str;
	}

	s32 get_generic_variable(u32 id) {
		return get_aise_value(0, 0, AiseId::GenericVariable, id, 0);
	}

	Unit* get_generic_rust_unit(Unit* unit, u32 id) {
		u32 val = get_aise_value(unit, 0, AiseId::GenericUnit, id, 0);
		return reinterpret_cast<Unit*>(val);
	}

	void set_generic_rust_unit(Unit* unit, u32 id, Unit* targ) {
		scbw::send_gptp_aise_command(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
	}

	void set_generic_variable(u32 id, s32 val) {
		scbw::send_gptp_aise_command(0, 0, GptpId::SetGenericVariable, id, val, 0);
	}

	u32 get_generic_rust_value(Unit* u, u32 val) {
		return get_aise_value(u, 0, AiseId::GenericValue, val, 0);
	}

	void set_generic_rust_value(Unit* u, u32 id, u32 val) {
		scbw::send_gptp_aise_command(u, 0, GptpId::SetGenericValue, id, val, 0);
	}

	void set_sprite_data(u32 entry, u32 index, u32 value) {
		return send_gptp_aise_command(0, 0, GptpId::SetSpriteData, entry, index, value);
	}

	const u32 Func_CanTargetSpell = 0x00492020;
	u32 get_aise_value(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2) {
		u32 result = 0;
		u32 v3 = 0; // pointer
		_asm {
			PUSHAD
			MOV ESI, v2 // fow_unit_id
			MOV EDI, unit2 // target
			MOV EDX, 50000 // tech
			PUSH v3 // outerror
			PUSH v1 // y
			PUSH command // x
			PUSH unit1 // unit
			CALL Func_CanTargetSpell
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_PsiPlacement = 0x00473920;
	u8 get_upgrade_level(u8 player, u8 upgradeId) {
		assert(player < PLAYER_COUNT);
//		assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);

		if (upgradeId < UpgradeId::PrismaticShield) {
			return UpgradesSc->current_level[player][upgradeId];
		/*
		} else if (upgradeId<UpgradeId::None) {
			return UpgradesBw->current_level[player][upgradeId - UpgradeId::PStructurePlating];
		*/
		} else {
			u32 result = 0;
			u32 player_u32 = player;
			u32 upgrade_u32 = upgradeId;
			_asm {
				PUSHAD
				MOV ESI, upgrade_u32
				MOV EAX, player_u32
				PUSH 0
				PUSH 50000
				CALL Func_PsiPlacement
				MOV result, EAX
				POPAD
			}
			return result;
		}
	}
	// EBX = max/current upgrade read, ECX - player, EDI - id

	u8 get_upgrade_max_level(u8 player, u8 upgradeId) {
		assert(player < PLAYER_COUNT);
//		assert(upgradeId < UpgradeId::None);
		assert(upgradeId < 256);
		if (upgradeId < UpgradeId::PrismaticShield) {
			return UpgradesSc->max_level[player][upgradeId];
		/*
		} else if (upgradeId < UpgradeId::None) {
			return UpgradesBw->max_level[player][upgradeId - UpgradeId::PStructurePlating];
		*/
		} else {
			u32 result = 0;
			u32 player_u32 = player;
			u32 upgrade_u32 = upgradeId;
			_asm {
				PUSHAD
				MOV ESI, upgrade_u32
				MOV EAX, player_u32
				PUSH 1
				PUSH 50000
				CALL Func_PsiPlacement
				MOV result, EAX
				POPAD
			}
			return result;
		}
	}

	void set_upgrade_level(u8 player, u8 upgradeId, u8 level) {
		assert(player < PLAYER_COUNT);
		assert(upgradeId < UpgradeId::None);
		if (upgradeId < UpgradeId::PrismaticShield) {
			UpgradesSc->current_level[player][upgradeId] = level;
		} else {
			UpgradesBw->current_level[player][upgradeId - UpgradeId::PrismaticShield] = level;
		}
	}

	void adjust_addon_x(u16 addonId, u16 sourceId, u16* x) {
		if (addonId == UnitId::TerranQuarry) {
			*x = 128;
		}
	}

	void adjust_addon_y(u16 addonId, u16 sourceId, u16* y) {
		if (addonId == UnitId::TerranQuarry) {
			*y = 32;
		}
	}

	void adjust_addon_coordinates(u16 addonId, u16 sourceId, u16* x, u16* y) {
		adjust_addon_x(addonId, sourceId, x);
		adjust_addon_y(addonId, sourceId, y);
	}

	//-------- Moving Units --------//

	bool move_unit(Unit* unit, s16 x, s16 y) {
		Point16 targetPos = {x, y};
		Point16 actualPos = {};
		Point16 prevPos = unit->sprite->position;

		// Based on the code for Recall/Nydus Canal/Move Unit trigger
		set_unit_position(unit, x, y);

		if (check_unit_collision_pos(unit, &targetPos, &actualPos)) {
			prepare_unit_move(unit);
			set_unit_position(unit, actualPos.x, actualPos.y);
			refresh_unit_after_move(unit);
			return true;
		} else {
			set_unit_position(unit, prevPos.x, prevPos.y);
			return false;
		}
	}

	bool move_unit_force(Unit* unit, s16 x, s16 y) {
		prepare_unit_move(unit);
		hooks::move_unit(unit, x, y, true);
		return true;
	}

	bool move_unit_improved(Unit* unit, s16 x, s16 y) {
		if (!(unit->status & UnitStatus::InAir)) {
			ActiveTile* original_tile = scbw::get_active_tile_at_coords(unit->position.x, unit->position.y);
			ActiveTile* final_tile = scbw::get_active_tile_at_coords(x, y);

			if (final_tile->is_unwalkable || original_tile->ground_height != final_tile->ground_height) {
				return false;
			}

			Unit* collision = scbw::UnitFinder(
				x - units_dat::unit_bounds[unit->id].left,
				y - units_dat::unit_bounds[unit->id].top,
				x + units_dat::unit_bounds[unit->id].right,
				y + units_dat::unit_bounds[unit->id].bottom
			).get_first(cosmonarchy_finder_match(unit, Flags::NotSelf | Flags::NoAir, {}));

			if (collision) {
				return false;
			}
		}

		prepare_unit_move_custom(unit);
		set_unit_position(unit, x, y);
		refresh_unit_after_move(unit);
		return true;
	}

	void prepare_unit_move_custom(Unit* unit) {
		helper::sort_unit(unit);
		helper::refresh_grp_image(unit->sprite);
		if (unit->subunit) {
			helper::refresh_grp_image(unit->subunit->sprite);
		}

		helper::toggle_unit_path(unit);
		char pathing_flags = unit->pathing_flags;
		unit->movement_state = 0;
		unit->pathing_flags = (pathing_flags ^ (unit->sprite->elevation < 12)) & 1 ^ pathing_flags;
	}

	//-------- Utility functions --------//

	Unit* create_unit_in_game(u16 unit_id, u8 player, u32 x, u32 y, Unit* creator, bool initAI) {
		if (*g_unit_count >= UnitLimit) {
			return 0; // Prevent crashes, move to create unit later. -Veeq7
		}
		Point16 pos = {x, y};
		helper::fix_target_location(&pos, unit_id);
		Unit* unit = helper::create_unit(unit_id, pos.x, pos.y, player);
		if (!unit) {
			helper::display_last_network_error_for_player(player);
			return 0;
		}
		helper::update_unit_stats_and_finish_building(unit);
		helper::update_unit_strength(unit);
		if (initAI) {
			scbw::give_ai_ex(unit); // no longer need creator if we are using this robust AI function, still keeping the argument for legacy reasons
		}
		return unit;
	}

	Unit* create_hangar_unit(Unit* source, u32 unit_id) {
		u32 count = source->unit_with_hangar.in_hangar_count;
		if (source->id == UnitId::ProtossSolarion) {
			count += source->unit_with_hangar.out_hangar_count;
		}
		if (count >= source->get_max_hangar_space()) {
			return 0;
		}
	
		Unit* unit = create_unit_in_game(unit_id, source->player, source->position.x, source->position.y, source, false);
		if (!unit) {
			return 0;
		}
		unit->hangar_unit.parent = source;
		helper::add_hangar_unit(source, unit);
		return unit;
	}

	void create_hangar_units(Unit* source, u32 count, u32 unit_id) {
		for (u32 i = 0; i < count; ++i) {
			create_hangar_unit(source, unit_id);
		}
	}

	u32 get_unit_overlay_adjustment(Unit* const unit) {
		if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
			return 2;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
			return 1;
		}
		return 0;
	}

	void refresh_screen(s32 left, s32 top, s32 right, s32 bottom) {
		left >>= 4; right = (right + 15) >> 4;
		top >>= 4; bottom = (bottom + 15) >> 4;

		if (left > right) {
			std::swap(left, right);
		}
		if (top > bottom) {
			std::swap(top, bottom);
		}

		// Rect out of bounds
		if (left >= 40 || right < 0 || top >= 30 || bottom < 0) {
			return;
		}

		left = std::max(left, 0); right = std::min(right, 40 - 1);
		top = std::max(top, 0); bottom = std::min(bottom, 30 - 1);

		for (s32 y = top; y <= bottom; ++y) {
			memset(&g_refresh_regions[40 * y + left], 1, right - left + 1);
		}
	}

	void refresh_screen() {
		memset(g_refresh_regions, 1, 1200);
	}

	/* /!\ Not working like 004DC550 rand_between /!\ */
	u32 rand_between(u32 min, u32 max) {
		assert(min <= max);
		return min + ((max - min + 1) * random() >> 15);
	}

	// using C stdlib rand
	u32 rand_between_local(u32 min, u32 max) {
		assert(min <= max);
		return rand() % (max - min + 1) + min;
	}

	// Similar to 004DC4A0 randomize_short, but doesn't update the RNG and doesn't use a parameter among possible differences.
	u16 random() {
		if (*g_is_in_game_loop) {
			*g_last_random_number = 22695477 * (*g_last_random_number) + 1;
			return (*g_last_random_number >> 16) % 32768; // Make a number between 0 and 32767
		}
		return 0;
	}

	void refresh_console() { // 004C36C0
		*g_force_button_refresh    = 1;
		*g_portrait_redraw           = 1;
		*g_status_screen_redraw      = 1;
		*g_control_under_mouse       = 0;
		*g_control_under_mouse_value = 0;
	}

	void create_unit_from_ability(s32 unit_id, s32 count, u16 player, Point16 position, Unit* creator) {
		u32 remove_timer = 0;
		switch (unit_id) {
			case UnitId::ZergRililisk:
			case UnitId::ZergRilirokor: {
				remove_timer = BroodspawnTimer;
			}
		}

		for (s32 i = 0; i < count; i++) {
			Unit* unit = scbw::create_unit_in_game(unit_id, player, position.x, position.y, creator, false);
			if (!unit) {
				return;
			}
			scbw::give_ai_ex(unit);
			if (creator && creator->status & UnitStatus::IsHallucination) {
				helper::apply_hallucination_stats(unit);
				unit->status |= UnitStatus::IsHallucination;
				unit->remove_timer = 1350;
				unit->sprite->create_overlay(ImageId::HallucinationDeath3);
			}
			switch (unit_id) {
				case UnitId::ZergRilirokor:
				case UnitId::ZergRililisk: {
					helper::play_sound_from_unit(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, unit, 1, 0);
					if (unit->remove_timer == 0 || unit->remove_timer > remove_timer) {
						unit->remove_timer = remove_timer;
					}
				} break;
				case UnitId::ProtossSimulacrum: {
					if (creator) {
						unit->hitpoints = std::max(256, creator->hitpoints);
						unit->shields = std::max(256, creator->shields);
						scbw::inherit_debuffs(creator, unit);
					}
					helper::play_sound_from_unit(SoundId::Simulacrum_Clone_01, unit, 1, 0);
				} break;
			}
		}
	}

	void range_modified_fire(u8 weapon_id, Unit* unit, s16 x, s16 y, u32 offset) {
		auto target_unit = unit->order_target.unit;
		auto target_pos = unit->order_target.pt;
		auto distance = scbw::get_distance_fast(unit->position.x, unit->position.y, unit->order_target.unit->position.x, unit->order_target.unit->position.y);
		distance += offset;
		if (distance < 0) {
			distance = 0;
		}
		auto polar_x = unit->get_x() + scbw::get_polar_x(distance, unit->current_direction_1);
		auto polar_y = unit->get_y() + scbw::get_polar_y(distance, unit->current_direction_1);
		unit->order_target.unit = nullptr;
		unit->order_target.pt.x = polar_x;
		unit->order_target.pt.y = polar_y;
		helper::create_bullet(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
		unit->order_target.unit = target_unit;
		unit->order_target.pt = target_pos;
	}

	bool can_spawn_broodling(Unit* target) {
		if (units_dat::group_flags[target->id].is_building) return false;
		return true;
	}

	s32 dir_saturate(s32 direction, s32 value) {
		s32 d = direction + value;
		if (d > 255) {
			return d - 255;
		}
		if (d < 0) {
			return d + 255;
		}
		return d;
	}

	u32 get_sized_overlay(Unit* unit, u32 initial) {
		if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
			return initial + 2;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
			return initial + 1;
		}
		return initial;
	}

	bool can_use_cliff_advantage(Unit* unit, u8 weapon) {
		return !(unit->status & UnitStatus::InAir)
		&&	   (!weapon || weapons_dat::max_range[weapon] > 32)
		&&		 unit->id != UnitId::TerranHarakan
		&&		 unit->id != UnitId::TerranMatadorCinderMode
		&&		 unit->id != UnitId::TerranMatadorTurretCinderMode
		&&		 weapon != WeaponId::DurendalAbaddonFlamethrowers;
	}

	s32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon) {
		if (!can_use_cliff_advantage(unit, weapon)) {
			return 0;
		}
		s32 source_height = scbw::get_active_tile_at_coords(unit->position.x, unit->position.y)->ground_height / 2;
		if (!target_unit) {
			return source_height * 2;
		}
		s32 target_height = scbw::get_active_tile_at_coords(target_unit->position.x, target_unit->position.y)->ground_height / 2;
		return std::max(source_height - target_height, 0) * 2;
	}

	void apply_ensnare(Unit* unit) { // 004F45E0
		if (!(unit->ensnare_timer)
			&& !(unit->status & UnitStatus::Burrowed)) {
			Unit* victim = unit->subunit ? unit->subunit : unit;
			victim->sprite->create_overlay(ImageId::EnsnareOverlay_Small + scbw::get_overlay_offset(unit->id));
		}
		unit->ensnare_timer = 37;
		unit->update_speed();
	}

	bool is_enabled(Unit* unit) {
		if (scbw::unit_is_stunned(unit) || unit->status & UnitStatus::Disabled) {
			return false;
		}
		return true;
	}

	s32 get_upgrade_type(Unit* unit) {
		s32 race = unit->get_race();
		switch (race) {
			case RaceId::Terran: {
				return UpgradeId::TerranInfantryWeapons;
			}
			case RaceId::Zerg: {
				return UpgradeId::ZergMeleeAttacks;
			} break;
			case RaceId::Protoss: {
				return UpgradeId::ProtossGroundWeapons;
			} break;
		}
		return UpgradeId::TerranInfantryWeapons;
	}

	s32 get_overlay_offset(u16 unit_id, bool xlarge_allowed) {
		if (xlarge_allowed && scbw::unit_id_uses_xlarge_overlays(unit_id)) {
			return 3;
		}
		if (units_dat::base_property[unit_id] & UnitProperty::LargeOverlay) {
			return 2;
		}
		if (units_dat::base_property[unit_id] & UnitProperty::MediumOverlay) {
			return 1;
		}
		return 0;
	}

	s32 get_final_damage(Unit* unit, u16 weapon_id) {
		s32 upgrade_type = get_upgrade_type(unit);
		s32 base_damage = weapons_dat::damage_amount[weapon_id];
		f64 bonus_damagge = (f64)weapons_dat::armor_pen[weapon_id] * (f64)scbw::get_upgrade_level(unit->player, upgrade_type);
		f64 final_damage = base_damage;
		if (bonus_damagge) {
			final_damage = base_damage + bonus_damagge;
		}
		return final_damage;
	}

	// I may need to make this function more flexible later -- Pr0nogo
	bool unit_is_pacifist(Unit* unit) {
		return !unit->has_weapon()
		&&		unit->id != UnitId::ZergLakizilisk; // LAME
	}

	// Identical to function order_is_spell @ 00436930 -- Pr0nogo
	// We should update the lardcode for Cosmonarchy
	bool order_is_spell(s32 order_id) {
		return order_id == OrderId::Plague
			|| order_id == OrderId::CarapaceSwarm
//			|| order_id == OrderId::Ensnare				// Land Grab
			|| order_id == OrderId::Irradiate
			|| order_id == OrderId::StasisField
			|| order_id == OrderId::EMPShockwave		// Observance
			|| order_id == OrderId::MagnaPulse			// Lobotomy Mine
			|| order_id == OrderId::PsiStorm
//			|| order_id == OrderId::SummonBroodlings	// unused
			|| order_id == OrderId::FireYamatoGun1;
	}

	// Creates the Plasma Shield flickering effect.
	// Identical to function @ 0x004E6140
	void create_shield_overlay(Unit* unit, u32 attackDirection) {
		const LO_Header* shield_lo = g_shield_overlays[unit->sprite->main_graphic->id];
		u32 frame_angle = (attackDirection - 124) / 8 % 32;

		Point8 offset = shield_lo->get_offset(unit->sprite->main_graphic->direction, frame_angle);
		unit->sprite->create_overlay(ImageId::ShieldOverlay, offset.x, offset.y, frame_angle);
	}

	void decloak_proc(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (is_decloakable(unit)) {
			if (unit_ext->timers[TimerDecloak] == 0) {
				helper::end_cloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);
			}
			if (unit_ext->timers[TimerDecloak] != 1) {
				unit_ext->timers[TimerDecloak] = 60 + 1;
			}
			unit_ext->timers[TimerSilvertonguePlotTwist] = 0;
		}
		if (unit->is_subunit() && unit->subunit) {
			decloak_proc(unit->subunit);
		}
	}

	bool is_decloakable(Unit* unit) {
		return (units_dat::base_property[unit->id] & UnitProperty::PermanentCloak || unit->status & UnitStatus::CloakingForFree) && !(unit->status & UnitStatus::Burrowed);
	}

	// Replacement for isUnitGhost
	bool unit_can_nuke(Unit* unit) {
		return (unit->id == UnitId::TerranEidolon || unit->id == UnitId::TerranSeraph);
	}

	bool unit_is_trap(Unit* unit) {
		return (unit->id == UnitId::TerranSpiderMine);
	}

	bool unit_is_ranged_attacker(Unit* unit) {
		// Would be nice to somehow avoid ID hardcode
		switch (unit->id)
		{
			// Prohibited IDs
			case UnitId::ProtossAmaranth:
			case UnitId::ZergAkistrokor:
			case UnitId::ZergUltrokor:
			{
				return false;
			} break;
			// Allowed IDs
			case UnitId::TerranHarakan:
			case UnitId::TerranGoliathTurret:
			case UnitId::TerranPhalanxTankMode:
			case UnitId::TerranPhalanxSiegeMode:
			case UnitId::TerranPhalanxTankModeTurret:
			case UnitId::TerranPhalanxSiegeModeTurret:
			case UnitId::TerranMatadorTankMode:
			case UnitId::TerranMatadorCinderMode:
			case UnitId::TerranMatadorTurretTankMode:
			case UnitId::TerranMatadorTurretCinderMode:
			{
				return true;
			} break;
		}

		u16 ground_weapon_id = units_dat::ground_weapon[unit->id];
		if (unit->subunit) {
			ground_weapon_id = units_dat::ground_weapon[unit->subunit->id];
		}
		if (ground_weapon_id != WeaponId::None) {
			if (weapons_dat::max_range[ground_weapon_id] > 32) {
				return true;
			}
		}
		u16 air_weapon_id = units_dat::air_weapon[unit->id];
		if (unit->subunit) {
			air_weapon_id = units_dat::air_weapon[unit->subunit->id];
		}
		if (air_weapon_id != WeaponId::None) {
			if (weapons_dat::max_range[air_weapon_id] > 32) {
				return true;
			}
		}
		return false;
	}

	void play_work_animation(Unit* unit) {
		Image* current_image = unit->sprite->images.head;
		unit->secondary_order_state = 2;
		while (current_image) {
			current_image->play_iscript_animation(IscriptAnimation::IsWorking);
			current_image = current_image->link.next;
		}
	}

	bool unit_id_is_available_without_an_addon(u16 id) {
		return id == UnitId::TerranMason
			|| id == UnitId::TerranTalos
			|| id == UnitId::TerranMaverick
			|| id == UnitId::TerranHarakan
			|| id == UnitId::TerranCyprian
			|| id == UnitId::TerranVulture
			|| id == UnitId::TerranCyclops
			|| id == UnitId::TerranGoliath
			|| id == UnitId::TerranWraith
			|| id == UnitId::TerranAncile
			|| id == UnitId::TerranTrojan
			|| id == UnitId::TerranMadcap
			|| id == UnitId::TerranHeracles
			|| id == UnitId::TerranApostle
			|| id == UnitId::TerranRamesses
			|| id == UnitId::TerranDurendal
			|| id == UnitId::TerranCataphract
			|| id == UnitId::TerranGorgon
			|| id == UnitId::TerranWyvern
			|| id == UnitId::TerranValkyrie
			|| id == UnitId::TerranTalos
			|| id == UnitId::TerranArmistice
			|| id == UnitId::TerranPythean
			|| id == UnitId::TerranDilettante
			|| id == UnitId::TerranSevenstep
			|| id == UnitId::TerranSilvertongue
			|| id == UnitId::TerranHypnagogue
			|| id == UnitId::TerranPaladin
			|| id == UnitId::TerranCuirassTankMode
			|| id == UnitId::TerranPazuzu
			|| id == UnitId::TerranCentaur
			|| id == UnitId::TerranMinotaur
			|| id == UnitId::TerranMagnetar;
	}

	void apply_hallucinating(Unit* unit, u8 player, u32 time) {
		UnitExt* unit_ext = get_unit_ext(unit);

		Unit* context = unit->subunit ? unit->subunit : unit;
		if (!context->get_overlay(ImageId::HallucinationHit)) {
			// Overlay
			context->sprite->create_overlay(ImageId::HallucinationHit, 0, 0, 0);

			// Sound
			if (!unit_ext->timers[TimerHallucinating]) {
				helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, unit, 1, 0);
			} else {
				helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
			}
		}
		
		// Value
		unit_ext->timers[TimerHallucinating] = time;
	}

	bool can_be_entered_by(Unit* transport, Unit* unit) {
		const u32 func_address = 0x004E6E00; // AKA CanEnterTransport()
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			PUSH unit
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void show_error_message_with_sfx(u32 player, u32 statTxtId, u32 sfxId) {
		const u32 func_address = 0x0048EE30;
		__asm {
			PUSHAD
			MOV ESI, sfxId
			MOV EDI, statTxtId
			MOV EBX, player
			CALL func_address
			POPAD
		}
	}

	u16 get_tech_user_error_message(Unit* target, u8 castingPlayer, u16 techId) {
		const u32 func_address = 0x00491E80;
		static u32 tech_id_xd;
		static u16 error_message_xd;
		tech_id_xd = techId;
		__asm {
			PUSHAD
			PUSH tech_id_xd
			MOV BL, castingPlayer
			MOV EAX, target
			CALL func_address
			MOV error_message_xd, AX
			POPAD
		}
		return error_message_xd;
	}

	u16 get_last_queue_slot_id(Unit* a1) {
		const u32 func_address = 0x0047B270;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void set_offset_to_parent_special_overlay(Image* image) {
		const u32 func_address = 0x004D5A00;
		__asm {
			PUSHAD
			MOV ECX, image
			CALL func_address
			POPAD
		}
	}

	u32 get_ground_height_at_pos(s32 x, s32 y) {
		const u32 func_address = 0x004BD0F0;
		static u32 height;
		__asm {
			PUSHAD
			MOV EAX, y
			MOV ECX, x
			CALL func_address
			MOV height, EAX
			POPAD
		}
		return height;
	}

	void disable_dialog(BinDlg* dialog) {
		const u32 func_address = 0x00418640;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	void show_dialog(BinDlg* dialog) {
		const u32 func_address = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	void hide_dialog(BinDlg* dialog) {
		const u32 func_address = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	void enable_control(BinDlg* dialog) {
		const u32 func_address = 0x00418E00;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL func_address
			POPAD
		}
	}

	void update_dialog(BinDlg* dialog) {
		const u32 func_address = 0x0041C400;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL func_address
			POPAD
		}
	}

	void prepare_unit_move(Unit* unit, bool hideUnit) {
		const u32 func_address = 0x00493CA0;
		assert(unit);
		static b32 hide_unit_xd;
		hide_unit_xd = hideUnit ? 1 : 0;
		__asm {
			PUSHAD
			PUSH hide_unit_xd
			MOV EDI, unit
			CALL func_address
			POPAD
		}
	}

	bool check_unit_collision_pos(Unit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea, bool hideErrorMsg, u32 someFlag) {
		const u32 func_address = 0x0049D3E0;
		assert(unit);
		assert(inPos);
		assert(outPos);
		static u32 result;
		static u32 hide_error_message_xd;
		hide_error_message_xd = hideErrorMsg;
		__asm {
			PUSHAD
			PUSH someFlag;
			PUSH hide_error_message_xd
			PUSH outPos
			PUSH inPos
			PUSH unit
			MOV EAX, moveArea
			CALL func_address
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void set_unit_position(Unit* unit, u16 x, u16 y) {
		const u32 Func_SetUnitPosition = 0x004EB9F0;
		assert(unit);
		__asm {
			PUSHAD
			MOV CX, y
			MOV AX, x
			MOV EDX, unit
			CALL Func_SetUnitPosition
			POPAD
		}
	}

	void refresh_unit_after_move(Unit* unit) {
		const u32 Func_RefreshRevealUnitAfterMove = 0x00494160;
		assert(unit);
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefreshRevealUnitAfterMove
			POPAD
		}
	}

	void make_to_hold_position(Unit* unit) {
		const u32 func_address = 0x004EB5B0;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL func_address
			POPAD
		}
	}

	// Player check added based on how others functions test it. Values above or equal to 8 means all players.
	void move_screen_to_unit(Unit* unit, u32 player) { // 004E6020
		const u32 func_address = 0x0049C440;
		if (player >= PlayerLimit || player == *g_local_nation_id) {
			s32 x = unit->sprite->position.x;
			s32 y = unit->sprite->position.y;
			if (x < 0) {
				x += 31;
			}
			if (y < 0) {
				y += 31;
			}
			x = ((x / 32) - 10) * 32;
			y = ((y / 32) - 6) * 32;
			__asm {
				PUSHAD
				MOV ECX, y
				MOV EAX, x
				CALL func_address
				POPAD
			}
		}
	}

	// Player check added based on how others functions test it
	// Values above or equal to 8 means all players
	// x and y are coords as gotten from unit->sprite->position
	void minimap_ping(u32 x, u32 y, s32 color, u32 player) {
		const u32 func_address = 0x004A34C0;
		if (player >= PlayerLimit || player == *g_local_nation_id) {
			__asm {
				PUSHAD
				PUSH color
				PUSH y
				PUSH x
				CALL func_address
				POPAD
			}
		}
	}

	u8 overlay_size(Unit* unit) {
		const u32 func_address = 0x0047B720;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			MOV result, AL
			POPAD
		}
		return result;
	}

	void hide_and_disable_unit(Unit* unit) {
		const u32 func_address = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_address
			POPAD
		}
	}

	u32 weapon_id_intercepts(u32 weapon_id) {
		switch (weapon_id) {
			case WeaponId::AccantorPlasmaShell:
				return InterceptType_GroundOnly;

			case WeaponId::EmpyreanPlasmaBarrage:
			case WeaponId::ValkyrieHaloRockets:
				return InterceptType_AirOnly;

			case WeaponId::SirenCharybdalRifle:
			case WeaponId::StarSovereignInterventionMissiles:
				return InterceptType_Any;
		}
		return InterceptType_None;
	}

	bool weapon_id_pierces(u32 weapon_id) {
		switch (weapon_id) {
			case WeaponId::CyprianTwinRailguns:
			case WeaponId::SavantDiscoidRailgunPiercing:
			case WeaponId::AutocratEminenceRifle:
			case WeaponId::SevenstepGeometricRiflePiercing:
			case WeaponId::RamessesPharoahCannons:
			case WeaponId::ClaymoreHarpoonRailgun:
			case WeaponId::PenumbraErebolShockCannon:
			case WeaponId::MagnetarNewtonianShockCannon:
			case WeaponId::HippogriffChimeraCannons:
			case WeaponId::SentinelCerberusRailgun:
			case WeaponId::OptectonSunsplitBeam:
			case WeaponId::PatriarchIronhandWeaver:
			case WeaponId::KalkaliskViscousAcid:
			case WeaponId::ZarcavrokorBonesofReflection:
				return true;
		}
		return false;
	}

	bool weapon_id_piercing_stops_on_armor(u32 weapon_id) {
		switch (weapon_id) {
			case WeaponId::PenumbraErebolShockCannon:
				return false;
		}
		return true;
	}

	void madrigal_dirge_fuse(Unit* unit, UnitExt* unit_ext) {
		unit->sprite->create_top_overlay(ImageId::MadrigalDirgeProc);
		const u32 radius = 2 * 32;
		// TODO: Unit search
		scbw::UnitFinder(
			unit->get_x() - radius,
			unit->get_y() - radius,
			unit->get_x() + radius,
			unit->get_y() + radius
		).for_each_in_match(cosmonarchy_finder_match(unit, Flags::IncludingSelf, {}), [&unit_ext](Unit* target) {
			target->damage_with_custom(unit_ext->values[ValueDirgeStacks] * MadrigalDirgeDamage * 256, 5, unit_ext->units[UnitDirgeSource], unit_ext->values[ValueDirgeSourcePlayer]);
		});
		unit_ext->values[ValueDirgeStacks] = 0;
	}

	void update_signify_status(Unit* unit, UnitExt* unit_ext) {
		unit->update_speed();

		const u32 cowardice_id = ImageId::Cowardice_Small + scbw::get_overlay_offset(unit->id);
		const u32 malice_id = ImageId::Malice_Small + scbw::get_overlay_offset(unit->id);
		if (unit_ext->values[ValueSignifyStacks] == 0) {
			unit->remove_overlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
			unit->remove_overlay(ImageId::Malice_Small, ImageId::Malice_Large);
		} else if (!(units_dat::base_property[unit->id] & UnitProperty::Building)
				|| units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding
				|| unit->has_weapon()) {
			if (unit_ext->values[ValueSignifyStacks] < 5 && !unit->get_overlay(cowardice_id)) {
				unit->sprite->create_top_overlay(cowardice_id);
			} else if (unit_ext->values[ValueSignifyStacks] >= 5 && !unit->get_overlay(malice_id)) {
				unit->remove_overlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
				unit->sprite->create_top_overlay(malice_id);
			}
		}
	}

	bool is_unit_isolated(Unit* unit) {
		u32 ally_count = 0;
		u32 enemy_count = 0;

		auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
		const u32 radius = 5 * 32;
		scbw::UnitFinder(
			unit->get_x() - radius,
			unit->get_y() - radius,
			unit->get_x() + radius,
			unit->get_y() + radius
		).for_each_in_match(match, [&](Unit* target) {
			if (scbw::is_allied_to(target->player, unit->player))
			{
				ally_count++;
			}
			else
			{
				enemy_count++;
			}
		});

		return ally_count < enemy_count;
	}

	bool unit_id_is_zerg_supply(u16 unit_id) {
		return (unit_id == UnitId::ZergOvileth
			|| unit_id == UnitId::ZergIroleth
			|| unit_id == UnitId::ZergOthstoleth
			|| unit_id == UnitId::ZergAlaszileth);
	}

	void empress_zombify(u32 player, Unit* source, Unit* target, u32 stacks) {
		UnitExt* target_ext = get_unit_ext(target);
		target_ext->units[UnitMindController] = 0;
		target_ext->values[ValueMindControlOriginalPlayer] = 0;

		// Recurse
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* unit = target->get_loaded_unit(i);
			if (unit) {
				empress_zombify(player, source, unit, stacks);
			}
		}
		for (Unit* unit : target_ext->unit_vectors[UnitVectorMindControlling]) {
			empress_zombify(player, source, unit, stacks);
		}
		target_ext->unit_vectors[UnitVectorMindControlling].clear();

		// Obtain control
		target->give_to(player);

		target->hitpoints = 0;
		target->shields = 0;
		target->heal_hp_then_shields(stacks * 256 * 5, source);
		target->remove_timer = 1 + stacks * 24 * 1;

		target->give_to(player);
		target_ext->values[ValueEmpressPassiveStacks] = 0;
		target_ext->values[ValueIsZombified] = 1;
		target_ext->values[ValueAlliedInvPlayer] = player;
		target_ext->timers[TimerAlliedInvFrames] = 24;

		// Audio Visuals
		target->sprite->create_overlay(ImageId::MindControlHit_Small + scbw::get_overlay_offset(target->id));
		helper::play_sound_from_unit(scbw::rand_between(SoundId::EmpressZombify01, SoundId::EmpressZombify03), target);
	}

	void mind_control(Unit* source, Unit* target) {
		if (!source || !target || source->player == target->player || scbw::is_allied_to(source->player, target->player)) {
			return;
		}

		UnitExt* source_ext = get_unit_ext(source);
		source_ext->unit_vectors[UnitVectorMindControlling].push_back(target);

		UnitExt* target_ext = get_unit_ext(target);
		target_ext->units[UnitMindController] = source;
		if (!target_ext->values[ValueMindControlOriginalPlayer]) {
			target_ext->values[ValueMindControlOriginalPlayer] = target->player + 1;
		}

		// Recurse
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* unit = target->get_loaded_unit(i);
			if (unit) {
				mind_control(source, unit);
			}
		}
		for (Unit* unit : target_ext->unit_vectors[UnitVectorMindControlling]) {
			mind_control(source, unit);
		}
		target_ext->unit_vectors[UnitVectorMindControlling].clear();

		// Mind Tyrant - apply behavior
		if (source->id == UnitId::ProtossMindTyrant) {
			target_ext->values[ValueTyrannyPlayer] = source->player + 1;
			target_ext->timers[TimerTyrannyRecede] = 5 * 24;
		}
		// Demiurge - apply behavior
		if (source->id == UnitId::ProtossDemiurge) {
			target_ext->values[ValueWorshipStack] = 1;
		}

		// Obtain control
		target->give_to(source->player);
		target_ext->values[ValueAlliedInvPlayer] = source->player;
		target_ext->timers[TimerAlliedInvFrames] = 24;

		// Audio Visuals
		if (!(source->sprite->flags & SpriteFlags::Hidden)) {
			if (source->id == UnitId::ProtossDemiurge) {
				target->sprite->create_top_overlay(ImageId::Worship_Small + scbw::get_overlay_offset(target->id), 0, 0, 0);
				helper::play_sound_from_unit(SoundId::DemiurgeWorship01, target, 1, 0);
			} else if (source->id == UnitId::ProtossMindTyrant) {
				target->sprite->create_top_overlay(ImageId::MindControlHit_Small + scbw::get_overlay_offset(target->id), 0, 0, 0);
			}
		}
	}

	void recede_mind_control(Unit* target) {
		if (!target) {
			return;
		}

		UnitExt* target_ext = get_unit_ext(target);
		Unit* mind_controller = target_ext->units[UnitMindController];
		if (mind_controller) {
			UnitExt* mind_controller_ext = get_unit_ext(mind_controller);
			auto& vec = mind_controller_ext->unit_vectors[UnitVectorMindControlling];
			vec.erase(std::remove(vec.begin(), vec.end(), target), vec.end());
		}
		target_ext->units[UnitMindController] = 0;

		if (target_ext->values[ValueMindControlOriginalPlayer]) {
			target->give_to(target_ext->values[ValueMindControlOriginalPlayer] - 1);
			target_ext->values[ValueMindControlOriginalPlayer] = 0;
		}
		target_ext->values[ValueTyrannyPlayer] = 0;
	}

	bool unit_id_is_zerg_production(u16 unit_id, bool town_center_only) {
		if (town_center_only) {
			return unit_id == UnitId::ZergHachirosk
				|| unit_id == UnitId::ZergCaphrolosk
				|| unit_id == UnitId::ZergGathtelosk;
		} else {
			return unit_id == UnitId::ZergHachirosk
				|| unit_id == UnitId::ZergCaphrolosk
				|| unit_id == UnitId::ZergGathtelosk
				|| unit_id == UnitId::ZergLarvoskColony
				|| unit_id == UnitId::ZergOcsalColony;
		}
	}
	
	bool unit_id_is_town_center(u16 unit_id) {
		return unit_id == UnitId::TerranCommandCenter
			|| unit_id == UnitId::ProtossNexus
			|| unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk;
	}

	bool unit_id_is_factional(u16 unit_id) {
		return unit_id == UnitId::TerranAspirant
			|| unit_id == UnitId::TerranCohort
			|| unit_id == UnitId::TerranLuminary
			|| unit_id == UnitId::TerranTarasque;
	}

	bool transport_is_full(Unit* transport) {
		UnitExt* transport_ext = get_unit_ext(transport);
		for (u32 i = 0; i < TransportCap; ++i) {
			if (!transport_ext->loaded_unit[i].index) {
				return false;
			}
		}
		return true;
	}

	bool transport_is_empty(Unit* transport) {
		UnitExt* transport_ext = get_unit_ext(transport);
		for (u32 i = 0; i < TransportCap; ++i) {
			if (transport_ext->loaded_unit[i].index) {
				return false;
			}
		}
		return true;
	}

	bool unit_id_uses_xlarge_overlays(u16 unit_id) {
		return unit_id == UnitId::TerranPenumbra
			|| unit_id == UnitId::TerranPhobos
			|| unit_id == UnitId::ProtossStarSovereign
			|| unit_id == UnitId::ZergOthstoleth
			|| unit_id == UnitId::ZergAlkajelisk;
	}

	bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures) {
		if (count_gas_structures) {
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser
				|| unit_id == UnitId::ResourceVespeneRidge
				|| unit_id == UnitId::TerranReservoir
				|| unit_id == UnitId::TerranWellbore
				|| unit_id == UnitId::ProtossAquifer
				|| unit_id == UnitId::ZergExcisant
				|| unit_id == UnitId::ZergEzigrisant;
		} else {
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser
				|| unit_id == UnitId::ResourceVespeneRidge;
		}
	}

	bool unit_id_is_minerals(u16 unit_id) {
		return unit_id == UnitId::ResourceMineralField
			|| unit_id == UnitId::ResourceMineralFieldType2
			|| unit_id == UnitId::ResourceMineralFieldType3;
	}

	void set_cmbw_ai_preference(u16 unit_id, u32 player) {
		g_unit_deaths_table[unit_id].player[player] = 1;
	}

	Box32 unit_area(Unit* unit) {
		Box32 area = {};
		area.w = units_dat::building_dimensions[unit->id].x;
		area.h = units_dat::building_dimensions[unit->id].y;
		area.x = unit->position.x - area.w / 2;
		area.y = unit->position.y - area.h / 2;
		return area;
	}

	void clear_creep(Box32 area) {
		for (u32 x = 0; x < area.w / 32; ++x) {
			for (u32 y = 0; y < area.h / 32; ++y) {
				u32 final_x = area.x + x * 32;
				u32 final_y = area.y + y * 32;
				helper::remove_creep_at_unit(final_x, final_y, UnitId::ZergEgg);
				// TODO: update creep so it respreads if possible, also maybe don't remove creep from underneath zerg structures?
			}
		}
	}
	
	u32 get_closest_resarea(Point16 position) {
		u32 min_distance = UINT32_MAX;
		u32 best_resarea = 0;
		for (u32 i = 1; i <= g_resource_areas->count; ++i) {
			ResourceArea* resarea = &g_resource_areas->areas[i];
			u32 distance = scbw::get_distance_fast(position, resarea->position);
			if (distance < min_distance) {
				min_distance = distance;
				best_resarea = i;
			}
		}
		return best_resarea;
	}
	
	AiTown* get_closest_town(u32 player, Point16 position) {
		u32 min_distance = UINT32_MAX;
		AiTown* best_town = 0;
		for (AiTown* town = g_ai_town_list[player].first; town; town = town->prev) {
			if (!town->inited) {
				continue;
			}
			u32 distance = scbw::get_distance_fast(position, town->position);
			if (distance < min_distance) {
				min_distance = distance;
				best_town = town;
			}
		}
		return best_town;
	}

	Point16 get_region_center(Region* region) {
		Point16 center = {};
		center.x = region->region_center_x >> 8;
		center.y = region->region_center_y >> 8;
		return center;
	}

	Box32 get_region_area(Region* region) {
		Box32 area = {};
		area.x = region->left;
		area.y = region->top;
		area.w = region->right;
		area.h = region->bottom;
		return area;
	}

	u32 get_ai_region_military_count(AiRegion* ai_region) {
		u32 count = 0;
		for (MilitaryAi* military_ai = ai_region->military.first; military_ai; military_ai = military_ai->link.prev) {
			count += 1;
		}
		return count;
	}

	void ai_defend(u32 player, u32 region_id) {
		AiRegion* ai_region = scbw::get_ai_region(player, region_id);
		switch (ai_region->state) {
			case RegionState::Nothing:
			case RegionState::EnemySighted:
			case RegionState::TownRegionLow:
			case RegionState::TownRegionHigh:
			case RegionState::Defending:
			case RegionState::ChaseAway: {
				helper::ai_set_region_state(RegionState::Defending, ai_region); // Priority is 110 when defending regions with buildings/workers, 100 otherwise
				ai_region->defense_priority += 15; // More than both of the vanilla defense priorities
				ai_region->defend_time = 2; // The minimal time (around ~60 frames)
			} break;
		}
	}

	void ask_for_ai_help(Unit* victim, Unit* attacker) {
		if (!victim || !attacker) {
			return;
		}

		EntryArray allied_structures = {};
		unit_search_radial(&allied_structures, victim->position, 20 * 32);
		unit_search_filter(&allied_structures, victim, FilterIncludingSelf | FilterAlly | FilterBuilding);
		if (allied_structures.count) {
			for (u32 ai_player = 0; ai_player < 8; ++ai_player) {
				if (g_player_table[ai_player].type == PlayerType::Computer && victim->is_visible_to(ai_player) &&
					!scbw::is_allied_to(ai_player, attacker->player) && scbw::is_allied_to(ai_player, victim->player)) {
					scbw::ai_defend(ai_player, scbw::get_region_id(attacker->position));
				}
			}
		}
	}

	bool unit_id_is_capped_gas_structure(u16 unit_id) {
		return unit_id == UnitId::TerranReservoir
		||	   unit_id == UnitId::TerranWellbore
		||	   unit_id == UnitId::ProtossAquifer
		||	   unit_id == UnitId::ZergExcisant
		||	   unit_id == UnitId::ZergEzigrisant;
	}

	bool unit_id_is_gas_structure(u16 unit_id) {
		return unit_id_is_capped_gas_structure(unit_id)
		||	   unit_id == UnitId::ResourceVespeneRidge
		||	   unit_id == UnitId::ResourceVespeneGeyser;
	}

	bool unit_id_is_smaller_building(u16 unit_id) {
		return (units_dat::base_property[unit_id] & UnitProperty::Addon &&
			(unit_id != UnitId::TerranFountainhead && unit_id != UnitId::TerranAtelier && unit_id != UnitId::TerranParticleAccelerator))
			|| unit_id == UnitId::TerranSupplyDepot
			|| unit_id == UnitId::TerranWatchdog
			|| unit_id == UnitId::TerranBunker
			|| unit_id == UnitId::TerranSentinel
			|| unit_id == UnitId::ProtossPylon
			|| unit_id == UnitId::ProtossWarden
			|| unit_id == UnitId::ProtossMatrix
			|| unit_id == UnitId::ZergCagrantColony
			|| unit_id == UnitId::ZergSurkithColony
			|| unit_id == UnitId::ZergSpraithColony
			|| unit_id == UnitId::ZergLarvoskColony
			|| unit_id == UnitId::ZergSkortrithColony
			|| unit_id == UnitId::ZergSpirtithColony
			|| unit_id == UnitId::ZergKazitrithColony
			|| unit_id == UnitId::ZergOcsalColony
			|| unit_id == UnitId::ZergNydusCesiant;
	}

	void spawn_custom_creep_at(Point16 pos, u32 unit_id, bool more_creep) {
		u32 w = units_dat::building_dimensions[unit_id].x / 32;
		u32 h = units_dat::building_dimensions[unit_id].y / 32;
		u32 x = pos.x / 32 - w / 2;
		u32 y = pos.y / 32 - h / 2;

		helper::clear_tile_flags_from_area(0x8000000, x, y, w, h);
		if (more_creep) {
			helper::set_creep(x - 2, y - 1, w + 4, h + 2, true);
		} else {
			helper::set_creep(x, y, w, h, false);
		}
		helper::set_map_tile_flags(0x8000000, x, y, w, h);
	}

	void spawn_custom_creep(Unit* unit) {
		spawn_custom_creep_at(unit->position, unit->id, units_dat::group_flags[unit->id].is_factory);
	}

	static void stop_custom_creep_disappearing(Unit* unit) {
		Box32 rect = {};
		bool spreads_creep = helper::get_creep_spread_area(UnitId::ZergGathtelosk, unit->position.x, unit->position.y, true, &rect);
		if (rect.top > rect.bottom) {
			return;
		}

		for (u32 y_tile = rect.top; y_tile <= rect.bottom; ++y_tile) {
			for (u32 x_tile = rect.left; x_tile <= rect.right; ++x_tile) {
				if ((*g_active_tile_array_int)[x_tile + y_tile * g_map_tile_size->width] & 0x10000000) {
					u32 x = x_tile * 32 - unit->position.x + 16;
					u32 y = y_tile * 32 - unit->position.y + 16;
					if (!spreads_creep || 100 * x * x + (y * y << 8) <= 10240000) {
						helper::stop_creep_disappearing(x_tile, y_tile);
					}
				}
			}
		}
	}

	void spread_custom_creep(Unit* unit) {
		scbw::stop_custom_creep_disappearing(unit);
		helper::spread_creep(UnitId::ZergGathtelosk, unit->position.x, unit->position.y);
	}

	void update_custom_creep_disappearance(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
			helper::update_creep_disappearance_unit_id(UnitId::ZergGathtelosk, unit->position.x, unit->position.y, true);
		}
	}

	void blissful_embrace(Unit* unit, u32 player) {
		// Sanity check. Spawning creep wouldn't work properly on units. Spawning creep on units or addons could corrupt/crash the game. Addons don't transfer properly anyways
		if (!(units_dat::base_property[unit->id] & UnitProperty::Building) || units_dat::base_property[unit->id] & UnitProperty::Addon) {
			return;
		}

		UnitExt* targetExt = get_unit_ext(unit);

		// Obtain control
		if (!targetExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
			targetExt->values[ValueBlissfulEmbraceOriginalPlayer] = unit->player + 1;
			targetExt->values[ValueBlissfulEmbraceOriginalHealth] = unit->hitpoints;
		}
		unit->give_to(player);

		// Spread initial creep
		scbw::spawn_custom_creep(unit);


		// Remove connected larva
		u16 larva_id = UnitId::ZergLarva;
		if (unit->id == UnitId::ZergCaphrolosk || unit->id == UnitId::ZergOcsalColony)
		{
			larva_id = UnitId::ZergOcsalLarva;
		}

		EntryArray larvae = {};
		unit_search_radial(&larvae, unit->position, 4 * 32);
		unit_search_filter(&larvae, unit, 0, {larva_id});
		for (u32 i = 0; i < larvae.count; ++i) {
			Unit* larva = larvae.units[i];
			if (larva->connected_unit == unit) {
				larva->connected_unit = 0;
			}
		}

		// Spawn infested overlay
		u32 infested_image_id = ImageId::InfestedOverlay_Large;
		if (scbw::unit_id_is_smaller_building(unit->id)) {
			infested_image_id = ImageId::InfestedOverlay_Small;
		}
		if (!unit->get_overlay(infested_image_id)) {
			unit->sprite->create_top_overlay(infested_image_id);
		}

		// Spawn effect overlay | TODO: Make this a sprite
		u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
		if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
			effect_image_id = ImageId::ZergBuildingExplosion;
		} else if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
			effect_image_id = ImageId::ZergAirDeathExplosion_Large;
		}
		unit->sprite->create_overlay(effect_image_id);

		// Play sound
		helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZBldgDth_WAV, unit, true, 0);
	}

	b32 unit_id_is_tech(u32 unit_id) {
		if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
			switch (unit_id) {
				case UnitId::TerranComsatStation:
				case UnitId::TerranMissileSilo:
				case UnitId::TerranQuarry:
					return false;
			}
			return true;
		}

		switch (unit_id) {
			// Terran
			case UnitId::TerranAtlas:
			case UnitId::TerranFountainhead:
			case UnitId::TerranDaedala:
			case UnitId::TerranAtelier:

			// Zerg
			case UnitId::ZergIrolIris:
			case UnitId::ZergIrulentIvoa:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergOrbothOmlia:
			case UnitId::ZergQuazrokPool:
			case UnitId::ZergLiivralPond:
			case UnitId::ZergMuthrokSpire:
			case UnitId::ZergVithrathHaunt:
			case UnitId::ZergAxtothAxis:
			case UnitId::ZergBegrozithBarrow:
			case UnitId::ZergHydrokDen:
			case UnitId::ZergGorgralSwamp:
			case UnitId::ZergIzkagIteth:
			case UnitId::ZergChokralgantChazsal:
			case UnitId::ZergGeszkathGrotto:
			case UnitId::ZergAlkajChasm:
			case UnitId::ZergZorkizShroud:
			case UnitId::ZergUltrokCavern:
			case UnitId::ZergMatravilNest:
			case UnitId::ZergAlmakisAntre:
			case UnitId::ZergElthisthMound:
			case UnitId::ZergRakzogthRoost:
				return true;
		}
		return false;
	}

	b32 unit_id_is_tech_tier_structure(u32 unit_id) {
		switch (unit_id) {
			// Terran
			case UnitId::TerranAtlas:
			case UnitId::TerranFountainhead:
			case UnitId::TerranDaedala:
			case UnitId::TerranAtelier:

			// Zerg
			case UnitId::ZergIrolIris:
			case UnitId::ZergIrulentIvoa:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergOrbothOmlia:
				return true;
		}
		return false;
	}

	AiScript* run_ai(u32 player, Box32 area, s32 name) {
		s32 is_bw = 0;
		if (!helper::is_ai_script_name_valid(player, name)) {
			return 0;
		}
		s32 block = helper::parse_aiscript_name(name, &is_bw);
		if (block) {
			AiScript* ai_script = helper::run_aiscript(&area, player, block, is_bw);
			if (ai_script) {
				Point16 center = {};
				center.x = area.left + (area.right - area.left)   / 2;
				center.y = area.top  + (area.top   - area.bottom) / 2;
				ai_script->flags |= scbw::get_closest_resarea(center) << 3;
			}
			return ai_script;
		} else {
			scbw::printf("scenario.cpp, run_ai: Invalid AI block!");
		}
		return 0;
	}

	AiScript* run_ai(u32 player, Point16 pos, s32 name) {
		Box32 area = {};
		area.left = pos.x - 2 * 32;
		area.right = pos.x + 2 * 32;
		area.top = pos.y - 1.5 * 32;
		area.bottom = pos.y + 1.5 * 32;
		return run_ai(player, area, name);
	}

	AiScript* run_default_ai(u32 player, Point16 pos) {
		u32 race = g_player_table[player].race;
		switch (race) {
			case RaceId::Terran:	return scbw::run_ai(player, pos, 'xCMT');
			case RaceId::Protoss:	return scbw::run_ai(player, pos, 'xCMP');
			case RaceId::Zerg:		return scbw::run_ai(player, pos, 'xCMZ');
		}
		return 0;
	}

	void give_ai_ex(Unit* unit) {
		if (!unit || unit->unit_ai || g_player_table[unit->player].type != PlayerType::Computer) {
			return;
		}
		if (units_dat::computer_ai_internal[unit->id] & 2) {
			helper::give_ai(unit);
		} else {
			if (unit->id == UnitId::TerranCleric && !g_aiscript_controller[unit->player].main_medic) {
				g_aiscript_controller[unit->player].main_medic = unit;
			}
			helper::add_military_ai(scbw::get_ai_region(unit->player, scbw::get_region_id(unit->position)), unit, true);
		}
	}

	u32 unit_get_creep_spreader_id(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (helper::spreads_creep(unit->id, unit->status & UnitStatus::Completed)) {
			return unit->id;
		}
		if (unit_ext->values[ValueBlissfulEmbraceOriginalPlayer] && !(unit->status & UnitStatus::InAir)) {
			return UnitId::ZergHachirosk;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::CreepBuilding) {
			return unit->id;
		}
		return UnitId::None;
	}

	bool sprite_id_is_corpse(u32 sprite_id) {
		switch (sprite_id) {
			case SpriteId::Corpse_Mason:
			case SpriteId::Corpse_Maverick:
			case SpriteId::Corpse_Harakan:
			case SpriteId::Corpse_Cyprian:
			case SpriteId::Corpse_Cleric:
			case SpriteId::Corpse_Shaman:
			case SpriteId::Corpse_Eidolon:
			case SpriteId::Corpse_Apostle:
			case SpriteId::Corpse_Vulture:
			case SpriteId::Corpse_Cyclops:
			case SpriteId::Corpse_Goliath:
			case SpriteId::Corpse_Blackjack:
			case SpriteId::Corpse_Wraith:
			case SpriteId::Corpse_Scribe:
			case SpriteId::Corpse_Zealot:
			case SpriteId::Corpse_Dracadin:
			case SpriteId::Corpse_Ecclesiast:
			case SpriteId::Corpse_Legionnaire:
			case SpriteId::Corpse_Larva:
			case SpriteId::Corpse_Egg:
			case SpriteId::Corpse_Droleth:
			case SpriteId::Corpse_Zethrokor:
			case SpriteId::Corpse_Quazilisk:
			case SpriteId::Corpse_Hydralisk:
			case SpriteId::Corpse_Izirokor:
			case SpriteId::Corpse_Zoryusthaleth:
			case SpriteId::Corpse_Bactalisk:
			case SpriteId::Corpse_Kagralisk:
			case SpriteId::Corpse_Evigrilisk:
			case SpriteId::Corpse_Lakizilisk:
			case SpriteId::Corpse_Konvilisk:
			case SpriteId::Corpse_Rilirokor:
			case SpriteId::Corpse_Ultrokor:
			case SpriteId::Corpse_Isthrathaleth:
			case SpriteId::Corpse_TerranBuildingLarge:
			case SpriteId::Corpse_TerranBuildingSmall:
			case SpriteId::Corpse_ProtossBuildingLarge:
			case SpriteId::Corpse_ProtossBuildingSmall:
			case SpriteId::Corpse_ZergBuildingLarge:
			case SpriteId::Corpse_ZergBuildingSmall:
			case SpriteId::Corpse_Aspirant:
			case SpriteId::Corpse_TerranUnitMedium:
			case SpriteId::Corpse_TerranUnitSmall:
			case SpriteId::Corpse_ProtossUnitMedium:
			case SpriteId::Corpse_ProtossUnitSmall:
			case SpriteId::Corpse_ZergUnitMedium:
			case SpriteId::Corpse_ZergUnitSmall:
			case SpriteId::Corpse_ZergUnitTiny:
			{
				return true;
			} break;
		}
		return false;
	}

	bool unit_queue_full(Unit* unit) {
		for (u32 i = 0; i < 5; ++i) {
			if (unit->build_queue[i] == UnitId::None) {
				return false;
			}
		}
		return true;
	}

	bool all_selected_units_have_addon(u32 unit_id) {
		bool has_selection = false;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (!unit) {
				break;
			}

			if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
				return false;
			}

			has_selection = true;
			if (!unit->building.addon || unit->building.addon->id != unit_id) {
				return false;
			}
		}
		return has_selection;
	}

	bool unit_is_stunned(Unit* unit) {
		return unit->status & UnitStatus::Frozen || unit->lockdown_timer || unit->stasis_timer || unit->maelstrom_timer;
	}

	bool unit_id_is_transport(u32 unit_id) {
		return unit_id == UnitId::TerranTrojan
			|| unit_id == UnitId::TerranPythean
			|| unit_id == UnitId::ProtossEnvoy
			|| unit_id == UnitId::ZergOvileth
			|| unit_id == UnitId::ZergIroleth
			|| unit_id == UnitId::ZergOthstoleth;
	}

	bool unit_id_race_check(u32 unit_id, u32 race) {
		switch (race) {
			case RaceId::Terran: {
				if (units_dat::group_flags[unit_id].is_terran) {
					return true;
				}
				switch (unit_id) {
					case UnitId::TerranCohort:
					case UnitId::TerranAspirant:
					case UnitId::TerranTarasque:
					case UnitId::TerranLuminary: {
						return true;
					}
				}
			} break;
			case RaceId::Protoss: {
				if (units_dat::group_flags[unit_id].is_protoss) {
					return true;
				}
			} break;
			case RaceId::Zerg: {
				if (units_dat::group_flags[unit_id].is_zerg) {
					return true;
				}
			} break;
		}
		return false;
	}

	void dump_sync_data() {
		FILE* file = fopen("desync.text", "wb");
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			fprintf(file, "Unit %s #%u", unit->get_name(), unit->get_unit_index() - 1);
			fprintf(file, "\tPlayer %u", unit->player);

			Sprite* sprite = unit->sprite;
			fprintf(file, "Sprite");
			fprintf(file, "\tSprite Id: %u", (u32)sprite->id);
			fprintf(file, "\tVisibility: %u", (u32)sprite->visibility_status);
		}
		fclose(file);
	}

	u32 get_zerg_regression(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueOriginatedFromOcsalLarva]) {
			// Ocsal Units
			return UnitId::None;
		}

		switch (unit->id) {
			// Structure Morphs
			case UnitId::ZergCaphrolosk:			return UnitId::ZergHachirosk;
			case UnitId::ZergGathtelosk:			return UnitId::ZergHachirosk;
			case UnitId::ZergEzigrisant:			return UnitId::ZergExcisant;
			case UnitId::ZergSurkithColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergSpraithColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergLarvoskColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergKazitrithColony:		return UnitId::ZergCagrantColony;
			case UnitId::ZergOcsalColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergSkortrithColony:		return UnitId::ZergSurkithColony;
			case UnitId::ZergSpirtithColony:		return UnitId::ZergSpraithColony;
			case UnitId::ZergOthstolOviform:		return UnitId::ZergIrolIris;
			case UnitId::ZergLiivralPond:			return UnitId::ZergQuazrokPool;
			case UnitId::ZergVithrathHaunt:			return UnitId::ZergMuthrokSpire;
			case UnitId::ZergGorgralSwamp:			return UnitId::ZergHydrokDen;
			case UnitId::ZergBegrozithBarrow:		return UnitId::ZergAxtothAxis;
			case UnitId::ZergAlkajChasm:			return UnitId::ZergGeszkathGrotto;
			case UnitId::ZergChokralgantChazsal:	return UnitId::ZergIzkagIteth;
			case UnitId::ZergUltrokCavern:			return UnitId::ZergZorkizShroud;
			case UnitId::ZergAlmakisAntre:			return UnitId::ZergMatravilNest;
			case UnitId::ZergRakzogthRoost:			return UnitId::ZergElthisthMound;

			// Base Structures
			case UnitId::ZergHachirosk:
			case UnitId::ZergCagrantColony:
			case UnitId::ZergExcisant:
			case UnitId::ZergIrolIris:				
			case UnitId::ZergNydusCesiant:			
			case UnitId::ZergQuazrokPool:			
			case UnitId::ZergMuthrokSpire:			
			case UnitId::ZergAxtothAxis:			
			case UnitId::ZergHydrokDen:				
			case UnitId::ZergIzkagIteth:			
			case UnitId::ZergGeszkathGrotto:		
			case UnitId::ZergZorkizShroud:			
			case UnitId::ZergMatravilNest:			
			case UnitId::ZergElthisthMound:			
				return UnitId::None; // TODO: Fix misc null sprite issues with regression into Droleth
			//	return unit->status & UnitStatus::Completed && !unit_ext->values[ValueOriginatedFromGosvileth] ? UnitId::ZergDroleth : UnitId::None;

			// Unit Mophs
			case UnitId::ZergVorvrokor:				return UnitId::ZergZethrokor;
			case UnitId::ZergLiiralisk:				return UnitId::ZergQuazilisk;
			case UnitId::ZergKalkalisk:				return UnitId::ZergNathrokor;
			case UnitId::ZergGhitorokor:			return UnitId::ZergCikralisk;
			case UnitId::ZergTosgrilisk:			return UnitId::ZergMutalisk;
			case UnitId::ZergBactalisk:				return UnitId::ZergHydralisk;
			case UnitId::ZergTethzorokor:			return UnitId::ZergIzirokor;
			case UnitId::ZergVithrilisk:			return UnitId::ZergSkithrokor;
			case UnitId::ZergCherbrathalor:			return UnitId::ZergKagralisk;
			case UnitId::ZergOkrimnilisk:			return UnitId::ZergEvigrilisk;
			case UnitId::ZergAlgoztalisk:			return UnitId::ZergAlmaksalisk;
			case UnitId::ZergKethrostalor:			return UnitId::ZergKeskathalor;
			case UnitId::ZergAlkajelisk:			return UnitId::ZergGeszithalor;
			case UnitId::ZergUltrokor:				return UnitId::ZergZoryusthaleth;
			case UnitId::ZergZobriolisk:			return UnitId::ZergLakizilisk;
			case UnitId::ZergZarcavrokor:			return UnitId::ZergGorgrokor;
			case UnitId::ZergTetcaerokor:			return UnitId::ZergKonvilisk;
			case UnitId::ZergSelkerilisk:			return UnitId::ZergVilgorokor;
			case UnitId::ZergAkistrokor:			return UnitId::ZergMatraleth;
		}
		return UnitId::None;
	}

	bool unit_is_allied_or_detected(Unit* unit, u32 player) {
		if (scbw::is_allied_to(unit->player, player)) {
			 return true;
		}
		if (helper::get_detecting_players(unit) & (1 << player)) {
			return true;
		}
		return false;
	}

	u32 get_ground_path_distance_tiles(Point16 a, Point16 b) {
		return helper::get_tile_distance(a.x, a.y, b.x, b.y);
	}

	bool order_id_is_aggressive(u32 order_id) {
		return order_id == OrderId::Attack1
			|| order_id == OrderId::Attack2
			|| order_id == OrderId::AttackFixedRange
			|| order_id == OrderId::AttackMove
			|| order_id == OrderId::AttackMoveComputer
			|| order_id == OrderId::AttackTile
			|| order_id == OrderId::AttackUnit
			|| order_id == OrderId::CarrierAttack1
			|| order_id == OrderId::CarrierAttack2
			|| order_id == OrderId::ReaverAttack1
			|| order_id == OrderId::ReaverAttack2
			|| order_id == OrderId::TowerAttack
			|| order_id == OrderId::TurretAttack;
	}

	bool ai_unit_id_can_flee_three_quarters(u32 unit_id) {
		// Just like default, for now... -Pr0nogo
		return unit_id == UnitId::ProtossDidact
			|| unit_id == UnitId::TerranWraith
			|| unit_id == UnitId::ZergMutalisk;
	}

	bool ai_unit_id_can_flee_two_thirds(u32 unit_id) {
		// Just like default, for now... -Pr0nogo
		return unit_id == UnitId::TerranMinotaur
			|| unit_id == UnitId::ProtossSolarion
			|| unit_id == UnitId::ProtossPanoptus;
	}

	bool unit_id_is_protoss_warp_factory(u32 unit_id) {
		return unit_id == UnitId::ProtossGateway
			|| unit_id == UnitId::ProtossCenotaph
			|| unit_id == UnitId::ProtossGrandLibrary
			|| unit_id == UnitId::ProtossRogueGallery
			|| unit_id == UnitId::ProtossProstrationStage
			|| unit_id == UnitId::ProtossSanctumOfSorrow
			|| unit_id == UnitId::ProtossAncestralArchives
			|| unit_id == UnitId::ProtossHallOfHeretics
			|| unit_id == UnitId::ProtossStargate
			|| unit_id == UnitId::ProtossAstralOmen
			|| unit_id == UnitId::ProtossArgosy
			|| unit_id == UnitId::ProtossFleetBeacon
			|| unit_id == UnitId::ProtossMonumentOfSin;
	}

	bool buttonset_id_is_submenu(u32 buttonset_id) {
		return buttonset_id == UnitId::Buttons_BasicZergBuildings
			|| buttonset_id == UnitId::Buttons_AdvancedZergBuildings
			|| buttonset_id == UnitId::Buttons_SupremeZergBuildings
			|| buttonset_id == UnitId::Buttons_BasicTerranBuildings
			|| buttonset_id == UnitId::Buttons_AdvancedTerranBuildings
			|| buttonset_id == UnitId::Buttons_SupremeTerranBuildings
			|| buttonset_id == UnitId::Buttons_BasicProtossBuildings
			|| buttonset_id == UnitId::Buttons_AdvancedProtossBuildings
			|| buttonset_id == UnitId::Buttons_RoboticProtossStructures
			|| buttonset_id == UnitId::Buttons_StellarProtossStructures
			|| buttonset_id == UnitId::Buttons_ProfaneProtossStructures
			|| buttonset_id == UnitId::Buttons_LarvaSwarm
			|| buttonset_id == UnitId::Buttons_LarvaAssault
			|| buttonset_id == UnitId::Buttons_LarvaControl
			|| buttonset_id == UnitId::Buttons_OcsalLarvaSwarm
			|| buttonset_id == UnitId::Buttons_OcsalLarvaAssault
			|| buttonset_id == UnitId::Buttons_OcsalLarvaControl
			|| buttonset_id == UnitId::Buttons_BasicPoliceStructures
			|| buttonset_id == UnitId::Buttons_AdvancedPoliceStructures;
	}

	bool unit_id_has_custom_air_weapon(u32 unit_id) {
		return unit_id == UnitId::TerranDurendal
			|| unit_id == UnitId::ProtossOptecton;
	}

	u32 get_custom_secondary_weapon_from_unit_id(u32 unit_id) {
		switch (unit_id) {
			case UnitId::TerranTarasque: {
				return WeaponId::TarasqueAcceleratedPlasmid;
			}
			case UnitId::TerranDurendal: {
				return WeaponId::DurendalPhilistineAutogun;
			}
			case UnitId::TerranCentaur: {
				return WeaponId::CentaurSunchaserMissiles;
			}
			case UnitId::TerranPegasus: {
				return WeaponId::PegasusLancetMissiles;
			}
			case UnitId::TerranOberon: {
				return WeaponId::OberonAcheronMissiles;
			}
			case UnitId::ProtossOptecton: {
				return WeaponId::OptectonSkybondRay;
			}
			case UnitId::ZergZarcavrokor: {
				return WeaponId::ZarcavrokorBonesofReflection;
			}
		}
		return WeaponId::None;
	}

	void set_bullet_target(Bullet* bullet, Point16 target) {
		bullet->attack_target.unit = 0;
		bullet->attack_target.pt   = target;
		bullet->move_target.unit   = 0;
		bullet->move_target.pt     = target;
		bullet->target_position    = target;
		bullet->target_position_2  = target;
	}

	void set_bullet_target(Bullet* bullet, Unit* target) {
		bullet->attack_target.unit = target;
		bullet->attack_target.pt   = target->position;
		bullet->move_target.unit   = target;
		bullet->move_target.pt     = target->position;
		bullet->target_position    = target->position;
		bullet->target_position_2  = target->position;
	}

	void set_bullet_target_follow(Bullet* bullet, Target target) {
		if (target.unit) {
			set_bullet_target(bullet, target.unit);
		} else {
			set_bullet_target(bullet, target.pt);
		}
	}

	void set_bullet_target_absolute(Bullet* bullet, Target target) {
		set_bullet_target(bullet, target.pt);
		bullet->attack_target.unit = target.unit;	
		bullet->move_target.unit = target.unit;	
	}

	void set_bullet_direction(Bullet* bullet, Point16 target) {
		u32 direction = helper::get_direction_from_points(bullet->position, target);
		bullet->current_direction_1 = direction;
		bullet->velocity_direction_1 = direction;
		for (Image* image = bullet->sprite->images.head; image; image = image->link.next) {
			helper::set_image_direction(image, direction);
		}
	}

	void set_bullet_direction(Bullet* bullet, Unit* target) {
		return set_bullet_direction(bullet, target->position);
	}

	bool move_unit_ai(Unit* unit, AiRegion* ai_region) {
		if (!unit->unit_ai) {
			return false;
		}
		if (unit->unit_ai->type == AiType::Military) {
			MilitaryAi* military_ai = &unit->unit_ai->military_ai;
			AiRegion* current_ai_region = military_ai->region;
			helper::delete_military_ai(military_ai, military_ai->region);
			unit->unit_ai = 0;
			helper::add_military_ai(ai_region, unit, 1);
			if (unit->status & UnitStatus::InBuilding) {
				helper::unload_unit_proc(unit);
			}
			helper::ai_recalculate_region_strength(current_ai_region);
			helper::ai_recalculate_region_strength(ai_region); // New
			return true;
		} else if (unit->unit_ai->type == AiType::Guard) {
			unit->unit_ai->guard_ai.parent = 0;
			unit->unit_ai = 0;
			helper::add_military_ai(ai_region, unit, 1);
			helper::ai_recalculate_region_strength(scbw::get_ai_region(unit->player, scbw::get_region_id(unit->position)));
			helper::ai_recalculate_region_strength(ai_region); // New
			return true;
		}
		return false;
	}

	bool unit_id_can_regenerate_shields(u32 unit_id) {
		return unit_id != UnitId::ProtossVagrantShade;
	}

	bool unit_id_is_stackable_building(u32 unit_id) {
		return false; // Disabled to see if it improves AI base layouts. -Pr0nogo
		switch (unit_id) {
			case UnitId::TerranWatchdog:
			case UnitId::TerranBunker:
			case UnitId::TerranSentinel:
			case UnitId::ProtossPylon:
			case UnitId::ProtossWarden:
			case UnitId::ProtossMatrix:
		//	case UnitId::ZergCagrantColony: // Disabled to prevent spamming. -Pr0nogo
			case UnitId::ZergSurkithColony:
			case UnitId::ZergSpraithColony:
			case UnitId::ZergLarvoskColony:
			case UnitId::ZergSkortrithColony:
			case UnitId::ZergSpirtithColony:
			case UnitId::ZergKazitrithColony:
			case UnitId::ZergOcsalColony:
			case UnitId::ZergIrolIris:
			case UnitId::ZergIrulentIvoa:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergOrbothOmlia:
			{
				return true;
			} break;
		}
		return false;
	}

	bool unit_id_has_addon(u32 unit_id) {
		switch (unit_id) {
			case UnitId::TerranCommandCenter:
			case UnitId::TerranTreasury:
			case UnitId::TerranBarracks:
			case UnitId::TerranCaptaincy:
			case UnitId::TerranIronCouncil:
			case UnitId::TerranFulcrum:
			case UnitId::TerranIronFoundry:
			case UnitId::TerranStarport:
			case UnitId::TerranNaniteAssembly:
			{
				return true;
			} break;
		}
		return false;
	}

	void astral_blessing_proc(Unit* victim, u32 attacking_player_id, u32 weapon_id) {
		u32 heal_amount = 2;
		u32 range = 64;
		switch (weapon_id) {
			case WeaponId::EcclesiastAstralBlaster: {
				heal_amount = 2;
				range = 64;
			} break;
			case WeaponId::PatriarchIronhandWeaver: {
				heal_amount = 5;
				range = 64;
			} break;
			case WeaponId::HeraldStellarTorch: {
				heal_amount = 1;
				range = 96;
			} break;
		}
		scbw::UnitFinder(
			victim->get_x() - range,
			victim->get_y() - range,
			victim->get_x() + range,
			victim->get_y() + range
		).for_each([&](Unit* unit) {
			if (attacking_player_id != unit->player && !scbw::is_allied_to(attacking_player_id, unit->player)) {
				// Ally check
				return;
			}
			if (units_dat::shields_enabled[unit->id] && unit->shields < units_dat::max_shield_points[unit->id] * 256) {
				// Shield check for default behavior
				helper::add_overlay_below_main(unit->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
				helper::add_overlay_below_main(victim->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
			} else {
				// Return if both checks failed
				return;
			}
			if (units_dat::shields_enabled[unit->id]) {
				// Shield heal
				unit->heal_shields(heal_amount * 256, victim);
			}
		});
	}

	bool is_attack_region(AiRegion* ai_region) {
		switch (ai_region->state) {
			case RegionState::ContinueAttackHere:
			case RegionState::Attacking:
			case RegionState::PreparingAttack:
			case RegionState::PreparingAttackOldUnreachable:
			{
				return true;
			}
		}
		return false;
	}

	u32 unit_id_get_larva_type(u32 unit_id) {
		switch (unit_id) {
			case UnitId::ZergCaphrolosk:
			case UnitId::ZergOcsalColony:
			case UnitId::ZergOcsalLarva:
			{
				return UnitId::ZergOcsalLarva;
			} break;
		}
		return UnitId::ZergLarva;
	}

	u32 unit_id_swarmhold_value(u32 unit_id) {
		switch (unit_id) {
			case UnitId::ZergGathtelosk: return 1; // 1 extra unit per egg
		}
		return 0;
	}

	void prepare_building_area(Point16 pos, u32 unit_id, Unit* builder) {
		Box32 rect = {};
		rect.x = pos.x - units_dat::building_dimensions[unit_id].x / 2;
		rect.y = pos.y - units_dat::building_dimensions[unit_id].y / 2;
		rect.w = pos.x + units_dat::building_dimensions[unit_id].x / 2;
		rect.h = pos.y + units_dat::building_dimensions[unit_id].y / 2;

		EntryArray units = {};
		unit_search_rect(&units, rect);
		for (u32 i = 0; i < units.count; ++i) {
			Unit* unit = units.units[i];
			if (unit == builder) {
				continue;
			}

			if (unit->id == UnitId::TerranPhalanxSiegeMode || unit->main_order_id == OrderId::SiegeMode) {
				unit->order_to(OrderId::TankMode);
			}

			if (unit->status & UnitStatus::Burrowed || unit->main_order_id == OrderId::Burrow) {
				helper::unburrow(unit);
			}

			if (unit->id == UnitId::TerranSpiderMine) {
				unit->remove();
			}
		}
	}

	void town_add_spending_request(AiTown* town, u32 count, u32 unit_id, u32 priority, bool request_base) {
		if (count < 0) return;

		if (request_base) {
			u32 base_unit_id = find_unit_base(unit_id);
			if (base_unit_id != UnitId::None) {
				u32 base_count = town_count_units(town, base_unit_id, false);
				if (base_count < count) {
					town_add_spending_request(town, count - base_count, base_unit_id, priority, request_base);
				}
			}
		}

		u32 request_type = RequestType::Building;
		if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
			request_type = RequestType::Worker;
		} else if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
			request_type = RequestType::Addon;
		}
		g_aiscript_controller[town->player].minerals_needed += units_dat::mineral_cost[unit_id] * count;
		g_aiscript_controller[town->player].vespene_needed += units_dat::vespene_cost[unit_id] * count;
		for (u32 i = 0; i < count; ++i) {
			helper::ai_add_spending_request(unit_id, town->player, request_type, priority, town);
		}
	}

	void military_add_spending_request(u32 player, u32 unit_id, Point16 position, u32 priority, bool request_base) {
		if (unit_id == UnitId::TerranPhalanxSiegeMode) {
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		if (request_base) {
			u32 base_unit_id = find_unit_base(unit_id);
			if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
				military_add_spending_request(player, unit_id, position, priority, request_base);
			}
		}
		helper::ai_add_spending_request(unit_id, player, RequestType::Military, priority, scbw::get_ai_region(player, scbw::get_region_id(position)));
	}

	void guard_add_spending_request(u32 player, GuardAi* guard_ai, u32 priority, bool request_base) {
		u32 unit_id = guard_ai->unit_id;
		if (unit_id == UnitId::TerranPhalanxSiegeMode) {
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		if (request_base) {
			u32 base_unit_id = find_unit_base(unit_id);
			if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
				military_add_spending_request(player, unit_id, guard_ai->current_home, priority, request_base); // Intentionally using military, since we don't want the base unit to take over the guard
			}
		}
		helper::ai_add_spending_request(unit_id, player, RequestType::Guard, priority, guard_ai);
	}

	// Based on the function at 0x00462670 -Veeq7
	GuardAi* create_guard_ai(u32 player, u32 unit_id, Point16 position) {
		GuardAiList* guard_list = &g_player_guard_list->player[player];
		GuardAi* guard_ai = guard_list->array->first_free_guard;
		if (guard_ai) {
			// Remove from free list
			guard_list->array->first_free_guard = guard_ai->link.prev;
			if (guard_ai->link.prev) {
				guard_ai->link.prev->link.next = 0;
			}

			// Zero init
			*guard_ai = {};

			// Add to active list
			guard_ai->link.prev = guard_list->first;
			if (guard_list->first) {
				guard_list->first->link.next = guard_ai;
			}
			guard_list->first = guard_ai;

			// Set vars
			guard_ai->unit_id = unit_id;
			guard_ai->type = 1;
			guard_ai->current_home = position;
			guard_ai->original_home = position;
		}
		return guard_ai;
	}

	bool weapon_is_melee_with_aoe(Bullet* bullet, u32 weapon_id) {
		if (bullet && bullet->source_unit) {
			switch (weapon_id) {
				case WeaponId::GolemBloodlessAssault:
				case WeaponId::NathrokorPlasmidBreath:
				case WeaponId::GorgrokorSerratedCleavers:
				case WeaponId::ZarcavrokorBisectingBlades:
				case WeaponId::TethzorokorSaberArray:
				case WeaponId::TetcaerokorCleavingMaw:
					return true;
			}
		}
		return false;
	}

	// Returns the special damage multiplier factor for units that don't use the "Damage Factor" property in weapons.dat.
	u8 get_damage_factor_for_tooltip(u8 weapon_id, Unit* unit)
	{
		switch (weapon_id)
		{
			case WeaponId::CohortGaussPistols:
			case WeaponId::HarakanFlamethrower:
			case WeaponId::HypnagogueKaleidoscopeCannon:
			case WeaponId::VultureFragmentationGrenade:
			case WeaponId::MatadorTwinstrikeFlamethrowers:
			case WeaponId::DurendalAbaddonFlamethrowers:
			case WeaponId::PazuzuJupiterDriver:
			case WeaponId::ZealotPhaseBlades:
			case WeaponId::LegionnairePhaseSpear:
			case WeaponId::AmaranthPsiCleaver:
			case WeaponId::GolemBloodlessAssault:
			case WeaponId::PariahIconoclastBlades:
			case WeaponId::StarSovereignCelestialCannon:
			case WeaponId::LiiraliskHollowedSpores:
			case WeaponId::CikraliskAcidSalvo:
			case WeaponId::MortothrokorWickedIncisors:
			case WeaponId::BactaliskCorrosiveAcid:
			case WeaponId::AlmaksaliskPiercingSpines:
			case WeaponId::KeskathalorScreamingVenom:
			{
				return 2;
			}
			case WeaponId::OptectonSkybondRay:
			{
				return 1;
			}
			default:
			{
				return weapons_dat::damage_factor[weapon_id];
			}
		}
	}

	void delete_ai_script(AiScript* ai_script) {
		if (*g_first_active_aiscript == ai_script) {
			*g_first_active_aiscript = ai_script->link.next;
		}

		if (ai_script->link.next) {
			ai_script->link.prev->link.prev = ai_script->link.prev;
		}
		if (ai_script->link.next) {
			ai_script->link.next->link.prev = ai_script->link.prev;
		}

		ai_script->link.prev = 0;
		ai_script->link.next = g_aiscript_array->first_free;
		if (g_aiscript_array->first_free) {
			g_aiscript_array->first_free->link.prev = ai_script;
		}
		g_aiscript_array->first_free = ai_script;
	}

	void delete_town(AiTown* ai_town) {
		helper::delete_aiscripts_using_town(ai_town);
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit->sprite && unit->main_order_id && units_dat::base_property[unit->id] & UnitProperty::ResourceContainer) {
				unit->building.resource.resource_group = 0;
			}
		}

		u8 resource_area = ai_town->resarea;
		if (resource_area) {
			g_base_array[resource_area].flags &= ~0x2;
		}

		s32 player = ai_town->player;
		if (g_ai_town_list[player].first == ai_town) {
			g_ai_town_list[player].first = ai_town->prev;
		}

		if (ai_town->next) {
			ai_town->next->prev = ai_town->prev;
		}
		if (ai_town->prev) {
			ai_town->prev->next = ai_town->next;
		}

		ai_town->next = 0;
		ai_town->prev = g_ai_town_list[player].array->first_free;
		if (g_ai_town_list[player].array->first_free) {
			g_ai_town_list[player].array->first_free->next = ai_town;
		}
		g_ai_town_list[player].array->first_free = ai_town;
	}

	void delete_fow_sprite(Thingy* thingy) {
		helper::remove_selection_circle(thingy->sprite);
		helper::sprite_destructor(thingy->sprite);
		thingy->sprite = 0;
		if (*g_first_fow_sprite == thingy) {
			*g_first_fow_sprite = thingy->next;
		}
		if (*g_last_fow_sprite == thingy ) {
			*g_last_fow_sprite = thingy->prev;
		}
		if (thingy->prev) {
			thingy->prev->next = thingy->next;
		}
		if (thingy->next) {
			thingy->next->prev = thingy->prev;
		}
		thingy->prev = 0;
		thingy->next = 0;

		Thingy* first_free = *g_first_free_fow_sprite;
		if (first_free) {
			if (first_free == *g_first_free_fow_sprite) {
				*g_last_free_fow_sprite = thingy;
			}
			thingy->prev = first_free;
			thingy->next = first_free->next;
			if (first_free->next) {
				first_free->next->prev = thingy;
			}
			first_free->next = thingy;
		} else {
			*g_first_free_fow_sprite = thingy;
			*g_last_free_fow_sprite = thingy;
		}
	}

	bool unit_id_is_broodspawn(u32 unit_id) {
		return unit_id == UnitId::ZergRilirokor
			|| unit_id == UnitId::ZergRililisk;
	}

	void binding_plasma_proc(Unit* victim) {
		UnitExt* victim_ext = get_unit_ext(victim);
		u32 player = victim_ext->values[ValueGladiusBindingPlasmaPlayer];

		EntryArray targets = {};
		unit_search_radial(&targets, victim->position, 3 * 32);
		unit_search_filter(&targets, victim, FilterNotSelf | FilterNotSubunit | FilterVisible);
		for (u32 i = 0; i < targets.count; ++i) {
			Unit* target = targets.units[i];
			if (scbw::is_allied_to(player, target->player)) {
				entry_array_remove(&targets, i--);
			}
		}
		unit_search_sort_by_distance(&targets, victim->position);

		if (!targets.count) {
			victim_ext->timers[TimerGladiusBindingPlasma] = 0;
			victim_ext->values[ValueGladiusBindingPlasmaStacks] = 0;
			return;
		}

		u32 target_index = 0;
		while (victim_ext->values[ValueGladiusBindingPlasmaStacks]) {
			Unit* target = targets.units[target_index];
			target_index = (target_index + 1) % targets.count;

			u32 angle = scbw::get_angle(victim->position.x, victim->position.y, target->position.x, target->position.y);
			Bullet* bullet = helper::create_bullet(WeaponId::GladiusPlasmaFlurry, victim, victim->position.x, victim->position.y, player, angle);
			if (!bullet) {
				break; 
			}

			scbw::set_bullet_target(bullet, target);
			victim_ext->values[ValueGladiusBindingPlasmaStacks]--;
		}

		if (!victim_ext->values[ValueGladiusBindingPlasmaStacks]) {
			victim_ext->timers[TimerGladiusBindingPlasma] = 0;
		}
	}

	bool order_id_is_harvest_related(u32 order_id) {
		return order_id == OrderId::Harvest1
			|| order_id == OrderId::Harvest5
			|| order_id == OrderId::MoveToMinerals
			|| order_id == OrderId::HarvestGas1
			|| order_id == OrderId::HarvestGas3
			|| order_id == OrderId::HarvestFinish
			|| order_id == OrderId::ReturnGas
			|| order_id == OrderId::ReturnMinerals
			|| order_id == OrderId::MiningMinerals
			|| order_id == OrderId::CanHarvestOre
			|| order_id == OrderId::CanEnterGasMine
			|| order_id == OrderId::ResetCollision2
			|| order_id == OrderId::MoveToHarvestObscured;
	}

	bool player_has_transport(u32 player) {
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (scbw::unit_id_is_transport(unit->id)) {
				return true;
			}
		}
		return false;
	}

	bool unit_in_combat_within_seconds(Unit* unit, u32 seconds) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->damage_dealt_timer >= seconds * 24
		&&	unit_ext->damage_taken_timer >= seconds * 24) {
			return false;
		}
		return true;
	}

	void update_on_damage_dealt(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->damage_dealt_timer = 0;
		switch (unit->id) {
			case UnitId::ZergMortothrokor: {
				if (unit_ext->timers[TimerMortothrokorPassive]) {
					unit_ext->timers[TimerMortothrokorPassive] = 2 * 24;
				}
			} break;
		}
	}

	void update_on_damage_receive(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->shield_regen_timer = 0;
		unit_ext->damage_taken_timer = 0;
		switch (unit->id) {
			case UnitId::ZergMortothrokor: {
				if (unit_ext->timers[TimerMortothrokorPassive]) {
					unit_ext->timers[TimerMortothrokorPassive] = 2 * 24;
				}
			} break;
		}
	}

	bool unit_id_is_vespene_node(u32 unit_id) {
		return unit_id == UnitId::ResourceVespeneRidge
			|| unit_id == UnitId::ResourceVespeneGeyser
			|| unit_id == UnitId::TerranReservoir
			|| unit_id == UnitId::TerranWellbore
			|| unit_id == UnitId::ProtossAquifer
			|| unit_id == UnitId::ZergExcisant
			|| unit_id == UnitId::ZergEzigrisant;
	}

	bool unit_id_is_ground_bunker(u32 unit_id) {
		return unit_id == UnitId::TerranBunker;
		//	|| unit_id == UnitId::TerranTencendur;
	}

	bool unit_id_is_bunker(u32 unit_id) {
		return unit_id_is_ground_bunker(unit_id); // Will add more bunker types later. -Pr0nogo
	}

	bool is_team_game_map(const char* map_name) {
		return strcmp(map_name, "Todas Complex") == 0; // This was for CB2023, should be reworked into a custom trigger. -Pr0nogo
	}

	bool image_id_is_powerup(u32 image_id)
	{
		return image_id == ImageId::MineralChunkType1
			|| image_id == ImageId::MineralChunkType2
			|| image_id == ImageId::GasRaw
			|| image_id == ImageId::GasRaw2
			|| image_id == ImageId::TerranGasTankType1
			|| image_id == ImageId::TerranGasTankType2
			|| image_id == ImageId::ProtossGasOrbType1
			|| image_id == ImageId::ProtossGasOrbType2
			|| image_id == ImageId::ZergGasSacType1
			|| image_id == ImageId::ZergGasSacType2
			|| image_id == ImageId::Flag
			|| image_id == ImageId::KhaydarinCrystal
			|| image_id == ImageId::PsiEmitter
			|| image_id == ImageId::YoungChrysalis
			|| image_id == ImageId::DataDisc
			|| image_id == ImageId::Uraj
			|| image_id == ImageId::Khalis;
	}

	Image* get_powerup_overlay(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			if (scbw::image_id_is_powerup(image->id)) {
				return image;
			}
		}
		return 0;
	}

	bool weapon_id_accelerates(u32 weapon_id) {
		return weapon_id == WeaponId::EmpyreanPlasmaBarrage;
	}

	bool weapon_id_is_melee_aoe(u32 weapon_id)
	{
		return weapon_id == WeaponId::NathrokorPlasmidBreath
			|| weapon_id == WeaponId::GorgrokorSerratedCleavers
			|| weapon_id == WeaponId::ZarcavrokorBisectingBlades
			|| weapon_id == WeaponId::TethzorokorSaberArray
			|| weapon_id == WeaponId::TetcaerokorCleavingMaw;
	}

	bool unit_regens_in_combat(Unit* unit)
	{
		return false; // Can update later with status checks and etc. -Pr0nogo
	}

	bool unit_id_is_melee_air_unit(u32 unit_id) {
		return unit_id == UnitId::ZergNathrokor
			|| unit_id == UnitId::ZergGorgrokor
			|| unit_id == UnitId::ZergTetcaerokor
			|| unit_id == UnitId::ZergAkistrokor
			|| unit_id == UnitId::ZergZarcavrokor;
	}

	bool weapon_id_is_for_harvesting(u32 weapon_id) {
		return	weapon_id == WeaponId::MasonFusionCutterHarvest
			|| weapon_id == WeaponId::ScribeParticleBeamHarvest
			|| weapon_id == WeaponId::DrolethSpinesHarvest;
	}

	bool weapon_id_is_standard_weapon(u32 weapon_id) {
		// Use this switch for weird exceptions:
		/*
		switch (weapon_id) {
			case WeaponId::XD:
				return true;

			case WeaponId::XD2:
				return false;
		}
		*/
		switch (weapons_dat::explosion_type[weapon_id]) {
			case WeaponEffect::NormalHit:
			case WeaponEffect::SplashRadial:
			case WeaponEffect::SplashEnemy:
			case WeaponEffect::SplashAir:
			case WeaponEffect::Unknown_Crash:
				return true;
		}
		return false;
	}

	bool unit_id_is_morphed_from_double_unit(u32 unit_id) {
		return unit_id == UnitId::ZergVorvrokor
			|| unit_id == UnitId::ZergKalkalisk
			|| unit_id == UnitId::ZergGhitorokor;
	}

	Image* get_addon_connector_image(Unit* unit) {
		u16 image_id = 0;
		switch (unit->id) {
			case UnitId::TerranMedbay:
			{
				image_id = ImageId::MedbayConnector;
			} break;
			case UnitId::TerranCovertOps:
			{
				image_id = ImageId::CovertOpsConnector;
			} break;
			case UnitId::TerranScrapyard:
			case UnitId::TerranMachineShop:
			case UnitId::TerranMunitionsBay:
			{
				image_id = ImageId::MachineShopConnector;
			} break;
			case UnitId::TerranCommandment:
			{
				image_id = ImageId::CommandmentConnector;
			} break;
		}
		if (image_id) {
			return unit->get_overlay(image_id);
		}
		return 0;
	}

	bool can_consume_corpse(Thingy* corpse) {
		if (!scbw::sprite_id_is_corpse(corpse->sprite->id) || corpse->sprite->flags & SpriteFlags::RemoveAfterDeath) {
			return false;
		}

		ThingyExt* corpse_ext = get_thingy_ext(corpse);
		u32 unit_id = corpse_ext->unit_id;
		if (units_dat::base_property[unit_id] & UnitProperty::Building
		||	unit_id == UnitId::ZergLarva
		||  unit_id == UnitId::ZergOcsalLarva
		||  unit_id == UnitId::ZergEgg
		||  unit_id == UnitId::ZergCocoon
		||  unit_id == UnitId::ProtossPlasmaShell
		||  unit_id == UnitId::ProtossSteward) {
			return false;
		}

		return true;
	}

	void consume_corpse(Thingy* corpse) {
		corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
		corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
	}

	Unit* reclaim_corpse(Thingy* corpse, u32 player) {
		if (!can_consume_corpse(corpse)) {
			return false;
		}

		ThingyExt* corpse_ext = get_thingy_ext(corpse);
		u32 unit_id = corpse_ext->unit_id;
		Point16 pos = corpse->sprite->position;
		if (units_dat::base_property[unit_id] & UnitProperty::Flyer) {
			if (pos.y > 42) {
				pos.y -= 42;
			} else {
				pos.y = 0;
			}
		}
		Unit* unit = scbw::create_unit_in_game(unit_id, player, pos.x, pos.y);
		if (unit) {
			UnitExt* unit_ext = get_unit_ext(unit);
			unit->status |= UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
			unit->play_iscript_animation(IscriptAnimation::Unused1);
			unit_ext->values[ValueReviveStarted] = 1;
			unit_ext->timers[TimerReviveFlagClearMin] = 14; // 3;
			unit_ext->timers[TimerReviveFlagClearMax] = 14;
			switch (unit->id) {
				case UnitId::ZergRililisk:
				case UnitId::ZergRilirokor:
				{
					unit->remove_timer = BroodspawnTimer;
				}
			}
			scbw::consume_corpse(corpse);

			unit->shields = 0;
			unit->set_hitpoints(unit->get_max_hp_raw() / 2);
			unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
			helper::play_sound_from_unit(SoundId::LazarusAgentProc, unit);
			unit_ext->values[ValueNoCorpse] = true;
		}
		return unit;
	}

	bool unit_id_is_power_generator(u32 unit_id) {
		return unit_id == UnitId::ProtossPylon
			|| unit_id == UnitId::ProtossCrucible;
	}

	PlayerColorCode get_player_color_code(u32 player) {
		PlayerColorCode code = {};
		code.data[0] = PlayerColorByte;
		code.data[1] = player + 1;
		return code;
	}

	bool unit_id_is_psi_provider(u16 unit_id) {
		if (unit_id == UnitId::ProtossPylon
			|| unit_id == UnitId::ProtossCrucible) {
			return true;
		}
		return false;
	}

	Unit* unit_index_get(UnitIndex unit_index, bool allow_non_standard_units) {
		if (!unit_index.index) {
			return 0;
		}

		Unit* unit = &g_unit_table[unit_index.index - 1];
		if (unit->unique_id != unit_index.unique_id) {
			return 0;
		}
		if (!unit->sprite || unit->sprite->flags & SpriteFlags::Hidden) {
			return 0;
		}
		if (unit->main_order_id == OrderId::Die && unit->main_order_state == 1) {
			return 0;
		}
		if (!allow_non_standard_units && !helper::unit_is_standard_and_movable(unit)) {
			return 0;
		}

		return unit;
	}

	void move_camera(Point16 pos) {
		if (pos.x >= 320) {
			pos.x -= 320;
		} else {
			pos.x = 0;
		}
		if (pos.y >= 200) {
			pos.y -= 200;
		} else {
			pos.y = 0;
		}
		helper::move_screen(pos);
	}

	bool unit_id_can_do_controlled_demolition(u32 unit_id) {
		return units_dat::base_property[unit_id] & UnitProperty::Addon
			|| unit_id == UnitId::TerranReservoir
			|| unit_id == UnitId::TerranWellbore;
	}

	bool tech_id_needs_target(u32 tech_id) {
		return tech_id != TechId::Hallucination
			&& tech_id != TechId::LuminaryAuralAssault
			&& tech_id != TechId::HypnagogueThroughTheLookingGlass;
	}

	void set_custom_egg(Unit* egg, u32 unit_id, u32 time, u32 hp) {
		egg->order_to_idle();
		egg->sprite->play_iscript_animation(IscriptAnimation::Walking, true);
		egg->build_queue[egg->build_queue_slot] = unit_id;
		egg->set_hitpoints(hp);

		UnitExt* egg_ext = get_unit_ext(egg);
		egg_ext->values[ValueCustomEggActive]   = true;
		egg_ext->timers[TimerCustomEggHatch]    = time;
		egg_ext->values[ValueCustomEggHatchMax] = time;
		egg_ext->values[ValueCustomEggHpMax]    = hp;
	}

	Quad quad_from_line(Point16 from, Point16 to, f32 line_width) {
		Vector32f line_diff = get_normalized_vector(get_vector_between(from, to));
		Vector32f p = {-line_diff.y * line_width / 2, line_diff.x * line_width / 2};
		Quad quad = {};
		quad.corners[0].x = from.x - p.x;
		quad.corners[0].y = from.y - p.y;
		quad.corners[1].x = to.x - p.x;
		quad.corners[1].y = to.y - p.y;
		quad.corners[2].x = to.x + p.x;
		quad.corners[2].y = to.y + p.y;
		quad.corners[3].x = from.x + p.x;
		quad.corners[3].y = from.y + p.y;
		return quad;
	}

	Quad quad_from_rect(Box16 rect) {
		Quad box_quad = {};
		box_quad.top_left.x = rect.left;
		box_quad.top_left.y = rect.top;
		box_quad.top_right.x = rect.right;
		box_quad.top_right.y = rect.top;
		box_quad.bottom_right.x = rect.right;
		box_quad.bottom_right.y = rect.bottom;
		box_quad.bottom_left.x = rect.left;
		box_quad.bottom_left.y = rect.bottom;
		return box_quad;
	}

	static bool sat_collision_internal(Vector32f* a, u32 a_count, Vector32f* b, u32 b_count) {
		for (u32 i = 0; i < a_count; ++i) {
			Vector32f diff = scbw::get_vectorf_between(a[i], a[(i + 1) % a_count]);
			Vector32f proj = {-diff.y, diff.x};

			f32 a_min = INFINITY;
			f32 a_max = -INFINITY;
			for (u32 j = 0; j < a_count; ++j) {
				f32 dot = scbw::dot_product(a[j], proj);
				if (dot < a_min) a_min = dot;
				if (dot > a_max) a_max = dot;
			}

			f32 b_min = INFINITY;
			f32 b_max = -INFINITY;
			for (u32 j = 0; j < b_count; ++j) {
				f32 dot = scbw::dot_product(b[j], proj);
				if (dot < b_min) b_min = dot;
				if (dot > b_max) b_max = dot;
			}

			if (a_max < b_min || b_max < a_min) {
				return false;
			}
		}
		return true;
	}
	
	bool sat_collision(Vector32f* a, u32 a_count, Vector32f* b, u32 b_count) {
		return sat_collision_internal(a, a_count, b, b_count) && sat_collision_internal(b, b_count, a, a_count);
	}

	f32 line_to_point_distance(Vector32f from, Vector32f to, Vector32f point, Vector32f* out_cp) {
		Vector32f ab = scbw::get_vectorf_between(from, to);
		Vector32f ap = scbw::get_vectorf_between(from, point);

		f32 dot = scbw::dot_product(ab, ap);
		dot /= scbw::dot_product(ab, ab);
		dot = Clamp(dot, 0, 1);

		Vector32f cp = from;
		cp.x += ab.x * dot;
		cp.y += ab.y * dot;
		if (out_cp) {
			*out_cp = cp;
		}

		return scbw::get_distance_between(cp, point);
	}

	static void line_intersects_rect_internal(Quad quad_a, Quad quad_b, Vector32f axis, f32* best_distance, Vector32f* best_cp, f32* best_t) {
		for (u32 i = 0; i < 4; ++i) {
			Vector32f line0 = quad_a.corners[i];
			Vector32f line1 = quad_a.corners[(i + 1) % 4];
			for (u32 j = 0; j < 4; ++j) {
				Vector32f point = quad_b.corners[j];
				Vector32f cp = {};
				f32 distance = line_to_point_distance(line0, line1, point, &cp);
				f32 t = scbw::dot_product(cp, axis);
				if (abs(distance - *best_distance) < 1) {
					if (t < *best_t) {
						*best_distance = distance;
						*best_cp = cp;
						*best_t = t;
					}
				} else if (distance < *best_distance) {
					*best_distance = distance;
					*best_cp = cp;
					*best_t = t;
				}
			}
		}
	}

	bool line_intersects_rect(Point16 from, Point16 to, f32 line_width, Box16 rect, f32* out_t, Point16* out_cp) {
		Quad line_quad = quad_from_line(from, to, line_width);
		Quad box_quad = quad_from_rect(rect);
		if (sat_collision(line_quad.corners, 4, box_quad.corners, 4)) {
			if (!out_t && !out_cp) {
				return true;
			}

			Vector32f axis = scbw::get_vectorf_between(from, to);
			f32 best_distance = INFINITY;
			Vector32f best_cp = {};
			best_cp.x = rect.left   + (rect.right - rect.left)   / 2;
			best_cp.y = rect.bottom + (rect.top   - rect.bottom) / 2;
			f32 best_t = INFINITY;

			line_intersects_rect_internal(line_quad, box_quad, axis, &best_distance, &best_cp, &best_t);
			line_intersects_rect_internal(box_quad, line_quad, axis, &best_distance, &best_cp, &best_t);

			if (out_cp) {
				out_cp->x = best_cp.x;
				out_cp->y = best_cp.y;
			}
			if (out_t) {
				*out_t = scbw::dot_product(best_cp, axis);
			}

			return true;
		}
		return false;
	}

	bool unit_paths_over_small_units(Unit* unit)
	{
		// Keeping arg as unit in case we want to add status effects later. -Pr0nogo
		return unit->id == UnitId::TerranPenumbra;
	}

	bool unit_collides_with_titanic(Unit* unit)
	{
		if (unit->status & UnitStatus::GroundedBuilding) {
			return unit->id != UnitId::ProtossPylon && !scbw::unit_id_is_minerals(unit->id);
		}
		return unit->id == UnitId::TerranClaymore
			|| unit->id == UnitId::TerranPenumbra
			|| unit->id == UnitId::TerranAion
			|| unit->id == UnitId::ProtossOptecton
			|| unit->id == UnitId::ProtossDemiurge
			|| unit->id == UnitId::ZergKeskathalor
			|| unit->id == UnitId::ZergKethrostalor
			|| unit->id == UnitId::ZergUltrokor;
	}

	bool unit_has_fore_castle(Unit* unit)
	{
		switch (unit->id)
		{
			case UnitId::TerranMinotaur:
			{
				return true;
			}
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueHasSublimeShepherd]) {
			return true;
		}
		return false;
	}

	bool unit_id_is_siege_mode(u32 unit_id)
	{
		return unit_id == UnitId::TerranMatadorCinderMode
			|| unit_id == UnitId::TerranPhalanxSiegeMode
			|| unit_id == UnitId::TerramCuirassTrenchMode;
	}

	bool unit_id_is_tank_mode(u32 unit_id)
	{
		return unit_id == UnitId::TerranMatadorTankMode
			|| unit_id == UnitId::TerranPhalanxTankMode
			|| unit_id == UnitId::TerranCuirassTankMode;
	}

	void transform_unit(Unit* unit, u16 unit_id) // Written by DarkenedFantasies for siege transformation.
	{
		Unit* subunit = unit->subunit;
		s8 subunit_hoffset = 0;
		s8 subunit_voffset = 0;
		Image* main_graphic = 0;
		if (subunit) {
			main_graphic = subunit->sprite->main_graphic;
			subunit_hoffset = main_graphic->horizontal_offset;
			subunit_voffset = main_graphic->vertical_offset;
		} else {
			subunit_voffset = subunit_hoffset = -1;
		}

		helper::replace_unit_with_id(unit, unit_id);
		u8 direction = unit->current_direction_1 = unit->velocity_direction_1 = unit->current_direction_2;
		helper::set_image_direction(unit->sprite->main_graphic, direction);
		subunit = unit->subunit;

		if (subunit) {
			direction = subunit->current_direction_2;
			if (subunit->id == UnitId::TerranPhalanxTankModeTurret || subunit->id == UnitId::TerranPhalanxSiegeModeTurret)
			{
				direction += 128; // Flip facing direction for the Phalanx turret. -DF
				subunit->current_direction_2 = direction;
			}
			subunit->current_direction_1 = subunit->velocity_direction_1 = subunit->velocity_direction_2 = direction;
			main_graphic = subunit->sprite->main_graphic;
			main_graphic->horizontal_offset = subunit_hoffset;
			main_graphic->vertical_offset = subunit_voffset;
			helper::set_image_direction(main_graphic, direction);
		}
	}
}