#include "stats_panel_display.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
#include "hooks/interface/buttonsets.h"

namespace hooks
{
	bool stat_lobotomy_mine_count(BinDlg* dialog, u32 index)
	{
		auto activeUnit = *activePortraitUnit;
		// Originally had code for research, removed
		sprintf_s(statusscreen_infobtn[index], 8, "%d ", activeUnit->vulture.spiderMineCount);
		if (dialog->controlType)
		{
			dialog = dialog->parent;
		}
		auto childDialog = dialog->children_dialog;
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
		scbw::show_dialog(childDialog);
		scbw::disable_dialog(childDialog);
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
			scbw::update_dialog(childDialog);
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
			|| (!*IS_IN_REPLAY && activeUnit->player != *LOCAL_NATION_ID))
		{
			bool bNoAirWeaponDisplay;
			u32 airWeapon;
			u8 airWeaponMainUnit;
			u8 airWeaponSubunit;
			u16 unitId = helper::get_last_queue_slot_id(activeUnit);
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
					else if (activeUnit->id == UnitId::ZergLakizilisk && !(activeUnit->status & UnitStatus::Burrowed))
					{
						groundWeapon = UnitId::None; // 00426D2A
					}
					else if (groundWeaponMainUnit != WeaponId::None)
					{
						groundWeapon = groundWeaponMainUnit; // 00426D35
					}
					else if (activeUnit->subunit == nullptr)
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
				|| activeUnit->id == UnitId::ProtossAccantor) // Originally had hardcode for Gantrithor and Warbringer
			{
				helper::StatHangerCount(dialog, index);
				index++;
			}

