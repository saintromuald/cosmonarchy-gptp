#include "spider_mine.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "byte_operations_utils.h"

namespace hooks
{
	// Return the best target for the mine. If there is no suitable target, return nullptr instead.
	Unit* findBestTerranMineTarget(Unit* mine) // 00441270
	{
		s32 range;
		// Don't search for a target if the mine is under a Disruption Web
		if (mine->status & UnitStatus::CanNotAttack)
		{
			return nullptr;
		}
		range = 32 * mine->getSeekRange();
		return
			scbw::UnitFinder::getNearestTarget(
				mine->getX() - range, mine->getY() - range,
				mine->getX() + range, mine->getY() + range,
				mine, [&](Unit* target)
		{
			// Don't attack friendly or allied units
			if (!mine->isTargetEnemy(target))
			{
				return false;
			}
			// Don't attack invincible units
			if (target->status & UnitStatus::Invincible)
			{
				return false;
			}
			// Don't attack air units / buildings
			if (target->status & (UnitStatus::InAir | UnitStatus::GroundedBuilding))
			{
				return false;
			}
			// Don't attack hovering units
			if (units_dat::MovementFlags[target->id] == MovementFlags::HoverUnit)
			{
				return false;
			}
			return true;
		});
	}

	void orders_TerranMine(Unit* unit) // 00463DF0
	{
		if (unit->mainOrderState == 0)
		{
			unit->groundWeaponCooldown = MineBurrowTime;
			unit->mainOrderState = 1;
		}

		if (unit->mainOrderState == 1)
		{
			if (unit->groundWeaponCooldown == 0)
			{
				unit->sprite->playIscriptAnim(IscriptAnimation::Burrow, true);
				unit->status |= UnitStatus::NoCollide;
				unit->mainOrderState = 2;
			}
		}

		if (unit->mainOrderState == 2)
		{
			if (unit->orderSignal & 4)
			{
				if (!(unit->status & UnitStatus::IsHallucination))
				{
					helper::burrowGeneric(unit); // related to movement
					helper::instantCloak(unit); // related to cloaking/stealth
				}
				unit->mainOrderState = 3;
			}
		}

		if (unit->mainOrderState == 3)
		{
			Unit* target = helper::SpiderMine_AcquireTarget(unit);

			if (target != nullptr)
			{
				unit->orderTarget.unit = target;
				unit->sprite->playIscriptAnim(IscriptAnimation::UnBurrow, true);
				unit->sprite->flags &= ~SpriteFlags::Burrowed;
				// might have been some cloaking order before
				unit->setSecondaryOrder(OrderId::Nothing2);
				unit->mainOrderState = 4;
			}
		}

		if (unit->mainOrderState == 4)
		{
			if (unit->orderSignal & 4)
			{
				unit->sprite->elevationLevel = units_dat::Elevation[unit->id];
				unit->status &= ~UnitStatus::NoCollide;
				if (unit->orderTarget.unit != nullptr)
				{
					// probably trigger movement/acceleration of unit toward the target
					helper::moveToCollide(unit, unit->orderTarget.unit);

					unit->mainOrderState = 5;
				}
				else
				{
					unit->mainOrderState = 1;
				}
			}
		}

		// unlike the previous states, going directly from state 4 to 5 in the same frame is impossible
		else if (unit->mainOrderState == 5)
		{
			bool bIsCompletelyInRange = false;
			bool bEndThere = false;
			helper::unburrowGeneric(unit); // related to movement, similar to function_004E9A30
			if (unit->orderTarget.unit != nullptr)
			{
				if (unit->isTargetWithinMinRange(unit->orderTarget.unit, MineSeekRange))
				{
					// probably trigger movement/acceleration of unit
					// toward the target
					helper::moveToCollide(unit, unit->orderTarget.unit);
					if (unit->isTargetWithinMinRange(unit->orderTarget.unit, MineDetonateRange))
					{
						bIsCompletelyInRange = true; // jump to 00463F22
					}
					else if (unit->getMovableState() != 2) // unit not unmovable
					{
						bEndThere = true;
					}
				}
			}
			if (!bEndThere && !bIsCompletelyInRange) // 00463F14
			{
				helper::setNextWaypoint(unit);
				unit->mainOrderState = 1;
			}

			if (bIsCompletelyInRange) // 00463F22
			{
				Image* current_image;
				unit->orderTarget.unit = nullptr;
				unit->orderTarget.pt.x = unit->sprite->position.x;
				unit->orderTarget.pt.y = unit->sprite->position.y;
				current_image = unit->sprite->images.head;
				IscriptAnimation::Enum anim = IscriptAnimation::SpecialState1;
				if (unit->id == UnitId::TerranLobotomyMine)
				{
					anim = IscriptAnimation::Unused1;
				}
				while (current_image != nullptr)
				{
					current_image->playIscriptAnim(anim);
					current_image = current_image->link.next;
				}
				unit->mainOrderState = 6;
			}
		}

		// unlike state 5 with state 4, state 6 can be chained from
		// state 5, so no else here
		if (unit->mainOrderState == 6)
		{
			if (unit->orderSignal & 1)
			{
				unit->remove();
			}
		}
	}

