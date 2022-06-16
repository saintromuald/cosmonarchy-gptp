#include "maxhp.h"
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
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
	Line 56570: 004262B7  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00426190 -- drawUnitInfo -- HOOKED
	Line 58295: 0042781F  |.  8A8A B0476600 MOV CL,BYTE PTR DS:[EDX+6647B0] -- 0x00427540 -- Ss_DrawSupplyUnitStatus -- HOOKED
	Line 129037: 004566B4  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004566B0 -- drawWireframeShield -- HOOKED
	Line 95955: 00440A9A   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x00440A20 -- AI_TargetEnemyProc -- HOOKED
	Line 58440: 004279F3   .  8A83 B0476600 MOV AL,BYTE PTR DS:[EBX+6647B0] -- 0x00427890 -- draw_building_info -- HOOKED
	Line 72970: 00431274  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00431270 -- calcUnitStrength -- HOOKED
	Line 73606: 00431865  |.  8A86 B0476600 MOV AL,BYTE PTR DS:[ESI+6647B0] -- 0x00431800 -- get_unit_strength -- HOOKED
	Line 95686: 00440800   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004407E0 -- AI_RestorationRequirementsProc -- HOOKED
	Line 96913: 004413E7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004413D0 -- AI_EMPShieldRequirementsProc -- HOOKED
	Line 180864: 00479777  |.  8A82 B0476600 MOV AL,BYTE PTR DS:[EDX+6647B0] -- 0x00479730 -- attack_overlay_and_notify -- HOOKED
	Line 182607: 0047A841  |.  8A8F B0476600 MOV CL,BYTE PTR DS:[EDI+6647B0] -- 0x0047A820 -- DrawImage_HpBar -- HOOKED
	Line 184040: 0047B6A5  |.  8A87 B0476600 MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x0047B6A0 -- setBuildShieldGain -- HOOKED, dysfunctional
	Line 206949: 0048AD35  |.  8A81 B0476600 MOV AL,BYTE PTR DS:[ECX+6647B0] -- 0x0048ACD0 -- getUnitBulletDamage -- HOOKED, compiler errors
	Line 217770: 00492239  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00492140 -- canCastSpell -- HOOKED
	Line 293899: 004C5A3D  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004C5A20 -- modifyUnitShields -- HOOKED
	Line 318718: 004D60BC  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004D6010 -- compileHealthBar -- HOOKED, compiler error
	Line 343282: 004E6FC6  |.  8A87 B0476600 |MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x004E6EF0 -- ai_find_transport -- HOOKED, compiler errors
	Line 349375: 004EB0F4   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x004EAFE0 -- mapDataTransfer -- HOOKED, type error?
*/

namespace hooks
{
	u32 maxhp(s32 id, s32 player)
	{
		u32 result = units_dat::MaxHitPoints[id];
		//if (id==UnitId::firebat && scbw::get_upgrade_level(player, UpgradeId::Heatshield)) {
		//	result += 15 * 256;
		//}
		return result;
	}

	u32 maxshields(s32 id, s32 player)
	{
		u32 result = units_dat::MaxShieldPoints[id];
		//if (units_dat::base_property[id] & UnitProperty::RoboticUnit && scbw::get_upgrade_level(player, UpgradeId::Autovitality)) {
		//	result += result / 2;
		//}
		return result;
	}

	u32 get_total_hp_plus_shields(Unit* a1)
	{ // 00402710
		s32 v1; // eax
		s32 v2; // ecx
		s32 v3; // eax

		v1 = (u16)a1->id;
		if (units_dat::ShieldsEnabled[v1])
			v2 = units_dat::MaxShieldPoints[v1];
		else
			v2 = 0;
		v3 = units_dat::MaxHitPoints[v1] >> 8;
		if (!v3)
		{
			v3 = (a1->hitpoints + 255) >> 8;
			if (!v3)
				v3 = 1;
		}
		return v2 + v3;
	}

