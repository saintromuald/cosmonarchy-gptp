#include <BW/api.h>
#include "building_modifier.h"
#include <BW/enumerations.h>
#include <BW/bwdata.h>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\building_making\building_helper.h"
#include "logger.h"
#include <globals.h>

namespace hooks {
	bool is_morphing(Unit* building) {
		return !(building->status & UnitStatus::Completed) && helper::is_valid_building_morph(building->build_queue[building->build_queue_slot]);
	}

	void orders_zerg_build_self(Unit* building) {
		UnitExt* building_ext = get_unit_ext(building);
		u16 building_id = building->build_queue[building->build_queue_slot];
		BuildingModifier modifier = get_building_modifier(building, 0, helper::is_valid_building_morph(building->id) ? false : true);

		if (!bw::is_zerg_building_morph_result(building_id)) {
			building_id = building->id;
		}

		switch (building->main_order_state) {
			case 0: {
				bw::prepare_building_area(building->position, building_id);
				// Remove statuses if the source worker had them before morphing
				building->status &= ~UnitStatus::NoCollide;
				building->status &= ~UnitStatus::IsGathering;
				if (helper::is_valid_building_morph(building_id)) {
					building->create_origin = building->id;
				}
				++building->main_order_state;
			} // fall
			case 1: {
				if (building->remaining_build_time < 3 * units_dat::time_cost[building_id] >> 2) {
					if (building->id == UnitId::ZergExcisant) {
						Image* overlay = building->sprite->get_overlay(ImageId::VespeneGeyser2);
						if (overlay) {
							overlay->free();
						}
					}
					building->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);
					++building->main_order_state;
				}
			} break;
			case 2: {
				if (building->remaining_build_time < units_dat::time_cost[building_id] >> 1) {
					building->sprite->play_iscript_animation(IscriptAnimation::SpecialState2, true);
					++building->main_order_state;
				}
			} break;
			case 3: {
				if (!building->remaining_build_time) {
					building->sprite->play_iscript_animation(IscriptAnimation::AlmostBuilt, true);
					helper::play_building_completed_sound(building);
					++building->main_order_state;
				}
			} break;
			case 4: {
				if (building->order_signal & 0x4) {
					building->order_signal &= ~0x4;

					if (helper::is_valid_building_morph(building_id)) {
						s32 hp_after_morph = building->hitpoints;
						u32 save_shields = building->shields;

						building->status |= UnitStatus::Completed;
						helper::increment_unit_scores_ex(building, -1, 0);

						building->status &= ~UnitStatus::Completed;
						helper::replace_unit_with_id(building, building_id);

						hp_after_morph += units_dat::max_hitpoints[building_id] - units_dat::max_hitpoints[building->previous_unit_type];
						if (hp_after_morph < 256) {
							hp_after_morph = 256;
						}
						building->set_hitpoints(hp_after_morph);
						building->remaining_build_time = 0;
					} else if (building->create_origin == UnitId::ZergGosvileth) {
						Unit* worker = bw::create_unit_in_game(UnitId::ZergGosvileth, building->player, building->position.x, building->position.y, 0, 0, 0);
						if (worker) {
							worker->set_hitpoints(building->previous_hitpoints << 8);
						}
					}

					helper::update_unit_stats_and_finish_building(building);
					helper::update_unit_strength(building);

					for (Image* image = building->sprite->images.head; image; image = image->link.next) {
						image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
					}

					helper::update_new_unit_vision(building_id, building->sprite->position.x, building->sprite->position.y);
				}
				return;
			} break;
			default: {
				return;
			} break;
		}

		u16 hp_gain = building->build_repair_hp_gain * modifier.multiplier;
		if (building->remaining_build_time) {
			--building->remaining_build_time;

			if (modifier.decrease) {
				if (building->remaining_build_time > modifier.decrease) {
					building->remaining_build_time -= modifier.decrease;
				} else {
					building->remaining_build_time = 0;
				}
			}
		}

