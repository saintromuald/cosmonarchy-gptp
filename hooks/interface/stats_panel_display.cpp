#include "stats_panel_display.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/buttonsets.h"
//Helper functions declaration

namespace {

	void hideDialog(BinDlg* dialog);										// 00418700
	void StatsNukesCount(BinDlg* dialog, uint32 index);						// 00425310
	void StatHangerCount(BinDlg* dialog, uint32 index);						// 004253D0
	void StatsShieldLevel(BinDlg* dialog, uint32 index);					// 00425510
	void StatsArmorLevel(BinDlg* dialog, uint32 index);						// 00425600
	void StatsWeaponLevel(BinDlg* dialog, uint32 index, uint32 weaponId);	// 00425790
	bool StatSpidermineCount(BinDlg* dialog, uint32 index);					// 00426300
	uint16 getLastQueueSlotType(CUnit* unit);								// 0047B270

	// For getUnitRankString
	__int8 unitGetRace(CUnit* a1);											// 0047B150
	char* getStatTxtString(__int16 a1);										// 004C36F0

	// For statsWeaponLevel / statsArmorLevel
	char getArmorCount(CUnit* unit, int32 a2);								// 00453FC0
	void showDialog(BinDlg* dialog);										// 004186A0
	int32 disableDialog(BinDlg* dialog);									// 00418640
	void updateDialog(BinDlg* dialog);										// 0041C400

} //unnamed namespace

namespace hooks {

	bool StatLobotomyMineCount(BinDlg* dialog, uint32 index) {
		auto v2 = *activePortraitUnit;
		auto v3 = dialog;
		// Originally had code for research, removed
		sprintf_s(statusscreen_infobtn[index], 8, "%d ", v2->vulture.spiderMineCount);
		if (v3->controlType) {
			v3 = v3->parent;
		}
		auto v5 = v3->childrenDlg;
		bool jump = false;
		if (v5) {
			while (v5->index != index + 9) {
				v5 = v5->next;
				if (!v5) {
					v5 = nullptr;
					break;
				}
			}
		} else {
			v5 = nullptr;
		}
		scbw::showDialog(v5);
		scbw::disableDialog(v5);
		auto statData = v5->statButtonUser;
		auto v7 = *cmdicons_grp;
		v5->graphic = UnitId::TerranLobotomyMine - 1;//or set icon id
		statData->button_grp = v7;
		//		statData->grp_frame = UnitId::LobotomyMine;
		statData->grp_frame = 13;
		statData->type = 3;
		//		statData->id = UnitId::LobotomyMine;
		statData->id = 13;
		auto v8 = v5->flags;
		v5->pszText = statusscreen_infobtn[index];
		if (!(v8 & 1)) {
			v5->flags = v8 | 1;
			scbw::updateDialog(v5);
		}
		return 1;
	}