	void drawUnitInfo(BinDlg* a1)
	{ // 00426190
		BinDlg* v1; // ebx
		s16 v2; // ax
		const char* v3; // eax
		u16 v4; // cx
		const char* v5; // eax
		BinDlg* v6; // eax
		s32 v7; // [esp-8h] [ebp-10h]
		BufferStr* v8; // [esp-8h] [ebp-10h]

		v1 = a1;
		*suppliesAvailableTemp = (*active_portrait_unit)->building.resource.resource_amount;
		if (*statusScreenFunc != 15)
		{
			helper::hide_optional_status_screen_controls(a1);
			helper::show_control_group(v1, -10, -13);
			*statusScreenFunc = 15;
		}
		helper::add_text_to_dialog(v1, -10, 0);
		helper::add_text_to_dialog(v1, -12, 0);
		helper::add_text_to_dialog(v1, -13, 0);
		v2 = (*active_portrait_unit)->id;
		if ((u16)v2 >= 0xB0u && (u16)v2 <= 0xB2u)
		{
			if (*statTxtTbl > 0x31Bu)
				v3 = (char*)statTxtTbl + statTxtTbl[796];
			else
				v3 = "";
			v7 = (u16)(*active_portrait_unit)->building.resource.resource_amount;
LABEL_14:
			sprintf_s(struct_Char->buf, 260, "%s %d", v3, v7);
			v8 = struct_Char;
			goto LABEL_19;
		}
		v4 = (*active_portrait_unit)->building.resource.resource_amount;
		if (v4)
		{
			if (*statTxtTbl > 0x31Cu)
				v3 = (char*)statTxtTbl + statTxtTbl[797];
			else
				v3 = "";
			v7 = v4;
			goto LABEL_14;
		}
		if (*statTxtTbl > 0x31Du)
			v5 = (char*)statTxtTbl + statTxtTbl[798];
		else
			v5 = "";
		v8 = (BufferStr*)v5;
LABEL_19:
		helper::add_text_to_dialog(v1, -11, (const char*)v8);
		if (units_dat::ShieldsEnabled[(*active_portrait_unit)->id])
		{
			helper::draw_shield_upgrade(v1, 0);
		}
		else
		{
			if (v1->control_type)
				v1 = *(BinDlg**)&v1->parent;
			v6 = *(BinDlg**)&v1->children_dialog;
			if (v6)
			{
				while (v6->index != 9)
				{
					v6 = v6->next;
					if (!v6)
						goto LABEL_26;
				}
			}
			else
			{
LABEL_26:
				v6 = 0;
			}
			helper::hide_dialog(v6);
		}
	}

