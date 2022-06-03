#include "ui_features.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "config.h"

namespace
{
	/*
	struct Action {
		u32 frame; // timestamp
	};
	*/
	//	std::vector<Action> actions;
	u64 totalActions;

	s32* const fonts = (s32*)0x006CE0F4;
	char* const printXYAlignment = (char*)0x006CE110;
	Box16* const drawTextBox = (Box16*)0x006CE0C8;
}

void format_time(char* buffer, u32 size, char* prefix, u32 seconds) {
	u32 sec = seconds % 60;
	u32 min = seconds / 60 % 60;
	u32 hrs = seconds / 60 / 60;

	sprintf_s(buffer, size, "%s%02u:%02u:%02u", prefix, hrs, min, sec);
}

namespace hooks
{
	void drawText(const char* text, s32 x, s32 y, s32 width, s32 height, char color)
	{
		helper::GetColorHandle(color);
		*printXYAlignment = 0x11;
		drawTextBox->left = x;
		drawTextBox->right = x + width;
		drawTextBox->top = y;
		drawTextBox->bottom = y + height;
		helper::BWFXN_PrintXY(text, x, y);
	}
	
	/*
		APM Counter TODO

		We have hooks for:
		- Most Orders
		- Shift, Shift+Box, Double Click
		- Queue Game Command

		We need hooks for:
		- Morph, training and building orders
		- Lift off, land orders
		- Single unit selection, drag selection, select larva
		- Invoking control groups
	*/
	void registerAction()
	{
		//actions.push_back(Action{*elapsedTimeFrames});
		totalActions++;
	}

	void clear_actions() // Called on map start
	{
		//actions.clear();
		totalActions = 0;
	}

	u32 getAPM()
	{
		u32 actionCount = totalActions;//actions.size();
		f64 divider = (*elapsedTimeFrames) / 24.0 / 60.0;

		return actionCount / divider;
	}

	void drawUIFeatures()
	{
		if (config.show_timer)
		{
			char buffer[32] = {0};
			format_time(buffer, 32, "", * elapsedTimeFrames / 24);
			drawText(buffer, 10, 3, 100, 15, 7);
		}

		//std::string apm = "APM: " + std::to_string(getAPM());
		//drawText(apm.c_str(), 10, 15, 100, 15, 7);
	}

	bool isFirstSelectedUnitOfCurrentPlayer(Unit* unit)
	{
		for (s32 i = 0; i < 12; i++)
		{
			if (CurrentUnitSelection->unit[i] == unit)
			{
				return true;
			}
			else if (CurrentUnitSelection->unit[i] != nullptr)
			{
				return false;
			}
		}
		return false;
	}

	u8 queue_game_command(const void* a1, u32 a2)
	{
		u32 v2; // eax
		u32 len; // ebx
		const void* buffer; // esi
		s32 v5; // eax
		u32 _sgdwBytesInCmdQueue; // eax
		u32 turns; // [esp+8h] [ebp-4h]

		len = a2;
		buffer = a1;
		v2 = LOWORD(a2 + *sgdwBytesInCmdQueue) | HIWORD(v2);
		if (a2 + *sgdwBytesInCmdQueue <= *provider_MaxBuffer)
			goto LABEL_8;
		if (*scMainState == 4)
			return v2;
		if (helper::SNetGetTurnsInTransit(&turns))
		{
			v2 = LOBYTE(turns) | HIBYTE(v2);
			if (turns >= 16 - *provider_LatencyCalls)
				return v2;
			helper::sendTurn();
LABEL_8:
			_sgdwBytesInCmdQueue = *sgdwBytesInCmdQueue;
			memcpy((void*)&TurnBuffer[*sgdwBytesInCmdQueue], buffer, len);
			v2 = len + _sgdwBytesInCmdQueue;
			*sgdwBytesInCmdQueue = v2;
			return v2;
		}
		v2 = LOBYTE(outOfGame) | HIBYTE(v2);
		if (!outOfGame)
		{
			v5 = helper::SErrGetLastError();
			v2 = helper::packetErrHandle(v5, 81, "Starcraft\\SWAR\\lang\\net_mgr.cpp", 225, 1) | HIBYTE(v2);
		}
		return v2;
	}

	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10)
	{
		if (a6 && orders_dat::CanBeQueued[(u8)a7])
		{
			switch (a3->main_order_id)
			{
				case 2:
				case 3:
				case 23:
				case 93:
				case 152:
				case 175:
					goto LABEL_15;
				default:
					if (a3->order_queue_count < 8u)
					{
						if (*COrderCount < 1800)
						{
							helper::queue_new_order((Order*)a5, a1, a4, a3, a7, a8, a2, (Vector32*)a9, (Unit*)a10);
						}
						else
						{
							u8 v11 = a3->player;
							if (*statTxtTbl > 0x368u)
								helper::show_stat_txt_to_player(*statTxtTbl + statTxtTbl[873], v11);
							else
								helper::show_stat_txt_to_player((s32)"", v11);
						}
					}
					else
					{
						u8 v10 = a3->player;
						if (*statTxtTbl > 0x366u)
							helper::show_stat_txt_to_player(*statTxtTbl + statTxtTbl[871], v10);
						else
							helper::show_stat_txt_to_player((s32)"", v10);
					}
					break;
			}
		}
		else
		{
LABEL_15:
			helper::set_order_target(a7, a1, a4, a5, (s32)a3, (Unit*)a8, (u16)a2, (Vector32*)a9, (Unit*)a10);
		}
	}

	s32 selectUnits(Unit** a1, s32 a2, u8 a3, s32 a4)
	{
		s32 v4 = helper::CUnitToUnitID(*a1);
		s32 result = helper::select_single_unit_from_id(v4); // eax

		if (a2 != 1 || !IS_KEYCODE_USED[18] || result == 0)
			result = helper::selectMultipleUnitsFromUnitList(a2, a1, a3, a4);
		return result;
	}

	b32 checkHotkeyChar(BinDlg* dialog, char hotkey)
	{
		if (dialog->index < 0 || dialog->index > 9)
		{
			return 0;
		}
		if (!(dialog->flags & BinDlgFlags::Visible) || dialog->flags & BinDlgFlags::Disabled)
		{
			return 0;
		}

		char* label = helper::getStatTxtString(dialog->button_user->reqStringID);
		if (*label == 27 && hotkey == label[0]) // escape handling for grid hotkeys
		{
			return 1;
		}

		if (config.grid_hotkeys && dialog->index > 0)
		{
			int index = dialog->index - 1;
			int x = index % 3;
			int y = index / 3;
			return hotkey == tolower(config.layout[y].at(x));
		}

		return hotkey == label[0];
	}

	char createUpgradeLevelTooltip(char* text)
	{
		if (!*upgradeLevelTooltip)
		{
			if (text)
			{
				return helper::create_tooltip(text);
			}
			return helper::refresh_selection_screen();
		}
		if (!text)
		{
			return helper::refresh_selection_screen();
		}
		const char* description;
		if (**statTxtBuffer > 794u)
		{
			description = (char *)*statTxtBuffer + (*statTxtBuffer)[795];
		}
		else
		{
			description = "";
		}
		char buffer[128];
		sprintf_s(buffer, 128, "%s\n%s %d", text, description, (s16)*upgradeLevelTooltip);
		scbw::printf("%s", buffer);
		return helper::create_tooltip(buffer);
	}

	// TODO: Hook 004593A0 for time icon/cost on tooltip (and removing supply icon from tooltip)
}