	//Renamed from sub_426C60
	//Attempting to be true to the original result
	//in badly unoptimized code
	void stats_panel_display(BinDlg* dialog) {

		BinDlg* current_dialog;
		CUnit* activeUnit = *activePortraitUnit;
		uint32 index = 0;

		if (!(activeUnit->status & UnitStatus::IsHallucination) ||
			(!*IS_IN_REPLAY && activeUnit->playerId != *LOCAL_NATION_ID)
			) {

			bool bNoAirWeaponDisplay;
			uint32 airWeapon;			//[EBP-01] then [EBP-05]
			uint8 airWeaponMainUnit;	//BL
			uint8 airWeaponSubunit;	//AL

			uint16 unitId = getLastQueueSlotType(activeUnit);

			if (units_dat::ArmorUpgrade[unitId] != UpgradeId::GlobalUpgrade60) {
				StatsArmorLevel(dialog, index);
				index++;
			}

			if (units_dat::ShieldsEnabled[activeUnit->id]) {
				StatsShieldLevel(dialog, index);
				index++;
			}

			if (activeUnit->id != UnitId::ZergLakizilisk
				|| activeUnit->status & UnitStatus::Burrowed) {
				bool bNoGroundWeaponDisplay = false;

				uint32 groundWeapon;													//[EBP-05]
				uint8 groundWeaponMainUnit = units_dat::GroundWeapon[activeUnit->id];	//BL
				if (activeUnit->id == UnitId::TerranMadrigal
					&& scbw::get_generic_rust_value(activeUnit, ValueId::TempoChange)
					&& groundWeaponMainUnit == WeaponId::MadrigalFuriaMissiles) {
					groundWeaponMainUnit = WeaponId::MadrigalImperiosoMissiles;
				}
				uint8 groundWeaponSubunit;												//CL

				if (groundWeaponMainUnit == WeaponId::None) {
					if (activeUnit->subunit == nullptr) {
						bNoGroundWeaponDisplay = true;
					}
					else {
						groundWeaponSubunit = units_dat::GroundWeapon[activeUnit->subunit->id];
					}
				} else {
					groundWeaponSubunit = groundWeaponMainUnit;
				}

				if (!bNoGroundWeaponDisplay) {
					if (groundWeaponSubunit == WeaponId::None) {
						bNoGroundWeaponDisplay = true;			//26D19
					}
					else
						if (activeUnit->id == UnitId::ZergLakizilisk && !(activeUnit->status & UnitStatus::Burrowed)) {
							groundWeapon = UnitId::None;			//26D2A
						}
						else
							if (groundWeaponMainUnit != WeaponId::None) {
								groundWeapon = groundWeaponMainUnit;	//26D35
							}
							else
								if (activeUnit->subunit == nullptr) {
									groundWeapon = WeaponId::None;			//26D41
								}
								else {
									groundWeapon = units_dat::GroundWeapon[activeUnit->subunit->id];	//26D51
								}

					if (!bNoGroundWeaponDisplay) {
						//26D54
						StatsWeaponLevel(dialog, index, groundWeapon);
						index++;
						// Display additional weapons
						if (activeUnit->id == UnitId::TerranCentaur) {
							StatsWeaponLevel(dialog, index, WeaponId::CentaurSunchaserMissiles);
							index++;
						}
					}
				}
			}

			//26D67
			bNoAirWeaponDisplay = false;
			airWeaponMainUnit = units_dat::AirWeapon[activeUnit->id];
			if (airWeaponMainUnit != WeaponId::None) {
				airWeaponSubunit = airWeaponMainUnit;	//26D79
			}
			else
				if (activeUnit->subunit == nullptr) {
					bNoAirWeaponDisplay = true;				//26D82
				}
				else {
					airWeaponSubunit = units_dat::AirWeapon[activeUnit->subunit->id];	//26D8C
				}

			if (!bNoAirWeaponDisplay) {
				if (airWeaponSubunit == WeaponId::None) {
					bNoAirWeaponDisplay = true;		//26D94
				}
				else
					if (airWeaponMainUnit != WeaponId::None) {
						airWeapon = airWeaponMainUnit;	//26DA2
					}
					else {
						if (activeUnit->subunit == nullptr) {
							airWeapon = WeaponId::None;	//26DAF
						}
						else {
							airWeapon = units_dat::AirWeapon[activeUnit->subunit->id];
						}
					}
				if (!bNoAirWeaponDisplay) {
					uint8 groundWeapon;	//AL

					//26DBE
					if (activeUnit->id == UnitId::ZergLakizilisk && !(activeUnit->status & UnitStatus::Burrowed)) {
						groundWeapon = WeaponId::None;
					}
					else {
						groundWeapon = units_dat::GroundWeapon[activeUnit->id];
						if (groundWeapon == WeaponId::None && activeUnit->subunit != nullptr) {
							groundWeapon = units_dat::GroundWeapon[activeUnit->subunit->id]; //26DE6
						}
					}

					//26DF0
					if (airWeapon == groundWeapon) {
						bNoAirWeaponDisplay = true;
					}
					else
						if (airWeaponMainUnit != WeaponId::None) {
							airWeapon = airWeaponMainUnit;	//26DFA
						}							
						else
							if (airWeaponMainUnit == WeaponId::None) { //26DFF
								if (activeUnit->subunit == nullptr) {
									airWeapon = WeaponId::None;	//26E06
								}									
								else {
									airWeapon = units_dat::AirWeapon[activeUnit->subunit->id]; //26E16
								}	
							}
					if (!bNoAirWeaponDisplay) { //26E19
						StatsWeaponLevel(dialog, index, airWeapon);
						index++;
					}
				} //if(!bNoAirWeaponDisplay)
			} //if(!bNoAirWeaponDisplay)

			//26E31
			if (activeUnit->id == UnitId::ProtossSolarion
				|| activeUnit->id == UnitId::ProtossAccantor
				) { // Originally had hardcode for Gantrithor and Warbringer, removed for HYDRA
				StatHangerCount(dialog, index);
				index++;
			}

			if (activeUnit->id == UnitId::TerranVulture) { // Originally also had hardcode for Raynor Vulture, removed for HYDRA
				if (StatSpidermineCount(dialog, index))
					index++; //don't increase the index if not displaying
			}
			if (activeUnit->id == UnitId::ghost) {
				if (StatLobotomyMineCount(dialog, index)) {
					index++;
				}
			}
			unitId = getLastQueueSlotType(activeUnit);
			if (unitId == UnitId::TerranNuclearSilo) {
				StatsNukesCount(dialog, index);
				index++;
			}
		}

		//26E96
		if (dialog->controlType == DialogControlTypes::DialogBox) {
			current_dialog = dialog;
		}
		else {
			current_dialog = dialog->parent;
		}
		if (current_dialog->childrenDlg != nullptr) {
			current_dialog = current_dialog->childrenDlg;
			while (current_dialog != nullptr && current_dialog->index != index + 9) {
				current_dialog = current_dialog->next;
			}
			while (index < 4 && current_dialog != nullptr) {
				hideDialog(current_dialog);
				current_dialog = current_dialog->next;
				index++;
			}
		}
	} //void stats_panel_display(BinDlg* dialog)

