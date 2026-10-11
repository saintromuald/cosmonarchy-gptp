#include "grafting.h"
#include <storm/sfile.h>
#include <hook_tools.h>
#include "button_functions.h"

#include <algorithm>
#include <vector>

// Pointers to current data so it can be freed if overwritten
REQ_SCRIPT* units_req = 0;
REQ_SCRIPT* upgrades_req = 0;
REQ_SCRIPT* research_req = 0;
REQ_SCRIPT* tech_req = 0;
REQ_SCRIPT* orders_req = 0;

namespace {
	struct ButtonGraftData {
		std::vector<Button> buttons;
		std::vector<ButtonSet> button_sets;
		std::vector<UnitStatFuncArrayEntry> stat_info;
	};
	ButtonGraftData g_button_graft_data;

	constexpr u32 NoConnectedUnit = 0xFFFF;

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

	[[nodiscard]] bool validate_id(u32 id, u32 table_size,
	                               const char* what, u32 index) {
		if (id >= table_size) {
			bw::printf("load_button_grafts: %s id %u at index %u out of range (max %u); aborting",
			           what, id, index, table_size - 1);
			return false;
		}
		return true;
	}

	bool address_in_table(u32 addr, const u32* table, u32 table_size) {
		for (u32 j = 0; j < table_size; ++j) {
			if (table[j] == addr) return true;
		}
		return false;
	}

	bool verify_button_graft_data() {
		for (size_t i = 0; i < g_button_graft_data.buttons.size(); ++i) {
			const auto& b = g_button_graft_data.buttons[i];
			const auto req_addr = static_cast<u32>(reinterpret_cast<uintptr_t>(b.requirement_func));
			const auto act_addr = static_cast<u32>(reinterpret_cast<uintptr_t>(b.action_func));
			if (!address_in_table(req_addr, button_req_funcs, button_req_count)) {
				bw::printf("verify_button_graft: button %zu requirement_func 0x%08x unknown", i, req_addr);
				return false;
			}
			if (!address_in_table(act_addr, button_act_funcs, button_act_count)) {
				bw::printf("verify_button_graft: button %zu action_func 0x%08x unknown", i, act_addr);
				return false;
			}
		}

		const auto* buttons_begin = g_button_graft_data.buttons.data();
		const auto* buttons_end   = buttons_begin + g_button_graft_data.buttons.size();
		for (size_t i = 0; i < g_button_graft_data.button_sets.size(); ++i) {
			const auto& s = g_button_graft_data.button_sets[i];
			if (s.buttons_in_set == 0) continue;
			if (s.first_button < buttons_begin || s.first_button >= buttons_end) {
				bw::printf("verify_button_graft: button_set %zu first_button out of bounds", i);
				return false;
			}
		}

		for (size_t i = 0; i < g_button_graft_data.stat_info.size(); ++i) {
			const auto& e = g_button_graft_data.stat_info[i];
			if (e.unit_stat_cond_func && !address_in_table(e.unit_stat_cond_func, stat_functions, statFuncCount)) {
				bw::printf("verify_button_graft: stat %zu cond_func 0x%08x unknown", i, e.unit_stat_cond_func);
				return false;
			}
			if (e.unit_stat_act_func && !address_in_table(e.unit_stat_act_func, disp_functions, disp_func_count)) {
				bw::printf("verify_button_graft: stat %zu act_func 0x%08x unknown", i, e.unit_stat_act_func);
				return false;
			}
		}

		bw::printf_console("verify_button_graft: OK (%zu buttons, %zu sets, %zu stat entries)",
			g_button_graft_data.buttons.size(),
			g_button_graft_data.button_sets.size(),
			g_button_graft_data.stat_info.size());
		return true;
	}

	// Have to use raw pointers for target for now since they're used in inline assembly and BW_DATA_DEF -Jesse
	void reload_req_graft(const char* filename, GraftType::Enum type, REQ_SCRIPT*& target) {
		auto data = sfile::try_load_file(filename).unsafe_cast<REQ_SCRIPT>();
		if (!data) return;

		hooks::inject_grafting_data(data.get(), type);
		if (target) {
			SMemFree(target);
		}
		target = data.release();
	}
}

namespace hooks {
	void load_req_grafts() {
		reload_req_graft("datreq\\units.graft", GraftType::units_req, units_req);
		reload_req_graft("datreq\\upgrades.graft", GraftType::upgrades_req, upgrades_req);
		reload_req_graft("datreq\\research.graft", GraftType::research_req, research_req);
		reload_req_graft("datreq\\tech.graft", GraftType::tech_req, tech_req);
		reload_req_graft("datreq\\orders.graft", GraftType::orders_req, orders_req);
	}

