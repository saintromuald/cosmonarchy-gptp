#include "nuke_orders.h"
#include <BW/api.h>
#include <hooks/limits/sfx_extender.h>

namespace hooks
{
	void orders_nuke_track(Unit* unit) // 00464D10
	{
		// Unit is nuke painter (e.g. Eidolon, Seraph). - Pr0nogo
		if (unit->main_order_state != 0) {
			if (unit->main_order_state == 6) {
				if (!unit->connected_unit || unit->connected_unit->main_order_state == 5) {
					Image* current_image = unit->sprite->images.head;
					if (!unit->order_queue_head) {
						unit->order_simple(units_dat::return_to_idle_order[unit->id], false);
					}
					helper::force_order_done(unit);
					helper::change_unit_buttonset(unit, unit->id);
					bw::refresh_console();

					while (current_image)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}

					if (unit->building.nuke_missile && unit->building.nuke_missile->sprite) {
						current_image = (unit->building.nuke_missile)->sprite->images.head;
						while (current_image)
						{
							current_image->play_iscript_animation(IscriptAnimation::Death);
							current_image = current_image->link.next;
						}
						unit->building.nuke_missile = 0;
					}
					helper::ai_return_to_nearest_base_forced(unit);
				}
			}
		} else {
			unit->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);
			Thingy* nuke_dot = helper::create_thingy(SpriteId::Nuke_Target_Dot, (unit->connected_unit)->order_target.pt.x, (unit->connected_unit)->order_target.pt.y, unit->player);
			unit->building.nuke_missile = (Unit*)nuke_dot;
			if (nuke_dot) {
				nuke_dot->sprite->elevation = 19;
				helper::set_thingy_visibility_flags(nuke_dot);
			}
			unit->main_order_state = 6;
			bw::decloak_passive_proc(unit);
		}
	}

	void orders_nuke_ground(Unit* unit)
	{
		if (unit->next_target_waypoint.x != unit->order_target.pt.x || unit->next_target_waypoint.y != unit->order_target.pt.y) {
			unit->next_target_waypoint.x = unit->order_target.pt.x;
			unit->next_target_waypoint.y = unit->order_target.pt.y;
		}

		u32 distance = 0;
		u32 sight_range = unit->get_sight_range(false) * 32;
		if (unit->unit_ai) {
			distance = bw::get_distance_fast(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
			if (distance / 256 <= sight_range * 3) {
				helper::ai_order_unit_cloaking(unit);
			}
		}

		distance = bw::get_distance_fast(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
		if (distance / 256 > sight_range) {
			if (unit->recent_order_timer == 0) {
				helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
			}
		} else {
			bool end_there_idiot = false;
			helper::set_resource_target(unit);
			if (unit->next_target_waypoint.x != unit->position.x || unit->next_target_waypoint.y != unit->position.y) {
				s32 direction = unit->current_direction_1 - bw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
				if (direction < -1 || direction > 1) {
					end_there_idiot = true;
				}
			}
			if (!end_there_idiot) {
				bool end_again_idiot = false;
				Unit* current_unit = first_player_unit->unit[unit->player];
				while (!end_again_idiot)
				{
					if (!current_unit) {
						end_again_idiot = true;
					} else if (current_unit->id == UnitId::TerranAnvil && current_unit->building.silo.is_ready) {
						end_again_idiot = true;
					} else {
						current_unit = current_unit->player_link.next;
					}
				}
				if (!current_unit) {
					unit->order_to_idle();
				} else {
					Unit* nuke = current_unit->building.silo.nuke;
					current_unit->building.silo.nuke = 0;
					current_unit->building.silo.is_ready = false;
					helper::play_sound_from_unit(SoundId::Terran_GHOST_TGhLas00_wav, unit, 1, 0);
					nuke->order_to(OrderId::NukeLaunch, unit->order_target.pt.x, unit->order_target.pt.y);
					nuke->connected_unit = unit;
					helper::set_all_overlay_directions_generic(nuke, 0);
					helper::show_and_enable_unit(nuke);
					unit->connected_unit = nuke;
					unit->order_to(OrderId::NukeTrack, unit->sprite->position.x, unit->sprite->position.y);
					if (unit->is_frozen()) {
						if (units_dat::base_property[unit->id] & UnitProperty::Building) {
							unit->current_buttonset = UnitId::Buttons_CancelNukeStrike;
						}
					} else {
						unit->current_buttonset = UnitId::Buttons_CancelNukeStrike;
					}

					u32 action_sound = SoundId::No_sound;
					switch (unit->id)
					{
						case UnitId::TerranEidolon:
						{
							action_sound = helper::rand_between(SoundId::Terran_GHOST_TGhPss00_wav, SoundId::Terran_GHOST_TGhPss01_wav, 31);
						} break;
					}

					if (action_sound) {
						if (helper::play_sound_from_unit(action_sound, unit, g_options_list->flags & 1, 99)
						&& !*g_is_in_replay && unit->player == *g_local_nation_id) {
							helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[action_sound] + 1000 * (sfx_dat::sound_data[action_sound].duration_ms / 1000), 0, action_sound);
						}
					}
					bw::refresh_console();
				}
			}
		}
	}
}