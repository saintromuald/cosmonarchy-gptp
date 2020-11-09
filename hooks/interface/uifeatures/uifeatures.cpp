#include "uifeatures.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "config.h"

namespace {
	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
	s32 GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

	s32* const fonts = (s32*)0x006CE0F4;
	char* const printXYAlignment = (char*)0x006CE110;
	Box16* const drawTextBox = (Box16*)0x006CE0C8;

	// For targetedOrder
	COrder* queueNewOrder(COrder* a1, s16 a2, s16 a3, CUnit* a4, u8 a5, s32 a6, s16 a7, Point32* a8, CUnit* a9);	//00474B90
	s32 showStatTxtToPlayer(s32 a1, s32 a2); //0048CF00
	void setOrderTarget(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, CUnit* a6, s32 a7, Point32* a8, CUnit* a9); //00475470

	// For selectUnits
	u32 CUnitToUnitID(CUnit* a1); //0047B1D0
	s32 selectSingleUnitFromID(s32 a1); //00496D30
	s32 selectMultipleUnitsFromUnitList(s32 a1, CUnit** a2, u8 a3, s32 a4); //0049AEF0
	s32 SNetGetTurnsInTransit(u32* turns);
	u8 sendTurn();
	s32 SErrGetLastError();
	u8 packetErrHandle(s32 result, s32 a1, const char* a3, s32 a4, s32 a5);
}

namespace {
	//struct Action {
	//	u32 frame; // timestamp
	//};
	//
	//std::vector<Action> actions;

	uint64_t totalActions;
}

namespace hooks {
	void drawText(const char* text, s32 x, s32 y, s32 width, s32 height, char color = 2) {
		GetColorHandle(color);
		*printXYAlignment = 0x11;
		drawTextBox->left = x;
		drawTextBox->right = x + width;
		drawTextBox->top = y;
		drawTextBox->bottom = y + height;
		BWFXN_PrintXY(text, x, y);
	}

	std::string numberWithLeadingZero(u32 n) {
		std::string str = std::to_string(n);
		if (n < 10) {
			str = std::string("0") + str;
		}
		return str;
	}

	std::string getFormattedTime(u32 seconds) {
		u32 sec = seconds % 60;
		u32 min = seconds / 60;
		u32 hrs = seconds / 60 / 60;
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

	u32 getAPM() {
		u32 actionCount = totalActions;//actions.size();
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
			} else if (CurrentUnitSelection->unit[i] != NULL) {
				return false;
			}
		}
		return false;
	}

	u8 QueueGameCommand(const void* a1, u32 a2) {
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
		if (SNetGetTurnsInTransit(&turns)) {
			v2 = LOBYTE(turns) | HIBYTE(v2);
			if (turns >= 16 - *provider_LatencyCalls)
				return v2;
			sendTurn();
LABEL_8:
			_sgdwBytesInCmdQueue = *sgdwBytesInCmdQueue;
			memcpy((void*)&TurnBuffer[*sgdwBytesInCmdQueue], buffer, len);
			v2 = len + _sgdwBytesInCmdQueue;
			*sgdwBytesInCmdQueue = v2;
			return v2;
		}
		v2 = LOBYTE(outOfGame) | HIBYTE(v2);
		if (!outOfGame) {
			v5 = SErrGetLastError();
			v2 = packetErrHandle(v5, 81, "Starcraft\\SWAR\\lang\\net_mgr.cpp", 225, 1) | HIBYTE(v2);
		}
		return v2;
	}

	void targetedOrder(s16 a1, s32 a2, CUnit* a3, s16 a4, s32 a5, s32 a6, u8 a7, s32 a8, s32 a9, s32 a10)
	{
		if (a6 && orders_dat::CanBeQueued[(u8)a7])
		{
			switch (a3->mainOrderId)
			{
			case 2:
			case 3:
			case 23:
			case 93:
			case 152:
			case 175:
				goto LABEL_15;
			default:
				if (a3->orderQueueCount < 8u)
				{
					if (*COrderCount < 1800)
					{
						queueNewOrder((COrder*)a5, a1, a4, a3, a7, a8, a2, (Point32*)a9, (CUnit*)a10);
					}
					else
					{
						u8 v11 = a3->playerId;
						if (*statTxtTbl > 0x368u)
							showStatTxtToPlayer(*statTxtTbl + statTxtTbl[873], v11);
						else
							showStatTxtToPlayer((s32)"", v11);
					}
				}
				else
				{
					u8 v10 = a3->playerId;
					if (*statTxtTbl > 0x366u)
						showStatTxtToPlayer(*statTxtTbl + statTxtTbl[871], v10);
					else
						showStatTxtToPlayer((s32)"", v10);
				}
				break;
			}
		}
		else
		{
		LABEL_15:
			setOrderTarget(a7, a1, a4, a5, (s32)a3, (CUnit*)a8, (u16)a2, (Point32*)a9, (CUnit*)a10);
		}
	}

	s32 selectUnits(CUnit** a1, s32 a2, u8 a3, s32 a4) {
		s32 v4 = CUnitToUnitID(*a1);
		s32 result = selectSingleUnitFromID(v4); // eax

		if (a2 != 1 || !IS_KEYCODE_USED[18] || result == 0)
			result = selectMultipleUnitsFromUnitList(a2, a1, a3, a4);
		return result;
	}
}

