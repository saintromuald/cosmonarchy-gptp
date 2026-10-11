#pragma once
#include "BW/api.h"

// This file is responsible for communication from Aise/Aize to GPTP

enum class AiseSignal {
	None                      = 0,

	GetTransportCargoCount    = 1,
	GetAiMain                 = 2,
	GetAllUnitsCount          = 3,
	GetCompleteUnitsCount     = 4,
	GetKilledUnitsTable       = 5,
	GetDeathTable             = 6,
	GetUnitAvailabilityTable  = 7,
	GetUnitTbl                = 8,
	GetUnitReq                = 9,
	GetUpgradesReq            = 10,
	GetResearchReq            = 11,
	GetClientSelection        = 12,
	GetAltConsoleHotkey       = 13,
	SetConsoleVisible         = 14,
	AreCheatsAllowed          = 15,
	InReplayOrWitness         = 16,
};

struct AiseState {
	bool console_visible;
	bool cheats_allowed;
};

inline AiseState g_aise_state;

u32 parse_aise_signal(Unit* unit1, Unit* unit2, AiseSignal type, u32 v1, u32 v2, u32 v3);