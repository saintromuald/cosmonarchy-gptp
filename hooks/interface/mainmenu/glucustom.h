#pragma once
#include "SCBW/structures.h"

namespace hooks {

	void raceDropdown(BinDlg** dialog, int);										//004ADF20
	int initSingleplayerCustom(BinDlg** dialog);									//004AD850
	int initGameLoad(int, char);													//004EF100
	void injectGlucustomHooks();
}