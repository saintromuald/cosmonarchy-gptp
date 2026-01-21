#include "maxhp.h"
#include "BW\api.h"
#include "BW\bwdata.h"
#include "hook_tools.h"
#include "byte_operations_utils.h"
#include <globals.h>

/*
	// Hooked elsewhere
	Line 2455: 004026D4  |.  8A91 B0476600 MOV DL,BYTE PTR DS:[ECX+6647B0] -- 0x004026D0 -- GetCurrentHealth -- HOOKED
	Line 56715: 0042645A  |>  8A87 B0476600 MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x004263E0 -- Ss_DrawHealthEnergy -- HOOKED
	Line 57377: 00426CC7  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00426C60 -- Ss_DrawDisplayButtons -- HOOKED
	Line 95486: 00440647  |.  80BA B0476600>|CMP BYTE PTR DS:[EDX+6647B0],0 -- 0x004405E0 -- PickBestTarget? -- HOOKED
	Line 181066: 004799B7  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00479930 -- DoWeaponDamage -- HOOKED
	Line 219510: 00493535  |.  8A82 B0476600 MOV AL,BYTE PTR DS:[EDX+6647B0] -- 0x00493520 -- unitCanRechargeShields -- HOOKED
	Line 351277: 004EC2C7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004EC290 -- ProgressUnitTimers -- HOOKED

	// Hooked in this file
	Line 2486: 00402714  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00402710 -- GetMaxHealth -- HOOKED
	Line 56570: 004262B7  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00426190 -- draw_resource_amount -- HOOKED
	Line 58295: 0042781F  |.  8A8A B0476600 MOV CL,BYTE PTR DS:[EDX+6647B0] -- 0x00427540 -- Ss_DrawSupplyUnitStatus -- HOOKED
	Line 129037: 004566B4  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004566B0 -- drawWireframeShield -- HOOKED
	Line 95955: 00440A9A   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x00440A20 -- ai_target_enemy_proc -- HOOKED
	Line 58440: 004279F3   .  8A83 B0476600 MOV AL,BYTE PTR DS:[EBX+6647B0] -- 0x00427890 -- draw_building_info -- HOOKED
	Line 72970: 00431274  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00431270 -- calculate_unit_strength -- HOOKED
	Line 73606: 00431865  |.  8A86 B0476600 MOV AL,BYTE PTR DS:[ESI+6647B0] -- 0x00431800 -- get_unit_strength -- HOOKED
	Line 95686: 00440800   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004407E0 -- AI_RestorationRequirementsProc -- HOOKED
	Line 96913: 004413E7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004413D0 -- AI_EMPShieldRequirementsProc -- HOOKED
	Line 180864: 00479777  |.  8A82 B0476600 MOV AL,BYTE PTR DS:[EDX+6647B0] -- 0x00479730 -- attack_overlay_and_notify -- HOOKED
	Line 182607: 0047A841  |.  8A8F B0476600 MOV CL,BYTE PTR DS:[EDI+6647B0] -- 0x0047A820 -- DrawImage_HpBar -- HOOKED
	Line 184040: 0047B6A5  |.  8A87 B0476600 MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x0047B6A0 -- update_building_construct_shields_amount -- HOOKED, dysfunctional
	Line 206949: 0048AD35  |.  8A81 B0476600 MOV AL,BYTE PTR DS:[ECX+6647B0] -- 0x0048ACD0 -- get_unit_bullet_damage -- HOOKED, compiler errors
	Line 217770: 00492239  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00492140 -- cannot_cast_spell -- HOOKED
	Line 293899: 004C5A3D  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004C5A20 -- modify_unit_shields -- HOOKED
	Line 318718: 004D60BC  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004D6010 -- compile_health_bar -- HOOKED, compiler error
	Line 343282: 004E6FC6  |.  8A87 B0476600 |MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x004E6EF0 -- ai_find_best_transport -- HOOKED, compiler errors
	Line 349375: 004EB0F4   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x004EAFE0 -- map_data_transfer -- HOOKED, type error?
*/

namespace hooks
{
	u32 get_max_hp(s32 id, s32 player)
	{
		u32 result = units_dat::max_hitpoints[id];
		return result;
	}

	u32 get_max_shields(s32 id, s32 player)
	{
		u32 result = units_dat::max_shield_points[id];
		//if (units_dat::base_property[id] & UnitProperty::RoboticUnit && bw::get_upgrade_level(player, UpgradeId::Autovitality)) {
		//	result += result / 2;
		//}
		return result;
	}

