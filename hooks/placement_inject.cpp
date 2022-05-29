#include "placement.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) InitPlacementBoxPos_Wrapper()
	{
		static s32 unused;
		static Point16* placement;
		static s32 result;

		__asm
		{
			MOV unused, ECX
			MOV placement, EDX
			PUSHAD
		}
		result = hooks::init_placement_box_pos(unused, placement);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) DrawPlacementBox_Wrapper()
	{
		static Point16 ecx0;
		static u8 result;

		__asm
		{
			MOV ecx0, ECX
			PUSHAD
		}
		result = hooks::draw_placement_box(ecx0);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) updatePlacementStateForPlacementBox_Wrapper() // 0048DCE0
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::update_placement_state_for_placement_box();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) updateBuildingPlacementState_Units_Wrapper() // 00473720
	{
		static s32 result;
		static s32 yTile;
		static Unit* builder;
		static s32 xTile;
		static u8 player;
		static u16 unitId;
		static s32 sizeWidthHeight;
		static s32 placementStateEntry;
		static char bDontIgnoreReacting;
		static char bAlsoInvisible;
		static s32 withoutVision;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV builder, EAX
			MOV EAX, [EBP + 0x0C]
			MOV xTile, EAX
			MOV EAX, [EBP + 0x10]
			MOV player, AL
			MOV EAX, [EBP + 0x14]
			MOV unitId, AX
			MOV EAX, [EBP + 0x18]
			MOV sizeWidthHeight, EAX
			MOV EAX, [EBP + 0x1C]
			MOV placementStateEntry, EAX
			MOV EAX, [EBP + 0x20]
			MOV bDontIgnoreReacting, AL
			MOV EAX, [EBP + 0x24]
			MOV bAlsoInvisible, AL
			MOV EAX, [EBP + 0x28]
			MOV withoutVision, EAX
			POP EAX
			MOV yTile, EAX
			PUSHAD
		}
		hooks::updateBuildingPlacementState_Units(yTile, builder, xTile, player, unitId, sizeWidthHeight, placementStateEntry, bDontIgnoreReacting, bAlsoInvisible, withoutVision);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN 0x24
		}
	}

	void __declspec(naked) isTileBlockedBy_Wrapper() { // 00473300
		static s32 result;
		static Unit** unitArray;
		static Unit* builder;
		static s32 x_tile;
		static s32 y_tile;
		static s8 dontIgnoreReacting;
		static s8 alsoInvisible;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV builder, EAX
			MOV EAX, [EBP + 0x0C]
			MOV x_tile, EAX
			MOV EAX, [EBP + 0x10]
			MOV y_tile, EAX
			MOV EAX, [EBP + 0x14]
			MOV dontIgnoreReacting, AL
			MOV EAX, [EBP + 0x18]
			MOV alsoInvisible, AL
			POP EAX
			MOV unitArray, EAX
			PUSHAD
		}
		result = hooks::is_tile_blocked_by(unitArray, builder, x_tile, y_tile, dontIgnoreReacting, alsoInvisible);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}
}

namespace hooks
{
	void injectPlacementHooks()
	{
		jump_patch(InitPlacementBoxPos_Wrapper,					0x0048D660, 0);
		jump_patch(DrawPlacementBox_Wrapper,					0x0048DDC0, 4);
//		jump_patch(updatePlacementStateForPlacementBox_Wrapper,	0x0048DCE0, 0);
//		jump_patch(updateBuildingPlacementState_Units_Wrapper,	0x00473720, 1);
		jump_patch(isTileBlockedBy_Wrapper,						0x00473300, 0);
	}
}