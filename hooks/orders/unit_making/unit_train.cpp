#include "unit_train.h"
#include <SCBW/api.h>
#include "hooks/cosmonarchy_finder.h"
#include "hooks/orders/spells/hallucination_spell.h"
#include "hooks/orders/building_making/building_modifier.h"
#include "globals.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

namespace hooks {
	s32 is_queue_slot_active(Unit* unit, u32 slot) {
		return unit->build_queue[(slot + unit->build_queue_slot) % 5] != UnitId::None;
	}

	void secondary_order_train_fighter(Unit* unit) {
		if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantorOld) {
			if (unit->secondary_order_state == 0 || unit->secondary_order_state == 1) {
				if (unit->build_queue[unit->build_queue_slot] == UnitId::None) {
					unit->secondary_order_state = 3;
					unit->current_build_unit = 0;
				} else {
					Unit* built_unit = helper::attempt_train_hatch_unit(unit, unit->build_queue[unit->build_queue_slot], unit->secondary_order_state == 0 ? 1 : 0);
					unit->current_build_unit = built_unit;
					if (!built_unit) {
						unit->secondary_order_state = 1;
					} else {
						built_unit->hangar_unit.parent = unit;
						built_unit->sprite->player = unit->sprite->player;
						unit->secondary_order_state = 2;
					}
				}
			} else if (unit->secondary_order_state == 2) {
				Unit* built_unit = unit->current_build_unit;
				if (built_unit) {
					u32 hp_gain = helper::get_hp_gain_for_repair(built_unit);
					helper::progress_build(built_unit, hp_gain, FALSE);
					if (built_unit->status & UnitStatus::Completed) {
						helper::add_hangar_unit(unit, built_unit);
						if (unit->status & UnitStatus::IsHallucination) {
							apply_hallucination(built_unit, unit, unit->remove_timer);
						}
						unit->build_queue[unit->build_queue_slot] = UnitId::None;
						unit->build_queue_slot++;

						if (unit->build_queue_slot >= 5)
						{
							unit->build_queue_slot = 0;
						}
						scbw::refresh_console();
						unit->current_build_unit = 0;
						unit->secondary_order_state = 0;
					}
				} else {
					scbw::refresh_console();
					unit->current_build_unit = 0;
					unit->secondary_order_state = 0;
				}
			} else if (unit->secondary_order_state == 3) {
				if (unit->id == UnitId::ProtossAccantorOld) {
					unit->secondary_order_state = 4;
				} else {
					Unit* in_hangar_child = unit->unit_with_hangar.in_hangar_child;
					if (in_hangar_child) {
						s32 hp_steward = units_dat::max_hitpoints[UnitId::ProtossSteward];
						while (in_hangar_child && in_hangar_child->hitpoints >= hp_steward) {
							in_hangar_child = in_hangar_child->hangar_unit.hangar_link.next;
						}
						if (in_hangar_child) {
							in_hangar_child->set_hitpoints(in_hangar_child->hitpoints + 128);
						}
					}
				}
			}
		}
	}

	// generic unit training function
	void order_train(Unit* unit) {
		UnitExt* unitExt = get_unit_ext(unit);
		if (!(unit->status & UnitStatus::Frozen) && !unit->is_frozen()) {
			bool jump_to_685D8 = false;
			if (unit->secondary_order_state <= 1) {
				if (unit->build_queue[unit->build_queue_slot] == UnitId::None) {
					unit->set_secondary_order(OrderId::Nothing2);
					unit->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle, true);
				} else {
					Unit* built_unit = helper::attempt_train_hatch_unit(unit, unit->build_queue[unit->build_queue_slot], unit->secondary_order_state == 0 ? 1 : 0);
					unit->current_build_unit = built_unit;
					if (!built_unit) {
						unit->secondary_order_state = 1;
					} else {
						built_unit->sprite->player = unit->sprite->player;
						if (built_unit->subunit) {
							built_unit->subunit->sprite->player = unit->sprite->player;
						}
						scbw::play_work_animation(unit);
					}
				}
			} else if (unit->secondary_order_state == 2) {
				Unit* built_unit = unit->current_build_unit;
				if (built_unit) {
					UnitExt* built_unit_ext = get_unit_ext(built_unit);
					bool extra_build = false;
					BuildingModifier modifier = get_building_modifier(unit, 0, false);
					u32 hp_gain = helper::get_hp_gain_for_repair(built_unit) * modifier.multiplier;
					if (modifier.decrease) {
						if (built_unit->remaining_build_time > modifier.decrease) {
							built_unit->remaining_build_time -= modifier.decrease;
						} else {
							built_unit->remaining_build_time = 0;
							if (scbw::get_game_kind() == GameKind_Debug) {
								helper::set_unit_hp(built_unit, built_unit->get_max_hp_raw());
							}
						}
					}
					if (extra_build || helper::progress_build(built_unit, hp_gain, true)) {
						if (built_unit->status & UnitStatus::Completed) {
							u32 count = 1;
							Unit* built_units[2] = {built_unit, 0};
							if (units_dat::base_property[built_unit->id] & UnitProperty::TwoUnitsIn1Egg) {
								count = 2;
								built_units[1] = scbw::create_unit_in_game(built_unit->id, built_unit->player, built_unit->position.x - 32, built_unit->position.y, unit, true);
							}

							bool spawn_point_set = false;
							Point16 spawn_point = unit->position;
							if (unit->rally.pt.x || unit->rally.pt.y) {
								Point16 new_spawn_point = scbw::get_rect_edge_point_towards(unit->position, units_dat::unit_bounds[unit->id], unit->rally.pt);
								if (new_spawn_point != unit->rally.pt) {
									if (scbw::get_active_tile_at_coords(spawn_point)->ground_height == scbw::get_active_tile_at_coords(new_spawn_point)->ground_height) {
										spawn_point_set = true;
										spawn_point = new_spawn_point;
									}
								}
							}

							for (u32 i = 0; i < count; i++) {
								Unit* new_unit = built_units[i];
								if (new_unit) {
									if (spawn_point_set) {
										scbw::move_unit(new_unit, spawn_point.x, spawn_point.y);
									}
									helper::ai_training_unit(new_unit, unit);
									if (built_unit->id == UnitId::TerranMorningstar)
									{
										helper::hide_and_disable_unit(new_unit);
									} else {
										helper::order_new_unit_to_rally(new_unit, unit);
									}
								}
							}
							helper::clear_building_ai_queue_slot(unit, unit->build_queue_slot);
							unit->build_queue[unit->build_queue_slot] = UnitId::None;
							unit->build_queue_slot = (unit->build_queue_slot + 1) % 5;
							jump_to_685D8 = true;
						}
					} else { // unit was canceled due to lack of space around the building
						u32 queue_slot = unit->build_queue_slot % 5;
						if (unit->build_queue[queue_slot] == UnitId::None) {
							jump_to_685D8 = true;
						} else {
							u8 counter = 4;
							if (unit->current_build_unit) {
								helper::cancel_unit(unit->current_build_unit);
							} else if (!units_dat::base_property[unit->build_queue[unit->build_queue_slot % 5]] & UnitProperty::Building) {
								helper::refund_unit_train_cost(unit->build_queue[unit->build_queue_slot % 5], unit->player);
							}
							do {
								counter--;
								unit->build_queue[unit->build_queue_slot] = unit->build_queue[(unit->build_queue_slot + 1) % 5];
								if (unit->unit_ai && *(u8*)((u32)unit->unit_ai + 8) == 3)
								{
									*(u8*)((u32)unit->unit_ai + unit->build_queue_slot + 9) = *(u8*)((u32)unit->unit_ai + (unit->build_queue_slot + 1) % 5 + 9);
									*(u32*)((u32)unit->unit_ai + unit->build_queue_slot * 4 + 0x18) = *(u32*)((u32)unit->unit_ai + ((unit->build_queue_slot + 1) % 5) * 4 + 0x18);
								}
							} while (counter != 0);
							unit->build_queue[(unit->build_queue_slot + 1) % 5] = UnitId::None;
							jump_to_685D8 = true;
						}
					}
				} else {
					jump_to_685D8 = true;
				}
				if (jump_to_685D8) {
					scbw::refresh_console();
					unit->current_build_unit = 0;
					unit->secondary_order_state = 0;
				}
			}
		}
	}

	char get_max_hangar_space(Unit* unit) { // 004653D0
		if (unit->id == UnitId::ProtossSolarion) {
			return 12;
		}
		if (unit->id == UnitId::ProtossAccantorOld) {
			return 1;
		}
		return 0;
	}

	b32 _stdcall button_is_training(Unit* unit) {
		return unit->build_queue[unit->build_queue_slot % 5] != UnitId::None;
	}

	// Hooked to try a different approach for Massacre resources. -Pr0nogo
	void update_unit_stats_and_finish_building(Unit* unit) // 004A01F0
	{
		if (unit->remaining_build_time) {
			unit->hitpoints = units_dat::max_hitpoints[unit->id];
			unit->shields = units_dat::max_shield_points[unit->id] << 8;
			unit->remaining_build_time = 0;
		}

		if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building || unit->id == UnitId::None) {
			unit->current_buttonset = unit->id;
		}

		if (unit->status & UnitStatus::GroundedBuilding) {
			unit->parasite_flags = 0;
			unit->is_blind = 0;
			helper::init_building(unit, 0);
			for (Image* image = unit->sprite->images.head; image; image = image->link.next)
			{
				play_image_iscript(image, IscriptAnimation::Built);
			}

			if (!*g_is_in_replay && unit->player == *g_local_nation_id) {
				scbw::refresh_console();
			}
		} else {
			if (unit->status & UnitStatus::CanTurnAroundToAttack) {
				u32 start_dir = units_dat::start_direction[unit->id];
				if (start_dir == 32) {
					start_dir = helper::randomize_short(36) & 0x1F;
				}
				helper::turn_unit(unit, 8 * start_dir);
			}
			// Previously didn't run this helper for traps
			helper::show_and_enable_unit(unit);
		}
	}
}