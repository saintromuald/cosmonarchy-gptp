#include "spider_mine.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "byte_operations_utils.h"

namespace hooks
{
	// Return the best target for the mine. If there is no suitable target, return 0 instead.
	Unit* find_best_mine_target(Unit* mine) // 00441270
	{
		// Don't search for a target if the mine is under a Disruption Field
		if (mine->status & UnitStatus::CanNotAttack)
		{
			return 0;
		}

		u32 range = 32 * mine->get_seek_range();
		return scbw::UnitFinder::get_nearest_target(
				mine->get_x() - range, mine->get_y() - range,
				mine->get_x() + range, mine->get_y() + range,
				mine, [&](Unit* target)
		{
			// Don't attack friendly or allied units
			if (!mine->is_target_enemy(target)) {
				return false;
			}
			// Don't attack invincible units
			if (target->status & UnitStatus::Invincible) {
				return false;
			}
			// Don't attack edge units / buildings
			if (target->status & (UnitStatus::InAir | UnitStatus::GroundedBuilding)) {
				return false;
			}
			// Don't attack hovering units
			if (units_dat::movement_flags[target->id] == MovementFlags_HoverUnit) {
				return false;
			}
			if (!target->is_visible_to(mine->player)) {
				return false;
			}
			if (scbw::get_distance_fast(mine->position, target->position) > range) {
				return false;
			}
			return true;
		});
	}

	void orders_terran_mine(Unit* unit) // 00463DF0
	{
		if (unit->main_order_state == 0)
		{
			helper::play_sound_from_unit(SoundId::SpiderMineDeploy01, unit, 1, 0);
			unit->ground_weapon_cooldown = MineBurrowTime;
			unit->main_order_state = 1;
		}

		if (unit->main_order_state == 1)
		{
			if (unit->ground_weapon_cooldown == 0)
			{
				unit->sprite->play_iscript_animation(IscriptAnimation::Burrow, true);
				unit->status |= UnitStatus::NoCollide;
				unit->main_order_state = 2;
			}
		}

		if (unit->main_order_state == 2)
		{
			if (unit->order_signal & 4)
			{
				if (!(unit->status & UnitStatus::IsHallucination))
				{
					helper::burrow_generic(unit);
					helper::instant_cloak(unit);
				}
				unit->main_order_state = 3;
			}
		}

		if (unit->main_order_state == 3)
		{
			Unit* target = helper::spider_mine_acquire_target(unit);

			if (target)
			{
				unit->order_target.unit = target;
				unit->sprite->play_iscript_animation(IscriptAnimation::UnBurrow, true);
				unit->sprite->flags &= ~SpriteFlags::Burrowed;
				// might have been some cloaking order before
				unit->set_secondary_order(OrderId::Nothing2);
				unit->main_order_state = 4;
			}
		}

		if (unit->main_order_state == 4)
		{
			if (unit->order_signal & 4)
			{
				unit->sprite->elevation = units_dat::elevation[unit->id];
				unit->status &= ~UnitStatus::NoCollide;
				if (unit->order_target.unit)
				{
					// probably trigger movement/acceleration of unit toward the target
					helper::move_to_collide(unit, unit->order_target.unit);

					unit->main_order_state = 5;
				} else {
					unit->main_order_state = 1;
				}
			}
		}

		// unlike the previous states, going directly from state 4 to 5 in the same frame is impossible
		else if (unit->main_order_state == 5)
		{
			bool is_completely_in_range = false;
			bool not_movable = false;
			helper::unburrow_generic(unit);
			if (unit->order_target.unit)
			{
				if (unit->is_target_within_min_range(unit->order_target.unit, MineSeekRange))
				{
					// probably trigger movement/acceleration of unit
					// toward the target
					helper::move_to_collide(unit, unit->order_target.unit);
					if (unit->is_target_within_min_range(unit->order_target.unit, MineDetonateRange))
					{
						is_completely_in_range = true; // jump to 00463F22
					}
					else if (unit->get_movable_state() != 2) // unit not unmovable
					{
						not_movable = true;
					}
				}
			}
			if (!not_movable && !is_completely_in_range) // 00463F14
			{
				helper::set_resource_target(unit);
				unit->main_order_state = 1;
			}

			if (is_completely_in_range) // 00463F22
			{
				Image* current_image;
				unit->order_target.unit = 0;
				unit->order_target.pt.x = unit->sprite->position.x;
				unit->order_target.pt.y = unit->sprite->position.y;
				current_image = unit->sprite->images.head;
				IscriptAnimation::Enum anim = IscriptAnimation::SpecialState1;
				if (unit->id == UnitId::TerranLobotomyMine) {
					// This is really stupid, but it works, because the weapon is actually hardcoded within iscript. -Pr0nogo
					anim = IscriptAnimation::Unused2;
				}
				while (current_image)
				{
					current_image->play_iscript_animation(anim);
					current_image = current_image->link.next;
				}
				unit->main_order_state = 6;
			}
		}

		// Unlike state 5 with state 4, state 6 can be chained from state 5, so no else here
		if (unit->main_order_state == 6)
		{
			if (unit->order_signal & 1)
			{
				unit->remove();
			}
		}
	}

