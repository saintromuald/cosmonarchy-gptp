#include "maxhp.h"
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "hook_tools.h"
#include "byte_operations_utils.h"

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
	Line 58440: 004279F3   .  8A83 B0476600 MOV AL,BYTE PTR DS:[EBX+6647B0] -- 0x00427890 -- drawBuildingInfo -- HOOKED
	Line 72970: 00431274  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00431270 -- calcUnitStrength -- HOOKED
	Line 73606: 00431865  |.  8A86 B0476600 MOV AL,BYTE PTR DS:[ESI+6647B0] -- 0x00431800 -- getUnitStrength -- HOOKED
	Line 95686: 00440800   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004407E0 -- AI_RestorationRequirementsProc -- HOOKED

	// Not hooked yet
	Line 96913: 004413E7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004413D0 -- AI_EMPShieldRequirementsProc
	Line 180864: 00479777  |.  8A82 B0476600 MOV AL,BYTE PTR DS:[EDX+6647B0] -- 0x00479730 -- HallucinationHit
	Line 182607: 0047A841  |.  8A8F B0476600 MOV CL,BYTE PTR DS:[EDI+6647B0] -- 0x0047A820 -- DrawImage_HpBar
	Line 184040: 0047B6A5  |.  8A87 B0476600 MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x0047B6A0 -- SetBuildShieldGain
	Line 206949: 0048AD35  |.  8A81 B0476600 MOV AL,BYTE PTR DS:[ECX+6647B0] -- 0x0048ACD0 -- GetWeaponDamage
	Line 217770: 00492239  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00492140 -- ShouldStopOrderedSpell
	Line 293899: 004C5A3D  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004C5A20 -- ModifyUnitShields
	Line 318718: 004D60BC  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004D6010 -- InitHealthBarImage
	Line 343282: 004E6FC6  |.  8A87 B0476600 |MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x004E6EF0 -- Ai_FindTransport
	Line 349375: 004EB0F4   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x004EAFE0 -- Command_MapData
*/

namespace {

	// For drawUnitInfo
	void hideOptionalStatusScreenControls(BinDlg* a1);					// 00457310
	void showControlGroup(BinDlg* a1, s16 a2, s16 a3);					// 00457250
	void addTextToDialog(int a1, s16 a2, int a3);						// 004258B0
	void drawShieldUpgrade(int a1, u16 a2);								// 00425510
	void hideDialog(BinDlg* a1);										// 00418700

	// For drawSupplyUnitInfo
	int getSuppliesUsed();												// 00424732
	u16 getLastQueueSlotType(CUnit* a1);								// 0047B270
	u32 getSuppliesAvailable(u8 a1, int a2);							// 00488900
	void drawArmor(int a1, int a2, u16 a3);								// 00425600
	void unitStatAct_Standard(void* a1);								// 00426F50

	// For AI_TargetEnemyProc
	bool AI_UnitCanAttack(CUnit* a1);									// 00476180

	// For drawBuildingInfo
	int needRedrawUI();													// 00424A10
	CUnit* genericStatusUpdateDrawnValues();							// 004248F0
	void drawProgressBar(int a1);										// 004260C0
	void drawNukeSiloInfo(int a1);										// 00426FF0
	bool unitIsActiveTransport(CUnit* a1);								// 004E6BA0
	bool isQueueSlotActive(CUnit* a1, int a2);							// 00401E70
	void drawTrainingProgress(BinDlg* a1);								// 004268D0
	bool isConstructingAddon(CUnit* a1);								// 004E66B0
	bool isAttemptingProtossBuild(CUnit* a1);							// 004E4C40
	void drawAddonProgress(int a1);										// 00425F30
	void drawTechProgress(int a1);										// 004266F0
	void drawUpgradeProgress(BinDlg* a1);								// 00426500
	bool unitIsGeyser(CUnit* a1);										// 004688B0
	void drawGasQuantity(BinDlg* a1);									// 00426190
	s8 unitGetGroundWeapon(CUnit* a1);									// 00475AD0
	void drawDisplayButtons(int a1);									// 00426C60
	BinDlg* getControlFromIndex(BinDlg* a1, s16 a2);					// 00418080
	void setTextStr(int a1);											// 004263E0
	void setUnitStatusStrText(int a1);									// 00425B50
	int unitGetName(CUnit* a1);											// 0047B5A0

