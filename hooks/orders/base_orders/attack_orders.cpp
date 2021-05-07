#include "attack_orders.h"
#include <SCBW/api.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

namespace hooks
{

	void orders_ReaverAttack1(Unit* unit)
	{

		Unit* target = unit->orderTarget.unit;

		if (target == nullptr)
		{

			if (unit->orderQueueHead != nullptr)
			{
				//656A2:
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				//656B2:
				if (unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else
					//656C6:
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}

		}
		else
		{

			//656D8:
			if (!unit->canAttackTarget(target, true))
				unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			else
			{

				//656F7:
				u32 status_saved = (unit->status & UnitStatus::HoldingPosition) + (unit->status & UnitStatus::Disabled);
				u8 newOrderId;

				if (unit->id == UnitId::ProtossAccantor)
				{
					//65720:
					if (unit->mainOrderId == OrderId::ReaverHold)
						newOrderId = OrderId::ReaverHold;
					else
						newOrderId = OrderId::ReaverFight;
				}
				else
				{
					//65730
					if (unit->mainOrderId == OrderId::HoldPosition1)
						newOrderId = OrderId::HoldPosition1;
					else
						newOrderId = OrderId::CarrierFight;
				}

				//6573E:
				helper::function_00474A70(unit, target, newOrderId);
				prepareForNextOrder(unit);

				//restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;

			}

		}

	} //void orders_ReaverAttack1(Unit* unit)

	;

	void orders_CarrierAttack1(Unit* unit)
	{

		Unit* target = unit->orderTarget.unit;

		if (target == nullptr)
		{

			if (unit->orderQueueHead != nullptr)
			{
				//65966:
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				//65981
				if (unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else //65998
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}

		}
		else
		{

			//659AD:
			if (!unit->canAttackTarget(target, true))
				unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			else
			{

				//659CF
				u32 status_saved = (unit->status & UnitStatus::Disabled) + (unit->status & UnitStatus::HoldingPosition);
				u8 newOrderId;

				//give orders to interceptors/scarabs (through outHangarChild)
				helper::function_00465780(unit);

				if (unit->id == UnitId::ProtossAccantor)
				{
					//65A03:
					if (unit->mainOrderId == OrderId::ReaverHold)
						newOrderId = OrderId::ReaverHold;
					else
						newOrderId = OrderId::ReaverFight;
				}
				else
				{
					//65A13:
					if (unit->mainOrderId == OrderId::HoldPosition1)
						newOrderId = OrderId::HoldPosition1;
					else
						newOrderId = OrderId::CarrierFight;
				}

				//65A21:

				helper::function_00474A70(unit, target, newOrderId);
				prepareForNextOrder(unit);

				//restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;

			}

		}

	} //void orders_CarrierAttack1(Unit* unit)

	;

	void orders_TurretAttack(Unit* unit)
	{

		Unit* target;
		Unit* current_unit;
		u8 current_unit_weaponId;

		bool jump_to_77A45 = false;
		bool jump_to_77A70 = false;
		bool jump_to_77A9D = false;
		bool jump_to_77AB7 = false;
		bool jump_to_77AFE = false;
		bool jump_to_77CDA = false;
		bool bJumpToEarlierCode;

		//perform lots of checks, possibly in an attempt to
		//auto-select a better target if the turret has choice
		helper::function_00477510(unit);

		target = unit->orderTarget.unit;

		if (target != nullptr)
		{
			unit->orderTarget.pt.x = target->sprite->position.x;
			unit->orderTarget.pt.y = target->sprite->position.y;
		}

		//779AE:
		if (
			unit->status & UnitStatus::DoodadStatesThing ||
			unit->lockdownTimer != 0 ||
			unit->stasisTimer != 0 ||
			unit->maelstromTimer != 0 ||
			target == nullptr ||
			target->status & UnitStatus::Invincible ||
			target->sprite->flags & CSprite_Flags::Hidden ||
			helper::unitCantSeeCloakedTarget(unit, target)
			)
			jump_to_77A45 = true;
		else
		{

			//77A04
			if ((unit->subunit)->isSubunit())
				current_unit = unit->subunit;
			else
				current_unit = unit;

			//identical to check performed by unitIsCarrier @ 0x00401470
			if (unit->id == UnitId::ProtossSolarion)
				jump_to_77AFE = true;
			else
				//identical to check performed by unitIsReaver @ 0x00401490, using if(!unitIsReaver())
				if (!(unit->id == UnitId::ProtossAccantor))
					jump_to_77A9D = true;
				else
					if (target->status & UnitStatus::InAir)
						jump_to_77A45 = true;
					else
						if (unit->hasPathToUnit(target))
							jump_to_77AFE = true;
						else
							jump_to_77A45 = true;

		} //If Unit Able To Fight (various conditions)

		do
		{

			bJumpToEarlierCode = false;

			if (jump_to_77A45)
			{

				unit->userActionFlags |= 1;

				if (unit->mainOrderId == OrderId::Die)
					prepareForNextOrder(unit);	//77CED
				else
					jump_to_77A70 = true;

			} //if(jump_to_77A45)

			jump_to_77A45 = false;

			if (jump_to_77A70)
			{ //77A70

				bool bStopLoop77A70 = false;

				while (!bStopLoop77A70 && unit->orderQueueTail != nullptr)
				{

					if (
						!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
						unit->orderQueueTail->orderId != OrderId::TurretGuard
						)
						bStopLoop77A70 = true;
					else
						//77A94:
						helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);
				}

				jump_to_77CDA = true;

			}

			jump_to_77A70 = false;

			if (jump_to_77A9D)
			{ //77A9D

				if (helper::unitCanInfest(unit))
				{

					if (helper::isInfestableUnit(target))
						jump_to_77AFE = true;
					else
					{
						jump_to_77A45 = true;
						bJumpToEarlierCode = true;
					}

				}
				else
					jump_to_77AB7 = true;

			}

			jump_to_77A9D = false;

			if (jump_to_77AB7)
			{
				/*
				if(unit->id == UnitId::ProtossArbiter && unit->pAI != nullptr) {
					jump_to_77A45 = true;
					bJumpToEarlierCode = true;
				}
				else {
					*/
					//77ACC
				if (target->status & UnitStatus::InAir)
					current_unit_weaponId = units_dat::AirWeapon[current_unit->id]; //77AD2
				else
					current_unit_weaponId = current_unit->getGroundWeapon();		//77AE6
				if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
				{
					current_unit_weaponId = WeaponId::None;
				}
				//77AF6
				if (current_unit_weaponId == WeaponId::None)
				{
					jump_to_77A45 = true;
					bJumpToEarlierCode = true;
				}
				else
					jump_to_77AFE = true;

				//} //if(unit->id != UnitId::ProtossArbiter || unit->pAI == nullptr)

			}

			jump_to_77AB7 = false;

			if (jump_to_77AFE)
			{

				if (!helper::isTargetVisible(unit, target))
				{
					jump_to_77A45 = true;
					bJumpToEarlierCode = true;
				}
				else
				{

					//77B0E
					Unit* current_subunit = unit->subunit;
					bool jump_to_77BD7 = false;

					if (
						unit->id != UnitId::TerranPhalanxTankModeTurret ||
						current_subunit->pAI == nullptr ||
						(current_subunit->canUseTech(TechId::TankSiegeMode, unit->playerId) != TRUE) ||
						!helper::isUnitVisible(unit)
						)
						jump_to_77BD7 = true;
					else
					{

						//77B5F:
						u32 result_getMaxWpnRange = unit->getMaxWeaponRange(units_dat::GroundWeapon[UnitId::TerranPhalanxSiegeModeTurret]);

						if (!helper::isTargetWithinMinMovementRange(current_subunit, target, result_getMaxWpnRange))
							jump_to_77BD7 = true;
						else
						{

							//77B77:
							result_getMaxWpnRange = unit->getMaxWeaponRange(units_dat::GroundWeapon[UnitId::TerranPhalanxTankModeTurret]);

							if (
								helper::isTargetWithinMinMovementRange(current_subunit, target, result_getMaxWpnRange) ||
								helper::isUnitCritter(target)
								)
								jump_to_77BD7 = true;
							else
							{ //77B9D

								unit->orderComputerCL(OrderId::Nothing2);

								//this orderId is hardcoded, not calculated 
								unit->orderSimple(units_dat::AttackUnitOrder[UnitId::TerranPhalanxSiegeModeTurret], false);

								current_subunit->orderComputerCL(OrderId::SiegeMode);

								//this orderId is hardcoded, not calculated 
								current_subunit->issueQueuedOrderTarget(units_dat::AttackUnitOrder[UnitId::TerranPhalanxSiegeMode], target, false);

							}

						} //if(isTargetWithinMinMovementRange(current_subunit,target,result_getMaxWpnRange))

					}

					if (jump_to_77BD7)
					{

						bool jump_to_77C78 = false;
						bool jump_to_77CA1 = false;

						if (target != nullptr)
						{

							if (helper::isTargetVisible(unit, target))
							{

								if ((unit->subunit)->isSubunit())
									current_unit = unit->subunit;
								else
									current_unit = unit;

								//77C0C:
								if (target->status & UnitStatus::InAir)
									current_unit_weaponId = current_unit->getAirWeapon();
								else
									current_unit_weaponId = current_unit->getGroundWeapon(); //include the lurker special case so it's ok
								if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
								{
									current_unit_weaponId = WeaponId::None;
								}
								//77C40
								if (current_unit_weaponId >= WeaponId::None)
									jump_to_77C78 = true;
								else
									if (weapons_dat::MinRange[current_unit_weaponId] != 0 &&
										current_unit->isTargetWithinMinRange(target, weapons_dat::MinRange[current_unit_weaponId])
										)
										jump_to_77C78 = true;
									else
									{

										//77C63:

										if (helper::isTargetWithinMinMovementRange
										(
											current_unit,
											target,
											unit->getMaxWeaponRange(current_unit_weaponId)
										)
											)
											jump_to_77CA1 = true;
										else
											jump_to_77C78 = true;

									}

							} //if(isTargetVisible(unit,target))
							else
								jump_to_77C78 = true;

						} //if(target != nullptr)
						else
							jump_to_77CA1 = true;

						if (jump_to_77C78)
						{

							if ((unit->subunit)->mainOrderId == units_dat::AttackUnitOrder[(unit->subunit)->id] &&
								helper::getRightClickActionOrder(unit->subunit) != RightClickActions::NoMove_NormalAttack
								)
								jump_to_77CA1 = true;
							else
								unit->orderComputerCL(OrderId::TurretGuard);	//77C93

						}

						jump_to_77C78 = false;

						if (jump_to_77CA1)
						{

							if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x ||
								unit->orderTarget.pt.y != unit->nextTargetWaypoint.y
								)
							{
								unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
								unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
							}

							//77CC1
							if (!((unit->subunit)->movementFlags & MovementFlags::Accelerating))
								helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);

						}

						jump_to_77CA1 = false;

					} //if(jump_to_77BD7)

					jump_to_77BD7 = false;

				}
			} //if(jump_to_77AFE)

			jump_to_77AFE = false;

			if (jump_to_77CDA)
			{

				unit->performAnotherOrder(
					OrderId::TurretGuard,
					0,
					0,
					nullptr,
					UnitId::None
				);

				prepareForNextOrder(unit);

			}

			jump_to_77CDA = false;

		}
		while (bJumpToEarlierCode);

	} //void orders_TurretAttack(Unit* unit)


	;

	void orders_AttackFixedRange(Unit* unit)
	{

		Unit* target;

		bool jump_to_77E48 = false;
		bool jump_to_77E99 = false;

		if (unit->pAI != nullptr)
			unit->orderTarget.unit = helper::findBestAttackTarget(unit); //77D13
		else
			//perform lots of checks, possibly in an attempt to
			//auto-select a better target if the unit has choice
			helper::function_00477510(unit); //77D1F

		//77D25:
		target = unit->orderTarget.unit;

		if (target != nullptr)
		{
			unit->orderTarget.pt.x = target->sprite->position.x;
			unit->orderTarget.pt.y = target->sprite->position.y;
		}

		//77D42:
		if (
			unit->status & UnitStatus::DoodadStatesThing ||
			unit->lockdownTimer != 0 ||
			unit->stasisTimer != 0 ||
			unit->maelstromTimer != 0 ||
			target == nullptr ||
			target->status & UnitStatus::Invincible ||
			target->sprite->flags & CSprite_Flags::Hidden ||
			helper::unitCantSeeCloakedTarget(unit, target)
			)
			jump_to_77E48 = true;
		else
		{

			Unit* current_unit;
			bool jump_to_77DEF = false;

			//77DB4:
			if (unit->subunit->isSubunit()) //check if is not null AND is a subunit
				current_unit = unit->subunit;
			else
				current_unit = unit;

			//77DC4:
			//identical to check performed by unitIsCarrier @ 0x00401470
			if (unit->id == UnitId::ProtossSolarion)
				jump_to_77E99 = true;
			else
				//identical to check performed by unitIsReaver @ 0x00401490, using if(!unitIsReaver())
				if (!(unit->id == UnitId::ProtossAccantor))
					jump_to_77DEF = true;
				else
					if (target->status & UnitStatus::InAir)
						jump_to_77E48 = true;
					else
						if (unit->hasPathToUnit(target))
							jump_to_77E99 = true;
						else
							jump_to_77E48 = true;

			if (jump_to_77DEF)
			{

				if (helper::unitCanInfest(unit))
				{
					if (helper::isInfestableUnit(target))
						jump_to_77E99 = true;
					else
						jump_to_77E48 = true;
				}
				else
				{

					//77E09:
					/*if(unit->id == UnitId::ProtossArbiter && unit->pAI != nullptr)
						jump_to_77E48 = true;
					else {
						*/
					u8 currentWeaponId;

					//77E1A:
					if (target->status & UnitStatus::InAir)
						currentWeaponId = current_unit->getAirWeapon();
					else
						currentWeaponId = current_unit->getGroundWeapon();
					if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
					{
						currentWeaponId = WeaponId::None;
					}
					//77E44
					if (currentWeaponId != WeaponId::None)
						jump_to_77E99 = true;
					else
						jump_to_77E48 = true;

					//}

				} //unit cannot infest

			} //jump_to_77DEF

		} //if unit able to fight (several conditions)

		if (jump_to_77E48)
		{

			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
				if (unit->pAI != nullptr) //77E6B
					unit->orderComputerCL(OrderId::ComputerAI);
				else //77E83
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);

		}

		if (jump_to_77E99)
		{

			bool jump_to_77EC8 = false;
			bool jump_to_77ED6 = false;

			if (helper::isUnitInWeaponRange(unit, target))
			{

				//77ED6
				if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x ||
					unit->orderTarget.pt.y != unit->nextTargetWaypoint.y
					)
				{
					unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
					unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
				}

				helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);

			}
			else
				if (unit->pAI == nullptr)
					unit->orderToIdle(); //77EC8
				else
					unit->orderTo(OrderId::AttackMove, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

		}

	} //void orders_AttackFixedRange(Unit* unit)

	;

	void orders_SapUnit(Unit* unit)
	{

		Unit* target = unit->orderTarget.unit;

		if (target == nullptr)
		{ //788EB

			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{

				if (unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);

			} //if(unit->orderQueueHead == nullptr)

		}
		else
		{ //78928

			if (!unit->canAttackTarget(target, true))
			{
				unit->orderTo(OrderId::Move, target->sprite->position.x, target->sprite->position.y);
			}
			else
			{ // 0047894D

				bool bStopThere = false;
				if (unit->mainOrderState == 0)
				{ // 00478965
					if (helper::moveToTarget(unit, target))
					{
						unit->mainOrderState = 1;
					}
					else
					{
						bStopThere = true;
					}
				}

				if (!bStopThere && unit->mainOrderState == 1)
				{ // 00478975

					if (!unit->isTargetWithinMinRange(target, 4)
						&& unit->getMovableState() != 2)
					{ // 00478998

						if (unit->isTargetWithinMinRange(target, 256)
							&& target->movementFlags & MovementFlags::Accelerating)							
						{
							// maybe trigger the acceleration of the infested terran, or something else related to distances?
							helper::moveToCollide(unit, target);
						}

						else // 004789BF
						{
							helper::moveTowards(unit, target);
						}

						bStopThere = true;

					}
					else
					{ // 004789CC

						helper::setNextWaypoint(unit);
						if (!unit->isTargetWithinMinRange(target, weapons_dat::OuterSplashRadius[WeaponId::Suicide_InfestedTerran]))
						{
							unit->orderToIdle();
							bStopThere = true;
						}
						else
						{ // 004789F4
							unit->orderTarget.unit = unit;
							Image* current_image = unit->sprite->images.head;
							while (current_image != nullptr)
							{
								current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
								current_image = current_image->link.next;
							}
							// 00478A11:
							unit->status |= UnitStatus::CanNotReceiveOrders;
							unit->mainOrderState = 2;
						}
					}
				}

				if (!bStopThere && unit->mainOrderState == 2)
				{ // 00478A1E
					if (unit->orderSignal & 1)
					{
						unit->status |= UnitStatus::IsSelfDestructing;
						unit->remove(); // orders_SelfDestructing
					}
				}
			}
		}
	}

	void orders_SapLocation(Unit* unit)
	{
		bool bStopThere = false;
		if (unit->mainOrderState == 0)
		{ // 00478A5C
			bStopThere = !helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
				|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
			}
			if (!bStopThere)
			{
				unit->mainOrderState = 1;
			}
		}

		if (!bStopThere && unit->mainOrderState == 1)
		{ // 00478A89

			if (unit->sprite->position.x != unit->moveTarget.pt.x
				|| unit->sprite->position.y != unit->moveTarget.pt.y)
			{
				bStopThere = true;
			}
			else
			{ // 00478AA0
				if (unit->status & UnitStatus::Unmovable)
				{ // 00478AB8
					u32 distance =
						scbw::getDistanceFast(
							unit->orderTarget.pt.x * 256,
							unit->orderTarget.pt.y * 256,
							unit->halt.x,
							unit->halt.y);
					if ((distance / 256) > weapons_dat::OuterSplashRadius[WeaponId::Suicide_InfestedTerran])
					{
						unit->orderToIdle();
						bStopThere = true;
					}
				}

				/*The original code set EAX to 0 or 1 depending on Unmovable status,
				  then add 1 to this value, thus equality to 0 is always impossible.
				  Kept an equivalent code in comment for bugchecking
							if((unit->status & UnitStatus::Unmovable) + 1) == false
								bStopThere = true;
				*/

				if (!bStopThere)
				{ // 00478AF3
					Image* current_image = unit->sprite->images.head;
					unit->orderTarget.unit = unit;
					while (current_image != nullptr)
					{
						current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					// 00478B10
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->mainOrderState = 2;
				}
			}
		}

		if (!bStopThere && unit->mainOrderState == 2)
		{ // 00478B1E
			if (unit->orderSignal & 1)
			{
				unit->status |= UnitStatus::IsSelfDestructing;
				unit->remove(); //orders_SelfDestructing
			}
		}
	}

	void orders_AttackMoveEP(Unit* unit)
	{

		u8 saved_mainOrderId = unit->mainOrderId; //[EBP-0x14]

		//function_00478370:
		//probably perform checks to see if the order
		//can be done, but which exactly I don't know

		if (helper::function_00478370(unit, unit->mainOrderId) &&
			unit->mainOrderTimer == 0
			)
		{ //78E0B

			unit->mainOrderTimer = 15;

			if (
				helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y) == helper::SAI_GetRegionIdFromPxEx(unit->orderTarget.pt.x, unit->orderTarget.pt.y)
				)
			{
				helper::makeToHoldPosition(unit);
				unit->orderToIdle();
			}
			else
			{ //78E65

				bool jump_to_78EAE = false;

				if (
					unit->getMovableState() != 0 &&
					(unit->orderTarget.pt.x != unit->sprite->position.x ||
						unit->orderTarget.pt.y != unit->sprite->position.y)
					)
				{ //78E80

					if (
						helper::changeMovementTargetMoveTarget2(				//return value is actually from 
							unit,						//SetMoveTarget_xy @ 0x004EB820
							unit->orderTarget.pt.x,
							unit->orderTarget.pt.y
						)
						)
						jump_to_78EAE = true;
					else //78E91
						if (helper::function_00476610(unit, (s32)unit->orderTarget.pt.x, (s32)unit->orderTarget.pt.y))
							helper::makeToHoldPosition(unit);

				}
				else
					jump_to_78EAE = true;

				if (jump_to_78EAE)
					helper::function_00477820(unit, saved_mainOrderId);	//use attack range, may cause idle,
												//may look for target, and others...

			} //if different regions

		} //if order timer is at 0 and others order checks

	} //void orders_AttackMoveEP(Unit* unit)

	;

	void orders_AttackMove(Unit* unit)
	{

		//function_00478370:
		//Logically, would check if a target is in range, and
		//if that's the case (not returning 0), stop the "move" 
		//part that follow to attack the unit met
		if (
			helper::function_00478370(unit, unit->mainOrderId) != 0 &&
			unit->mainOrderTimer == 0
			)
		{ //79058

			unit->mainOrderTimer = 15;

			if (
				unit->moveTarget.pt.x != unit->sprite->position.x ||
				unit->moveTarget.pt.y != unit->sprite->position.y
				/*|| (unit->status & UnitStatus::Unmovable) + 1 == 0)*/ //impossible code yet in original
				)
				//Logically, this function would check one last time if a target is in range,
				//and attack it if that's the case, else AttackMove end here
				helper::function_00477820(unit, unit->mainOrderId); //79092
			else
				unit->orderToIdle();

		}

	} //void orders_AttackMove(Unit* unit)

	;

	void orders_AttackUnit(Unit* unit)
	{

		if (
			playerTable[unit->playerId].type != PlayerType::Computer
			||
			(
				unit->orderTarget.unit != nullptr &&
				units_dat::BaseProperty[(unit->orderTarget.unit)->id] & UnitProperty::Building
				)
			||
			!helper::function_00462EA0(unit, 0)	//if unit controlled by AI, do lots of things and
										//return true after/if validating a final order
			)
		{ //79C05

		 //perform lots of checks, possibly in an attempt to
		 //auto-select a better target if the unit has choice
			helper::function_00477510(unit);

			//function perform tests to see if attack is possible
			//and how it should be done and return true if attack
			//can be done
			if (helper::function_00478540(unit))
			{ //79C16

				if (unit->pAI != nullptr)
					helper::AI_AttackUnit(unit);

				helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);

			}

		}

	} //void orders_AttackUnit(Unit* unit)

	;

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

//End of helper functions
