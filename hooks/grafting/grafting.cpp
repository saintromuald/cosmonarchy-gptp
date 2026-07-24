#include "grafting.h"
#include <BW/Storm.h>
#include <hook_tools.h>
#include "button_functions.h"

// Pointers to current data so it can be freed if overwritten
REQ_SCRIPT* units_req = 0;
REQ_SCRIPT* upgrades_req = 0;
REQ_SCRIPT* research_req = 0;
REQ_SCRIPT* tech_req = 0;
REQ_SCRIPT* orders_req = 0;

void* buttons_data = 0;

namespace {
	u32 button_req_funcs[] = {
		ButtonRequirements::Slow_Down_Replay,
		ButtonRequirements::Speed_Up_Replay,
		ButtonRequirements::PlayorPause_Replay,
		ButtonRequirements::Always,
		ButtonRequirements::Can_Create_UnitorBuilding,
		ButtonRequirements::Can_Build_Subunit,
		ButtonRequirements::Mixed_Group_MoveorPatrolorHold_Position,
		ButtonRequirements::Mixed_Group_Stop,
		ButtonRequirements::Unit_Has_A_Weapon,
		ButtonRequirements::Attacking_Building,
		ButtonRequirements::Cancel_Last,
		ButtonRequirements::Cancel_AddOn,
		ButtonRequirements::Larva_Exists_Upgrading_Only,
		ButtonRequirements::Rally_Point,
		ButtonRequirements::Rally_Point_While_Upgrading_Only,
		ButtonRequirements::ConstructionorMutation_Underway,
		ButtonRequirements::Upgrade_Underway,
		ButtonRequirements::Research_Underway,
		ButtonRequirements::Unit_Is_Carrying_Nothing__Gather_Probe,
		ButtonRequirements::Unit_Is_Carrying_Something_Return_Cargo_Probe,
		ButtonRequirements::Unit_Capacity_Has_Not_Been_Met,
		ButtonRequirements::Carrying_Some_Units,
		ButtonRequirements::Tech_Spell_Not_Researched,
		ButtonRequirements::Spell_Researched,
		ButtonRequirements::Has_Spidermine_and_Is_Researched,
		ButtonRequirements::Upgrade_Not_At_Max_Level,
		ButtonRequirements::Can_Cloak,
		ButtonRequirements::Can_Decloak,
		ButtonRequirements::Can_Cloak_Mixed,
		ButtonRequirements::Can_Decloak_Mixed,
		ButtonRequirements::Tank_Is_In_Tank_Mode,
		ButtonRequirements::Tank_Is_In_Siege_Mode,
		ButtonRequirements::Tank_Is_In_Tank_Mode_Move,
		ButtonRequirements::Nuke_Available,
		ButtonRequirements::Recharge_Shields,
		ButtonRequirements::Building_Can_Move_And_Has_Lifted_Off,
		ButtonRequirements::Building_Has_Lifted_Off,
		ButtonRequirements::Building_Has_Landed,
		ButtonRequirements::Ability_Is_Researched,
		ButtonRequirements::Is_Using_Ability,
		ButtonRequirements::Morph_To_Lurker,
		ButtonRequirements::Not_Burrowed_MoveorStoporPatrolorHold_Position,
		ButtonRequirements::Not_Burrowed__Attack,
		ButtonRequirements::Has_No_Cargo__Gather_Drone,
		ButtonRequirements::Has_Cargo__Return_Cargo_Drone,
		ButtonRequirements::Burrowed__Stop_Lurker,
		ButtonRequirements::Has_No_Exit,
		ButtonRequirements::Zerg_Basic_Buildings,
		ButtonRequirements::Zerg_Advanced_Buildings,
		ButtonRequirements::Has_Interceptors,
		ButtonRequirements::Has_Scarabs,
		ButtonRequirements::Two_Units_Selected_and_Not_Researched_Archon_Merge,
		ButtonRequirements::Two_Units_Selected__Archon_Warp,
		ButtonRequirements::Two_Units_Selected_and_Not_Researched_Dark_Archon_Meld,
		ButtonRequirements::Two_Units_Selected__Dark_Archon_Meld,
		ButtonRequirements::Protoss_Basic_Buildings,
		ButtonRequirements::Protoss_Advanced_Buildings,
		ButtonRequirements::Is_Constructing__Halt_SCV,
		ButtonRequirements::Move__SCV,
		ButtonRequirements::Stop__SCV,
		ButtonRequirements::Attack__SCV,
		ButtonRequirements::Repair__SCV,
		ButtonRequirements::Gather__SCV,
		ButtonRequirements::Return_Cargo__SCV,
		ButtonRequirements::Terran_Basic_Buildings,
		ButtonRequirements::Terran_Advanced_Buildings,
		ButtonRequirements::Nuke_Train
	};
	u32 button_req_count = sizeof(button_req_funcs) / sizeof(u32);