	// Not tested yet. -Pr0nogo
	void orders_place_mine(Unit* unit) // 00464FD0
	{
		if (!unit->mine_carrier.mine_count) {
			helper::order_return_to_idle(unit);
			return;
		}

		if (unit->main_order_state)
		{
			Sprite* unit_sprite = unit->sprite;
			if  (helper::tech_use_allowed(unit, TechId::SpiderMines, unit->player)
			&&  (unit->move_target.pt.x != unit_sprite->position.x) || unit->move_target.pt.y != unit_sprite->position.y || ((unit->status & UnitStatus::Unmovable) != 0) != 1)
			{
				if ((helper::get_distance_fast(unit->halt.x, unit->order_target.pt.x << 8, unit->halt.y, unit->order_target.pt.y << 8) & 0xFFFFFF00) <= 5120) {
					helper::orders_hold_pos_suicidal(unit);
					Point16 target_pos = {unit->order_target.pt.x, unit->order_target.pt.y};
					u32 mine_id = scbw::get_trap_id(unit);
					helper::fix_target_location(&target_pos, mine_id);
					Unit* mine = helper::create_unit(mine_id, target_pos.x, target_pos.y, unit->player);
					if (mine)
					{
						helper::update_unit_stats_and_finish_building(mine);
						helper::update_unit_strength_and_apply_default_orders(mine);
						mine->sprite->player = unit->sprite->player;
						helper::turn_unit(mine, unit->current_direction_1);
						unit->mine_carrier.mine_count -= 1;
					} else {
						helper::display_last_network_error_for_player(unit->player);
					}
					helper::to_idle(unit);
				} else if (unit->unit_ai) {
					if (helper::is_unit_movable(unit)) {
						helper::to_idle(unit);
					}
				}
			} else if (unit->order_queue_head) {
				unit->user_action_flags |= 1;
				helper::do_next_queued_order(unit);
			} else if (unit->unit_ai) {
				helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
			}
		} else {
			Unit* target = unit->order_target.unit;
			if (target)
			{
				bool can_move = helper::move_to_target(target, unit);
				Sprite* target_sprite = target->sprite;
				s32 sprite_pos_x = target_sprite->position.x;
				s32 sprite_pos_y = target_sprite->position.y;
				if (sprite_pos_x != unit->next_target_waypoint.x || sprite_pos_y != unit->next_target_waypoint.y)
				{
					unit->next_target_waypoint.x = sprite_pos_x;
					unit->next_target_waypoint.y = sprite_pos_y;
				}
				if (can_move)
				{
					unit->order_target.pt.x = target->sprite->position.x;
					unit->order_target.pt.y = target->sprite->position.y;
					unit->main_order_state = 1;
				}
			} else {
				s32 sprite_pos_x = unit->order_target.pt.x;
				s32 sprite_pos_y = unit->order_target.pt.y;
				bool move_target = helper::set_move_target_xy(unit, sprite_pos_x, sprite_pos_y);
				if (sprite_pos_x != unit->next_target_waypoint.x || sprite_pos_y != unit->next_target_waypoint.y)
				{
					unit->next_target_waypoint.x = sprite_pos_x;
					unit->next_target_waypoint.y = sprite_pos_y;
				}
				if (move_target)
				{
					unit->main_order_state = 1;
				}
			}
		}
	}

	// Hooked to try to fix mine burrowing - Pr0nogo
	void burrow_generic(Unit* unit) // 004E9A30
	{
		helper::toggle_unit_path(unit);

		char pathing_flags = unit->pathing_flags;
		unit->movement_state = 0;
		unit->pathing_flags = (pathing_flags ^ (unit->sprite->elevation < 12)) & 1 ^ pathing_flags;
		unit->status |= (UnitStatus::CloakingForFree | UnitStatus::Burrowed);
		unit->sprite->flags |= SpriteFlags::Burrowed;
		Sprite* old_sprite = unit->sprite;
		Sprite* new_sprite = 0;
		if (unit->id == UnitId::ZergLakizilisk)
		{
			helper::switch_sprite_id(old_sprite, SpriteId::White_Circle2);
			unit->sprite->elevation = 1;
		} else {
			helper::switch_sprite_id(old_sprite, SpriteId::White_Circle_Invisible);
			unit->sprite->elevation = 1;
		}
	}

}