	void draw_supply_unit_info(s32 a1)
	{ // 00427540
		BinDlg* v1; // ebx
		Unit* v2; // esi
		GroupFlag v3; // al
		u8 v4; // cl
		s32 v5; // edx
		s16 v6; // ax
		const char* v7; // ecx
		s16 v8; // ax
		const char* v9; // esi
		u16 v10; // ax
		s32 v11; // ecx
		const char* v12; // esi
		s32 v13; // eax
		GroupFlag v14; // al
		s32 v15; // ecx
		s16 v16; // ax
		const char* v17; // eax
		u16 v18; // ax
		BinDlg* v19; // eax
		s32 v20; // [esp+0h] [ebp-8h]
		u8 v21; // [esp+7h] [ebp-1h]
		u8 v22; // [esp+7h] [ebp-1h]

		v1 = (BinDlg*)a1;
		v2 = *active_portrait_unit;
		if (*bw_is_in_replay || (*active_portrait_unit)->player == *LOCAL_NATION_ID)
		{
			helper::get_supplies_used_no_args();
			if (*statusScreenFunc != 12)
			{
				helper::hide_optional_status_screen_controls(v1);
				helper::show_control_group(v1, -10, -13);
				v2 = *active_portrait_unit;
				*statusScreenFunc = 12;
			}
			v3 = units_dat::group_flags[(u16)v2->id];
			if (v3.is_zerg)
			{
				v21 = 0;
			}
			else if (v3.is_protoss)
			{
				v21 = 2;
			}
			else
			{
				v21 = (v3.is_terran) != 0 ? RaceId::Terran : RaceId::Neutral;
			}
			v4 = v2->player;
			if (v21)
			{ // !RACE_Zerg
				if (v21 == RaceId::Terran)
				{
					v5 = raceSupply[RaceId::Terran].used[v4];
				}
				else if (v21 == RaceId::Protoss)
				{
					v5 = raceSupply[RaceId::Protoss].used[v4];
				}
				else
				{
					v5 = 0;
				}
			}
			else
			{
				v5 = raceSupply[RaceId::Zerg].used[v4];
			}
			ByteOperations::SetLowWord(v20, v21);
			v6 = v21 + 820;
			if (v21 == -821)
			{
				v7 = 0;
			}
			else if (v6 < *statTxtTbl)
			{
				v7 = (char*)statTxtTbl + statTxtTbl[v6 + 1];
			}
			else
			{
				v7 = "";
			}
			sprintf_s(struct_Char2->buf, 260, "%s %d", v7, (v5 + 1) / 2);
			v8 = v21 + 814;
			if (v21 == -815)
			{
				v9 = 0;
			}
			else if (v8 < *statTxtTbl)
			{
				v9 = (char*)statTxtTbl + statTxtTbl[v8 + 1];
			}
			else
			{
				v9 = "";
			}
			v10 = helper::get_last_queue_slot_id((*active_portrait_unit));
			sprintf_s("", 260, "%s %d", v9, (units_dat::SupplyProvided[v10] + 1) / 2);
			v11 = v20 + 66353;
			if (v21 == -818)
			{
				v12 = 0;
			}
			else if ((u16)v11 < *statTxtTbl)
			{
				v11 = (u16)v11;
				v12 = (char*)statTxtTbl + statTxtTbl[(u16)v11 + 1];
			}
			else
			{
				v12 = "";
			}
			ByteOperations::SetLowByte(v11, v21);
			v13 = helper::get_supplies_available((*active_portrait_unit)->player, v11);
			sprintf_s(unkString_Char, 260, "%s %d", v12, (v13 + 1) / 2);
			v22 = (*active_portrait_unit)->player;
			v14 = units_dat::group_flags[helper::get_last_queue_slot_id((*active_portrait_unit))];
			s32 race = (*active_portrait_unit)->get_race();
			if (v14.is_zerg)
			{
				v15 = raceSupply[RaceId::Zerg].max[v22];
			}
			else if (v14.is_terran)
			{
				v15 = raceSupply[RaceId::Terran].max[v22];
			}
			else if (v14.is_protoss)
			{
				v15 = raceSupply[RaceId::Protoss].max[v22];
			}
			else
			{
				v15 = 0;
			}
			v16 = v20 + 823;
			if ((s16)v20 == -824)
			{
				v17 = 0;
			}
			else if (v16 < *statTxtTbl)
			{
				v17 = (char*)statTxtTbl + statTxtTbl[v16 + 1];
			}
			else
			{
				v17 = "";
			}
			sprintf_s(struct_Char2->buf, 260, "%s %d", v17, (u32)(v15 + 1) >> 1);
			helper::add_text_to_dialog(v1, -10, (const char*)struct_Char);
			helper::add_text_to_dialog(v1, -11, "");
			helper::add_text_to_dialog(v1, -12, (const char*)unkString_Char);
			helper::add_text_to_dialog(v1, -13, (const char*)struct_Char2);
			v18 = (*active_portrait_unit)->id;
			if (units_dat::ShieldsEnabled[v18])
			{
				helper::draw_shield_upgrade(v1, 0);
			}
			else if (v18 != UnitId::ZergOvileth && v18 != UnitId::Hero_Yggdrasill)
			{
				if (v1->control_type)
					v1 = *(BinDlg**)&v1->parent;
				v19 = *(BinDlg**)&v1->children_dialog;
				if (v19)
				{
					while (v19->index != 9)
					{
						v19 = v19->next;
						if (!v19)
							goto LABEL_55;
					}
				}
				else
				{
LABEL_55:
					v19 = 0;
				}
				helper::hide_dialog(v19);
			}
			else
			{
				helper::drawArmor(v1, v18, 0);
			}
		}
		else
		{
			helper::unit_stat_act_standard((void*)a1);
		}
	}

