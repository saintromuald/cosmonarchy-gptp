#include "research_upgrade_orders.h"
#include <SCBW/api.h>

namespace hooks
{

	void orders_Upgrade(Unit* unit)
	{

		bool bStopThere = false;
		Image* current_image;

		if (unit->status & UnitStatus::GroundedBuilding &&
			unit->building.upgradeType != UpgradeId::None
			)
		{

			unit->building.upgradeResearchTime--;

			if (unit->building.upgradeResearchTime != 0xFFFF)
			{

				u8 upgradeLevel;

				if (unit->building.upgradeType >= UpgradeId::PrismaticShield)
					upgradeLevel = UpgradesBw->currentLevel[unit->playerId][unit->building.upgradeType - UpgradeId::PrismaticShield];
				else
					upgradeLevel = UpgradesSc->currentLevel[unit->playerId][unit->building.upgradeType];

				if (upgradeLevel < unit->building.upgradeLevel &&
					!(*CHEAT_STATE & CheatFlags::OperationCwal))
					bStopThere = true;

			}

			//5471D
			if (!bStopThere)
			{

				u8 upgradeLevel;

				helper::playUpgradeCompleteSound();

				if (unit->building.upgradeType >= UpgradeId::PrismaticShield)
					upgradeLevel = UpgradesBw->currentLevel[unit->playerId][unit->building.upgradeType - UpgradeId::PrismaticShield];
				else
					upgradeLevel = UpgradesSc->currentLevel[unit->playerId][unit->building.upgradeType];

				if (upgradeLevel < unit->building.upgradeLevel)
				{

					if (helper::isUnitUpgradeAvailable(unit))
					{
						helper::setUpgradeLevel(unit->playerId, unit->building.upgradeType, unit->building.upgradeLevel);
						helper::ApplySpeedUpgradeFromUpgradeType(unit, unit->building.upgradeType);
					}

				}

				//5477F
				scbw::refreshConsole();

			}

		}

		//547A7
		if (!bStopThere)
		{

			current_image = unit->sprite->images.head;

			while (current_image != nullptr)
			{
				current_image->playIscriptAnim(IscriptAnimation::WorkingToIdle);
				current_image = current_image->link.next;
			}

			//code I could not understand enough to convert
			__asm {

				PUSHAD

				MOV ESI, unit
				MOVZX ECX, byte ptr[ESI + 0xC9]	//unit->building.upgradeType
				MOVZX EAX, byte ptr[ESI + 0x4C]	//unit->playerId
				MOV EDX, ECX
				SHR EDX, 0x3
				AND ECX, 0x80000007
				LEA EAX, [EDX + EAX * 0x8 + 0x0058F3E0] //EAX = &(UpgradesBw->currentLevel[unit->playerId][unit->building.upgradeType CONVERTED?])
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

			unit->building.upgradeType = UpgradeId::None;
			unit->building.upgradeLevel = 0;

			if (unit->pAI != nullptr)
			{

				unit->userActionFlags |= 1;

				if (unit->mainOrderId != OrderId::Die)
				{

					bool bStopLoop = (unit->orderQueueTail == nullptr);

					while (!bStopLoop)
					{

						if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
							unit->orderQueueTail->orderId != OrderId::ComputerAI
							)
							bStopLoop = true;
						else
							helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);

					}

					unit->performAnotherOrder(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);

				}

				prepareForNextOrder(unit);

			}
			else
			{ //54875

				if (unit->orderQueueHead != nullptr)
				{
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				}
				else //54896
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);

			}

		}

	}

	;

	void orders_ResearchTech(Unit* unit)
	{

		bool bStopThere = false;

		if (unit->building.techType != TechId::None)
		{

			unit->building.upgradeResearchTime--;

			if (unit->building.upgradeResearchTime != 0xFFFF)
			{

				u8 bIsResearched;

				if (unit->building.techType < TechId::Restoration)
					bIsResearched = TechSc->isResearched[unit->playerId][unit->building.techType];
				else
					bIsResearched = TechBw->isResearched[unit->playerId][unit->building.techType - TechId::Restoration];

				if (bIsResearched == 0 &&
					!(*CHEAT_STATE & CheatFlags::OperationCwal)
					)
					bStopThere = true;

			}

			//54923
			if (!bStopThere)
			{

				helper::playResearchCompleteSound();

				if (unit->building.techType < TechId::Restoration)
					TechSc->isResearched[unit->playerId][unit->building.techType] = 1;
				else if (*IS_BROOD_WAR)
					TechBw->isResearched[unit->playerId][unit->building.techType - TechId::Restoration] = 1;

				scbw::refreshConsole();

			}

		}

		if (!bStopThere)
		{

			Image* current_image;

			current_image = unit->sprite->images.head;

			while (current_image != nullptr)
			{
				current_image->playIscriptAnim(IscriptAnimation::WorkingToIdle);
				current_image = current_image->link.next;
			}

			//code I could not understand enough to convert
			__asm {

				PUSHAD

				MOV ESI, unit
				MOVZX ECX, byte ptr[ESI + 0xC8] //unit->building.techType
				MOVZX EAX, byte ptr[ESI + 0x4C] //unit->playerId
				MOV EDX, ECX
				SHR EDX, 0x3
				AND ECX, 0x80000007
				LEA EAX, [EAX + EAX * 0x2]
				LEA EAX, [EDX + EAX * 0x2 + 0x0058F230] //EAX = &(TechBw->isResearched[unit->playerId][unit->building.techType CONVERTED?])
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

			unit->building.techType = TechId::None;

			if (unit->pAI != nullptr)
			{

				unit->userActionFlags |= 1;

				if (unit->mainOrderId != OrderId::Die)
				{

					bool bStopLoop = (unit->orderQueueTail == nullptr);

					while (!bStopLoop)
					{
						if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
							unit->orderQueueTail->orderId != OrderId::ComputerAI
							)
							bStopLoop = true;
						else
							helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);
					}

					unit->performAnotherOrder(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);

				}

				prepareForNextOrder(unit);

			}
			else
			{ //54A47

				if (unit->orderQueueHead != nullptr)
				{
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				}
				else
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}
}