	u32 get_max_vitality(Unit* unit) // 00402710
	{
		s32 current_shields = 0;
		if (bw::unit_has_shields(unit))
		{
			current_shields = units_dat::max_shield_points[unit->id];
		}		
		s32 hitpoints = units_dat::max_hitpoints[unit->id] >> 8;
		if (!hitpoints)
		{
			hitpoints = (unit->hitpoints + 255) >> 8;
			if (!hitpoints)
			{
				hitpoints = 1;
			}
		}
		return current_shields + hitpoints;
	}

	void __stdcall stats_panel_draw_health_shields(BinDlg* dialog) // 004263E0
	{
		const char* string = "";
		Unit* active_unit = *g_active_portrait_unit;
		s32 max_hitpoints = units_dat::max_hitpoints[active_unit->id] >> 8;
		if ((max_hitpoints || (max_hitpoints = (active_unit->hitpoints + 255) >> 8) != 0) && max_hitpoints > 9999
		||  units_dat::max_shield_points[active_unit->id] > 9999 || active_unit->status & UnitStatus::Invincible)
		{
			string = 0;
		} else {
			u8 hp_state = 0;
			if (helper::unit_hp_below_33_percent(active_unit)) {
				hp_state = 6;
			} else {
				hp_state = helper::unit_hp_between_33_66_percent(active_unit) != 0 ? 3 : 7;
			}

			if (bw::unit_has_shields(active_unit)) {
				s32 max_hp = helper::get_max_hp(active_unit);
				s32 active_hp = (active_unit->hitpoints + 255) >> 8;
				u16 queue_slot = helper::get_last_queue_slot_id(active_unit);
				s32 max_shields = units_dat::max_shield_points[queue_slot];
			//	helper::bw_sprintf_s(0, 32, "%d/%d %c%d/%d", active_unit->shields >> 8, max_shields, hp_state, active_hp, max_hp);
			} else {
				s32 max_hp = helper::get_max_hp(active_unit);
			//	helper::bw_sprintf_s(0, 32, "%c%d/%d", hp_state, (active_unit->hitpoints + 255) >> 8, max_hp);
			}
			string = "";
		}
		helper::add_text_to_dialog(dialog, -7, string);
		helper::stats_panel_draw_energy(dialog);
	}

	b32 ai_target_enemy_proc(Unit* target, Unit* unit) { // 00440A20
		b32 result = 0;
		if (target != unit) {
			u8 target_player = target->player == 11 ? target->sprite->player : target->player;
			if (!bw::is_allied_to(unit->player, target_player)) {
				if (helper::ai_unit_can_attack(target)) {
					result = 1;
				}
			}
		}
		return result;
	}

	u32 calculate_unit_strength(u32 unit_id, u32 weapon_id) { // 00431270
		s32 max_shields = 0;
		if (units_dat::shields_enabled[unit_id]) {
			max_shields = units_dat::max_shield_points[unit_id];
		}

		s32 max_vitality = max_shields + (units_dat::max_hitpoints[unit_id] >> 8);
		u64 final_strength = 0;
		if (!max_vitality) {
			return (u32)final_strength;
		}
		u32 damage_factor = weapons_dat::damage_factor[weapon_id];
		u32 weapon_cooldown = weapons_dat::cooldown[weapon_id];
		u32 weapon_damage = weapons_dat::damage_amount[weapon_id];
		u32 weapon_strength = 0;
		if (weapon_id != WeaponId::None) {
			weapon_strength = (weapon_damage * damage_factor << 11) / (s32)weapon_cooldown;
		}
		final_strength = (u64)(sqrt((double)(weapon_damage * damage_factor * (weapons_dat::max_range[weapon_id] / weapon_cooldown)
									+ ((u32)(max_vitality * weapon_strength) >> 8))) * 7.58);
		return (u32)final_strength;
	}

	u32 get_unit_strength(Unit* unit, s32 unused, bool is_ground) { // 00431800
		if (unit->id == UnitId::ZergLarva || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination) {
			if (helper::get_current_hp_plus_shields(unit) < get_max_vitality(unit)) {
				return 0;
			}
		}
		s32 base_strength = helper::get_base_strength(is_ground, unit);
		s32 current_vitality = helper::get_current_hp_plus_shields(unit);
		s32 max_shields = 0;
		if (bw::unit_has_shields(unit)) {
			max_shields = units_dat::max_shield_points[unit->id];
		}
		s32 max_vitality = max_shields + helper::get_max_hp(unit);
		if (!max_vitality) {
			max_vitality = 1;
		}
		return base_strength * current_vitality / max_vitality;
	}

