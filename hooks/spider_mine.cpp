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
		s32 range;
		// Don't search for a target if the mine is under a Disruption Web
		if (mine->status & UnitStatus::CanNotAttack)
		{
			return 0;
		}
		range = 32 * mine->get_seek_range();
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
			// Don't attack air units / buildings
			if (target->status & (UnitStatus::InAir | UnitStatus::GroundedBuilding)) {
				return false;
			}
			// Don't attack hovering units
			if (units_dat::movement_flags[target->id] == MovementFlags::HoverUnit) {
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
				}
				else
				{
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
				while (current_image)
				{
					current_image->play_iscript_animation(anim);
					current_image = current_image->link.next;
				}
				unit->main_order_state = 6;
			}
		}

		// unlike state 5 with state 4, state 6 can be chained from
		// state 5, so no else here
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
		if (unit->main_order_state)
		{
			Sprite* unit_sprite = unit->sprite;
			if  (helper::tech_use_allowed(unit, 3u, unit->player)
			&&  (unit->move_target.pt.x != unit_sprite->position.x) || unit->move_target.pt.y != unit_sprite->position.y || ((unit->status & UnitStatus::Unmovable) != 0) != 1)
			{
				if ((helper::get_distance_fast(unit->halt.x, unit->order_target.pt.x << 8, unit->halt.y, unit->order_target.pt.y << 8) & 0xFFFFFF00) <= 0x1400) {
					helper::orders_hold_pos_suicidal(unit);
					Point16 target_pos = {unit->order_target.pt.x, unit->order_target.pt.y};
					helper::fix_target_location(&target_pos, UnitId::TerranSpiderMine);
					Unit* mine = helper::create_unit(UnitId::TerranSpiderMine, target_pos.x, target_pos.y, unit->player);
					if (mine)
					{
						helper::update_unit_stats_and_finish_building(mine);
						helper::update_unit_strength_and_apply_default_orders(mine);
						mine->sprite->player = unit->sprite->player;
						helper::turn_unit(mine, unit->current_direction_1);
						unit->mine_carrier.mine_count -= 1;
					}
					else
					{
						helper::display_last_network_error_for_player(unit->player);
					}
					helper::to_idle(unit);
				}
				else if (unit->unit_ai)
				{
					if (helper::is_unit_movable(unit))
					{
						helper::to_idle(unit);
					}
				}
			}
			else if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				helper::orders_nothing_2(unit);
			}
			else if (unit->unit_ai)
			{
				helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
			}
			else
			{
				helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
			}
		}
		else
		{
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
			}
			else
			{
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
	char burrow_generic(Unit* unit) // 004E9A30
	{
		Unit* unit_var = unit;
		helper::toggle_unit_path(unit);

		Sprite* unit_sprite = unit_var->sprite;
		char unitPathing = unit_var->pathing_flags;
		unit_var->movement_state = 0;
		unit_var->pathing_flags = (unitPathing ^ (unit_sprite->elevation < 12u)) & 1 ^ unitPathing;
		unit_var->status |= UnitStatus::CloakingForFree | UnitStatus::Burrowed;
		unit_sprite->flags |= 0x40u; // image is hidden

		Sprite* old_sprite = unit_var->sprite;
		Sprite* new_sprite = 0;
		if (unit_var->id == UnitId::ZergLakizilisk)
		{
			// Warning, tism detected
			// ByteOperations::SetLowByte(new_sprite, switch_sprite_id(old_sprite, SpriteId::White_Circle_Invisible));
			unit_var->sprite->elevation = 1;
		}
		else
		{
			helper::switch_sprite_id(old_sprite, SpriteId::White_Circle_Invisible);
			new_sprite = unit_var->sprite;
			new_sprite->elevation = 1;
		}
		return (char)new_sprite;
	}

}
