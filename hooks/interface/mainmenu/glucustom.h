#pragma once
#include "SCBW/structures.h"

namespace hooks {
	/*
	void raceDropdown(BinDlg** dialog);												//004ADF20
	int initSingleplayerCustom(BinDlg** dialog);									//004AD850
	int initGameLoad(int a1, char a2);												//004EF100
	*/
	void displayEstablishingShot(); // 004D9200, hooked by X405
	void injectGlucustomHooks();
}