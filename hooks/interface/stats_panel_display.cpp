#include "stats_panel_display.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/buttonsets.h"

namespace hooks
{
	bool StatLobotomyMineCount(BinDlg* dialog, u32 index)
	{
		auto activeUnit = *activePortraitUnit;
		// Originally had code for research, removed
		sprintf_s(statusscreen_infobtn[index], 8, "%d ", activeUnit->vulture.spiderMineCount);
		if (dialog->controlType)
		{
			dialog = dialog->parent;
		}
		auto childDialog = dialog->childrenDlg;
		bool jump = false;
		if (childDialog)
		{
			while (childDialog->index != index + 9)
			{
				childDialog = childDialog->next;
				if (!childDialog)
				{
					childDialog = nullptr;
					break;
				}
			}
		}
		else
		{
			childDialog = nullptr;
		}
		scbw::showDialog(childDialog);
		scbw::disableDialog(childDialog);
		auto statData = childDialog->statButtonUser;
		auto v7 = *cmdicons_grp;
		childDialog->graphic = UnitId::TerranLobotomyMine - 1; // or set icon id
		statData->button_grp = v7;
//		statData->grp_frame = UnitId::LobotomyMine;
		statData->grp_frame = 13;
		statData->type = 3;
//		statData->id = UnitId::LobotomyMine;
		statData->id = 13;
		auto v8 = childDialog->flags;
		childDialog->pszText = statusscreen_infobtn[index];
		if (!(v8 & 1))
		{
			childDialog->flags = v8 | 1;
			scbw::updateDialog(childDialog);
		}
		return 1;
	}