	b32 ai_restoration_requirements_proc(Unit* caster, Unit* target) { // 004407E0
		b32 result = 0;
		if (caster->player == target->player && helper::can_weapon_target_unit(WeaponId::Restoration, caster, target)) {
			s32 shields = bw::unit_has_shields(target) ? target->shields >> 8 : 0;
			if (shields + ((target->hitpoints + 255) >> 8) > 60
			&& (target->ensnare_timer || target->plague_timer || target->irradiate_timer || target->lockdown_timer || target->maelstrom_timer || target->corrosive_spore_count_unused))
			{
				result = 1;
			}
		}
		return result;
	}

	b32 ai_emp_shield_requirements_proc(Unit* target, Unit* caster) // 004413D0
	{
		if (!helper::ai_target_unit_is_worth_hitting(target, caster) || !bw::unit_has_shields(target) || !helper::can_weapon_target_unit(WeaponId::PsiAssault, target, caster))
		{
			return 0;
		}
		u16 pos_x = target->sprite->position.x;
		u16 pos_y = target->sprite->position.y;
		*g_temp_hp_shields = 0;
		*g_temp_weapon = WeaponId::PsiAssault;
		helper::create_splash_weapon_proc(160, pos_x, pos_y, (s32(__fastcall*)(s32, s32))helper::unit_shields_clump_in_area_accumulator_proc, caster);
		return *g_temp_hp_shields >= 200;
	}

	// Hooked to improve functionality and extend max HP / shields. -Pr0nogo
	void attack_overlay_and_notify(Unit* victim, Unit* attacker, u8 weapon_id, u8 direction) { // 00479730
		s32 hitpoints = victim->hitpoints;
		if (hitpoints && !(victim->status & UnitStatus::Invincible))
		{
			if (attacker) {
				helper::unit_was_hit(victim, attacker, weapon_id != WeaponId::Irradiate);
				if (weapon_id != WeaponId::Irradiate && victim->player != attacker->player) {
					helper::unit_attack_notify_noop(victim);
				}
			}
			if (bw::unit_has_shields(victim)) {
				if (victim->shields >= 256) {
					if (weapons_dat::damage_type[weapon_id]) {
						helper::create_shield_overlay(weapon_id, victim);
					}
				}
			}
		}
	}

