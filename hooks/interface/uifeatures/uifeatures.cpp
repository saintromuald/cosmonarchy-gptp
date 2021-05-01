#include "uifeatures.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "config.h"

namespace {
	/*
	struct Action {
		uint32 frame; // timestamp
	};
	*/
//	std::vector<Action> actions;
	uint64 totalActions;

	int32* const fonts = (int32*)0x006CE0F4;
	char* const printXYAlignment = (char*)0x006CE110;
	Box16* const drawTextBox = (Box16*)0x006CE0C8;
}

namespace hooks {
	void drawText(const char* text, int32 x, int32 y, int32 width, int32 height, char color = 2) {
		helper::GetColorHandle(color);
		*printXYAlignment = 0x11;
		drawTextBox->left = x;
		drawTextBox->right = x + width;
		drawTextBox->top = y;
		drawTextBox->bottom = y + height;
		helper::BWFXN_PrintXY(text, x, y);
	}

	std::string numberWithLeadingZero(uint32 n) {
		std::string str = std::to_string(n);
		if (n < 10) {
			str = std::string("0") + str;
		}
		return str;
	}

	std::string getFormattedTime(uint32 seconds) {
		uint32 sec = seconds % 60;
		uint32 min = seconds / 60;
		uint32 hrs = seconds / 60 / 60;
		return numberWithLeadingZero(hrs) + std::string(":") + numberWithLeadingZero(min) + std::string(":") + numberWithLeadingZero(sec);
	}
	/*
		We have hooks for:
		- Most Orders
		- Shift, Shift+Box, Double Click
		- Queue Game Command

		We need hooks for*:
		- Morph, training and building orders
		- Lift off, land orders
		- Single unit selection, drag selection, select larva
		- Invoking control groups
	*/
	void registerAction() {
		//actions.push_back(Action{*elapsedTimeFrames});
		totalActions++;
	}

	void clearActions() { // Called on map start
		//actions.clear();
		totalActions = 0;
	}

	uint32 getAPM() {
		uint32 actionCount = totalActions;//actions.size();
		double divider = (*elapsedTimeFrames) / 24.0 / 60.0;

		return actionCount / divider;
	}

	void drawUIFeatures() {
		if (config.show_timer) {
			std::string time = getFormattedTime(*elapsedTimeFrames / 24);
			drawText(time.c_str(), 10, 3, 100, 15, 7);
		}

		//std::string apm = "APM: " + std::to_string(getAPM());
		//drawText(apm.c_str(), 10, 15, 100, 15, 7);
	}

	bool isFirstSelectedUnitOfCurrentPlayer(CUnit* unit) {
		for (int i = 0; i < 12; i++) {
			if (CurrentUnitSelection->unit[i] == unit) {
				return true;
			} else if (CurrentUnitSelection->unit[i] != nullptr) {
				return false;
			}
		}
		return false;
	}

	uint8 QueueGameCommand(const void* a1, uint32 a2) {
		uint32 v2; // eax
		uint32 len; // ebx
		const void* buffer; // esi
		int32 v5; // eax
		uint32 _sgdwBytesInCmdQueue; // eax
		uint32 turns; // [esp+8h] [ebp-4h]

		len = a2;
		buffer = a1;
		v2 = LOWORD(a2 + *sgdwBytesInCmdQueue) | HIWORD(v2);
		if (a2 + *sgdwBytesInCmdQueue <= *provider_MaxBuffer)
			goto LABEL_8;
		if (*scMainState == 4)
			return v2;
		if (helper::SNetGetTurnsInTransit(&turns)) {
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
		if (!outOfGame) {
			v5 = helper::SErrGetLastError();
			v2 = helper::packetErrHandle(v5, 81, "Starcraft\\SWAR\\lang\\net_mgr.cpp", 225, 1) | HIBYTE(v2);
		}
		return v2;
	}

	void targetedOrder(int16 a1, int32 a2, CUnit* a3, int16 a4, int32 a5, int32 a6, uint8 a7, int32 a8, int32 a9, int32 a10) {
		if (a6 && orders_dat::CanBeQueued[(uint8)a7]) {
			switch (a3->mainOrderId) {
				case 2:
				case 3:
				case 23:
				case 93:
				case 152:
				case 175:
					goto LABEL_15;
				default:
					if (a3->orderQueueCount < 8u) {
						if (*COrderCount < 1800) {
							helper::queueNewOrder((COrder*)a5, a1, a4, a3, a7, a8, a2, (Point32*)a9, (CUnit*)a10);
						} else {
							uint8 v11 = a3->playerId;
							if (*statTxtTbl > 0x368u)
								helper::showStatTxtToPlayer(*statTxtTbl + statTxtTbl[873], v11);
							else
								helper::showStatTxtToPlayer((int32)"", v11);
						}
					} else {
						uint8 v10 = a3->playerId;
						if (*statTxtTbl > 0x366u)
							helper::showStatTxtToPlayer(*statTxtTbl + statTxtTbl[871], v10);
						else
							helper::showStatTxtToPlayer((int32)"", v10);
					}
					break;
			}
		} else {
		LABEL_15:
			helper::setOrderTarget(a7, a1, a4, a5, (int32)a3, (CUnit*)a8, (uint16)a2, (Point32*)a9, (CUnit*)a10);
		}
	}

	int32 selectUnits(CUnit** a1, int32 a2, uint8 a3, int32 a4) {
		int32 v4 = helper::CUnitToUnitID(*a1);
		int32 result = helper::selectSingleUnitFromID(v4); // eax

		if (a2 != 1 || !IS_KEYCODE_USED[18] || result == 0)
			result = helper::selectMultipleUnitsFromUnitList(a2, a1, a3, a4);
		return result;
	}
}