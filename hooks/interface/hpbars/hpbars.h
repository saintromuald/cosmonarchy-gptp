#pragma once
#include "SCBW/scbwdata.h"

namespace hooks
{
	void ShowHealthBar(Unit* unit);
	void compileHealthBar(Image* a1, Sprite* a2);

	void injectHpBarsHooks();

	extern Unit* _unitTemp;
}