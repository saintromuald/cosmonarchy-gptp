#include "research_upgrade_orders.h"
#include <SCBW/api.h>

namespace hooks
{

	void orders_Upgrade(Unit* unit)
	{

		bool bStopThere = false;
		Image* current_image;

		if (unit->status & UnitStatus::GroundedBuilding &&
			unit->building.upgrade_id != UpgradeId::None
			)
		{

			unit->building.upgrade_research_time--;

			if (unit->building.upgrade_research_time != 0xFFFF)
			{

				u8 upgrade_level;

				if (unit->building.upgrade_id >= UpgradeId::PrismaticShield)
					upgrade_level = UpgradesBw->currentLevel[unit->player][unit->building.upgrade_id - UpgradeId::PrismaticShield];
				else
					upgrade_level = UpgradesSc->currentLevel[unit->player][unit->building.upgrade_id];

				if (upgrade_level < unit->building.upgrade_level &&
					!(*bw_cheat_state & CheatFlags::OperationCwal))
					bStopThere = true;

			}

			//5471D
			if (!bStopThere)
			{

				u8 upgrade_level;

				helper::play_upgrade_complete_sound();

				if (unit->building.upgrade_id >= UpgradeId::PrismaticShield)
					upgrade_level = UpgradesBw->currentLevel[unit->player][unit->building.upgrade_id - UpgradeId::PrismaticShield];
				else
					upgrade_level = UpgradesSc->currentLevel[unit->player][unit->building.upgrade_id];

				if (upgrade_level < unit->building.upgrade_level)
				{

					if (helper::is_unit_upgrade_available(unit))
					{
						helper::set_upgrade_level(unit->player, unit->building.upgrade_id, unit->building.upgrade_level);
						helper::apply_speed_upgrade_from_upgrade_id(unit, unit->building.upgrade_id);
					}

				}

				//5477F
				scbw::refresh_console();

			}

		}

		//547A7
		if (!bStopThere)
		{

			current_image = unit->sprite->images.head;

			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WorkingToIdle);
				current_image = current_image->link.next;
			}

			//code I could not understand enough to convert
			__asm {

				PUSHAD

				MOV ESI, unit
				MOVZX ECX, byte ptr[ESI + 0xC9]	//unit->building.upgrade_id
				MOVZX EAX, byte ptr[ESI + 0x4C]	//unit->player
				MOV EDX, ECX
				SHR EDX, 0x3
				AND ECX, 0x80000007
				LEA EAX, [EDX + EAX * 0x8 + 0x0058F3E0] //EAX = &(UpgradesBw->currentLevel[unit->player][unit->building.upgrade_id CONVERTED?])
				JNS lbl547E7

				DEC ECX
				OR ECX, -0x8
				INC ECX

				lbl547E7 :

				MOV DL, 0x1
					SHL DL, CL
					MOV CL, [EAX]
					NOT DL
					AND CL, DL
					MOV[EAX], CL

					POPAD

			}

			unit->building.upgrade_id = UpgradeId::None;
			unit->building.upgrade_level = 0;

			if (unit->unit_ai != nullptr)
			{

				unit->user_action_flags |= 1;

				if (unit->main_order_id != OrderId::Die)
				{

					bool bStopLoop = (unit->order_queue_tail == nullptr);

					while (!bStopLoop)
					{

						if (!orders_dat::CanBeInterrupted[unit->order_queue_tail->order_id] &&
							unit->order_queue_tail->order_id != OrderId::ComputerAI
							)
							bStopLoop = true;
						else
							helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);

					}

					unit->perform_another_order(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);

				}

				prepare_for_next_order(unit);

			}
			else
			{ //54875

				if (unit->order_queue_head != nullptr)
				{
					unit->user_action_flags |= 1;
					prepare_for_next_order(unit);
				}
				else //54896
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);

			}

		}

	}

	;

	void orders_ResearchTech(Unit* unit)
	{

		bool bStopThere = false;

		if (unit->building.tech_id != TechId::None)
		{

			unit->building.upgrade_research_time--;

			if (unit->building.upgrade_research_time != 0xFFFF)
			{

				u8 bIsResearched;

				if (unit->building.tech_id < TechId::Restoration)
					bIsResearched = TechSc->isResearched[unit->player][unit->building.tech_id];
				else
					bIsResearched = TechBw->isResearched[unit->player][unit->building.tech_id - TechId::Restoration];

				if (bIsResearched == 0 &&
					!(*bw_cheat_state & CheatFlags::OperationCwal)
					)
					bStopThere = true;

			}

			//54923
			if (!bStopThere)
			{

				helper::play_research_complete_sound();

				if (unit->building.tech_id < TechId::Restoration)
					TechSc->isResearched[unit->player][unit->building.tech_id] = 1;
				else if (*bw_is_brood_war)
					TechBw->isResearched[unit->player][unit->building.tech_id - TechId::Restoration] = 1;

				scbw::refresh_console();

			}

		}

		if (!bStopThere)
		{

			Image* current_image;

			current_image = unit->sprite->images.head;

			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WorkingToIdle);
				current_image = current_image->link.next;
			}

			//code I could not understand enough to convert
			__asm {

				PUSHAD

				MOV ESI, unit
				MOVZX ECX, byte ptr[ESI + 0xC8] //unit->building.tech_id
				MOVZX EAX, byte ptr[ESI + 0x4C] //unit->player
				MOV EDX, ECX
				SHR EDX, 0x3
				AND ECX, 0x80000007
				LEA EAX, [EAX + EAX * 0x2]
				LEA EAX, [EDX + EAX * 0x2 + 0x0058F230] //EAX = &(TechBw->isResearched[unit->player][unit->building.tech_id CONVERTED?])
				JNS lbl549C7

				DEC ECX
				OR ECX, -0x8
				INC ECX

				lbl549C7 :
				MOV DL, BL
					SHL DL, CL
					MOV CL, [EAX]
					XOR EDI, EDI
					NOT DL
					AND CL, DL
					MOV[EAX], CL

					POPAD

			}

			unit->building.tech_id = TechId::None;

			if (unit->unit_ai != nullptr)
			{

				unit->user_action_flags |= 1;

				if (unit->main_order_id != OrderId::Die)
				{

					bool bStopLoop = (unit->order_queue_tail == nullptr);

					while (!bStopLoop)
					{
						if (!orders_dat::CanBeInterrupted[unit->order_queue_tail->order_id] &&
							unit->order_queue_tail->order_id != OrderId::ComputerAI
							)
							bStopLoop = true;
						else
							helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
					}

					unit->perform_another_order(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);

				}

				prepare_for_next_order(unit);

			}
			else
			{ //54A47

				if (unit->order_queue_head != nullptr)
				{
					unit->user_action_flags |= 1;
					prepare_for_next_order(unit);
				}
				else
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}
}