	// Hooked to enable/disable rank display based on new unit definitions -- Pr0nogo
	char* getUnitRankString(CUnit* a2) { // 0042F370
		unsigned __int16 _type; // ax
		int v2; // ecx

		_type = a2->id;
		if (units_dat::BaseProperty[_type] & UnitProperty::NeutralAccessories
			|| _type == UnitId::TerranLobotomyMine
			|| _type == UnitId::TerranSpiderMine)
		{
			return 0;
		}
		// Originally had Gantrithor - Tassadar hardcode here, removed for HYDRA
		if (_type != UnitId::TerranMinotaur && _type != UnitId::Hero_NoradII && _type != UnitId::Hero_Hyperion
			|| !(units_dat::BaseProperty[_type] & UnitProperty::Hero)
			|| !units_dat::MapStringId[_type])
		{
			if (unitGetRace(a2) == RaceId::Terran) {
				return getStatTxtString((unsigned __int8)(a2->rankIncrease + units_dat::Rank[_type]) + 1302);
			}
			return 0;
		}
		if (statTxtTblNew->getString(570)) {
			return "";
		}
		return (char*)statTxtTblNew->getString(571); // "Admiral"
	}

	// Hooked to draw new values to icons -- Pr0nogo
	void statsArmorLevel(BinDlg* dialog, int32 unused, uint16 buttonRelated) { // 00425600
		BinDlg* v3; // ebx
		BinDlg* v4; // esi
		void* v5; // ecx
		int32 v6; // edx
		CUnit* v7; // eax
		uint16 v8; // di
		uint16 v9; // cx
		int16 v10; // di
		uint16 v11; // ax
		int32 v12; // eax
		uint8 v13; // [esp+13h] [ebp-1h]

		v3 = dialog;
		v13 = getArmorCount(*activePortraitUnit, unused);
		sprintf_s(statusscreen_infobtn[buttonRelated], 8, "%d ", v13);
		if (v3->controlType) {
			v3 = *(BinDlg**)&v3->parent;
		}
		v4 = *(BinDlg**)&v3->childrenDlg;
		if (v4)
		{
			while (v4->index != buttonRelated + 9)
			{
				v4 = v4->next;
				if (!v4)
					goto LABEL_6;
			}
		}
		else
		{
		LABEL_6:
			v4 = 0;
		}
		showDialog(v4);
		disableDialog(v4);
		v5 = cmdicons_grp;
		v6 = *v4->user;
		v7 = *activePortraitUnit;
		v4->graphic = 12;
		// TODO: Fix compiler error
//		*(int32*)v6 = v5;
		if (v7->status & UnitStatus::Completed
			|| (v8 = v7->buildQueue[(uint8)v7->buildQueueSlot], !scbw::isZergBuildingMorphResult(v8)))
		{
			v9 = v7->id;
		}
		else
		{
			v9 = v7->buildQueue[(uint8)v7->buildQueueSlot];
		}
		*(int16*)(v6 + 4) = upgrades_dat::UpgradeIcon[(uint8)units_dat::ArmorUpgrade[v9]];
		*(int16*)(v6 + 6) = UnitStatus::Completed;
		if (v7->status & UnitStatus::Completed
			|| (v10 = v7->buildQueue[(uint8)v7->buildQueueSlot], !scbw::isZergBuildingMorphResult(v8)))
		{
			v11 = v7->id;
		}
		else
		{
			v11 = v7->buildQueue[(uint8)v7->buildQueueSlot];
		}
		*(int16*)(v6 + 8) = (uint8)units_dat::ArmorUpgrade[v11];
		v4->pszText = statusscreen_infobtn[buttonRelated];
		v12 = v4->flags;
		if (!(v12 & UnitStatus::Completed))
		{
			v4->flags = v12 | UnitStatus::Completed;
			updateDialog(v4);
		}
		*unkUpgradeCount += v13;
	}