	// For getUnitStrength
	int getCurrentHitPointsPlusShields(CUnit* a1);						// 004026D0
	int getBaseStrength(int a1, CUnit* a2);								// 004316D0
	int getMaxHP(CUnit* a1);											// 00401400

	// For AI_RestorationRequirementsProc
	int canWeaponTargetUnit(u8 a1, CUnit* a2, CUnit* a3);				// 00475CE0
}

namespace hooks {
	u32 maxhp(int id, int playerId) {
		u32 result = units_dat::MaxHitPoints[id];
		//if (id==UnitId::firebat && scbw::getUpgradeLevel(playerId, UpgradeId::Heatshield)) {
		//	result += 15 * 256;
		//}
		return result;
	}

	u32 maxshields(int id, int playerId) {
		u32 result = units_dat::MaxShieldPoints[id];
		//if (units_dat::BaseProperty[id] & UnitProperty::RoboticUnit && scbw::getUpgradeLevel(playerId, UpgradeId::Autovitality)) {
		//	result += result / 2;
		//}
		return result;
	}

	u32 getTotalHitPointsPlusShields(CUnit* a1) { // 00402710
		int v1; // eax
		int v2; // ecx
		int v3; // eax

		v1 = (u16)a1->id;
		if (units_dat::ShieldsEnabled[v1])
			v2 = units_dat::MaxShieldPoints[v1];
		else
			v2 = 0;
		v3 = units_dat::MaxHitPoints[v1] >> 8;
		if (!v3) {
			v3 = (a1->hitPoints + 255) >> 8;
			if (!v3)
				v3 = 1;
		}
		return v2 + v3;
	}

