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
		range = 32 * mine->get_seek_range();
		return
			scbw::UnitFinder::get_nearest_target(
				mine->get_x() - range, mine->get_y() - range,
				mine->get_x() + range, mine->get_y() + range,
				mine, [&](Unit* target)
		{
			// Don't attack friendly or allied units
			if (!mine->is_target_enemy(target))
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
		if (unit->main_order_state == 0)
		{
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
					helper::burrowGeneric(unit); // related to movement
					helper::instantCloak(unit); // related to cloaking/stealth
				}
				unit->main_order_state = 3;
			}
		}

		if (unit->main_order_state == 3)
		{
			Unit* target = helper::SpiderMine_AcquireTarget(unit);

			if (target != nullptr)
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
				unit->sprite->elevationLevel = units_dat::Elevation[unit->id];
				unit->status &= ~UnitStatus::NoCollide;
				if (unit->order_target.unit != nullptr)
				{
					// probably trigger movement/acceleration of unit toward the target
					helper::moveToCollide(unit, unit->order_target.unit);

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
			bool bIsCompletelyInRange = false;
			bool bEndThere = false;
			helper::unburrowGeneric(unit); // related to movement, similar to function_004E9A30
			if (unit->order_target.unit != nullptr)
			{
				if (unit->is_target_within_min_range(unit->order_target.unit, MineSeekRange))
				{
					// probably trigger movement/acceleration of unit
					// toward the target
					helper::moveToCollide(unit, unit->order_target.unit);
					if (unit->is_target_within_min_range(unit->order_target.unit, MineDetonateRange))
					{
						bIsCompletelyInRange = true; // jump to 00463F22
					}
					else if (unit->get_movable_state() != 2) // unit not unmovable
					{
						bEndThere = true;
					}
				}
			}
			if (!bEndThere && !bIsCompletelyInRange) // 00463F14
			{
				helper::set_resource_target(unit);
				unit->main_order_state = 1;
			}

			if (bIsCompletelyInRange) // 00463F22
			{
				Image* current_image;
				unit->order_target.unit = nullptr;
				unit->order_target.pt.x = unit->sprite->position.x;
				unit->order_target.pt.y = unit->sprite->position.y;
				current_image = unit->sprite->images.head;
				IscriptAnimation::Enum anim = IscriptAnimation::SpecialState1;
				if (unit->id == UnitId::TerranLobotomyMine)
				{
					anim = IscriptAnimation::Unused1;
				}
				while (current_image != nullptr)
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
		if (unit->main_order_state)
		{
			if (helper::techUseAllowed(unit, 3u, unit->player)
				&& ((v11 = v1->sprite, v1->move_target.pt.x != v11->position.x)
					|| v1->move_target.pt.y != v11->position.y
					|| ((v1->status & UnitStatus::Unmovable) != 0) != 1))
			{
				if ((helper::get_distance_fast(v1->halt.x, v1->order_target.pt.x << 8, v1->halt.y, v1->order_target.pt.y << 8) & 0xFFFFFF00) <= 0x1400)
				{
					helper::orders_HoldPosSuicidal(v1);
					v12 = v1->order_target.pt.y;
					a2.x = v1->order_target.pt.x;
					a2.y = v12;
					helper::fix_target_location(&a2, 0xDu);
					v13 = helper::create_unit(0xDu, a2.x, a2.y, (unsigned __int8)v1->player);
					v14 = v13;
					if (v13)
					{
						helper::update_unit_stats_and_finish_building(v13);
						helper::update_unit_strength_and_apply_default_orders(v14);
						helper::turnUnit(v14, v1->current_direction_1);
						v1->mine_carrier.mine_count -= 1;
					}
					else
					{
						helper::display_last_network_error_for_player(v1->player);
					}
					helper::to_idle(v1);
				}
				else if (v1->unit_ai)
				{
					if (helper::is_unit_movable(v1))
						helper::to_idle(v1);
				}
			}
			else if (v1->order_queue_head)
			{
				v1->user_action_flags |= 1u;
				helper::orders_nothing_2(v1);
			}
			else if (v1->unit_ai)
			{
				helper::issue_order_targeting_nothing(v1, 0x9Cu);
			}
			else
			{
				helper::issue_order_targeting_nothing(v1, units_dat::ReturnToIdleOrder[(unsigned __int16)v1->id]);
			}
		}
		else
		{
			target = unit->order_target.unit;
			if (target)
			{
				v3 = helper::move_to_target(target, unit);
				v4 = target->sprite;
				v5 = v4->position.y;
				v6 = v4->position.x;
				if (v6 != v1->next_target_waypoint.x || v5 != v1->next_target_waypoint.y)
				{
					v1->next_target_waypoint.x = v6;
					v1->next_target_waypoint.y = v5;
				}
				if (v3)
				{
					v7 = v1->order_target.unit;
					v1->order_target.pt.x = v7->sprite->position.x;
					v1->order_target.pt.y = v7->sprite->position.y;
					v1->main_order_state = 1;
				}
			}
			else
			{
				v8 = unit->order_target.pt.y;
				v9 = unit->order_target.pt.x;
				ByteOperations::SetLowByte(v10, helper::set_move_target_xy(v9, v8, unit));
				if (v9 != v1->next_target_waypoint.x || v8 != v1->next_target_waypoint.y)
				{
					v1->next_target_waypoint.x = v9;
					v1->next_target_waypoint.y = v8;
				}
				if (v10)
				{
					v1->main_order_state = 1;
				}
			}
		}
	}

	// Hooked to try to fix mine burrowing - Pr0nogo
	char burrowGeneric(Unit* unit)
	{ // 004E9A30
		Unit* unitVar = unit;
		helper::toggle_unit_path(unit);

		Sprite* unitSprite = unitVar->sprite;
		char unitPathing = unitVar->pathing_flags;
		unitVar->movement_state = 0;
		unitVar->pathing_flags = (unitPathing ^ (unitSprite->elevationLevel < 12u)) & 1 ^ unitPathing;
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