	void draw_image_hp_bar(s32 g_screen_x, s32 g_screen_y, GrpFrame* pFrame, Box32* grpRect, s32 colorData)
	{ // 0047A820
		s32 v5; // esi
		s32 v6; // edi
		char v7; // cl
		s32 v8; // eax
		s32 v9; // ecx
		s32 v10; // eax
		unk_61* v11; // ecx
		s32 v12; // edx
		u8 v13; // dl
		s32 v14; // eax
		s32 v15; // ecx
		s32 v16; // ebx
		unk_62* v17; // eax
		s32 v18; // ecx
		u16 v19; // di
		s32 v20; // ebx
		u16 v21; // ax
		s32 v22; // ebx
		s32 v23; // edi
		s32 v24; // ecx
		u16 v25; // ax
		s32 v26; // eax
		s32 v27; // ebx
		s32 v28; // edx
		bool v29; // zf
		s32 v30; // eax
		unk_61* v31; // ecx
		s32 v32; // edx
		s32 v33; // [esp+Ch] [ebp-1Ch]
		s32 v34; // [esp+10h] [ebp-18h]
		s32 v35; // [esp+14h] [ebp-14h]
		s32 v36; // [esp+18h] [ebp-10h]
		s32 v37; // [esp+1Ch] [ebp-Ch]
		s32 v38; // [esp+20h] [ebp-8h]
		s32 v39; // [esp+24h] [ebp-4h]

		v37 = g_screen_x;
		v5 = colorData;
		v6 = *(u16*)(colorData + 100);
		v33 = grpRect->left;
		v34 = grpRect->top;
		v7 = units_dat::shields_enabled[v6];
		v38 = g_screen_y;
		v8 = grpRect->bottom;
		v35 = grpRect->right;
		if (v7)
		{
			v36 = 7;
			if (v8 <= 7)
				v36 = v8;
			v13 = sprites_dat::is_visible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390];
			v14 = units_dat::max_hitpoints[v6] >> 8;
			ByteOperations::SetHighByte(colorData, sprites_dat::is_visible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390]);
			if (v14)
			{
				v15 = v14;
			}
			else
			{
				v15 = (*(s32*)(v5 + 8) + 255) >> 8;
				if (!v15)
					v15 = 1;
			}
			v16 = helper::unk_billions((*(s32*)(v5 + 8) + 255) >> 8, v15, (s32)&v39, v13);
			helper::unk_billions2(*(s32*)(v5 + 96) >> 8, units_dat::max_shield_points[v6], (s32)&colorData, HIBYTE(colorData));
			v17 = struct_5140E8;
			if (v16 < (u8)(*struct_5140E8).anonymous_0)
			{
				do
				{
					v18 = (u8)v17[1].anonymous_0;
					++v17;
				}
				while (v16 < v18);
			}
			v19 = v38;
			v20 = v37;
			helper::hpBarSurfaceBuffer(v37, v38, &v33, v39, (s32)v17);
			helper::hpBarSurfaceBuffer2(colorData, v20, v19, (s32)&v33);
			ByteOperations::SetHighByte(colorData, (u8)8);
		}
		else
		{
			v36 = 5;
			if (v8 <= 5)
				v36 = v8;
			if (units_dat::max_hitpoints[v6] >> 8)
			{
				v9 = units_dat::max_hitpoints[v6] >> 8;
			}
			else
			{
				v9 = (*(s32*)(colorData + 8) + 255) >> 8;
				if (!v9)
					v9 = 1;
			}
			v10 = helper::unk_billions(
				(*(s32*)(colorData + 8) + 255) >> 8,
				v9,
				(s32)&v39,
				(u8)sprites_dat::is_visible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390]);
			v11 = struct_5140C4;
			if (v10 < (u8)(*struct_5140C4).anonymous_0)
			{
				do
				{
					v12 = (u8)v11[1].anonymous_0;
					++v11;
				}
				while (v10 < v12);
			}
			helper::renderHPBar(v37, v38, &v33, v39, (s32)v11);
			ByteOperations::SetHighByte(colorData, 6);
		}
		v21 = *(s16*)(v5 + 100);
		v22 = units_dat::base_property[v21] & UnitProperty::Spellcaster;
		if (v22 && (v23 = (*(s32*)(v5 + 220) >> 30) & 1) == 0
			|| (v23 = (*(s32*)(v5 + 220) >> 30) & 1) != 0
			|| v21 == 40)
		{
			if (*(u8*)(v5 + 76) == g_local_nation_id)
			{
				if (v22)
				{
					if (!v23)
					{
						v24 = helper::get_unit_max_energy((Unit*)v5) >> 8;
						goto LABEL_32;
					}
				}
				else if (!v23)
				{
					v25 = v21 != 40 ? 0 : 0x708;
					goto LABEL_31;
				}
				v25 = 1350;
LABEL_31:
				v24 = v25;
LABEL_32:
				if (!v22 || v23)
					v26 = *(u16*)(v5 + 272);
				else
					v26 = *(u8*)(v5 + 163);
				v27 = HIBYTE(colorData);
				v28 = grpRect->bottom - HIBYTE(colorData);
				v29 = grpRect->bottom == HIBYTE(colorData);
				v34 = HIBYTE(colorData) + grpRect->top;
				v36 = v28;
				if (v28 >= 0 && !v29)
				{
					v30 = helper::getHpBarDrawSizes(
						v26,
						v24,
						(s32)&v39,
						(u8)sprites_dat::is_visible[*(u16*)(*(s32*)(v5 + 12) + 8) + 390]);
					v31 = struct_5140D8;
					if (v30 < (u8)(*struct_5140D8).anonymous_0)
					{
						do
						{
							v32 = (u8)v31[1].anonymous_0;
							++v31;
						}
						while (v30 < v32);
					}
					helper::renderHPBar(v37, v38 + v27, &v33, v39, (s32)v31);
				}
				return;
			}
		}
	}

	void update_building_construct_shields_amount(Unit* unit) // 0047B6A0
	{
		if (bw::unit_has_shields(unit))
		{
			if (unit->status & UnitStatus::GroundedBuilding)
			{
				u32 max_shields = units_dat::max_shield_points[unit->id];
				s32 shields_xd = (s32)((u64)(1717986919i64 * (max_shields << 8)) >> 32) >> 2;
				s32 shields = shields_xd + (shields_xd >> 31);
				unit->shields = shields;
				u32 time_cost = units_dat::time_cost[unit->id];
				u32 missing_shields = (((u32)max_shields << 8) - shields) / time_cost;
				if (!time_cost || unit->build_repair_hp_gain != missing_shields)
				{
					unit->build_repair_hp_gain = 1;
				}
			}
		}
	}

	u32 get_unit_bullet_damage(Unit* victim, Bullet* bullet) // 0048ACD0
	{
		// Originally had Nuclear Missile code here.

		u32 damage_amount = weapons_dat::damage_amount[bullet->weapon_id];

		// Originally had code for upgrades here.

		return damage_amount << 8;
	}

	b32 __stdcall modify_unit_shields(Unit* unit, s32 new_amount) // 004C5A20
	{
		if (!unit)
		{
			helper::SErrSetLastError(87);
			return 0;
		}
		if (!bw::unit_has_shields(unit))
		{
			return 0;
		}

		s32 max_shields = units_dat::max_shield_points[unit->id];
		if (max_shields >= (signed int)(new_amount * max_shields / 100))
		{
			max_shields = new_amount * units_dat::max_shield_points[unit->id] / 100;
		}
		unit->shields = max_shields << 8;
		return 1;
	}

	b32 ai_find_best_transport(Unit* unit, Unit* result_unit) // 004E6EF0
	{
		result_unit->link.prev = 0;
		Unit* first_unit = first_player_unit[unit->player].unit[0];
		if (!first_unit)
		{
			return 0;
		}

		b32 found_transport = 0;
		u32 max_distance = 99999999;
		while (1)
		{
			if (!(first_unit->status & UnitStatus::IsHallucination)) {
				if (units_dat::space_provided[first_unit->id]) {
					if (first_unit->status & UnitStatus::Completed && first_unit->status & UnitStatus::Reacts) {
						break;
					}
				}
			}
		valid_transport:
			first_unit = first_unit->player_link.next;
			if (!first_unit)
			{
				return found_transport;
			}
		}

		if (first_unit->order_target.unit != unit)
		{
			s32 first_unit_hp = units_dat::max_hitpoints[first_unit->id] >> 8;
			if (!first_unit_hp)
			{
				first_unit_hp = (first_unit->hitpoints + 255) >> 8;
				if (!first_unit_hp)
				{
					first_unit_hp = 1;
				}
			}
			if ((first_unit->hitpoints + 255) >> 8 >= 3 * first_unit_hp / 4
			||	 bw::unit_has_shields(first_unit) && first_unit->shields >> 8 == units_dat::max_shield_points[first_unit->id])
			{
				found_transport = 1;
				if (helper::can_enter_transport(first_unit, unit))
				{
					u32 order_id = first_unit->main_order_id;
					if (order_id != OrderId::Pickup2 && order_id != OrderId::Unload)
					{
						u32 region_id = region_id = helper::get_region_id(first_unit->order_target.pt.x, first_unit->order_target.pt.y);
						if (order_id != OrderId::MoveUnload || region_id == helper::get_region_id(unit->order_target.pt.x, unit->order_target.pt.y))
						{
							u32 distance = helper::get_unit_distance_to_halt(first_unit->sprite->position.y, unit, first_unit->sprite->position.x);
							if (distance < max_distance)
							{
								result_unit->link.prev = first_unit;
								max_distance = distance;
							}
						}
					}
				}
			}
			goto valid_transport;
		}
		result_unit->link.prev = first_unit;
		return 1;
	}

	s32 map_data_transfer(char a1, s32 a2, s32 a3)
	{ // 004EAFE0
		s32 v3; // eax
		s32 result; // eax

		if (!a2 || !a3)
			return 0;
		switch (*(unsigned __int8*)a2)
		{
			case 0u:
				result = helper::dataTransfer00(a3, a1, a2);
				break;
			case 1u:
				result = helper::dataTransfer01(a3, a2, a1);
				break;
			case 2u:
				result = helper::dataTransfer02(a3, a2, a1);
				break;
			case 3u:
				result = helper::dataTransfer03(a3, a1, a2);
				break;
			case 4u:
				result = helper::dataTransfer04(a3, a2, a1);
				break;
			case 5u:
				result = helper::dataTransfer05(a3, a2);
				break;
			case 6u:
				if (a3 == 3 && (v3 = (s32)helper::vectorTransfer(*(s8*)(a2 + 2), *(s8*)(a2 + 1))) != 0)
				{
					*(s8*)(v3 + 52) &= 0xFDu;
					helper::dataTransfer06(v3);
					result = 1;
				}
				else
				{
					result = 0;
				}
				break;
			default:
				return 0;
		}
		return result;
	}
}