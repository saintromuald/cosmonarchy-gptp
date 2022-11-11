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
	void orders_reaver_attack(Unit* unit)
	{
		Unit* target = unit->order_target.unit;
		if (target == nullptr)
		{
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		}
		else
		{
			if (!unit->can_attack_target(target, true))
			{
				unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
			}
			else
			{
				u32 status_saved = (unit->status & UnitStatus::HoldingPosition) + (unit->status & UnitStatus::Disabled);
				u8 newOrderId;

				if (unit->id == UnitId::ProtossAccantorOld)
				{
					if (unit->main_order_id == OrderId::ReaverHold)
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
					if (unit->main_order_id == OrderId::HoldPosition1)
					{
						newOrderId = OrderId::HoldPosition1;
					}
					else
					{
						newOrderId = OrderId::CarrierFight;
					}
				}
				helper::prepend_order_targeting_unit(unit, target, newOrderId);
				prepare_for_next_order(unit);

				// restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;
			}
		}
	}

	void orders_carrier_attack(Unit* unit)
	{
		Unit* target = unit->order_target.unit;
		if (target == nullptr)
		{
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		}
		else
		{
			if (!unit->can_attack_target(target, true))
			{
				unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
			}
			else
			{
				u32 status_saved = (unit->status & UnitStatus::Disabled) + (unit->status & UnitStatus::HoldingPosition);
				u8 newOrderId;
				// give orders to Stewards and Plasma Shells (through out_hangar_child)
				helper::set_interceptors_to_standby(unit);

				if (unit->id == UnitId::ProtossAccantorOld)
				{
					if (unit->main_order_id == OrderId::ReaverHold)
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
					if (unit->main_order_id == OrderId::HoldPosition1)
					{
						newOrderId = OrderId::HoldPosition1;
					}
					else
					{
						newOrderId = OrderId::CarrierFight;
					}
				}
				helper::prepend_order_targeting_unit(unit, target, newOrderId);
				prepare_for_next_order(unit);

				// restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled + UnitStatus::HoldingPosition);
				unit->status = status_saved | unit->status;
			}
		}
	}

	void orders_turret_attack(Unit* unit)
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

		// perform lots of checks, possibly in an attempt to auto-select a better target if the turret has choice
		helper::pick_new_target_if_needed(unit);
		target = unit->order_target.unit;

		if (target != nullptr)
		{
			unit->order_target.pt.x = target->sprite->position.x;
			unit->order_target.pt.y = target->sprite->position.y;
		}

		if (unit->is_frozen()
			|| target == nullptr
			|| target->status & UnitStatus::Invincible
			|| target->sprite->flags & SpriteFlags::Hidden
			|| helper::unit_cant_see_cloaked_target(unit, target))
		{
			jump_to_77A45 = true;
		}
		else
		{
			if ((unit->subunit)->is_subunit())
			{
				current_unit = unit->subunit;
			}
			else
			{
				current_unit = unit;
			}

			// identical to check performed by unit_is_carrier @ 0x00401470
			if (unit->id == UnitId::ProtossSolarion)
			{
				jump_to_77AFE = true;
			}
			// identical to check performed by unit_is_reaver @ 0x00401490, using if(!unit_is_reaver())
			else if (!(unit->id == UnitId::ProtossAccantorOld))
			{
				jump_to_77A9D = true;
			}
			else if (target->status & UnitStatus::InAir)
			{
				jump_to_77A45 = true;
			}
			else if (unit->has_path_to_unit(target))
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
				unit->user_action_flags |= 1;
				if (unit->main_order_id == OrderId::Die)
				{
					prepare_for_next_order(unit);
				}
				else
				{
					jump_to_77A70 = true;
				}
			}

			jump_to_77A45 = false;
			if (jump_to_77A70)
			{
				bool bStopLoop77A70 = false;
				while (!bStopLoop77A70 && unit->order_queue_tail != nullptr)
				{
					if (!orders_dat::can_be_interrupted[unit->order_queue_tail->order_id] &&
						unit->order_queue_tail->order_id != OrderId::TurretGuard)
					{
						bStopLoop77A70 = true;
					}
					else
					{
						helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
					}
				}
				jump_to_77CDA = true;
			}
			jump_to_77A70 = false;
			if (jump_to_77A9D)
			{

				if (helper::unit_can_infest(unit))
				{
					if (helper::is_infestable_unit(target))
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
				if (target->status & UnitStatus::InAir)
				{
					current_unit_weaponId = units_dat::air_weapon[current_unit->id];
				}
				else
				{
					current_unit_weaponId = current_unit->get_ground_weapon();
				}
				if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
				{
					current_unit_weaponId = WeaponId::None;
				}
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
				if (!helper::is_target_visible(unit, target))
				{
					jump_to_77A45 = true;
					bJumpToEarlierCode = true;
				}
				else
				{
					Unit* current_subunit = unit->subunit;
					bool jump_to_77BD7 = false;

					if (unit->id != UnitId::TerranPhalanxTankModeTurret
					||  current_subunit->unit_ai == nullptr
					|| (current_subunit->can_use_tech(TechId::TankSiegeMode, unit->player) != TRUE)
					|| !helper::is_unit_visible(unit))
					{
						jump_to_77BD7 = true;
					}
					else
					{
						u32 result_getMaxWpnRange = unit->get_max_weapon_range(units_dat::ground_weapon[UnitId::TerranPhalanxSiegeModeTurret]);
						if (!helper::is_target_within_min_movement_range(current_subunit, target, result_getMaxWpnRange))
						{
							jump_to_77BD7 = true;
						}
						else
						{
							result_getMaxWpnRange = unit->get_max_weapon_range(units_dat::ground_weapon[UnitId::TerranPhalanxTankModeTurret]);

							if (helper::is_target_within_min_movement_range(current_subunit, target, result_getMaxWpnRange)
							||  helper::is_unit_critter(target))
							{
								jump_to_77BD7 = true;
							}
							else
							{
								unit->order_computer(OrderId::Nothing2);

								// this order_id is hardcoded, not calculated 
								unit->order_simple(units_dat::attack_unit_order[UnitId::TerranPhalanxSiegeModeTurret], false);
								current_subunit->order_computer(OrderId::SiegeMode);

								// this order_id is hardcoded, not calculated
								current_subunit->issue_queued_order_target(units_dat::attack_unit_order[UnitId::TerranPhalanxSiegeMode], target, false);
							}
						}
					}

					if (jump_to_77BD7)
					{
						bool jump_to_77C78 = false;
						bool jump_to_77CA1 = false;
						if (target != nullptr)
						{
							if (helper::is_target_visible(unit, target))
							{
								if ((unit->subunit)->is_subunit())
								{
									current_unit = unit->subunit;
								}
								else
								{
									current_unit = unit;
								}

								if (target->status & UnitStatus::InAir)
								{
									current_unit_weaponId = current_unit->get_air_weapon();
								}
								else
								{
									current_unit_weaponId = current_unit->get_ground_weapon(); // include the Lakizilisk special case so it's ok
								}
								if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
								{
									current_unit_weaponId = WeaponId::None;
								}
								if (current_unit_weaponId == WeaponId::None)
								{
									jump_to_77C78 = true;
								}
								else
									if (weapons_dat::min_range[current_unit_weaponId] != 0
										&& current_unit->is_target_within_min_range(target, weapons_dat::min_range[current_unit_weaponId]))
									{
										jump_to_77C78 = true;
									}
									else
									{
										if (helper::is_target_within_min_movement_range(current_unit, target, unit->get_max_weapon_range(current_unit_weaponId)))
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
							if ((unit->subunit)->main_order_id == units_dat::attack_unit_order[(unit->subunit)->id]
								&& helper::get_right_click_action_order(unit->subunit) != RightClickActions::NoMove_NormalAttack)
							{
								jump_to_77CA1 = true;
							}
							else
							{
								unit->order_computer(OrderId::TurretGuard);
							}
						}

						jump_to_77C78 = false;
						if (jump_to_77CA1)
						{
							if (unit->order_target.pt.x != unit->next_target_waypoint.x
								|| unit->order_target.pt.y != unit->next_target_waypoint.y)
							{
								unit->next_target_waypoint.x = unit->order_target.pt.x;
								unit->next_target_waypoint.y = unit->order_target.pt.y;
							}
							if (!((unit->subunit)->movement_flags & MovementFlags::Accelerating))
							{
								helper::get_weapon_begin_iscriptt(unit, IscriptAnimation::GndAttkRpt);
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
				unit->perform_another_order(OrderId::TurretGuard, 0, 0, nullptr, UnitId::None);
				prepare_for_next_order(unit);
			}
			jump_to_77CDA = false;
		}
		while (bJumpToEarlierCode);
	}

	void orders_attack_fixed_range(Unit* unit)
	{
		Unit* target;
		bool jump_to_77E48 = false;
		bool jump_to_77E99 = false;
		if (unit->unit_ai != nullptr)
		{
			unit->order_target.unit = helper::find_best_attack_target(unit); // 00477D13
		}
		else
		{
			// perform lots of checks, possibly in an attempt to
			// auto-select a better target if the unit has choice
			helper::pick_new_target_if_needed(unit); // 00477D1F
		}

		target = unit->order_target.unit;
		if (target != nullptr)
		{
			unit->order_target.pt.x = target->sprite->position.x;
			unit->order_target.pt.y = target->sprite->position.y;
		}
		if (unit->is_frozen()
		||  target == nullptr
		||  target->status & UnitStatus::Invincible
		||  target->sprite->flags & SpriteFlags::Hidden
		||  helper::unit_cant_see_cloaked_target(unit, target))
		{
			jump_to_77E48 = true;
		}
		else
		{
			Unit* current_unit;
			bool jump_to_77DEF = false;
			if (unit->subunit->is_subunit()) // check if is not null AND is a subunit
			{
				current_unit = unit->subunit;
			}
			else
			{
				current_unit = unit;
			}

			if (unit->id == UnitId::ProtossSolarion)
			{
				jump_to_77E99 = true;
			}
			else if (!(unit->id == UnitId::ProtossAccantorOld))
			{
				jump_to_77DEF = true;
			}
			else if (target->status & UnitStatus::InAir)
			{
				jump_to_77E48 = true;
			}
			else if (unit->has_path_to_unit(target))
			{
				jump_to_77E99 = true;
			}
			else
			{
				jump_to_77E48 = true;
			}
			if (jump_to_77DEF)
			{
				if (helper::unit_can_infest(unit))
				{
					if (helper::is_infestable_unit(target))
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
					u8 currentWeaponId = current_unit->get_ground_weapon();
					if (target->status & UnitStatus::InAir)
					{
						currentWeaponId = current_unit->get_air_weapon();
					}
					if (current_unit->id == UnitId::ProtossClarion && scbw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
					{
						currentWeaponId = WeaponId::None;
					}
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
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else if (unit->unit_ai)
			{
				unit->order_computer(OrderId::ComputerAI);
			}
			else
			{
				unit->order_computer(units_dat::return_to_idle_order[unit->id]);
			}
		}

		if (jump_to_77E99)
		{
			bool jump_to_77EC8 = false;
			bool jump_to_77ED6 = false;
			if (helper::is_unit_in_weapon_range(unit, target))
			{
				if (unit->order_target.pt.x != unit->next_target_waypoint.x
					|| unit->order_target.pt.y != unit->next_target_waypoint.y)
				{
					unit->next_target_waypoint.x = unit->order_target.pt.x;
					unit->next_target_waypoint.y = unit->order_target.pt.y;
				}
				helper::get_weapon_begin_iscriptt(unit, IscriptAnimation::GndAttkRpt);
			}
			else if (!unit->unit_ai)
			{
				unit->order_to_idle();
			}
			else
			{
				unit->order_to(OrderId::AttackMove, unit->order_target.pt.x, unit->order_target.pt.y);
			}
		}
	}

	void orders_sap_unit(Unit* unit)
	{
		Unit* target = unit->order_target.unit;
		if (!target)
		{
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		}
		else
		{
			if (!unit->can_attack_target(target, true))
			{
				unit->order_to(OrderId::Move, target->sprite->position.x, target->sprite->position.y);
			}
			else
			{
				bool bStopThere = false;
				if (unit->main_order_state == 0)
				{
					if (helper::move_to_target(target, unit))
					{
						unit->main_order_state = 1;
					}
					else
					{
						bStopThere = true;
					}
				}

				if (!bStopThere && unit->main_order_state == 1)
				{
					if (!unit->is_target_within_min_range(target, 4)
						&& unit->get_movable_state() != 2)
					{
						if (unit->is_target_within_min_range(target, 256)
							&& target->movement_flags & MovementFlags::Accelerating)							
						{
							// maybe trigger the acceleration of the infested terran, or something else related to distances?
							helper::move_to_collide(unit, target);
						}
						else
						{
							helper::move_towards(unit, target);
						}
						bStopThere = true;
					}

					else
					{
						helper::set_resource_target(unit);
						if (!unit->is_target_within_min_range(target, weapons_dat::outer_splash_radius[WeaponId::InfestedTerranSuicide]))
						{
							unit->order_to_idle();
							bStopThere = true;
						}
						else
						{
							unit->order_target.unit = unit;
							Image* current_image = unit->sprite->images.head;
							while (current_image)
							{
								current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
								current_image = current_image->link.next;
							}
							unit->status |= UnitStatus::CanNotReceiveOrders;
							unit->main_order_state = 2;
						}
					}
				}

				if (!bStopThere && unit->main_order_state == 2)
				{
					if (unit->order_signal & 1)
					{
						unit->status |= UnitStatus::IsSelfDestructing;
						unit->remove(); // orders_self_destructing
					}
				}
			}
		}
	}

	void orders_sap_location(Unit* unit)
	{
		bool bStopThere = false;
		if (unit->main_order_state == 0)
		{
			bStopThere = !helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
			if (unit->order_target.pt.x != unit->next_target_waypoint.x
				|| unit->order_target.pt.y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = unit->order_target.pt.x;
				unit->next_target_waypoint.y = unit->order_target.pt.y;
			}
			if (!bStopThere)
			{
				unit->main_order_state = 1;
			}
		}

		if (!bStopThere && unit->main_order_state == 1)
		{
			if (unit->sprite->position.x != unit->move_target.pt.x
				|| unit->sprite->position.y != unit->move_target.pt.y)
			{
				bStopThere = true;
			}
			else
			{
				if (unit->status & UnitStatus::Unmovable)
				{
					u32 distance =
						scbw::get_distance_fast(
							unit->order_target.pt.x * 256,
							unit->order_target.pt.y * 256,
							unit->halt.x,
							unit->halt.y);
					if ((distance / 256) > weapons_dat::outer_splash_radius[WeaponId::InfestedTerranSuicide])
					{
						unit->order_to_idle();
						bStopThere = true;
					}
				}

				/*
				  The original code set EAX to 0 or 1 depending on Unmovable status, then add 1 to this value, thus equality to 0 is always impossible.
				  Kept an equivalent code in comment for bugchecking
				if ((unit->status & UnitStatus::Unmovable) + 1) == false
				{
					bStopThere = true;
				}
				*/

				if (!bStopThere)
				{
					Image* current_image = unit->sprite->images.head;
					unit->order_target.unit = unit;
					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->main_order_state = 2;
				}
			}
		}

		if (!bStopThere && unit->main_order_state == 2)
		{
			if (unit->order_signal & 1)
			{
				unit->status |= UnitStatus::IsSelfDestructing;
				unit->remove(); // orders_self_destructing
			}
		}
	}

	void orders_attack_move_ep(Unit* unit)
	{
		u8 saved_mainOrderId = unit->main_order_id; //[EBP-0x14]
		// attack_move_react_to_attack: probably perform checks to see if the order can be done
		if (helper::attack_move_react_to_attack(unit, unit->main_order_id)
		&&  unit->main_order_timer == 0)
		{
			unit->main_order_timer = 0; // used to be 15
			if (helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) == helper::get_region_id(unit->order_target.pt.x, unit->order_target.pt.y))
			{
				helper::make_to_hold_position(unit);
				unit->order_to_idle();
			}
			else
			{
				bool jump_to_78EAE = false;
				if (unit->get_movable_state() != 0
				&& (unit->order_target.pt.x != unit->sprite->position.x || unit->order_target.pt.y != unit->sprite->position.y))
				{

					if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
					{
						jump_to_78EAE = true;
					}
					else if (helper::ai_is_unreachable_position(unit, (s32)unit->order_target.pt.x, (s32)unit->order_target.pt.y))
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
					helper::attack_move_try_pick_target(unit, saved_mainOrderId);
				}
			}
		}
	}

	void orders_attack_move(Unit* unit)
	{
		// attack_move_react_to_attack:
		// Logically, would check if a target is in distance, and if that's the case (not returning 0), stop the "move" part that follow to attack the unit met
		if (helper::attack_move_react_to_attack(unit, unit->main_order_id) != 0
		&&  unit->main_order_timer == 0)
		{
			unit->main_order_timer = 0; // used to be 15
			if (unit->move_target.pt.x != unit->sprite->position.x
			||  unit->move_target.pt.y != unit->sprite->position.y)
			{
				// Logically, this function would check one last time if a target is in distance, and attack it if that's the case, else AttackMove end here
				helper::attack_move_try_pick_target(unit, unit->main_order_id);
			}
			else
			{
				unit->order_to_idle();
			}
		}
	}

	void orders_attack_unit(Unit* unit)
	{
		if (g_player_table[unit->player].type != PlayerType::Computer
		|| (unit->order_target.unit != nullptr && units_dat::base_property[(unit->order_target.unit)->id] & UnitProperty::Building)
		|| !helper::ai_try_return_home(unit, 0))
		{
			// perform lots of checks, possibly in an attempt to auto-select a better target if the unit has choice
			helper::pick_new_target_if_needed(unit);

			// function perform tests to see if attack is possible and how it should be done and return true if attack can be done
			if (helper::attack_unit_main(unit))
			{
				if (unit->unit_ai != nullptr)
				{
					helper::ai_attack_unit(unit);
				}
				helper::get_weapon_begin_iscriptt(unit, IscriptAnimation::GndAttkRpt);
			}
		}
	}

	// Ported from public GPTP to fix the Plasma Shell launch sound. -Pr0nogo
	void orders_strafe_unit2(Unit* unit) // 00465E00
	{
		if (unit->main_order_state <= 6)
		{
			if (unit->main_order_state == 0)
			{
				if (unit->remove_timer == 0 || unit->remove_timer > 90)
				{
					unit->remove_timer = 90;
				}

				unit->status |= UnitStatus::NoCollide;
				unit->main_order_timer = 7;
				if (unit->order_target.unit)
				{
					u8 rand_value = helper::randomize_short(0) & 1;
					helper::play_sound_from_unit(SoundId::Bullet_pTrFir00_wav + rand_value, unit, 1, 0);
					helper::move_to_target(unit, unit->order_target.unit);
					unit->main_order_state = 2;
				}
				else if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
				{
					unit->main_order_state = 2;
				}
			}
			else if (unit->main_order_state == 2)
			{
				if (unit->main_order_timer == 0)
				{
					unit->status &= ~UnitStatus::NoCollide; // unset flag if set
					unit->main_order_state = 3;
				}
			}

			if (unit->main_order_state == 2 || unit->main_order_state == 3)
			{
				Unit* target = unit->order_target.unit;
				bool bNotEndThere;
				if (target)
				{
					if (unit->is_target_within_min_range(target, 256)
					&&  target->movement_flags & MovementFlags::Accelerating)
					{
						helper::move_to_collide(unit, target);
					}
					else
					{
						helper::move_towards(unit, target);
					}
					bNotEndThere = unit->is_target_within_min_range(target, weapons_dat::inner_splash_radius[units_dat::ground_weapon[unit->id]]);
				}
				else
				{
					bNotEndThere = (unit->get_movable_state() != MovableState::NotReachedDestination);
				}

				if (bNotEndThere)
				{
					Image* current_image;
					unit->order_target.pt.x = unit->sprite->position.x;
					unit->order_target.pt.y = unit->sprite->position.y;
					current_image = unit->sprite->images.head;
					while (current_image != NULL)
					{
						current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					unit->main_order_state = 6;
				}
			}
			else if (unit->main_order_state == 6)
			{
				if (unit->order_signal & 1)
				{
					unit->status |= UnitStatus::IsSelfDestructing;
					unit->remove();
				}
			}
		}
	}

	// Hooked to debug Stewards in a post-extender environment. -Pr0nogo
	void set_interceptors_to_standby(Unit* unit) // 00465780
	{
		for (Unit* out_hangar_child = unit->unit_with_hangar.out_hangar_child; out_hangar_child; out_hangar_child = out_hangar_child->hangar_unit.hangar_link.next)
		{
			out_hangar_child->sprite->elevation = unit->sprite->elevation - 1;
			Unit* target = unit->order_target.unit;
			u32 attack_order = units_dat::attack_unit_order[out_hangar_child->id];
			out_hangar_child->user_action_flags |= 1u;
			if (target)
			{
				helper::order(1, out_hangar_child, attack_order, target->sprite->position, target, UnitId::None);
			}
			else
			{
				helper::order(1, out_hangar_child, attack_order, {}, 0, UnitId::None);
			}
			helper::orders_nothing_2(out_hangar_child);
		}
	}
}