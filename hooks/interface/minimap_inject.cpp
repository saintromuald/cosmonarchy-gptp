#include "minimap.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) minimap_ping_wrapper() {
		static s32 posX;
		static s32 posY;
		static u32 color;

		__asm {
			PUSH	EBP
			MOV		EBP, ESP
			MOV		EAX, [EBP + 0x8]
			MOV		posX, EAX
			MOV		EAX, [EBP + 0xC]
			MOV		posY, EAX
			MOV		EAX, [EBP + 0x10]
			MOV		color, EAX
			PUSHAD
		}

		hooks::minimap_ping(posX, posY, (u8)color);

		__asm {
			POPAD
			MOV		ESP, EBP
			POP		EBP
			RETN	12
		}
	}

	void __declspec(naked) draw_unit_box_wrapper() {
		static s32 posX;
		static u32 color;
		static s32 posY;
		static u32 sizeX;
		static u32 sizeY;
		static b32 isBuilding;

		__asm {
			PUSH	EBP
			MOV		EBP, ESP
			MOV		posX, EAX
			MOV		color, ECX
			MOV		EAX, [EBP + 0x8]
			MOV		posY, EAX
			MOV		EAX, [EBP + 0xC]
			MOV		sizeX, EAX
			MOV		EAX, [EBP + 0x10]
			MOV		sizeY, EAX
			MOV		EAX, [EBP + 0x14]
			MOV		isBuilding, EAX
			PUSHAD
		}

		hooks::draw_unit_box(posX, (u8)color, posY, sizeX, sizeY, (bool)isBuilding);

		__asm {
			POPAD
			MOV		ESP, EBP
			POP		EBP
			RETN	16
		}
	}

	void __declspec(naked) minimapGameMouseUpdate_wrapper() {
		static BinDlg* minimap;

		__asm {
			//Truncated.
			MOV		minimap, EDI
			PUSHAD
		}

		hooks::minimapGameMouseUpdate(minimap);

		__asm {
			POPAD
			MOV		ESP, EBP
			POP		EBP
			RET
		}
	}

	void __declspec(naked) setMapSizeConstants_wrapper() {
		__asm {
			PUSHAD
		}

		hooks::setMapSizeConstants();

		__asm {
			POPAD
			RET
		}
	}
}

namespace hooks {
	void inject_minimap_hooks() {
		jump_patch(minimap_ping_wrapper, 0x004A34C0, 1);
		jump_patch(draw_unit_box_wrapper, 0x004A3FD0, 1);
		jump_patch(setMapSizeConstants_wrapper, 0x004A4400, 1);
		memory_patch(0x004A4CEA, &MinimapImageUpdate);
		jump_patch(minimapGameMouseUpdate_wrapper, 0x004A4D61, 2);
	}
}