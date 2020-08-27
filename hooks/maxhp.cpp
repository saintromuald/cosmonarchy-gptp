#include "maxhp.h"
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "hook_tools.h"

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

	// Not hooked yet
	Line 56570: 004262B7  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00426190 -- drawUnitInfo
	Line 58295: 0042781F  |.  8A8A B0476600 MOV CL,BYTE PTR DS:[EDX+6647B0] -- 0x00427540 -- Ss_DrawSupplyUnitStatus
	Line 58440: 004279F3   .  8A83 B0476600 MOV AL,BYTE PTR DS:[EBX+6647B0] -- 0x00427890 -- DrawGenericBuildingStatus
	Line 72970: 00431274  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x00431270 -- CalculateBaseStrength
	Line 73606: 00431865  |.  8A86 B0476600 MOV AL,BYTE PTR DS:[ESI+6647B0] -- 0x00431800 -- CalculateStrength
	Line 95686: 00440800   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004407E0 -- AI_RestorationRequirementsProc
	Line 95955: 00440A9A   .  8A90 B0476600 MOV DL,BYTE PTR DS:[EAX+6647B0] -- 0x00440A20 -- AI_targetEnemyProc
	Line 96913: 004413E7   .  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004413D0 -- AI_EMPShieldRequirementsProc
	Line 129037: 004566B4  |.  8A88 B0476600 MOV CL,BYTE PTR DS:[EAX+6647B0] -- 0x004566B0 -- sub_4566B0
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
	void hideOptionalStatusScreenControls(BinDlg* a1); // 00457310
	void showControlGroup(BinDlg* a1, s16 a2, s16 a3); // 00457250
	void addTextToDialog(int a1, s16 a2, int a3); // 004258B0
	void drawShieldUpgrade(int a1, u16 a2); // 00425510
	void hideDialog(BinDlg* a1); // 00418700
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

		v1 = (unsigned __int16)a1->id;
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
		char* v8; // [esp-8h] [ebp-10h]

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
			sprintf_s(struct_Char, 260, "%s %d", v3, v7);
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
		v8 = (char*)v5;
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
}