#include "unit_train.h"
#include <SCBW/api.h>
#include "hooks/cosmonarchy_finder.h"
#include "globals.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

namespace hooks
{
	s32 is_queue_slot_active(Unit* unit, u32 slot)
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
			else if (unit->secondaryOrderState == 2)
			{
				Unit* builtUnit = unit->currentBuildUnit;
				u32 hpGain;
				if (builtUnit != nullptr)
				{
					hpGain = helper::getHPGainForRepair(builtUnit);
					helper::progress_build(builtUnit, hpGain, FALSE);
					if (builtUnit->status & UnitStatus::Completed)
					{
						helper::addHangarUnit(unit, builtUnit);
						unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
						unit->buildQueueSlot++;

						if (unit->buildQueueSlot >= 5)
						{
							unit->buildQueueSlot = 0;
						}
						scbw::refresh_console();
						unit->currentBuildUnit = nullptr;
						unit->secondaryOrderState = 0;
					}
				}
				else
				{
					scbw::refresh_console();
					unit->currentBuildUnit = nullptr;
					unit->secondaryOrderState = 0;
				}
			}
			else if (unit->secondaryOrderState == 3)
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
		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (!(unit->status & UnitStatus::Frozen)
			&& !(unit->is_frozen()))
		{
			bool jump_to_685D8 = false;
			if (unit->secondaryOrderState <= 1)
			{
				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::None)
				{
					unit->set_secondary_order(OrderId::Nothing2);
					unit->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle, true);
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
						scbw::play_work_animation(unit);
					}
				}
			}
			else if (unit->secondaryOrderState == 2)
			{
				Unit* builtUnit = unit->currentBuildUnit;
				if (builtUnit != nullptr)
				{
					UnitExt* builtUnitExt = globals.get_unit_ext(builtUnit);
					u32 hpGain = helper::getHPGainForRepair(builtUnit);
					bool extraBuild = false;
					if (builtUnitExt->values[ValueCrucible] == 0)
					{
						for (Unit* unit : unitExt->unit_vectors[UnitVectorCrucibles])
						{
							builtUnitExt->values[ValueCrucible] = 18; // 6 is ~115%, 18 should be ~105%
							helper::progress_build(builtUnit, hpGain, TRUE);
							if (builtUnit->status & UnitStatus::Completed)
							{
								extraBuild = true;
							}
						}
					}
					else
					{
						builtUnitExt->values[ValueCrucible]--;
					}
					if (extraBuild || helper::progress_build(builtUnit, hpGain, TRUE))
					{
						if (builtUnit->status & UnitStatus::Completed)
						{
							u32 count = 1;
							Unit* builtUnits[2] = {builtUnit, nullptr};
							if (units_dat::BaseProperty[builtUnit->id] & UnitProperty::TwoUnitsIn1Egg)
							{
								count = 2;
								builtUnits[1] = scbw::create_unit_ingame(builtUnit->id, builtUnit->player, builtUnit->position.x - 32, builtUnit->position.y, unit, true);
							}

							for (u32 i = 0; i < count; i++)
							{
								Unit* newUnit = builtUnits[i];
								helper::ai_training_unit(newUnit, unit);
								if (builtUnit->id == UnitId::TerranNuclearMissile)
								{
									helper::hide_and_disable_unit(newUnit);
								}
								else
								{
									helper::order_new_unit_to_rally(newUnit, unit);
								}
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
								helper::ai_cancel_structure(unit->currentBuildUnit);
							}
							else if (!units_dat::BaseProperty[unit->buildQueue[unit->buildQueueSlot % 5]] & UnitProperty::Building)
							{
								helper::refund_unit_train_cost(unit->buildQueue[unit->buildQueueSlot % 5], unit->player);
							}
							do
							{
								counter--;
								unit->buildQueue[unit->buildQueueSlot] = unit->buildQueue[(unit->buildQueueSlot + 1) % 5];
								if (unit->unit_ai != nullptr && *(u8*)((u32)unit->unit_ai + 8) == 3)
								{
									*(u8*)((u32)unit->unit_ai + unit->buildQueueSlot + 9) = *(u8*)((u32)unit->unit_ai + (unit->buildQueueSlot + 1) % 5 + 9);
									*(u32*)((u32)unit->unit_ai + unit->buildQueueSlot * 4 + 0x18) = *(u32*)((u32)unit->unit_ai + ((unit->buildQueueSlot + 1) % 5) * 4 + 0x18);
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
						u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0}; // 12
						buf[0] = 0x99; // new general command
						buf[1] = NewGameCommand::CarrierPrebuild;
						buf[2] = interceptors;
						buf[3] = *ACTIVE_PLAYER_ID;
						memcpy(&buf[4], &builtUnit, 4);
						helper::queue_game_command(buf, 0xc);
					}
					scbw::refresh_console();
					unit->currentBuildUnit = nullptr;
					unit->secondaryOrderState = 0;
				}
			}
		}
	}

	char getMaxHangerSpace(Unit* unit) // 0x004653D0
	{
		u16 id;
		s32 hangarSpace;

		id = unit->id;
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

	b32 _stdcall ButtonIsTraining(Unit* unit)
	{
		return unit->buildQueue[unit->buildQueueSlot % 5] != UnitId::None;
	}
}