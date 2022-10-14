#include "aise_signal.h"
#include "DatExtend/datext.h"
#include "hooks/grafting/grafting.h"

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
			return (u32)&requirements::units[units_dat::requirement_offsets[v1]];
		} break;
		case AiseSignal_GetUpgradesReq: {
			return (u32)&requirements::upgrade[upgrades_dat::requirement_offsets[v1]];
		} break;
		case AiseSignal_GetResearchReq: {
			return (u32)&requirements::research[techdata_dat::requirement_offsets[v1]];
		} break;
	}
	return 0;
}