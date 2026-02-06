#include "cast_order.h"
#include <BW/api.h>
#include "BW/stat_txt_spec.h"
#include "hooks/orders/pathfinding.h"
#include "hooks/unit_stats/sight_range.h"

namespace hooks {
	// Hooked to change order conditions and actions for custom spells. - Pr0nogo
	s32 spell_order(s32 default_order_string, Unit* caster, u16 tech_id, s32 range, u16* energy_cost) { // 004926D0
		Unit* order_target = caster->order_target.unit;
		u16 error_string = (u16)default_order_string;
		if (!tech_id_has_custom_targeting(tech_id)
		||	(order_target && hooks::can_target_spell(tech_id, order_target, UnitId::None, caster, order_target->sprite->position.x, order_target->sprite->position.y, &error_string)))
		{
			u16 tech_cost = techdata_dat::energy_cost[tech_id] << 8;
			*energy_cost = tech_cost;
			if (caster->energy >= tech_cost) {
				if (tech_id == TechId::Hallucination && !order_target) {
					auto& unit_ext = caster->ext();
					if (unit_ext.values[ValueJustCastedHallucination]) {
						return 0;
					}
					Point16 target_pos = caster->order_target.pt;
					if (bw::distance_between(caster->position, target_pos) > range) {
						helper::set_move_target_xy(caster, target_pos.x, target_pos.y);
						return 0;
					}
					helper::set_move_target_xy(caster, caster->position.x, caster->position.y);
					if (!helper::is_angle_in_attack_angle(target_pos.x, target_pos.y, caster, TechId::Hallucination)) {
						helper::turn_unit(caster, bw::get_angle(caster->position, target_pos));
						return 0;
					}
					return 1;
				}
				return helper::spell_order_move_towards_target(caster, range);
			}
			specialist_energy_error(caster);
			helper::to_idle(caster);
			return 0;
		}

		s16 order_string_index = default_order_string - 1;
		const char* final_string;
		if (default_order_string) {
			if (order_string_index < g_stat_txt_buffer.get_string_count32()) {
				final_string = g_stat_txt_buffer.get_string32(order_string_index + 1);
			} else {
				final_string = "";
			}
		} else {
			final_string = 0;
		}
		helper::get_stat_txt_string(caster, caster->player, final_string);

		if (caster->order_queue_head) {
			caster->user_action_flags |= UserActionFlags_IscriptGoToIdle;
			helper::do_next_queued_order(caster);
		} else {
			if (caster->unit_ai) {
				helper::issue_order_targeting_nothing(caster, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(caster, units_dat::return_to_idle_order[caster->id]);
			}
		}
		return 0;
	}

	// Hooked to change order conditions and actions for custom spells. - Pr0nogo
	bool can_target_spell(u16 tech_id, Unit* target, s16 fow_unit_id, Unit* caster, s16 x, s16 y, u16* error) { // 00492020
		switch (tech_id) {
			// Targeting terrain; added many new cases
			case TechId::Irradiate:
			case TechId::ScannerSweep:
			case TechId::CarapaceSwarm:
			case TechId::Plague:
			case TechId::Ensnare:
			case TechId::PsionicStorm:
			case TechId::Hallucination:
			case TechId::DimensionalRecall:
			case TechId::StasisField:
			case TechId::MindTyrantMaelstrom:
			case TechId::LuminaryAuralAssault:
			case TechId::HypnagogueThroughTheLookingGlass:
			case TechId::MatralethParasite:
			case TechId::DivineDiscipline:
			case TechId::GoskaLobotomyGrenade: {
				return true;
			}
			case TechId::AskosiRematerialize:
			case TechId::AskosiRapidRendezvous:
			case TechId::AskosiSalvage: {
				return target && bw::are_mutual_allies(caster->player, target->player);
			}
			case TechId::AskosiHijack: {
				if (target) {
					return bw::are_mutual_allies(caster->player, target->player) || (bw::are_mutual_allies(caster->player, target->player) && target->is_askosi_embedded());
				}
				return false;
			}
			case TechId::TritonLobotomyWarhead:
			case TechId::RejoinderOfService: {
				return target != nullptr;
			}
			default: {
				if (tech_id != TechId::SpiderMines) {
					bool has_target = false;
					u16 error_stat_string = 0;
					if (target) {
						if (!target->is_invincible()) {
						LABEL_20:
							if (fow_unit_id == UnitId::None) {
								error_stat_string = helper::get_tech_use_error_message(target, caster->player, tech_id);
							} else {
								error_stat_string = helper::can_target_spell_on_fow_unit((s32)tech_id, fow_unit_id);
							}							
							if (error_stat_string) {
								if (error) {
									*error = error_stat_string;
								}
							}
							return error_stat_string == 0;
						}
						has_target = fow_unit_id != UnitId::None;
					} else {
						has_target = fow_unit_id != UnitId::None;
						if (fow_unit_id == UnitId::None) {
							if (error) {
								*error = 878; // Must target units.<0>
							}
							return 0;
						}
					}
					if (!has_target || units_dat::base_property[fow_unit_id] & UnitProperty::Invincible) {
						if (error) {
							*error = 876; // Invalid target.<0>
						}
						return 0;
					}
					goto LABEL_20;
				}
				if (hooks::get_active_tile_flag(y, x, 0x410000, 0)) {
					return 1;
				}
				if (error) {
					*error = 902; // Must target passable terrain.
				}
				return 0;
			}
		}
	}

	void orders_spell(Unit* unit) { // 00492850
		auto& unit_ext = unit->ext();
		u32 order_weapon_id = orders_dat::order_weapon_id[unit->main_order_id];
		Unit* target = unit->order_target.unit;

		// Paladin Skyfall
		if (unit->id == UnitId::TerranPaladin) {
			bw::decloak_passive_proc(unit);
			if (unit->order_signal & 0x20) {
				unit->order_signal &= ~0x20;
				unit_ext.timers[TimerSkyfall] = 3;
				unit_ext.values[ValueSkyfallStacks] = 0;
				Point16 pt = unit->order_target.pt;
				if (unit->order_target.unit) {
					pt = unit->order_target.unit->position;
					unit->order_target.unit = 0;
				}
				unit_ext.values[ValueSkyfallX] = unit->order_target.pt.x;
				unit_ext.values[ValueSkyfallY] = unit->order_target.pt.y;
			}
		}

		if (unit->id == UnitId::TerranWyvern) {
			unit_ext.values[ValueWyvernLandGrabOn] = 1;
			unit_ext.values[ValueWyvernLandGrabX] = unit->order_target.pt.x;
			unit_ext.values[ValueWyvernLandGrabY] = unit->order_target.pt.y;
		}

		if (unit->main_order_state == 2) {
			if (unit->order_signal & 2) {
				unit->order_signal -= 2;
				unit->order_to_idle();
			} else if (target) {
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
				if (target->sprite->position.x != unit->next_target_waypoint.x || target->sprite->position.y != unit->next_target_waypoint.y) {
					unit->next_target_waypoint.x = target->sprite->position.x;
					unit->next_target_waypoint.y = target->sprite->position.y;
				}
			}
			return;
		}

		if (helper::cannot_cast_spell(unit)) {
			char* message = (char*)g_stat_txt_tbl->get_string32(876); // Invalid target
			helper::get_stat_txt_string(unit, unit->player, message); // do the actual transmission corresponding to the message
			helper::set_resource_target(unit); // make the unit stop since the order failed
			unit->order_to_idle();
			return;
		}

		u8 tech_used = orders_dat::tech_used[unit->main_order_id];
		u16 spell_cost = 0;
		if (tech_used != TechId::None) {
			spell_cost = techdata_dat::energy_cost[tech_used] * 256;
		}

		if (unit->energy < spell_cost) {
			u8 race_id = unit->get_race();
			if (g_local_nation_id == unit->player && g_active_player_id == g_local_human_id) {
				helper::set_unit_stat_txt_error_message("Not enough energy");
			}
			unit->order_to_idle();
		} else {
			bool progress_moving = false;
			if (unit->main_order_state == 0) {
				progress_moving = helper::ordered_spell_progress_moving(unit, 1);
			} else {
				progress_moving = helper::ordered_spell_progress_moving(unit, 0);
			}
			if (unit->main_order_state == 0) {
				unit->main_order_state = 1;
			}

			bool gcd_condition = true;
			if (unit->id == UnitId::TerranWyvern && unit->ground_weapon_cooldown > 0) {
				gcd_condition = false; // new
			}
			if (!progress_moving) {
				if (unit->get_movable_state() == MovableState_UnmovableAtDestination) {
					unit->order_to_idle();
				}
			} else if (unit->get_spell_countdown() == 0 &&	gcd_condition && helper::is_ready_to_attack(unit, order_weapon_id) && !(unit->movement_flags & MovementFlags_Accelerating) &&	unit->get_movable_state() != MovableState_NotReachedDestination) {
				bool cant_cast = false;
				// in case it was modified by a previous function, relinking
				target = unit->order_target.unit;
				if (unit->unit_ai && target) {
					// Experimenting with removing weapon id checks entirely, as it was not scalable.
					// This code only runs for AI, so any damage it could do is limited.			-Pr0nogo 09/01/25
					
					// array bound to the function
					const u8* const array_00492B44 = (u8*)(0x00492B44);

					// This code would not work for weapons id of NuclearMissile, YamatoGun, and weapons id before those, they must be excluded somewhere else
					// Maybe the switch and array use could be removed in a future version since enough elements use default value (switch 6)
					u32 switch_value = array_00492B44[order_weapon_id - 0x20];
					if (switch_value != 6) {
						// No idea why irradiate is so important
						u32 analyzed_target_status = target->irradiate_timer;
						if (switch_value == 2) {
							analyzed_target_status |= target->parasite_flags;
						}
						if (switch_value == 3) {
							analyzed_target_status |= target->ensnare_timer;
						}
						if (switch_value == 4) {
							analyzed_target_status |= target->plague_timer;
						}
						if (switch_value == 5) {
							analyzed_target_status |= target->is_blind;
						}
						if (switch_value == 0 && unit->is_frozen()) {
							analyzed_target_status |= 1;
						}
						if (analyzed_target_status != 0) {
							unit->order_to_idle();
							cant_cast = true;
						}
					}
				}

				if (!cant_cast) {
					unit->spend_energy(spell_cost);
					u8 cooldown = helper::get_weapon_cooldown(unit, order_weapon_id);
					if (order_weapon_id == WeaponId::PsiStorm) {
						cooldown = 0; // Recast doesn't have cooldown, but we apply the cooldown in the event of cancel
					}
					unit->spell_cooldown = cooldown;
					if (unit->id == UnitId::TerranWyvern) {
						unit->ground_weapon_cooldown = cooldown; // new
					}

					// disable flag 2 if enabled, else has no effect
					unit->order_signal &= ~2;
					unit->movement_flags |= MovementFlags_StartingAttack;
					if (unit->sprite->images.head) {
						Image* current_image = unit->sprite->images.head;
						IscriptAnimation::Enum anim = (IscriptAnimation::Enum)orders_dat::order_animation[unit->main_order_id];
						while (current_image) {
							current_image->play_iscript_animation(anim);
							current_image = current_image->link.next;
						}
					}
					unit->main_order_state = 2;
					if (unit->unit_ai) {
						g_aiscript_controller[unit->player].spellcaster_timer = 1;
					}
				}
			}
		}
	}

	// Hooked to safeguard against crashes. -Pr0nogo
	void issue_order_targeting_ground(Unit* unit, u32 id, u16 coord_x, u16 coord_y) { // 00475260
		if (!unit) {
			return;
		}
		unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
		unit->order(id, coord_x, coord_y, 0, UnitId::None, true);
		helper::do_next_queued_order(unit);
	}

	// Hooked to investigate custom abilities. -Pr0nogo
	// We will likely need to hook all calls to this if order ids ever exceed 256.
	void issue_tri_target_order(u8 order_1, u8 order_2, u8 order_3) { // 0048CAF0
		g_is_targeting = 1;
		*g_order_id_unit = order_1;
		*g_order_id_obstructed = order_2;
		*g_order_id_ground = order_3;
		const char* error = "";
		if (g_network_table_DF->get_string_count() > 62) {
			error = g_network_table_DF->get_string(63);
		}
		helper::send_message_something(order_3, error);
		helper::set_ingame_input_procs();
		g_minimap_cursor = 2;
		g_buttonset_special_buttonset_id = 229;
		helper::update_current_buttonset();
	}

	// Hooked to delard energy expenditure. -Pr0nogo
	void orders_defensive_matrix(Unit* caster) { // 004550A0
		u16 energy_cost;
		if (helper::spell_order(caster, TechId::DefensiveMatrix, &energy_cost, 8 * 32, 880)) {
			caster->spend_energy(energy_cost);
			helper::create_defensive_matrix(caster->order_target.unit);
			helper::play_sound_from_unit(SoundId::Terran_VESSEL_TVeDef00_WAV, caster, 1, 0);
			if (caster->order_queue_head) {
				caster->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				helper::do_next_queued_order(caster);
			} else if (caster->unit_ai) {
				helper::issue_order_targeting_nothing(caster, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(caster, units_dat::return_to_idle_order[caster->id]);
			}
		}
	}

	// Hooked to handle new ability cases. -Pr0nogo
	bool cannot_cast_spell(Unit* unit) { // 00492140
		u32 order_id = unit->main_order_id;
		u32 order_weapon_id = orders_dat::order_weapon_id[order_id];
		Unit* target = unit->order_target.unit;
		if (unit->unit_ai) {
			switch (order_id) {
				// Stasis Field logic was originally also used for LobotomyGrenade (aka Magna Pulse)
				case OrderId::LobotomyWarhead:
				case OrderId::StasisField:
				case OrderId::AskosiRematerialize:
				case OrderId::AskosiRapidRendezvous:
				case OrderId::AskosiHijack: {
					if (!target || target->is_frozen()) {
						return true;
					}
				} break;
				case OrderId::AskosiSalvage: {
					if (!target || target->ext().timers[TimerSalvageStun]) {
						return true;
					}
				} break;
				// Originally had Parasite code here
				// Originally had EMP Shockwave code here
				case OrderId::Irradiate: {
					if (!target || target->irradiate_timer) {
						return true;
					}
				} break;
				case OrderId::Plague: {
					if (!target || target->plague_timer) {
						return true;
					}
				} break;
				// Originally had Ensnare code here
			}
		}
		if (orders_dat::use_weapon_targeting[order_id]) {
			return helper::can_weapon_target_unit(order_weapon_id, target, unit) == false;
		}
		u32 tech_id = orders_dat::tech_used[order_id];
		switch (tech_id) {
			case TechId::None:
			case TechId::ScannerSweep:
			case TechId::CarapaceSwarm:
			case TechId::Plague:
			case TechId::PsionicStorm:
			case TechId::DimensionalRecall:
			case TechId::StasisField:
			case TechId::MindTyrantMaelstrom:
			case TechId::HypnagogueThroughTheLookingGlass:
			case TechId::MatralethParasite:
			case TechId::Hallucination:
			case TechId::YamatoGun:
			case TechId::Irradiate:
			case TechId::LuminaryAuralAssault:
			case TechId::GoskaLobotomyGrenade:
			case TechId::TritonLobotomyWarhead:
			case TechId::DivineDiscipline:
			case TechId::RejoinderOfService:
			case TechId::AskosiRematerialize:
			case TechId::AskosiRapidRendezvous:
			case TechId::AskosiSalvage:
			case TechId::AskosiHijack: {
				return false;
			} break;
			default: {
				b32 can_cast_on_tile = false;
				if (tech_id == TechId::SpiderMines) {
					u32 order_target_x = unit->order_target.pt.x;
					u32 order_target_y = unit->order_target.pt.y;
					can_cast_on_tile = get_active_tile_flag(order_target_y, order_target_x, 0x410000, 0);
				} else {
					if (!target || target->is_invincible()) {
						return true;
					}
					can_cast_on_tile = (u16)helper::get_tech_use_error_message(target, unit->player, tech_id) == 0;
				}
				if (can_cast_on_tile) {
					return false;
				}
			} break;
		}
	}

	// Hooked to handle new ability cases. -Pr0nogo
	b32 spell_order_move_towards_target(Unit* caster, u32 range) { // 00492450
		u32 final_range = range;
		Unit* target = caster->order_target.unit;
		if (target && target->sprite) { // Sprite check is new.
			u32 tgt_pos_x = target->sprite->position.x;
			u32 tgt_pos_y = target->sprite->position.y;
			caster->order_target.pt.x = tgt_pos_x;
			caster->order_target.pt.y = tgt_pos_y;
			if (tgt_pos_x != caster->next_target_waypoint.x || tgt_pos_y != caster->next_target_waypoint.y) {
				caster->next_target_waypoint.x = tgt_pos_x;
				caster->next_target_waypoint.y = tgt_pos_y;
			}
			if (!helper::is_unit_visible(caster)) {
				if (caster->move_target.pt.x == caster->sprite->position.x && caster->move_target.pt.y == caster->sprite->position.y && caster->status & UnitStatus::Unmovable) {
					helper::set_resource_target(caster);
					helper::to_idle(caster);
					return false;
				}
				if (target) {
					helper::move_towards(caster, target);
					return false;
				}
				if (!caster->recent_order_timer) {
					helper::set_move_target_xy(caster, caster->order_target.pt.x, caster->order_target.pt.y);
					return false;
				}
			}
			if (caster->movement_flags & MovementFlags_Accelerating) {
				if (!(target->movement_flags & MovementFlags_Accelerating)) {
					final_range = range + (helper::get_flingy_halt_distance(caster) >> 8);
				}

				u32 velocity_diff = (u8)(target->velocity_direction_1 - caster->velocity_direction_1);
				if (velocity_diff > 128) {
					velocity_diff = 256 - velocity_diff;
				}
				if (velocity_diff > 32) {
					final_range = range + (helper::get_flingy_halt_distance(caster) >> 8);
				}
			}

			if (helper::is_target_within_min_range(caster, final_range, target)) {
				helper::set_resource_target(caster);
				if (caster->next_target_waypoint == caster->position) {
					return true;
				}
				u32 curr_dir1 = (u8)caster->current_direction_1 - helper::get_direction_from_points(caster->sprite->position, caster->next_target_waypoint);
				u32 curr_dir_2 = curr_dir1 + ((s8)-(curr_dir1 < 0) & 256);
				if (curr_dir_2 > 128) {
					curr_dir_2 = 256 - curr_dir_2;
				}
				if (curr_dir_2 <= 5) {
					return true;
				}
			} else {
				Point16 sprite_pos = caster->sprite->position;
				if (caster->move_target.pt.x == sprite_pos.x && caster->move_target.pt.y == sprite_pos.y && ((caster->status & UnitStatus::Unmovable) != 0) == 1) {
					helper::set_resource_target(caster);
					helper::to_idle(caster);
					return false;
				}
				if (target) {
					helper::move_towards(caster, target);
					return false;
				}
				if (!caster->recent_order_timer) {
					helper::set_move_target_xy(caster, caster->order_target.pt.x, caster->order_target.pt.y);
					return false;
				}
			}
			return false;
		} else if (!tech_id_has_custom_targeting(orders_dat::tech_used[caster->main_order_id])) {
			if (bw::distance_between(caster->position, caster->order_target.pt) >= range) {
				helper::set_move_target_xy(caster, caster->order_target.pt.x, caster->order_target.pt.y);
			}
			return true;
		} else {
			helper::set_resource_target(caster);
			if (caster->order_queue_head) {
				caster->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				helper::do_next_queued_order(caster);
				return false;
			} else {
				if (caster->unit_ai) {
					helper::issue_order_targeting_nothing(caster, OrderId::ComputerAI);
				} else {
					helper::issue_order_targeting_nothing(caster, units_dat::return_to_idle_order[caster->id]);
				}
				return false;
			}
		}
	}
}