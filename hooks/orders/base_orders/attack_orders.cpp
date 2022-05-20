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
				// 004656A2:
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				// 004656B2:
				if (unit->unit_ai != nullptr)
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else
				{
					// 004656C6
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{
			// 004656D8:
			if (!unit->canAttackTarget(target, true))
			{
				unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			}
			else
			{
				// 004656F7:
				u32 status_saved = (unit->status & UnitStatus::HoldingPosition) + (unit->status & UnitStatus::Disabled);
				u8 newOrderId;

				if (unit->id == UnitId::ProtossAccantor)
				{
					// 00465720:
					if (unit->mainOrderId == OrderId::ReaverHold)
					{
						newOrderId = OrderId::ReaverHold;
					}
					else
					{
						newOrderId = OrderId::ReaverFight;
					}
				}
				else
				{
					// 00465730
					if (unit->mainOrderId == OrderId::HoldPosition1)
					{
						newOrderId = OrderId::HoldPosition1;
					}
					else
					{
						newOrderId = OrderId::CarrierFight;
					}
				}

				// 0046573E:
				helper::prepend_order_targeting_unit(unit, target, newOrderId);
				prepareForNextOrder(unit);

				// restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;
			}
		}
	}

	void orders_CarrierAttack1(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target == nullptr)
		{
			if (unit->orderQueueHead != nullptr)
			{
				// 00465966:
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				// 00465981
				if (unit->unit_ai != nullptr)
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else // 00465998
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{
			// 004659AD:
			if (!unit->canAttackTarget(target, true))
			{
				unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			}
			else
			{
				// 004659CF
				u32 status_saved = (unit->status & UnitStatus::Disabled) + (unit->status & UnitStatus::HoldingPosition);
				u8 newOrderId;
				// give orders to Stewards and Plasma Shells (through outHangarChild)
				helper::function_00465780(unit);

				if (unit->id == UnitId::ProtossAccantor)
				{
					// 00465A03:
					if (unit->mainOrderId == OrderId::ReaverHold)
					{
						newOrderId = OrderId::ReaverHold;
					}
					else
					{
						newOrderId = OrderId::ReaverFight;
					}
				}
				else
				{
					// 00465A13:
					if (unit->mainOrderId == OrderId::HoldPosition1)
					{
						newOrderId = OrderId::HoldPosition1;
					}
					else
					{
						newOrderId = OrderId::CarrierFight;
					}
				}
				// 00465A21:
				helper::prepend_order_targeting_unit(unit, target, newOrderId);
				prepareForNextOrder(unit);

				// restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;
			}
		}
	}

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

		// perform lots of checks, possibly in an attempt to
		// auto-select a better target if the turret has choice
		helper::PickNewAttackTargetIfNeeded(unit);
		target = unit->orderTarget.unit;

		if (target != nullptr)
		{
			unit->orderTarget.pt.x = target->sprite->position.x;
			unit->orderTarget.pt.y = target->sprite->position.y;
		}

		// 004779AE:
		if (unit->isFrozen()
			|| target == nullptr
			|| target->status & UnitStatus::Invincible
			|| target->sprite->flags & SpriteFlags::Hidden
			|| helper::unitCantSeeCloakedTarget(unit, target))
		{
			jump_to_77A45 = true;
		}
		else
		{
			// 00477A04
			if ((unit->subunit)->isSubunit())
			{
				current_unit = unit->subunit;
			}
			else
			{
				current_unit = unit;
			}

			// identical to check performed by unitIsCarrier @ 0x00401470
			if (unit->id == UnitId::ProtossSolarion)
			{
				jump_to_77AFE = true;
			}
			// identical to check performed by unitIsReaver @ 0x00401490, using if(!unitIsReaver())
			else if (!(unit->id == UnitId::ProtossAccantor))
			{
				jump_to_77A9D = true;
			}
			else if (target->status & UnitStatus::InAir)
			{
				jump_to_77A45 = true;
			}
			else if (unit->hasPathToUnit(target))
			{
				jump_to_77AFE = true;
			}
			else
			{
				jump_to_77A45 = true;
			}
		}

		do
		{
			bJumpToEarlierCode = false;
			if (jump_to_77A45)
			{
				unit->userActionFlags |= 1;
				if (unit->mainOrderId == OrderId::Die)
				{
					prepareForNextOrder(unit);	// 00477CED
				}
				else
				{
					jump_to_77A70 = true;
				}
			}

			jump_to_77A45 = false;
			if (jump_to_77A70) // 00477A70
			{
				bool bStopLoop77A70 = false;
				while (!bStopLoop77A70 && unit->orderQueueTail != nullptr)
				{
					if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
						unit->orderQueueTail->orderId != OrderId::TurretGuard)
					{
						bStopLoop77A70 = true;
					}
					else // 00477A94:
					{
						helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
					}
				}
				jump_to_77CDA = true;
			}
			jump_to_77A70 = false;
			if (jump_to_77A9D) // 00477A9D
			{

				if (helper::unitCanInfest(unit))
				{
					if (helper::isInfestableUnit(target))
					{
						jump_to_77AFE = true;
					}
					else
					{
						jump_to_77A45 = true;
						bJumpToEarlierCode = true;
					}
				}
				else
				{
					jump_to_77AB7 = true;
				}
			}

			jump_to_77A9D = false;
			if (jump_to_77AB7)
			{
				// 00477ACC
				if (target->status & UnitStatus::InAir)
				{
					current_unit_weaponId = units_dat::AirWeapon[current_unit->id]; // 00477AD2
				}
				else
				{
					current_unit_weaponId = current_unit->getGroundWeapon();		// 00477AE6
				}
				if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
				{
					current_unit_weaponId = WeaponId::None;
				}
				// 00477AF6
				if (current_unit_weaponId == WeaponId::None)
				{
					jump_to_77A45 = true;
					bJumpToEarlierCode = true;
				}
				else
				{
					jump_to_77AFE = true;
				}
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
					// 00477B0E
					Unit* current_subunit = unit->subunit;
					bool jump_to_77BD7 = false;

					if (unit->id != UnitId::TerranPhalanxTankModeTurret
						|| current_subunit->unit_ai == nullptr
						|| (current_subunit->canUseTech(TechId::TankSiegeMode, unit->playerId) != TRUE)
						|| !helper::isUnitVisible(unit))
						jump_to_77BD7 = true;
					else
					{
						// 00477B5F:
						u32 result_getMaxWpnRange = unit->getMaxWeaponRange(units_dat::GroundWeapon[UnitId::TerranPhalanxSiegeModeTurret]);
						if (!helper::is_target_within_min_movement_range(current_subunit, target, result_getMaxWpnRange))
						{
							jump_to_77BD7 = true;
						}
						else
						{
							// 00477B77:
							result_getMaxWpnRange = unit->getMaxWeaponRange(units_dat::GroundWeapon[UnitId::TerranPhalanxTankModeTurret]);

							if (helper::is_target_within_min_movement_range(current_subunit, target, result_getMaxWpnRange)
								|| helper::isUnitCritter(target))
							{
								jump_to_77BD7 = true;
							}
							else
							{ // 00477B9D
								unit->orderComputer(OrderId::Nothing2);

								// this orderId is hardcoded, not calculated 
								unit->orderSimple(units_dat::AttackUnitOrder[UnitId::TerranPhalanxSiegeModeTurret], false);
								current_subunit->orderComputer(OrderId::SiegeMode);

								// this orderId is hardcoded, not calculated
								current_subunit->issueQueuedOrderTarget(units_dat::AttackUnitOrder[UnitId::TerranPhalanxSiegeMode], target, false);
							}
						}
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
								{
									current_unit = unit->subunit;
								}
								else
								{
									current_unit = unit;
								}

								// 00477C0C
								if (target->status & UnitStatus::InAir)
								{
									current_unit_weaponId = current_unit->getAirWeapon();
								}
								else
								{
									current_unit_weaponId = current_unit->getGroundWeapon(); // include the lurker special case so it's ok
								}
								if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
								{
									current_unit_weaponId = WeaponId::None;
								}
								// 00477C40
								if (current_unit_weaponId >= WeaponId::None)
								{
									jump_to_77C78 = true;
								}
								else
									if (weapons_dat::MinRange[current_unit_weaponId] != 0
										&& current_unit->isTargetWithinMinRange(target, weapons_dat::MinRange[current_unit_weaponId]))
									{
										jump_to_77C78 = true;
									}
									else
									{
										// 00477C63:
										if (helper::is_target_within_min_movement_range(current_unit, target, unit->getMaxWeaponRange(current_unit_weaponId)))
										{
											jump_to_77CA1 = true;
										}
										else
										{
											jump_to_77C78 = true;
										}
									}
							}
							else
							{
								jump_to_77C78 = true;
							}
						}
						else
						{
							jump_to_77CA1 = true;
						}
						if (jump_to_77C78)
						{
							if ((unit->subunit)->mainOrderId == units_dat::AttackUnitOrder[(unit->subunit)->id]
								&& helper::getRightClickActionOrder(unit->subunit) != RightClickActions::NoMove_NormalAttack)
							{
								jump_to_77CA1 = true;
							}
							else
							{
								unit->orderComputer(OrderId::TurretGuard); // 00477C93
							}
						}

						jump_to_77C78 = false;
						if (jump_to_77CA1)
						{
							if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
								|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
							{
								unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
								unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
							}

							// 00477CC1
							if (!((unit->subunit)->movementFlags & MovementFlags::Accelerating))
							{
								helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);
							}
						}
						jump_to_77CA1 = false;
					}
					jump_to_77BD7 = false;
				}
			}
			jump_to_77AFE = false;
			if (jump_to_77CDA)
			{
				unit->performAnotherOrder(OrderId::TurretGuard, 0, 0, nullptr, UnitId::None);
				prepareForNextOrder(unit);
			}
			jump_to_77CDA = false;
		}
		while (bJumpToEarlierCode);
	}

	void orders_AttackFixedRange(Unit* unit)
	{
		Unit* target;
		bool jump_to_77E48 = false;
		bool jump_to_77E99 = false;
		if (unit->unit_ai != nullptr)
		{
			unit->orderTarget.unit = helper::findBestAttackTarget(unit); // 00477D13
		}
		else
		{
			// perform lots of checks, possibly in an attempt to
			// auto-select a better target if the unit has choice
			helper::PickNewAttackTargetIfNeeded(unit); // 00477D1F
		}

		// 00477D25:
		target = unit->orderTarget.unit;
		if (target != nullptr)
		{
			unit->orderTarget.pt.x = target->sprite->position.x;
			unit->orderTarget.pt.y = target->sprite->position.y;
		}
		// 00477D42:
		if (unit->isFrozen()
			|| target == nullptr
			|| target->status & UnitStatus::Invincible
			|| target->sprite->flags & SpriteFlags::Hidden
			|| helper::unitCantSeeCloakedTarget(unit, target))
		{
			jump_to_77E48 = true;
		}
		else
		{
			Unit* current_unit;
			bool jump_to_77DEF = false;
			// 00477DB4:
			if (unit->subunit->isSubunit()) // check if is not null AND is a subunit
			{
				current_unit = unit->subunit;
			}
			else
			{
				current_unit = unit;
			}

			// 00477DC4
			if (unit->id == UnitId::ProtossSolarion)
			{
				jump_to_77E99 = true;
			}
			else
				if (!(unit->id == UnitId::ProtossAccantor))
				{
					jump_to_77DEF = true;
				}
				else
					if (target->status & UnitStatus::InAir)
					{
						jump_to_77E48 = true;
					}
					else
						if (unit->hasPathToUnit(target))
						{
							jump_to_77E99 = true;
						}
						else
						{
							jump_to_77E48 = true;
						}
			if (jump_to_77DEF)
			{
				if (helper::unitCanInfest(unit))
				{
					if (helper::isInfestableUnit(target))
					{
						jump_to_77E99 = true;
					}
					else
					{
						jump_to_77E48 = true;
					}
				}
				else
				{
					u8 currentWeaponId;
					// 00477E1A
					if (target->status & UnitStatus::InAir)
					{
						currentWeaponId = current_unit->getAirWeapon();
					}
					else
					{
						currentWeaponId = current_unit->getGroundWeapon();
					}
					if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
					{
						currentWeaponId = WeaponId::None;
					}
					// 00477E44
					if (currentWeaponId != WeaponId::None)
					{
						jump_to_77E99 = true;
					}
					else
					{
						jump_to_77E48 = true;
					}
				}
			}
		}

		if (jump_to_77E48)
		{
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
				if (unit->unit_ai != nullptr) // 00477E6B
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else // 00477E83
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
		}

		if (jump_to_77E99)
		{
			bool jump_to_77EC8 = false;
			bool jump_to_77ED6 = false;
			if (helper::isUnitInWeaponRange(unit, target))
			{
				// 00477ED6
				if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
					|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
				{
					unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
					unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
				}
				helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);
			}
			else
				if (unit->unit_ai == nullptr)
				{
					unit->orderToIdle(); // 00477EC8
				}
				else
				{
					unit->orderTo(OrderId::AttackMove, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}
		}
	}

	void orders_SapUnit(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target == nullptr)
		{ // 004788EB
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 00478928
			if (!unit->canAttackTarget(target, true))
			{
				unit->orderTo(OrderId::Move, target->sprite->position.x, target->sprite->position.y);
			}
			else
			{ // 0047894D
				bool bStopThere = false;
				if (unit->mainOrderState == 0)
				{ // 00478965
					if (helper::move_to_target(target, unit))
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
						helper::set_resource_target(unit);
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
							// 00478A11
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
		// function_00478370:
		// probably perform checks to see if the order can be done
		if (helper::function_00478370(unit, unit->mainOrderId)
			&& unit->mainOrderTimer == 0)
		{ // 00478E0B
			unit->mainOrderTimer = 0; // used to be 15
			if (helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) == helper::get_region_id(unit->orderTarget.pt.x, unit->orderTarget.pt.y))
			{
				helper::make_to_hold_position(unit);
				unit->orderToIdle();
			}
			else
			{ // 00478E65
				bool jump_to_78EAE = false;
				if (unit->getMovableState() != 0
					&& (unit->orderTarget.pt.x != unit->sprite->position.x || unit->orderTarget.pt.y != unit->sprite->position.y))
				{ // 00478E80

					if (helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y))
					{
						jump_to_78EAE = true;
					}
					// 00478E91
					else if (helper::function_00476610(unit, (s32)unit->orderTarget.pt.x, (s32)unit->orderTarget.pt.y))
						{
							helper::make_to_hold_position(unit);
						}
				}
				else
				{
					jump_to_78EAE = true;
				}

				if (jump_to_78EAE)
				{
					helper::function_00477820(unit, saved_mainOrderId);
				}
			}
		}
	}

	void orders_AttackMove(Unit* unit)
	{
		// function_00478370:
		// Logically, would check if a target is in range, and
		// if that's the case (not returning 0), stop the "move" 
		// part that follow to attack the unit met
		if (helper::function_00478370(unit, unit->mainOrderId) != 0
			&& unit->mainOrderTimer == 0)
		{ // 00479058
			unit->mainOrderTimer = 0; // used to be 15
			if (unit->moveTarget.pt.x != unit->sprite->position.x
				|| unit->moveTarget.pt.y != unit->sprite->position.y)
			{ // Logically, this function would check one last time if a target is in range, and attack it if that's the case, else AttackMove end here
				helper::function_00477820(unit, unit->mainOrderId); // 00479092
			}
			else
			{
				unit->orderToIdle();
			}
		}
	}

	void orders_AttackUnit(Unit* unit)
	{
		if (playerTable[unit->playerId].type != PlayerType::Computer
			|| (unit->orderTarget.unit != nullptr && units_dat::BaseProperty[(unit->orderTarget.unit)->id] & UnitProperty::Building)
			|| !helper::Ai_TryReturnHome(unit, 0))
		{ // 00479C05

			// perform lots of checks, possibly in an attempt to
			// auto-select a better target if the unit has choice
			helper::PickNewAttackTargetIfNeeded(unit);

			// function perform tests to see if attack is possible
			// and how it should be done and return true if attack
			// can be done
			if (helper::Order_AttackUnit_Main(unit))
			{ // 00479C16
				if (unit->unit_ai != nullptr)
				{
					helper::AI_AttackUnit(unit);
				}
				helper::getWeaponBeginIscript(unit, IscriptAnimation::GndAttkRpt);
			}
		}
	}
}