	void load_button_grafts() {
		auto file = sfile::try_load_file("datreq\\buttons.graft");
		if (!file) return;

		// Header: u32 button_count, unit_count, unit_stat_count
		constexpr size_t header_size = 3 * sizeof(u32);
		if (file.size < header_size) {
			bw::printf("load_button_grafts: file smaller than header (%zu bytes)", file.size);
			return;
		}

		const auto* bytes  = reinterpret_cast<const u8*>(file.data.get());
		const auto* header = reinterpret_cast<const u32*>(bytes);
		const u32 button_count    = header[0];
		const u32 unit_count      = header[1];
		const u32 unit_stat_count = header[2];

		const size_t expected_size = header_size
			+ button_count * sizeof(Button)
			+ unit_count * sizeof(ButtonSet)
			+ unit_stat_count * sizeof(UnitStatFuncArrayEntry);
		if (file.size < expected_size) {
			bw::printf("load_button_grafts: file too small for declared counts "
			           "(need %zu, got %zu)", expected_size, file.size);
			return;
		}

		const auto* src_buttons = reinterpret_cast<const Button*>(bytes + header_size);
		const auto* src_button_sets = reinterpret_cast<const ButtonSet*>(src_buttons + button_count);
		const auto* src_stat_info = reinterpret_cast<const UnitStatFuncArrayEntry*>(src_button_sets + unit_count);

		const u32 unit_count_new = std::max(unit_count, static_cast<u32>(UNIT_TYPE_COUNT));
		const u32 unit_stat_count_new = std::max(unit_stat_count, static_cast<u32>(UNIT_TYPE_COUNT));

		ButtonGraftData staging;
		staging.buttons.assign(src_buttons, src_buttons + button_count);
		staging.button_sets.assign(src_button_sets, src_button_sets + unit_count);
		staging.button_sets.resize(unit_count_new, ButtonSet{0, nullptr, NoConnectedUnit});
		staging.stat_info.assign(src_stat_info, src_stat_info + unit_stat_count);
		staging.stat_info.resize(unit_stat_count_new);

		// Resolve button function IDs to pointers
		for (u32 i = 0; i < button_count; ++i) {
			auto& button = staging.buttons[i];
			const auto req_id = reinterpret_cast<uintptr_t>(button.requirement_func);
			const auto act_id = reinterpret_cast<uintptr_t>(button.action_func);
			if (!validate_id(static_cast<u32>(req_id), button_req_count, "requirement", i)) return;
			if (!validate_id(static_cast<u32>(act_id), button_act_count, "action", i)) return;
			button.requirement_func = reinterpret_cast<ReqFunc*>(button_req_funcs[req_id]);
			button.action_func = reinterpret_cast<ActFunc*>(button_act_funcs[act_id]);
		}

		// Connect button-set pointers to staging.buttons
		for (u32 i = 0; i < unit_count; ++i) {
			auto& set = staging.button_sets[i];
			if (set.buttons_in_set == 0) {
				continue;
			}
			const auto first = reinterpret_cast<uintptr_t>(set.first_button);
			if (!validate_id(static_cast<u32>(first), button_count, "button set first_button", i)) return;
			set.first_button = &staging.buttons[first];
		}

		// Resolve stat-info function IDs
		for (u32 i = 0; i < unit_stat_count; ++i) {
			auto& entry = staging.stat_info[i];
			if (!validate_id(entry.unit_stat_cond_func, statFuncCount, "stat cond", i)) return;
			if (!validate_id(entry.unit_stat_act_func, disp_func_count, "stat act", i)) return;
			entry.unit_stat_cond_func = stat_functions[entry.unit_stat_cond_func];
			entry.unit_stat_act_func = disp_functions[entry.unit_stat_act_func];
		}

		// Fill expanded stat_info entries using [0] and [4] as templates
		if (unit_stat_count > 0) {
			const auto& normal_template = staging.stat_info[0];
			const auto& unselectable_template = (unit_stat_count > 4)
				? staging.stat_info[4] : normal_template;
			for (u32 i = unit_stat_count; i < unit_stat_count_new; ++i) {
				auto& entry = staging.stat_info[i];
				if (i < 250) { // Unsure where this number comes from -Jesse
					entry.unit_id = 0;
					entry.unit_stat_cond_func = unselectable_template.unit_stat_cond_func;
					entry.unit_stat_act_func = unselectable_template.unit_stat_act_func;
				} else {
					entry.unit_id = i;
					entry.unit_stat_cond_func = normal_template.unit_stat_cond_func;
					entry.unit_stat_act_func = normal_template.unit_stat_act_func;
				}
			}
		}

		inject_grafting_data(staging.button_sets.data(), GraftType::buttons);
		inject_grafting_data(staging.stat_info.data(),   GraftType::statInfo);
		g_button_graft_data = std::move(staging);

		verify_button_graft_data();
	}
}