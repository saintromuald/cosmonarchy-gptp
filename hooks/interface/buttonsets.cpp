#include "buttonsets.h"
#include "SCBW/api.h"
#include "globals.h"
#include "hooks/game_hooks.h"
#include "hooks/grafting/button_functions.h"

static bool player_has_unit_id(s32 player_id, u32 unit_id) {
	return Player::number_of_units_of_id(player_id, unit_id);
}

static bool player_has_completed_unit_id(s32 player_id, u32 unit_id) {
	return Player::number_of_completed_units_of_id(player_id, unit_id);
}

static bool player_has_completed_landed_unit_id(s32 player_id, u32 unit_id) {
	return Player::number_of_completed_landed_units_of_id(player_id, unit_id);
}

// Example on how to create a custom button set.
// This makes a copy of the Marine buttonset.
// Note that the function would have to be duplicated for each custom buttonset
ButtonSet* get_custom_buttonset()
{
	// Use static variables to avoid memory allocation trouble
	static ButtonSet customButtonSet;
	static Button customButtonsArray[50];

	// Since it's a copy of the marine set, it would have the same amount of buttons
	customButtonSet.buttons_in_set = g_buttonset_table[UnitId::TerranMaverick].buttons_in_set;

	// connected_unit should usually be init at 0xFFFF.Almost unused (maybe not fully implemented).
	customButtonSet.connected_unit = 0xFFFF;

	// This should always be like this since the array is static
	customButtonSet.first_button = &customButtonsArray[0];

	// For each button in the array until reaching the amount of buttons_in_set, copy the data for each member of the button structure from the marine to the custom set.
	for (u32 i = 0; i < customButtonSet.buttons_in_set; i++)
	{
		customButtonsArray[i].action_func = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_func;
		customButtonsArray[i].action_string_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_string_id;
		customButtonsArray[i].action_var = g_buttonset_table[UnitId::TerranMaverick].first_button[i].action_var;
		customButtonsArray[i].icon_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].icon_id;
		customButtonsArray[i].position = g_buttonset_table[UnitId::TerranMaverick].first_button[i].position;
		customButtonsArray[i].requirement_func = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_func;
		customButtonsArray[i].requirement_string_id = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_string_id;
		customButtonsArray[i].requirement_var = g_buttonset_table[UnitId::TerranMaverick].first_button[i].requirement_var;
	}
	return &customButtonSet;
}