		if (!bw::is_zerg_building_morph_result(building_id) || building_ext->values[ValueIsNydusExit]) {
			building->set_hitpoints(building->hitpoints + hp_gain);
		}
	}

	void cancel_zerg_building(Unit* unit) { // 0045DA40
		if (unit->status & UnitStatus::Completed || !helper::is_valid_building_morph(unit->build_queue[unit->build_queue_slot])) {
			helper::refund_building_cost(unit->id, unit->player);
			unit_score[unit->player] -= units_dat::build_score[UnitId::ZergDroleth]; // TODO: Check if a Gosvileth is the builder and do nothing to score
			--units_owned[unit->player];

			if (unit->id == UnitId::ZergExcisant) {
				Unit* worker = helper::create_unit(unit->create_origin, unit->get_x(), unit->get_y(), unit->player);

				if (worker) {
					helper::update_unit_stats_and_finish_building(worker);
					helper::update_unit_strength(worker);
					worker->set_hitpoints(unit->previous_hitpoints << 8);
					worker->sprite->player = unit->sprite->player;
				} else {
					helper::refund_unit_train_cost(unit->create_origin, unit->player);
					helper::display_last_network_error_for_player(unit->player);
				}
				unit->remove();
			} else {
				Thingy* thingy = helper::create_thingy(SpriteId::Zerg_Building_Spawn_Small, unit->get_x(), unit->get_y(), 0);

				if (thingy) {
					thingy->sprite->elevation = unit->sprite->elevation + 1;
					helper::set_thingy_visibility_flags(thingy);
				}

				u32 previous_unit_id = unit->id;
				u32 previous_hitpoints = unit->previous_hitpoints << 8;
				helper::play_sound_from_unit(SoundId::Misc_ZBldgPlc_wav_1, unit, 1, 0);
				helper::replace_unit_with_id(unit, unit->create_origin);
				helper::update_building_ai(unit);
				helper::update_unit_stats_and_finish_building(unit);
				helper::update_unit_strength(unit);

				u32 vertical_offset = 7;
				if (unit->id == UnitId::ZergGosvileth) {
					vertical_offset = 42;
				}

				if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW && unit->sprite->images.tail->vertical_offset != vertical_offset) {
					unit->sprite->images.tail->flags |= ImageFlags::Redraw;
					unit->sprite->images.tail->vertical_offset = vertical_offset;
				}

				helper::refresh_sprite_data(unit->sprite);
				unit->order_computer(OrderId::ResetCollision1);
				unit->order(units_dat::return_to_idle_order[unit->id], 0, 0, 0, UnitId::None, false);
				unit->set_hitpoints(previous_hitpoints);
				helper::update_creep_disappearance_unit_id(previous_unit_id, unit->get_x(), unit->get_y(), false);
			}
		} else {
			helper::cancel_zerg_building_morph(unit);
		}
	}
}

// from unhooked
namespace
{
	s32 BuildingHelper::get_unit_id_count(const Unit* unit, u16 unit_id, bool ignoreIncomplete);
}

namespace hooks
{
	// Checks if @p unit_id is a building that can be morphed from another building.
	// Note: This hook affects the behavior of Unit::is_remorphing_building().
	bool is_morphed_building(u16 unit_id)
	{
		if (bw::is_zerg_building_morph_result(unit_id))
		{
			return true;
		}
		return false;
	}

	// Determines the number of units which have @p unit_id and are being built by the @p unit.
	// This hook is used by StarCraft's AI for counting units and buildings.
	s32 get_morph_building_id_count(const Unit* unit, u16 unit_id, bool ignore_incomplete)
	{
		s32 unit_count = BuildingHelper::get_unit_id_count(unit, unit_id, ignore_incomplete);

		switch (unit_id)
		{
			case UnitId::ZergHachirosk:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergCaphrolosk, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergGathtelosk, ignore_incomplete);
			} break;

