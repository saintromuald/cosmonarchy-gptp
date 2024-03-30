#include "aise_signal.h"
#include "DatExtend/datext.h"
#include "hooks/grafting/grafting.h"
#include "config.h"

AiseState aise_state = {};

static u16* find_req_id(u16* req_table, u32 req_id) {
	u16* current = req_table + 1;
	while (true) {
		u32 current_id = *current;
		if (current_id == req_id) {
			return current + 1;
		}
		if (current_id == 0xffff) {
			return 0;
		}
		while (*current != 0xffff) {
			current++;
		}
		current++;
	}
}

u32 parse_aise_signal(Unit* unit1, Unit* unit2, AiseSignal type, u32 v1, u32 v2, u32 v3) {
	switch (type) {
		case AiseSignal_GetTransportCargoCount: {
			u32 count = 0;
			for (u32 i = 0; i < TransportCap; ++i) {
				if (unit1->get_loaded_unit(i)) {
					count += 1;
				}
			}
			return count;
		} break;
		case AiseSignal_GetAiMain: {
			return (u32)g_aiscript_controller;
		} break;
		case AiseSignal_GetAllUnitsCount: {
			return (u32)g_all_unit_count_table;
		} break;
		case AiseSignal_GetCompleteUnitsCount: {
			return (u32)g_completed_unit_count_table;
		} break;
		case AiseSignal_GetKilledUnitsTable: {
			return (u32)g_unit_killed_table;
		} break;
		case AiseSignal_GetDeathTable: {
			return (u32)g_unit_deaths_table;
		} break;
		case AiseSignal_GetUnitAvailabilityTable: {
			return (u32)g_unit_availability->available[0];
		} break;
		case AiseSignal_GetUnitTbl: {
			if (DatExt::unitNamesTbl) {
				return (u32)DatExt::unitNamesTbl;
			}
			return (u32)g_stat_txt_tbl;
		} break;
		case AiseSignal_GetUnitReq: {
			return (u32)find_req_id(requirements::units, v1);
		} break;
		case AiseSignal_GetUpgradesReq: {
			return (u32)find_req_id(requirements::upgrade, v1);
		} break;
		case AiseSignal_GetResearchReq: {
			return (u32)find_req_id(requirements::research, v1);
		} break;
		case AiseSignal_GetClientSelection: {
			return (u32)ccmu_extender.new_client_selection_group;
		} break;
		case AiseSignal_GetAltConsoleHotkey: {
			return (u32)g_config.tilde_control_group;
		} break;
		case AiseSignal_SetConsoleVisible: {
			aise_state.console_visible = v1;
		} break;
	}
	return 0;
}