	u32 button_act_funcs[] = {
		ButtonActions::Cancel_Infestation,
		ButtonActions::Rally_Point,
		ButtonActions::Select_Larva,
		ButtonActions::Create_Unit,
		ButtonActions::Cancel_Last,
		ButtonActions::Tank_Mode,
		ButtonActions::Siege_Mode,
		ButtonActions::Cancel_Construction,
		ButtonActions::Cancel_Morph,
		ButtonActions::Move,
		ButtonActions::Stop,
		ButtonActions::Attack,
		ButtonActions::Attack_Suicide,
		ButtonActions::Attack_Building,
		ButtonActions::Move__CarrierorReaver,
		ButtonActions::Stop_Carrier,
		ButtonActions::Stop_Reaver,
		ButtonActions::Attack_Carrier,
		ButtonActions::Attack_Reaver,
		ButtonActions::Build_Subunit,
		ButtonActions::Patrol,
		ButtonActions::Hold_Position,
		ButtonActions::Research_Technology,
		ButtonActions::Cancel_Technology_Research,
		ButtonActions::Use_Technology,
		ButtonActions::StimPack,
		ButtonActions::Research_Upgrade,
		ButtonActions::Cancel_Upgrade_Research,
		ButtonActions::Cancel_AddOn,
		ButtonActions::Create_Building_Terran,
		ButtonActions::Place_COP,
		ButtonActions::Create_Building_Protoss,
		ButtonActions::Create_AddOn,
		ButtonActions::Create_Building_Zerg,
		ButtonActions::Place_Nydus_Canal_Exit,
		ButtonActions::Building_Morph,
		ButtonActions::Land,
		ButtonActions::Repair,
		ButtonActions::Unit_Morph,
		ButtonActions::Gather,
		ButtonActions::Return_Cargo,
		ButtonActions::Burrow,
		ButtonActions::Unburrow,
		ButtonActions::Cloak,
		ButtonActions::Decloak,
		ButtonActions::Lift_Off,
		ButtonActions::Load,
		ButtonActions::Unload,
		ButtonActions::Archon_Warp,
		ButtonActions::Dark_Archon_Meld,
		ButtonActions::Recharge_Shields,
		ButtonActions::Nuclear_Strike,
		ButtonActions::Cancel_Nuclear_Strike,
		ButtonActions::Heal,
		ButtonActions::Slow_Down_Replay,
		ButtonActions::PlayorPause_Replay,
		ButtonActions::Speed_Up_Replay,
		ButtonActions::Cancel,
		ButtonActions::Cancel_Place_Building,
		ButtonActions::Change_Displayed_Buttons,
	};
	u32 button_act_count = sizeof(button_act_funcs) / sizeof(u32);

	u32 stat_functions[] = {
		0x00424500, // Status Function 1
		0x00424980, // Status Function 2
		0x00425180, // Status Function 3
		0x00424F10, // Status Function 4
		0x00424B50, // Status Function 5
		0x00424AF0, // Status Function 6
		0x00424AC0, // Status Function 7
		0x00425900, // Status Function 8
		0x00424520  // Status Function 9
	};
	u32 statFuncCount = sizeof(stat_functions) / sizeof(u32);

	u32 disp_functions[] = {
		0x00426F50, // Display Function 1
		0x00427890, // Display Function 2
		0x00427C90, // Display Function 3
		0x004274A0, // Display Function 4
		0x004273E0, // Display Function 5
		0x00427260, // Display Function 6
		0x00427D30, // Display Function 7
		0x00426EE0, // Display Function 8
		0x00425EE0  // Display Function 9
	};
	u32 disp_func_count = sizeof(disp_functions) / sizeof(u32);
};

