#include "unit_train.h"
#include <SCBW/api.h>
#include "hooks/cosmonarchyFinder.h"
#include "globals.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

namespace hooks
{
	s32 isQueueSlotActive(Unit* unit, u32 slot)
	{
		return unit->buildQueue[(slot + unit->buildQueueSlot) % 5] < UnitId::None;
	}

	void secondaryOrd_TrainFighter(Unit* unit)
	{
		if (unit->id == UnitId::ProtossSolarion
			|| unit->id == UnitId::ProtossAccantor)
		{
			if (unit->secondaryOrderState == 0 || unit->secondaryOrderState == 1)
			{
				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::None)
				{
					unit->secondaryOrderState = 3;
					unit->currentBuildUnit = nullptr;
				}
				else
				{
					Unit* builtUnit;
					if (unit->secondaryOrderState == 0)
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 1);
					}
					else
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 0);
					}
					unit->currentBuildUnit = builtUnit;
					if (builtUnit == nullptr)
					{
						unit->secondaryOrderState = 1;
					}
					else
					{
						builtUnit->interceptor.parent = unit;
						unit->secondaryOrderState = 2;
					}
				}
			}
			else
				if (unit->secondaryOrderState == 2)
				{
					Unit* builtUnit = unit->currentBuildUnit;
					u32 hpGain;
					if (builtUnit != nullptr)
					{
						hpGain = helper::getHPGainForRepair(builtUnit);
						helper::buildingAddon(builtUnit, hpGain, FALSE);
						if (builtUnit->status & UnitStatus::Completed)
						{
							helper::addHangarUnit(unit, builtUnit);
							unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
							unit->buildQueueSlot++;

							if (unit->buildQueueSlot >= 5)
							{
								unit->buildQueueSlot = 0;
							}
							scbw::refreshConsole();
							unit->currentBuildUnit = nullptr;
							unit->secondaryOrderState = 0;
						}
					}
					else
					{
						scbw::refreshConsole();
						unit->currentBuildUnit = nullptr;
						unit->secondaryOrderState = 0;
					}
				}
				else
					if (unit->secondaryOrderState == 3)
					{
						if (unit->id == UnitId::ProtossAccantor)
						{
							unit->secondaryOrderState = 4;
						}
						else
						{
							Unit* inHangarChild = unit->carrier.inHangarChild;
							if (inHangarChild != nullptr)
							{
								s32 hpInterceptor = units_dat::MaxHitPoints[UnitId::ProtossSteward];
								while (inHangarChild != nullptr && inHangarChild->hitPoints >= hpInterceptor)
								{
									inHangarChild = inHangarChild->interceptor.hangar_link.next;
								}
								if (inHangarChild != nullptr)
								{
									inHangarChild->setHp(inHangarChild->hitPoints + 128);
								}
							}
						}
					}
		}
	}

	// generic unit training function
	void orderTrain(Unit* unit)
	{
		if (!(unit->status & UnitStatus::DoodadStatesThing)
			&& !(unit->isFrozen()))
		{
			bool jump_to_685D8 = false;
			if (unit->secondaryOrderState <= 1)
			{
				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::None)
				{
					unit->setSecondaryOrder(OrderId::Nothing2);
					unit->sprite->playIscriptAnim(IscriptAnimation::WorkingToIdle, true);
				}
				else
				{
					Unit* builtUnit;
					if (unit->secondaryOrderState == 0)
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 1);
					}
					else
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 0);
					}
					unit->currentBuildUnit = builtUnit;
					if (builtUnit == nullptr)
					{
						unit->secondaryOrderState = 1;
					}
					else
					{
						scbw::playWorkAnimation(unit);
					}
				}
			}
			else
				if (unit->secondaryOrderState == 2)
				{
					Unit* builtUnit = unit->currentBuildUnit;
					if (builtUnit != nullptr)
					{
						UnitExt* builtUnitExt = globals.GetUnitExt(builtUnit);
						u32 hpGain = helper::getHPGainForRepair(builtUnit);
						bool extraBuild = false;
						if (builtUnitExt->values[ValueCrucible] == 0)
						{
							auto r = 256;
							auto n = cosmonarchyFinderMatch(nullptr, Flags::Ally | Flags::Powered | Flags::Completed,
								{UnitId::ProtossCrucible});
							scbw::UnitFinder unitsInSplash(builtUnit->position.x - r, builtUnit->position.y - r,
								builtUnit->position.x + r, builtUnit->position.y + r);
							for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
							{
								auto unit = unitsInSplash.getUnit(i);
								if (n(unit))
								{
									builtUnitExt->values[ValueCrucible] = 5; // 6 is ~100/15
									helper::buildingAddon(builtUnit, hpGain, TRUE);
									if (builtUnit->status & UnitStatus::Completed)
									{
										extraBuild = true;
									}
									break;
								}
							}
						}
						else
						{
							builtUnitExt->values[ValueCrucible]--;
						}
						if (extraBuild || helper::buildingAddon(builtUnit, hpGain, TRUE))
						{
							if (builtUnit->status & UnitStatus::Completed)
							{
								helper::AI_TrainingUnit(builtUnit, unit);
								if (builtUnit->id == UnitId::TerranNuclearMissile)
								{
									helper::hideAndDisableUnit(builtUnit);
								}
								else
								{
									helper::orderNewUnitToRally(builtUnit, unit);
								}
								helper::function_00432430(unit, unit->buildQueueSlot);
								unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
								unit->buildQueueSlot = (unit->buildQueueSlot + 1) % 5;
								jump_to_685D8 = true;
							}
						}
						else // unit was canceled due to lack of space around the building
						{
							u32 queueSlot = unit->buildQueueSlot % 5;
							if (unit->buildQueue[queueSlot] == UnitId::None)
							{
								jump_to_685D8 = true;
							}
							else
							{
								u8 counter = 4;
								if (unit->currentBuildUnit != nullptr)
								{
									helper::AI_CancelStructure(unit->currentBuildUnit);
								}
								else
								{
									if (!units_dat::BaseProperty[unit->buildQueue[unit->buildQueueSlot % 5]] & UnitProperty::Building)
									{
										helper::refundUnitTrainCost(unit->buildQueue[unit->buildQueueSlot % 5], unit->playerId);
									}
								}
								do
								{
									counter--;
									unit->buildQueue[unit->buildQueueSlot] = unit->buildQueue[(unit->buildQueueSlot + 1) % 5];
									if (unit->pAI != nullptr && *(u8*)((u32)unit->pAI + 8) == 3)
									{
										*(u8*)((u32)unit->pAI + unit->buildQueueSlot + 9) = *(u8*)((u32)unit->pAI + (unit->buildQueueSlot + 1) % 5 + 9);
										*(u32*)((u32)unit->pAI + unit->buildQueueSlot * 4 + 0x18) = *(u32*)((u32)unit->pAI + ((unit->buildQueueSlot + 1) % 5) * 4 + 0x18);
									}
								} while (counter != 0);
								unit->buildQueue[(unit->buildQueueSlot + 1) % 5] = UnitId::None;
								jump_to_685D8 = true;
							}
						}
					}
					else
					{
						jump_to_685D8 = true;
					}
					if (jump_to_685D8)
					{
						// Solarion - start with 12 interceptors
						if (builtUnit != nullptr && builtUnit->id == UnitId::ProtossSolarion)
						{
							s32 interceptors = 12;
							u8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0}; // 12
							buf[0] = 0x99; // new general command
							buf[1] = NewGameCommand::CarrierPrebuild;
							buf[2] = interceptors;
							buf[3] = *ACTIVE_PLAYER_ID;
							memcpy(&buf[4], &builtUnit, 4);
							helper::QueueGameCommand(buf, 0xc);
						}
						scbw::refreshConsole();
						unit->currentBuildUnit = nullptr;
						unit->secondaryOrderState = 0;
					}
				}
		}
	}

	char getMaxHangerSpace(Unit* a1)
	{ // 0x004653D0
		u16 id; // ax
		s32 hangarSpace; // eax

		id = a1->id;
		if (id != UnitId::ProtossSolarion)
		{
			if (id != UnitId::ProtossAccantor)
			{
				hangarSpace = 0;
			}
			else
			{
				hangarSpace = 5;
			}
		}
		else
		{
			hangarSpace = 12;
		}
		return hangarSpace;
	}
}