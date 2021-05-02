#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	/*
	void raceDropdown(BinDlg** dialog);												//004ADF20
	s32 initSingleplayerCustom(BinDlg** dialog);									//004AD850
	s32 initGameLoad(s32 a1, char a2);												//004EF100
	*/
	void displayEstablishingShot(); // 004D9200, hooked by X405
	void injectGlucustomHooks();
}