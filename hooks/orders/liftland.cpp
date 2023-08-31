#include "larva_creep_spawn.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\unit_speed.h"
#include "hooks/load_unload_proc.h"

namespace hooks
{
	void orders_building_land(Unit* unit) { // 00464360
		Target unit_target = {};
		unit_target.unit = unit->order_target.unit;
		unit_target.pt = unit->order_target.pt;
		Point16 coords = {};
		coords.x = 0;
		coords.y = 0;

		if (unit->status & UnitStatus::GroundedBuilding) {
			unit->user_action_flags |= 1;
			if (unit->main_order_id != OrderId::Die) {
				bool break_loop = false;
				while (unit->order_queue_tail && !break_loop) {
					break_loop = !orders_dat::can_be_interrupted[unit->order_queue_tail->order_id] && (unit->order_queue_tail->order_id != OrderId::BuildingLiftoff);
					if (!break_loop) {
						helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
					}
				}
				unit->perform_another_order(OrderId::BuildingLiftoff, coords.x, coords.y, 0, 0);
			}
			prepare_for_next_order(unit);
			if (unit->main_order_id != OrderId::Die) {
				while (unit->order_queue_tail && unit->order_queue_tail->order_id == OrderId::BuildingLand) {
					helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
				}
				unit->perform_another_order(OrderId::BuildingLand, unit_target.pt.x, unit_target.pt.y, unit_target.unit, UnitId::None);
			}
			unit->order(units_dat::return_to_idle_order[unit->id], 0, 0, 0, UnitId::None, false);
		} else if (unit->main_order_state == 0) {
			// Plan vertical movement toward ground
			helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y - unit->sprite->images.tail->vertical_offset);
			unit->main_order_state = 1;
		} else if (unit->main_order_state == 1) {
			if (unit->get_movable_state() != 0) {
				s32 x = units_dat::building_dimensions[unit->id].x;
				s32 y = units_dat::building_dimensions[unit->id].y;
				if (x < 0) {
					x += 1; if (y < 0) y += 1;
				}

				x /= 2; y /= 2;
				x = unit->order_target.pt.x - x;
				y = unit->order_target.pt.y - y;
				if (x < 0) {
					x += 31; if (y < 0) y += 31;
				}
				x /= 32; y /= 32;

				// Check for units occupying the landing zone before the final vertical movement started.
				if (helper::can_build_on_tile_location(unit, unit->player, x, y, unit->id, 0, 0, 1, 0)) {
					// Play "landing sequence interrupted".
					helper::play_building_land_sound(unit);

					if (unit->order_queue_head
					&&	unit->order_queue_head->order_id == OrderId::PlaceAddon)
					{
						helper::remove_order_from_unit_queue(unit->order_queue_head, unit);
					}
					unit->order_to_idle();
				} else {
				//	helper::turn_unit(unit, 128); -- Causes teleportation when landing diagonally while slowed! -Pr0nogo

					// Use an add_fog_mask() function after some coordinates calculations
					helper::set_building_tile_flag(unit, unit->order_target.pt.x, unit->order_target.pt.y);
					unit->building.landing_timer = 1;

					// Plan movement again after latest updates
					helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
					if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW)
					{
						unit->sprite->images.tail->flags |= ImageFlags::DontUpdate;
					}
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->flingy_top_speed = 256;
					helper::set_unit_movement_speed(unit, 256);
					unit->main_order_state = 2;
					helper::play_sound_from_unit(SoundId::TerranBuildingLand01, unit, 1, 0);
				}
			}
		} else if (unit->main_order_state == 2) {
			UnitExt* unit_ext = get_unit_ext(unit);
			unit_ext->timers[TimerLiftLandDelay] = 52 + 1;
			bool stop_there = false;
			if (!helper::is_unit_positions_2_equal(unit)) {
				stop_there = ((unit->velocity_direction_1 - scbw::get_angle(unit->position.x, unit->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y)) != 0);
			}
			if (!stop_there) {
				unit->sprite->play_iscript_animation(IscriptAnimation::Landing);
				unit->main_order_state = 3;
			}
		}

		// There's no "else" here, main_order_state = 2 can continue directly into main_order_state = 3 if main_order_state was updated
		if (unit->main_order_state == 3) {
			if ((unit->get_movable_state() != MovableState_NotReachedDestination) && (unit->order_signal & 0x10)) {
				unit->order_signal &= ~0x10;
				unit->flingy_turn_rate = get_modified_unit_turn_speed(unit);

				if (unit->order_target.pt.x >= 0 && unit->order_target.pt.x < g_map_tile_size->width * 32 && 
					unit->order_target.pt.y >= 0 && unit->order_target.pt.y < g_map_tile_size->height * 32) { // New sanity check; Attempt on fixing clear tile flags
					helper::clear_building_from_tile_flag(unit, unit->order_target.pt.x, unit->order_target.pt.y);
				}

				helper::end_collision_tracking(unit); // Recalculate position and update effect on surroundings?
				helper::toggle_pathing_related(unit); // Toggle pathing-related stuff to prepare to no longer use pathing

				unit->sprite->elevation = 4;
				helper::set_unit_pathing(unit, 0);
				scbw::set_unit_position(unit, unit->order_target.pt.x, unit->order_target.pt.y);

				helper::start_collision_tracking(unit); // Maybe update orders targeting the unit, set repulse for units in surroundings, and some other things?

				// Maybe the landing unit would get excluded from multi-units selection if that could happen in normal Starcraft gameplay? That may be what this function does.
				if (unit->sprite->flags & SpriteFlags::Selected && *g_client_selection_count > 1)
				{
					helper::update_building_land_unit_selection(0, unit);
				}

				helper::set_shadow_y(unit->sprite, 0);
				if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW)
				{
					unit->sprite->images.tail->flags &= ~ImageFlags::DontUpdate;
				}
				helper::create_landing_dust(unit->sprite);
				scbw::refresh_console();
				scbw::prepare_building_area(unit->position, unit->id);

				if (units_dat::base_property[unit->id] & UnitProperty::Attack) {
					unit->status |= UnitStatus::CanTurnAroundToAttack;
				}

				if (unit->order_queue_head) {
					bool has_move_order = false;
					if (unit->order_queue_head->order_id == OrderId::Move
					||	unit->order_queue_head->order_id == OrderId::Follow) {
						has_move_order = true;
					} else {
						Order* order = unit->order_queue_head;
						if (order->order_id != OrderId::PlaceAddon) {
							order = 0;
						}

						while (unit->order_queue_tail != order) {
							helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
						}
						if (!unit->order_queue_head) {
							unit->order_computer(units_dat::return_to_idle_order[unit->id]);
						}
					}

					if (has_move_order) {
						// This causes the landed building to immediately take off again to execute the queued move/follow order
						helper::prepend_order_targeting_nothing(unit, unit->order_queue_head, OrderId::BuildingLiftoff);
					}
				}
				// Some units iterations, idle orders affectations, and unknown order assignment?
				// Maybe affect another selected unit depending on attempt at giving order?
				helper::flying_building_lift_if_still_blocked(unit);
				helper::force_order_done(unit);

				UnitExt* unit_ext = get_unit_ext(unit);
				if (unit_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
					scbw::spawn_custom_creep(unit);
				}
				Unit* addon = helper::connect_addon_check(unit);
				if (addon) {
					helper::complete_addon(unit, addon);
				}

				if (unit_id_is_bunker(unit->id))
				{
					for (s32 i = 0; i < TransportCap; i++)
					{
						Unit* cargo_unit = unit->get_loaded_unit(i);
						if (!cargo_unit) continue;
					//	scbw::set_unit_position(cargo_unit, unit->position.x, unit->position.y);
						if (cargo_unit->subunit) {
							scbw::set_unit_position(cargo_unit->subunit, unit->position.x, unit->position.y);
						}
					}
				}

				if (*g_is_placing_building) {
					helper::place_building_proc();

					// Add the last minute occupation info of landing building to update an ongoing attempt to choose a building or landing location?
					if (!helper::can_still_place_building()) {
						helper::refresh_layers_3_and_4();
						helper::end_building_placement();
					}
				}
			}
		}
	}

	void orders_building_liftoff(Unit* unit) { // 004649B0
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueIsNeutralBunker]) {
			unit->order_to_idle();
			return;
		}
		if ((unit->status & UnitStatus::GroundedBuilding) && (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding))
		{
			Unit* addon = unit->building.addon;
			if (addon) {
				if (addon->id == UnitId::TerranLodestone) {
					UnitExt* addon_ext = get_unit_ext(addon);
					for (u32 i = 0; i < TransportCap; ++i) {
						Unit* unit = get_loaded_unit_from_index(addon, i);
						if (unit) {
							addon->main_order_timer = 0;
							helper::unload_unit_proc(unit);
						}
						addon_ext->loaded_unit[i].value = 0;
					}
				}
				helper::disconnect_from_addon(unit);
			}
			if (unit->is_factory()) {
				unit->rally.unit = unit;
			}

			helper::end_collision_tracking(unit); // Update globally known position and may remove repulse tile if no longer needed
			helper::modify_building_grounded_status(unit); // Disable GroundedBuilding status and prepare to enable pathing?

			unit->sprite->elevation = 12;
			helper::set_unit_pathing(unit, 1);
			helper::start_collision_tracking(unit); // Maybe update orders targetting the unit, set repulse for units in surroundings, and some others things?

			if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW) {
				unit->sprite->images.tail->flags |= ImageFlags::DontUpdate;
			}

			scbw::refresh_console();
			if (unit->sprite->position.x != unit->next_target_waypoint.x
			||	unit->sprite->position.y - 42 != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = unit->sprite->position.x;
				unit->next_target_waypoint.y = unit->sprite->position.y - 42;
			}
			// Originally called update_creep_disappearance_unit when lifting off Infested Command Centers
			scbw::update_custom_creep_disappearance(unit);

			if (*g_is_placing_building) {
				helper::place_building_proc();
			}
			unit->main_order_id = OrderId::Liftoff;

			UnitExt* unit_ext = get_unit_ext(unit);
			unit_ext->timers[TimerLiftLandDelay] = 52 + 1;
			helper::turn_unit(unit, 0); // Fix turning structures having difficulty lifting
		} else {
			if (!unit->order_queue_head) {
				unit->order(units_dat::return_to_idle_order[unit->id], 0, 0, 0, UnitId::None, 0);
			}
			helper::force_order_done(unit);
		}
	}
}