	void statsWeaponLevel(BinDlg* dialog, uint16 buttonRelated, uint8 weaponId) { // 00425790
		BinDlg* v3; // ebx
		int32 v4; // eax
		uint8 v5; // al
		BinDlg* v6; // esi
		int32 v7; // eax
		void* v8; // ecx
		int32 v9; // eax
		int16 v10; // [esp+10h] [ebp-4h]

		v3 = dialog;
		v4 = (*activePortraitUnit)->playerId;
		if (weapons_dat::DamageUpgrade[weaponId] >= 0x2Eu) {
//			v5 = *upgradesLevelBW[v4][weapons_dat::DamageUpgrade[weaponId]];
		}
		else {
//			v5 = *upgradesLevelSC[v4][weapons_dat::DamageUpgrade[weaponId]];
		}
		v10 = v5;
		sprintf_s(statusscreen_infobtn[buttonRelated], 8, "%d ", v5);
		if (v3->controlType) {
			v3 = v3->parent;
		}
		v6 = v3->childrenDlg;
		if (v6)	{
			while (v6->index != buttonRelated + 9)
			{
				v6 = v6->next;
				if (!v6)
					goto LABEL_9;
			}
		}
		else
		{
		LABEL_9:
			v6 = 0;
		}
		showDialog(v6);
		disableDialog(v6);
//		v7 = v6->user;
		v8 = cmdicons_grp;
		v6->graphic = 12;
//		*(int32*)v7 = v8;
		*(int16*)(v7 + 4) = weapons_dat::Icon[weaponId];
		*(int16*)(v7 + 6) = 0;
		*(int16*)(v7 + 8) = weaponId;
		v9 = v6->flags;
		v6->pszText = statusscreen_infobtn[buttonRelated];
		if (!(v9 & 1))
		{
			v6->flags = v9 | 1;
			updateDialog(v6);
		}
		*unkUpgradeCount += v10;
	}
	

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const uint32 Func_HideDialog = 0x00418700;
	void hideDialog(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_HideDialog
			POPAD
		}
	}

	const uint32 Func_Sub425310 = 0x00425310;
	void StatsNukesCount(BinDlg* dialog, uint32 index) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425310
			POPAD
		}
	}

	const uint32 Func_StatHangerCount = 0x004253D0;
	void StatHangerCount(BinDlg* dialog, uint32 index) {
		__asm {
			PUSHAD
			MOV EBX, dialog
			PUSH index
			CALL Func_StatHangerCount
			POPAD
		}
	}

	const uint32 Func_Sub425510 = 0x00425510;
	void StatsShieldLevel(BinDlg* dialog, uint32 index) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425510
			POPAD
		}
	}

	const uint32 Func_Sub425600 = 0x00425600;
	void StatsArmorLevel(BinDlg* dialog, uint32 index) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_Sub425600
			POPAD
		}
	}

	const uint32 Func_Sub425790 = 0x00425790;
	void StatsWeaponLevel(BinDlg* dialog, uint32 index, uint32 weaponId) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH weaponId
			PUSH index
			CALL Func_Sub425790
			POPAD
		}
	}

	const uint32 Func_StatSpidermineCount = 0x00426300;
	bool StatSpidermineCount(BinDlg* dialog, uint32 index) {
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, dialog
			PUSH index
			CALL Func_StatSpidermineCount
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	//Identical to function at 0x0047B270
	//EDX = unit
	//If the unit is currently mutating into
	//a level 2/3 zerg building, return the
	//id of the building morphed into, else
	//return the normal unit id.
	uint16 getLastQueueSlotType(CUnit* unit) {
		uint16 unitId;
		if (unit->status & UnitStatus::Completed) {
			unitId = unit->id;
		}
		else {
			unitId = unit->buildQueue[unit->buildQueueSlot];
			if (!(scbw::isZergBuildingMorphResult)) {
				unitId = unit->id;
			}
		}
		return unitId;
	}

	// For getUnitRankString

	const uint32 Func_unitGetRace = 0x0047B150;
	__int8 unitGetRace(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_unitGetRace
			MOV result, EAX
			POPAD
		}
		return (__int8)result;
	}

	const uint32 Func_getStatTxtString = 0x004C36F0;
	char* getStatTxtString(__int16 a1) {
		static int result;
		__asm {
			PUSHAD
			MOVZX CX, a1
			CALL Func_getStatTxtString
			MOV result, EAX
			POPAD
		}
		return (char*)result;
	}

	// For statsWeaponLevel / statsArmorLevel
	const uint32 Func_getArmorCount = 0x00453FC0;
	char getArmorCount(CUnit* unit, int32 unused) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EDX, unused
			MOV EAX, unit
			CALL Func_getArmorCount
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	const uint32 Func_showDialog = 0x004186A0;
	void showDialog(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_showDialog
			POPAD
		}
	}

	const uint32 Func_disableDialog = 0x00418640;
	int32 disableDialog(BinDlg* dialog) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ESI, dialog
			CALL Func_disableDialog
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_updateDialog = 0x0041C400;
	void updateDialog(BinDlg* dialog) {
		__asm {
			PUSHAD
			MOV EAX, dialog
			CALL Func_updateDialog
			POPAD
		}
	}

} //unnamed namespace

//End of helper functions