	// Attempting to be true to the original result in unoptimized code
	void stats_panel_display(BinDlg* dialog)
	{
		BinDlg* current_dialog;
		Unit* activeUnit = *activePortraitUnit;
		u32 index = 0;

		if (!(activeUnit->status & UnitStatus::IsHallucination)
			|| (!*IS_IN_REPLAY && activeUnit->playerId != *LOCAL_NATION_ID))
		{
			bool bNoAirWeaponDisplay;
			u32 airWeapon;
			u8 airWeaponMainUnit;
			u8 airWeaponSubunit;
			u16 unitId = helper::getLastQueueSlotType(activeUnit);
			if (units_dat::ArmorUpgrade[unitId] != UpgradeId::GlobalUpgrade60)
			{
				helper::StatsArmorLevel(dialog, index);
				index++;
			}
			if (units_dat::ShieldsEnabled[activeUnit->id])
			{
				helper::StatsShieldLevel(dialog, index);
				index++;
			}
			if (activeUnit->id != UnitId::ZergLakizilisk
				|| activeUnit->status & UnitStatus::Burrowed)
			{
				bool bNoGroundWeaponDisplay = false;
				u32 groundWeapon;
				u8 groundWeaponMainUnit = units_dat::GroundWeapon[activeUnit->id];
				if (activeUnit->id == UnitId::TerranMadrigal
					&& scbw::get_generic_rust_value(activeUnit, RustId::ToggleTempoChange)
					&& groundWeaponMainUnit == WeaponId::MadrigalFuriaMissiles)
				{
					groundWeaponMainUnit = WeaponId::MadrigalImperiosoMissiles;
				}

				u8 groundWeaponSubunit;
				if (groundWeaponMainUnit == WeaponId::None)
				{
					if (activeUnit->subunit == nullptr)
					{
						bNoGroundWeaponDisplay = true;
					}
					else
					{
						groundWeaponSubunit = units_dat::GroundWeapon[activeUnit->subunit->id];
					}
				}
				else
				{
					groundWeaponSubunit = groundWeaponMainUnit;
				}

				if (!bNoGroundWeaponDisplay)
				{
					if (groundWeaponSubunit == WeaponId::None)
					{
						bNoGroundWeaponDisplay = true; // 00426D19
					}
					else
						if (activeUnit->id == UnitId::ZergLakizilisk && !(activeUnit->status & UnitStatus::Burrowed))
						{
							groundWeapon = UnitId::None; // 00426D2A
						}
						else
							if (groundWeaponMainUnit != WeaponId::None)
							{
								groundWeapon = groundWeaponMainUnit; // 00426D35
							}
							else
								if (activeUnit->subunit == nullptr)
								{
									groundWeapon = WeaponId::None; // 00426D41
								}
								else
								{
									groundWeapon = units_dat::GroundWeapon[activeUnit->subunit->id]; // 00426D51
								}
					if (!bNoGroundWeaponDisplay)
					{
						// 00426D54
						helper::StatsWeaponLevel(dialog, index, groundWeapon);
						index++;

						// Display additional weapons
						if (activeUnit->id == UnitId::TerranCentaur)
						{
							helper::StatsWeaponLevel(dialog, index, WeaponId::CentaurSunchaserMissiles);
							index++;
						}
						else if (activeUnit->id == UnitId::SharedTarasque)
						{
							helper::StatsWeaponLevel(dialog, index, WeaponId::TarasqueAcceleratedPlasmid);
							index++;
						}
					}
				}
			}

			// 00426D67
			bNoAirWeaponDisplay = false;
			airWeaponMainUnit = units_dat::AirWeapon[activeUnit->id];
			if (airWeaponMainUnit != WeaponId::None)
			{
				airWeaponSubunit = airWeaponMainUnit; // 00426D79
			}
			else if (activeUnit->subunit == nullptr)
			{
				bNoAirWeaponDisplay = true; // 00426D82
			}
			else
			{
				airWeaponSubunit = units_dat::AirWeapon[activeUnit->subunit->id]; // 00426D8C
			}

			if (!bNoAirWeaponDisplay)
			{
				if (airWeaponSubunit == WeaponId::None)
				{
					bNoAirWeaponDisplay = true; // 00426D94
				}
				else if (airWeaponMainUnit != WeaponId::None)
				{
					airWeapon = airWeaponMainUnit;	// 00426DA2
				}
				else
				{
					if (activeUnit->subunit == nullptr)
					{
						airWeapon = WeaponId::None;	// 00426DAF
					}
					else
					{
						airWeapon = units_dat::AirWeapon[activeUnit->subunit->id];
					}
				}
				if (!bNoAirWeaponDisplay)
				{
					u8 groundWeapon;

					// 00426DBE
					if (activeUnit->id == UnitId::ZergLakizilisk && !(activeUnit->status & UnitStatus::Burrowed))
					{
						groundWeapon = WeaponId::None;
					}
					else
					{
						groundWeapon = units_dat::GroundWeapon[activeUnit->id];
						if (groundWeapon == WeaponId::None && activeUnit->subunit != nullptr)
						{
							groundWeapon = units_dat::GroundWeapon[activeUnit->subunit->id]; // 00426DE6
						}
					}

					// 00426DF0
					if (airWeapon == groundWeapon)
					{
						bNoAirWeaponDisplay = true;
					}
					else if (airWeaponMainUnit != WeaponId::None)
					{
						airWeapon = airWeaponMainUnit;	// 00426DFA
					}
					else if (airWeaponMainUnit == WeaponId::None)
					{ // 00426DFF
						if (activeUnit->subunit == nullptr)
						{
							airWeapon = WeaponId::None;	// 00426E06
						}
						else
						{
							airWeapon = units_dat::AirWeapon[activeUnit->subunit->id]; // 00426E16
						}
					}
					if (!bNoAirWeaponDisplay)
					{ // 00426E19
						helper::StatsWeaponLevel(dialog, index, airWeapon);
						index++;
					}
				}
			}

			// 00426E31
			if (activeUnit->id == UnitId::ProtossSolarion
				|| activeUnit->id == UnitId::ProtossAccantor)
			{ // Originally had hardcode for Gantrithor and Warbringer
				helper::StatHangerCount(dialog, index);
				index++;
			}

			if (activeUnit->id == UnitId::TerranVulture)
			{ // Originally also had hardcode for Raynor Vulture
				if (helper::StatSpidermineCount(dialog, index))
				{
					index++;
				}
			}
			if (activeUnit->id == UnitId::TerranEidolon)
			{
				if (StatLobotomyMineCount(dialog, index))
				{
					index++;
				}
			}
			unitId = helper::getLastQueueSlotType(activeUnit);
			if (unitId == UnitId::TerranNuclearSilo)
			{
				helper::StatsNukesCount(dialog, index);
				index++;
			}
		}

		// 00426E96
		if (dialog->controlType == DialogControlTypes::DialogBoxDC)
		{
			current_dialog = dialog;
		}
		else
		{
			current_dialog = dialog->parent;
		}
		if (current_dialog->childrenDlg != nullptr)
		{
			current_dialog = current_dialog->childrenDlg;
			while (current_dialog != nullptr && current_dialog->index != index + 9)
			{
				current_dialog = current_dialog->next;
			}
			while (index < 4 && current_dialog != nullptr)
			{
				helper::hideDialog(current_dialog);
				current_dialog = current_dialog->next;
				index++;
			}
		}
	}

