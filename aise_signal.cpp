#include "aise_signal.h"
#include "DatExtend/datext.h"
#include "hooks/grafting/grafting.h"
#include "systems/display/frame_overlay.h"
#include "config.h"

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
		case AiseSignal::GetTransportCargoCount: {
			u32 count = 0;
			for (u32 i = 0; i < TransportCap; ++i) {
				if (unit1->get_loaded_unit(i)) {
					count += 1;
				}
			}
			return count;
		} break;
		case AiseSignal::GetAiMain: {
			return (u32)g_aiscript_controller;
		} break;
		case AiseSignal::GetAllUnitsCount: {
			return (u32)g_all_unit_count_table;
		} break;
		case AiseSignal::GetCompleteUnitsCount: {
			return (u32)g_completed_unit_count_table;
		} break;
		case AiseSignal::GetKilledUnitsTable: {
			return (u32)g_unit_killed_table;
		} break;
		case AiseSignal::GetDeathTable: {
			return (u32)g_unit_deaths_table;
		} break;
		case AiseSignal::GetUnitAvailabilityTable: {
			return (u32)g_unit_availability->available[0];
		} break;
		case AiseSignal::GetUnitTbl: {
			if (DatExt::g_unit_names_tbl) {
				return (u32)DatExt::g_unit_names_tbl;
			}
			return (u32)g_stat_txt_tbl;
		} break;
		case AiseSignal::GetUnitReq: {
			return (u32)find_req_id(requirements::units, v1);
		} break;
		case AiseSignal::GetUpgradesReq: {
			return (u32)find_req_id(requirements::upgrade, v1);
		} break;
		case AiseSignal::GetResearchReq: {
			return (u32)find_req_id(requirements::research, v1);
		} break;
		case AiseSignal::GetClientSelection: {
			return (u32)ccmu_extender.new_client_selection_group;
		} break;
		case AiseSignal::GetAltConsoleHotkey: {
			return (u32)g_config.tilde_control_group;
		} break;
		case AiseSignal::SetConsoleVisible: {
			g_aise_state.console_visible = v1;
		} break;
		case AiseSignal::AreCheatsAllowed: {
			return g_aise_state.cheats_allowed;
		} break;
		case AiseSignal::InReplayOrWitness: {
			return bw::in_replay_or_witness();
		} break;
		// Callbacks passed in v2/v3 because 00479930 marshals v1 via 16-bit AX.
		// Returns 1 to signal protocol support (older builds return 0).
		case AiseSignal::SetFrameOverlay: {
			g_frame_overlay.set_procs((FrameOverlay::DrawProc)v2, (FrameOverlay::SkipProc)v3);
			return 1;
		} break;
	}
	return 0;
}