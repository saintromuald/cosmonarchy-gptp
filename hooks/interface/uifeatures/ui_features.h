#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

void format_time(char* buffer, u32 size, char* prefix, u32 seconds);

namespace hooks
{
	void drawText(const char* text, s32 x, s32 y, s32 width, s32 height, char color = 2);

	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10);
	s32 selectUnits(Unit** a1, s32 a2, u8 a3, s32 a4);
	u8 QueueGameCommand(const void* a1, u32 a2);

	b32 checkHotkeyChar(BinDlg* dialog, char hotkey);
	char createUpgradeLevelTooltip(char* text);

	void injectUIFeaturesHooks();

	void drawUIFeatures();
	void registerAction();
	void clearActions();
}