	bool AI_TargetEnemyProc(Unit* a1, Unit* a2)
	{ // 00440A20
		s8 v2; // dl
		s8 v3; // bl
		s32 v4; // eax
		s8 result; // eax

		result = 0;
		if (a1 != a2)
		{
			v2 = a2->player;
			v3 = a1->player == 11 ? a1->sprite->player : a1->player;
			if (!scbw::is_allied_to(v2, v3))
			{
				ByteOperations::SetLowByte(v4, helper::ai_unit_can_attack(a1));
				if (v4)
					result = 1;
			}
		}
		return result;
	}

	s32 calcUnitStrength(s32 a1, u8 a2)
	{ // 00431270
		s32 v2; // ecx
		unsigned _int64 v3; // rax
		s32 v4; // ebx
		u32 v5; // esi
		s32 v6; // edi
		s32 v7; // eax
		s32 totalHits; // [esp+0h] [ebp-4h]

		if (units_dat::ShieldsEnabled[a1])
		{
			v2 = units_dat::MaxShieldPoints[a1];
		}
		else
		{
			v2 = 0;
		}

		totalHits = v2 + (units_dat::MaxHitPoints[a1] >> 8);
		if (totalHits)
		{
			v4 = (u8)weapons_dat::DamageFactor[a2];
			v5 = weapons_dat::Cooldown[a2];
			v6 = weapons_dat::DamageAmount[a2];
			if (a2 == -126)
			{
				v7 = 0;
			}
			else
			{
				v7 = (v6 * v4 << 11) / (s32)v5;
			}
			v3 = (unsigned _int64)(sqrt((f64)(v6 * v4 * (weapons_dat::MaxRange[a2] / v5)
				+ ((u32)(totalHits * v7) >> 8)))
				* 7.58);
		}
		else
		{
			v3 = 0;
		}
		return v3;
	}

	u32 getUnitStrengthHook(Unit* a1, s32 a2, bool a3)
	{ // 00431800
		Unit* v3; // edi
		s32 v4; // esi
		s32 v5; // ebx
		s32 v7; // ebx
		s32 v8; // esi
		u32 v9; // ecx
		s32 groundAir; // [esp+10h] [ebp+8h]

		v3 = a1;
		v4 = (u16)a1->id;
		if (v4 == UnitId::ZergLarva || v4 == UnitId::ZergEgg || v4 == UnitId::ZergCocoon)
			return 0;
		if (a1->status & UnitStatus::IsHallucination)
		{
			v5 = get_total_hp_plus_shields(a1);
			if (helper::get_current_hp_plus_shields(v3) < v5)
				return 0;
		}
		v7 = helper::get_base_strength(a3, v3);
		groundAir = helper::get_current_hp_plus_shields(v3);
		if (units_dat::ShieldsEnabled[v4])
			v8 = units_dat::MaxShieldPoints[v4];
		else
			v8 = 0;
		v9 = v8 + helper::get_max_hp(v3);
		if (!v9)
			v9 = 1;
		return v7 * groundAir / v9;
	}

