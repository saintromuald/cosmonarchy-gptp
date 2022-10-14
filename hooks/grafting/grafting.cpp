#include "grafting.h"
#include <SCBW/Storm.h>
#include <hook_tools.h>
#include "button_functions.h"

// Pointers to current data so it can be freed if overwritten
REQ_SCRIPT* units_req = NULL;
REQ_SCRIPT* upgrades_req = NULL;
REQ_SCRIPT* research_req = NULL;
REQ_SCRIPT* tech_req = NULL;
REQ_SCRIPT* orders_req = NULL;

void* buttons_data = NULL;

namespace
{
	u32 buttonReqFuncs[] = {
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
	u32 buttonReqCount = sizeof(buttonReqFuncs) / sizeof(u32);

	u32 buttonActFuncs[] = {
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
	u32 buttonActCount = sizeof(buttonActFuncs) / sizeof(u32);

	u32 statFunctions[] = {
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
	u32 statFuncCount = sizeof(statFunctions) / sizeof(u32);

	u32 dispFunctions[] = {
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
	u32 dispFuncCount = sizeof(dispFunctions) / sizeof(u32);
};

namespace hooks {

	void load_req_grafts()
	{
		void* data = NULL;

		data = FastFileRead("datreq\\units.graft", NULL, true, 0, NULL);
		if (data != NULL)
		{
			inject_grafting_data(data, GraftType::units_req);
			if (units_req != NULL)
			{
				SMemFree(units_req, "grafting.cpp", __LINE__, 0);
			}
			units_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\upgrades.graft", NULL, true, 0, NULL);
		if (data != NULL)
		{
			inject_grafting_data(data, GraftType::upgrades_req);
			if (upgrades_req != NULL)
			{
				SMemFree(upgrades_req, "grafting.cpp", __LINE__, 0);
			}
			upgrades_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\research.graft", NULL, true, 0, NULL);
		if (data != NULL)
		{
			inject_grafting_data(data, GraftType::research_req);
			if (research_req != NULL)
			{
				SMemFree(research_req, "grafting.cpp", __LINE__, 0);
			}
			research_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\tech.graft", NULL, true, 0, NULL);
		if (data != NULL)
		{
			inject_grafting_data(data, GraftType::tech_req);
			if (tech_req != NULL)
			{
				SMemFree(tech_req, "grafting.cpp", __LINE__, 0);
			}
			tech_req = (REQ_SCRIPT*)data;
		}

		data = FastFileRead("datreq\\orders.graft", NULL, true, 0, NULL);
		if (data != NULL)
		{
			inject_grafting_data(data, GraftType::orders_req);
			if (orders_req != NULL)
			{
				SMemFree(orders_req, "grafting.cpp", __LINE__, 0);
			}
			orders_req = (REQ_SCRIPT*)data;
		}
	}

	void load_button_grafts()
	{
		void* data = FastFileRead("datreq\\buttons.graft", NULL, true, 0, NULL);
		// File not found
		if (data == NULL)
		{
			return;
		}

		// unpack data
		u32 buttonCount = ((u32*)data)[0];
		u32 unitCount = ((u32*)data)[1];
		u32 unitStatCount = ((u32*)data)[2];
		Button* buttons = (Button*)(&(((u32*)data)[3]));
		ButtonSet* buttonSets = (ButtonSet*)(&buttons[buttonCount]);
		UnitStatFuncArrayEntry* statInfo = (UnitStatFuncArrayEntry*)(&buttonSets[unitCount]);

		// allocate new, possibly larger, data
		u32 unitCountNew = std::max(unitCount, (u32)UNIT_TYPE_COUNT);
		u32 unitStatCountNew = std::max(unitStatCount, (u32)UNIT_TYPE_COUNT);
		u32 dataSize = buttonCount*sizeof(Button) + unitCountNew*sizeof(ButtonSet) + unitStatCountNew*sizeof(UnitStatFuncArrayEntry);
		void* newData = malloc(dataSize);
		Button* buttonsNew = (Button*)newData;
		ButtonSet* buttonSetsNew = (ButtonSet*)(&buttonsNew[buttonCount]);
		UnitStatFuncArrayEntry* statInfoNew = (UnitStatFuncArrayEntry*)(&buttonSetsNew[unitCountNew]);

		// Copy data to new, possibly larger, buffer
		memcpy(buttonsNew, buttons, buttonCount * sizeof(Button));
		memcpy(buttonSetsNew, buttonSets, unitCount * sizeof(ButtonSet));
		memcpy(statInfoNew, statInfo, unitStatCount * sizeof(UnitStatFuncArrayEntry));

		// Remove file buffer
		SMemFree(data, "grafting.cpp", __LINE__, 0);

		// Convert function ID's to pointers
		for (u32 i = 0; i < buttonCount; i++) {
			if ((u32)(buttonsNew[i].requirement_func) < buttonReqCount) {
				buttonsNew[i].requirement_func = (ReqFunc*)(buttonReqFuncs[(u32)(buttonsNew[i].requirement_func)]);
			} else {
			// what to do here?
			}
			if ((u32)(buttonsNew[i].action_func) < buttonActCount) {
				buttonsNew[i].action_func = (ActFunc*)(buttonActFuncs[(u32)(buttonsNew[i].action_func)]);
			} else {
			// what to do here?
			}
		}

		// Convert button ID's to pointers
		for (u32 i = 0; i < unitCount; i++) {
			if (buttonSetsNew[i].buttons_in_set != 0) {
				buttonSetsNew[i].first_button = &buttonsNew[(u32)(buttonSetsNew[i].first_button)];
			}
		}

		// Convert function ID's to pointers
		for (u32 i = 0; i < unitStatCount; i++) {
			if (statInfoNew[i].unit_stat_cond_func < statFuncCount) {
				statInfoNew[i].unit_stat_cond_func = statFunctions[statInfoNew[i].unit_stat_cond_func];
			} else {
			// what to do here?
			}
			if (statInfoNew[i].unit_stat_act_func < dispFuncCount) {
				statInfoNew[i].unit_stat_act_func = dispFunctions[statInfoNew[i].unit_stat_act_func];
			} else {
			// what to do here?
			}
		}

		// Fill in any undefined IDs
		for (u32 i = unitCount; i < unitCountNew; i++) {
			buttonSetsNew[i].buttons_in_set = 0;
			buttonSetsNew[i].first_button = NULL;
			buttonSetsNew[i].connected_unit = 65535;
		}

		for (u32 i = unitStatCount; i < unitStatCountNew; i++) {
			if (i < 250) { // Unselectable, unusable
				statInfoNew[i].unit_id = 0;
				statInfoNew[i].unit_stat_cond_func = statInfoNew[4].unit_stat_cond_func;
				statInfoNew[i].unit_stat_act_func = statInfoNew[4].unit_stat_act_func;
			} else { // Normal
				statInfoNew[i].unit_id = i;
				statInfoNew[i].unit_stat_cond_func = statInfoNew[0].unit_stat_cond_func;
				statInfoNew[i].unit_stat_act_func = statInfoNew[0].unit_stat_act_func;
			}
		}

		// Inject data
		inject_grafting_data(buttonSetsNew, GraftType::buttons);
		inject_grafting_data(statInfoNew, GraftType::statInfo);

		if (buttons_data != NULL)
		{
			free(buttons_data);
		}
		buttons_data = newData;
	}
}