	// Hooked to enable/disable rank display based on new unit definitions -- Pr0nogo
	char* getUnitRankString(Unit* unit) // 0042F370
	{
		u16 unitId = unit->id;
		if (units_dat::BaseProperty[unitId] & UnitProperty::NeutralAccessories
			|| scbw::unitIsTrap(unit))
		{
			return 0;
		}

		if (unit->playerId == 11) {
			return "";
		} else {
			return playerNameList->player[unit->playerId].player_name;
		}

		/*
		// Originally had Gantrithor - Tassadar hardcode here
		if (unitId != UnitId::TerranMinotaur
			|| !(units_dat::BaseProperty[unitId] & UnitProperty::Hero)
			|| !units_dat::MapStringId[unitId])
		{
			if (helper::unitGetRace(unit) == RaceId::Terran
				&& !scbw::unitIdIsShared(unit->id))
			{
				return helper::getStatTxtString((unit->rankIncrease + units_dat::Rank[unitId]) + 1302);
			}
			return 0;
		}
		if (statTxtTblNew->getString(570))
		{
			return "";
		}
		return (char*)statTxtTblNew->getString(571); // "Admiral"
		*/
	}

	void drawUnitKillsString(BinDlg* dialog) // 00425DD0
	{
		Unit* unit = *activePortraitUnit;
		helper::addTextToDialog(dialog, -21, playerNameList->player[unit->playerId].force_name);

		/*
		if (unit->id == UnitId::ZergSkryling
			|| unit->id == UnitId::ZergInfestedTerran
			|| unit->status & UnitStatus::IsHallucination
			&& unit->playerId == *LOCAL_NATION_ID
			&& !*IS_IN_REPLAY)
		{
			BinDlg* actual_dialog = dialog->controlType ? dialog->parent : dialog;
			BinDlg* child = actual_dialog->childrenDlg;
			while (child && child->index != 65515)
			{
				child = child->next;
			}
			if (child) {
				helper::hideDialog(child);
			}
		}
		else
		{
			if (helper::AI_UnitCanAttack(unit) || unit->killCount)
			{
				char* text = "";
				if (**statTxtBuffer > 0x2FBu)
				{
					text = (char *)*statTxtBuffer + (*statTxtBuffer)[764];
				}
				sprintf_s(kills_buffer, 260, "%s %d", text, unit->killCount);
				helper::addTextToDialog(dialog, -21, kills_buffer);
			}
			else
			{
				BinDlg* other = helper::getControlFromIndex(dialog, -21);
				helper::hideDialog(other);
			}
		}
		*/
	}