	void drawUnitInfo(BinDlg* a1) { // 00426190
		BinDlg* v1; // ebx
		s16 v2; // ax
		const char* v3; // eax
		u16 v4; // cx
		const char* v5; // eax
		BinDlg* v6; // eax
		int v7; // [esp-8h] [ebp-10h]
		BufferStr* v8; // [esp-8h] [ebp-10h]

		v1 = a1;
		*suppliesAvailableTemp = (*activePortraitUnit)->building.resource.resourceAmount;
		if (*statusScreenFunc != 15) {
			hideOptionalStatusScreenControls(a1);
			showControlGroup(v1, -10, -13);
			*statusScreenFunc = 15;
		}
		addTextToDialog((int)v1, -10, 0);
		addTextToDialog((int)v1, -12, 0);
		addTextToDialog((int)v1, -13, 0);
		v2 = (*activePortraitUnit)->id;
		if ((u16)v2 >= 0xB0u && (u16)v2 <= 0xB2u) {
			if (*statTxtTbl > 0x31Bu)
				v3 = (char*)statTxtTbl + statTxtTbl[796];
			else
				v3 = "";
			v7 = (u16)(*activePortraitUnit)->building.resource.resourceAmount;
		LABEL_14:
			sprintf_s(struct_Char->buf, 260, "%s %d", v3, v7);
			v8 = struct_Char;
			goto LABEL_19;
		}
		v4 = (*activePortraitUnit)->building.resource.resourceAmount;
		if (v4) {
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
		addTextToDialog((int)v1, -11, (int)v8);
		if (units_dat::ShieldsEnabled[(*activePortraitUnit)->id]) {
			drawShieldUpgrade((int)v1, 0);
		}
		else {
			if (v1->controlType)
				v1 = *(BinDlg**)&v1->parent;
			v6 = *(BinDlg**)&v1->childrenDlg;
			if (v6)	{
				while (v6->index != 9)	{
					v6 = v6->next;
					if (!v6)
						goto LABEL_26;
				}
			}
			else {
			LABEL_26:
				v6 = 0;
			}
			hideDialog(v6);
		}
	}

	void drawSupplyUnitInfo(int a1) { // 00427540
		BinDlg* v1; // ebx
		CUnit* v2; // esi
		GroupFlag v3; // al
		u8 v4; // cl
		int v5; // edx
		s16 v6; // ax
		const char* v7; // ecx
		s16 v8; // ax
		const char* v9; // esi
		u16 v10; // ax
		int v11; // ecx
		const char* v12; // esi
		int v13; // eax
		GroupFlag v14; // al
		int v15; // ecx
		s16 v16; // ax
		const char* v17; // eax
		u16 v18; // ax
		BinDlg* v19; // eax
		int v20; // [esp+0h] [ebp-8h]
		u8 v21; // [esp+7h] [ebp-1h]
		u8 v22; // [esp+7h] [ebp-1h]

		v1 = (BinDlg*)a1;
		v2 = *activePortraitUnit;
		if (IS_IN_REPLAY || (u8)(*activePortraitUnit)->playerId == *LOCAL_NATION_ID) {
			getSuppliesUsed();
			if (*statusScreenFunc != 12) {
				hideOptionalStatusScreenControls(v1);
				showControlGroup(v1, -10, -13);
				v2 = *activePortraitUnit;
				*statusScreenFunc = 12;
			}
			v3 = units_dat::GroupFlags[(u16)v2->id];
			if (v3.isZerg) {
				v21 = 0;
			}
			else if (v3.isProtoss) {
				v21 = 2;
			}
			else {
				v21 = (v3.isTerran) != 0 ? RaceId::Terran : RaceId::Neutral;
			}
			v4 = v2->playerId;
			if (v21) { // !RACE_Zerg
				if (v21 == RaceId::Terran) {
					v5 = raceSupply[RaceId::Terran].used[v4];
				}
				else if (v21 == RaceId::Protoss) {
					v5 = raceSupply[RaceId::Protoss].used[v4];
				}
				else {
					v5 = 0;
				}
			}
			else {
				v5 = raceSupply[RaceId::Zerg].used[v4];
			}
			ByteOperations::SetLowWord(v20, v21);
			v6 = v21 + 820;
			if (v21 == -821) {
				v7 = 0;
			}
			else if (v6 < *statTxtTbl) {
				v7 = (char*)statTxtTbl + statTxtTbl[v6 + 1];
			}
			else {
				v7 = "";
			}
			sprintf_s(struct_Char2->buf, 260, "%s %d", v7, (v5 + 1) / 2);
			v8 = v21 + 814;
			if (v21 == -815) {
				v9 = 0;
			}
			else if (v8 < *statTxtTbl) {
				v9 = (char*)statTxtTbl + statTxtTbl[v8 + 1];
			}
			else {
				v9 = "";
			}
			v10 = getLastQueueSlotType((*activePortraitUnit));
			sprintf_s("", 260, "%s %d", v9, (units_dat::SupplyProvided[v10] + 1) / 2);
			v11 = v20 + 66353;
			if (v21 == -818) {
				v12 = 0;
			}
			else if ((u16)v11 < *statTxtTbl) {
				v11 = (u16)v11;
				v12 = (char*)statTxtTbl + statTxtTbl[(u16)v11 + 1];
			}
			else {
				v12 = "";
			}
			ByteOperations::SetLowByte(v11, v21);
			v13 = getSuppliesAvailable((*activePortraitUnit)->playerId, v11);
			sprintf_s(unkString_Char, 260, "%s %d", v12, (v13 + 1) / 2);
			v22 = (*activePortraitUnit)->playerId;
			v14 = units_dat::GroupFlags[getLastQueueSlotType((*activePortraitUnit))];
			int race = (*activePortraitUnit)->getRace();
			if (v14.isZerg) {
				v15 = raceSupply[RaceId::Zerg].max[v22];
			}
			else if (v14.isTerran) {
				v15 = raceSupply[RaceId::Terran].max[v22];
			}
			else if (v14.isProtoss) {
				v15 = raceSupply[RaceId::Protoss].max[v22];
			}
			else {
				v15 = 0;
			}
			v16 = v20 + 823;
			if ((s16)v20 == -824) {
				v17 = 0;
			}
			else if (v16 < *statTxtTbl) {
				v17 = (char*)statTxtTbl + statTxtTbl[v16 + 1];
			}
			else {
				v17 = "";
			}
			sprintf_s(struct_Char2->buf, 260, "%s %d", v17, (unsigned int)(v15 + 1) >> 1);
			addTextToDialog((int)v1, -10, (int)struct_Char);
			addTextToDialog((int)v1, -11, (int)"");
			addTextToDialog((int)v1, -12, (int)unkString_Char);
			addTextToDialog((int)v1, -13, (int)struct_Char2);
			v18 = (*activePortraitUnit)->id;
			if (units_dat::ShieldsEnabled[v18]) {
				drawShieldUpgrade((int)v1, 0);
			}
			else if (v18 != UnitId::ZergOverlord && v18 != UnitId::Hero_Yggdrasill) {
				if (v1->controlType)
					v1 = *(BinDlg**)&v1->parent;
				v19 = *(BinDlg**)&v1->childrenDlg;
				if (v19) {
					while (v19->index != 9)
					{
						v19 = v19->next;
						if (!v19)
							goto LABEL_55;
					}
				}
				else {
				LABEL_55:
					v19 = 0;
				}
				hideDialog(v19);
			}
			else {
				drawArmor((int)v1, v18, 0);
			}
		}
		else {
			unitStatAct_Standard((void*)a1);
		}
	}

	char drawWireframeShield(int a1, CUnit* a2) { // 004566B0
		CUnit* v2; // edx
		u16 v3; // ax
		char result; // al
		CUnit* v5; // edx
		int v6; // eax
		char v7; // cl

		if (units_dat::ShieldsEnabled[(u16)a2->id] && units_dat::MaxShieldPoints[getLastQueueSlotType(a2)]) {
			v3 = getLastQueueSlotType(v2);
			v6 = 7 * (v5->shields >> 8) / units_dat::MaxShieldPoints[v3];
			v7 = charBuffer[(u8)wireframeConstants[2 * v6]];
			result = charBuffer[(u8)wireframeConstants[2 * v6 + 1]];
			imgDrawPaletteIndex[192] = v7;
			imgDrawPaletteIndex[193] = result;
		}
		else {
			result = byteShieldStr[0];
			imgDrawPaletteIndex[192] = byteShieldStr[0];
			imgDrawPaletteIndex[193] = byteShieldStr[0];
		}
		return result;
	}

	bool AI_TargetEnemyProc(CUnit* a1, CUnit* a2) { // 00440A20
		s8 v2; // dl
		s8 v3; // bl
		int v4; // eax
		s8 result; // eax

		result = 0;
		if (a1 != a2) {
			v2 = a2->playerId;
			v3 = a1->playerId == 11 ? a1->sprite->playerId : a1->playerId;
			if (!scbw::isAlliedTo(v2, v3)) {
				ByteOperations::SetLowByte(v4, AI_UnitCanAttack(a1));
				if (v4)
					result = 1;
			}
		}
		return result;
	}

	void drawBuildingInfo(void* dlg) { // 00427890
		CUnit* v1; // esi
		BinDlg* v2; // edi
		int v3; // edx
		int v4; // ebx
		int v5; // eax
		int v6; // eax
		BinDlg* v7; // eax
		int v8; // eax

		v1 = *activePortraitUnit;
		selectionGroupHP[0] = (*activePortraitUnit)->hitPoints;
		v2 = (BinDlg*)dlg;
		v3 = (*activePortraitUnit)->shields >> 8;
		*userInterfaceEnergy = (*activePortraitUnit)->energy;
		*userInterfaceShields = v3;
		*redrawTransportUI = needRedrawUI();
		genericStatusUpdateDrawnValues();
		ByteOperations::SetLowWord(v4, v1->id);
		if ((u16)v4 >= 0xB0u && (u16)v4 <= 0xB2u || (s16)v4 == 188)
			goto LABEL_35;
		if (!IS_IN_REPLAY && (unsigned __int8)v1->id != *LOCAL_NATION_ID)
			goto LABEL_6;
		if (!(v1->status & 1)) {
			drawProgressBar((int)v2);
			goto LABEL_32;
		}
		if ((s16)v4 == 108) {
			drawNukeSiloInfo((int)v2);
			goto LABEL_32;
		}
		if (unitIsActiveTransport(v1))
			goto LABEL_6;
		ByteOperations::SetLowByte(v5, isQueueSlotActive(v1, 0));
		if (v5) {
			drawTrainingProgress(v2);
			goto LABEL_32;
		}
		if (isConstructingAddon(v1) || isAttemptingProtossBuild(v1)) {
			drawAddonProgress((int)v2);
			goto LABEL_32;
		}
		if (v1->building.techType != 44) {
			drawTechProgress((int)v2);
			goto LABEL_32;
		}
		if (v1->building.upgradeType != 61) {
			drawUpgradeProgress(v2);
			goto LABEL_32;
		}
		v4 = (u16)v4;
		if (units_dat::SupplyProvided[(u16)v4]) {
			drawSupplyUnitInfo((int)v2);
			goto LABEL_32;
		}
		ByteOperations::SetLowByte(v6, unitIsGeyser(v1));
		if (v6) {
		LABEL_35:
			drawGasQuantity(v2);
			goto LABEL_32;
		}
		if (units_dat::AirWeapon[getLastQueueSlotType(v1)] == -126
			&& unitGetGroundWeapon(v1) == -126
			&& !units_dat::ShieldsEnabled[v4]) {
		LABEL_6:
			if (*statusScreenFunc != 4) {
				hideOptionalStatusScreenControls(v2);
				*statusScreenFunc = 4;
			}
			goto LABEL_32;
		}
		if (*statusScreenFunc != 3) {
			hideOptionalStatusScreenControls(v2);
			showControlGroup(v2, -20, 12);
			*statusScreenFunc = 3;
		}
		drawDisplayButtons((int)v2);
		v7 = getControlFromIndex(v2, -21);
		hideDialog(v7);
	LABEL_32:
		setTextStr((int)v2);
		setUnitStatusStrText((int)v2);
		v8 = unitGetName(*activePortraitUnit);
		addTextToDialog((int)v2, -5, v8);
	}

	int calcUnitStrength(int a1, u8 a2) { // 00431270
		int v2; // ecx
		unsigned __int64 v3; // rax
		int v4; // ebx
		u32 v5; // esi
		int v6; // edi
		int v7; // eax
		int totalHits; // [esp+0h] [ebp-4h]

		if (units_dat::ShieldsEnabled[a1]) {
			v2 = units_dat::MaxShieldPoints[a1];
		}
		else {
			v2 = 0;
		}

		totalHits = v2 + (units_dat::MaxHitPoints[a1] >> 8);
		if (totalHits) {
			v4 = (u8)weapons_dat::DamageFactor[a2];
			v5 = weapons_dat::Cooldown[a2];
			v6 = weapons_dat::DamageAmount[a2];
			if (a2 == -126) {
				v7 = 0;
			}
			else {
				v7 = (v6 * v4 << 11) / (s32)v5;
			}
			v3 = (unsigned __int64)(sqrt((double)(v6 * v4 * (weapons_dat::MaxRange[a2] / v5)
				+ ((u32)(totalHits * v7) >> 8)))
				* 7.58);
		}
		else {
			v3 = 0;
		}
		return v3;
	}

	u32 getUnitStrength(CUnit* a1, int a2, bool a3) {
		CUnit* v3; // edi
		int v4; // esi
		int v5; // ebx
		int v7; // ebx
		int v8; // esi
		u32 v9; // ecx
		int groundAir; // [esp+10h] [ebp+8h]

		v3 = a1;
		v4 = (u16)a1->id;
		if (v4 == UnitId::ZergLarva || v4 == UnitId::ZergEgg || v4 == UnitId::ZergCocoon || v4 == UnitId::ZergLurkerEgg)
			return 0;
		if (a1->status & UnitStatus::IsHallucination) {
			v5 = getTotalHitPointsPlusShields(a1);
			if (getCurrentHitPointsPlusShields(v3) < v5)
				return 0;
		}
		v7 = getBaseStrength(a3, v3);
		groundAir = getCurrentHitPointsPlusShields(v3);
		if (units_dat::ShieldsEnabled[v4])
			v8 = units_dat::MaxShieldPoints[v4];
		else
			v8 = 0;
		v9 = v8 + getMaxHP(v3);
		if (!v9)
			v9 = 1;
		return v7 * groundAir / v9;
	}

	bool AI_RestorationRequirementsProc(CUnit* a1, CUnit* a2) {
		CUnit* v2; // edx
		int v3; // eax
		bool result; // eax

		result = 0;
		if (a1->playerId == a2->playerId) {
			if (canWeaponTargetUnit(WeaponId::Restoration, a1, a2))	{
				v3 = units_dat::ShieldsEnabled[(u16)v2->id] ? v2->shields >> 8 : 0;
				if (v3 + ((v2->hitPoints + 255) >> 8) > 60
					&& (v2->ensnareTimer
						|| v2->plagueTimer
						|| v2->irradiateTimer
						|| v2->lockdownTimer
						|| v2->maelstromTimer
						|| v2->acidSporeCount))	{
					result = 1;
				}
			}
		}
		return result;
	}
}

namespace { // Helpers