			case UnitId::ZergCagrantCesiant:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSurkithCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpraithCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLarvoskCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOcsalCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergKazitrithCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSkortrithCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpirtithCesiant, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergNydusCesiant, ignore_incomplete);
			} break;

			case UnitId::ZergSurkithCesiant:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSkortrithCesiant, ignore_incomplete);
			}

			case UnitId::ZergSpraithCesiant:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpirtithCesiant, ignore_incomplete);
			}

			case UnitId::ZergIrolIris:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOthstolOviform, ignore_incomplete);
			} break;

			case UnitId::ZergQuazethPool:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergVornathPond, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergKalkiirLake, ignore_incomplete);
			} break;

			case UnitId::ZergVornathPond:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergKalkiirLake, ignore_incomplete);
			} break;

			case UnitId::ZergHydrithDen:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSkibactScab, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergTethvithSwamp, ignore_incomplete);
			} break;

			case UnitId::ZergSkibactScab:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergTethvithSwamp, ignore_incomplete);
			} break;

			case UnitId::ZergMutathSpire:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergTosvrolHaunt, ignore_incomplete);
			} break;

			case UnitId::ZergMatravilNest:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlmakisAntre, ignore_incomplete);
			} break;

			case UnitId::ZergGeszkathGrotto:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlkajChasm, ignore_incomplete);
			} break;

			case UnitId::ZergZorkizShroud:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergUltravCavern, ignore_incomplete); 
			} break;
		}
		return unit_count;
	}

	void set_special_state_1(Sprite* sprite) {
		for (Image* image = sprite->images.head; image; image = image->link.next) {
			if (image->id != ImageId::StargateOverlay) {
				image->play_iscript_animation(IscriptAnimation::SpecialState1);
			}
		}
	}

	// Hooked to help Veeq7 remember to go to sleep. -Pr0nogo
	void drone_morph_standard(Unit* unit, u32 id) { // 0045D890
		helper::increment_unit_scores_ex(unit, -1, 0);
		unit->status &= ~UnitStatus::Completed;
		unit->create_origin = unit->id;
		helper::replace_unit_with_id(unit, id);
		if (g_player_table[unit->player].type == PlayerType::Computer) {
			if (unit->unit_ai) {
				WorkerAi* worker_ai = (WorkerAi*)unit->unit_ai;
				AiTown* unit_town = worker_ai->town;
				unit->unit_ai = 0;
				helper::delete_worker_ai(worker_ai, &unit_town->workers);
				--unit_town->worker_count;
				helper::add_worker_structure_ai(unit, unit_town);
			}
		}
		helper::drone_build_execute(unit);
	}

	// Hooked to help Veeq7 remember to go to sleep. -Pr0nogo
	Unit* begin_gas_building(Unit* unit, u16 unit_id) { // 004678A0
		Unit* target = unit->order_target.unit;
		if (!target || target->id != UnitId::ResourceVespeneGeyser) {
			target = helper::find_unit_at_tile(unit->order_target.pt.y, unit->order_target.pt.x, OrderId::Fatal);
		}
		if (!target) {
			return 0;
		}
		if (*can_draw_placement_preview) {
			helper::inherit_gas_overlays(target->sprite); // Guessed helper name, probably wrong
		}
		helper::increment_unit_scores_ex(target, -1, 1);
		target->status &= ~UnitStatus::Completed;
		helper::give_unit(target, unit->player, 0);
		helper::give_sprite(target, unit->player);
		target->create_origin = unit->id;
		helper::replace_unit_with_id(target, unit_id);
		if (!bw::unit_is_stunned(target) || units_dat::base_property[target->id] & UnitProperty::Building) {
			target->current_buttonset = UnitId::Buttons_CancelConstruction;
		}
		helper::add_overlay_below_main(target->sprite, ImageId::VespeneGeyser2, 0, 0, 0);
		target->hitpoints = units_dat::max_hitpoints[unit_id] / 10;
		return target;
	}

	// Hooked to clear placement area (and maybe fix the AI stacked Droleth bug). -Pr0nogo
	void orders_drone_build(Unit* builder) { // 0045E090
		Sprite* builder_sprite = builder->sprite;
		Image* underlay = builder_sprite->images.tail;

		u32 vertical_offset = 7;
		if (builder->id == UnitId::ZergGosvileth) {
			vertical_offset = 42;
		}

		if (builder_sprite->position.x != builder->order_target.pt.x
		||  builder_sprite->position.y != builder->order_target.pt.y)
		{
			// Change shadow pos
			if (underlay->palette_type == 10 && underlay->vertical_offset != vertical_offset) {
				underlay->flags |= ImageFlags::Redraw;
				underlay->vertical_offset = vertical_offset;
			}
			helper::refresh_sprite_data(builder->sprite);

			Order* order_head = builder->order_queue_head;
			if (order_head) {
				helper::insert_order_before(builder, order_head, OrderId::ResetCollision1, 0, 0, 0, 228);
			} else {
				helper::perform_another_order(OrderId::ResetCollision1, 0, builder, 228, {}, 0);
			}
			helper::force_order_done(builder);
		} else {
			u32 building_id = builder->build_queue[builder->build_queue_slot];
			u16 pos_x = builder_sprite->position.x;
			u16 pos_y = builder_sprite->position.y;
			u32 dims_x = units_dat::building_dimensions[building_id].x;
			u32 dims_y = units_dat::building_dimensions[building_id].y;
			u32 tile_y = (pos_y - (dims_y >> 1)) / 32;
			if (helper::can_build_on_tile_location(builder, builder->player, (pos_x - (dims_x >> 1)) / 32, tile_y, building_id, 0, 0, 1, 1) // This is probably WRONG
			|| builder_sprite->position.x != builder->order_target.pt.x
			|| builder_sprite->position.y != builder->order_target.pt.y)
			{
				helper::play_worker_error_sound(builder, 856);
				if (underlay->palette_type == 10 && underlay->vertical_offset != vertical_offset) {
					underlay->flags |= ImageFlags::Redraw;
					underlay->vertical_offset = vertical_offset;
				}
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
				return;
			}

			u32 player = builder->player;
			g_ai_minerals_reserve[player] = units_dat::mineral_cost[building_id];
			g_ai_vespene_reserve[player] = units_dat::vespene_cost[building_id];

			// @Todo: Tidy this into not being inlined
			if (g_game_minerals[player] < g_ai_minerals_reserve[player]) {
				helper::display_error_message_id_with_sfx(player, 850, *g_console_index + 147);
				helper::set_shadow_y(builder->sprite, 7);
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
			}
			if (g_game_vespene[player] < g_ai_vespene_reserve[player]) {
				helper::display_error_message_id_with_sfx(player, 851, *g_console_index + 150);
				helper::set_shadow_y(builder->sprite, 7);
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
			}

			helper::subtract_queue_cost(builder->player);
			Unit* powerup = helper::get_unit_powerup(builder);
			if (powerup) {
				helper::drop_powerup(builder);
			}
			if (building_id == UnitId::ZergExcisant) {
				helper::droleth_build_excisant(builder);
			} else {
				drone_morph_standard(builder, building_id);
			}
			if (powerup) {
				helper::move_unit(powerup, powerup->powerup_origin.x, powerup->powerup_origin.y);
			}
		}
	}

	// Hooked to clear placement area (and maybe fix the AI stacked Droleth bug). -Pr0nogo
	void orders_drone_land(Unit* unit) { // 004E9AA0
		if (unit->unit_ai) {
			unit->status |= UnitStatus::IsGathering;
		}
		u32 order_state = unit->main_order_state;
		if (unit->main_order_state) {
			order_state -= 1;
			if (order_state) {
				if (order_state == 1) {
					Sprite* unit_sprite = unit->sprite;
					if (unit->move_target.pt.x == unit_sprite->position.x && unit->move_target.pt.y == unit_sprite->position.y) {
						u32 unmovable = ((unit->status & UnitStatus::Unmovable) != 0) + 1;
						if (((unit->status & UnitStatus::Unmovable) != 0) != -1) {
							Image* underlay = unit_sprite->images.tail;
							if (underlay->palette_type == 10) {
								if (underlay->vertical_offset) {
									underlay->flags |= ImageFlags::Redraw;
									underlay->vertical_offset = 0;
								}
							}
							if (unit->sprite->images.tail->palette_type == 10) { // Deliberately not using underlay var
								unit->sprite->images.tail->flags &= 0xFBu;
							}

							u32 vertical_offset = 7;
							if (unit->id == UnitId::ZergGosvileth) {
								vertical_offset = 42;
							}

							if (unmovable == 2) {
								helper::play_landing_interrupt_sound(unit);
								Order* order_queue = unit->order_queue_head;
								if (order_queue)
								{
									if (order_queue->order_id == OrderId::DroneBuild)
									{
										helper::remove_order_from_unit_queue(order_queue, unit);
									}
								}
								underlay = unit->sprite->images.tail;
								if (underlay->palette_type == 10 && underlay->vertical_offset != vertical_offset) {
									underlay->flags |= ImageFlags::Redraw;
									underlay->vertical_offset = vertical_offset;
								}
								helper::refresh_sprite_data(unit->sprite);
								helper::order_immediate(unit, OrderId::ResetCollision1);
								helper::force_order_done(unit);
							} else {
								Order* order_queue = unit->order_queue_head;
								if (order_queue && order_queue->order_id == OrderId::DroneBuild) {
									helper::move_unit(unit, unit->order_target.pt.x, unit->order_target.pt.y);
								} else {
									underlay = unit->sprite->images.tail;
									if (underlay->palette_type == 10 && underlay->vertical_offset != vertical_offset)
									{
										underlay->flags |= ImageFlags::Redraw;
										underlay->vertical_offset = vertical_offset;
									}
									helper::refresh_sprite_data(unit->sprite);
									helper::order_immediate(unit, OrderId::ResetCollision1);
								}
								bw::refresh_console();
								helper::force_order_done(unit);
							}
						}
					}
				}
				return;
			}
			if (unit->move_target.pt.x == unit->sprite->position.x && unit->move_target.pt.y == unit->sprite->position.y) {
				u32 unmovable = ((unit->status & UnitStatus::Unmovable) != 0) + 1;
				if (((unit->status & UnitStatus::Unmovable) != 0) != -1) {
					u32 queue_slot_id = unit->build_queue[unit->build_queue_slot];
					u32 can_build = helper::can_build_on_tile_location(unit, unit->player,
						(unit->order_target.pt.x - units_dat::building_dimensions[queue_slot_id].x / 2) / 32,
						(unit->order_target.pt.y - units_dat::building_dimensions[queue_slot_id].y / 2) / 32,
						queue_slot_id, 0, 0, 1, 1
					);
					if (queue_slot_id != UnitId::ZergExcisant || unmovable != 1) {
						if (!can_build && unmovable != 2) {
							if (helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128)) {
								if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y) || unit->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) { // Was BYTE1(unit->status)
									Image* underlay = unit->sprite->images.tail;
									if (underlay->palette_type == 10) {
										underlay->flags |= ImageFlags::DontUpdate;
									}
									u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
									unit->main_order_state = 2;
									unit->status = status;
								}
								return;
							}
						}
					} else if (!can_build) {
						Unit* unit_at_tile = helper::find_unit_at_tile(unit->order_target.pt.y, unit->order_target.pt.x, 188);
						if (unit_at_tile && helper::is_target_within_min_range(unit, 32, unit_at_tile)) {
							u32 builder_id = unit->id;
							unit->status &= 0xFFEFFFFF;
							unit->sprite->elevation = units_dat::elevation[builder_id] + 1;
							helper::order_targeting_nothing(OrderId::ResetCollision1, unit, 0);
							helper::order_targeting_nothing(units_dat::return_to_idle_order[unit->id], unit, 0);
							if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
								u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
								unit->main_order_state = 2;
								unit->status = status;
							}
							return;
						}
						if (helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128)) {
							if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y) || unit->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) { // Was BYTE1(unit->status)
								Image* underlay = unit->sprite->images.tail;
								if (underlay->palette_type == 10) {
									underlay->flags |= ImageFlags::DontUpdate;
								}
								u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
								unit->main_order_state = 2;
								unit->status = status;
							}
							return;
						}
						helper::play_landing_interrupt_sound(unit);
						Order* order_queue = unit->order_queue_head;
						if (order_queue) {
							if (order_queue->order_id == OrderId::DroneBuild) {
								helper::remove_order_from_unit_queue(order_queue, unit);
							}
						}
						helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
						return;
					}
					helper::play_landing_interrupt_sound(unit);
					Order* order_queue = unit->order_queue_head;
					if (order_queue) {
						if (order_queue->order_id == OrderId::DroneBuild) {
							helper::remove_order_from_unit_queue(order_queue, unit);
						}
					}
					helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
					return;
				}
			}
		} else {
			u32 tgt_x = unit->order_target.pt.x;
			u32 tgt_y = unit->order_target.pt.y - 7;
			bool move_target = helper::set_move_target_xy(unit, tgt_x, tgt_y);
			if (tgt_x != unit->next_target_waypoint.x || tgt_y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = tgt_x;
				unit->next_target_waypoint.y = tgt_y;
			}
			if (move_target) {
				unit->main_order_state = 1;
			}
		}
	}

	// Hooked to troubleshoot multi-building select queuing building mutation orders. -Pr0nogo
	void orders_morph2(Unit* unit) { // 0045D0D0
		u32 player = unit->player;
		u32 unit_id = unit->build_queue[unit->build_queue_slot];
		g_ai_minerals_reserve[player] = units_dat::mineral_cost[unit_id];
		g_ai_vespene_reserve[player] = units_dat::vespene_cost[unit_id];
		if (!helper::has_supplies_for_unit(player, unit_id, 1)) {
			unit->build_queue[unit->build_queue_slot] = UnitId::None;
			unit->order_to_idle();
			return;
		}
		if (g_game_minerals[player] < g_ai_minerals_reserve[player]) {
			u32 err_string_id = 850;
			u32 err_reason = *g_console_index + 0x93;
			helper::display_error_message_id_with_sfx(player, err_string_id, err_reason);

			unit->build_queue[unit->build_queue_slot] = UnitId::None;
			unit->order_to_idle();
			return;
		}
		if (g_game_vespene[player] < g_ai_vespene_reserve[player]) {
			u32 err_string_id = 851;
			u32 err_reason = *g_console_index + 0x96;
			helper::display_error_message_id_with_sfx(player, err_string_id, err_reason);

			unit->build_queue[unit->build_queue_slot] = UnitId::None;
			unit->order_to_idle();
			return;
		}

		g_game_minerals[player] -= g_ai_minerals_reserve[player];
		g_game_vespene[player] -= g_ai_vespene_reserve[player];
		unit->status = unit->status & ~UnitStatus::Completed;
		if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building) {
			unit->current_buttonset = UnitId::Buttons_HatcheryMorphing;
		}

		helper::init_building(unit, 1);
		unit->remaining_build_time = units_dat::time_cost[unit_id];
		unit->user_action_flags |= 1;
		if (unit->main_order_id) {
			for (Order* order = unit->order_queue_tail; order; order = unit->order_queue_tail) {
				if (!orders_dat::can_be_interrupted[order->order_id] && order->order_id != OrderId::ZergBuildSelf) {
					break;
				}
				helper::remove_order_from_unit_queue(order, unit);
			}
			helper::perform_another_order(OrderId::ZergBuildSelf, 0, unit, UnitId::None, {}, 0);
		}
		helper::do_next_queued_order(unit);
		for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
			play_image_iscript(image, IscriptAnimation::SpecialState1);
		}
		bw::refresh_console();
	}
}