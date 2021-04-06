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
	Line 96913: 004413E7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004413D0 -- AI_EMPShieldRequirementsProc -- HOOKED
	Line 180864: 00479777  |.  8A82 B0476600 MOV AL,BYTE PTR DS:[EDX+6647B0] -- 0x00479730 -- attackOverlayAndNotify -- HOOKED
	Line 182607: 0047A841  |.  8A8F B0476600 MOV CL,BYTE PTR DS:[EDI+6647B0] -- 0x0047A820 -- DrawImage_HpBar -- HOOKED
	Line 184040: 0047B6A5  |.  8A87 B0476600 MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x0047B6A0 -- setBuildShieldGain -- HOOKED, dysfunctional
	Line 206949: 0048AD35  |.  8A81 B0476600 MOV AL,BYTE PTR DS:[ECX+6647B0] -- 0x0048ACD0 -- getUnitBulletDamage -- HOOKED, compiler errors
	Line 217770: 00492239  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00492140 -- canCastSpell -- HOOKED
	Line 293899: 004C5A3D  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004C5A20 -- modifyUnitShields -- HOOKED
	Line 318718: 004D60BC  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004D6010 -- compileHealthBar -- HOOKED, compiler error
	Line 343282: 004E6FC6  |.  8A87 B0476600 |MOV AL,BYTE PTR DS:[EDI+6647B0] -- 0x004E6EF0 -- Ai_FindTransport -- HOOKED, compiler errors
	Line 349375: 004EB0F4   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x004EAFE0 -- mapDataTransfer -- HOOKED, type error?
*/

namespace {

	// For drawUnitInfo
	void hideOptionalStatusScreenControls(BinDlg* a1);											// 00457310
	void showControlGroup(BinDlg* a1, int16 a2, int16 a3);											// 00457250
	void addTextToDialog(int a1, int16 a2, int a3);												// 004258B0
	void drawShieldUpgrade(int a1, uint16 a2);														// 00425510
	void hideDialog(BinDlg* a1);																// 00418700

	// For drawSupplyUnitInfo
	int getSuppliesUsed();																		// 00424732
	uint16 getLastQueueSlotType(CUnit* a1);														// 0047B270
	uint32 getSuppliesAvailable(uint8 a1, int a2);													// 00488900
	void drawArmor(int a1, int a2, uint16 a3);														// 00425600
	void unitStatAct_Standard(void* a1);														// 00426F50

	// For AI_TargetEnemyProc
	bool AI_UnitCanAttack(CUnit* a1);															// 00476180

	// For drawBuildingInfo
	int needRedrawUI();																			// 00424A10
	CUnit* genericStatusUpdateDrawnValues();													// 004248F0
	void drawProgressBar(int a1);																// 004260C0
	void drawNukeSiloInfo(int a1);																// 00426FF0
	bool unitIsActiveTransport(CUnit* a1);														// 004E6BA0
	bool isQueueSlotActive(CUnit* a1, int a2);													// 00401E70
	void drawTrainingProgress(BinDlg* a1);														// 004268D0
	bool isConstructingAddon(CUnit* a1);														// 004E66B0
	bool isAttemptingProtossBuild(CUnit* a1);													// 004E4C40
	void drawAddonProgress(int a1);																// 00425F30
	void drawTechProgress(int a1);																// 004266F0
	void drawUpgradeProgress(BinDlg* a1);														// 00426500
	bool unitIsGeyser(CUnit* a1);																// 004688B0
	void drawGasQuantity(BinDlg* a1);															// 00426190
	int8 unitGetGroundWeapon(CUnit* a1);															// 00475AD0
	void drawDisplayButtons(int a1);															// 00426C60
	BinDlg* getControlFromIndex(BinDlg* a1, int16 a2);											// 00418080
	void setTextStr(int a1);																	// 004263E0
	void setUnitStatusStrText(int a1);															// 00425B50
	int unitGetName(CUnit* a1);																	// 0047B5A0

	// For getUnitStrength
	int getCurrentHitPointsPlusShields(CUnit* a1);												// 004026D0
	int getBaseStrength(int a1, CUnit* a2);														// 004316D0
	int getMaxHP(CUnit* a1);																	// 00401400

	// For AI_RestorationRequirementsProc
	int canWeaponTargetUnit(uint8 a1, CUnit* a2, CUnit* a3);										// 00475CE0

	// For AI_EMPShieldRequirementsProc
	int32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2);										// 00440E30
	int createSplashWeaponProc(int16 a1, int16 a2, int16 a3, int(__fastcall* a4)(int, int), int a5);	// 004E82E0
	int unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2);							// 00440A60

	// For hallucinationHit
	void unitWasHit(CUnit* a1, CUnit* a2, int a3);												// 004795D0
	void doAttackNotifyEvent(CUnit* a1);														// 0048F230
	CImage* createShieldOverlay(int a1, CUnit* a2);												// 004E6140

	// For drawImageHPBar
	int unk_billions(int a1, int32 a2, int a3, int a4);											// 0047A750
	int unk_billions2(int a1, int32 a2, int a3, int a4);											// 0047A6E0
	int hpBarSurfaceBuffer(int a1, int a2, int* a3, int a4, int a5);							// 0047A350
	int hpBarSurfaceBuffer2(int a1, int a2, uint16 a3, int a4);									// 0047A2C3
	int renderHPBar(int a1, int a2, int* a3, int a4, int a5);									// 0047A4E0
	int32 getUnitMaxEnergy(CUnit* a1);															// 00491870
	int getHpBarDrawSizes(int a1, int32 a2, int a3, int a4);										// 0047A670

	// For canCastSpell
	bool unitIsFrozen(CUnit* a1);																// 004020B0
	bool isUnitCasterNotHallucination(CUnit* a1);												// 00401210
	bool getActiveTileFlag(int16 a1, int16 a2, int a3, int a4);										// 00472F80
	CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, int a3);						// 00491E80

	// For modifyUnitShields
	int SErrSetLastError(char a1);																// 00410172

	// For compileHealthBar
	void updateImageDrawData(CImage* a1);														// 004D57B0
	void playImageIscript(CImage* a1, char a2);													// 004D8470

	// For AI_FindTransport
	bool canEnterTransport(CUnit* a1, CUnit* a2);												// 004E6E00
	int16 SAI_GetRegionIdFromPxEx(int32 a1, int32 a2);												// 0049C9F0
	int32 getUnitDistanceToHalt(int a1, CUnit* a2, int a3);										// 00402140

	// For mapDataTransfer
	int32 dataTransfer00(int a1, char a2, int a3);												// 00472A00
	int dataTransfer01(uint32 a1, int a2, uint32 a3);													// 0045A600
	int32 dataTransfer02(int a1, int a2, char a3);												// 0045A230
	int32 dataTransfer03(int a1, char a2, int a3);												// 0045A1F0
	int dataTransfer04(uint32 a1, int a2, int a3);													// 0045A390
	int32 dataTransfer05(int a1, int a2);															// 0045A190
	vectorHead* vectorTransfer(char a1, char a2);												// 0045A010
	int dataTransfer06(int a1);																	// 00472900
}

