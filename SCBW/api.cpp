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

	bool teleport(Unit* u, Point16 target, u16 sprite_id, bool preserveOrder) {
		Point16 previous_pos = u->sprite->position;
		Point16 new_pos = {};
		bool result = false;

		// Identical code to set_unit_position() but for some reason, is using another function to do exactly the same thing
		helper::move_unit_duplicate(u, target.x, target.y);

		if (!scbw::check_unit_collision_pos(u, &(u->sprite->position), &new_pos, nullptr, false, 0)) {
			// Collision detected, cancel the movement
			scbw::set_unit_position(u, previous_pos.x, previous_pos.y);
		} else {
			result = true;

			// Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepare_unit_move(u, false);
			if (!preserveOrder) {
				u->order_target.unit = 0;
			}
			scbw::set_unit_position(u, new_pos.x, new_pos.y);
			scbw::refresh_unit_after_move(u);

			// Stop some currently active orders
			if (u->id != UnitId::ZergCocoon && !preserveOrder) {
				u->order_computer(units_dat::return_to_idle_order[u->id]);
			}

			// Create the Recall animation over the unit
			if (sprite_id) {
				Thingy* recall_effect_sprite = helper::create_thingy(sprite_id, new_pos.x, new_pos.y, 0);
				if (recall_effect_sprite) {
					recall_effect_sprite->sprite->elevation = u->sprite->elevation + 1;
					helper::set_thingy_visibility_flags(recall_effect_sprite);
				}

			}

			// Disconnect a nuke from a unit using it
			if (scbw::unit_can_nuke(u)) {
				if (u->connected_unit && u->connected_unit->id == UnitId::TerranNuclearMissile) {
					u->connected_unit->connected_unit = 0;
					u->connected_unit = 0;
				}
			}
		}
		return result;
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
		if (weapon_id >= WEAPON_TYPE_COUNT) {
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
		if (target_flags.nonBuilding && targetProps & UnitProperty::Building) {
			return false;
		}
		if (target_flags.nonRobotic && targetProps & UnitProperty::RoboticUnit) {
			return false;
		}
		if (target_flags.orgOrMech && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical))) {
			return false;
		}
		if (target_flags.playerOwned && target->player != attacker->player) {
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

		s32 supplyProvided;
		if (is_cheat_enabled(CheatFlags::FoodForThought)) {
			supplyProvided = g_race_supply[RaceId::Terran].max[player];
		} else {
			supplyProvided = std::min(g_race_supply[RaceId::Terran].max[player], g_race_supply[RaceId::Terran].provided[player]);
		}
		return supplyProvided - g_race_supply[RaceId::Terran].used[player];
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
			case UnitId::ZergSibraelosk:
			case UnitId::ZergGathtelosk:
			case UnitId::ZergCaphrolosk:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
			case UnitId::ZergLarvoskColony:
			case UnitId::ZergSpraithColony:
			case UnitId::ZergSurkithColony:
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
			case UnitId::ZergGathtelosk:
			case UnitId::ZergCaphrolosk:
			case UnitId::ZergHachirosk:
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergCagrantColony:
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
		const char* data = nullptr;
		u32 len = 0;
		auto data_ptr = &data;
		auto len_ptr = &len;
		u32 val = reinterpret_cast<u32>(name);
		auto length = scbw::get_aise_value(nullptr, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
		char* buf = (char*)scbw::get_aise_value(nullptr, nullptr, AiseId::ReadSamaseFile_Ptr, 0, 0);
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
			ActiveTile originalTile = scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y);
			ActiveTile finalTile = scbw::get_stupid_reference_to_active_tile_at(x, y);

			if (finalTile.is_unwalkable || originalTile.ground_height != finalTile.ground_height) {
				return false;
			}

			Unit* collision = scbw::UnitFinder(
				unit->get_left(),
				unit->get_top(),
				unit->get_right(),
				unit->get_bottom()
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
		if (*g_unit_count >= 1700) {
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
			memset(&refreshRegions[40 * y + left], 1, right - left + 1);
		}
	}

	void refresh_screen() {
		memset(refreshRegions, 1, 1200);
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
		*force_button_refresh      = 1;
		*portrait_redraw           = 1;
		*status_screen_redraw      = 1;
		*control_under_mouse       = 0;
		*control_under_mouse_value = 0;
	}

	void create_unit_from_ability(s32 unit_id, s32 count, u16 player, Point16 position, Unit* creator) {
		for (s32 i = 0; i < count; i++) {
			u32 remove_timer = 0;
			switch (unit_id) {
				case UnitId::ZergRililisk:
				case UnitId::ZergRilirokor:
				{
					remove_timer = 720;
				}
			}
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
					u32 range = 6 * 32;
					scbw::UnitFinder(
						unit->get_x() - range,
						unit->get_y() - range,
						unit->get_x() + range,
						unit->get_y() + range
					).for_each([&](Unit* target) {
						if (target->id != UnitId::ZergMatraleth) {
							return;
						}
						if (!scbw::is_allied_to(unit->player, target->player)) {
							return;
						}
						if (target->energy < 200 * 256) {
							target->energy = std::min((u16)(target->energy + (u16)5 * (u16)256), target->get_max_energy());
							target->sprite->create_underlay(425);
						}
					});
					break;
				}
				case UnitId::ProtossSimulacrum: {
					if (creator) {
						unit->hitpoints = std::max(256, creator->hitpoints);
						unit->shields = std::max(256, creator->shields);
						scbw::inherit_debuffs(creator, unit);
					}
					helper::play_sound_from_unit(SoundId::Simulacrum_Clone_01, unit, 1, 0);
				}
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
		&&		 unit->id != UnitId::ProtossPlasmaShell
		&&	   (!weapon || weapons_dat::max_range[weapon] > 32)
		&&		 unit->id != UnitId::TerranHarakan;
	}

	s32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon) {
		return 0;
		if (!can_use_cliff_advantage(unit, weapon)) {
			return 0;
		}
		s32 source_height = scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y).ground_height / 2;
		if (!target_unit) {
			return source_height * 2;
		}
		s32 target_height = scbw::get_stupid_reference_to_active_tile_at(target_unit->position.x, target_unit->position.y).ground_height / 2;
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

	s32 get_overlay_offset(u16 unit_id, bool xlargeAllowed) {
		if (xlargeAllowed && scbw::unit_id_uses_xlarge_overlays(unit_id)) {
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
	bool unit_id_is_pacifist(Unit* unit) {
		return !unit->has_weapon()
		&&		unit->id != UnitId::ZergLakizilisk; // LAME
	}

	// Identical to function order_is_spell @ 00436930 -- Pr0nogo
	// We should update the lardcode for Cosmonarchy
	bool order_is_spell(s32 order_id) {
		return order_id == OrderId::Plague
			|| order_id == OrderId::DarkSwarm
//			|| order_id == OrderId::Ensnare				// Land Grab
			|| order_id == OrderId::Irradiate
			|| order_id == OrderId::StasisField
			|| order_id == OrderId::EmpShockwave		// Observance
			|| order_id == OrderId::MagnaPulse			// Lobotomy Mine
			|| order_id == OrderId::PsiStorm
//			|| order_id == OrderId::SummonBroodlings	// unused
			|| order_id == OrderId::FireYamatoGun1;
	}

	// Creates the Plasma Shield flickering effect.
	// Identical to function @ 0x004E6140
	void create_shield_overlay(Unit* unit, u32 attackDirection) {
		const LO_Header* shield_lo = g_shield_overlays[unit->sprite->main_graphic->id];
		u32 frameAngle = (attackDirection - 124) / 8 % 32;

		Point8 offset = shield_lo->get_offset(unit->sprite->main_graphic->direction, frameAngle);
		unit->sprite->create_overlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);
	}

	void decloak_proc(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->status & UnitStatus::Burrowed) {
			unit_ext->timers[TimerDecloakBurrow] = 48;
		} else if (is_decloakable(unit)) {
			if (unit_ext->timers[TimerDecloak] == 0) {
				unit->detection_status = 0xff;
				helper::anim_decloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);
			}
			if (unit_ext->timers[TimerDecloak] != 1) {
				unit_ext->timers[TimerDecloak] = 48 + 1;
			}
		}
		if (unit->is_subunit() && unit->subunit) {
			decloak_proc(unit->subunit);
		}
	}

	bool is_decloakable(Unit* unit) {
		return units_dat::base_property[unit->id] & UnitProperty::PermanentCloak || unit->status & UnitStatus::CloakingForFree;
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
			case UnitId::ProtossAccantorOld:
			case UnitId::ProtossPlasmaShell:
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
			|| id == UnitId::TerranHypnagogue
			|| id == UnitId::TerranPaladin
			|| id == UnitId::TerranCuirass
			|| id == UnitId::TerranPazuzu
			|| id == UnitId::TerranCentaur
			|| id == UnitId::TerranMinotaur
			|| id == UnitId::TerranMagnetar;
		//	|| id == UnitId::TerranAmbassador
		//	|| id == UnitId::TerranHeir
		//	|| id == UnitId::TerranHorseman
	}

	void apply_hallucinating(Unit* unit, u8 player, u32 time) {
		UnitExt* unitExt = get_unit_ext(unit);

		Unit* context = unit->subunit ? unit->subunit : unit;
		if (!context->get_overlay(ImageId::HallucinationHit)) {
			// Overlay
			context->sprite->create_overlay(ImageId::HallucinationHit, 0, 0, 0);

			// Sound
			if (!unitExt->timers[TimerHallucinating]) {
				helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, unit, 1, 0);
			} else {
				helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
			}
		}
		
		// Value
		unitExt->timers[TimerHallucinating] = time;
	}

	bool can_be_entered_by(Unit* transport, Unit* unit) {
		const u32 Func_CanBeEnteredBy = 0x004E6E00; // AKA CanEnterTransport()
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, transport
			PUSH unit
			CALL Func_CanBeEnteredBy
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	void show_error_message_with_sfx(u32 player, u32 statTxtId, u32 sfxId) {
		const u32 Func_ShowErrorMessageWithSfx = 0x0048EE30;
		__asm {
			PUSHAD
			MOV ESI, sfxId
			MOV EDI, statTxtId
			MOV EBX, player
			CALL Func_ShowErrorMessageWithSfx
			POPAD
		}
	}

	u16 get_tech_user_error_message(Unit* target, u8 castingPlayer, u16 techId) {
		const u32 Func_GetTechUseErrorMessage = 0x00491E80;
		static u32 techId_;
		static u16 errorMessage;
		techId_ = techId;
		__asm {
			PUSHAD
			PUSH techId_
			MOV BL, castingPlayer
			MOV EAX, target
			CALL Func_GetTechUseErrorMessage
			MOV errorMessage, AX
			POPAD
		}
		return errorMessage;
	}

	u16 get_last_queue_slot_id(Unit* a1) {
		const u32 Func_getLastQueueSlotType = 0x0047B270;
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	void set_offset_to_parent_special_overlay(Image* image) {
		const u32 Func_updateImagePositionOffset = 0x004D5A00;
		__asm {
			PUSHAD
			MOV ECX, image
			CALL Func_updateImagePositionOffset
			POPAD
		}
	}

	u32 get_ground_height_at_pos(s32 x, s32 y) {
		const u32 Func_GetGroundHeightAtPos = 0x004BD0F0;
		static u32 height;
		__asm {
			PUSHAD
			MOV EAX, y
			MOV ECX, x
			CALL Func_GetGroundHeightAtPos
			MOV height, EAX
			POPAD
		}
		return height;
	}

	void disable_dialog(BinDlg* dialog) {
		const u32 Func_DisableDialog = 0x00418640;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_DisableDialog
			POPAD
		}
	}

	void show_dialog(BinDlg* dialog) {
		const u32 Func_ShowDialog = 0x004186A0;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_ShowDialog
			POPAD
		}
	}

	void hide_dialog(BinDlg* dialog) {
		const u32 Func_HideDialog = 0x00418700;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_HideDialog
			POPAD
		}
	}

	void enable_control(BinDlg* dialog) {
		const u32 Func_Sub418E00 = 0x00418E00;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_Sub418E00
			POPAD
		}
	}

	void update_dialog(BinDlg* dialog) {
		const u32 Func_UpdateDialog = 0x0041C400;
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_UpdateDialog
			POPAD
		}
	}

	void prepare_unit_move(Unit* unit, bool hideUnit) {
		const u32 Func_PrepareUnitMoveClearRefs = 0x00493CA0;
		assert(unit);
		static b32 _hideUnit;
		_hideUnit = hideUnit ? 1 : 0;
		__asm {
			PUSHAD
			PUSH _hideUnit
			MOV EDI, unit
			CALL Func_PrepareUnitMoveClearRefs
			POPAD
		}
	}

	bool check_unit_collision_pos(Unit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea, bool hideErrorMsg, u32 someFlag) {
		const u32 Func_CheckUnitCollisionPos = 0x0049D3E0;
		assert(unit);
		assert(inPos);
		assert(outPos);
		static u32 result;
		static u32 _hideErrorMsg;
		_hideErrorMsg = hideErrorMsg;
		__asm {
			PUSHAD
			PUSH someFlag;
			PUSH _hideErrorMsg
				PUSH outPos
				PUSH inPos
				PUSH unit
				MOV EAX, moveArea
				CALL Func_CheckUnitCollisionPos
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
		const u32 Func_OrdersHoldPositionSuicidal = 0x004EB5B0;
		__asm
		{
			PUSHAD
			MOV ESI, unit
			CALL Func_OrdersHoldPositionSuicidal
			POPAD
		}
	}

	// Player check added based on how others functions test it. Values above or equal to 8 means all players.
	void move_screen_to_unit(Unit* unit, u32 player) { // 004E6020
		const u32 Func_MoveScreen = 0x0049C440;
		if (player >= 8 || player == *g_local_nation_id) {
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
				CALL Func_MoveScreen
				POPAD
			}
		}
	}

	// Player check added based on how others functions test it
	// Values above or equal to 8 means all players
	// x and y are coords as gotten from unit->sprite->position
	void minimap_ping(u32 x, u32 y, s32 color, u32 player) {
		const u32 Func_minimapPing = 0x004A34C0;
		if (player >= 8 || player == *g_local_nation_id) {
			__asm {
				PUSHAD
				PUSH color
				PUSH y
				PUSH x
				CALL Func_minimapPing
				POPAD
			}
		}
	}

	u8 has_overlay(Unit* unit) {
		const u32 Func_hasOverlay = 0x0047B720;
		static u8 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hasOverlay
			MOV result, AL
			POPAD
		}
		return result;
	}

	void hide_and_disable_unit(Unit* unit) {
		const u32 Func_unitDeathSomething_0 = 0x004E6340;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}
	}

	bool weapon_pierces_targets(u16 weapon_id) {
		switch (weapon_id) {
			case WeaponId::SentinelCerberusRailgun:
			case WeaponId::ClaymoreHarpoonRailgun:
			case WeaponId::PenumbraErebolShockCannon:
			case WeaponId::KalkaliskViscousAcid:
			{
				return true;
			}
		}
		return false;
	}

	void madrigal_dirge_fuse(Unit* unit, UnitExt* unitExt) {
		unit->sprite->create_top_overlay(ImageId::ArcliteShockCannon_Damageless);
		const u32 radius = 2 * 32;
		scbw::UnitFinder(
			unit->get_x() - radius,
			unit->get_y() - radius,
			unit->get_x() + radius,
			unit->get_y() + radius
		).for_each_in_match(cosmonarchy_finder_match(unit, Flags::IncludingSelf, {}),
		[&unitExt](Unit* target) {
			target->damage_with_custom(unitExt->values[ValueDirgeStacks] * MadrigalDirgeDamage * 256, damage_type::Explosive, unitExt->units[UnitDirgeSource], unitExt->values[ValueDirgeSourcePlayer], 0, 1, false);
		});
		unitExt->values[ValueDirgeStacks] = 0;
	}

	void update_signify_status(Unit* unit, UnitExt* unitExt) {
		unit->update_speed();

		const u32 cowardiceId = ImageId::Cowardice_Small + scbw::get_overlay_offset(unit->id);
		const u32 maliceId = ImageId::Malice_Small + scbw::get_overlay_offset(unit->id);

		if (unitExt->values[ValueSignifyStacks] == 0) {
			unit->remove_overlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
			unit->remove_overlay(ImageId::Malice_Small, ImageId::Malice_Large);
		} else if (!(units_dat::base_property[unit->id] & UnitProperty::Building)
				|| units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding
				|| unit->has_weapon()) {
			if (unitExt->values[ValueSignifyStacks] < 5 && !unit->get_overlay(cowardiceId)) {
				unit->sprite->create_top_overlay(cowardiceId);
			} else if (unitExt->values[ValueSignifyStacks] >= 5 && !unit->get_overlay(maliceId)) {
				unit->remove_overlay(ImageId::Cowardice_Small, ImageId::Cowardice_Large);
				unit->sprite->create_top_overlay(maliceId);
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
		for (u32 i = 0; i < 8; ++i) {
			Unit* unit = target->get_loaded_unit(i);
			mind_control(source, unit);
		}
		for (Unit* unit : target_ext->unit_vectors[UnitVectorMindControlling]) {
			mind_control(source, unit);
		}
		target_ext->unit_vectors[UnitVectorMindControlling].clear();

		// Demiurge - apply behavior
		if (source->id == UnitId::ProtossDemiurge) {
			auto& vec = source_ext->unit_vectors[UnitVectorWorshipersDemiurge];
			if (std::find(vec.begin(), vec.end(), target) == vec.end()) {
				vec.push_back(target);
			}
			target_ext->values[ValueWorshipStack] = 1;
		}

		// Mind Tyrant - apply behavior
		if (source->id == UnitId::ProtossMindTyrant) {
			target_ext->values[ValueTyrannyPlayer] = source->player + 1;
			target_ext->timers[TimerTyrannyRecede] = 5 * 24;
		}

		// Obtain control
		target->give_to(source->player);

		// Audio Visuals
		if (!(source->sprite->flags & SpriteFlags::Hidden)) {
			u32 offset = scbw::get_overlay_offset(target->id);
			if (source->id == UnitId::ProtossDemiurge) {
				target->sprite->create_top_overlay(ImageId::Worship_Small + offset, 0, 0, 0);
				helper::play_sound_from_unit(SoundId::DemiurgeWorship01, target, 1, 0);
			} else if (source->id == UnitId::ProtossMindTyrant) {
				target->sprite->create_top_overlay(ImageId::MindControlHit_Small + offset, 0, 0, 0);
			}
		}
	}

	void recede_mind_control(Unit* target) {
		if (!target) {
			return;
		}

		UnitExt* targetExt = get_unit_ext(target);
		Unit* mind_controller = targetExt->units[UnitMindController];
		targetExt->units[UnitMindController] = 0;

		if (mind_controller) {
			UnitExt* mind_controller_ext = get_unit_ext(mind_controller);
			auto& vec = mind_controller_ext->unit_vectors[UnitVectorMindControlling];
			vec.erase(std::remove(vec.begin(), vec.end(), target), vec.end());
		}

		target->give_to(targetExt->values[ValueMindControlOriginalPlayer] - 1);
		targetExt->values[ValueMindControlOriginalPlayer] = 0;

		targetExt->values[ValueWorshipStack] = 0;
		targetExt->timers[TimerGodsMadeFlesh] = 0;
		targetExt->values[ValueTyrannyPlayer] = 0;
	}

	bool unit_id_is_zerg_production(u16 unit_id) {
		return unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk
			|| unit_id == UnitId::ZergSibraelosk
			|| unit_id == UnitId::ZergNephrilosk
			|| unit_id == UnitId::ZergLarvoskColony
			|| unit_id == UnitId::ZergOcsalColony;
	}
	
	bool unit_id_is_town_center(u16 unit_id) {
		return unit_id == UnitId::TerranCommandCenter
			|| unit_id == UnitId::ProtossNexus
			|| unit_id == UnitId::ZergHachirosk
			|| unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergGathtelosk
			|| unit_id == UnitId::ZergSibraelosk
			|| unit_id == UnitId::ZergNephrilosk;
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
			|| unit_id == UnitId::TerranOberon
			|| unit_id == UnitId::TerranPhobos
			|| unit_id == UnitId::ProtossStarSovereign;
	}

	bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures) {
		if (count_gas_structures) {
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser
				|| unit_id == UnitId::ResourceVespeneRidge
				|| unit_id == UnitId::TerranReservoir
				|| unit_id == UnitId::ProtossAquifer
				|| unit_id == UnitId::ZergExcisant;
		} else {
			return unit_id == UnitId::ResourceMineralField
				|| unit_id == UnitId::ResourceMineralFieldType2
				|| unit_id == UnitId::ResourceMineralFieldType3
				|| unit_id == UnitId::ResourceVespeneGeyser
				|| unit_id == UnitId::ResourceVespeneRidge;
		}
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
				helper::removeCreepAtUnit(final_x, final_y, UnitId::ZergEgg);
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
		for (AiTown* town = gi_ai_active_town_array[player].first; town; town = town->prev) {
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

	u32 get_region_id(Point16 position) {
		return helper::get_region_id(position.x, position.y);
	}

	AiRegion* get_ai_region(u32 player, u32 index) {
		return &g_ai_region_captains[player][index];
	}

	u32 get_ai_region_military_count(AiRegion* ai_region) {
		u32 count = 0;
		for (MilitaryAi* military_ai = ai_region->first_military_ai; military_ai; military_ai = military_ai->link.prev) {
			count += 1;
		}
		return count;
	}

	Region* get_region(u32 index) {
		return &(*g_sai_paths)->regions[index];
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

	void ai_defend(u32 player, u32 region_id) {
		AiRegion* ai_region = scbw::get_ai_region(player, region_id);
		switch (ai_region->state) {
			case RegionState::Nothing:
			case RegionState::EnemySighted:
			case RegionState::TownRegionLow:
			case RegionState::TownRegionHigh:
			case RegionState::Defending:
			case RegionState::ChaseAway: {
				helper::ai_assign_military_to_region(RegionState::Defending, ai_region); // Priority is 110 when defending regions with buildings/workers, 100 otherwise
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
		unit_search_radial(&allied_structures, victim->position, 30 * 32);
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
		return unit_id == UnitId::TerranReservoir || unit_id == UnitId::ProtossAquifer || unit_id == UnitId::ZergExcisant;
	}

	bool unit_id_is_gas_structure(u16 unit_id) {
		return unit_id_is_capped_gas_structure(unit_id) || unit_id == UnitId::ResourceVespeneRidge || unit_id == UnitId::ResourceVespeneGeyser;
	}

	bool unit_id_is_smaller_building(u16 unit_id) {
		return units_dat::base_property[unit_id] & UnitProperty::Addon
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
			|| unit_id == UnitId::ZergOcsalColony
			|| unit_id == UnitId::ZergKazitrithColony
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
		UnitExt* unitExt = get_unit_ext(unit);
		if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
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
		if (unit->id == UnitId::ZergCaphrolosk || unit->id == UnitId::ZergSibraelosk)
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
			case UnitId::TerranEngineeringBay:
			case UnitId::TerranScienceFacility:
			case UnitId::TerranQuantumInstitute:
		//	case UnitId::TerranCapitol:

			// Zerg
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
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
			case UnitId::TerranEngineeringBay:
			case UnitId::TerranScienceFacility:
			case UnitId::TerranQuantumInstitute:
		//	case UnitId::TerranCapitol:

			// Zerg
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
			case UnitId::ZergOrbothOmlia:
				return true;
		}
		return false;
	}

	AiScript* run_ai(u32 player, Box32 area, s32 name) {
		s32 is_bw = 0;
		s32 block = helper::parse_aiscript_name(name, &is_bw);
		if (block) {
			AiScript* ai_script = helper::run_aiscript(&area, player, block, is_bw);
			if (ai_script) {
				Point16 center = {};
				center.x = area.x + area.w / 2;
				center.y = area.y + area.h / 2;

				ai_script->flags &= 0x3;
				ai_script->flags |= scbw::get_closest_resarea(center) << 3;
			}
			return ai_script;
		} else {
			scbw::printf("scenario.cpp, run_ai: Invalid AI block!");
		}
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
		if (race == RaceId::Terran) {
			return scbw::run_ai(player, pos, 'xCMT');
		} else if (race == RaceId::Protoss) {
			return scbw::run_ai(player, pos, 'xCMP');
		} else if (race == RaceId::Zerg) {
			return scbw::run_ai(player, pos, 'xCMZ');
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
			case SpriteId::Corpse_Cleric:
			case SpriteId::Corpse_Eidolon:
			case SpriteId::Corpse_Apostle:
			case SpriteId::Corpse_Blackjack:
			case SpriteId::Corpse_Scribe:
			case SpriteId::Corpse_Dracadin:
			case SpriteId::Corpse_Larva:
			case SpriteId::Corpse_Egg:
			case SpriteId::Corpse_Droleth:
			case SpriteId::Corpse_Zethrokor:
			case SpriteId::Corpse_Hydralisk:
			case SpriteId::Corpse_Izirokor:
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
			case SpriteId::Corpse_Aspirant: {
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

	u32 get_zerg_regression(u32 unit_id) {
		switch (unit_id) {
			case UnitId::ZergSurkithColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergSpraithColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergLarvoskColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergKazitrithColony:		return UnitId::ZergCagrantColony;
			case UnitId::ZergOcsalColony:			return UnitId::ZergCagrantColony;
			case UnitId::ZergCaphrolosk:			return UnitId::ZergHachirosk;
			case UnitId::ZergGathtelosk:			return UnitId::ZergHachirosk;
			case UnitId::ZergSibraelosk:			return UnitId::ZergCaphrolosk;
			case UnitId::ZergNephrilosk:			return UnitId::ZergGathtelosk;
			case UnitId::ZergOthstolOviform:		return UnitId::ZergIrolIris;
			case UnitId::ZergAlaszilArbor:			return UnitId::ZergOthstolOviform;
			case UnitId::ZergOrbothOmlia:			return UnitId::ZergAlaszilArbor;
			case UnitId::ZergLiivralPond:			return UnitId::ZergQuazrokPool;
			case UnitId::ZergVithrathHaunt:			return UnitId::ZergMuthrokSpire;
			case UnitId::ZergGorgralSwamp:			return UnitId::ZergHydrokDen;
			case UnitId::ZergBegrozithBarrow:		return UnitId::ZergAxtothAxis;
			case UnitId::ZergAlkajChasm:			return UnitId::ZergGeszkathGrotto;
			case UnitId::ZergChokralgantChazsal:	return UnitId::ZergIzkagIteth;
			case UnitId::ZergUltrokCavern:			return UnitId::ZergZorkizShroud;
			case UnitId::ZergAlmakisAntre:			return UnitId::ZergMatravilNest;
			case UnitId::ZergRakzogthRoost:			return UnitId::ZergElthisthMound;
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

	CorpseConsume unit_get_corpse_consume(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (unit_ext->parasite_spawn_count[i]) {
				return CorpseConsume_Parasite;
			}
		}
		if (unit_ext->values[ValueLazarusAgent] || unit_ext->timers[TimerAnticthonRevive]) {
			return CorpseConsume_Revive;
		}
		return CorpseConsume_None;
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
			|| buttonset_id == UnitId::Buttons_LarvaShared;
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
			case UnitId::TerranAmbassador: {
				return WeaponId::AmbassadorSignatorySalvo;
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

	void set_bullet_target(Bullet* bullet, Unit* target) {
		bullet->attack_target.unit = target;
		bullet->attack_target.pt   = target->position;
		bullet->move_target.unit   = target;
		bullet->move_target.pt     = target->position;
		bullet->targetPosition     = target->position;
		bullet->targetPosition2    = target->position;
	}

	void set_bullet_target(Bullet* bullet, Point16 target) {
		bullet->attack_target.unit = 0;
		bullet->attack_target.pt   = target;
		bullet->move_target.unit   = 0;
		bullet->move_target.pt     = target;
		bullet->targetPosition     = target;
		bullet->targetPosition2    = target;
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
		return unit_id != UnitId::TerranSevenstep
			&& unit_id != UnitId::ProtossVagrantShade;
	}

	bool unit_id_is_stackable_building(u32 unit_id) {
		switch (unit_id) {
			case UnitId::TerranWatchdog:
			case UnitId::TerranBunker:
			case UnitId::TerranSentinel:
			case UnitId::ProtossPylon:
			case UnitId::ProtossWarden:
			case UnitId::ProtossMatrix:
			case UnitId::ZergCagrantColony:
			case UnitId::ZergSurkithColony:
			case UnitId::ZergSpraithColony:
			case UnitId::ZergLarvoskColony:
			case UnitId::ZergKazitrithColony:
			case UnitId::ZergOcsalColony:
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
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
		switch (weapon_id) {
			case WeaponId::EcclesiastAstralBlaster: {
				heal_amount = 2;
			} break;
			case WeaponId::PatriarchIronhandWeaver: {
				heal_amount = 5;
			} break;
		}
		const s32 range = 64;
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
				unit->heal_shields(2 * 256, victim);
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

	bool unit_id_spawns_ocsal_larva(u32 unit_id) {
		return unit_id == UnitId::ZergCaphrolosk
			|| unit_id == UnitId::ZergSibraelosk
			|| unit_id == UnitId::ZergOcsalColony;
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

	void town_add_spending_request(AiTown* town, u32 count, u32 unit_id, u32 priority) {
		if (count < 0) return;

		u32 base_unit_id = find_unit_base(unit_id);
		if (base_unit_id != UnitId::None) {
			u32 base_count = town_count_units(town, base_unit_id, false);
			if (base_count < count) {
				town_add_spending_request(town, count - base_count, base_unit_id, priority);
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

	void military_add_spending_request(u32 player, u32 unit_id, Point16 position, u32 priority) {
		if (unit_id == UnitId::TerranPhalanxSiegeMode) {
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		u32 base_unit_id = find_unit_base(unit_id);
		if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
			military_add_spending_request(player, unit_id, position, priority);
		}
		helper::ai_add_spending_request(unit_id, player, RequestType::Military, priority, scbw::get_ai_region(player, scbw::get_region_id(position)));
	}

	void guard_add_spending_request(u32 player, GuardAi* guard_ai, u32 priority) {
		u32 unit_id = guard_ai->unit_id;
		if (unit_id == UnitId::TerranPhalanxSiegeMode) {
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		u32 base_unit_id = find_unit_base(unit_id);
		if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
			military_add_spending_request(player, unit_id, guard_ai->current_home, priority); // Intentionally using military, since we don't want the base unit to take over the guard
		}
		helper::ai_add_spending_request(unit_id, player, RequestType::Guard, priority, guard_ai);
	}

	// Based on the function at 0x00462670 -Veeq7
	GuardAi* create_guard_ai(u32 player, u32 unit_id, Point16 position) {
		GuardAiList* guard_list = &g_player_guard_list->player[player];
		GuardAi* guard_ai = guard_list->guard_array->first_free_guard;
		if (guard_ai) {
			// Remove from free list
			guard_list->guard_array->first_free_guard = guard_ai->link.prev;
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
}