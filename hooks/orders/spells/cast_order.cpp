#include "cast_order.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
#include "hooks/orders/pathfinding.h"

namespace hooks
{
	// Hooked to change order conditions and actions for custom spells. - Pr0nogo
	s32 spell_order(s32 default_order_string, Unit* caster, u16 tech_id, s32 range, u16* energy_cost) // 004926D0
	{
		Unit* order_target = caster->order_target.unit;
		u16 meme = (u16)default_order_string;
		if (!tech_id_needs_target(tech_id)
		||	(order_target && hooks::can_target_spell(tech_id, order_target, UnitId::None, caster, order_target->sprite->position.x, order_target->sprite->position.y, &meme)))
		{
			u16 tech_cost = techdata_dat::energy_cost[tech_id] << 8;
			*energy_cost = tech_cost;
			if (caster->energy >= tech_cost) // Used to check for TheGathering cheat.
			{
				if (tech_id == TechId::Hallucination && !order_target) {
					Point16 target_pos = caster->order_target.pt;
					if (scbw::get_distance_fast(caster->position, target_pos) > range) {
						helper::set_move_target_xy(caster, target_pos.x, target_pos.y);
						return 0;
					}
					helper::set_move_target_xy(caster, caster->position.x, caster->position.y);

					if (!helper::is_angle_in_attack_angle(target_pos.y, target_pos.x, caster, TechId::Hallucination)) {
						helper::turn_unit(caster, scbw::get_angle(caster->position, target_pos));
						return 0;
					}

					return 1;
				}
				return helper::spell_order_move_towards_target(caster, range);
			}

			u8 caster_race = caster->get_race();
			char* string = helper::get_stat_txt_string(caster_race + 864);
			if (string) {
				if (*g_active_player_id == *g_local_human_id) {
					helper::set_unit_stat_txt_error_message(string);
				}
			}
			helper::to_idle(caster);
			return 0;
		}

		s16 order_string_index = default_order_string - 1;
		char* final_string;
		if (default_order_string) {
			if (order_string_index < g_stat_txt_buffer->get_string_count()) {
				final_string = (char*)g_stat_txt_buffer->get_string(order_string_index + 1);
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
	bool can_target_spell(u16 tech_id, Unit* target, s16 fow_unit_id, Unit* caster, s16 x, s16 y, u16* error) // 00492020
	{
		switch (tech_id) {
			// Targeting ground
			case TechId::Irradiate:
			case TechId::EMPShockwave:
			case TechId::ScannerSweep:
			case TechId::CarapaceSwarm:
			case TechId::Plague:
			case TechId::Ensnare:
			case TechId::PsionicStorm:
			case TechId::Hallucination: // New
			case TechId::DimensionalRecall:
			case TechId::StasisField:
		//	case TechId::LanifectDisruptionField:
			case TechId::MindTyrantMaelstrom:
			case TechId::LuminaryAuralAssault:
			case TechId::HypnagogueThroughTheLookingGlass:
			{
				return 1;
			}
			default:
			{
				if (tech_id != TechId::SpiderMines
				&&  tech_id != TechId::UnusedLobotomyMine)
				{
					bool has_target = false;
					u16 error_stat_string = 0;
					if (target) {
						if (!(target->status & UnitStatus::Invincible)) {
						LABEL_20:
							if (fow_unit_id == UnitId::None)
							{
								error_stat_string = (u32)helper::get_target_unit_ability_error_stat_string(target, caster->player, tech_id);
							}
							else
							{
								error_stat_string = helper::can_target_spell_on_fow_unit((s32)tech_id, fow_unit_id);
							}
							
							if (error_stat_string)
							{
								if (error)
								{
									*error = error_stat_string;
								}
							}
							return error_stat_string == 0;
						}
						has_target = fow_unit_id != UnitId::None;
					}
					else
					{
						has_target = fow_unit_id != UnitId::None;
						if (fow_unit_id == UnitId::None)
						{
							if (error)
							{
								*error = 878; // Must target units.<0>
							}
							return 0;
						}
					}
					if (!has_target || !(~(units_dat::base_property[fow_unit_id] >> 29) & 1))
					{
						if (error)
						{
							*error = 876; // Invalid target.<0>
						}
						return 0;
					}
					goto LABEL_20;
				}
				if (hooks::get_active_tile_flag(y, x, 0x410000, 0))
				{
					return 1;
				}
				if (error)
				{
					*error = 902; // Must target passable terrain.
				}
				return 0;
			}
		}
	}

	void orders_spell(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		u8 order_weapon_id = orders_dat::order_weapon_id[unit->main_order_id];
		Unit* target = unit->order_target.unit;

		// Paladin Skyfall
		if (unit->id == UnitId::TerranPaladin) {
			scbw::decloak_proc(unit);
			if (unit->order_signal & 0x20) {
				unit->order_signal &= ~0x20;
				unit_ext->timers[TimerSkyfall] = 3;
				unit_ext->values[ValueSkyfallStacks] = 0;
				Point16 pt = unit->order_target.pt;
				if (unit->order_target.unit) {
					pt = unit->order_target.unit->position;
					unit->order_target.unit = 0;
				}
				unit_ext->values[ValueSkyfallX] = unit->order_target.pt.x;
				unit_ext->values[ValueSkyfallY] = unit->order_target.pt.y;
			}
		}

		if (unit->id == UnitId::TerranWyvern) {
			unit_ext->values[ValueWyvernLandGrabOn] = 1;
			unit_ext->values[ValueWyvernLandGrabX] = unit->order_target.pt.x;
			unit_ext->values[ValueWyvernLandGrabY] = unit->order_target.pt.y;
		}

		if (unit->main_order_state == 2) {
			if (unit->order_signal & 2) {
				unit->order_signal -= 2;
				unit->order_to_idle();
			} else {
				if (target) {
					unit->order_target.pt.x = target->sprite->position.x;
					unit->order_target.pt.y = target->sprite->position.y;

					if (target->sprite->position.x != unit->next_target_waypoint.x
					||	target->sprite->position.y != unit->next_target_waypoint.y)
					{
						unit->next_target_waypoint.x = target->sprite->position.x;
						unit->next_target_waypoint.y = target->sprite->position.y;
					}
				}
			}
		} else {
			if (!helper::can_cast_spell(unit)) {
				char* message = (char*)g_stat_txt_tbl_new->get_string(876); // Invalid target
				helper::get_stat_txt_string(unit, unit->player, message); // do the actual transmission corresponding to the message
				helper::set_resource_target(unit); // make the unit stop since the order failed
				unit->order_to_idle();
			} else {
				u8 tech_used = orders_dat::tech_used[unit->main_order_id];
				s32 spell_cost;
				if (tech_used == TechId::None) {
					spell_cost = 0;
				} else {
					spell_cost = techdata_dat::energy_cost[tech_used] * 256;
				}

				if (unit->energy < spell_cost)
				{
					u8 race_id = unit->get_race();
					char* message = (char*)g_stat_txt_tbl_new->get_string(864 + race_id); // Not enough energy
					if (message) {
						if (*g_local_nation_id == unit->player && *g_active_player_id == *g_local_human_id)
						{
							helper::set_unit_stat_txt_error_message(message); // display an error message only for the owner of the unit
						}
					}
					unit->order_to_idle();
				} else {
					u32 progress_moving; // may initiate movement from unit depending on main_order_state
					if (unit->main_order_state == 0)
					{
						progress_moving = helper::ordered_spell_progress_moving(unit, 1);
					} else {
						progress_moving = helper::ordered_spell_progress_moving(unit, 0);
					}

					if (unit->main_order_state == 0)
					{
						unit->main_order_state = 1;
					}
					bool gcd_condition = true;
					if (unit->id == UnitId::TerranWyvern && unit->ground_weapon_cooldown > 0)
					{
						gcd_condition = false; // new
					}
					if (progress_moving == 0)
					{
						if (unit->get_movable_state() == MovableState_UnmovableAtDestination)
						{
							unit->order_to_idle();
						}
					} else {
						if (unit->get_spell_countdown() == 0
						&&	gcd_condition // new
						&&	helper::is_ready_to_attack(unit, order_weapon_id) // is ready to fire weapon (angle, distance...) maybe?
						&&	!(unit->movement_flags & MovementFlags_Accelerating)
						&&	unit->get_movable_state() != 0)
						{
							bool idiot = false;
							// in case it was modified by a previous function, relinking
							target = unit->order_target.unit;
							if (unit->unit_ai && target)
							{
								if (order_weapon_id <= WeaponId::MindTyrantMaelstrom
								||  order_weapon_id == WeaponId::HypnagogueThroughTheLookingGlass) // Maelstrom is the last spell-related weapon, by default
								{
									// array bound to the function
									const u8* const array_00492B44 = (u8*)(0x00492B44);

									// This code would not work for weapons id of NuclearMissile, YamatoGun, and weapons id before those, they must be excluded somewhere else
									// Maybe the switch and array use could be removed in a future version since enough elements use default value (switch 6)
									u32 switch_value = array_00492B44[order_weapon_id - 0x20];
									if (switch_value != 6)
									{
										// no idea why irradiate is so important
										u32 analyzed_target_status = target->irradiate_timer;
										if (switch_value == 2)
										{
											analyzed_target_status |= target->parasite_flags;
										}
										if (switch_value == 3)
										{
											analyzed_target_status |= target->ensnare_timer;
										}
										if (switch_value == 4)
										{
											analyzed_target_status |= target->plague_timer;
										}
										if (switch_value == 5)
										{
											analyzed_target_status |= target->is_blind;
										}
										if (switch_value == 0 && unit->is_frozen())
										{
											analyzed_target_status |= 1;
										}
										if (analyzed_target_status != 0)
										{
											unit->order_to_idle();
											idiot = true;
										}
									}
								}
							}
							if (!idiot)
							{
								unit->spend_unit_energy(spell_cost);

								u8 cooldown = helper::get_weapon_cooldown(unit, order_weapon_id);
								unit->spell_cooldown = cooldown;
								if (unit->id == UnitId::TerranWyvern)
								{
									unit->ground_weapon_cooldown = cooldown; // new
								}

								// disable flag 2 if enabled, else has no effect
								unit->order_signal &= ~2; //&= -3
								unit->movement_flags |= MovementFlags_StartingAttack;
								if (unit->sprite->images.head)
								{
									Image* current_image = unit->sprite->images.head;
									IscriptAnimation::Enum anim = (IscriptAnimation::Enum)orders_dat::order_animation[unit->main_order_id];
									if (tech_used == TechId::Lockdown && unit->mine_carrier.mine_count > 0)
									{
										unit->mine_carrier.mine_count--;
									}

									while (current_image)
									{
										current_image->play_iscript_animation(anim);
										current_image = current_image->link.next;
									}
								}
								unit->main_order_state = 2;
								if (unit->unit_ai)
								{
									g_aiscript_controller[unit->player].spellcaster_timer = 1;
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked to safeguard against crashes. -Pr0nogo
	void issue_order_targeting_ground(Unit* unit, u32 id, u16 coord_x, u16 coord_y) // 00475260
	{
		if (!unit) {
			return;
		}
		unit->user_action_flags |= 1u;
		unit->order(id, coord_x, coord_y, 0, UnitId::None, true);
		helper::do_next_queued_order(unit);
	}

	// Hooked to investigate custom abilities. -Pr0nogo
	// We will likely need to hook all calls to this if order ids ever exceed 256.
	void issue_tri_target_order(u8 order_1, u8 order_2, u8 order_3) // 0048CAF0
	{
		*g_is_targeting = 1;
		*g_order_id_unit = order_1;
		*g_order_id_obstructed = order_2;
		*g_order_id_ground = order_3;
		char* error = "";
		if (*g_network_table > 62)
		{
			error = (char*)g_network_table_DF->get_string(63);
		}
		helper::send_message_something(order_3, error);
		helper::set_ingame_input_procs();
		*g_minimap_cursor = 2;
		*g_buttonset_special_buttonset_id = 229;
		helper::update_current_buttonset();
	}
}