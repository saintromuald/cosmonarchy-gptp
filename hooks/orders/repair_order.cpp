#include "repair_order.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_Repair1(Unit* unit) { // 004673D0
		Unit* target = unit->orderTarget.unit;
		if (!target
		//||	target->hitPoints >= units_dat::MaxHitPoints[target->id]
		||	target->stasisTimer != 0
		||	(unit->unit_ai && target->status & UnitStatus::IsBuilding && target->orderTarget.unit != unit)
		||	target->status & UnitStatus::InTransport
		||  !(target->status & UnitStatus::Completed)) {
			unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);
			unit->orderToIdle();
			if (target && !(target->status & UnitStatus::InTransport)) {
				target->connectedUnit = 0;
			}
			return;
		}

		switch (unit->mainOrderState) {
			case 0: {
				s16 unkRepairInfo = 0;
				u32 repairCostMinerals = 0, repairCostGas = 0;

				helper::getRepairInfo(target, &repairCostMinerals, &repairCostGas, &unkRepairInfo);
				helper::verifyResources(repairCostMinerals, repairCostGas, unit->playerId);

				unit->worker.repairResourceLossTimer = 0;
				unit->mainOrderState = 1; // continue directly into this case
			} // fallthrough
			case 1: {
				if (!(target->status & UnitStatus::Completed)) {
					unit->orderTo(OrderId::ConstructingBuilding, target);
					if (*IS_PLACING_BUILDING) {
						if (!helper::canStillPlaceBuilding()) {
							helper::refreshLayers3And4();
							helper::endBuildingPlacement();
						}
					}
					return;
				}

				if (helper::orderToMoveToTarget(unit, target)) {
					helper::unitOrderMoveToTargetUnit(unit, target);
					unit->mainOrderState = 6;
				}
				return;
			} break;
			case 6: {
				if (!unit->getMovableState()) { // not reached destination
					helper::moveTowards(unit, target); // may cause unit to move or become idle
					return;
				}
				if (!(unit->isTargetWithinMinRange(target, 5))) {
					unit->mainOrderState = 1;
					return;
				}

				helper::setSecondaryWaypoint(unit, target->sprite->position.x, target->sprite->position.y);
				helper::unitOrderMoveToTargetUnit(unit, target);
				unit->mainOrderState = 7;
			} break;
			case 7: {
				if (!(unit->isTargetWithinMinRange(target, 5))) {
					unit->mainOrderState = 1;
					return;
				} else {
					helper::setSecondaryWaypoint(unit, target->sprite->position.x, target->sprite->position.y);
					helper::unitOrderMoveToTargetUnit(unit, target);
					if (!helper::isUnitPositions2Equal(unit)) {
						s32 angle_delta = scbw::getAngle(unit->sprite->position.x, unit->sprite->position.y, unit->nextTargetWaypoint.x, unit->nextTargetWaypoint.y);
						s32 current_angle = unit->currentDirection1 - angle_delta;
						if (current_angle < 0) {
							current_angle += 256;
						}
						if (current_angle > 128) {
							current_angle = 256 - current_angle;
						}
						u8 weapon = unit->getGroundWeapon();
						if (current_angle <= weapons_dat::AttackAngle[weapon]) {
							unit->playIscriptAnim(IscriptAnimation::AlmostBuilt);
							unit->mainOrderState = 8;
						}
					} else {
						unit->playIscriptAnim(IscriptAnimation::AlmostBuilt);
						unit->mainOrderState = 8;
					}
				}
			} break;
			case 8: {
				helper::setSecondaryWaypoint(unit, target->sprite->position.x, target->sprite->position.y);
				helper::unitOrderMoveToTargetUnit(unit, target);
				if (!unit->isTargetWithinMinRange(target, 5) || target->mainOrderId == OrderId::Liftoff) { 
					unit->playIscriptAnim(IscriptAnimation::GndAttkToIdle);
					unit->mainOrderState = 1;
				}
			} break;
		}

		bool do_repair = false;
		if (unit->worker.repairResourceLossTimer == 0) {
			unit->worker.repairResourceLossTimer--;
			do_repair = helper::ord_repair_subtract(unit, target);
			if (!do_repair)
			{
				unit->playIscriptAnim(IscriptAnimation::WalkingToIdle);
				unit->mainOrderState = 1; //unit->orderToIdle();
				return;
			}
		} else {
			unit->worker.repairResourceLossTimer--;
			do_repair = true;
		}
		if (do_repair) {
			u32 hpGain = target->buildRepairHpGain;
			if (*CHEAT_STATE & CheatFlags::OperationCwal) {
				hpGain *= 16;
			}
			// Talos - Advanced repair speed
			if (unit->id == UnitId::TerranTalos) {
				hpGain *= 1.5;
			}
			target->heal(hpGain, unit);

			if (target->hitPoints < units_dat::MaxHitPoints[target->id]) {
				return;
			}
		}

		unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);
		unit->mainOrderState = 1; //unit->orderToIdle();
		if (target && !(target->status & UnitStatus::InTransport)) {
			target->connectedUnit = 0;
		}
	}

	// Hooked to allow for general Repair and some other shit - Pr0nogo
	s32 isOrderValid(char orderId, Unit* unit, s32 unused, Unit* target, s32 bErrorFlag) // 00474D90
	{
		u16 errorStringId;
		bool ofsubCompare;
		u8 ofsubResult;
		s32 unusedResult;

		switch (orderId)
		{
			case OrderId::Repair1:
			{
				errorStringId = 884;
				if (!target)
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				errorStringId = 883;
				if (!(target->status & UnitStatus::Completed))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				//if (target->hitPoints >= units_dat::MaxHitPoints[target->id])
				//{
				//	if (bErrorFlag)
				//	{
				//		*(s16*)bErrorFlag = errorStringId;
				//	}
				//	return 0;
				//}
				return 1;
			} break;
			case OrderId::Harvest1:
			{
				if (!target)
				{
					return 1;
				}
				errorStringId = unit->getRace() + 893;
				if (target->id == UnitId::ResourceVespeneGeyser)
				{
					//if (bErrorFlag)
					//{
					//	*(s16*)bErrorFlag = errorStringId;
					//}
					//return 0;

					return 1;
				}
				if (!helper::unitIsResource(target))
				{
					return 0;
				}
				errorStringId = 892;
				if (!(units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				errorStringId = 896;
				if (unit->playerId != target->playerId && scbw::unit_id_is_capped_gas_structure(target->id))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				return 1;
			} break;
			case OrderId::Pickup2:
			{
				errorStringId = 890;
				if (!target || unit->playerId != target->playerId)
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				if (helper::canEnterTransport(unit, target))
				{
					return 1;
				}
				if (units_dat::SpaceProvided[unit->id] >= units_dat::SpaceRequired[target->id])
				{
					errorStringId = 891;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = errorStringId;
				}
				return 0;
			} break;
			case OrderId::HealMove:
			{
				if (!target)
				{
					return 1;
				}
				if (!helper::techUseAllowed(unit, TechId::Healing, unit->playerId))
				{
					return 0;
				}
				errorStringId = 1325;
				if (!(target->status & 6) // GroundedBuilding and InAir
					&& target->hitPoints < units_dat::MaxHitPoints[target->id])
				{
					return 1;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = errorStringId;
				}
				return 0;
			} break;
			default:
			{
				return 1;
			}
		}
		return unusedResult;
	}

	// Hooked to investigate Repair crashes - Pr0nogo
	s32 orderAllowed(Unit* unit, u16 orderId, s32 player) // 0046DC20
	{
		s32 result = 0;
		s16 unitId;

		*lastInternalError = 0;
		if (orderId < OrderId::None)
		{
			if (orders_dat::RequirementsOffset[orderId] == -1)
			{
				helper::parseOrdersDatReqs();
			}
			if (player == unit->playerId)
			{
				if (orderId == OrderId::RallyPoint1 || orderId == OrderId::RallyPoint2 || unit->status & UnitStatus::Completed)
				{
					if (unit->isFrozen())
					{
						*lastInternalError = 10;
						result = 0;
					}
					else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk && orderId != OrderId::Unburrow)
					{
						*lastInternalError = 11;
						result = 0;
					}
					else
					{
						unitId = unit->id;
						if (unitId == UnitId::TerranMason && unit->mainOrderId == OrderId::ConstructingBuilding && orderId != 1
							|| (scbw::unitCanNuke(unit) && unit->mainOrderId == OrderId::NukeTrack)
							|| unitId == UnitId::ProtossArchon && unit->mainOrderId == OrderId::CompletingArchonSummon
							|| unitId == UnitId::ProtossMindTyrant && unit->mainOrderId == OrderId::CompletingArchonSummon)
						{
							*lastInternalError = 8;
							result = 0;
						}
						else
						{
							if (orders_dat::RequirementsOffset[orderId])
							{
								result = helper::parseRequirementOpcodes(unit,
									orderId,
									player,
									orders_dat::RequirementsOffset[orderId],
									(s32)ordersRequirementTree);
							}
							else
							{
								*lastInternalError = 23;
							}
						}
					}
				}
				else
				{
					*lastInternalError = 20;
					result = 0;
				}
			}
			else
			{
				*lastInternalError = 1;
				result = 0;
			}
		}
		else
		{
			*lastInternalError = 15;
			result = 0;
		}
		return result;
	}

	// Hooked to remove right-click error message on harvesting from geysers. -Pr0nogo
	s32 show_right_click_error_if_needed(Unit* target) // 00455A00
	{
		s32 sfx_id = -1;
		s32 text_id = -1;
		Unit** selected_units = &clientSelectionGroup->unit[11];
		Unit** selected_units_2 = &clientSelectionGroup->unit[11];
		do
		{
			Unit* unit = *selected_units;
			if (*selected_units)
			{
				u8 action = RightClickActions::NoCommand_AutoAttack;
				u16 unit_id = unit->id;
				if (unit_id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
				{
					action = RightClickActions::NoMove_NormalAttack;
				}
				else
				{
					char right_click_order = units_dat::RightClickAction[unit_id];
					if (!right_click_order)
					{
						if (unit->status & UnitStatus::GroundedBuilding)
						{
							if (helper::unit_is_factory(unit))
							{
								right_click_order = RightClickActions::NormalMove_NoAttack;
							}
						}
					}
					action = right_click_order;
				}
				if (unit->status & UnitStatus::IsHallucination)
				{
					if (action == RightClickActions::Harvest || action == RightClickActions::HarvestAndRepair)
					{
						action = RightClickActions::NormalMove_NormalAttack;
					}
				}

				switch (action)
				{
				case RightClickActions::NormalMove_NormalAttack:
					if (!target)
					{
						return 1;
					}
					if (!helper::is_unit_enemy(*activePortraitUnit, target))
					{
						return 1;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
					{
						return 1;
					}
					if (helper::unit_can_attack_target(target, unit, 1))
					{
						return 1;
					}
					text_id = 879;
					sfx_id = unit->getRace() + 138;
				case RightClickActions::NoMove_NormalAttack:
					if (!target)
					{
						break;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
					{
						break;
					}

					if (helper::unit_can_attack_target(target, unit, 1))
					{
						if (helper::is_unit_target_out_of_max_range(unit, target))
						{
							text_id = 888;
						}
						else
						{
							if (!helper::is_unit_target_inside_min_range(unit, target))
							{
								return 1;
							}
							text_id = 889;
						}
					}
					else
					{
						text_id = 879;
					}
					sfx_id = unit->getRace() + 138;
				case RightClickActions::Harvest:
				case RightClickActions::HarvestAndRepair:
					if (!target)
					{
						return 1;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
					{
						return 1;
					}
					if (target->id == UnitId::ResourceVespeneGeyser)
					{
						//sfx_id = unit->getRace() + 1;
						//text_id = unit->getRace()  + 893;
					}
					else
					{
						if (!helper::is_unit_enemy(*activePortraitUnit, target))
						{
							return 1;
						}
						if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
						{
							return 1;
						}
						if (helper::unit_can_attack_target(target, unit, 1))
						{
							return 1;
						}
						text_id = 879;
						sfx_id = unit->getRace() + 138;
					}
					break;
				default:
					return 1;
				}
			}
			selected_units = selected_units_2 - 1;
			--selected_units_2;
		} while (selected_units_2 >= clientSelectionGroup->unit);
		if (sfx_id != -1)
		{
			if (sfx_id)
			{
				if (optionsList->sfx)
				{
					s32 sfx_volume = sfx_dat::Volume[sfx_id];
					u32 max_volume = 99;
					if (sfx_volume >= 99)
					{
						if (sfx_volume >= 0)
						{
							if (sfx_volume >= 99)
							{
								max_volume = sfx_dat::Volume[sfx_id];
							}
						}
						else
						{
							max_volume = 0;
						}
					}
					if (*use_quiet_sounds)
					{
						max_volume /= 5;
					}
					if (max_volume > 10)
					{
						helper::play_transmission_from_location(sfx_id, max_volume, 0, 0);
					}
				}
			}
		}
		if (text_id != -1)
		{
			u32 text_id_zeroed = text_id - 1;
			if (text_id)
			{
				char* text_string = 0;
				if (text_id_zeroed < *statTxtTbl) {
					text_string = (char*)statTxtTbl + statTxtTbl[text_id_zeroed + 1];
				}
				if (text_string) {
					if (*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
					{
						strncpy(draw_out_text_left, text_string, 0xDAu);
						text_bytes[12] = 4;
						*unit_stat_update_timer = GetTickCount() + 7000;
						helper::update_text_display(12);
					}
					return 0;
				}
			}
		}
		return 0;
	}
}