#include "spider_mine.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "byte_operations_utils.h"

namespace hooks {

	const int SPIDERMINE_BURROW_TIME = 60;
	const int SPIDERMINE_SEEKING_RANGE = 576;
	const int SPIDERMINE_DETONATE_RANGE = 30;

	//Return the best target for the Spider Mine. If there is no suitable target,
	//return nullptr instead.
	CUnit* findBestSpiderMineTargetHook(CUnit* spiderMine) {

		//Default StarCraft behavior
		int32 range;

		//Don't search for a target if the spider mine is under a Disruption Web
		if (spiderMine->status & UnitStatus::CanNotAttack)
			return nullptr;

		range = 32 * spiderMine->getSeekRange();

		return
			scbw::UnitFinder::getNearestTarget(
				spiderMine->getX() - range, spiderMine->getY() - range,
				spiderMine->getX() + range, spiderMine->getY() + range,
				spiderMine, [&](CUnit* target) {
			//Don't attack friendly / allied units
			if (!spiderMine->isTargetEnemy(target))
				return false;

			//Don't attack invincible units
			if (target->status & UnitStatus::Invincible)
				return false;

			//Don't attack air units / buildings
			if (target->status & (UnitStatus::InAir | UnitStatus::GroundedBuilding))
				return false;

			//Don't attack hovering units
			if (units_dat::MovementFlags[target->id] == MovementFlags::HoverUnit)
				return false;

			return true;
		}
		);

	};

	void orders_VultureMine(CUnit* unit) {

		if (unit->mainOrderState == 0) {
			unit->groundWeaponCooldown = SPIDERMINE_BURROW_TIME;
			unit->mainOrderState = 1;
		}

		if (unit->mainOrderState == 1) {

			if (unit->groundWeaponCooldown == 0) {
				unit->sprite->playIscriptAnim(IscriptAnimation::Burrow, true);
				unit->status |= UnitStatus::NoCollide;
				unit->mainOrderState = 2;
			}

		}

		if (unit->mainOrderState == 2) {
			if (!(unit->orderSignal & 4)) {

				if (!(unit->status & UnitStatus::IsHallucination)) {
					function_004E9A30(unit); //related to movement
					function_0049B1E0(unit); //related to cloaking/stealth
				}

				unit->mainOrderState = 3;

			}

		}

		if (unit->mainOrderState == 3) {
			CUnit* target = SpiderMine_AcquireTarget(unit);

			if (target != nullptr) {

				unit->orderTarget.unit = target;

				unit->sprite->playIscriptAnim(IscriptAnimation::UnBurrow, true);

				//disable burrowed flag if still on
				unit->sprite->flags &= ~CSprite_Flags::Burrowed;

				//might have been some cloaking order before
				unit->setSecondaryOrder(OrderId::Nothing2);

				unit->mainOrderState = 4;

			}

		}

		if (unit->mainOrderState == 4) {

			if (unit->orderSignal & 4) {

				unit->sprite->elevationLevel = units_dat::Elevation[unit->id];

				//disabled NoCollide flag if was still on
				unit->status &= ~UnitStatus::NoCollide;

				if (unit->orderTarget.unit != nullptr) {

					//probably trigger movement/acceleration of unit
					//toward the target
					function_00495400(unit, unit->orderTarget.unit);

					unit->mainOrderState = 5;

				} else
					unit->mainOrderState = 1;

			}

		}

		else //unlike the previous states, going directly from state 4 to 5 in the same frame is impossible
			if (unit->mainOrderState == 5) {
				bool bIsCompletelyInRange = false;
				bool bEndThere = false;

				function_004E99D0(unit); //related to movement, similar to function_004E9A30

				if (unit->orderTarget.unit != nullptr) {

					if (unit->isTargetWithinMinRange(unit->orderTarget.unit, SPIDERMINE_SEEKING_RANGE)) {

						//probably trigger movement/acceleration of unit
						//toward the target
						function_00495400(unit, unit->orderTarget.unit);

						if (unit->isTargetWithinMinRange(unit->orderTarget.unit, SPIDERMINE_DETONATE_RANGE))
							bIsCompletelyInRange = true; //jump to 63F22
						else
							if (unit->getMovableState() != 2) //unit not unmovable
								bEndThere = true;

					}

				}

				if (!bEndThere && !bIsCompletelyInRange) { //63F14
					setNextWaypoint_Sub4EB290(unit);
					unit->mainOrderState = 1;
				}

				if (bIsCompletelyInRange) { //63F22

					CImage* current_image;

					unit->orderTarget.unit = nullptr;
					unit->orderTarget.pt.x = unit->sprite->position.x;
					unit->orderTarget.pt.y = unit->sprite->position.y;

					current_image = unit->sprite->images.head;
					IscriptAnimation::Enum anim = IscriptAnimation::SpecialState1;
					if (unit->id == UnitId::TerranLobotomyMine) {
						anim = IscriptAnimation::Unused1;
					}
					while (current_image != nullptr) {
						current_image->playIscriptAnim(anim);
						current_image = current_image->link.next;
					}

					unit->mainOrderState = 6;

				}

			}

		//unlike state 5 with state 4, state 6 can be chained from
		//state 5, so no else here
		if (unit->mainOrderState == 6) {
			if (unit->orderSignal & 1) {
				//GPTP::logger << "SpiderMine - remove" << std::endl;
				unit->remove();
			}
		}

	};

