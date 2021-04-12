#include "spider_mine.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "byte_operations_utils.h"

//helper functions def
namespace {
	CUnit* SpiderMine_AcquireTarget(CUnit* spiderMine);					//0x00441270
	void function_00495400(CUnit* unit, CUnit* target);					//0x00495400
	void function_0049B1E0(CUnit* unit);								//0x0049B1E0
	void function_004E99D0(CUnit* unit);								//0x004E99D0
	void function_004E9A30(CUnit* unit);								//0x004E9A30
	void setNextWaypoint_Sub4EB290(CUnit* unit);						//0x004EB290

	// For orders_PlaceMine
	int techUseAllowed(CUnit* a1, __int16 a2, int a3);					//0x0046DD80
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4);		//0x0040C360
	int orders_HoldPosSuicidal(CUnit* a1);								//0x004EB5B0
	void fixTargetLocation(unsigned __int16 a1, Point32* a2);			//0x00401FA0
	CUnit* createUnit(__int16 a1, int a2, int a3, int a4);				//0x004A09D0
	void updateUnitStatsFinishBuilding(CUnit* a1);						//0x004A01F0
	unsigned int updateUnitStrengthAndApplyDefaultOrders(CUnit* a1);	//0x0049FA40
	void turnUnit(CUnit* a1, char a2);									//0x00495F20
	void displayLastNetErrorForPlayer(unsigned __int8 a1);				//0x0049E530
	void toIdle(CUnit* a1);												//0x004753A0
	int isUnitMovable(CUnit* a1);										//0x00401DC0
	void orders_Nothing2(CUnit* a1);									//0x00475000
	void orderComputerClear(CUnit* a1, unsigned __int8 a2);				//0x00475310
	signed int moveToTarget(CUnit* a1, CUnit* a2);						//0x004EB720
	bool setMoveTargetXY(int a1, int a2, CUnit* a3);					//0x004EB820

} //unnamed namespace

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

} //hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	CUnit* SpiderMine_AcquireTarget(CUnit* spiderMine) {

		const uint32 Func_SpiderMine_AcquireTarget = 0x00441270;

		static CUnit* target;

		__asm {
			PUSHAD
			MOV ESI, spiderMine
			CALL Func_SpiderMine_AcquireTarget
			MOV target, EAX
			POPAD
		}

		return target;

	};

	const uint32 Func_Sub495400 = 0x00495400;
	void function_00495400(CUnit* unit, CUnit* target) {

		__asm {
			PUSHAD
			MOV EDI, unit
			MOV EAX, target
			CALL Func_Sub495400
			POPAD
		}

	};

	void function_0049B1E0(CUnit* unit) {

		const uint32 Func_Sub49B1E0 = 0x0049B1E0;

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub49B1E0
			POPAD
		}

	};

	void function_004E99D0(CUnit* unit) {

		const uint32 Func_Sub4E99D0 = 0x004E99D0;

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E99D0
			POPAD
		}

	};

	void function_004E9A30(CUnit* unit) {

		const uint32 Func_Sub4E9A30 = 0x004E9A30;

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4E9A30
			POPAD
		}

	};

	//Related to path/movement decision
	const uint32 Func_sub_4EB290 = 0x004EB290;
	void setNextWaypoint_Sub4EB290(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_sub_4EB290
			POPAD
		}
	};

	// For orders_PlaceMine
	const uint32 Func_techUseAllowed = 0x0046DD80;
	int techUseAllowed(CUnit* a1, __int16 a2, int a3) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX DI, a2
			PUSH a3
			CALL Func_techUseAllowed
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getDistanceFast = 0x0040C360;
	unsigned int getDistanceFast(int a1, int a2, int a3, int a4) {
		static unsigned int result;
		__asm {
			PUSHAD
			MOV ECX, a1
			MOV EDX, a2
			PUSH a3
			PUSH a4
			CALL Func_getDistanceFast
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_orders_HoldPosSuicidal = 0x004EB5B0;
	int orders_HoldPosSuicidal(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV ESI, a1
			CALL Func_orders_HoldPosSuicidal
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_fixTargetLocation = 0x00401FA0;
	void fixTargetLocation(unsigned __int16 a1, Point32* a2) {
		__asm {
			PUSHAD
			MOVZX AX, a1
			MOV EDX, a2
			CALL Func_fixTargetLocation
			POPAD
		}
	}

	const uint32 Func_createUnit = 0x004A09D0;
	CUnit* createUnit(__int16 a1, int a2, int a3, int a4) {
		static CUnit* result;
		__asm {
			PUSHAD
			MOVZX CX, a1
			MOV EAX, a2
			PUSH a3
			PUSH a4
			CALL Func_createUnit
			POPAD
		}
		return result;
	}

	const uint32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
	void updateUnitStatsFinishBuilding(CUnit* a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	const uint32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
	unsigned int updateUnitStrengthAndApplyDefaultOrders(CUnit* a1) {
		static unsigned int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_updateUnitStrengthAndApplyDefaultOrders
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_turnUnit = 0x00495F20;
	void turnUnit(CUnit* a1, char a2) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_turnUnit
			POPAD
		}
	}

	const uint32 Func_displayLastNetErrorForPlayer = 0x0049E530;
	void displayLastNetErrorForPlayer(unsigned __int8 a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_displayLastNetErrorForPlayer
			POPAD
		}
	}

	const uint32 Func_toIdle = 0x004753A0;
	void toIdle(CUnit* a1) {
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_toIdle
			POPAD
		}
	}

	const uint32 Func_isUnitMovable = 0x00401DC0;
	int isUnitMovable(CUnit* a1) {
		static int result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isUnitMovable
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_orders_Nothing2 = 0x00475000;
	void orders_Nothing2(CUnit* a1) {
		__asm {
			PUSHAD
			MOV ECX, a1
			CALL Func_orders_Nothing2
			POPAD
		}
	}

	const uint32 Func_orderComputerClear = 0x00475310;
	void orderComputerClear(CUnit* a1, unsigned __int8 a2) {
		__asm {
			PUSHAD
			MOV ESI, a1
			MOVZX CL, a2
			CALL Func_orderComputerClear
			POPAD
		}
	}

	const uint32 Func_moveToTarget = 0x004EB720;
	signed int moveToTarget(CUnit* a1, CUnit* a2) {
		static signed int result;
		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_moveToTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_setMoveTargetXY = 0x004EB820;
	bool setMoveTargetXY(int a1, int a2, CUnit* a3) {
		static int result;
		__asm {
			PUSHAD
			MOV EBX, a1
			MOV EDI, a2
			MOV ESI, a3
			CALL Func_setMoveTargetXY
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

} //Unnamed namespace

//End of helper functions