	bool AI_RestorationRequirementsProc(Unit* a1, Unit* a2)
	{ // 004407E0
		Unit* v2; // edx
		s32 v3; // eax
		bool result; // eax

		result = 0;
		if (a1->player == a2->player)
		{
			if (helper::can_weapon_target_unit(WeaponId::Restoration, a1, a2))
			{
				v3 = units_dat::ShieldsEnabled[(u16)v2->id] ? v2->shields >> 8 : 0;
				if (v3 + ((v2->hitpoints + 255) >> 8) > 60
					&& scbw::is_debuffed(v2))
				{
					result = 1;
				}
			}
		}
		return result;
	}

	bool AI_EMPShieldRequirementsProc(Unit* a1, Unit* a2)
	{ // 004413D0
		Unit* v2; // esi
		Unit* v3; // edi
		Sprite* v5; // edx
		s16 v6; // cx
		s16 spriteX;

		v2 = a1;
		v3 = a2;
		if (!helper::ai_target_unit_is_worth_hitting(a1, a2)
			|| !units_dat::ShieldsEnabled[(u16)v2->id]
			|| !helper::can_weapon_target_unit(WeaponId::EMP_Shockwave, v2, v3))
		{
			return 0;
		}
		v5 = v2->sprite;
		v6 = v5->position.y;
		spriteX = v5->position.x;
		*tempHPShields = 0;
		*tempWeapon = WeaponId::EMP_Shockwave;
		helper::create_splash_weapon_proc(
			160,
			spriteX,
			v6,
			(s32(__fastcall*)(s32, s32))helper::unit_shields_clump_in_area_accumulator_proc,
			(s32)v3);
		return *tempHPShields >= 200;
	}

	char attackOverlayAndNotify(Unit* a1, Unit* a2, u8 a3, char a4)
	{ // 00479730
		s32* v4; // eax
		Unit* v5; // ebx
		char returnValue = 0;

		v5 = a1;
		v4 = &a1->hitpoints;
		if (v4)
		{
			if (!(v5->status & UnitStatus::Invincible))
			{
				if (a2)
				{
					helper::unit_was_hit(v5, a2, a3 != WeaponId::Irradiate);
					if (a3 != WeaponId::Irradiate && v5->player != a2->player)
						helper::unit_attack_notify(v5);
				}
				if (units_dat::ShieldsEnabled[(u16)v5->id])
				{
					if (v5->shields >= 256)
					{
						*v4 = a3;
						if (weapons_dat::DamageType[a3])
						{
							returnValue = (char)helper::create_shield_overlay(a4, v5);
						}
					}
				}
			}
		}
		return returnValue;
	}

	void drawImageHPBar(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, s32 colorData)
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

