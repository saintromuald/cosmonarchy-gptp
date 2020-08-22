#include "uifeatures.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace {
	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
	int GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

	s32* const fonts = (s32*)0x006CE0F4;
	char* const printXYAlignment = (char*)0x006CE110;
	Box16* const drawTextBox = (Box16*)0x006CE0C8;
	void* const currentSurface = (void*)0x006CF4A8;

	// For targetedOrder
	COrder* queueNewOrder(COrder* a1, __int16 a2, __int16 a3, CUnit* a4, unsigned __int8 a5, int a6, __int16 a7, Point32* a8, CUnit* a9);	//00474B90
	int showStatTxtToPlayer(int a1, int a2); //0048CF00
	void setOrderTarget(char a1, __int16 a2, __int16 a3, int a4, int a5, CUnit* a6, int a7, Point32* a8, CUnit* a9); //00475470
}

namespace {
	struct Action {
		u32 frame; // timestamp
	};

	std::vector<Action> actions;
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

	void registerAction() {
		actions.push_back(Action{*elapsedTimeFrames});
	}

	void cleanActions() {
		std::remove_if(actions.begin(), actions.end(), [](const Action& action) {
			return *elapsedTimeFrames - action.frame > 24 * 60; // remove actions older than one minute
		});
	}

	void drawUIFeatures() {
		std::string time = getFormattedTime(*elapsedTimeFrames / 24);
		drawText(time.c_str(), 10, 3, 100, 20, 7);

		hooks::cleanActions();
		std::string apm = "APM: " + std::to_string(actions.size());
		drawText(apm.c_str(), 10, 23, 100, 20, 7);
	}

	void targetedOrder(__int16 a1, int a2, CUnit* a3, __int16 a4, int a5, int a6, char a7, int a8, int a9, int a10)
	{
		unsigned __int8 v10; // dl
		unsigned __int8 v11; // dl

		if (a6 && orders_dat::CanBeQueued[(unsigned __int8)a7])
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
					if ((unsigned int)COrderCount < 1800)
					{
						queueNewOrder((COrder*)a5, a1, a4, a3, a7, a8, a2, (Point32*)a9, (CUnit*)a10);
					}
					else
					{
						v11 = a3->playerId;
						if (*statTxtTbl > 0x368u)
							showStatTxtToPlayer((int)statTxtTbl + statTxtTbl[873], v11);
						else
							showStatTxtToPlayer((int)"", v11);
					}
				}
				else
				{
					v10 = a3->playerId;
					if (*statTxtTbl > 0x366u)
						showStatTxtToPlayer((int)statTxtTbl + statTxtTbl[871], v10);
					else
						showStatTxtToPlayer((int)"", v10);
				}
				break;
			}
		}
		else
		{
		LABEL_15:
			setOrderTarget(a7, a1, a4, a5, (int)a3, (CUnit*)a8, (unsigned __int16)a2, (Point32*)a9, (CUnit*)a10);
		}
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
	int GetColorHandle(u8 color) {
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
	COrder* queueNewOrder(COrder* a1, __int16 a2, __int16 a3, CUnit* a4, unsigned __int8 a5, int a6, __int16 a7, Point32* a8, CUnit* a9) {
		static COrder* result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX DX, a2
			MOVZX CX, a3
			PUSH a4
			PUSH a5
			PUSH a6
			PUSH a7
			PUSH a8
			PUSH a9
			CALL Func_queueNewOrder
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_showStatTxtToPlayer = 0x0048CF00;
	int showStatTxtToPlayer(int a1, int a2) {
		static int result;
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
	void setOrderTarget(char a1, __int16 a2, __int16 a3, int a4, int a5, CUnit* a6, int a7, Point32* a8, CUnit* a9) {
		__asm {
			PUSHAD
			MOVZX CL, a1
			MOVZX DX, a2
			MOVZX AX, a3
			MOV EDI, a4
			PUSH a5
			PUSH a6
			PUSH a7
			PUSH a8
			PUSH a9
			CALL Func_setOrderTarget
			POPAD
		}
	}

}