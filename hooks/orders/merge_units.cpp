#include "merge_units.h"
#include <SCBW/api.h>
#include <algorithm>

#ifndef MAXINT32
#define MAXINT32    (0xFFFFFFFF >> 1)
#endif
#ifndef MAXINT16
#define MAXINT16    (0xFFFF >> 1)
#endif

namespace hooks
{
	//equivalent to 0x004BFA80, but not identical
	Unit* templarMergePartner(Unit* unit, Unit** units_list, u32 units_list_length)
	{
		Unit* nearest_unit = nullptr;	//was ebp-04
		u32 best_distance = MAXINT32;
		for (u32 i = 0; i < units_list_length; i++)
		{

			if (units_list[i] != nullptr)
			{

				u32 x_distance, y_distance;

				x_distance =
					std::max(unit->sprite->position.x, units_list[i]->sprite->position.x) -
					std::min(unit->sprite->position.x, units_list[i]->sprite->position.x);

				x_distance *= x_distance;

				y_distance =
					std::max(unit->sprite->position.y, units_list[i]->sprite->position.y) -
					std::min(unit->sprite->position.y, units_list[i]->sprite->position.y);

				y_distance *= y_distance;

				if ((x_distance + y_distance) < best_distance)
				{

					Unit* new_nearest_unit = units_list[i];
					units_list[i] = nearest_unit;
					nearest_unit = new_nearest_unit;
					best_distance = x_distance + y_distance;

				}

			} //if(units_list[i] != nullptr)

		} //for(s32 i = units_list_length; i >= 0; i--)

		return nearest_unit;

	}

	;

	void orders_CompletingArchonSummon(Unit* unit)
	{

		u32* const bCanUpdateCurrentButtonSet = (u32*)0x0068C1B0;
		u8* const bCanUpdateSelectedUnitPortrait = (u8*)0x0068AC74;
		u8* const bCanUpdateStatDataDialog = (u8*)0x0068C1F8;
		BinDlg** const someDialogUnknown = (BinDlg**)0x0068C1E8;
		BinDlg** const someDialogUnknownUser = (BinDlg**)0x0068C1EC;

		bool stopFunction = false;

		if (unit->mainOrderState == 0)
		{

			//93BB2
			if (helper::advanceRemainingBuildTime(unit))
			{

				unit->sprite->flags &= ~SpriteFlags::IscriptCode;	//unset that flag

				if (unit->sprite->images.head != nullptr)
				{

					Image* current_image = unit->sprite->images.head;

					do
					{
						current_image->playIscriptAnim(IscriptAnimation::Init);
						current_image = current_image->link.next;
					}
					while (current_image != nullptr);

				}

				unit->mainOrderState = 1;

			} //if(advanceRemainingBuildTime_Sub466940(unit)

		}
		else
			if (unit->mainOrderState == 1 && (unit->orderSignal & 4))
			{
				//93B37

				unit->orderSignal -= 4;
				unit->status |= UnitStatus::IsBuilding;

				//pathing related
				helper::function_0046A820(unit);

				helper::changeUnitButtonSet(unit, unit->id);

				*bCanUpdateCurrentButtonSet = 1;
				*bCanUpdateSelectedUnitPortrait = 1;
				*bCanUpdateStatDataDialog = 1;
				*someDialogUnknown = nullptr;
				*someDialogUnknownUser = nullptr;

				helper::playMorphingCompleteSound(unit);
				unit->order(units_dat::HumanIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, 0);
				helper::force_order_done(unit);
				if (unit->id == UnitId::ProtossAugur)
				{
					unit->status |= UnitStatus::Completed;

				}
			}

	}

	;

