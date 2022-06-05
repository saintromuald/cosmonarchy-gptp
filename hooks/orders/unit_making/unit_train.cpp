#include "unit_train.h"
#include <SCBW/api.h>
#include "hooks/cosmonarchy_finder.h"
#include "hooks/orders/spells/hallucination_spell.h"
#include "hooks/orders/building_making/building_modifier.h"
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
		return unit->build_queue[(slot + unit->build_queue_slot) % 5] < UnitId::None;
	}

	void secondaryOrd_TrainFighter(Unit* unit) {
		if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
			if (unit->secondary_order_state == 0 || unit->secondary_order_state == 1) {
				if (unit->build_queue[unit->build_queue_slot] == UnitId::None) {
					unit->secondary_order_state = 3;
					unit->current_build_unit = 0;
				} else {
					Unit* built_unit = 0;
					if (unit->secondary_order_state == 0) {
						built_unit = helper::attemptTrainHatchUnit(unit, unit->build_queue[unit->build_queue_slot], 1);
					} else {
						built_unit = helper::attemptTrainHatchUnit(unit, unit->build_queue[unit->build_queue_slot], 0);
					}
					unit->current_build_unit = built_unit;
					if (!built_unit) {
						unit->secondary_order_state = 1;
					} else {
						built_unit->hangar_unit.parent = unit;
						unit->secondary_order_state = 2;
					}
				}
			} else if (unit->secondary_order_state == 2) {
				Unit* built_unit = unit->current_build_unit;
				if (built_unit) {
					u32 hpGain = helper::getHPGainForRepair(built_unit);
					helper::progress_build(built_unit, hpGain, FALSE);
					if (built_unit->status & UnitStatus::Completed) {
						helper::add_hangar_unit(unit, built_unit);
						if (unit->status & UnitStatus::IsHallucination) {
							apply_hallucination(built_unit, unit);
						}
						unit->build_queue[unit->build_queue_slot] = UnitId::None;
						unit->build_queue_slot++;

						if (unit->build_queue_slot >= 5)
						{
							unit->build_queue_slot = 0;
						}
						scbw::refresh_console();
						unit->current_build_unit = 0;
						unit->secondary_order_state = 0;
					}
				} else {
					scbw::refresh_console();
					unit->current_build_unit = 0;
					unit->secondary_order_state = 0;
				}
			} else if (unit->secondary_order_state == 3) {
				if (unit->id == UnitId::ProtossAccantor) {
					unit->secondary_order_state = 4;
				} else {
					Unit* in_hangar_child = unit->unit_with_hangar.in_hangar_child;
					if (in_hangar_child) {
						s32 hpInterceptor = units_dat::MaxHitPoints[UnitId::ProtossSteward];
						while (in_hangar_child && in_hangar_child->hitpoints >= hpInterceptor) {
							in_hangar_child = in_hangar_child->hangar_unit.hangar_link.next;
						}
						if (in_hangar_child) {
							in_hangar_child->setHp(in_hangar_child->hitpoints + 128);
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
			if (unit->secondary_order_state <= 1)
			{
				if (unit->build_queue[unit->build_queue_slot] == UnitId::None)
				{
					unit->set_secondary_order(OrderId::Nothing2);
					unit->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle, true);
				}
				else
				{
					Unit* builtUnit;
					if (unit->secondary_order_state == 0)
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->build_queue[unit->build_queue_slot], 1);
					}
					else
					{
						builtUnit = helper::attemptTrainHatchUnit(unit, unit->build_queue[unit->build_queue_slot], 0);
					}
					unit->current_build_unit = builtUnit;
					if (builtUnit == nullptr)
					{
						unit->secondary_order_state = 1;
					}
					else
					{
						scbw::play_work_animation(unit);
					}
				}
			}
			else if (unit->secondary_order_state == 2)
			{
				Unit* built_unit = unit->current_build_unit;
				if (built_unit != nullptr)
				{
					UnitExt* builtUnitExt = globals.get_unit_ext(built_unit);
					bool extraBuild = false;
					BuildingModifier modifier = get_building_modifier(unit, 0, false);
					u32 hpGain = helper::getHPGainForRepair(built_unit) * modifier.multiplier;
					if (modifier.decrease) {
						if (built_unit->remaining_build_time > modifier.decrease) {
							built_unit->remaining_build_time -= modifier.decrease;
						} else {
							built_unit->remaining_build_time = 0;
						}
					}
					if (extraBuild || helper::progress_build(built_unit, hpGain, TRUE))
					{
						if (built_unit->status & UnitStatus::Completed)
						{
							u32 count = 1;
							Unit* builtUnits[2] = {built_unit, nullptr};
							if (units_dat::BaseProperty[built_unit->id] & UnitProperty::TwoUnitsIn1Egg) {
								count = 2;
								builtUnits[1] = scbw::create_unit_in_game(built_unit->id, built_unit->player, built_unit->position.x - 32, built_unit->position.y, unit, true);
							}

							for (u32 i = 0; i < count; i++)
							{
								Unit* newUnit = builtUnits[i];
								helper::ai_training_unit(newUnit, unit);
								if (built_unit->id == UnitId::TerranNuclearMissile)
								{
									helper::hide_and_disable_unit(newUnit);
								}
								else
								{
									helper::order_new_unit_to_rally(newUnit, unit);
								}
							}
							helper::function_00432430(unit, unit->build_queue_slot);
							unit->build_queue[unit->build_queue_slot] = UnitId::None;
							unit->build_queue_slot = (unit->build_queue_slot + 1) % 5;
							jump_to_685D8 = true;
						}
					}
					else // unit was canceled due to lack of space around the building
					{
						u32 queueSlot = unit->build_queue_slot % 5;
						if (unit->build_queue[queueSlot] == UnitId::None)
						{
							jump_to_685D8 = true;
						}
						else
						{
							u8 counter = 4;
							if (unit->current_build_unit != nullptr)
							{
								helper::ai_cancel_structure(unit->current_build_unit);
							}
							else if (!units_dat::BaseProperty[unit->build_queue[unit->build_queue_slot % 5]] & UnitProperty::Building)
							{
								helper::refund_unit_train_cost(unit->build_queue[unit->build_queue_slot % 5], unit->player);
							}
							do
							{
								counter--;
								unit->build_queue[unit->build_queue_slot] = unit->build_queue[(unit->build_queue_slot + 1) % 5];
								if (unit->unit_ai != nullptr && *(u8*)((u32)unit->unit_ai + 8) == 3)
								{
									*(u8*)((u32)unit->unit_ai + unit->build_queue_slot + 9) = *(u8*)((u32)unit->unit_ai + (unit->build_queue_slot + 1) % 5 + 9);
									*(u32*)((u32)unit->unit_ai + unit->build_queue_slot * 4 + 0x18) = *(u32*)((u32)unit->unit_ai + ((unit->build_queue_slot + 1) % 5) * 4 + 0x18);
								}
							} while (counter != 0);
							unit->build_queue[(unit->build_queue_slot + 1) % 5] = UnitId::None;
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
					if (built_unit && built_unit->id == UnitId::ProtossSolarion)
					{
						scbw::create_hangar_units(built_unit, 20, UnitId::ProtossSteward);
						
						// s32 interceptors = 12;
						// u8 buf[0xc] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0}; // 12
						// buf[0] = 0x99; // new general command
						// buf[1] = NewGameCommand::CarrierPrebuild;
						// buf[2] = interceptors;
						// buf[3] = *ACTIVE_PLAYER_ID;
						// memcpy(&buf[4], &built_unit, 4);
						// helper::queue_game_command(buf, 0xc);
					}
					scbw::refresh_console();
					unit->current_build_unit = nullptr;
					unit->secondary_order_state = 0;
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
		return unit->build_queue[unit->build_queue_slot % 5] != UnitId::None;
	}
}