#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void displayEstablishingShot();					// 004D9200, hooked by X405
	void CMDACT_game_speed(BinDlg* control);		// 00481EF0
	void inject_glucustom_hooks();
}