	// TODO: Fix progress bar drawing during addon construction, unit training
	// Hooked to draw armor icons to buildings - Pr0nogo
	void drawBuildingInfo(BinDlg* dialog) // 00427890
	{
		Unit* activeUnit = *activePortraitUnit;
		selectionGroupHP[0] = (*activePortraitUnit)->hitPoints;
		s32 shields = (*activePortraitUnit)->shields >> 8;
		*userInterfaceEnergy = (*activePortraitUnit)->energy;
		*userInterfaceShields = shields;
		*redrawTransportUI = helper::needRedrawUI();
		helper::genericStatusUpdateDrawnValues();
		s32 unitId = activeUnit->id;
		if (unitId == UnitId::ResourceMineralField 
			|| unitId == UnitId::ResourceMineralFieldType2
			|| unitId == UnitId::ResourceMineralFieldType3
			|| unitId == UnitId::ResourceVespeneGeyser)
		{
			goto LABEL_35;
		}
		if (!IS_IN_REPLAY && activeUnit->playerId != *LOCAL_NATION_ID)
		{
			goto LABEL_6;
		}
		if (!(activeUnit->status & UnitStatus::Completed))
		{
			scbw::print("drawProgressBar");
			helper::drawProgressBar(dialog);
			goto LABEL_32;
		}
		if (unitId == UnitId::TerranNuclearSilo)
		{
			scbw::print("drawNukeSiloInfo");
			helper::drawNukeSiloInfo(dialog);
			goto LABEL_32;
		}
		if (helper::unitIsActiveTransport(activeUnit))
		{
			scbw::print("unitIsActiveTransport");
			goto LABEL_6;
		}
		if (helper::isQueueSlotActive(activeUnit, 0))
		{
			scbw::print("drawTrainingProgress");
			helper::drawTrainingProgress(dialog);
			goto LABEL_32;
		}
		if (helper::isConstructingAddon(activeUnit) || helper::isAttemptingProtossBuild(activeUnit))
		{
			scbw::print("drawAddonProgress");
			helper::drawAddonProgress(dialog);
			goto LABEL_32;
		}
		if (activeUnit->building.techType != 44)
		{
			scbw::print("drawTechProgress");
			helper::drawTechProgress(dialog);
			goto LABEL_32;
		}
		if (activeUnit->building.upgradeType != 61)
		{
			scbw::print("drawUpgradeProgress");
			helper::drawUpgradeProgress(dialog);
			goto LABEL_32;
		}
		if (units_dat::SupplyProvided[unitId])
		{
			scbw::print("drawSupplyUnitInfo");
			helper::drawSupplyUnitInfo(dialog);
			goto LABEL_32;
		}
		if (helper::unitIsGeyser(activeUnit))
		{
		LABEL_35:
			scbw::print("drawGasQuantity");
			helper::drawGasQuantity(dialog);
			goto LABEL_32;
		}
		if (units_dat::AirWeapon[helper::getLastQueueSlotType(activeUnit)] == -126
			&& units_dat::GroundWeapon[helper::getLastQueueSlotType(activeUnit)] == -126
			&& units_dat::AirWeapon[activeUnit->id]
			&& units_dat::GroundWeapon[activeUnit->id]
			&& !units_dat::ShieldsEnabled[unitId])
		{
		LABEL_6:
			if (*statusScreenFunc != 4)
			{
				helper::hideOptionalStatusScreenControls(dialog);
				*statusScreenFunc = 4;
			}
			goto LABEL_32;
		}
		if (*statusScreenFunc != 3)
		{
			helper::hideOptionalStatusScreenControls(dialog);
			helper::showControlGroup(dialog, -20, 12);
			*statusScreenFunc = 3;
		}
		helper::drawDisplayButtons(dialog);
		helper::hideDialog(helper::getControlFromIndex(dialog, -21));
	LABEL_32:
		helper::setTextStr(dialog);
		helper::setUnitStatusStrText(dialog);
		helper::addTextToDialog(dialog, -5, getUnitName(*activePortraitUnit));
	}

