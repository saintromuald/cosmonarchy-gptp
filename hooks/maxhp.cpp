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

	// Not hooked yet
	Line 58440: 004279F3   .  8A83 B0476600 MOV AL,BYTE PTR DS:[EBX+6647B0] -- 0x00427890 -- DrawGenericBuildingStatus
	Line 72970: 00431274  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00431270 -- CalculateBaseStrength
	Line 73606: 00431865  |.  8A86 B0476600 MOV AL,BYTE PTR DS:[ESI+6647B0] -- 0x00431800 -- CalculateStrength
	Line 95686: 00440800   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004407E0 -- AI_RestorationRequirementsProc
	Line 95955: 00440A9A   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x00440A20 -- AI_targetEnemyProc
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

	u32 getTotalHitPointsPlusShields(CUnit* a1) {
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

	void drawUnitInfo(BinDlg* a1) {
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
				if (v21 == 1) { // RACE_Terran
					v5 = raceSupply[RaceId::Terran].used[v4];
				}
				else if (v21 == 2) { // RACE_Protoss
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
}