	void orders_PlaceMine(Unit* unit) // 00464FD0
	{
		Unit* v1;
		Unit* target;
		s32 v3;
		Sprite* v4;
		s32 v5;
		s32 v6;
		Unit* v7;
		s32 v8;
		s32 v9;
		s32 v10;
		Sprite* v11;
		__int16 v12;
		Unit* v13;
		Unit* v14;
		Point16 a2;
		v1 = unit;
		if (unit->mainOrderState)
		{
			if (helper::techUseAllowed(unit, 3u, unit->playerId)
				&& ((v11 = v1->sprite, v1->moveTarget.pt.x != v11->position.x)
					|| v1->moveTarget.pt.y != v11->position.y
					|| ((v1->status & UnitStatus::Unmovable) != 0) != 1))
			{
				if ((helper::getDistanceFast(v1->halt.x, v1->orderTarget.pt.x << 8, v1->halt.y, v1->orderTarget.pt.y << 8) & 0xFFFFFF00) <= 0x1400)
				{
					helper::orders_HoldPosSuicidal(v1);
					v12 = v1->orderTarget.pt.y;
					a2.x = v1->orderTarget.pt.x;
					a2.y = v12;
					helper::fixTargetLocation(&a2, 0xDu);
					v13 = helper::createUnit(0xDu, a2.x, a2.y, (unsigned __int8)v1->playerId);
					v14 = v13;
					if (v13)
					{
						helper::updateUnitStatsFinishBuilding(v13);
						helper::updateUnitStrengthAndApplyDefaultOrders(v14);
						helper::turnUnit(v14, v1->currentDirection1);
						v1->vulture.spiderMineCount -= 1;
					}
					else
					{
						helper::displayLastNetErrorForPlayer(v1->playerId);
					}
					helper::toIdle(v1);
				}
				else if (v1->pAI)
				{
					if (helper::isUnitMovable(v1))
						helper::toIdle(v1);
				}
			}
			else if (v1->orderQueueHead)
			{
				v1->userActionFlags |= 1u;
				helper::orders_Nothing2(v1);
			}
			else if (v1->pAI)
			{
				helper::orderComputerClear(v1, 0x9Cu);
			}
			else
			{
				helper::orderComputerClear(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		}
		else
		{
			target = unit->orderTarget.unit;
			if (target)
			{
				v3 = helper::moveToTarget(target, unit);
				v4 = target->sprite;
				v5 = v4->position.y;
				v6 = v4->position.x;
				if (v6 != v1->nextTargetWaypoint.x || v5 != v1->nextTargetWaypoint.y)
				{
					v1->nextTargetWaypoint.x = v6;
					v1->nextTargetWaypoint.y = v5;
				}
				if (v3)
				{
					v7 = v1->orderTarget.unit;
					v1->orderTarget.pt.x = v7->sprite->position.x;
					v1->orderTarget.pt.y = v7->sprite->position.y;
					v1->mainOrderState = 1;
				}
			}
			else
			{
				v8 = unit->orderTarget.pt.y;
				v9 = unit->orderTarget.pt.x;
				ByteOperations::SetLowByte(v10, helper::setMoveTargetXY(v9, v8, unit));
				if (v9 != v1->nextTargetWaypoint.x || v8 != v1->nextTargetWaypoint.y)
				{
					v1->nextTargetWaypoint.x = v9;
					v1->nextTargetWaypoint.y = v8;
				}
				if (v10)
				{
					v1->mainOrderState = 1;
				}
			}
		}
	}

	// Hooked to try to fix mine burrowing - Pr0nogo
	char burrowGeneric(Unit* unit)
	{ // 004E9A30
		Unit* unitVar = unit;
		helper::toggleUnitPath(unit);

		Sprite* unitSprite = unitVar->sprite;
		char unitPathing = unitVar->pathingFlags;
		unitVar->movementState = 0;
		unitVar->pathingFlags = (unitPathing ^ (unitSprite->elevationLevel < 12u)) & 1 ^ unitPathing;
		unitVar->status |= UnitStatus::CloakingForFree | UnitStatus::Burrowed;
		unitSprite->flags |= 0x40u; // image is hidden

		Sprite* oldSprite = unitVar->sprite;
		Sprite* newSprite;
		if (unitVar->id == UnitId::ZergLakizilisk)
		{
			// Warning, tism detected
			// ByteOperations::SetLowByte(newSprite, switchSpriteId(oldSprite, SpriteId::White_Circle_Invisible));
			unitVar->sprite->elevationLevel = 1;
		}
		else
		{
			helper::switchSpriteId(oldSprite, SpriteId::White_Circle_Invisible);
			newSprite = unitVar->sprite;
			newSprite->elevationLevel = 1;
		}
		return (char)newSprite;
	}

}