			if (activeUnit->id == UnitId::TerranVulture) // Originally also had hardcode for Raynor Vulture
			{
				if (helper::StatSpidermineCount(dialog, index))
				{
					index++;
				}
			}
			if (activeUnit->id == UnitId::TerranEidolon)
			{
				if (stat_lobotomy_mine_count(dialog, index))
				{
					index++;
				}
			}
			unitId = helper::get_last_queue_slot_id(activeUnit);
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
		if (current_dialog->children_dialog != nullptr)
		{
			current_dialog = current_dialog->children_dialog;
			while (current_dialog != nullptr && current_dialog->index != index + 9)
			{
				current_dialog = current_dialog->next;
			}
			while (index < 4 && current_dialog != nullptr)
			{
				helper::hide_dialog(current_dialog);
				current_dialog = current_dialog->next;
				index++;
			}
		}
	}

	// Hooked to enable/disable rank display based on new unit definitions -- Pr0nogo
	char* get_unit_rank_string(Unit* unit) // 0042F370
	{
		u16 unitId = unit->id;
		if (units_dat::BaseProperty[unitId] & UnitProperty::NeutralAccessories)
		//	|| scbw::unit_is_trap(unit))
		{
			return 0;
		}

		if (unit->player == 11) {
			return "";
		}
		else {
			return playerNameList->player[unit->player].player_name;
		}

		/*
		// Originally had Gantrithor - Tassadar hardcode here
		if (unitId != UnitId::TerranMinotaur
			|| !(units_dat::BaseProperty[unitId] & UnitProperty::Hero)
			|| !units_dat::MapStringId[unitId])
		{
			if (helper::unitGetRace(unit) == RaceId::Terran
				&& !scbw::unit_id_is_shared(unit->id))
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

	void draw_unit_kills_string(BinDlg* dialog) // 00425DD0
	{
		Unit* unit = *activePortraitUnit;

		static char buffer[256] = {};
		strcpy_s(buffer, playerNameList->player[unit->player].force_name);
		helper::add_text_to_dialog(dialog, -21, buffer);

		/*
		if (unit->id == UnitId::ZergSkryling
			|| unit->id == UnitId::ZergInfestedTerran
			|| unit->status & UnitStatus::IsHallucination
			&& unit->playerId == *LOCAL_NATION_ID
			&& !*IS_IN_REPLAY)
		{
			BinDlg* actual_dialog = dialog->controlType ? dialog->parent : dialog;
			BinDlg* child = actual_dialog->children_dialog;
			while (child && child->index != 65515)
			{
				child = child->next;
			}
			if (child) {
				helper::hide_dialog(child);
			}
		}
		else
		{
			if (helper::ai_unit_can_attack(unit) || unit->killCount)
			{
				char* text = "";
				if (**statTxtBuffer > 0x2FBu)
				{
					text = (char *)*statTxtBuffer + (*statTxtBuffer)[764];
				}
				sprintf_s(kills_buffer, 260, "%s %d", text, unit->killCount);
				helper::add_text_to_dialog(dialog, -21, kills_buffer);
			}
			else
			{
				BinDlg* other = helper::get_control_from_index(dialog, -21);
				helper::hide_dialog(other);
			}
		}
		*/
	}

	// TODO: Fix progress bar drawing during addon construction, unit training
	// Hooked to draw armor icons to buildings. - Pr0nogo
	void draw_building_info(BinDlg* dialog) // 00427890
	{
		Unit* activeUnit = *activePortraitUnit;
		selectionGroupHP[0] = (*activePortraitUnit)->hitpoints;
		s32 shields = (*activePortraitUnit)->shields >> 8;
		*userInterfaceEnergy = (*activePortraitUnit)->energy;
		*userInterfaceShields = shields;
		*redrawTransportUI = helper::ui_needs_redraw();
		helper::generic_status_update_drawn_values();
		s32 unitId = activeUnit->id;
		if (unitId == UnitId::ResourceMineralField 
			|| unitId == UnitId::ResourceMineralFieldType2
			|| unitId == UnitId::ResourceMineralFieldType3
			|| unitId == UnitId::ResourceVespeneGeyser)
		{
			goto LABEL_35;
		}
		if (!IS_IN_REPLAY && activeUnit->player != *LOCAL_NATION_ID)
		{
			goto LABEL_6;
		}
		if (!(activeUnit->status & UnitStatus::Completed))
		{
			scbw::print("draw_progress_bar");
			helper::draw_progress_bar(dialog);
			goto LABEL_32;
		}
		if (unitId == UnitId::TerranNuclearSilo)
		{
			scbw::print("draw_nuke_silo_info");
			helper::draw_nuke_silo_info(dialog);
			goto LABEL_32;
		}
		if (helper::unit_is_active_transport(activeUnit))
		{
			scbw::print("unitIsActiveTransport");
			goto LABEL_6;
		}
		if (helper::is_queue_slot_active(activeUnit, 0))
		{
			scbw::print("draw_training_progress");
			helper::draw_training_progress(dialog);
			goto LABEL_32;
		}
		if (helper::is_constructing_addon(activeUnit) || helper::is_attempting_protoss_build(activeUnit))
		{
			scbw::print("draw_addon_progress");
			helper::draw_addon_progress(dialog);
			goto LABEL_32;
		}
		if (activeUnit->building.techType != 44)
		{
			scbw::print("draw_tech_progress");
			helper::draw_tech_progress(dialog);
			goto LABEL_32;
		}
		if (activeUnit->building.upgradeType != 61)
		{
			scbw::print("draw_upgrade_progress");
			helper::draw_upgrade_progress(dialog);
			goto LABEL_32;
		}
		if (units_dat::SupplyProvided[unitId])
		{
			scbw::print("draw_supply_unit_info");
			helper::draw_supply_unit_info(dialog);
			goto LABEL_32;
		}
		if (helper::unit_is_geyser(activeUnit))
		{
		LABEL_35:
			scbw::print("draw_gas_quantity");
			helper::draw_gas_quantity(dialog);
			goto LABEL_32;
		}
		if (units_dat::AirWeapon[helper::get_last_queue_slot_id(activeUnit)] == -126
			&& units_dat::GroundWeapon[helper::get_last_queue_slot_id(activeUnit)] == -126
			&& units_dat::AirWeapon[activeUnit->id]
			&& units_dat::GroundWeapon[activeUnit->id]
			&& !units_dat::ShieldsEnabled[unitId])
		{
		LABEL_6:
			if (*statusScreenFunc != 4)
			{
				helper::hide_optional_status_screen_controls(dialog);
				*statusScreenFunc = 4;
			}
			goto LABEL_32;
		}
		if (*statusScreenFunc != 3)
		{
			helper::hide_optional_status_screen_controls(dialog);
			helper::show_control_group(dialog, -20, 12);
			*statusScreenFunc = 3;
		}
		helper::draw_display_buttons(dialog);
		helper::hide_dialog(helper::get_control_from_index(dialog, -21));
	LABEL_32:
		helper::set_text_string(dialog);
		helper::set_unit_status_string_text(dialog);
		helper::add_text_to_dialog(dialog, -5, get_unit_name(*activePortraitUnit));
	}

	// Hooked to draw new values to icons. -- Pr0nogo
	void stats_armor_level(BinDlg* dialog, s32 unused, u16 buttonRelated) // 00425600
	{
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
		v4 = *(BinDlg**)&v3->children_dialog;
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
		helper::show_dialog(v4);
		helper::disable_dialog(v4);
		v5 = cmdicons_grp;
		v6 = *v4->user;
		v7 = *activePortraitUnit;
		v4->graphic = 12;
		// TODO: Fix compiler error
	//	*(s32*)v6 = v5;
		if (v7->status & UnitStatus::Completed
			|| (v8 = v7->buildQueue[(u8)v7->buildQueueSlot], !scbw::is_zerg_building_morph_result(v8)))
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
			|| (v10 = v7->buildQueue[(u8)v7->buildQueueSlot], !scbw::is_zerg_building_morph_result(v8)))
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
			helper::update_dialog(v4);
		}
		*unkUpgradeCount += v13;
	}

	void stats_weapon_level(BinDlg* dialog, u16 buttonRelated, u8 weaponId) // 00425790
	{
		BinDlg* v3;
		s32 v4;
		u8 v5;
		BinDlg* v6;
		s32 v7;
		void* v8;
		s32 v9;
		s16 v10;

		v3 = dialog;
		v4 = (*activePortraitUnit)->player;
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
		v6 = v3->children_dialog;
		if (v6)
		{
			while (v6->index != buttonRelated + 9)
			{
				v6 = v6->next;
				if (!v6)
				{
					goto LABEL_9;
				}
			}
		}
		else
		{
	LABEL_9:
			v6 = 0;
		}
		helper::show_dialog(v6);
		helper::disable_dialog(v6);
	//	v7 = v6->user;
		v8 = cmdicons_grp;
		v6->graphic = 12;
	//	*(s32*)v7 = v8;
		*(s16*)(v7 + 4) = weapons_dat::Icon[weaponId];
		*(s16*)(v7 + 6) = 0;
		*(s16*)(v7 + 8) = weaponId;
		v9 = v6->flags;
		v6->pszText = statusscreen_infobtn[buttonRelated];
		if (!(v9 & 1))
		{
			v6->flags = v9 | 1;
			helper::update_dialog(v6);
		}
		*unkUpgradeCount += v10;
	}

	// Hooked to change the name of new building morphs. - Pr0nogo
	const char* get_unit_name(Unit* unit)
	{
		u16 queue = unit->buildQueue[unit->buildQueueSlot];
		if (unit->status & UnitStatus::Completed
			|| !scbw::is_zerg_building_morph_result(queue)) // Fixes lardcode
		{
			return helper::get_unit_name_stat_string(unit->id);
		}
		else
		{
			return helper::get_unit_name_stat_string(queue);
		}
	}

	static u32 tooltip_time_cost;

	// 00459360 004593B0 004597D5 - Hooked to add time costs to tooltip info. -Veeq7
	void reset_tooltip_info() {
		helper::refresh_selection_screen();

		*tooltip_ore_cost = 0;	
		*tooltip_gas_cost = 0;
		*tooltip_p_energy_cost = 0;
		*tooltip_t_energy_cost = 0;
		*tooltip_z_energy_cost = 0;
		*tooltip_p_supply_cost = 0;
		*tooltip_t_supply_cost = 0;
		*tooltip_z_supply_cost = 0;

		tooltip_time_cost = 0;
	}

	// 00458800 - Same as above. -Veeq7
	void set_tooltip_info(u32 mineral_cost, u32 gas_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u32 z_supply) {
		*tooltip_ore_cost = mineral_cost;	
		*tooltip_gas_cost = gas_cost;
		*tooltip_p_energy_cost = p_energy;
		*tooltip_t_energy_cost = t_energy;
		*tooltip_z_energy_cost = z_energy;
		*tooltip_p_supply_cost = p_supply;
		*tooltip_t_supply_cost = t_supply;
		*tooltip_z_supply_cost = z_supply;

		tooltip_time_cost = 0;
	}

	// 00459531 - Same as above. -Veeq7
	void set_tooltip_info_unit(u32 unit_id) {
		u32 mineral_cost = units_dat::MineralCost[unit_id];
		u32 gas_cost = units_dat::GasCost[unit_id];
		u32 time_cost = units_dat::TimeCost[unit_id] / 24;

		helper::set_tooltip_info(mineral_cost, gas_cost, 0, 0, 0, 0, 0, 0);
		tooltip_time_cost = time_cost;
	}

	// Same as above. -Pr0nogo
	void draw_tooltip_with_icon(Box16* area) // 00458EF0
	{
		Box16 result = {};
		s32 right_dim = 2;
		Box16 frame_dims = {};
		s32 field_index = 0;
		u32 x_max = 2;
		GrpHead* stat_icon_offset = 0;
		u32 iterations = 0;
		char dest;
		bool draw = false;
		do
		{
			if (*(&tooltip_ore_cost + field_index))
			{
				u8 stat_icon = grp_stat_res_icon_mapping[field_index];
				if (stat_icon == 1)
				{
					stat_icon = *console_index + 1;
				}
				if (stat_icon >= icons_grp->frameCount)
				{
					stat_icon = 0;
				}
				u16 frame_bottom = icons_grp->frames[stat_icon].width;
				u16 frame_right = icons_grp->frames[stat_icon].height;
				u32 frame_offset = icons_grp->frames[stat_icon].data_offset;
				frame_dims.top = frame_bottom;
				if (frame_offset < 0)
				{
					frame_offset &= 0x7FFFFFFFu;
					frame_right += 256;
				}
				stat_icon_offset = (icons_grp + frame_offset);
				Box16 frame_area = {};
				u16 uninitialized = 0;
				u16 frame_right_minus = frame_right - 1;
				u16 bottom_dim = area->bottom;
				u16 frame_bottom_var = frame_bottom;
				frame_dims.bottom = frame_right;
				u16 frame_right_var = frame_right;
				u16 y_min_minus = frame_dims.top - 1;
				helper::bw_print_icon(&frame_area, &frame_dims, right_dim, bottom_dim);
				sprintf_s(&dest, 16, "%d", *(&tooltip_ore_cost + field_index));
				helper::bw_print_xy(&dest, frame_right + x_max + 2, area->bottom);
				field_index = iterations;
				x_max = required_area->right;
				draw = 1;
				right_dim = required_area->right;
			}
			iterations = ++field_index;
		} while (field_index < 8);
		if (draw)
		{
			if (right_dim > area->right)
			{
				area->right = right_dim;
			}
			area->bottom += frame_dims.top + 1;
		}
		return;
	}

	// Hooked to help with tooltip debugging. -Pr0nogo
	s32 stat_data_unit_wireframe_selection(BinDlg* dialog, Event* event) { // 004583E0
		BinDlg* dialog_var = dialog;
		if (event->type == 3) {
			helper::status_screen_update_tooltip_mouse_held(dialog, event);
		} else if (event->type == 14) {
			switch (event->extended_command_type) {
				case 0: {
					dialog->fxnUpdate = helper::stat_data_unit_wireframe_select_update;
					dialog->graphic = 0;
					dialog->user = (u32*)helper::SMemAlloc(8, "Starcraft\\SWAR\\lang\\statdata.cpp", 775, 0);
				} break;
				case 1: {
					void* user = dialog->user;
					if (user)
					{
						helper::SMemFree(user, "Starcraft\\SWAR\\lang\\statdata.cpp", 780, 0);
					}
				} break;
				case 2: {
					helper::status_screen_button(dialog, dialog);
					return 1;
				} break;
				case 4: {
					if (!(dialog->flags & 8))
					{
						break;
					}
					return 1;
				} break;
				case 6: {
					if (dialog == *control_under_mouse)
					{
						helper::refresh_selection_screen();
						*control_under_mouse = 0;
					}
				} break;
			}
		}

		return generic_interact_functions[dialog_var->controlType](dialog, event);
	}
}