namespace hooks {
	uint32 maxhp(int id, int playerId) {
		uint32 result = units_dat::MaxHitPoints[id];
		//if (id==UnitId::firebat && scbw::getUpgradeLevel(playerId, UpgradeId::Heatshield)) {
		//	result += 15 * 256;
		//}
		return result;
	}

	uint32 maxshields(int id, int playerId) {
		uint32 result = units_dat::MaxShieldPoints[id];
		//if (units_dat::BaseProperty[id] & UnitProperty::RoboticUnit && scbw::getUpgradeLevel(playerId, UpgradeId::Autovitality)) {
		//	result += result / 2;
		//}
		return result;
	}

	uint32 getTotalHitPointsPlusShields(CUnit* a1) { // 00402710
		int v1; // eax
		int v2; // ecx
		int v3; // eax

		v1 = (uint16)a1->id;
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
		int16 v2; // ax
		const char* v3; // eax
		uint16 v4; // cx
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
		if ((uint16)v2 >= 0xB0u && (uint16)v2 <= 0xB2u) {
			if (*statTxtTbl > 0x31Bu)
				v3 = (char*)statTxtTbl + statTxtTbl[796];
			else
				v3 = "";
			v7 = (uint16)(*activePortraitUnit)->building.resource.resourceAmount;
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
		} else {
			if (v1->controlType)
				v1 = *(BinDlg**)&v1->parent;
			v6 = *(BinDlg**)&v1->childrenDlg;
			if (v6) {
				while (v6->index != 9) {
					v6 = v6->next;
					if (!v6)
						goto LABEL_26;
				}
			} else {
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
		uint8 v4; // cl
		int v5; // edx
		int16 v6; // ax
		const char* v7; // ecx
		int16 v8; // ax
		const char* v9; // esi
		uint16 v10; // ax
		int v11; // ecx
		const char* v12; // esi
		int v13; // eax
		GroupFlag v14; // al
		int v15; // ecx
		int16 v16; // ax
		const char* v17; // eax
		uint16 v18; // ax
		BinDlg* v19; // eax
		int v20; // [esp+0h] [ebp-8h]
		uint8 v21; // [esp+7h] [ebp-1h]
		uint8 v22; // [esp+7h] [ebp-1h]

		v1 = (BinDlg*)a1;
		v2 = *activePortraitUnit;
		if (IS_IN_REPLAY || (uint8)(*activePortraitUnit)->playerId == *LOCAL_NATION_ID) {
			getSuppliesUsed();
			if (*statusScreenFunc != 12) {
				hideOptionalStatusScreenControls(v1);
				showControlGroup(v1, -10, -13);
				v2 = *activePortraitUnit;
				*statusScreenFunc = 12;
			}
			v3 = units_dat::GroupFlags[(uint16)v2->id];
			if (v3.isZerg) {
				v21 = 0;
			} else if (v3.isProtoss) {
				v21 = 2;
			} else {
				v21 = (v3.isTerran) != 0 ? RaceId::Terran : RaceId::Neutral;
			}
			v4 = v2->playerId;
			if (v21) { // !RACE_Zerg
				if (v21 == RaceId::Terran) {
					v5 = raceSupply[RaceId::Terran].used[v4];
				} else if (v21 == RaceId::Protoss) {
					v5 = raceSupply[RaceId::Protoss].used[v4];
				} else {
					v5 = 0;
				}
			} else {
				v5 = raceSupply[RaceId::Zerg].used[v4];
			}
			ByteOperations::SetLowWord(v20, v21);
			v6 = v21 + 820;
			if (v21 == -821) {
				v7 = 0;
			} else if (v6 < *statTxtTbl) {
				v7 = (char*)statTxtTbl + statTxtTbl[v6 + 1];
			} else {
				v7 = "";
			}
			sprintf_s(struct_Char2->buf, 260, "%s %d", v7, (v5 + 1) / 2);
			v8 = v21 + 814;
			if (v21 == -815) {
				v9 = 0;
			} else if (v8 < *statTxtTbl) {
				v9 = (char*)statTxtTbl + statTxtTbl[v8 + 1];
			} else {
				v9 = "";
			}
			v10 = getLastQueueSlotType((*activePortraitUnit));
			sprintf_s("", 260, "%s %d", v9, (units_dat::SupplyProvided[v10] + 1) / 2);
			v11 = v20 + 66353;
			if (v21 == -818) {
				v12 = 0;
			} else if ((uint16)v11 < *statTxtTbl) {
				v11 = (uint16)v11;
				v12 = (char*)statTxtTbl + statTxtTbl[(uint16)v11 + 1];
			} else {
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
			} else if (v14.isTerran) {
				v15 = raceSupply[RaceId::Terran].max[v22];
			} else if (v14.isProtoss) {
				v15 = raceSupply[RaceId::Protoss].max[v22];
			} else {
				v15 = 0;
			}
			v16 = v20 + 823;
			if ((int16)v20 == -824) {
				v17 = 0;
			} else if (v16 < *statTxtTbl) {
				v17 = (char*)statTxtTbl + statTxtTbl[v16 + 1];
			} else {
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
			} else if (v18 != UnitId::ZergOvileth && v18 != UnitId::Hero_Yggdrasill) {
				if (v1->controlType)
					v1 = *(BinDlg**)&v1->parent;
				v19 = *(BinDlg**)&v1->childrenDlg;
				if (v19) {
					while (v19->index != 9) {
						v19 = v19->next;
						if (!v19)
							goto LABEL_55;
					}
				} else {
				LABEL_55:
					v19 = 0;
				}
				hideDialog(v19);
			} else {
				drawArmor((int)v1, v18, 0);
			}
		} else {
			unitStatAct_Standard((void*)a1);
		}
	}

	bool AI_TargetEnemyProc(CUnit* a1, CUnit* a2) { // 00440A20
		int8 v2; // dl
		int8 v3; // bl
		int v4; // eax
		int8 result; // eax

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
		if ((uint16)v4 >= 0xB0u && (uint16)v4 <= 0xB2u || (int16)v4 == 188)
			goto LABEL_35;
		if (!IS_IN_REPLAY && (uint8)v1->id != *LOCAL_NATION_ID)
			goto LABEL_6;
		if (!(v1->status & 1)) {
			drawProgressBar((int)v2);
			goto LABEL_32;
		}
		if ((int16)v4 == 108) {
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
		v4 = (uint16)v4;
		if (units_dat::SupplyProvided[(uint16)v4]) {
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

	int calcUnitStrength(int a1, uint8 a2) { // 00431270
		int v2; // ecx
		unsigned _int64 v3; // rax
		int v4; // ebx
		uint32 v5; // esi
		int v6; // edi
		int v7; // eax
		int totalHits; // [esp+0h] [ebp-4h]

		if (units_dat::ShieldsEnabled[a1]) {
			v2 = units_dat::MaxShieldPoints[a1];
		} else {
			v2 = 0;
		}

		totalHits = v2 + (units_dat::MaxHitPoints[a1] >> 8);
		if (totalHits) {
			v4 = (uint8)weapons_dat::DamageFactor[a2];
			v5 = weapons_dat::Cooldown[a2];
			v6 = weapons_dat::DamageAmount[a2];
			if (a2 == -126) {
				v7 = 0;
			} else {
				v7 = (v6 * v4 << 11) / (int32)v5;
			}
			v3 = (unsigned _int64)(sqrt((double)(v6 * v4 * (weapons_dat::MaxRange[a2] / v5)
				+ ((uint32)(totalHits * v7) >> 8)))
				* 7.58);
		} else {
			v3 = 0;
		}
		return v3;
	}

	uint32 getUnitStrength(CUnit* a1, int a2, bool a3) { // 00431800
		CUnit* v3; // edi
		int v4; // esi
		int v5; // ebx
		int v7; // ebx
		int v8; // esi
		uint32 v9; // ecx
		int groundAir; // [esp+10h] [ebp+8h]

		v3 = a1;
		v4 = (uint16)a1->id;
		if (v4 == UnitId::ZergLarva || v4 == UnitId::ZergEgg || v4 == UnitId::ZergCocoon)
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

	bool AI_RestorationRequirementsProc(CUnit* a1, CUnit* a2) { // 004407E0
		CUnit* v2; // edx
		int v3; // eax
		bool result; // eax

		result = 0;
		if (a1->playerId == a2->playerId) {
			if (canWeaponTargetUnit(WeaponId::Restoration, a1, a2)) {
				v3 = units_dat::ShieldsEnabled[(uint16)v2->id] ? v2->shields >> 8 : 0;
				if (v3 + ((v2->hitPoints + 255) >> 8) > 60
					&& (v2->ensnareTimer
						|| v2->plagueTimer
						|| v2->irradiateTimer
						|| v2->lockdownTimer
						|| v2->maelstromTimer
						|| v2->corrosiveSporeCount)) {
					result = 1;
				}
			}
		}
		return result;
	}

	bool AI_EMPShieldRequirementsProc(CUnit* a1, CUnit* a2) { // 004413D0
		CUnit* v2; // esi
		CUnit* v3; // edi
		CSprite* v5; // edx
		int16 v6; // cx
		int16 spriteX;

		v2 = a1;
		v3 = a2;
		if (!AI_TargetUnitIsWorthHitting(a1, a2)
			|| !units_dat::ShieldsEnabled[(uint16)v2->id]
			|| !canWeaponTargetUnit(WeaponId::EMP_Shockwave, v2, v3)) {
			return 0;
		}
		v5 = v2->sprite;
		v6 = v5->position.y;
		spriteX = v5->position.x;
		*tempHPShields = 0;
		*tempWeapon = WeaponId::EMP_Shockwave;
		createSplashWeaponProc(
			160,
			spriteX,
			v6,
			(int(__fastcall*)(int, int))unitShieldsClumpInAreaAccumulatorProc,
			(int)v3);
		return *tempHPShields >= 200;
	}

	char attackOverlayAndNotify(CUnit* a1, CUnit* a2, uint8 a3, char a4) { // 00479730
		int32* v4; // eax
		CUnit* v5; // ebx
		char returnValue = 0;

		v5 = a1;
		v4 = &a1->hitPoints;
		if (v4) {
			if (!(v5->status & UnitStatus::Invincible)) {
				if (a2) {
					unitWasHit(v5, a2, a3 != WeaponId::Irradiate);
					if (a3 != WeaponId::Irradiate && v5->playerId != a2->playerId)
						doAttackNotifyEvent(v5);
				}
				if (units_dat::ShieldsEnabled[(uint16)v5->id]) {
					if (v5->shields >= 256) {
						*v4 = a3;
						if (weapons_dat::DamageType[a3]) {
							returnValue = (char)createShieldOverlay(a4, v5);
						}
					}
				}
			}
		}
		return returnValue;
	}

	void drawImageHPBar(int screenX, int screenY, GrpFrame* pFrame, Box32* grpRect, int colorData) { // 0047A820
		int v5; // esi
		int v6; // edi
		char v7; // cl
		int v8; // eax
		int v9; // ecx
		int v10; // eax
		unk_61* v11; // ecx
		int v12; // edx
		uint8 v13; // dl
		int v14; // eax
		int v15; // ecx
		int v16; // ebx
		unk_62* v17; // eax
		int v18; // ecx
		uint16 v19; // di
		int v20; // ebx
		uint16 v21; // ax
		int v22; // ebx
		int v23; // edi
		signed int v24; // ecx
		uint16 v25; // ax
		int v26; // eax
		int v27; // ebx
		int v28; // edx
		bool v29; // zf
		int v30; // eax
		unk_61* v31; // ecx
		int v32; // edx
		int v33; // [esp+Ch] [ebp-1Ch]
		int v34; // [esp+10h] [ebp-18h]
		int v35; // [esp+14h] [ebp-14h]
		int v36; // [esp+18h] [ebp-10h]
		int v37; // [esp+1Ch] [ebp-Ch]
		int v38; // [esp+20h] [ebp-8h]
		int v39; // [esp+24h] [ebp-4h]

		v37 = screenX;
		v5 = colorData;
		v6 = *(uint16*)(colorData + 100);
		v33 = grpRect->left;
		v34 = grpRect->top;
		v7 = units_dat::ShieldsEnabled[v6];
		v38 = screenY;
		v8 = grpRect->bottom;
		v35 = grpRect->right;
		if (v7) {
			v36 = 7;
			if (v8 <= 7)
				v36 = v8;
			v13 = sprites_dat::IsVisible[*(uint16*)(*(int*)(colorData + 12) + 8) + 390];
			v14 = units_dat::MaxHitPoints[v6] >> 8;
			ByteOperations::SetHighByte(colorData, sprites_dat::IsVisible[*(uint16*)(*(int*)(colorData + 12) + 8) + 390]);
			if (v14) {
				v15 = v14;
			} else {
				v15 = (*(int*)(v5 + 8) + 255) >> 8;
				if (!v15)
					v15 = 1;
			}
			v16 = unk_billions((*(int*)(v5 + 8) + 255) >> 8, v15, (int)&v39, v13);
			unk_billions2(*(int*)(v5 + 96) >> 8, units_dat::MaxShieldPoints[v6], (int)&colorData, HIBYTE(colorData));
			v17 = struct_5140E8;
			if (v16 < (uint8)(*struct_5140E8).anonymous_0) {
				do {
					v18 = (uint8)v17[1].anonymous_0;
					++v17;
				} while (v16 < v18);
			}
			v19 = v38;
			v20 = v37;
			hpBarSurfaceBuffer(v37, v38, &v33, v39, (int)v17);
			hpBarSurfaceBuffer2(colorData, v20, v19, (int)&v33);
			ByteOperations::SetHighByte(colorData, (uint8)8);
		} else {
			v36 = 5;
			if (v8 <= 5)
				v36 = v8;
			if (units_dat::MaxHitPoints[v6] >> 8) {
				v9 = units_dat::MaxHitPoints[v6] >> 8;
			} else {
				v9 = (*(int*)(colorData + 8) + 255) >> 8;
				if (!v9)
					v9 = 1;
			}
			v10 = unk_billions(
				(*(int*)(colorData + 8) + 255) >> 8,
				v9,
				(int)&v39,
				(uint8)sprites_dat::IsVisible[*(uint16*)(*(int*)(colorData + 12) + 8) + 390]);
			v11 = struct_5140C4;
			if (v10 < (uint8)(*struct_5140C4).anonymous_0) {
				do {
					v12 = (uint8)v11[1].anonymous_0;
					++v11;
				} while (v10 < v12);
			}
			renderHPBar(v37, v38, &v33, v39, (int)v11);
			ByteOperations::SetHighByte(colorData, 6);
		}
		v21 = *(int16*)(v5 + 100);
		v22 = units_dat::BaseProperty[v21] & UnitProperty::Spellcaster;
		if (v22 && (v23 = (*(int*)(v5 + 220) >> 30) & 1) == 0
			|| (v23 = (*(int*)(v5 + 220) >> 30) & 1) != 0
			|| v21 == 40) {
			if (*(uint8*)(v5 + 76) == *LOCAL_NATION_ID) {
				if (v22) {
					if (!v23) {
						v24 = getUnitMaxEnergy((CUnit*)v5) >> 8;
						goto LABEL_32;
					}
				} else if (!v23) {
					v25 = v21 != 40 ? 0 : 0x708;
					goto LABEL_31;
				}
				v25 = 1350;
			LABEL_31:
				v24 = v25;
			LABEL_32:
				if (!v22 || v23)
					v26 = *(uint16*)(v5 + 272);
				else
					v26 = *(uint8*)(v5 + 163);
				v27 = HIBYTE(colorData);
				v28 = grpRect->bottom - HIBYTE(colorData);
				v29 = grpRect->bottom == HIBYTE(colorData);
				v34 = HIBYTE(colorData) + grpRect->top;
				v36 = v28;
				if (v28 >= 0 && !v29) {
					v30 = getHpBarDrawSizes(
						v26,
						v24,
						(int)&v39,
						(uint8)sprites_dat::IsVisible[*(uint16*)(*(int*)(v5 + 12) + 8) + 390]);
					v31 = struct_5140D8;
					if (v30 < (uint8)(*struct_5140D8).anonymous_0) {
						do {
							v32 = (uint8)v31[1].anonymous_0;
							++v31;
						} while (v30 < v32);
					}
					renderHPBar(v37, v38 + v27, &v33, v39, (int)v31);
				}
				return;
			}
		}
	}

	void setBuildShieldGain(CUnit* a1) { // 0047B6A0
		int _type; // edi
		uint16 v2; // dx
		int v3; // ecx
		uint32 v4; // edx
		uint32 v5; // eax

		_type = (uint16)a1->id;
		if (units_dat::ShieldsEnabled[_type]) {
			if (a1->status & UnitStatus::GroundedBuilding) {
				v4 = (int32)((unsigned __int64)(1717986919i64 * (units_dat::MaxShieldPoints[_type] << 8)) >> 0x20) >> 2;
				v3 = v4 + (v4 >> 31);
				a1->shields = v3;
				v2 = units_dat::TimeCost[_type];
				if (!v2
					|| (v5 = (((uint32)units_dat::MaxShieldPoints[_type] << 8) - v3) / v2, (a1->buildRepairHpGain = v5) == 0)) {
					a1->buildRepairHpGain = 1;
				}
			}
		}
	}

	int getUnitBulletDamage(CUnit* victim, CBullet* bullet) { // 0048ACD0
		int v2; // edi
		CUnit* v3; // esi
		int v4; // eax
		int v5; // eax
		unsigned __int16 v6; // cx
		int v7; // esi
		int8 v8; // cl
		int result; // eax
		int v10; // eax
		int unitType; // ecx
		signed int v12; // eax
		int v13; // ecx
		signed int v14; // eax
		int v15; // eax

		v2 = (uint8)bullet->weaponType;
		v3 = victim;
		if (weapons_dat::ExplosionType[v2] == WeaponEffect::NuclearMissile) {
			unitType = (uint16)victim->id;
			v10 = units_dat::MaxHitPoints[unitType];
			if (units_dat::ShieldsEnabled[unitType]) {
				v12 = v10 >> 8;
				if (!v12) {
					v12 = (v3->hitPoints + 255) >> 8;
					if (!v12)
						v12 = 1;
				}
				v13 = 2 * (v12 + units_dat::MaxShieldPoints[unitType]);
			} else {
				v14 = v10 >> 8;
				if (!v14) {
					v14 = (v3->hitPoints + 255) >> 8;
					if (!v14)
						v14 = 1;
				}
				v13 = 2 * v14;
			}
			v15 = v13 / 3;
			if (v13 / 3 < 500)
				v15 = 500;
			result = v15 << 8;
		} else {
			v5 = (uint8)bullet->weaponType;
			v6 = weapons_dat::DamageUpgrade[v5];
			v7 = weapons_dat::DamageAmount[v5];
			v4 = (uint8)bullet->srcPlayer;
			if (v6 >= (uint16)BwUpgrade::UnusedUpgrade46)
				/*v8 = upgradesLevelBW[v4][v6];
			else
				v8 = upgradesLevelSC[v4][v6];*/
				result = (v7 + v8 * weapons_dat::DamageBonus[v2]) << 8;
		}
		return result;
	}

	int32 canCastSpell(CUnit* a1) { // 00492140
		uint8 v1; // al
		int v2; // ebx
		CUnit* v3; // esi
		signed int result; // eax
		int v5; // eax
		int v6; // eax
		bool v7; // eax
		int16 v8; // [esp+8h] [ebp-Ch]
		int16 v9; // [esp+Ch] [ebp-8h]
		uint8 v10; // [esp+13h] [ebp-1h]

		v1 = a1->mainOrderId;
		v2 = v1;
		v10 = orders_dat::OrderWeaponId[v1];
		v8 = a1->orderTarget.pt.y;
		v3 = a1->orderTarget.unit;
		v9 = a1->orderTarget.pt.x;
		if (a1->pAI) {
			switch (v1) {
				case OrderId::MagnaPulse:
				case OrderId::StasisField:
					if (v3 && !unitIsFrozen(v3))
						goto LABEL_23;
					goto LABEL_17;
				case OrderId::CastParasite:
					if (v3 && !v3->parasiteFlags)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::EmpShockwave:
					if (!v3)
						goto LABEL_23;
					ByteOperations::SetLowByte(v5, isUnitCasterNotHallucination(v3));
					if (v5) {
						if (HIBYTE(v3->energy) >= 0x64u)
							goto LABEL_23;
					}
					if (units_dat::ShieldsEnabled[(uint16)v3->id] && (int32)(v3->shields & 0xFFFFFF00) >= 25600)
						goto LABEL_23;
				LABEL_17:
					result = 1;
					break;
				case OrderId::Irradiate:
					if (v3 && !v3->irradiateTimer)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::Plague:
					if (v3 && !v3->plagueTimer)
						goto LABEL_23;
					result = 1;
					break;
				case OrderId::Ensnare:
					if (!v3 || !v3->ensnareTimer)
						goto LABEL_23;
					result = 1;
					break;
				default:
					goto LABEL_23;
			}
		} else {
		LABEL_23:
			if (orders_dat::UseWeaponTargeting[v2]) {
				if (!canWeaponTargetUnit(v10, v3, a1))
					return 1;
			} else {
				ByteOperations::SetLowByte(v6, orders_dat::TechUsed[v2]);
				if ((uint8)v6 < (uint8)TechId::None) {
					v6 = (uint8)v6;
					switch ((uint8)v6) {
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
							if ((uint8)v6 == TechId::SpiderMines) {
								ByteOperations::SetLowByte(v7, getActiveTileFlag(v8, v9, 0x410000, 0));
							} else {
								if (!v3 || v3->status & 0x4000000)
									goto LABEL_17;
								v7 = (uint16)getTargetUnitAbilityErrorStatString(v3, a1->playerId, v6) == 0;
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

	int32 modifyUnitShields(int a1, int a2) { // 004C5A20
		int v3; // eax
		signed int v4; // ecx

		if (!a1) {
			SErrSetLastError(87);
			return 0;
		}
		v3 = *(uint16*)(a1 + 100);
		if (!units_dat::ShieldsEnabled[v3])
			return 0;
		v4 = units_dat::MaxShieldPoints[v3];
		if (v4 >= (int32)(a2 * v4 / 0x64u))
			v4 = a2 * (uint32)units_dat::MaxShieldPoints[v3] / 0x64;
		*(int*)(a1 + 96) = v4 << 8;
		return 1;
	}

	void compileHealthBar(int a1, int a2) { // 004D6010
		int v2; // eax
		CUnit* v3; // edi
		uint8 v4; // cl
		int v5; // eax
		uint16 v6; // ax
		int v7; // eax
		char v8; // cl
		char v9; // cl
		int32* v10; // edx

		v2 = *(uint16*)(a2 + 8);
		v3 = activePlayerSelection->unit[*(uint8*)(a2 + 11)];
		v4 = sprites_dat::IsVisible[v2 + 390];
		*(int8*)(a1 + 15) = sprites_dat::HpBarSize[v2 + 262]
			+ (imageGrpGraphics[(uint8)orders_dat::Unused3[v2 + 62] + 561]->frames[0].height >> 1)
			+ 8;
		*(int8*)(*(int32*)(a1 + 44) + 6) = 0;
		*(int8*)(*(int32*)(a1 + 44) + 7) = 0;
		*(int8*)(*(int32*)(a1 + 44) + 8) = v4 - (v4 - 1) % 3;
		v5 = *(int32*)(a1 + 44);
		if (*(int8*)(v5 + 8) < 0x13u)
			*(int8*)(v5 + 8) = 19;
		*(int8*)(*(int32*)(a1 + 44) + 9) = 0;
		v6 = v3->id;
		if (units_dat::BaseProperty[v6] & 0x200000 && !(v3->status & 0x40000000)
			|| v3->status & 0x40000000
			|| v6 == 40) {
			*(int8*)(*(int32*)(a1 + 44) + 9) += 6;
		}
		v7 = *(int32*)(a1 + 44);
		v8 = *(int8*)(v7 + 9);
		if (units_dat::ShieldsEnabled[(unsigned __int16)v3->id])
			v9 = v8 + 7;
		else
			v9 = v8 + 5;
		*(int8*)(v7 + 9) = v9;
		*(int16*)(*(int32*)(a1 + 44) + 2) = *(unsigned __int8*)(*(int32*)(a1 + 44) + 8);
		*(int16*)(*(int32*)(a1 + 44) + 4) = *(unsigned __int8*)(*(int32*)(a1 + 44) + 9);
		*(int32*)(a1 + 60) = a2;
		*(int16*)(a1 + 12) = 0;
		v10 = (int32*)(a1 + 36);
		*v10 = 0;
		v10[1] = 0;
		//		*(int32*)(a1 + 48) = v3;
		updateImageDrawData((CImage*)a1);
		*(int8*)(a1 + 12) |= 1u;
		playImageIscript((CImage*)a1, 0);
	}

	int32 AI_FindTransport(CUnit* a1, int a2) { // 004E6EF0
		CUnit* v2; // esi
		int32 v3; // ecx
		uint16 v4; // ax
		int v5; // edi
		int v6; // eax
		char v7; // al
		int16 v8; // ST16_2
		uint32 v9; // eax
		int32 v11; // [esp+8h] [ebp-Ch]
		uint32 v12; // [esp+Ch] [ebp-8h]

		*(int32*)a2 = 0;
		v2 = firstPlayerUnit->unit[(uint8)a1->playerId];
		v11 = 0;
		v12 = 99999999;
		if (!v2)
			return v11;
		while (1) {
			v3 = v2->status;
			if (v3 & 0x40000000)
				goto LABEL_22;
			v4 = v2->id;
			//			if (v4 == 42 && !upgradesLevelSC[(uint8)v2->playerId][24])
			goto LABEL_22;
			v5 = v4;
			if (!units_dat::SpaceProvided[v4] || !(v3 & 1) || !(v3 & 0x20000))
				goto LABEL_22;
			if (v2->orderTarget.unit == a1)
				break;
			v6 = units_dat::MaxHitPoints[v4] >> 8;
			if (!v6) {
				v6 = (v2->hitPoints + 255) >> 8;
				if (!v6)
					v6 = 1;
			}
			if ((v2->hitPoints + 255) >> 8 >= 3 * v6 / 4 || units_dat::ShieldsEnabled[v5] && v2->shields >> 8 == units_dat::MaxShieldPoints[v5]) {
				v11 = 1;
				if (canEnterTransport(v2, a1)) {
					v7 = v2->mainOrderId;
					if (v7 != OrderId::Pickup2 && v7 != OrderId::Unload) {
						if (v7 != OrderId::MoveUnload
							|| (v8 = SAI_GetRegionIdFromPxEx(v2->orderTarget.pt.x, v2->orderTarget.pt.y),
								v8 == SAI_GetRegionIdFromPxEx(a1->orderTarget.pt.x, a1->orderTarget.pt.y))) {
							v9 = getUnitDistanceToHalt(v2->sprite->position.y, a1, v2->sprite->position.x);
							if (v9 < v12) {
								//								*(int32*)a2 = v2;
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
		//		*(int32*)a2 = v2;
		return 1;
	}

	int32 mapDataTransfer(char a1, int a2, int a3) { // 004EAFE0
		int v3; // eax
		int32 result; // eax

		if (!a2 || !a3)
			return 0;
		switch (*(unsigned __int8*)a2) {
			case 0u:
				result = dataTransfer00(a3, a1, a2);
				break;
			case 1u:
				result = dataTransfer01(a3, a2, a1);
				break;
			case 2u:
				result = dataTransfer02(a3, a2, a1);
				break;
			case 3u:
				result = dataTransfer03(a3, a1, a2);
				break;
			case 4u:
				result = dataTransfer04(a3, a2, a1);
				break;
			case 5u:
				result = dataTransfer05(a3, a2);
				break;
			case 6u:
				if (a3 == 3 && (v3 = (int)vectorTransfer(*(int8*)(a2 + 2), *(int8*)(a2 + 1))) != 0) {
					*(int8*)(v3 + 52) &= 0xFDu;
					dataTransfer06(v3);
					result = 1;
				} else {
					result = 0;
				}
				break;
			default:
				return 0;
		}
		return result;
	}
}

namespace { // Helpers

	// For drawUnitInfo

	const uint32 Func_hideOptionalStatusScreenControls = 0x00457310;
	void hideOptionalStatusScreenControls(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_hideOptionalStatusScreenControls
			POPAD
		}
	}

	const uint32 Func_showControlGroup = 0x00457250;
	void showControlGroup(BinDlg* a1, int16 a2, int16 a3) {
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_showControlGroup
			POPAD
		}
	}

	const uint32 Func_addTextToDialog = 0x004258B0;
	void addTextToDialog(int a1, int16 a2, int a3) {
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX CX, a2
			CALL Func_addTextToDialog
			POPAD
		}
	}

	const uint32 Func_drawShieldUpgrade = 0x00425510;
	void drawShieldUpgrade(int a1, uint16 a2) {
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_drawShieldUpgrade
			POPAD
		}
	}

	const uint32 Func_hideDialog = 0x00418700;
	void hideDialog(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_hideDialog
			POPAD
		}
	}

	// For drawSupplyUnitInfo

	const uint32 Func_getSuppliesUsed = 0x00424732;
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

	const uint32 Func_getLastQueueSlotType = 0x0047B270;
	uint16 getLastQueueSlotType(CUnit* a1) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV EDX, a1
			CALL Func_getLastQueueSlotType
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	const uint32 Func_getSuppliesAvailable = 0x00488900;
	uint32 getSuppliesAvailable(uint8 a1, int a2) {
		static uint32 result;
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

	const uint32 Func_drawArmor = 0x00425600;
	void drawArmor(int a1, int a2, uint16 a3) {
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOV EDX, a2
			CALL Func_drawArmor
			POPAD
		}
	}

	const uint32 Func_unitStatAct_Standard = 0x00426F50;
	void unitStatAct_Standard(void* a1) {
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitStatAct_Standard
			POPAD
		}
	}

	// For AI_TargetEnemyProc

	const uint32 Func_AI_UnitCanAttack = 0x00476180;
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

	const uint32 Func_needRedrawUI = 0x00424A10;
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

	const uint32 Func_genericStatusUpdateDrawnValues = 0x004248F0;
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

	const uint32 Func_drawProgressBar = 0x004260C0;
	void drawProgressBar(int a1) {
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawProgressBar
			POPAD
		}
	}

	const uint32 Func_drawNukeSiloInfo = 0x00426FF0;
	void drawNukeSiloInfo(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawNukeSiloInfo
			POPAD
		}
	}

	const uint32 Func_unitIsActiveTransport = 0x004E6BA0;
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

	const uint32 Func_isQueueSlotActive = 0x00401E70;
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

	const uint32 Func_drawTrainingProgress = 0x004268D0;
	void drawTrainingProgress(BinDlg* a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTrainingProgress
			POPAD
		}
	}

	const uint32 Func_isConstructingAddon = 0x004E66B0;
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

	const uint32 Func_isAttemptingProtossBuild = 0x004E4C40;
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

	const uint32 Func_drawAddonProgress = 0x00425F30;
	void drawAddonProgress(int a1) {
		__asm {
			PUSHAD
			MOV EDI, a1
			CALL Func_drawAddonProgress
			POPAD
		}
	}

	const uint32 Func_drawTechProgress = 0x004266F0;
	void drawTechProgress(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawTechProgress
			POPAD
		}
	}

	const uint32 Func_drawUpgradeProgress = 0x00426500;
	void drawUpgradeProgress(BinDlg* a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawUpgradeProgress
			POPAD
		}
	}

	const uint32 Func_unitIsGeyser = 0x004688B0;
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

	const uint32 Func_drawGasQuantity = 0x00426190;
	void drawGasQuantity(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_drawGasQuantity
			POPAD
		}
	}

	const uint32 Func_unitGetGroundWeapon = 0x00475AD0;
	int8 unitGetGroundWeapon(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_unitGetGroundWeapon
			MOV result, EAX
			POPAD
		}
		return (int8)result;
	}

	const uint32 Func_drawDisplayButtons = 0x00426C60;
	void drawDisplayButtons(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_drawDisplayButtons
			POPAD
		}
	}

	const uint32 Func_getControlFromIndex = 0x00418080;
	BinDlg* getControlFromIndex(BinDlg* a1, int16 a2) {
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

	const uint32 Func_setTextStr = 0x004263E0;
	void setTextStr(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setTextStr
			POPAD
		}
	}

	const uint32 Func_setUnitStatusStrText = 0x00425B50;
	void setUnitStatusStrText(int a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_setUnitStatusStrText
			POPAD
		}
	}

	const uint32 Func_unitGetName = 0x0047B5A0;
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
	const uint32 Func_getCurrentHitPointsPlusShields = 0x004026D0;
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

	const uint32 Func_getBaseStrength = 0x004316D0;
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

	const uint32 Func_getMaxHP = 0x00401400;
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
	const uint32 Func_canWeaponTargetUnit = 0x00475CE0;
	int canWeaponTargetUnit(uint8 a1, CUnit* a2, CUnit* a3) {
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

	// For AI_EMPShieldRequirementsProc

	const uint32 Func_AI_TargetUnitIsWorthHitting = 0x00440E30;
	int32 AI_TargetUnitIsWorthHitting(CUnit* a1, CUnit* a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_AI_TargetUnitIsWorthHitting
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_createSplashWeaponProc = 0x004E82E0;
	int createSplashWeaponProc(int16 a1, int16 a2, int16 a3, int(__fastcall* a4)(int, int), int a5) {
		static int result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			MOV AX, a1
			MOV DX, a2
			MOV CX, a3
			CALL Func_createSplashWeaponProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_unitShieldsClumpInAreaAccumulatorProc = 0x00440A60;
	int unitShieldsClumpInAreaAccumulatorProc(CUnit* a1, CUnit* a2) {
		static int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			CALL Func_unitShieldsClumpInAreaAccumulatorProc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For hallucinationHit
	const uint32 Func_unitWasHit = 0x004795D0;
	void unitWasHit(CUnit* a1, CUnit* a2, int a3) {
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EBX, a1
			CALL Func_unitWasHit
			POPAD
		}
	}

	const uint32 Func_doAttackNotifyEvent = 0x0048F230;
	void doAttackNotifyEvent(CUnit* a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_doAttackNotifyEvent
			POPAD
		}
	}

	const uint32 Func_createShieldOverlay = 0x004E6140;
	CImage* createShieldOverlay(int a1, CUnit* a2) {
		static CImage* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_createShieldOverlay
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For drawImageHPBar
	const uint32 Func_unk_billions = 0x0047A750;
	int unk_billions(int a1, int32 a2, int a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EBX, a3
			CALL Func_unk_billions
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_unk_billions2 = 0x0047A6E0;
	int unk_billions2(int a1, int32 a2, int a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_unk_billions2
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_hpBarSurfaceBuffer = 0x0047A350;
	int hpBarSurfaceBuffer(int a1, int a2, int* a3, int a4, int a5) {
		static int result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_hpBarSurfaceBuffer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_hpBarSurfaceBuffer2 = 0x0047A120;
	int hpBarSurfaceBuffer2(int a1, int a2, uint16 a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_hpBarSurfaceBuffer2
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_renderHPBar = 0x0047A4E0;
	int renderHPBar(int a1, int a2, int* a3, int a4, int a5) {
		static int result;
		__asm {
			PUSHAD
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_renderHPBar
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getUnitMaxEnergy = 0x00491870;
	int32 getUnitMaxEnergy(CUnit* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_getUnitMaxEnergy
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getHpBarDrawSizes = 0x0047A670;
	int getHpBarDrawSizes(int a1, int32 a2, int a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			MOV EAX, a1
			MOV ECX, a2
			MOV EDI, a3
			CALL Func_getHpBarDrawSizes
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For canCastSpell
	const uint32 Func_unitIsFrozen = 0x004020B0;
	bool unitIsFrozen(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitIsFrozen
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_isUnitCasterNotHallucination = 0x00401210;
	bool isUnitCasterNotHallucination(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitCasterNotHallucination
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_getActiveTileFlag = 0x00472F80;
	bool getActiveTileFlag(int16 a1, int16 a2, int a3, int a4) {
		static int result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			MOVZX AX, a1
			CALL Func_getActiveTileFlag
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_getTargetUnitAbilityErrorStatString = 0x00491E80;
	CUnit* getTargetUnitAbilityErrorStatString(CUnit* a1, char a2, int a3) {
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_getTargetUnitAbilityErrorStatString
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For modifyUnitShields
	const uint32 Func_SErrSetLastError = 0x00410172;
	int SErrSetLastError(char a1) {
		static int result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_SErrSetLastError
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For compileHealthBar

	const uint32 Func_updateImageDrawData = 0x004D57B0;
	void updateImageDrawData(CImage* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_updateImageDrawData
			POPAD
		}
	}

	const uint32 Func_playImageIscript = 0x004D8470;
	void playImageIscript(CImage* a1, char a2) {
		__asm {
			PUSHAD
			PUSH a2
			MOV ECX, a1
			CALL Func_playImageIscript
			POPAD
		}
	}

	// For AI_FindTransport
	const uint32 Func_canEnterTransport = 0x004E6E00;
	bool canEnterTransport(CUnit* a1, CUnit* a2) {
		static int result;
		__asm {
			PUSHAD
			PUSH a2
			MOV EAX, a1
			CALL Func_canEnterTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
	int16 SAI_GetRegionIdFromPxEx(int32 a1, int32 a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ECX, a2
			CALL Func_SAI_GetRegionIdFromPxEx
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	const uint32 Func_getUnitDistanceToHalt = 0x00402140;
	int32 getUnitDistanceToHalt(int a1, CUnit* a2, int a3) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV ECX, a1
			MOV EAX, a2
			CALL Func_getUnitDistanceToHalt
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For mapDataTransfer
	const uint32 Func_dataTransfer00 = 0x00472A00;
	int32 dataTransfer00(int a1, char a2, int a3) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX CL, a2
			MOV ESI, a3
			CALL Func_dataTransfer00
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer01 = 0x0045A600;
	int dataTransfer01(uint32 a1, int a2, uint32 a3) {
		static int result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_dataTransfer01
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer02 = 0x0045A230;
	int32 dataTransfer02(int a1, int a2, char a3) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			MOVZX BL, a3
			CALL Func_dataTransfer02
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer03 = 0x0045A1F0;
	int32 dataTransfer03(int a1, char a2, int a3) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_dataTransfer03
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer04 = 0x0045A390;
	int dataTransfer04(uint32 a1, int a2, int a3) {
		static int result;
		__asm {
			PUSHAD
			PUSH a3
			PUSH a2
			MOV EAX, a1
			CALL Func_dataTransfer04
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer05 = 0x0045A190;
	int32 dataTransfer05(int a1, int a2) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV EDI, a2
			CALL Func_dataTransfer05
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_vectorTransfer = 0x0045A010;
	vectorHead* vectorTransfer(char a1, char a2) {
		static vectorHead* result;
		__asm {
			PUSHAD
			PUSH a2
			MOVZX BL, a1
			CALL Func_vectorTransfer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_dataTransfer06 = 0x00472900;
	int dataTransfer06(int a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_dataTransfer06
			MOV result, EAX
			POPAD
		}
		return result;
	}
}