	void orders_WarpingDarkArchon(Unit* unit)
	{

		Unit* targetUnit = unit->orderTarget.unit;

		if (
			targetUnit == nullptr ||
			unit->playerId != targetUnit->playerId ||
			targetUnit->mainOrderId != OrderId::WarpingDarkArchon ||
			targetUnit->orderTarget.unit != unit
			)
		{	//conditions for merge not ok

	 //94838
			helper::set_resource_target(unit);

			if (
				unit->moveTarget.pt.x != unit->nextTargetWaypoint.x ||
				unit->moveTarget.pt.y != unit->nextTargetWaypoint.y
				)
			{
				//94857
				unit->nextTargetWaypoint.x = unit->moveTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->moveTarget.pt.y;
			}

			//9485F

			if (unit->orderQueueHead != nullptr)
			{

				//94866

				unit->userActionFlags |= 1;	//set flag
				prepareForNextOrder(unit);

			}
			else
			{

				//9487D
				if (unit->pAI != nullptr)
					unit->orderComputer(OrderId::ComputerAI);
				else
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);

			}

		}
		else
		{

			//946BE
			u32 distanceBetweenUnits;

			if (unit->status & UnitStatus::IsNormal)
			{
				if (unit->isTargetWithinMinRange(targetUnit, (2 * unit->current_speed2) / 256))
				{
					unit->status &= ~UnitStatus::IsNormal; //unset the flag
					helper::order_immediate(unit, OrderId::ResetCollision1);
				}
			}

			//946F3
			distanceBetweenUnits =
				scbw::getDistanceFast(
					unit->sprite->position.x,
					unit->sprite->position.y,
					targetUnit->sprite->position.x,
					targetUnit->sprite->position.y
				);

			if (distanceBetweenUnits > 20)
			{

				//94724

												//unit unmovable			
				if (unit->mainOrderState != 0 && unit->getMovableState() == 2)
				{
					helper::make_to_hold_position(unit);
					unit->orderToIdle();
				}
				else
				{

					//94746

					unit->mainOrderState = 1;

					if (distanceBetweenUnits > 19)
						helper::delayedSetMoveTarget_xy_Sub4EB960(
							unit,
							targetUnit->sprite->position.x,
							targetUnit->sprite->position.y
						);
					else
					{

						//94765 (will never happen?And that would be good because
						//I'm not sure what that stuff did and if I converted it
						//properly)

						s32 x, y;

						if (unit->sprite->position.x > MAXINT16)
							x = (0xFFFF0000) + unit->sprite->position.x;
						else
							x = unit->sprite->position.x;

						if (targetUnit->sprite->position.x > MAXINT16)
							x += (0xFFFF0000) + targetUnit->sprite->position.x;
						else
							x += targetUnit->sprite->position.x;

						if (x < 0)
							x -= 0xFFFFFFFF;

						if (unit->sprite->position.y > MAXINT16)
							y = (0xFFFF0000) + unit->sprite->position.y;
						else
							y = unit->sprite->position.y;

						if (targetUnit->sprite->position.y > MAXINT16)
							y += (0xFFFF0000) + targetUnit->sprite->position.y;
						else
							y += targetUnit->sprite->position.y;

						if (y < 0)
							y -= 0xFFFFFFFF;

						x /= 2; y /= 2;

						if ((u16)x > MAXINT16)
							x = 0xFFFF0000 + (u16)x;
						else
							x = (u16)x;

						if ((u16)y > MAXINT16)
							y = 0xFFFF0000 + (u16)y;
						else
							y = (u16)y;

						helper::delayedSetMoveTarget_xy_Sub4EB960(unit, x, y);

					} //if(distanceBetweenUnits <= 19)

				} //if(unit->mainOrderState == 0 || unit->getMovableState() != 2)

			} //if(distanceBetweenUnits > 20)
			else
			{

				//9479C:
				helper::mergeStatusIntoUnit_Sub493180(unit, targetUnit);
				helper::replaceUnitWithType(unit, UnitId::ProtossMindTyrant);

				//update stuff related to the beginning of merge process I think
				helper::animDecloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);

				unit->sprite->flags &= ~SpriteFlags::IscriptCode; //disable the flag if set

				if (unit->sprite->images.head != nullptr)
				{

					Image* current_image = unit->sprite->images.head;

					do
					{
						current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					while (current_image != nullptr);

				}

				//947E0:
				helper::changeUnitButtonSet(unit, UnitId::None);
				scbw::refreshConsole();

				unit->killCount += targetUnit->killCount;
				unit->energy = 12800;	//50 energy ingame

				//remove the 2nd unit of the merge process
				targetUnit->userActionFlags |= 4;
				targetUnit->remove();

				unit->status &= ~UnitStatus::IsBuilding; //disable the flag if set
				helper::function_0046A820(unit); ////pathing related

				unit->orderComputer(OrderId::CompletingArchonSummon);

			} //if(distanceBetweenUnits <= 20)

		} //conditions for merge are ok

	} //void orders_WarpingDarkArchon(Unit* unit)

	;

	void orders_WarpingArchon(Unit* unit)
	{

		Unit* targetUnit = unit->orderTarget.unit;
		if (
			targetUnit == nullptr ||
			unit->playerId != targetUnit->playerId ||
			targetUnit->mainOrderId != OrderId::WarpingArchon ||
			targetUnit->orderTarget.unit != unit
			)
		{	//conditions for merge not ok

	 //94A3F
			helper::set_resource_target(unit);

			if (
				unit->moveTarget.pt.x != unit->nextTargetWaypoint.x ||
				unit->moveTarget.pt.y != unit->nextTargetWaypoint.y
				)
			{
				//94A5E
				unit->nextTargetWaypoint.x = unit->moveTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->moveTarget.pt.y;
			}

			//94A66

			if (unit->orderQueueHead != nullptr)
			{

				//94866
				unit->userActionFlags |= 1;	//set flag
				prepareForNextOrder(unit);

			}
			else
			{

				//94A84
				if (unit->pAI != nullptr)
					unit->orderComputer(OrderId::ComputerAI);
				else
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);

			}

		}
		else
		{

			//948EA
			u32 distanceBetweenUnits;

			if (unit->status & UnitStatus::IsNormal)
			{
				if (unit->isTargetWithinMinRange(targetUnit, (2 * unit->current_speed2) / 256))
				{
					unit->status &= ~UnitStatus::IsNormal; //unset the flag
					helper::order_immediate(unit, OrderId::ResetCollision1);
				}
			}

			//94913
			distanceBetweenUnits =
				scbw::getDistanceFast(
					unit->sprite->position.x,
					unit->sprite->position.y,
					targetUnit->sprite->position.x,
					targetUnit->sprite->position.y
				);

			if (distanceBetweenUnits > 2)
			{

				//94944

												//unit unmovable
				if (unit->mainOrderState != 0 && unit->getMovableState() == 2)
				{
					helper::make_to_hold_position(unit);
					unit->orderToIdle();
				}
				else
				{

					//94966

					unit->mainOrderState = 1;

					if (distanceBetweenUnits > 10)
						helper::delayedSetMoveTarget_xy_Sub4EB960(
							unit,
							targetUnit->sprite->position.x,
							targetUnit->sprite->position.y
						);

					else
					{

						//94985 (will never happen?And that would be good because
						//I'm not sure what that stuff did and if I converted it
						//properly)

						s32 x, y;

						if (unit->sprite->position.x > MAXINT16)
							x = (0xFFFF0000) + unit->sprite->position.x;
						else
							x = unit->sprite->position.x;

						if (targetUnit->sprite->position.x > MAXINT16)
							x += (0xFFFF0000) + targetUnit->sprite->position.x;
						else
							x += targetUnit->sprite->position.x;

						if (x < 0)
							x -= 0xFFFFFFFF;

						if (unit->sprite->position.y > MAXINT16)
							y = (0xFFFF0000) + unit->sprite->position.y;
						else
							y = unit->sprite->position.y;

						if (targetUnit->sprite->position.y > MAXINT16)
							y += (0xFFFF0000) + targetUnit->sprite->position.y;
						else
							y += targetUnit->sprite->position.y;

						if (y < 0)
							y -= 0xFFFFFFFF;

						x /= 2; y /= 2;

						if ((u16)x > MAXINT16)
							x = 0xFFFF0000 + (u16)x;
						else
							x = (u16)x;

						if ((u16)y > MAXINT16)
							y = 0xFFFF0000 + (u16)y;
						else
							y = (u16)y;

						helper::delayedSetMoveTarget_xy_Sub4EB960(unit, x, y);

					} //if(distanceBetweenUnits <= 19)

				} //if(unit->mainOrderState == 0 || unit->getMovableState() != 2)

			} //if(distanceBetweenUnits > 20)
			else
			{

				//949BC:
				helper::mergeStatusIntoUnit_Sub493180(unit, targetUnit);
				helper::replaceUnitWithType(unit, UnitId::ProtossArchon);

				unit->sprite->flags &= ~SpriteFlags::IscriptCode; //disable the flag if set

				if (unit->sprite->images.head != nullptr)
				{

					Image* current_image = unit->sprite->images.head;

					do
					{
						current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					while (current_image != nullptr);

				}

				//949F0:
				helper::changeUnitButtonSet(unit, UnitId::None);
				scbw::refreshConsole();

				unit->killCount += targetUnit->killCount;

				//remove the 2nd unit of the merge process
				targetUnit->userActionFlags |= 4;
				targetUnit->remove();

				unit->status &= ~UnitStatus::IsBuilding; //disable the flag if set
				helper::function_0046A820(unit); ////pathing related

				unit->orderComputer(OrderId::CompletingArchonSummon);
			}
		}
	}

}