	// For drawUnitInfo

	const u32 Func_hideOptionalStatusScreenControls = 0x00457310;
	void hideOptionalStatusScreenControls(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_hideOptionalStatusScreenControls
			POPAD
		}
	}

	const u32 Func_showControlGroup = 0x00457250;
	void showControlGroup(BinDlg* a1, s16 a2, s16 a3) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH a3
			CALL Func_showControlGroup
			POPAD
		}
	}

	const u32 Func_addTextToDialog = 0x004258B0;
	void addTextToDialog(int a1, s16 a2, int a3) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			PUSH a3
			CALL Func_addTextToDialog
			POPAD
		}
	}

	const u32 Func_drawShieldUpgrade = 0x00425510;
	void drawShieldUpgrade(int a1, u16 a2) {
		__asm {
			PUSHAD
			MOV EAX, a1
			PUSH a2
			CALL Func_drawShieldUpgrade
			POPAD
		}
	}

	const u32 Func_hideDialog = 0x00418700;
	void hideDialog(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_hideDialog
			POPAD
		}
	}

	// For drawSupplyUnitInfo

	const u32 Func_getSuppliesUsed = 0x00424732;
	int getSuppliesUsed() {
		static int result;
		__asm {
			PUSHAD
			CALL Func_getSuppliesUsed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_getLastQueueSlotType = 0x0047B270;
	u16 getLastQueueSlotType(CUnit* a1) {
		static u32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (u16)result;
	}

	const u32 Func_getSuppliesAvailable = 0x00488900;
	u32 getSuppliesAvailable(u8 a1, int a2) {
		static u32 result;
		__asm {
			PUSHAD
			MOVZX AL, a1
			MOV ECX, a2
			CALL Func_getSuppliesAvailable
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_drawArmor = 0x00425600;
	void drawArmor(int a1, int a2, u16 a3) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDX, a2
			PUSH a3
			CALL Func_drawArmor
			POPAD
		}
	}

	const u32 Func_unitStatAct_Standard = 0x00426F50;
	void unitStatAct_Standard(void* a1) {
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitStatAct_Standard
			POPAD
		}
	}

	// For AI_TargetEnemyProc

	const u32 Func_AI_UnitCanAttack = 0x00476180;
	bool AI_UnitCanAttack(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_AI_UnitCanAttack
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	// For drawBuildingInfo

	const u32 Func_needRedrawUI = 0x00424A10;
	int needRedrawUI() {
		static int result;
		__asm {
			PUSHAD
			CALL Func_needRedrawUI
			MOV result, EAX
			POPAD
		}
		return result;
	}
	
	const u32 Func_genericStatusUpdateDrawnValues = 0x004248F0;
	CUnit* genericStatusUpdateDrawnValues() {
		static CUnit* result;
		__asm {
			PUSHAD
			CALL Func_genericStatusUpdateDrawnValues
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_drawProgressBar = 0x004260C0;
	void drawProgressBar(int a1) {
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawProgressBar
			POPAD
		}
	}

	const u32 Func_drawNukeSiloInfo = 0x00426FF0;
	void drawNukeSiloInfo(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	const u32 Func_unitIsActiveTransport = 0x004E6BA0;
	bool unitIsActiveTransport(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitIsActiveTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_isQueueSlotActive = 0x00401E70;
	bool isQueueSlotActive(CUnit* a1, int a2) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_isQueueSlotActive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_drawTrainingProgress = 0x004268D0;
	void drawTrainingProgress(BinDlg* a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	const u32 Func_isConstructingAddon = 0x004E66B0;
	bool isConstructingAddon(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isConstructingAddon
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_isAttemptingProtossBuild = 0x004E4C40;
	bool isAttemptingProtossBuild(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isAttemptingProtossBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_drawAddonProgress = 0x00425F30;
	void drawAddonProgress(int a1) {
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	const u32 Func_drawTechProgress = 0x004266F0;
	void drawTechProgress(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	const u32 Func_drawUpgradeProgress = 0x00426500;
	void drawUpgradeProgress(BinDlg* a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	const u32 Func_unitIsGeyser = 0x004688B0;
	bool unitIsGeyser(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsGeyser
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_drawGasQuantity = 0x00426190;
	void drawGasQuantity(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	const u32 Func_unitGetGroundWeapon = 0x00475AD0;
	s8 unitGetGroundWeapon(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetGroundWeapon
			MOV result, EAX
			POPAD
		}
		return (s8)result;
	}

	const u32 Func_drawDisplayButtons = 0x00426C60;
	void drawDisplayButtons(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	const u32 Func_getControlFromIndex = 0x00418080;
	BinDlg* getControlFromIndex(BinDlg* a1, s16 a2) {
		static BinDlg* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_getControlFromIndex
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_setTextStr = 0x004263E0;
	void setTextStr(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	const u32 Func_setUnitStatusStrText = 0x00425B50;
	void setUnitStatusStrText(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}
	
	const u32 Func_unitGetName = 0x0047B5A0;
	int unitGetName(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetName
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For getUnitStrength
	const u32 Func_getCurrentHitPointsPlusShields = 0x004026D0;
	int getCurrentHitPointsPlusShields(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getCurrentHitPointsPlusShields
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_getBaseStrength = 0x004316D0;
	int getBaseStrength(int a1, CUnit* a2) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDI, a2
			CALL Func_getBaseStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_getMaxHP = 0x00401400;
	int getMaxHP(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getMaxHP
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For AI_RestorationRequirementsProc
	const u32 Func_canWeaponTargetUnit = 0x00475CE0;
	int canWeaponTargetUnit(u8 a1, CUnit* a2, CUnit* a3) {
		static int result;
		__asm {
			PUSHAD
			MOV AL, a1
			MOV EDX, a2
			PUSH a3
			CALL Func_canWeaponTargetUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}
}