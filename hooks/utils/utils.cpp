#include "utils.h"
#include <SCBW/api.h>

// Those functions are less likely to be useful (especially if modified alone against as a part of a larger modification), or may cause more trouble if modified the wrong way, so they are put here a bit on the side.

namespace hooks
{	
	void initialize_dialog(BinDlg* dialog, void* dlgFunc) // 00419D20
	{
		struct
		{
			u32 unknown_value;		//[EBP-0x14]	[00]
			u32 always_zero_here;	//[EBP-0x10]	[04]
			u32 not_allocated_2;	//[EBP-0x0C]	[08]
			u16 always_14_here;		//[EBP-0x08]	[0C]
			s16 mouseX;				//[EBP-0x06]	[0E]
			s16 mouseY;				//[EBP-0x04]	[10]
			u16 not_allocated_1;	//[EBP-0x02]	[12]
		} stack_placeholder;

		// not allocated on original code, but result is probably more true to the original like this
		stack_placeholder.not_allocated_1 = 0;
		stack_placeholder.not_allocated_2 = 0;

		dialog->flags |= (BinDlgFlags::NoClickSound + BinDlgFlags::VerticalAlignmentBottom);
		dialog->onInitDialog0x46.unk4a = 0;

		if (dlgFunc != nullptr)
		{
			dialog->fxn_interact = dlgFunc;
		}

		dialog->onInitDialog0x36.response_area_bit_array = (u32*)helper::SMemAlloc(dialog->onInitDialog0x36.response_area_width * dialog->onInitDialog0x36.response_area_height, (char*)0x00505F98, 481, 0);
		dialog->index = 0;

		stack_placeholder.mouseX = mouse->x;
		stack_placeholder.always_14_here = 14;	// 0x000E
		stack_placeholder.unknown_value = 7;	// 0x00000007
		stack_placeholder.always_zero_here = 0;
		stack_placeholder.mouseY = mouse->y;

		helper::fxn_interact(dialog, (u32)&stack_placeholder);

		dialog->bounds.right += dialog->bounds.left;
		dialog->bounds.bottom += dialog->bounds.top;

		stack_placeholder.mouseY = mouse->y;
		stack_placeholder.always_14_here = 14;	// 0x000E
		stack_placeholder.unknown_value = 10;	// 0x0000000A
		stack_placeholder.always_zero_here = 0;
		stack_placeholder.mouseX = mouse->x;

		helper::fxn_interact(dialog, (u32)&stack_placeholder);

		stack_placeholder.mouseX = mouse->x;
		stack_placeholder.always_14_here = 14;	// 0x000E
		stack_placeholder.unknown_value = 0;
		stack_placeholder.always_zero_here = 0;
		stack_placeholder.mouseY = mouse->y;

		helper::fxn_interact(dialog, (u32)&stack_placeholder);
		helper::set_callback_timer(dialog, 12, 100, 0);

		// equivalent to "if(dialog->flags & BinDlgFlags::VerticalAlignmentBottom) dialog->flags -= BinDlgFlags::VerticalAlignmentBottom;"
		dialog->flags &= ~BinDlgFlags::VerticalAlignmentBottom;
	}
	
	bool unit_is_standard_and_movable(Unit* unit) { // 0047B770
		if (units_dat::BaseProperty[unit->id] & UnitProperty::NeutralAccessories
			|| unit->status & UnitStatus::Frozen
			|| unit->id == UnitId::TerranSpiderMine
			|| unit->id == UnitId::TerranLobotomyMine
			|| unit->id == UnitId::Spell_DarkSwarm
			|| unit->id == UnitId::Spell_DisruptionWeb
			|| unit->id == UnitId::Special_PitDoor
			|| unit->id == UnitId::Special_RightPitDoor
			|| unit->id == UnitId::Special_UpperLevelDoor
			|| unit->id == UnitId::Special_RightUpperLevelDoor) {
			return false;
		}
		return true;
	}
		
	Unit* get_active_player_first_selection() // 0049A850
	{
		Unit* unit;
		if (*unit_iterator_pos >= 12)
		{
			unit = nullptr;
		}
		else
		{
			s32 player = *ACTIVE_PLAYER_ID;
			u8 index = *unit_iterator_pos;
			bool bStopLoop = false;
			while (!bStopLoop)
			{
				unit = players_selections->unit[player][index];
				if (unit == nullptr)
				{
					bStopLoop = true;
				}
				else if (unit->sprite != nullptr)
				{
					if (unit->main_order_id != OrderId::Die)
					{
						bStopLoop = true;
					}
					else if (unit->main_order_state != 1)
					{
						bStopLoop = true;
					}
				}

				if (!bStopLoop)
				{
					helper::remove_from_selections(unit);
				}
			}
			*unit_iterator_pos = *unit_iterator_pos + 1;
		}
		return unit;
	}
}
