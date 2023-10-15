#include "update_unit_sight.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) gndSightCB_Inside_wrapper() {
		static u32 tileX;
		static u32 tileY;
		static SightRange* sightVals;
		static ActiveTile* maskData;
		static u32 relFlags;
		static u32 newVisionFlags;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
			MOV tileX, ECX
			MOV tileY, EDX
			MOV EAX, [EBP + 0x8]
			MOV sightVals, EAX
			MOV EAX, [EBP + 0xC]
			MOV maskData, EAX
			MOV EAX, [EBP + 0x10]
			MOV relFlags, EAX
			MOV EAX, [EBP + 0x14]
			MOV newVisionFlags, EAX
		}

		hooks::gndSightCB_Inside(tileX, tileY, sightVals, maskData, relFlags, newVisionFlags);

		__asm {
			POPAD
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) gndSightCB_Edge_wrapper() {
		static u32 tileX;
		static u32 tileY;
		static SightRange* sightVals;
		static ActiveTile* maskData;
		static u32 relFlags;
		static u32 newVisionFlags;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
			MOV tileX, ECX
			MOV tileY, EDX
			MOV EAX, [EBP + 0x8]
			MOV sightVals, EAX
			MOV EAX, [EBP + 0xC]
			MOV maskData, EAX
			MOV EAX, [EBP + 0x10]
			MOV relFlags, EAX
			MOV EAX, [EBP + 0x14]
			MOV newVisionFlags, EAX
		}

		hooks::gndSightCB_Edge(tileX, tileY, sightVals, maskData, relFlags, newVisionFlags);

		__asm {
			POPAD
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) reveal_sight_at_location_wrapper() {
		static u32 sightRange;
		static u32 visionFlags;
		static s32 ptX;
		static s32 ptY;
		static u32 isAirUnit;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV sightRange, EAX
			MOV visionFlags, ECX
			MOV ptY, EDX
			MOV EAX, [EBP + 0x08]
			MOV ptX, EAX
		//	MOV EAX, [EBP + 0x0C]
		//	MOV ptY, EAX
			MOV EAX, [EBP + 0x10]
			MOV isAirUnit, EAX
			PUSHAD
		}

		hooks::reveal_sight_at_location(sightRange, visionFlags, ptX, ptY, isAirUnit);

		__asm {
			POPAD
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) getGroundHeightAtPx_wrapper() {
		static u32 offsetX;
		static u32 offsetY;

		__asm {
			PUSHAD
			MOV offsetY, EAX
			MOV offsetX, ECX
		}

		offsetX = hooks::get_ground_height_at_px(offsetY, offsetX);

		__asm {
			POPAD
			MOV EAX, offsetX
			RETN
		}
	}
}

namespace hooks {
	void inject_sight_update_hooks() {
		memory_patch(0x004D94D6, (u32)10); // Fog Update Counter
		call_patch(update_fog, 0x004D9500);
		jump_patch(reveal_sight_at_location_wrapper, 0x004806F0, 1);
		jump_patch(getGroundHeightAtPx_wrapper, 0x004BD0F0, 6);
	}
}