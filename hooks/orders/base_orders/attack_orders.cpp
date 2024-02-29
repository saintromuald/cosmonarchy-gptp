#include "attack_orders.h"
#include <BW/api.h>

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
		if (!target)
		{
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			if (!unit->can_attack_target(target, true))
			{
				unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
			} else {
				u32 status_saved = (unit->status & UnitStatus::AttackMove) + (unit->status & UnitStatus::Disabled);
				u8 new_order_id = OrderId::Die;
				if (unit->id == UnitId::ProtossAccantorOld)
				{
					if (unit->main_order_id == OrderId::ReaverHold)
					{
						new_order_id = OrderId::ReaverHold;
					} else {
						new_order_id = OrderId::ReaverFight;
					}
				}
				else {
					if (unit->main_order_id == OrderId::HoldPositionSolarion)
					{
						new_order_id = OrderId::HoldPositionSolarion;
					} else
					{
						new_order_id = OrderId::CarrierFight;
					}
				}
				helper::prepend_order_targeting_unit(unit, target, new_order_id);
				prepare_for_next_order(unit);

				unit->status &= ~(UnitStatus::Disabled | UnitStatus::AttackMove);
				unit->status = status_saved | unit->status;
			}
		}
	}

	void orders_carrier_attack(Unit* unit) // 00465950
	{
		Unit* target = unit->order_target.unit;
		if (!target) {
			unit->order_to_idle();
		} else {
			if (!unit->can_attack_target(target, true)) {
				unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
			} else {
				u32 status_saved = (unit->status & UnitStatus::Disabled) | (unit->status & UnitStatus::AttackMove);
				// give orders to Stewards and Plasma Shells (through out_hangar_child)
				helper::set_interceptors_to_standby(unit);

				u8 new_order_id = 0;
				if (unit->id == UnitId::ProtossAccantorOld) {
					if (unit->main_order_id == OrderId::ReaverHold) {
						new_order_id = OrderId::ReaverHold;
					} else {
						new_order_id = OrderId::ReaverFight;
					}
				} else {
					if (unit->main_order_id == OrderId::HoldPositionSolarion) {
						new_order_id = OrderId::HoldPositionSolarion;
					} else {
						new_order_id = OrderId::CarrierFight;
					}
				}
				helper::prepend_order_targeting_unit(unit, target, new_order_id);
				prepare_for_next_order(unit);

				// restore the "disabled" and "holding position" status if they were lost
				unit->status &= ~(UnitStatus::Disabled | UnitStatus::AttackMove);
				unit->status = status_saved | unit->status;
			}
		}
	}

	void orders_attack_fixed_range(Unit* unit)
	{
		Unit* target;
		bool jump_to_77E48 = false;
		bool jump_to_77E99 = false;
		if (unit->unit_ai != nullptr)
		{
			unit->order_target.unit = helper::find_best_attack_target(unit); // 00477D13
		} else {
			// perform lots of checks, possibly in an attempt to auto-select a better target if the unit has choice
			helper::pick_new_target_if_needed(unit); // 00477D1F
		}

		target = unit->order_target.unit;
		if (!target)
		{
			unit->order_target.pt.x = target->position.x;
			unit->order_target.pt.y = target->position.y;
		}
		if (unit->is_frozen()
		|| !target
		||  target->status & UnitStatus::Invincible
		||  target->sprite->flags & SpriteFlags::Hidden
		||  helper::unit_cant_see_cloaked_target(unit, target))
		{
			jump_to_77E48 = true;
		} else {
			Unit* current_unit;
			bool jump_to_77DEF = false;
			if (unit->subunit->is_subunit()) // check if is not null AND is a subunit
			{
				current_unit = unit->subunit;
			} else {
				current_unit = unit;
			}

			if (unit->id == UnitId::ProtossSolarion)
			{
				jump_to_77E99 = true;
			} else if (!(unit->id == UnitId::ProtossAccantorOld)) {
				jump_to_77DEF = true;
			} else if (target->status & UnitStatus::InAir) {
				jump_to_77E48 = true;
			} else if (unit->has_path_to_unit(target)) {
				jump_to_77E99 = true;
			} else {
				jump_to_77E48 = true;
			}
			if (jump_to_77DEF)
			{
				if (helper::unit_can_infest(unit))
				{
					if (helper::is_infestable_unit(target))
					{
						jump_to_77E99 = true;
					} else {
						jump_to_77E48 = true;
					}
				} else {
					u8 current_weapon_id = current_unit->get_ground_weapon();
					if (target->status & UnitStatus::InAir)
					{
						current_weapon_id = current_unit->get_air_weapon();
					}
					if (current_unit->id == UnitId::ProtossClarion && bw::get_aise_value(current_unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
					{
						current_weapon_id = WeaponId::None;
					}
					if (current_weapon_id != WeaponId::None)
					{
						jump_to_77E99 = true;
					} else {
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
			} else if (unit->unit_ai) {
				unit->order_computer(OrderId::ComputerAI);
			} else {
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
				helper::get_weapon_begin_iscript(unit, IscriptAnimation::GndAttkRpt);
			} else if (!unit->unit_ai) {
				unit->order_to_idle();
			} else {
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
			} else {
				if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			if (!unit->can_attack_target(target, true))
			{
				unit->order_to(OrderId::Move, target->position.x, target->position.y);
			} else {
				bool bStopThere = false;
				if (unit->main_order_state == 0)
				{
					if (helper::move_to_target(target, unit))
					{
						unit->main_order_state = 1;
					} else {
						bStopThere = true;
					}
				}

				if (!bStopThere && unit->main_order_state == 1)
				{
					if (!unit->is_target_within_min_range(target, 4)
						&& unit->get_movable_state() != 2)
					{
						if (unit->is_target_within_min_range(target, 256)
							&& target->movement_flags & MovementFlags_Accelerating)							
						{
							// maybe trigger the acceleration of the infested terran, or something else related to distances?
							helper::move_to_collide(unit, target);
						} else {
							helper::move_towards(unit, target);
						}
						bStopThere = true;
					} else {
						helper::set_resource_target(unit);
						if (!unit->is_target_within_min_range(target, weapons_dat::outer_splash_radius[WeaponId::InfestedTerranSuicide]))
						{
							unit->order_to_idle();
							bStopThere = true;
						} else {
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
			if (unit->position.x != unit->move_target.pt.x
				|| unit->position.y != unit->move_target.pt.y)
			{
				bStopThere = true;
			} else {
				if (unit->status & UnitStatus::Unmovable)
				{
					u32 distance = bw::get_distance_fast(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
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
		u8 previous_main_order_id = unit->main_order_id;
		if (helper::attack_move_react_to_attack(unit, unit->main_order_id) && unit->main_order_timer == 0)
		{
			unit->main_order_timer = 0; // used to be 15
			if (helper::get_region_id(unit->position.x, unit->position.y) == helper::get_region_id(unit->order_target.pt.x, unit->order_target.pt.y))
			{
				helper::make_to_hold_position(unit);
				unit->order_to_idle();
			} else {
				bool jump_to_78EAE = false;
				if (unit->get_movable_state() != 0
				&& (unit->order_target.pt.x != unit->position.x || unit->order_target.pt.y != unit->position.y))
				{

					if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
					{
						jump_to_78EAE = true;
					} else if (helper::ai_is_unreachable_position(unit, (s32)unit->order_target.pt.x, (s32)unit->order_target.pt.y)) {
						helper::make_to_hold_position(unit);
					}
				} else {
					jump_to_78EAE = true;
				}

				if (jump_to_78EAE)
				{
					helper::attack_move_try_pick_target(unit, previous_main_order_id);
				}
			}
		}
	}

	void orders_attack_move(Unit* unit)
	{
		// Logically, would check if a target is in distance, and if that's the case (not returning 0),
		// stop the "move" part that follow to attack the unit met
		if (helper::attack_move_react_to_attack(unit, unit->main_order_id) != 0 && unit->main_order_timer == 0)
		{
			unit->main_order_timer = 0; // used to be 15
			if (unit->move_target.pt.x != unit->position.x
			||  unit->move_target.pt.y != unit->position.y)
			{
				// This function would check one last time if a target is in distance, and attack it if that's the case, else AttackMove end here
				helper::attack_move_try_pick_target(unit, unit->main_order_id);
			} else {
				unit->order_to_idle();
			}
		}
	}

	void orders_attack_unit(Unit* unit)
	{
		if (g_player_table[unit->player].type != PlayerType::Computer
		|| (unit->order_target.unit && units_dat::base_property[(unit->order_target.unit)->id] & UnitProperty::Building)
		|| !helper::ai_try_return_home(unit, 0))
		{
			// perform lots of checks, possibly in an attempt to auto-select a better target if the unit has choice
			helper::pick_new_target_if_needed(unit);

			// function perform tests to see if attack is possible and how it should be done and return true if attack can be done
			if (helper::attack_unit_main(unit))
			{
				if (unit->unit_ai)
				{
					helper::ai_attack_unit(unit);
				}
				helper::get_weapon_begin_iscript(unit, IscriptAnimation::GndAttkRpt);
			}
		}
	}

	// Ported from public GPTP to fix the Plasma Shell launch sound. -Pr0nogo
	void orders_strafe_unit2(Unit* unit) // 00465E00
	{
		if (unit->main_order_state <= 6) {
			if (unit->main_order_state == 0) {
				if (unit->remove_timer == 0 || unit->remove_timer > 90) {
					unit->remove_timer = 90;
				}

				unit->status |= UnitStatus::NoCollide;
				unit->main_order_timer = 7;
				if (unit->order_target.unit) {
					u8 rand_value = helper::randomize_short(0) & 1;
					helper::play_sound_from_unit(SoundId::Bullet_pTrFir00_wav + rand_value, unit, 1, 0);
					helper::move_to_target(unit, unit->order_target.unit);
					unit->main_order_state = 2;
				} else if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
					unit->main_order_state = 2;
				}
			} else if (unit->main_order_state == 2) {
				if (unit->main_order_timer == 0) {
					unit->status &= ~UnitStatus::NoCollide; // unset flag if set
					unit->main_order_state = 3;
				}
			}

			if (unit->main_order_state == 2 || unit->main_order_state == 3) {
				Unit* target = unit->order_target.unit;
				bool bNotEndThere;
				if (target) {
					if (unit->is_target_within_min_range(target, 256) && target->movement_flags & MovementFlags_Accelerating) {
						helper::move_to_collide(unit, target);
					} else {
						helper::move_towards(unit, target);
					}
					bNotEndThere = unit->is_target_within_min_range(target, weapons_dat::inner_splash_radius[units_dat::ground_weapon[unit->id]]);
				} else {
					bNotEndThere = (unit->get_movable_state() != MovableState_NotReachedDestination);
				}

				if (bNotEndThere) {
					Image* current_image;
					unit->order_target.pt.x = unit->position.x;
					unit->order_target.pt.y = unit->position.y;
					current_image = unit->sprite->images.head;
					while (current_image)
					{
						current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					unit->main_order_state = 6;
				}
			} else if (unit->main_order_state == 6) {
				if (unit->order_signal & 1)
				{
					unit->status |= UnitStatus::IsSelfDestructing;
					unit->remove();
				}
			}
		}
	}

	// Hooked to debug Stewards in a post-extender environment. -Pr0nogo
	void set_interceptors_to_standby(Unit* unit) { // 00465780
		for (Unit* out_hangar_child = unit->unit_with_hangar.out_hangar_child; out_hangar_child; out_hangar_child = out_hangar_child->hangar_unit.hangar_link.next) {
			out_hangar_child->sprite->elevation = unit->sprite->elevation - 1;
			Unit* target = unit->order_target.unit;
			u32 attack_order = units_dat::attack_unit_order[out_hangar_child->id];
			out_hangar_child->user_action_flags |= 1u;
			if (target) {
				helper::order(1, out_hangar_child, attack_order, target->position, target, UnitId::None);
			} else {
				helper::order(1, out_hangar_child, attack_order, {}, 0, UnitId::None);
			}
			helper::do_next_queued_order(out_hangar_child);
		}
	}

	// Hooked to debug Stewards in a post-extender environment. -Pr0nogo
	void orders_strafe_unit(Unit* unit) // 00465F60
	{
		if (unit->hangar_unit.parent && unit->hitpoints < units_dat::max_hitpoints[UnitId::ProtossSteward] << 8 >> 2) // Return if shields are below some threshold
		{
			helper::issue_order_targeting_nothing(unit, OrderId::Return);
		}
		else if (helper::return_interceptor_if_cannot_attack(unit))
		{
			Unit* target = unit->order_target.unit; // Compiler error avoidance
			switch (unit->main_order_state)
			{
			case 0:
				if (unit->hangar_unit.parent)
				{
					helper::play_sound_from_unit(616, unit, 1, 0);
					helper::get_next_strafe_position(unit, 3u, unit->order_target.pt);
					unit->main_order_timer = 15;
					unit->main_order_state = 1;
					goto LABEL_9;
				}
				helper::orders_self_destructing(unit);
				break;
			case 1:
			LABEL_9:
				if (!unit->main_order_timer)
				{
					unit->sprite->elevation += 2;
					unit->main_order_state = 3;
					goto LABEL_11;
				}
				break;
			case 3:
			LABEL_11:
				if (!target)
				{
					helper::get_weapon_iscript_init(unit, IscriptAnimation::GndAttkRpt);
				}
				else if (helper::is_target_visible(target, unit))
				{
					Unit* final_unit = unit->subunit;
					if (!final_unit || !(units_dat::base_property[final_unit->id] & UnitProperty::Subunit))
					{
						final_unit = unit;
					}

					u32 weapon_id = WeaponId::None;
					if (target->status & UnitStatus::InAir)
					{
						weapon_id = units_dat::air_weapon[final_unit->id];
					} else {
						if (!unit_id_is_burrowed_attacker(final_unit->id) || final_unit->status & UnitStatus::Burrowed)
						{
							weapon_id = units_dat::ground_weapon[final_unit->id];
						}
					}
					if (weapon_id != WeaponId::None)
					{
						u32 min_range = weapons_dat::min_range[weapon_id];
						if (!min_range || !helper::is_target_within_min_range(final_unit, min_range, target))
						{
							u32 max_weapon_range = helper::get_unit_max_weapon_range(unit, weapon_id);
							if (helper::is_target_within_min_movement_range(final_unit, target, max_weapon_range))
							{
								helper::get_weapon_iscript_init(unit, IscriptAnimation::GndAttkRpt);
							}
						}
					}
				}
				if (helper::is_target_within_min_movement_range(unit, 0, 50))
				{
					helper::get_next_strafe_position(unit, 1u, unit->order_target.pt);
					unit->main_order_state = 4;
				} else {
					helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
				}
				break;
			case 4:
				if (helper::is_distance_greater_than_halt_distance(unit, unit->move_target.pt.x, unit->move_target.pt.y, 50))
				{
					helper::get_next_strafe_position(unit, 2, unit->order_target.pt);
					unit->main_order_state = 5;
				}
				break;
			case 5:
				if (helper::is_distance_greater_than_halt_distance(unit, unit->move_target.pt.x, unit->move_target.pt.y, 50))
				{
					if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
					{
						unit->main_order_state = 3;
					}
				}
				break;
			default:
				return;
			}
		}
	}

	// Hooked to remove randomness. -Pr0nogo
	void get_next_strafe_position(Unit* unit, u8 strafe_type, Point16 position) // 00465D30
	{
		// TODO: replace randomness with some kind of consistent rotation.
		u32 rand_off_x = strafe_type * helper::rand_between(-127, 128, 11);
		u32 rand_off_y = strafe_type * helper::rand_between(-127, 128, 11);

		u32 rand_pos_x = rand_off_x + position.x;
		if (rand_pos_x < units_dat::unit_bounds[UnitId::ProtossSteward].x
		||  rand_pos_x >= *g_map_width_pixels - units_dat::unit_bounds[UnitId::ProtossSteward].w)
		{
			rand_off_x = -rand_off_x;
		}
		u32 rand_pos_y = rand_off_y + position.y;
		if (rand_pos_y < units_dat::unit_bounds[UnitId::ProtossSteward].y
		||  rand_pos_y >= *g_map_height_pixels - units_dat::unit_bounds[UnitId::ProtossSteward].h)
		{
			rand_off_y = -rand_off_y;
		}
		position.x += rand_off_x;
		position.y += rand_off_y;

		helper::fix_target_location(&position, UnitId::ProtossSteward);
		u32 pos_x = position.x;
		u32 pos_y = position.y;
		helper::set_move_target_xy(unit, position.x, position.y);
		if (pos_x != unit->next_target_waypoint.x || pos_y != unit->next_target_waypoint.y)
		{
			unit->next_target_waypoint.x = pos_x;
			unit->next_target_waypoint.y = pos_y;
		}
	}

	// Ported from GPTP to change the Solarion-Steward launch timer. -Pr0nogo
	// Used when unit->main_order_id is OrderId::Carrier, OrderId::CarrierFight or OrderId::HoldPositionSolarion
	void orders_carrier_idle(Unit* unit) // 004666A0
	{
		if (helper::order_hangar_unit(unit, 0, 6 * 32))
		{
			Unit* out_unit = release_fighter(unit);
			if (out_unit) {
				// Originally set shields to full.
				out_unit->sprite->elevation = unit->sprite->elevation- 1;
				out_unit->order_to(units_dat::attack_unit_order[UnitId::ProtossSteward], unit->order_target.unit);
				unit->order_queue_timer = 4; // Originally unit->main_order_timer = 7;
				bw::decloak_passive_proc(unit);
			}

		} else if (unit->unit_ai && unit->main_order_timer == 0) {
			unit->order_computer(OrderId::ComputerAI);
		}
	}

	// Ported from GPTP as a dependency of orders_carrier_idle. -Pr0nogo
	// Only used here, and making a modified copy could be useful, so hardcoded. Not using helpers, a copy should not reproduce that bad coding behavior. -GPTP
	Unit* release_fighter(Unit* unit) // 00466440
	{
		Unit* launched_unit = unit->unit_with_hangar.in_hangar_child;
		while (launched_unit && !(launched_unit->status & UnitStatus::Completed))
		{
			launched_unit = launched_unit->hangar_unit.hangar_link.prev;
		}

		if (launched_unit)
		{
			Unit* temp_unit = launched_unit->hangar_unit.hangar_link.prev;

			while (temp_unit) {
				if ((temp_unit->status & UnitStatus::Completed)
				&&	(temp_unit->hitpoints > launched_unit->hitpoints))
				{
					launched_unit = temp_unit;
				}
				temp_unit = temp_unit->hangar_unit.hangar_link.prev;
			}

			if (launched_unit->id == UnitId::ProtossSteward) {
				u32 max_hp = units_dat::max_hitpoints[UnitId::ProtossSteward];
				if (max_hp < 0)
				{
					max_hp += 1;
				}
				if (launched_unit->hitpoints < max_hp / 2)
				{
					launched_unit = 0;
				}
			}

			if (launched_unit) {
				const u32 Func_InitUnitTrapDoodad = 0x004E6490; // move to helpers if making a modified copy
				u32 x, y;

				if (launched_unit->id == UnitId::ProtossPlasmaShell) {
					const u32 Func_turnGraphic = 0x00495F20;	// move to helpers if making a modified copy
					u8 direction = unit->current_direction_1;

					x = unit->position.x + g_angle_distance[direction].x * 25 / 256;
					y = unit->position.y + g_angle_distance[direction].y * 25 / 256;

					//void turnGraphic(Unit* unit, u8 direction) @ 00495F20
					__asm {
						PUSHAD
						MOV EAX, launched_unit
						MOV BL, direction
						CALL Func_turnGraphic
						POPAD
					}

				} else {
					x = unit->position.x;
					y = unit->position.y;
				}

				bw::refresh_console();
				if (unit->unit_with_hangar.in_hangar_child == launched_unit)
				{
					unit->unit_with_hangar.in_hangar_child = launched_unit->hangar_unit.hangar_link.prev;
				}
				if (launched_unit->hangar_unit.hangar_link.prev)
				{
					(launched_unit->hangar_unit.hangar_link.prev)->hangar_unit.hangar_link.next = launched_unit->hangar_unit.hangar_link.next;
				}
				if (launched_unit->hangar_unit.hangar_link.next)
				{
					(launched_unit->hangar_unit.hangar_link.next)->hangar_unit.hangar_link.prev = launched_unit->hangar_unit.hangar_link.prev;
				}
				unit->unit_with_hangar.in_hangar_count--;
				launched_unit->hangar_unit.hangar_link.prev = unit->unit_with_hangar.out_hangar_child;
				launched_unit->hangar_unit.hangar_link.next = 0;
				if (unit->unit_with_hangar.out_hangar_child)
				{
					(unit->unit_with_hangar.out_hangar_child)->hangar_unit.hangar_link.next = launched_unit;
				}
				unit->unit_with_hangar.out_hangar_child = launched_unit;
				unit->unit_with_hangar.out_hangar_count++;

				bw::set_unit_position(launched_unit, x, y);
				//void showAndEnableUnit(Unit* unit) @ 004E6490
				__asm {
					PUSHAD
					MOV EDI, launched_unit
					CALL Func_InitUnitTrapDoodad
					POPAD
				}
				launched_unit->hangar_unit.is_outside_hangar = true;
			}
		}
		return launched_unit;
	}

	// Hooked to fix null sprite crash. -Pr0nogo
	s32 order_attack_behavior(Unit* unit) // 00478540
	{
		u32 status = unit->status;
		Unit* target_unit = unit->order_target.unit;
		if (unit->is_frozen()
		|| !target_unit || !target_unit->sprite // Nullcheck is new.
		||  target_unit->status & UnitStatus::Invincible
		||  target_unit->sprite->flags & SpriteFlags::Hidden
		||  helper::unit_can_see_cloaked_target(target_unit, unit))
		{
			goto cannot_attack;
		}

		// Required for "goto process_attack" to compile in C++20 mode :( -Veeq7
		b32 has_weapon;
		Unit* final_unit;

		if (helper::unit_is_carrier(unit))
		{
			goto process_attack;
		}
		if (!(target_unit->status & UnitStatus::InAir) && helper::unit_has_path_to_target(unit, target_unit))
		{
			goto process_attack;
		}

		if (unit->id == UnitId::ProtossAccantorOld) // Here to preserve old flow control.
		{
		cannot_attack:
			s32 result = 0;
			if (!target_unit || unit->status & UnitStatus::AttackMove)
			{
				helper::set_resource_target(unit);
				if (unit->order_queue_head)
				{
					unit->user_action_flags |= 1u;
					helper::do_next_queued_order(unit);
					result = 0;
				} else {
					if (unit->unit_ai)
					{
						helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
					} else {
						helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
					}
					result = 0;
				}
			} else {
				if (!unit->unit_ai)
				{
					helper::prepend_order_targeting_ground(OrderId::Move, target_unit->position.x, target_unit->position.y, unit); // Originally passed sprite position.
				}
				helper::to_idle(unit);
				result = 0;
			}
			return result;
		}

		// Originally handled Matraleth infest fail case, and AI Didact, neither of which apply anymore. -Pr0nogo

		has_weapon = false;
		final_unit = unit->subunit;
		if (!helper::unit_is_subunit(unit->subunit))
		{
			final_unit = unit;
		}
		if (target_unit->status & UnitStatus::InAir)
		{
			has_weapon = units_dat::air_weapon[final_unit->id] != WeaponId::None;
		} else {
			has_weapon = units_dat::ground_weapon[final_unit->id] != WeaponId::None;
		}
		if (!has_weapon)
		{
			goto cannot_attack;
		}

	process_attack:
		u16 target_x = target_unit->position.x; // Originally checked for sprite position.
		u16 target_y = target_unit->position.y; // ^
		unit->order_target.pt.x = target_x;
		unit->order_target.pt.y = target_y;
		if (target_x != unit->next_target_waypoint.x || target_y != unit->next_target_waypoint.y)
		{
			unit->next_target_waypoint.x = target_x;
			unit->next_target_waypoint.y = target_y;
		}
		if (!unit->main_order_state)
		{
			if (!target_unit)
			{
				unit->main_order_state = 1;
				return 1;
			}
			if (helper::is_target_visible(unit, target_unit))
			{
				final_unit = unit->subunit;
				if (!final_unit || !(units_dat::base_property[final_unit->id] & UnitProperty::Subunit))
				{
					final_unit = unit;
				}

				u32 weapon_id = WeaponId::None;
				if (target_unit->status & UnitStatus::InAir)
				{
					weapon_id = units_dat::air_weapon[final_unit->id];
				} else {
					if (!unit_id_is_burrowed_attacker(final_unit->id) || final_unit->status & UnitStatus::Burrowed)
					{
						weapon_id = units_dat::ground_weapon[final_unit->id];
					} else {
						weapon_id = WeaponId::None;
					}
				}
				if (weapon_id != WeaponId::None)
				{
					u32 min_range = weapons_dat::min_range[weapon_id];
					if (!min_range || !helper::is_target_within_min_range(final_unit, min_range, target_unit))
					{
						u32 max_range = helper::get_unit_max_weapon_range(unit, weapon_id);
						if (helper::is_target_within_min_movement_range(final_unit, target_unit, max_range))
						{
							unit->main_order_state = 1;
							return 1;
						}
					}
				}
			}
			if (helper::move_to_target(unit->order_target.unit, unit))
			{
				unit->main_order_state = 1;
				return 1;
			}
			return 0;
		}
		if (helper::is_unit_target_out_of_max_range(unit, target_unit))
		{
			if (unit->unit_ai)
			{
				if (helper::is_unreachable(unit->order_target.unit, unit) || helper::is_unit_movable(unit) == 2)
				{
					if (helper::ai_update_attack_target(unit, 0, 1, 0))
					{
						return 0;
					}
					helper::make_to_hold_position(unit);
					helper::to_idle(unit);
					return 0;
				}
			} else {
				if (unit->status & UnitStatus::AttackMove && !(unit->status & (UnitStatus::Disabled | UnitStatus::InAir))
				&&  unit->order_target.unit->status & UnitStatus::InAir || helper::is_unit_movable(unit) == 2)
				{
					helper::make_to_hold_position(unit);
					helper::to_idle(unit);
					return 0;
				}
			}
			if (helper::should_stay_at_range(unit))
			{
				helper::move_towards(unit, target_unit);
				return 0;
			}
			helper::move_for_melee_range(target_unit, unit);
			return 0;
		}
		if (unit_id_is_melee_air_unit(unit->id))
		{
			if (unit->id != UnitId::ZergGorgrokor || !(target_unit->status & UnitStatus::InAir)) {
				helper::move_for_melee_range(unit->order_target.unit, unit);
			}
		} else {
			helper::set_resource_target(unit);
		}
		return 1;
	}

	// Hooked to nullcheck sprite (commonly crashed with Lakizilisks) -Pr0nogo
	b32 attack_move_react_to_attack(Unit* unit, u32 order_id) // 00478370
	{
		if (!unit->main_order_state)
		{
			u32 pt_x = unit->order_target.pt.x;
			u32 pt_y = unit->order_target.pt.y;
			bool move_target_set = helper::set_move_target_xy(unit, pt_x, pt_y);
			if (pt_x != unit->next_target_waypoint.x || pt_y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = pt_x;
				unit->next_target_waypoint.y = pt_y;
			}
			if (!move_target_set)
			{
				return 0;
			}
			unit->main_order_state = 1;
		}

		Unit* auto_target = unit->auto_target_unit;
		if (auto_target)
		{
			if (auto_target->is_dead()) return 0; // New

			u8 target_player = auto_target->player == 11 ? auto_target->sprite->player : auto_target->player;
			if (!bw::is_allied_to(unit->player, target_player)
			&& !(auto_target->sprite->flags & SpriteFlags::Hidden)
			&& !(auto_target->status & UnitStatus::Invincible))
			{
				if (!helper::unit_can_see_cloaked_target(auto_target, unit))
				{
					if (unit->unit_ai)
					{
						return helper::ai_update_attack_target(unit, 0, 0, 0) == 0;
					}
					helper::set_resource_target(unit);
					helper::prepend_order(unit, order_id, unit->order_target.pt, unit->order_target.unit);
					helper::insert_order_targeting_unit_before(unit->auto_target_unit, units_dat::attack_unit_order[unit->id], unit->order_queue_head, unit);
					unit->auto_target_unit = 0;
					helper::force_order_done(unit);
					helper::set_flag_attack_move(unit);
					return 0;
				}
			}
		}
		return 1;
	}

	// Hooked to fix Optectons when they have edge targets! -Pr0nogo
	void order_attack_move_try_pick_target(Unit* unit, u8 order_id) // 00477820
	{
		if (helper::get_target_acquisition_range_upgrade(unit))
		{
			if (unit->unit_ai)
			{
				helper::ai_update_attack_target(unit, 0, 0, 0);
			} else {
				Unit* target = helper::find_best_attack_target(unit);
				if (target)
				{
					if (unit->id == UnitId::ProtossOptecton && target->status & UnitStatus::InAir) {
						return;
					}
					helper::set_resource_target(unit);
					helper::prepend_order(unit, order_id, unit->order_target.pt, unit->order_target.unit);
					helper::insert_order_before(unit, unit->order_queue_head, units_dat::attack_unit_order[unit->id],
											    target->position.x, target->position.y, target, UnitId::None);
					helper::force_order_done(unit);
					target = 0;
					unit->status |= UnitStatus::AttackMove;
					Unit* subunit = unit->subunit;
					if (subunit)
					{
						subunit->status |= UnitStatus::AttackMove;
					}
				}
			}
		}
	}

	// Hooked to protect against nullsprite crashes. -Pr0nogo
	void orders_turret_attack(Unit* unit) // 00477980
	{
		helper::pick_new_target_if_needed(unit);
		Unit* target = unit->order_target.unit;
		if (target && !target->is_dead()) { // is_dead is new
			unit->order_target.pt.x = target->sprite->position.x;
			unit->order_target.pt.y = target->sprite->position.y;
		}

		bool has_weapon_id = false;
		Unit* final_unit = unit->subunit;
		Order* tail = 0;
		u32 order_id = 0;
		u32 last_order = 0;

		if (unit->is_frozen() || !target || target->is_dead()) // is_dead is new
		{
			goto cannot_attack;
		}
		if (target->status & UnitStatus::Invincible || target->sprite->flags & SpriteFlags::Hidden || helper::unit_can_see_cloaked_target(target, unit))
		{
			goto cannot_attack;
		}

		if (!helper::unit_is_subunit(unit->subunit)) {
			final_unit = unit;
		}

		if (!helper::unit_is_carrier(unit)) {
			if (helper::unit_is_reaver(unit)) {
				if (target->status & UnitStatus::InAir || !helper::unit_has_path_to_target(unit, target))
				{
					goto cannot_attack;
				}
			} else {
				if (target->status & UnitStatus::InAir) {
					has_weapon_id = final_unit->get_air_weapon() != WeaponId::None;
				} else {
					has_weapon_id = final_unit->get_ground_weapon() != WeaponId::None;
				}

				if (!has_weapon_id)
				{
				cannot_attack:
					order_id = unit->main_order_id;
					unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
					if (order_id) {
						while (1)
						{
							tail = unit->order_queue_tail;
							if (!tail) {
								break;
							}
							last_order = tail->order_id;
							if (!orders_dat::can_be_interrupted[last_order] && last_order != OrderId::TurretGuard) {
								break;
							}
							helper::remove_order_from_unit_queue(tail, unit);
						}
						helper::perform_another_order(OrderId::TurretGuard, 0, unit, UnitId::None, {}, 0);
					}
					helper::do_next_queued_order(unit);
					return;
				}
			}
		}
		if (!helper::is_target_visible(unit, unit->order_target.unit))
		{
			goto cannot_attack;
		}

		Unit* subunit = unit->subunit;
		u32 siege_mode_weapon = WeaponId::None;
		u32 tank_mode_weapon = WeaponId::None;
		u32 range = 0;
		u32 range2 = 0;
		u32 final_weapon = WeaponId::None;
		u32 final_range = 0;
		u32 max_range = 0;

		if (unit->id != UnitId::TerranPhalanxTankModeTurret || !subunit->unit_ai)
		{
			goto test_range;
		}

		siege_mode_weapon = units_dat::ground_weapon[UnitId::TerranPhalanxSiegeModeTurret];
		tank_mode_weapon = units_dat::ground_weapon[UnitId::TerranPhalanxTankModeTurret];
		range = helper::get_unit_max_weapon_range(unit, siege_mode_weapon);
		range2 = helper::get_unit_max_weapon_range(unit, tank_mode_weapon);
		if (helper::is_unit_visible(unit)
		&&  helper::is_target_within_min_movement_range(subunit, unit->order_target.unit, range)
		&& !helper::is_target_within_min_movement_range(subunit, unit->order_target.unit, range2)
		&& !helper::unit_is_critter(unit->order_target.unit))
		{
			helper::issue_order_targeting_nothing(unit, OrderId::Nothing2);
			helper::order_targeting_nothing(units_dat::attack_unit_order[UnitId::TerranPhalanxSiegeModeTurret], unit, 0);
			helper::issue_order_targeting_nothing(subunit, OrderId::SiegeMode);
			helper::issue_queued_order_target(target, subunit, units_dat::attack_unit_order[UnitId::TerranPhalanxSiegeMode], 0);
		} else {
		test_range:
			if (!target) {
				goto move_or_repeat;
			}
			if (helper::is_target_visible(unit, target)) {
				if (!subunit || !(units_dat::base_property[subunit->id] & UnitProperty::Subunit)) {
					subunit = unit;
				}
				if (target->status & UnitStatus::InAir) {
					final_weapon = units_dat::air_weapon[subunit->id];
				} else {
					if (subunit->id != UnitId::ZergLakizilisk || subunit->status & UnitStatus::Burrowed)
					{
						final_weapon = units_dat::ground_weapon[subunit->id];
					} else {
						final_weapon = WeaponId::None;
					}
				}

				if (final_weapon != WeaponId::None) {
					final_range = weapons_dat::min_range[final_weapon];
					if (!final_range || !helper::is_target_within_min_range(subunit, final_range, target))
					{
						max_range = helper::get_unit_max_weapon_range(unit, final_weapon);
						if (helper::is_target_within_min_movement_range(subunit, target, max_range))
						{
							goto move_or_repeat;
						}
					}
				}
			}
			if (subunit->main_order_id != units_dat::attack_unit_order[subunit->id]
			||  helper::get_right_click_action_order(subunit) == OrderId::PlayerGuard)
			{
				helper::issue_order_targeting_nothing(unit, OrderId::TurretGuard);
			} else {
			move_or_repeat:
				u16 tgt_x = unit->order_target.pt.x;
				u16 tgt_y = unit->order_target.pt.y;
				if (tgt_x != unit->next_target_waypoint.x || tgt_y != unit->next_target_waypoint.y)
				{
					unit->next_target_waypoint.x = tgt_x;
					unit->next_target_waypoint.y = tgt_y;
				}
				if (!(unit->subunit->movement_flags & MovementFlags_Accelerating)) {
					helper::get_weapon_iscript_init(unit, IscriptAnimation::GndAttkRpt);
				}
			}
		}
	}
}