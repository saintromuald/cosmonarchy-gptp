#include "uifeatures.h"
#include <string>
#include <sstream>

namespace {
	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
	//void SetFontHandle(s32* font);
	int GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

	s32* const fonts = (s32*)0x006CE0F4;
	char* const printXYAlignment = (char*)0x006CE110;
	Box16* const drawTextBox = (Box16*)0x006CE0C8;
	void* const currentSurface = (void*)0x006CF4A8;
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

	void drawUIFeatures() {
		std::string time = "Time: " + getFormattedTime(*elapsedTimeFrames / 24);
		drawText(time.c_str(), 10, 3, 100, 20, 7);

		//std::string apm = "APM: " + 0;
		//drawText(apm.c_str(), 10, 23, 100, 20, 7);
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

	/*
	const u32 SetFontHandle_function = 0x0041FB30;
	void SetFontHandle(s32* font) {
		_asm {
			PUSHAD
			MOV ECX, font
			CALL SetFontHandle_function
			POPAD
		}
	}
	*/

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
}