	// Hooked to draw new values to icons -- Pr0nogo
	void statsArmorLevel(BinDlg* dialog, s32 unused, u16 buttonRelated)
	{ // 00425600
		BinDlg* v3;
		BinDlg* v4;
		void* v5;
		s32 v6;
		Unit* v7;
		u16 v8;
		u16 v9;
		s16 v10;
		u16 v11;
		s32 v12;
		u8 v13;

		v3 = dialog;
		v13 = helper::getArmorCount(*activePortraitUnit, unused);
		sprintf_s(statusscreen_infobtn[buttonRelated], 8, "%d ", v13);
		if (v3->controlType)
		{
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
		helper::showDialog(v4);
		helper::disableDialog(v4);
		v5 = cmdicons_grp;
		v6 = *v4->user;
		v7 = *activePortraitUnit;
		v4->graphic = 12;
		// TODO: Fix compiler error
//		*(s32*)v6 = v5;
		if (v7->status & UnitStatus::Completed
			|| (v8 = v7->buildQueue[(u8)v7->buildQueueSlot], !scbw::isZergBuildingMorphResult(v8)))
		{
			v9 = v7->id;
		}
		else
		{
			v9 = v7->buildQueue[(u8)v7->buildQueueSlot];
		}
		*(s16*)(v6 + 4) = upgrades_dat::UpgradeIcon[(u8)units_dat::ArmorUpgrade[v9]];
		*(s16*)(v6 + 6) = UnitStatus::Completed;
		if (v7->status & UnitStatus::Completed
			|| (v10 = v7->buildQueue[(u8)v7->buildQueueSlot], !scbw::isZergBuildingMorphResult(v8)))
		{
			v11 = v7->id;
		}
		else
		{
			v11 = v7->buildQueue[(u8)v7->buildQueueSlot];
		}
		*(s16*)(v6 + 8) = (u8)units_dat::ArmorUpgrade[v11];
		v4->pszText = statusscreen_infobtn[buttonRelated];
		v12 = v4->flags;
		if (!(v12 & UnitStatus::Completed))
		{
			v4->flags = v12 | UnitStatus::Completed;
			helper::updateDialog(v4);
		}
		*unkUpgradeCount += v13;
	}

	void statsWeaponLevel(BinDlg* dialog, u16 buttonRelated, u8 weaponId)
	{ // 00425790
		BinDlg* v3;
		s32 v4;
		u8 v5;
		BinDlg* v6;
		s32 v7;
		void* v8;
		s32 v9;
		s16 v10;

		v3 = dialog;
		v4 = (*activePortraitUnit)->playerId;
		if (weapons_dat::DamageUpgrade[weaponId] >= 0x2Eu)
		{
//			v5 = *upgradesLevelBW[v4][weapons_dat::DamageUpgrade[weaponId]];
		}
		else
		{
//			v5 = *upgradesLevelSC[v4][weapons_dat::DamageUpgrade[weaponId]];
		}
		v10 = v5;
		sprintf_s(statusscreen_infobtn[buttonRelated], 8, "%d ", v5);
		if (v3->controlType)
		{
			v3 = v3->parent;
		}
		v6 = v3->childrenDlg;
		if (v6)
		{
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
		helper::showDialog(v6);
		helper::disableDialog(v6);
		//		v7 = v6->user;
		v8 = cmdicons_grp;
		v6->graphic = 12;
		//		*(s32*)v7 = v8;
		*(s16*)(v7 + 4) = weapons_dat::Icon[weaponId];
		*(s16*)(v7 + 6) = 0;
		*(s16*)(v7 + 8) = weaponId;
		v9 = v6->flags;
		v6->pszText = statusscreen_infobtn[buttonRelated];
		if (!(v9 & 1))
		{
			v6->flags = v9 | 1;
			helper::updateDialog(v6);
		}
		*unkUpgradeCount += v10;
	}

	// Hooked to change the name of new building morphs - Pr0nogo
	const char* getUnitName(Unit* unit)
	{
		u16 queue = unit->buildQueue[unit->buildQueueSlot];
		if (unit->status & UnitStatus::Completed
			|| !scbw::isZergBuildingMorphResult(queue)) // Fixes lardcode
		{
			return helper::getUnitNameStatString(unit->id);
		}
		else
		{
			return helper::getUnitNameStatString(queue);
		}
	}
}