namespace {
	const u32 BWFXN_PrintXY_function = 0x004202B0;
	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y) {
		static s32 result;
		_asm {
			PUSHAD
			PUSH y
			MOV EAX, text
			MOV ESI, x
			CALL BWFXN_PrintXY_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 GetColorHandle_function = 0x0041F610;
	s32 GetColorHandle(u8 color) {
		static int result;
		_asm {
			PUSHAD
			MOV AL, color
			CALL GetColorHandle_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For targetedOrder

	const u32 Func_queueNewOrder = 0x00474B90;
	COrder* queueNewOrder(COrder* a1, s16 a2, s16 a3, CUnit* a4, u8 a5, s32 a6, s16 a7, Point32* a8, CUnit* a9) {
		static COrder* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV DX, a2
			MOV CX, a3
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			PUSH a4
			CALL Func_queueNewOrder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_showStatTxtToPlayer = 0x0048CF00;
	s32 showStatTxtToPlayer(s32 a1, s32 a2) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOV ECX, a2
			CALL Func_showStatTxtToPlayer
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_setOrderTarget = 0x00475470;
	void setOrderTarget(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, CUnit* a6, s32 a7, Point32* a8, CUnit* a9) {
		__asm {
			PUSHAD
			MOV CL, a1
			MOV DX, a2
			MOV AX, a3
			MOV EDI, a4
			PUSH a9
			PUSH a8
			PUSH a7
			PUSH a6
			PUSH a5
			CALL Func_setOrderTarget
			POPAD
		}
	}

	// For selectUnits

	const u32 Func_CUnitToUnitId = 0x0047B1D0;
	u32 CUnitToUnitID(CUnit* a1) {
		static u32 result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_CUnitToUnitId
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_selectSingleUnitFromID = 0x00496D30;
	s32 selectSingleUnitFromID(s32 a1) {
		static s32 result;
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_selectSingleUnitFromID
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
	s32 selectMultipleUnitsFromUnitList(s32 a1, CUnit** a2, u8 a3, s32 a4) {
		static s32 result;
		__asm {
			PUSHAD
			PUSH a4
			PUSH a3
			PUSH a2
			PUSH a1
			CALL Func_selectMultipleUnitsFromUnitList
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 SNetGetTurnsInTransit_function = 0x00410275;
	s32 SNetGetTurnsInTransit(u32* turns) {
		static s32 result;
		_asm {
			PUSHAD
			PUSH turns
			CALL SNetGetTurnsInTransit_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 sendTurn_function = 0x00485A40;
	u8 sendTurn() {
		static u32 result;
		_asm {
			PUSHAD
			CALL sendTurn_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 SErrGetLastError_function = 0x004100FA;
	s32 SErrGetLastError() {
		static s32 result;
		_asm {
			PUSHAD
			CALL SErrGetLastError_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 packetErrHandle_function = 0x004BB0B0;
	u8 packetErrHandle(s32 result_arg, s32 a1, const char* a3, s32 a4, s32 a5) {
		static u32 result;
		_asm {
			PUSHAD
			MOV EAX, result_arg
			PUSH a5
			PUSH a4
			PUSH a3
			PUSH a1
			CALL packetErrHandle_function
			MOV result, EAX
			POPAD
		}
		return result;
	}
}