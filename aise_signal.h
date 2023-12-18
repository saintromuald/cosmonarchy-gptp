#pragma once
#include "SCBW/api.h"

enum AiseSignal {
	AiseSignal_None = 0,
	AiseSignal_GetTransportCargoCount = 1,
	AiseSignal_GetAiMain = 2,
	AiseSignal_GetAllUnitsCount = 3,
	AiseSignal_GetCompleteUnitsCount = 4,
	AiseSignal_GetKilledUnitsTable = 5,
	AiseSignal_GetDeathTable = 6,
	AiseSignal_GetUnitAvailabilityTable = 7,
	AiseSignal_GetUnitTbl = 8,
	AiseSignal_GetUnitReq = 9,
	AiseSignal_GetUpgradesReq = 10,
	AiseSignal_GetResearchReq = 11,
	AiseSignal_GetClientSelection = 12,
	AiseSignal_GetAltConsoleHotkey = 13,
};

u32 parse_aise_signal(Unit* unit1, Unit* unit2, AiseSignal type, u32 v1, u32 v2, u32 v3);