namespace hooks {
	static Unit* get_selected_unit() {
		Unit* active = *g_active_portrait_unit;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (unit && unit->status & UnitStatus::Completed) {
				return unit;
			}
		}
		return active;
	}

	// 00458BC0
	// Called (with a jump) as last instruction of update_buttonset
	// Will update the buttonset for the currently unit active type, or handle special cases of mixed selections
	void update_buttonset_ex() {
		u16* const BUTTONSET_CURRENT_BUTTONSETID = (u16*)0x0068C1C4;
		u8 count_workers = 0;
		u8 count_burrowables = 0;
		u8 count_larva = 0;
		u8 count_eggs  = 0;
		u8 count_hatches = 0;
		u8 count_zergs = 0;
		u8 count_buildings = 0;
		u8 count_attacking_buildings = 0;
		u8 count_lifted_buildings = 0;
		u16 active_button_set_id;
		u16 selected_button_set_id;
		u32 selected_properties;
		Unit* selected;

		Unit* active = get_selected_unit();
		active_button_set_id = active->current_buttonset;

		for (u32 i = 0; i < SelectionCap; ++i) { // count units to know if will be using special group buttonset (peons, cloakables...)
			Unit* selected = g_client_selection_group->unit[i];
			if (selected) {
				selected_button_set_id = selected->current_buttonset;

				selected_properties = units_dat::base_property[selected->id];
				if (selected_properties & UnitProperty::Worker) {
					count_workers++;
				}

				if (selected_properties & UnitProperty::Building) {
					count_buildings++;
					if (selected->get_ground_weapon() != WeaponId::None || selected->get_air_weapon() != WeaponId::None) {
						count_attacking_buildings++;
					}
					if (selected->status & UnitStatus::InAir) {
						count_lifted_buildings++;
					}
				}

				if (selected_properties & UnitProperty::Burrowable) {
					count_burrowables++;
				}

				if (scbw::unit_id_is_zerg_production(selected->id)) {
					count_hatches++;
				}

				if (units_dat::group_flags[selected->id].is_zerg) {
					count_zergs++;
				}

				if (selected->id == UnitId::ZergLarva) {
					count_larva++;
				}

				if (selected->id == UnitId::ZergEgg) {
					count_eggs++;
				}

				// if you use a custom buttonset, and are reusing the current_buttonset property of a active differently, change these conditions to take it in account
				if (selected->status & UnitStatus::Completed && selected_button_set_id != active_button_set_id) {
					ButtonSet* selected_button_set = hooks::get_buttonset(selected_button_set_id);
					ButtonSet* active_button_set = hooks::get_buttonset(active_button_set_id);
					if (active_button_set->first_button != selected_button_set->first_button) {
						if (selected_button_set->connected_unit != active_button_set_id) {
							// Kerrigan (Ghost) and Raynor (Vulture) have the corresponding normal active as "connected active"
							if (active_button_set->connected_unit == selected_button_set_id) {
								active_button_set_id = selected_button_set_id;
								*BUTTONSET_CURRENT_BUTTONSETID = selected_button_set_id;
							} else {
								*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupMixed;
							}
						}
					}
				}
			}
		}
		if (count_larva && count_larva + count_eggs == *g_client_selection_count) {
			*BUTTONSET_CURRENT_BUTTONSETID = UnitId::ZergLarva;
		}
		if (*BUTTONSET_CURRENT_BUTTONSETID != UnitId::Buttons_Blank) {
			if (count_workers == *g_client_selection_count) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupPeons;
			} else if (count_burrowables != 0 && count_zergs == *g_client_selection_count) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupBurrower; // non-zergs units prevent this, maybe they shouldn't?
			} else if (count_hatches == *g_client_selection_count) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::ZergHachirosk;
			} else if (count_lifted_buildings) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_GroupMixed; // TODO: Replace with Buttons_LiftedMixed or something
			} else if (count_attacking_buildings) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::ProtossWarden;
			} else if (count_buildings) {
				*BUTTONSET_CURRENT_BUTTONSETID = UnitId::TerranGoliathTurret;
			}
		}
	}

	// 00458D50
	// Not calling others converted to C functions
	// Probably making it so the button corresponding to current order has light on
	// Last function called within updateCurrentButtonSet
	void highlight_button()
	{
		BinDlg** const BUTTONSET_DIALOG = (BinDlg**)0x0068C148;
		u8* const BUTTONSET_SUBUNIT_MAINORDERID = (u8*)0x0068C1B8;
		u16* const BUTTONSET_MAINBUTTONGRAPHIC = (u16*)0x0068C1BC;
		u8* const BUTTONSET_UNIT_MAINORDERID = (u8*)0x0068C1E4;

		u8 currentUnitMainOrderId;
		u8 currentSubUnitMainOrderId;
		u16 currentUnitButtonIcon;
		u16 currentSubUnitButtonIcon;
		u16 usedButtonIcon;
		BinDlg* currentButtonDlg;

		currentUnitMainOrderId = *BUTTONSET_UNIT_MAINORDERID;
		currentUnitButtonIcon = orders_dat::button_icon[currentUnitMainOrderId];
		currentButtonDlg = (*BUTTONSET_DIALOG)->child_dialog;
		currentSubUnitMainOrderId = *BUTTONSET_SUBUNIT_MAINORDERID;

		// if the subunit holds the weapon, get the icon from it
		if (currentSubUnitMainOrderId != 0xFF)
		{
			currentSubUnitButtonIcon = orders_dat::button_icon[currentSubUnitMainOrderId];
		}
		else
		{
			currentSubUnitButtonIcon = 0xFFFF;
		}

		if (currentUnitButtonIcon != 0xFFFF)
		{
			if (currentSubUnitButtonIcon == 0xFFFF || currentUnitButtonIcon != IconId::Stop)
			{
				usedButtonIcon = currentUnitButtonIcon;
			}
			else
			{
				usedButtonIcon = currentSubUnitButtonIcon;
			}
		}
		else
		{
			usedButtonIcon = currentSubUnitButtonIcon;
		}

		if (currentButtonDlg != nullptr)
		{
			bool bStopLooping = false;
			do
			{
				// look for the button corresponding to the one used for the current mainOrder
				if (currentButtonDlg->flags & BinDlgFlags::Visible)
				{
					if (currentButtonDlg->graphic != usedButtonIcon)
					{
						currentButtonDlg = currentButtonDlg->next;
					}
					else
					{
						bStopLooping = true;
					}
				}
				else
				{
					currentButtonDlg = currentButtonDlg->next;
				}
			} while (!bStopLooping && currentButtonDlg != nullptr);
		}
		if (currentButtonDlg == nullptr)
		{
			*BUTTONSET_MAINBUTTONGRAPHIC = 0xFFFF;
		}
		else
		{
			if (!(currentButtonDlg->flags & BinDlgFlags::Unknown0))
			{
				currentButtonDlg->flags |= BinDlgFlags::Unknown0;
				scbw::update_dialog(currentButtonDlg);
			}
			*BUTTONSET_MAINBUTTONGRAPHIC = usedButtonIcon;
		}
	}

	// 00458DE0
	// Case when buttonset would use BUTTONSET_SPECIAL_BUTTONSETID
	// Use update_buttonset_ex() for individual units and groups
	// Called by update_current_buttonset and updateSelectedUnitData (004C38B0)
	void update_buttonset()
	{
		u16* const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)0x0068C14C;
		u16* const BUTTONSET_CURRENT_BUTTONSETID = (u16*)0x0068C1C4;
		u16* const BUTTONSET_SPECIAL_BUTTONSETID = (u16*)0x0068C1C8; // replay, cancel building...
		b8* const bDoingCancellableTargetOrder = (b8*)0x00641694;
		*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_Blank;
		*BUTTONSET_PORTRAIT_BUTTONSETID = UnitId::Buttons_Blank;
		if (!scbw::is_in_replay())
		{
			if (*g_client_selection_count != 0)
			{
				if (*bDoingCancellableTargetOrder == 0)
				{
					if (*g_is_placing_building)
					{
						*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_CancelPlaceBuilding;
					}
				}
				else
				{
					*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_Cancel;
				}
			}
		}
		else
		{
			if (*g_is_replay_paused != 0)
			{
				*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_ReplayPaused;
			}
			else
			{
				*BUTTONSET_SPECIAL_BUTTONSETID = UnitId::Buttons_ReplayPlaying;
			}
		}
		*BUTTONSET_CURRENT_BUTTONSETID = UnitId::Buttons_Blank;
		if ((*g_client_selection_count > 1) && (!scbw::is_in_replay()))
		{
			update_buttonset_ex();
		}
	}

	// Add active/unit units checks here and you can change the buttonset to any you created here in GPTP using something based on get_custom_buttonset method or a more customized way.
	// Note: if you use update_command_card, you can disable the injection of this function in the _inject.cpp file.
	ButtonSet* get_buttonset(s32 index)
	{
		return &(g_buttonset_table[index]);
	}

	// Main update_buttonset function because it call update_buttonset() and others
	void update_current_buttonset() // 004599A0
	{
		// Quick casting post handling
		if (hooks::quickcasting_signal) {
			hooks::quickcasting_signal = false;
			*orders_buttonset = hooks::quickcasting_command_card;
			*g_minimap_cursor = 0;
		}

		BinDlg** const BUTTONSET_DIALOG               = (BinDlg**)0x0068C148;
		u16*     const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)    0x0068C14C;
		u8*      const BUTTONSET_SUBUNIT_MAINORDERID  = (u8*)     0x0068C1B8;
		u16*     const BUTTONSET_CURRENT_BUTTONSETID  = (u16*)    0x0068C1C4;
		u16*     const BUTTONSET_SPECIAL_BUTTONSETID  = (u16*)    0x0068C1C8; // replay, cancel building...
		u8*      const BUTTONSET_UNIT_MAINORDERID     = (u8*)     0x0068C1E4;
		u32*     const bCanUpdateCurrentButtonSet     = (u32*)    0x0068C1B0;
		bool jumpTo59A38 = false;
		bool jumpTo59A9F = false;
		bool jumpTo59A83 = false;
		bool jumpTo59A86 = false;
		u8 mainOrderIdHolderUnit_EBP;
		u8 mainOrderIdHolderSubUnit_BL;
		u16 CX_Holder;
		if (!*bCanUpdateCurrentButtonSet)
		{
			// 004599B7
			bool bNeedUpdateButtonSet = false;
			if (*BUTTONSET_SPECIAL_BUTTONSETID != UnitId::Buttons_Blank)
			{
				if (*g_is_in_replay)
				{
					bNeedUpdateButtonSet = true;
				}
			}
			else
			{
				bNeedUpdateButtonSet = true;
			}
			if (bNeedUpdateButtonSet)
			{
				update_buttonset();
			}
		}
		CX_Holder = *BUTTONSET_SPECIAL_BUTTONSETID;
		if (*BUTTONSET_SPECIAL_BUTTONSETID == UnitId::Buttons_Blank)
		{
			CX_Holder = *BUTTONSET_CURRENT_BUTTONSETID;
			if (*BUTTONSET_CURRENT_BUTTONSETID == UnitId::Buttons_Blank)
			{
				if (!*g_active_portrait_unit)
				{
					jumpTo59A38 = true;
				}
				else
				{
					CX_Holder = (*g_active_portrait_unit)->current_buttonset;
				}
			}
		}

		if (!jumpTo59A38)
		{
			if (*BUTTONSET_PORTRAIT_BUTTONSETID == CX_Holder)
			{
				jumpTo59A38 = true;
			}
			else
			{
				*BUTTONSET_PORTRAIT_BUTTONSETID = CX_Holder;
				*bCanUpdateCurrentButtonSet = 1;
			}
			if (!jumpTo59A38)
			{
				helper::update_command_card();
				helper::refresh_button_tooltip();
				jumpTo59A9F = *g_active_portrait_unit;
				if (!jumpTo59A9F)
				{
					mainOrderIdHolderUnit_EBP = (*g_active_portrait_unit)->main_order_id;
					CX_Holder = mainOrderIdHolderUnit_EBP;
					if (!(*g_active_portrait_unit)->subunit)
					{
						jumpTo59A83 = true;
					}
					else
					{
						mainOrderIdHolderSubUnit_BL = (*g_active_portrait_unit)->subunit->main_order_id;
						jumpTo59A86 = true;
					}
				}
			}
		}

		if (jumpTo59A38)
		{
			if (*bCanUpdateCurrentButtonSet)
			{
				helper::update_command_card();
				helper::refresh_button_tooltip();
				jumpTo59A9F = !*g_active_portrait_unit;
				if (!jumpTo59A9F)
				{
					mainOrderIdHolderUnit_EBP = (*g_active_portrait_unit)->main_order_id;
					CX_Holder = mainOrderIdHolderUnit_EBP;
					if (!(*g_active_portrait_unit)->subunit)
					{
						jumpTo59A83 = true;
					}
					else
					{
						mainOrderIdHolderSubUnit_BL = (*g_active_portrait_unit)->subunit->main_order_id;
						jumpTo59A86 = true;
					}
				}
			}
		}

		if (!jumpTo59A83 && !jumpTo59A86 && !jumpTo59A9F)
		{
			// 00459A3C
			if (*g_active_portrait_unit)
			{
				bool jumpTo59ABD = false;
				mainOrderIdHolderUnit_EBP = (*g_active_portrait_unit)->main_order_id;
				if ((*g_active_portrait_unit)->subunit)
				{
					mainOrderIdHolderSubUnit_BL = (*g_active_portrait_unit)->subunit->main_order_id;
				}
				else
				{
					mainOrderIdHolderSubUnit_BL = 0xFF;
				}
				// 00459A55
				if (*BUTTONSET_UNIT_MAINORDERID == mainOrderIdHolderUnit_EBP)
				{
					// 00459A5D
					if (*BUTTONSET_SUBUNIT_MAINORDERID == mainOrderIdHolderSubUnit_BL)
					{
						jumpTo59ABD = true;
					}
				}
				if (!jumpTo59ABD)
				{
					// 00459A65
					// light up the button of the current order
					highlight_button();
					*BUTTONSET_UNIT_MAINORDERID = mainOrderIdHolderUnit_EBP;
					*BUTTONSET_SUBUNIT_MAINORDERID = mainOrderIdHolderSubUnit_BL;
					// light up the button of the current order
					highlight_button();
				}
			}
		}
		if (jumpTo59A83) // cannot be set if a jump beyond is set
		{
			mainOrderIdHolderSubUnit_BL = 0xFF;
			jumpTo59A86 = true;
		}
		if (jumpTo59A86)
		{
			// light up the button of the current order
			highlight_button();
			*BUTTONSET_UNIT_MAINORDERID = mainOrderIdHolderUnit_EBP;
			*BUTTONSET_SUBUNIT_MAINORDERID = mainOrderIdHolderSubUnit_BL;
			// light up the button of the current order
			highlight_button();
			jumpTo59A9F = true;
		}

		if (jumpTo59A9F)
		{
			if (*g_is_in_replay)
			{
				helper::replay_stat_buttons(*BUTTONSET_DIALOG);
			}
			// 00459AB3
			*bCanUpdateCurrentButtonSet = 0;
		}
	}

	s32 compare_button(const void* a, const void* b) {
		Button* button_a = (Button*)a;
		Button* button_b = (Button*)b;

		s32 pos_diff = button_a->position - button_b->position;
		if (pos_diff) return button_a->position - button_b->position;

		return button_a->icon_id - button_b->icon_id;
	}

	void update_command_card()
	{
		BinDlg** const BUTTONSET_DIALOG = (BinDlg**)0x0068C148;
		u16* const BUTTONSET_PORTRAIT_BUTTONSETID = (u16*)0x0068C14C;
		auto buttonset_id = *BUTTONSET_PORTRAIT_BUTTONSETID;
		BinDlg* current_dialog = *BUTTONSET_DIALOG;

		// Custom buttonset overrides
		if (buttonset_id == UnitId::TerranTreasury)
		{
			buttonset_id = UnitId::Special_StartLocation;
		}

		bool has_larva = false;
		bool has_egg = false;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (!unit) {
				break;
			}
			if (unit->id == UnitId::ZergLarva) {
				has_larva = true;
			}
			if (unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) {
				has_egg = true;
			}
		}

		// Fix larva category buttons not clearing for eggs
		if (!has_larva && has_egg) {
			if (buttonset_id == UnitId::Buttons_LarvaSwarm
			||  buttonset_id == UnitId::Buttons_LarvaAssault
			||  buttonset_id == UnitId::Buttons_LarvaControl
			||  buttonset_id == UnitId::Buttons_LarvaShared) {
				helper::BTNSACT_ChangeButtons(UnitId::ZergEgg);
				return;
			}
		}
		if (has_larva && !has_egg && buttonset_id == UnitId::ZergEgg) {
			helper::BTNSACT_ChangeButtons(UnitId::ZergLarva);
			return;
		}

		ButtonSet* const current_buttonset = get_buttonset(buttonset_id);
		u32* const unknown_0066FF60 = (u32*)0x0066FF60;
		bool bStopThere = false;
		bool jumpto59336 = false;
		bool jumpto5933C = false;
		bool jumpto5934E = false;
		s32 current_button_state;
		Button* current_button;
		u16 buttons_count = 0;
		current_button = current_buttonset->first_button;
		current_button_state = Button_State::Invisible;

		Unit* unit = get_selected_unit();
		bool changed = false;
		for (u32 i = 0; i < current_buttonset->buttons_in_set; ++i) {
			Button* button = &current_buttonset->first_button[i];
			if (button->icon_id == IconId::Cancel) {
				if (buttonset_id == UnitId::ZergLarva || buttonset_id == UnitId::ZergEgg || buttonset_id == UnitId::ZergCocoon) {
					button->position = 6;
				} else if (units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory) {
					button->position = 6;
				} else {
					button->position = 9;
				}
				changed = true;
			}
		}
		if (changed) {
			qsort(current_buttonset->first_button, current_buttonset->buttons_in_set, sizeof(Button), compare_button);
		}

		if (current_dialog->control_type != DialogControlTypes::DialogBoxDC)
		{
			current_dialog = current_dialog->parent;
		}
		// 00459203:
		current_dialog = current_dialog->child_dialog;
		while (current_dialog && current_dialog->index != 1)
		{
			current_dialog = current_dialog->next;
		}
		if (!current_dialog || current_dialog->index != 1)
		{
			bStopThere = true;
		}
		while (!bStopThere)
		{
			// 00459230
			if (current_dialog->index < 1)
			{
				bStopThere = true;
			}
			else if (current_dialog->index > 9)
			{
				jumpto5934E = true;
			}
			else
			{ // loop to ignore the first unused/disabled buttons
				current_button_state = Button_State::Invisible;
				while (current_button_state == Button_State::Invisible && (buttons_count < current_buttonset->buttons_in_set))
				{
					current_button_state = helper::req_check((u32)current_button->requirement_func, (u8)current_button->requirement_var, (u8)*g_local_nation_id, unit);

					// Fight that lardcode, tony!
					if (current_button->action_func == (ActFunc*)ButtonActions::Change_Displayed_Buttons)
					{
						if (buttonset_id == UnitId::ZergLarva
						|| buttonset_id == UnitId::TerranMason
						|| buttonset_id == UnitId::TerranTalos)
						{
							current_button_state = Button_State::Enabled;
						}
					}
					if (scbw::unit_id_is_zerg_production(unit->id) && current_button->icon_id == UnitId::TerranCohort) { // Remove this later
						current_button_state = Button_State::Enabled;
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Spell_Researched)
					{
						UnitExt* unitExt = get_unit_ext(unit);
						// Terran structure buttonset handling - Controlled Demolition / Emergency Detonation
						if (current_button->requirement_var == TechId::ControlledDemolition)
						{
							if (unitExt->timers[TimerControlledDemolition] || unitExt->timers[TimerEmergencyDetonation])
							{
								current_button_state = Button_State::Invisible;
							}
							else
							{
								current_button_state = Button_State::Enabled;
							}
						}
						else if (current_button->requirement_var == TechId::EmergencyDetonation)							
						{
							if (!unitExt->timers[TimerControlledDemolition] || unitExt->timers[TimerEmergencyDetonation])
							{
								current_button_state = Button_State::Invisible;
							}
							else
							{
								current_button_state = Button_State::Enabled;
							}
						}
						else if (current_button->requirement_var == TechId::HypnagogueThroughTheLookingGlass)
						{
							current_button_state = Button_State::Enabled;
						}
						else if (current_button->requirement_var == TechId::LarvalSubmenuSelectSwarm || current_button->requirement_var == TechId::LarvalSubmenuSelectAssault || current_button->requirement_var == TechId::LarvalSubmenuSelectControl)							
						{
							if ((units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory) || unit->id == UnitId::ZergLarva) {
								current_button_state = Button_State::Enabled;
							} else {
								current_button_state = Button_State::Invisible;
							}
						}
					}

					if ((unit->id == UnitId::ZergLarva || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) && has_egg && current_button->icon_id == IconId::SetRallyPoint) {
						current_button_state = Button_State::Enabled;
					}
					if (unit->id == UnitId::ZergCagrantColony && current_button->icon_id == IconId::SetRallyPoint) {
						current_button_state = Button_State::Enabled;
					}

					if (current_button->icon_id == IconId::Burrow || current_button->icon_id == IconId::LurkerBurrow) {
						bool has_burrowed = false;
						bool has_unburrowed = false;

						for (u32 i = 0; i < *g_client_selection_count; ++i) {
							Unit* unit = g_client_selection_group->unit[i];
							if (unit->status & UnitStatus::Burrowed) {
								has_burrowed = true;
							} else {
								has_unburrowed = true;
							}
						}

						if (has_unburrowed && !has_burrowed) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (current_button->icon_id == IconId::Unburrow || current_button->icon_id == IconId::LurkerUnburrow) {
						bool has_burrowed = false;
						bool has_unburrowed = false;

						for (u32 i = 0; i < *g_client_selection_count; ++i) {
							Unit* unit = g_client_selection_group->unit[i];
							if (unit->status & UnitStatus::Burrowed) {
								has_burrowed = true;
							} else {
								has_unburrowed = true;
							}
						}

						if (has_burrowed) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (current_button->icon_id == IconId::Larva) {
						if ((units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory) || unit->id == UnitId::ZergLarva) {
							current_button_state = Button_State::Enabled;
						} else {
							current_button_state = Button_State::Invisible;
						}
					}

					if (buttonset_id == UnitId::ZergLarva && !has_egg && current_button->icon_id == IconId::Cancel) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->action_func == (ActFunc*)ButtonActions::Create_AddOn && *g_client_selection_count > 1) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->action_func == (ActFunc*)ButtonActions::Attack_Building && unit->status & UnitStatus::InAir) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Attacking_Building && unit->status & UnitStatus::InAir) {
						current_button_state = Button_State::Invisible;
					}

					if (current_button->icon_id == IconId::Land && *g_client_selection_count > 1) {
						current_button_state = Button_State::Invisible;
					}

					bool has_addon = units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon;
					bool in_air = unit->status & UnitStatus::InAir;
					bool is_training = unit->build_queue[unit->build_queue_slot] != UnitId::None;
					bool is_burrowed = unit->status & UnitStatus::Burrowed;

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Can_Create_UnitorBuilding)
					{
						// Barracks buttonset handling
						if (unit->id == UnitId::TerranBarracks)
						{
							if (current_button->requirement_var == UnitId::TerranCleric
							||	current_button->requirement_var == UnitId::TerranShaman)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranMedbay)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranEidolon
							||		 current_button->requirement_var == UnitId::TerranSavant)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranCovertOps)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Captaincy buttonset handling
						else if (unit->id == UnitId::TerranCaptaincy)
						{
							if (current_button->requirement_var == UnitId::TerranSiren
								|| current_button->requirement_var == UnitId::TerranStriga)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranTinkerersTower)
									|| scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranOlympian
								|| current_button->requirement_var == UnitId::TerranAutocrat)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranGuildhall)
									|| scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Fulcrum buttonset handling
						else if (unit->id == UnitId::TerranFulcrum)
						{
							if (current_button->requirement_var == UnitId::TerranBlackjack
							||  current_button->requirement_var == UnitId::TerranMatador)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranScrapyard)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranPhalanxTankMode
							||		 current_button->requirement_var == UnitId::TerranMadrigal)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranMachineShop)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Iron Foundry buttonset handling
						if (unit->id == UnitId::TerranIronFoundry)
						{
							if (current_button->requirement_var == UnitId::TerranFutureStation)
							{
								if (!has_addon && !is_training)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							}
							if (current_button->requirement_var == UnitId::TerranClaymore
							||	current_button->requirement_var == UnitId::TerranPenumbra)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranFutureStation)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranAnticthon
							||		 current_button->requirement_var == UnitId::TerranAion)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranBioticBase)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Starport buttonset handling
						else if (unit->id == UnitId::TerranStarport)
						{
							if (current_button->requirement_var == UnitId::TerranSalamander
								|| current_button->requirement_var == UnitId::TerranSundog)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranMunitionsBay)
									|| scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranAzazel
								|| current_button->requirement_var == UnitId::TerranSeraph)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranControlTower)
									|| scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Nanite Assembly buttonset handling
						if (unit->id == UnitId::TerranNaniteAssembly)
						{
							if (current_button->requirement_var == UnitId::TerranAerie)
							{
								if (!has_addon && !is_training)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							}
							if (current_button->requirement_var == UnitId::TerranPegasus
							||	current_button->requirement_var == UnitId::TerranHippogriff)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranAerie)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
							else if (current_button->requirement_var == UnitId::TerranOberon
							||		 current_button->requirement_var == UnitId::TerranPhobos)
							{
								if (!scbw::all_selected_units_have_addon(UnitId::TerranParticleAccelerator)
								||	 scbw::unit_queue_full(unit))
								{
									current_button_state = Button_State::Invisible;
								}
								else
								{
									current_button_state = Button_State::Enabled;
								}
							}
						}

						// Experiment to see if extended unit ids can be forced on lol
						switch (unit->id)
						{
							case UnitId::TerranMason:
							case UnitId::TerranTalos:
							{
								if (unit->main_order_id != OrderId::BuildTerran && unit->main_order_id != OrderId::ConstructingBuilding) {
									switch (current_button->requirement_var)
									{
										case UnitId::TerranCommandCenter:
										case UnitId::TerranWatchdog:
										case UnitId::TerranReservoir:
										case UnitId::TerranBarracks:
										case UnitId::TerranFulcrum:
										case UnitId::TerranStarpad:
										case UnitId::TerranBunker:
										case UnitId::TerranEngineeringBay:
										{
											current_button_state = Button_State::Enabled;
										} break;
										case UnitId::TerranTreasury:
										case UnitId::TerranSentinel:
										case UnitId::TerranScienceFacility:
										case UnitId::TerranCaptaincy:
										case UnitId::TerranMantle:
										case UnitId::TerranStarport:
										{
											if (player_has_completed_landed_unit_id(unit->player, UnitId::TerranEngineeringBay)) {
												current_button_state = Button_State::Enabled;
											}
											else {
												current_button_state = Button_State::Disabled;
											}
										} break;
										case UnitId::TerranParthenon:
										case UnitId::TerranIonCannon:
										case UnitId::TerranQuantumInstitute:
										case UnitId::TerranApothecary:
										case UnitId::TerranIronFoundry:
										case UnitId::TerranNaniteAssembly:
										{
											if (player_has_completed_landed_unit_id(unit->player, UnitId::TerranScienceFacility)) {
												current_button_state = Button_State::Enabled;
											}
											else {
												current_button_state = Button_State::Disabled;
											}
										} break;
										/*
										case UnitId::TerranCapitol:
										case UnitId::TerranAmphitheater:
										{
											if (player_has_completed_landed_unit_id(unit->player, UnitId::TerranQuantumInstitute)) {
												current_button_state = Button_State::Enabled;
											}
											else {
												current_button_state = Button_State::Disabled;
											}
										}
										*/
										default:
										{
											current_button_state = Button_State::Invisible;
										} break;
									}
								}
							} break;
							case UnitId::TerranParthenon:
							{
								if (current_button->requirement_var == UnitId::TerranTalos
								||  current_button->requirement_var == UnitId::TerranArmistice
								||  current_button->requirement_var == UnitId::TerranPythean
								&&  unit->status & UnitStatus::GroundedBuilding)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::TerranMantle:
							{
								if (current_button->requirement_var == UnitId::TerranRamesses
								||  current_button->requirement_var == UnitId::TerranDurendal
								||  current_button->requirement_var == UnitId::TerranCataphract
								&&  unit->status & UnitStatus::GroundedBuilding)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::TerranApothecary:
							{
								if (current_button->requirement_var == UnitId::TerranDilettante
								||  current_button->requirement_var == UnitId::TerranSevenstep
								||  current_button->requirement_var == UnitId::TerranHypnagogue
								&&  unit->status & UnitStatus::GroundedBuilding)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossScribe:
							case UnitId::ProtossArtisan:
							{
								if (current_button->requirement_var == UnitId::ProtossNexus
								||  current_button->requirement_var == UnitId::ProtossPylon
								||  current_button->requirement_var == UnitId::ProtossAquifer
								||  current_button->requirement_var == UnitId::ProtossWarden
								||  current_button->requirement_var == UnitId::ProtossEmbassy
								||  current_button->requirement_var == UnitId::ProtossCrucible
								||  current_button->requirement_var == UnitId::ProtossMatrix
								||  current_button->requirement_var == UnitId::ProtossStridentStratum
								||  current_button->requirement_var == UnitId::ProtossGateway
								||  current_button->requirement_var == UnitId::ProtossCenotaph
								||  current_button->requirement_var == UnitId::ProtossGrandLibrary
								||  current_button->requirement_var == UnitId::ProtossRogueGallery
								||  current_button->requirement_var == UnitId::ProtossProstrationStage
								||  current_button->requirement_var == UnitId::ProtossSanctumOfSorrow
								||  current_button->requirement_var == UnitId::ProtossAncestralArchives
								||  current_button->requirement_var == UnitId::ProtossHallOfHeretics
								||  current_button->requirement_var == UnitId::ProtossLattice
								||  current_button->requirement_var == UnitId::ProtossAutomatonRegister
								||  current_button->requirement_var == UnitId::ProtossArdentAuthority
								||  current_button->requirement_var == UnitId::ProtossSequesteredIcon
								||  current_button->requirement_var == UnitId::ProtossAuxiliaryOasis
								||  current_button->requirement_var == UnitId::ProtossUnitaryUnion
								||  current_button->requirement_var == UnitId::ProtossSyntheticSynod
								||  current_button->requirement_var == UnitId::ProtossStargate
								||  current_button->requirement_var == UnitId::ProtossAstralOmen
								||  current_button->requirement_var == UnitId::ProtossArgosy
								||  current_button->requirement_var == UnitId::ProtossFleetBeacon
								||  current_button->requirement_var == UnitId::ProtossMonumentOfSin)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossGrandLibrary:
							{
								if (current_button->requirement_var == UnitId::ProtossAmaranth
								||  current_button->requirement_var == UnitId::ProtossHerald
								||  current_button->requirement_var == UnitId::ProtossAtreus
								||  current_button->requirement_var == UnitId::ProtossCantavis)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossAncestralArchives:
							{
								if (current_button->requirement_var == UnitId::ProtossArchon
								||  current_button->requirement_var == UnitId::ProtossOptecton
								||  current_button->requirement_var == UnitId::ProtossPatriarch)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossSequesteredIcon:
							{
								if (current_button->requirement_var == UnitId::ProtossAccantorOld
								||  current_button->requirement_var == UnitId::ProtossArchitect)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ProtossArgosy:
							{
								if (current_button->requirement_var == UnitId::ProtossMagister
								||  current_button->requirement_var == UnitId::ProtossEpigraph
								||  current_button->requirement_var == UnitId::ProtossDidact)
								{
									current_button_state = Button_State::Enabled;
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergLarva:
							{
								switch (current_button->requirement_var)
								{
									case UnitId::ZergDroleth:
									case UnitId::ZergOvileth:
									{
										current_button_state = Button_State::Enabled;
									} break;
									case UnitId::ZergOthstoleth:
									case UnitId::ZergAgrileth:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergOthstolOviform)
											|| player_has_unit_id(unit->player, UnitId::ZergAlaszilArbor)
											|| player_has_unit_id(unit->player, UnitId::ZergOrbothOmlia))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergZethrokor:
									case UnitId::ZergQuazilisk:
									case UnitId::ZergNathrokor:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergQuazrokPool)
											|| player_has_unit_id(unit->player, UnitId::ZergLiivralPond))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergHydralisk:
									case UnitId::ZergIzirokor:
									case UnitId::ZergSkithrokor:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergHydrokDen)
											|| player_has_unit_id(unit->player, UnitId::ZergGorgralSwamp))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergZoryusthaleth:
									case UnitId::ZergLakizilisk:
									case UnitId::ZergGorgrokor:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergZorkizShroud)
											|| player_has_unit_id(unit->player, UnitId::ZergUltrokCavern))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergMutalisk:
									case UnitId::ZergProtathalor:
									case UnitId::ZergCikralisk:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergMuthrokSpire)
											|| player_has_unit_id(unit->player, UnitId::ZergVithrathHaunt))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKagralisk:
									case UnitId::ZergAxitrilisk:
									case UnitId::ZergEvigrilisk:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergIzkagIteth))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergKonvilisk:
									case UnitId::ZergVilgorokor:
									case UnitId::ZergMatraleth:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergMatravilNest)
											|| player_has_unit_id(unit->player, UnitId::ZergAlmakisAntre))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergAlmaksalisk:
									case UnitId::ZergKeskathalor:
									case UnitId::ZergGeszithalor:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergGeszkathGrotto)
											|| player_has_unit_id(unit->player, UnitId::ZergAlkajChasm))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									case UnitId::ZergIsthrathaleth:
									{
										if (player_has_completed_unit_id(unit->player, UnitId::ZergElthisthMound))
										{
											current_button_state = Button_State::Enabled;
										}
										else
										{
											current_button_state = Button_State::Disabled;
										}
									} break;
									default:
									{
										current_button_state = Button_State::Invisible;
									} break;
								}
							} break;
							case UnitId::ZergMutalisk:
							{
								if (current_button->requirement_var == UnitId::ZergTosgrilisk)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergVithrathHaunt))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergCikralisk:
							{
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergGhitorokor)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergVithrathHaunt))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergIzirokor:
							{
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergTethzorokor)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergGorgralSwamp))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergLakizilisk:
							{
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergZobriolisk)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergUltrokCavern))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergGorgrokor:
							{
								if (current_button->requirement_var == UnitId::ZergZarcavrokor)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergUltrokCavern))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							case UnitId::ZergKonvilisk:
							{
								if (!is_burrowed && current_button->requirement_var == UnitId::ZergTetcaerokor)
								{
									if (player_has_completed_unit_id(unit->player, UnitId::ZergAlmakisAntre))
									{
										current_button_state = Button_State::Enabled;
									}
									else
									{
										current_button_state = Button_State::Disabled;
									}
								}
								else
								{
									current_button_state = Button_State::Invisible;
								}
							} break;
							default:
							{
							}
						}
					}

					if (current_button->requirement_func == (ReqFunc*)ButtonRequirements::Spell_Researched)
					{
						// Conditional hiding of custom ability icons
						if (current_button->icon_id == IconId::MasonFusionCutter
						&&  unit->main_order_id == OrderId::ConstructingBuilding) // Talos Salvage ability
						{
							current_button_state = Button_State::Invisible;
						}

						// Passive icons (yes, it's quite ugly)
						if (current_button->requirement_var == TechId::Healing && current_button->icon_id != IconId::Heal)
						{
							current_button_state = Button_State::Disabled;
							switch (unit->id)
							{
								// SHARED \\

								case UnitId::TerranCohort:
								{
									if (current_button->icon_id == IconId::TerranInfantryArmor) { // Cohort - Servile Vigor
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAspirant:
								{
									if (current_button->icon_id == IconId::ScarabDamage) { // Aspirant - Criticality
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranTarasque:
								{
									if (current_button->icon_id == IconId::GeszithalorAcidSpore) { // Tarasque - Fighting Flight
										current_button_state = Button_State::Enabled;
									}
								} break;

								// TERRAN \\

								case UnitId::TerranTalos:
								{
									if (current_button->icon_id == IconId::FusionCutterHarvest) { // Talos - Fusion Sealers
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHarakan:
								{
									if (current_button->icon_id == IconId::FlyerAttack) { // Harakan - Apollyoid Adhesive
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranShaman:
								{
									if (current_button->icon_id == IconId::Restoration) { // Shaman - Nanite Field
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranEidolon:
								{
									if (current_button->icon_id == IconId::OcularImplants) { // Eidolon - Somatic Implants
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSavant:
								{
									if (current_button->icon_id == IconId::DidactPhaseDisruptorCannon && scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 0) // Savant - Entropy Gauntlets
									{
										current_button_state = Button_State::Enabled;
									}
									else if (current_button->icon_id == IconId::IonThrusters && scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) > 0) // Savant - Power Siphon
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranMadcap:
								{
									if (current_button->icon_id == IconId::GoliathTwinAutocannons) // Madcap Maverick Feeders passive
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranHeracles:
								{
									if (current_button->icon_id == IconId::TitanReactor) // Heracles Nemean Reactors passive
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranApostle:
								{
									if (current_button->icon_id == IconId::SuicideScourge) // Apostle Last Breath passive
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranStriga:
								{
									if (current_button->icon_id == IconId::PsiAssault) // Striga - Fleeting Focus
									{
										current_button_state = Button_State::Enabled;
									}
									else if (current_button->icon_id == IconId::EMPShockwave) // Striga - Transient Travels
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranOlympian:
								{
									if (current_button->icon_id == IconId::OcularImplants) // Olympian - Dishonorable Discharge
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAutocrat:
								{
									if (current_button->icon_id == IconId::OcularImplants) // Autocrat - Controlled Opposition
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranDilettante:
								{
									if (current_button->icon_id == IconId::ApialSensors) // Dilettante - Beginner's Luck
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranSevenstep:
								{
									if (current_button->icon_id == IconId::DefensiveMatrix) // Virtuoso - Curtain Call
									{
										current_button_state = Button_State::Enabled;
									}
								}
								case UnitId::TerranAnticthon:
								{
									if (current_button->icon_id == IconId::RilirokorToxicSpores) // Anticthon - Counter-Proposal
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::TerranAncile:
								{
									if (current_button->icon_id == IconId::DefensiveMatrix) // Ancile - Providence
									{
										current_button_state = Button_State::Enabled;
									}
								} break;

								// PROTOSS \\

								case UnitId::ProtossHerald:
								{
									if (current_button->icon_id == IconId::RechargeShields) // Herald - Triumphant Rise
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossCharlatan:
								{
									if (current_button->icon_id == IconId::DemiurgePassiveWorship) // Charlatan - Leading the Blind
									{
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ProtossEmpress:
								{
									if (current_button->icon_id == IconId::ParticleBeamHarvest) // Empress - Anodyne Annex
									{
										current_button_state = Button_State::Enabled;
									}
								}
								break;

								// ZERG \\

								case UnitId::ZergVilgorokor:
								{
									if (current_button->icon_id == IconId::PneumatizedCaparace) { // Vilgorokor - Reconstitution
										current_button_state = Button_State::Enabled;
									}
								} break;
								case UnitId::ZergTetcaerokor:
								{
									if (current_button->icon_id == IconId::RoarUnused) { // Tetcaerokor - Virile Coating
										current_button_state = Button_State::Enabled;
									}
								} break;
							}

							if (unit->id == UnitId::TerranCuirass &&  
								current_button->icon_id == IconId::MoebiusReactor) // Cuirass Stonewall Protocol passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranCyclops &&  
								current_button->icon_id == IconId::VultureFragmentationGrenade) // Cyclops Fragmentary Shells passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranPhalanxTankMode &&  
								current_button->icon_id == IconId::TerranVehiclePlating) // Phalanx Babylon Plating passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranPhalanxSiegeMode &&  
								current_button->icon_id == IconId::TerranVehiclePlating) // Phalanx Babylon Plating passive
							{
								current_button_state = Button_State::Disabled; // Disable while sieged
							}
							else if (unit->id == UnitId::TerranMadrigal &&  
								current_button->icon_id == IconId::GeminiMissiles &&
								scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 0) // Madrigal Furia Salvo passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranMadrigal &&  
								current_button->icon_id == IconId::FusionCutterHarvest &&
								scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) > 0) // Madrigal Shrike Payload passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranPazuzu &&  
								current_button->icon_id == IconId::ScarabDamage) // Pazuzu Deconstruction passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranPenumbra &&  
								current_button->icon_id == IconId::WraithBurstLasers) // Penumbra Shroud Piercer passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranWraith &&  
								current_button->icon_id == IconId::WraithBurstLasers) // Wraith Capacitors passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranGorgon &&  
								current_button->icon_id == IconId::ArgusJewel) // Gorgon Petrification passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranValkyrie &&  
								current_button->icon_id == IconId::GraviticBoosters) // Valkyrie Charon Afterburners passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranTrojan &&  
								current_button->icon_id == IconId::Land) // Trojan Hotdrop passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranWyvern &&  
								current_button->icon_id == IconId::AfterburnersOnUnused) // Wyvern Reverse Thrust passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranSalamander &&  
								current_button->icon_id == IconId::HarakanFlameThrower) // Salamander Incendiary Payload passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranAzazel &&  
								current_button->icon_id == IconId::Restoration_Research) // Azazel Innervating Matrices passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranCentaur &&  
								current_button->icon_id == IconId::GeminiMissiles) // Centaur Sunchaser Salvo passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranMinotaur &&   
								current_button->icon_id == IconId::TerranShipPlating) // Minotaur Fore Castle passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranMinotaur &&   
								current_button->icon_id == IconId::SingularityCharge) // Minotaur Monoceros Targeting passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranMagnetar &&  
								current_button->icon_id == IconId::ArchonPsionicShockwave) // Magnetar Law of Attraction passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranPhobos &&   
								current_button->icon_id == IconId::KhaydarinCore) // Phobos Argent Capacitors passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranBlackjack &&   
								current_button->icon_id == IconId::ProtossAirPlating) // Blackjack Wear and Tear passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranClaymore &&   
								current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) // Claymore Prolonged Arms passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranClaymore &&   
								current_button->icon_id == IconId::FusionCutterHarvest) // Claymore Kelvin Munitions passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranSiren &&   
								current_button->icon_id == IconId::DidactPhaseDisruptorCannon) // Siren Discovered Attacks passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranMissileSilo &&  
								current_button->icon_id == IconId::RadioactiveUnused2) // Missile Silo Scorched Earth passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranQuarry &&   
								current_button->icon_id == IconId::ArgusJewel) // Quarry Meskalloid Stimulant passive
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::TerranSentinel &&  
								current_button->icon_id == IconId::AtreusDauntlessDisruptionCannon) // Sentinel Prolonged Arms passive
							{
								if (unit->status & UnitStatus::GroundedBuilding) {
									current_button_state = Button_State::Enabled;
								}
								else {
									current_button_state = Button_State::Invisible;
								}
							}
							else if (unit->id == UnitId::TerranIonCannon &&   
								current_button->icon_id == IconId::OpticalFlare) // Ion Cannon Ion Field passive
							{
								if (unit->status & UnitStatus::GroundedBuilding) {
									current_button_state = Button_State::Enabled;
								}
								else {
									current_button_state = Button_State::Invisible;
								}
							}

							// PROTOSS \\
							
							else if (unit->id == UnitId::ProtossArtisan &&  // Artisan Concordance Field passive
								current_button->icon_id == IconId::KhaydarinCore)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossEcclesiast &&  // Ecclesiast Astral Blessing passive 
								current_button->icon_id == IconId::ProtossGroundWeapons)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossLegionnaire && // Legionnaire Unified Strikes passive
								current_button->icon_id == IconId::ProtossGroundWeapons)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossLegionnaire &&  // Legionnaire Blade Vortex passive
								current_button->icon_id == IconId::PsiAssault)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossHierophant && // Hierophant Cowardice passive
								current_button->icon_id == IconId::PhotonCannonWeapon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossHierophant && // Hierophant Malice passive
								current_button->icon_id == IconId::DidactPhaseDisruptorCannon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossVagrant &&  // Vagrant Nihilism passive
								current_button->icon_id == IconId::MindControlUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossCabalist &&  // Cabalist Silent Strikes passive
								current_button->icon_id == IconId::ProtossGroundWeapons)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossCantavis &&  // Cantavis Ascendancy passive
								current_button->icon_id == IconId::MindControlUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossCantavis &&  // Cantavis Ephemeral Blades passive
								current_button->icon_id == IconId::ProtossGroundWeapons)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAmaranth &&  // Amaranth Eternal Service passive 
								current_button->icon_id == IconId::DidactPhaseDisruptorCannon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAtreus &&  // Atreus Stalwart Stance passive 
								current_button->icon_id == IconId::PlasmaShields)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossArchon &&  // Archon Astral Aegis passive
								current_button->icon_id == IconId::RechargeShields)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAugur &&  // Augur Final Hour passive
								current_button->icon_id == IconId::ArchonPsionicShockwave)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossPariah &&  // Pariah Heretical Bedlam passive 
								current_button->icon_id == IconId::DracadinPhaseDisruptor)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossVassal &&  // Vassal Terminal Surge passive 
								current_button->icon_id == IconId::RechargeShields)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossSimulacrum &&  // Simulacrum Autovitality passive 
								current_button->icon_id == IconId::ApialSensors)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossGolem &&  // Golem Deathless Procession passive 
								current_button->icon_id == IconId::LegEnhancements)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossBarghest &&  // Barghest Repentent Chorus passive 
								current_button->icon_id == IconId::PhotonCannonWeapon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossClarion &&  // Clarion Phase Link passive
							current_button->icon_id == IconId::RechargeShields)
							{
							current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossArchitect &&  // Architect Reactive Payload passive 
								current_button->icon_id == IconId::AntimatterMissiles)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossDemiurge &&  // Demiurge Worship passive 
								current_button->icon_id == IconId::DemiurgePassiveWorship)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAurora &&  // Aurora Last Orders passive
								current_button->icon_id == IconId::MojoScout)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossGladius &&  // Gladius Binding Plasma passive
								current_button->icon_id == IconId::DracadinPhaseDisruptor)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossMagister &&  // Magister Stellar Enforcement passive
								current_button->icon_id == IconId::DidactPhaseDisruptorCannon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossDidact &&  // Didact Shroud of Adun passive
								current_button->icon_id == IconId::DidactPassiveShroudOfAdun)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossEmpyrean &&  // Empyrean Heavenly Blaze passive
								current_button->icon_id == IconId::StewardPulseCannon)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossSolarion &&  // Solarion Robotic Fleet passive
								current_button->icon_id == IconId::Interceptor)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossStarSovereign &&  // Star Sovereign Intervention passive
								current_button->icon_id == IconId::AntimatterMissiles)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAnthelion &&  // Anthelion Gravity Well passive 
							current_button->icon_id == IconId::Recall)
							{
							current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossAnthelion &&  // Anthelion Gravity Well passive 
							current_button->icon_id == IconId::ArgusJewel)
							{
							current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossMatrix &&  // Matrix Multi-Attack IV passive
								current_button->icon_id == IconId::ApialSensors)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ProtossCrucible &&  // Crucible Khaydarin Charge passive
								current_button->icon_id == IconId::KhaydarinCore)
							{
								current_button_state = Button_State::Enabled;
							}

							// ZERG \\
							
							else if (unit->id == UnitId::ZergGosvileth &&  // Gosvileth Plethoral Gestation passive
								current_button->icon_id == IconId::VentralSacs)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergZethrokor &&  // Zethrokor Adrenal Glands passive
								current_button->icon_id == IconId::AdrenalGlands)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergVorvrokor && // Vorvrokor Adrenal Surge passive
								current_button->icon_id == IconId::MetabolicBoost)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergVorvrokor && // Vorvrokor Captivating ZethrokorClaws passive
								current_button->icon_id == IconId::ZethrokorClaws)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergQuazilisk && // Quazilisk Melting Points passive
								current_button->icon_id == IconId::SuicideInfestedTerran)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergEvigrilisk && // Evigrilisk Lashing Limbs passive
								current_button->icon_id == IconId::RilirokorToxicSpores)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergIzirokor && // Izirokor Crushing Tide passive
								current_button->icon_id == IconId::GeszithalorAcidSpore)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergKalkalisk && // Kalkalisk Lithe Organelle passive
								current_button->icon_id == IconId::GeszithalorAcidSpore)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergBactalisk && // Bactalisk Caustic Fission passive
								current_button->icon_id == IconId::GeszithalorAcidSpore) {
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergLiiralisk && // Liiralisk Lithe Organelle passive
								current_button->icon_id == IconId::GeszithalorAcidSpore)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergAlmaksalisk && // Almaksalisk Flanking DrolethSpines passive
								current_button->icon_id == IconId::ChitinousPlating)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergLakizilisk && // Lakizilisk Seismic DrolethSpines passive
								current_button->icon_id == IconId::LakiziliskSubterraneanSpines)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergMutalisk &&  // Mutalisk Lithe Organelle passive
								current_button->icon_id == IconId::GeszithalorAcidSpore)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergGorgrokor &&  // Gorgrokor Probing Incisors passive
								current_button->icon_id == IconId::UltrokorKaiserBlades)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergGorgrokor &&  // Gorgrokor Ravening Mandible passive
								current_button->icon_id == IconId::RoarUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergVithrilisk &&  // Vithrlisk Corrosive Touch passive
								current_button->icon_id == IconId::VenomUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergVithrilisk &&  // Vithrlisk Critical Mass passive
								current_button->icon_id == IconId::SuicideInfestedTerran)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergGeszithalor &&  // Geszithalor Searing Acid passive
								current_button->icon_id == IconId::ZergFlyerAttacks)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergMatraleth &&  // Matraleth Gamete Meiosis passive
								current_button->icon_id == IconId::GameteMeiosis)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergRilirokor &&  // Broodling Incubators passive
								current_button->icon_id == IconId::RoarUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergKonvilisk &&  // Konvilisk Fertile DrolethSpines passive
								current_button->icon_id == IconId::SunkenColonySubterraneanTentacle)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergUltrokor &&  // Ultrakor Saber Siphon passive
								current_button->icon_id == IconId::UltrokorKaiserBlades)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergUltrokor &&  // Ultrakor Kaiser Rampage passive
								current_button->icon_id == IconId::AnabolicSynthesis)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergKeskathalor &&  // Keskathalor Caustic Discharge passive
								current_button->icon_id == IconId::VenomUnused)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergAlkajelisk &&  // Alkajelisk Parasitic Spores passive
								current_button->icon_id == IconId::SpraithColonySeekerSpores)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergZoryusthaleth &&  // Zoryusthaleth Reconstitution passive
								current_button->icon_id == IconId::PneumatizedCaparace)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergZoryusthaleth &&  // Zoryusthaleth Swarming Omen passive
								current_button->icon_id == IconId::MetabolicBoost)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergAkistrokor &&		// Akistrokor Devouring Instinct passive
								current_button->icon_id == IconId::SuicideScourge)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergSovroleth &&		// Sovroleth Writhing Web passive
								current_button->icon_id == IconId::Ensnare)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergSovroleth &&		// Sovroleth Corpse Cocoon passive
								current_button->icon_id == IconId::SunkenColonySubterraneanTentacle)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergIroleth &&			// Iroleth Endomorphic Accelerant passive
								current_button->icon_id == IconId::ZergBasicBuildings)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergAlaszileth &&		// Alaszileth Dermal Accelerant passive
								current_button->icon_id == IconId::Consume)
							{
								current_button_state = Button_State::Enabled;
							}
							else if ((unit->id == UnitId::ZergCaphrolosk				// Lair, Hive, Sire Larval Gestation passive
								|| unit->id == UnitId::ZergGathtelosk
								|| unit->id == UnitId::ZergSibraelosk) &&
								current_button->icon_id == IconId::SuicideScourge)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergSibraelosk &&			// Sire Ectomorphic Accelerant passive
								current_button->icon_id == IconId::ZergAdvancedBuildings)
							{
								current_button_state = Button_State::Enabled;
							}
							else if (unit->id == UnitId::ZergLarvoskColony &&	// Larval Colony Larval Gestation passive
								current_button->icon_id == IconId::MetasynapticNode)
							{
								current_button_state = Button_State::Enabled;
							}

							// Passive icons: END \\

						}
					}

					// Attack overrides
					if (current_button->action_func == (ActFunc*)ButtonActions::Attack)
					{
						if (unit->id == UnitId::TerranBunker)
						{
							int loaded = 0;
							for (int i = 0; i < 8; ++i)
							{								
								Unit* bunkerUnit = unit->get_loaded_unit(i);
								if (bunkerUnit && scbw::unit_is_ranged_attacker(bunkerUnit))
								{
									loaded++;
								}
							}
							if (loaded)
							{
								current_button_state = Button_State::Enabled;
							}
							else
							{
								current_button_state = Button_State::Disabled;
							}
						}
					}

					// Form switch/mode change?
					if (current_button->action_func == (ActFunc*)ButtonActions::Use_Technology)
					{
						if (unit->id != UnitId::ProtossMindTyrant)
						{
							for (s32 i = 0; i < generictoggles; i++)
							{
								if (current_button->action_var == toggle_abilities[i])
								{
									if (scbw::get_generic_rust_value(unit, toggle_values[i]) == 1)
									{
										current_button->icon_id = button_toggle_off[i];
									}
									else
									{
										current_button->icon_id = button_toggle_on[i];
									}
								}
							}
						}

						// Cancel new abilities!
						if (current_button->action_var == TechId::CancelNewAbility)
						{
							bool canCancel = false;
							if (unit->id == UnitId::TerranWyvern)
							{
								UnitExt* activeExt = get_unit_ext(unit);
								if (activeExt->values[ValueWyvernLandGrabOn])
								{
									canCancel = true;
								}
							}

							if (!canCancel)
							{
								current_button_state = Button_State::Invisible;
							}
						}
					}

					// TODO: Reenable and test
					//if ((*g_active_portrait_unit)->player != *g_local_nation_id && current_button_state == Button_State::Enabled) {
					//	current_button_state = Button_State::Disabled;
					//}

					if (current_button_state == Button_State::Invisible)
					{
						buttons_count++;
						current_button++; // select next button (advance by sizeof(Button))
					}
				}
			}

			// 00459278
			if (!bStopThere && !jumpto5934E)
			{
				jumpto5933C = (!*g_is_in_replay) && (*g_active_portrait_unit != nullptr) && ((*g_active_portrait_unit)->player != *g_local_nation_id);
				// TODO: Reenable and test
				// jumpto5933C = false;
				if (!jumpto5933C)
				{
					// 0045929A
					if (buttons_count >= current_buttonset->buttons_in_set
						|| current_dialog->index < current_button->position)
					{
						jumpto5933C = true;
					}
					else
					{
						// 004592B5
						current_dialog->button_user = current_button;
						if (current_dialog->graphic != current_button->icon_id)
						{
							// 004592C2
							struct
							{
								u32 unknown_value_1;	//[EBP-1C]	[00]
								u32 unknown_value_2;	//[EBP-18]	[04]
								u32 not_allocated;		//[EBP-14]	[08]
								u16 unknown_value_3;	//[EBP-10]	[0C]
								s16 mouseX;				//[EBP-0E]	[0E]
								s16 mouseY;				//[EBP-0C]	[10]
								u16 not_allocated_2;	//[EBP-0A]	[12]
								u32 buttonState;		//[EBP-08]	[14]
								ButtonSet* buttonset;	//[EBP-04]	[18]
							} stack_placeholder;
							current_dialog->graphic = current_button->icon_id;
							if (!(current_dialog->flags & BinDlgFlags::Unknown0))
							{
								current_dialog->flags |= BinDlgFlags::Unknown0;
								scbw::update_dialog(current_dialog);
							}
							stack_placeholder.not_allocated = 0;
							stack_placeholder.not_allocated_2 = 0;
							stack_placeholder.unknown_value_1 = 0x00000006;
							stack_placeholder.unknown_value_2 = 0x00000000;
							stack_placeholder.unknown_value_3 = 0x000E;
							stack_placeholder.mouseY = g_mouse->y;
							stack_placeholder.mouseX = g_mouse->x;
							stack_placeholder.buttonState = current_button_state;
							stack_placeholder.buttonset = current_buttonset;
							helper::fxn_interact(current_dialog, (u32)&stack_placeholder);
							current_button_state = stack_placeholder.buttonState;
						}
						// 0045930B
						scbw::show_dialog(current_dialog);
						if (current_button_state == Button_State::Disabled)
						{
							scbw::disable_dialog(current_dialog);
							// 0045931C
							if (*unknown_0066FF60 == 0x15)
							{
								current_button->action_string_id = 0x02FA; // Text: Not Implemented.
								current_button++;	// select next button (advance by sizeof(Button)
								buttons_count++;
								jumpto5934E = true;
							}
							else
							{
								jumpto59336 = true;
							}
						}
						// 00459331
						if (!jumpto59336 && !jumpto5934E)
						{
							helper::enable_control(current_dialog);
						}
						if (!jumpto5934E)
						{
							// 00459336
							current_button++;	// select next button (advance by sizeof(Button)
							buttons_count++;
							jumpto5934E = true;
						}
					}
				}
			}
			if (!bStopThere && !jumpto5934E)
			{
				// 0045933C
				scbw::hide_dialog(current_dialog);
				current_dialog->graphic = 0xFFFF;
				current_dialog->button_user = nullptr;
			}
			// 0045934E
			current_dialog = current_dialog->next;
			if (current_dialog == nullptr)
			{
				bStopThere = true;
			}
			jumpto59336 = false;
			jumpto5933C = false;
			jumpto5934E = false;
		}
	}

	// 00459AD0
	// Identical to array at 0x0051AD30
	// Size is 40 = 4 * 10 = 0x28
	// 0x004598D0 is statbtn_Btn_Interact
	// 0x00459890 is statbtn_BIN_ReplayProgressbar
	const u32 Btn_fxnInteractFuncs[] =
	{
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x004598D0, 0x004598D0, 0x004598D0,
		0x00459890
	};

	// Using a local array instead of the original may allow to go past the 9 buttons limitations
	// May need a special case using the original array adress and count when in replay if modified
	void statbtn_bin_custom_ctrl_id(BinDlg* dialog)
	{
		helper::register_user_dialog_action(dialog, (u32)&Btn_fxnInteractFuncs, sizeof(Btn_fxnInteractFuncs));
		helper::bindlg_blit_surface(dialog);
	}
}