	void orders_PlaceMine(CUnit* a1) {		// 0x00464FD0
		CUnit* v1; // esi
		CUnit* v2; // edi
		signed int v3; // eax
		CSprite* v4; // ecx
		int v5; // edx
		int v6; // ecx
		CUnit* v7; // eax
		int v8; // edi
		int v9; // ebx
		int v10; // eax
		CSprite* v11; // eax
		__int16 v12; // cx
		CUnit* v13; // eax
		CUnit* v14; // edi
		Point32 a2; // [esp+Ch] [ebp-4h]

		//scbw::print("Try place mine");
		v1 = a1;
		if (a1->mainOrderState) {
			if (techUseAllowed(a1, 3u, (unsigned __int8)a1->playerId)
				&& ((v11 = v1->sprite, v1->moveTarget.pt.x != v11->position.x)
					|| v1->moveTarget.pt.y != v11->position.y
					|| ((v1->status & UnitStatus::Unmovable) != 0) != 1)) {
				//scbw::printf("distance %X %X", (getDistanceFast(v1->halt.x, v1->orderTarget.pt.x << 8, v1->halt.y, v1->orderTarget.pt.y << 8) & 0xFFFFFF00),
				//	0x1400);
				if ((getDistanceFast(v1->halt.x, v1->orderTarget.pt.x << 8, v1->halt.y, v1->orderTarget.pt.y << 8) & 0xFFFFFF00) <= 0x1400) {
					//scbw::print("hold");
					orders_HoldPosSuicidal(v1);
					v12 = v1->orderTarget.pt.y;
					a2.x = v1->orderTarget.pt.x;
					a2.y = v12;
					fixTargetLocation(0xDu, &a2);
					//scbw::printf("create at %d %d, order target is %d %d", a2.x, a2.y,v1->orderTarget.pt.x,v1->orderTarget.pt.y);
					v13 = createUnit(0xDu, a2.x, a2.y, (unsigned __int8)v1->playerId);
					v14 = v13;
					if (v13) {
						updateUnitStatsFinishBuilding(v13);
						updateUnitStrengthAndApplyDefaultOrders(v14);
						turnUnit(v14, v1->currentDirection1);
						--v1->vulture.spiderMineCount;
					} else {
						displayLastNetErrorForPlayer(v1->playerId);
					}
					toIdle(v1);
				} else if (v1->pAI) {
					if (isUnitMovable(v1))
						toIdle(v1);
				}
			} else if (v1->orderQueueHead) {
				v1->userActionFlags |= 1u;
				orders_Nothing2(v1);
			} else if (v1->pAI) {
				orderComputerClear(v1, 0x9Cu);
			} else {
				orderComputerClear(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		} else {
			v2 = a1->orderTarget.unit;
			if (v2) {
				v3 = moveToTarget(v2, a1);
				v4 = v2->sprite;
				v5 = v4->position.y;
				v6 = v4->position.x;
				if (v6 != v1->nextTargetWaypoint.x || v5 != v1->nextTargetWaypoint.y) {
					v1->nextTargetWaypoint.x = v6;
					v1->nextTargetWaypoint.y = v5;
				}
				if (v3) {
					v7 = v1->orderTarget.unit;
					v1->orderTarget.pt.x = v7->sprite->position.x;
					v1->orderTarget.pt.y = v7->sprite->position.y;
					v1->mainOrderState = 1;
				}
			} else {
				v8 = a1->orderTarget.pt.y;
				v9 = a1->orderTarget.pt.x;
				ByteOperations::SetLowByte(v10, setMoveTargetXY(v9, v8, a1));
				if (v9 != v1->nextTargetWaypoint.x || v8 != v1->nextTargetWaypoint.y) {
					v1->nextTargetWaypoint.x = v9;
					v1->nextTargetWaypoint.y = v8;
				}
				if (v10)
					v1->mainOrderState = 1;
			}
		}
	}

}