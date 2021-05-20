#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace hooks
{
	void drawUIFeatures();
	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10);
	s32 selectUnits(Unit** a1, s32 a2, u8 a3, s32 a4);
	u8 QueueGameCommand(const void* a1, u32 a2);
	b32 checkHotkeyChar(BinDlg* dialog, char hotkey);

	void injectUIFeaturesHooks();

	void registerAction();
	void clearActions();
}