namespace hooks {
	void load_req_grafts() {
		void* data = 0;

		data = FastFileRead("datreq\\units.graft", 0, true, 0, 0);
		if (data) {
			inject_grafting_data(data, GraftType::units_req);
			if (units_req) {
				helper::SMemFree(units_req);
			}
			units_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\upgrades.graft", 0, true, 0, 0);
		if (data) {
			inject_grafting_data(data, GraftType::upgrades_req);
			if (upgrades_req) {
				helper::SMemFree(upgrades_req);
			}
			upgrades_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\research.graft", 0, true, 0, 0);
		if (data) {
			inject_grafting_data(data, GraftType::research_req);
			if (research_req) {
				helper::SMemFree(research_req);
			}
			research_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\tech.graft", 0, true, 0, 0);
		if (data) {
			inject_grafting_data(data, GraftType::tech_req);
			if (tech_req) {
				helper::SMemFree(tech_req);
			}
			tech_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\orders.graft", 0, true, 0, 0);
		if (data) {
			inject_grafting_data(data, GraftType::orders_req);
			if (orders_req) {
				helper::SMemFree(orders_req);
			}
			orders_req = (REQ_SCRIPT*)data;
		}
	}

	void load_button_grafts() {
		void* data = FastFileRead("datreq\\buttons.graft", 0, true, 0, 0);
		// File not found
		if (!data) {
			return;
		}

		// unpack data
		u32 button_count = ((u32*)data)[0];
		u32 unit_count = ((u32*)data)[1];
		u32 unit_stat_count = ((u32*)data)[2];
		Button* buttons = (Button*)(&(((u32*)data)[3]));
		ButtonSet* button_sets = (ButtonSet*)(&buttons[button_count]);
		UnitStatFuncArrayEntry* stat_info = (UnitStatFuncArrayEntry*)(&button_sets[unit_count]);

		// allocate new, possibly larger, data
		u32 unitCountNew = std::max(unit_count, (u32)UNIT_TYPE_COUNT);
		u32 unitStatCountNew = std::max(unit_stat_count, (u32)UNIT_TYPE_COUNT);
		u32 dataSize = button_count*sizeof(Button) + unitCountNew*sizeof(ButtonSet) + unitStatCountNew*sizeof(UnitStatFuncArrayEntry);
		void* new_data = malloc(dataSize);
		Button* buttons_new = (Button*)new_data;
		ButtonSet* button_sets_new = (ButtonSet*)(&buttons_new[button_count]);
		UnitStatFuncArrayEntry* stat_info_new = (UnitStatFuncArrayEntry*)(&button_sets_new[unitCountNew]);

		// Copy data to new, possibly larger, buffer
		memcpy(buttons_new, buttons, button_count * sizeof(Button));
		memcpy(button_sets_new, button_sets, unit_count * sizeof(ButtonSet));
		memcpy(stat_info_new, stat_info, unit_stat_count * sizeof(UnitStatFuncArrayEntry));

		// Remove file buffer
		helper::SMemFree(data);

		// Convert function ID's to pointers
		for (u32 i = 0; i < button_count; i++) {
			if ((u32)(buttons_new[i].requirement_func) < button_req_count) {
				buttons_new[i].requirement_func = (ReqFunc*)(button_req_funcs[(u32)(buttons_new[i].requirement_func)]);
			} else {
				// what to do here?
			}
			if ((u32)(buttons_new[i].action_func) < button_act_count) {
				buttons_new[i].action_func = (ActFunc*)(button_act_funcs[(u32)(buttons_new[i].action_func)]);
			} else {
				// what to do here?
			}
		}

		// Convert button ID's to pointers
		for (u32 i = 0; i < unit_count; i++) {
			if (button_sets_new[i].buttons_in_set != 0) {
				button_sets_new[i].first_button = &buttons_new[(u32)(button_sets_new[i].first_button)];
			}
		}

		// Convert function ID's to pointers
		for (u32 i = 0; i < unit_stat_count; i++) {
			if (stat_info_new[i].unit_stat_cond_func < statFuncCount) {
				stat_info_new[i].unit_stat_cond_func = stat_functions[stat_info_new[i].unit_stat_cond_func];
			} else {
				// what to do here?
			}
			if (stat_info_new[i].unit_stat_act_func < disp_func_count) {
				stat_info_new[i].unit_stat_act_func = disp_functions[stat_info_new[i].unit_stat_act_func];
			} else {
				// what to do here?
			}
		}

		// Fill in any undefined IDs
		for (u32 i = unit_count; i < unitCountNew; i++) {
			button_sets_new[i].buttons_in_set = 0;
			button_sets_new[i].first_button = 0;
			button_sets_new[i].connected_unit = 65535;
		}

		for (u32 i = unit_stat_count; i < unitStatCountNew; i++) {
			if (i < 250) { // Unselectable, unusable
				stat_info_new[i].unit_id = 0;
				stat_info_new[i].unit_stat_cond_func = stat_info_new[4].unit_stat_cond_func;
				stat_info_new[i].unit_stat_act_func = stat_info_new[4].unit_stat_act_func;
			} else { // Normal
				stat_info_new[i].unit_id = i;
				stat_info_new[i].unit_stat_cond_func = stat_info_new[0].unit_stat_cond_func;
				stat_info_new[i].unit_stat_act_func = stat_info_new[0].unit_stat_act_func;
			}
		}

		// Inject data
		inject_grafting_data(button_sets_new, GraftType::buttons);
		inject_grafting_data(stat_info_new, GraftType::statInfo);

		if (buttons_data) {
			free(buttons_data);
		}
		buttons_data = new_data;
	}
}