		v37 = screenX;
		v5 = colorData;
		v6 = *(u16*)(colorData + 100);
		v33 = grpRect->left;
		v34 = grpRect->top;
		v7 = units_dat::ShieldsEnabled[v6];
		v38 = screenY;
		v8 = grpRect->bottom;
		v35 = grpRect->right;
		if (v7)
		{
			v36 = 7;
			if (v8 <= 7)
				v36 = v8;
			v13 = sprites_dat::IsVisible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390];
			v14 = units_dat::MaxHitPoints[v6] >> 8;
			ByteOperations::SetHighByte(colorData, sprites_dat::IsVisible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390]);
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
			helper::unk_billions2(*(s32*)(v5 + 96) >> 8, units_dat::MaxShieldPoints[v6], (s32)&colorData, HIBYTE(colorData));
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
			if (units_dat::MaxHitPoints[v6] >> 8)
			{
				v9 = units_dat::MaxHitPoints[v6] >> 8;
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
				(u8)sprites_dat::IsVisible[*(u16*)(*(s32*)(colorData + 12) + 8) + 390]);
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
			if (*(u8*)(v5 + 76) == *LOCAL_NATION_ID)
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
						(u8)sprites_dat::IsVisible[*(u16*)(*(s32*)(v5 + 12) + 8) + 390]);
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

	void setBuildShieldGain(Unit* a1)
	{ // 0047B6A0
		s32 _type; // edi
		u16 v2; // dx
		s32 v3; // ecx
		u32 v4; // edx
		u32 v5; // eax

		_type = (u16)a1->id;
		if (units_dat::ShieldsEnabled[_type])
		{
			if (a1->status & UnitStatus::GroundedBuilding)
			{
				v4 = (s32)((unsigned __int64)(1717986919i64 * (units_dat::MaxShieldPoints[_type] << 8)) >> 0x20) >> 2;
				v3 = v4 + (v4 >> 31);
				a1->shields = v3;
				v2 = units_dat::time_cost[_type];
				if (!v2
					|| (v5 = (((u32)units_dat::MaxShieldPoints[_type] << 8) - v3) / v2, (a1->build_repair_hp_gain = v5) == 0))
				{
					a1->build_repair_hp_gain = 1;
				}
			}
		}
	}

	s32 getUnitBulletDamage(Unit* victim, Bullet* bullet)
	{ // 0048ACD0
		s32 v2; // edi
		Unit* v3; // esi
		s32 v4; // eax
		s32 v5; // eax
		unsigned __int16 v6; // cx
		s32 v7; // esi
		s8 v8; // cl
		s32 result; // eax
		s32 v10; // eax
		s32 unit_id; // ecx
		s32 v12; // eax
		s32 v13; // ecx
		s32 v14; // eax
		s32 v15; // eax

		v2 = (u8)bullet->weapon_id;
		v3 = victim;
		if (weapons_dat::ExplosionType[v2] == WeaponEffect::NuclearMissile)
		{
			unit_id = (u16)victim->id;
			v10 = units_dat::MaxHitPoints[unit_id];
			if (units_dat::ShieldsEnabled[unit_id])
			{
				v12 = v10 >> 8;
				if (!v12)
				{
					v12 = (v3->hitpoints + 255) >> 8;
					if (!v12)
						v12 = 1;
				}
				v13 = 2 * (v12 + units_dat::MaxShieldPoints[unit_id]);
			}
			else
			{
				v14 = v10 >> 8;
				if (!v14)
				{
					v14 = (v3->hitpoints + 255) >> 8;
					if (!v14)
						v14 = 1;
				}
				v13 = 2 * v14;
			}
			v15 = v13 / 3;
			if (v13 / 3 < 500)
				v15 = 500;
			result = v15 << 8;
		}
		else
		{
			v5 = (u8)bullet->weapon_id;
			v6 = weapons_dat::DamageUpgrade[v5];
			v7 = weapons_dat::DamageAmount[v5];
			v4 = (u8)bullet->srcPlayer;
			if (v6 >= (u16)BwUpgrade::UnusedUpgrade46)
				/*v8 = upgradesLevelBW[v4][v6];
			else
				v8 = upgradesLevelSC[v4][v6];*/
				result = (v7 + v8 * weapons_dat::DamageBonus[v2]) << 8;
		}
		return result;
	}

	s32 canCastSpell(Unit* a1)
	{ // 00492140
		u8 v1; // al
		s32 v2; // ebx
		Unit* v3; // esi
		s32 result; // eax
		s32 v5; // eax
		s32 v6; // eax
		bool v7; // eax
		s16 v8; // [esp+8h] [ebp-Ch]
		s16 v9; // [esp+Ch] [ebp-8h]
		u8 v10; // [esp+13h] [ebp-1h]

		v1 = a1->main_order_id;
		v2 = v1;
		v10 = orders_dat::OrderWeaponId[v1];
		v8 = a1->order_target.pt.y;
		v3 = a1->order_target.unit;
		v9 = a1->order_target.pt.x;
		if (a1->unit_ai)
		{
			switch (v1)
			{
				case OrderId::MagnaPulse:
				case OrderId::StasisField:
					if (v3 && !helper::unitIsFrozen(v3))
						goto LABEL_23;
					goto LABEL_17;
				case OrderId::CastParasite:
					if (v3 && !v3->parasite_flags)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::EmpShockwave:
					if (!v3)
						goto LABEL_23;
					ByteOperations::SetLowByte(v5, helper::isUnitCasterNotHallucination(v3));
					if (v5)
					{
						if (HIBYTE(v3->energy) >= 0x64u)
							goto LABEL_23;
					}
					if (units_dat::ShieldsEnabled[(u16)v3->id] && (s32)(v3->shields & 0xFFFFFF00) >= 25600)
						goto LABEL_23;
LABEL_17:
					result = 1;
					break;
				case OrderId::Irradiate:
					if (v3 && !v3->irradiate_timer)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::Plague:
					if (v3 && !v3->plague_timer)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::Ensnare:
					if (!v3 || !v3->ensnare_timer)
						goto LABEL_23;
					result = 1;
					break;
				default:
					goto LABEL_23;
			}
		}
		else
		{
LABEL_23:
			if (orders_dat::UseWeaponTargeting[v2])
			{
				if (!helper::can_weapon_target_unit(v10, v3, a1))
					return 1;
			}
			else
			{
				ByteOperations::SetLowByte(v6, orders_dat::TechUsed[v2]);
				if ((u8)v6 < (u8)TechId::None)
				{
					v6 = (u8)v6;
					switch ((u8)v6)
					{
						case TechId::EMPShockwave:
						case TechId::ScannerSweep:
						case TechId::DarkSwarm:
						case TechId::Plague:
						case TechId::Ensnare:
						case TechId::PsionicStorm:
						case TechId::Recall:
						case TechId::StasisField:
						case TechId::DisruptionWeb:
						case TechId::Maelstrom:
							break;
						default:
							if ((u8)v6 == TechId::SpiderMines)
							{
								ByteOperations::SetLowByte(v7, helper::getActiveTileFlag(v8, v9, 0x410000, 0));
							}
							else
							{
								if (!v3 || v3->status & 0x4000000)
									goto LABEL_17;
								v7 = (u16)helper::get_target_unit_ability_error_stat_string(v3, a1->player, v6) == 0;
							}
							if (!v7)
								goto LABEL_17;
							break;
					}
				}
			}
			result = 0;
		}
		return result;
	}

	s32 modifyUnitShields(s32 a1, s32 a2)
	{ // 004C5A20
		s32 v3; // eax
		s32 v4; // ecx

		if (!a1)
		{
			helper::SErrSetLastError(87);
			return 0;
		}
		v3 = *(u16*)(a1 + 100);
		if (!units_dat::ShieldsEnabled[v3])
			return 0;
		v4 = units_dat::MaxShieldPoints[v3];
		if (v4 >= (s32)(a2 * v4 / 0x64u))
			v4 = a2 * (u32)units_dat::MaxShieldPoints[v3] / 0x64;
		*(s32*)(a1 + 96) = v4 << 8;
		return 1;
	}

	void compileHealthBar(s32 a1, s32 a2)
	{ // 004D6010
		s32 v2; // eax
		Unit* v3; // edi
		u8 v4; // cl
		s32 v5; // eax
		u16 v6; // ax
		s32 v7; // eax
		char v8; // cl
		char v9; // cl
		s32* v10; // edx

		v2 = *(u16*)(a2 + 8);
		v3 = active_player_selection->unit[*(u8*)(a2 + 11)];
		v4 = sprites_dat::IsVisible[v2 + 390];
		*(s8*)(a1 + 15) = sprites_dat::HpBarSize[v2 + 262]
			+ (imageGrpGraphics[(u8)orders_dat::Unused3[v2 + 62] + 561]->frames[0].height >> 1)
			+ 8;
		*(s8*)(*(s32*)(a1 + 44) + 6) = 0;
		*(s8*)(*(s32*)(a1 + 44) + 7) = 0;
		*(s8*)(*(s32*)(a1 + 44) + 8) = v4 - (v4 - 1) % 3;
		v5 = *(s32*)(a1 + 44);
		if (*(s8*)(v5 + 8) < 0x13u)
			*(s8*)(v5 + 8) = 19;
		*(s8*)(*(s32*)(a1 + 44) + 9) = 0;
		v6 = v3->id;
		if (units_dat::base_property[v6] & 0x200000 && !(v3->status & 0x40000000)
			|| v3->status & 0x40000000
			|| v6 == 40)
		{
			*(s8*)(*(s32*)(a1 + 44) + 9) += 6;
		}
		v7 = *(s32*)(a1 + 44);
		v8 = *(s8*)(v7 + 9);
		if (units_dat::ShieldsEnabled[(unsigned __int16)v3->id])
			v9 = v8 + 7;
		else
			v9 = v8 + 5;
		*(s8*)(v7 + 9) = v9;
		*(s16*)(*(s32*)(a1 + 44) + 2) = *(unsigned __int8*)(*(s32*)(a1 + 44) + 8);
		*(s16*)(*(s32*)(a1 + 44) + 4) = *(unsigned __int8*)(*(s32*)(a1 + 44) + 9);
		*(s32*)(a1 + 60) = a2;
		*(s16*)(a1 + 12) = 0;
		v10 = (s32*)(a1 + 36);
		*v10 = 0;
		v10[1] = 0;
		// *(s32*)(a1 + 48) = v3;
		helper::updateImageDrawData((Image*)a1);
		*(s8*)(a1 + 12) |= 1u;
		helper::play_image_iscript((Image*)a1, 0);
	}

	s32 AI_FindTransport(Unit* a1, s32 a2)
	{ // 004E6EF0
		Unit* v2; // esi
		s32 v3; // ecx
		u16 v4; // ax
		s32 v5; // edi
		s32 v6; // eax
		char v7; // al
		s16 v8; // ST16_2
		u32 v9; // eax
		s32 v11; // [esp+8h] [ebp-Ch]
		u32 v12; // [esp+Ch] [ebp-8h]

		*(s32*)a2 = 0;
		v2 = first_player_unit->unit[(u8)a1->player];
		v11 = 0;
		v12 = 99999999;
		if (!v2)
			return v11;
		while (1)
		{
			v3 = v2->status;
			if (v3 & 0x40000000)
				goto LABEL_22;
			v4 = v2->id;
			//			if (v4 == 42 && !upgradesLevelSC[(u8)v2->player][24])
			goto LABEL_22;
			v5 = v4;
			if (!units_dat::SpaceProvided[v4] || !(v3 & 1) || !(v3 & 0x20000))
				goto LABEL_22;
			if (v2->order_target.unit == a1)
				break;
			v6 = units_dat::MaxHitPoints[v4] >> 8;
			if (!v6)
			{
				v6 = (v2->hitpoints + 255) >> 8;
				if (!v6)
					v6 = 1;
			}
			if ((v2->hitpoints + 255) >> 8 >= 3 * v6 / 4 || units_dat::ShieldsEnabled[v5] && v2->shields >> 8 == units_dat::MaxShieldPoints[v5])
			{
				v11 = 1;
				if (helper::can_enter_transport(v2, a1))
				{
					v7 = v2->main_order_id;
					if (v7 != OrderId::Pickup2 && v7 != OrderId::Unload)
					{
						if (v7 != OrderId::MoveUnload
							|| (v8 = helper::get_region_id(v2->order_target.pt.x, v2->order_target.pt.y),
								v8 == helper::get_region_id(a1->order_target.pt.x, a1->order_target.pt.y)))
						{
							v9 = helper::getUnitDistanceToHalt(v2->sprite->position.y, a1, v2->sprite->position.x);
							if (v9 < v12)
							{
								//								*(s32*)a2 = v2;
								v12 = v9;
							}
						}
					}
				}
			}
LABEL_22:
			v2 = v2->player_link.next;
			if (!v2)
				return v11;
		}
		//		*(s32*)a2 = v2;
		return 1;
	}

	s32 mapDataTransfer(char a1, s32 a2, s32 a3)
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