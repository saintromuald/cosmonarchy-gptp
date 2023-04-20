#include "building_protoss.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks {
	void orders_probe_build(Unit* unit) { // 004E4D00
		if (unit->unit_ai) {
			unit->status |= UnitStatus::IsGathering;
		}

		switch (unit->main_order_state) {
			case 0: {
				if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
					++unit->main_order_state;
				}
			} break;
			case 1: {
				u32 distance = scbw::get_distance_fast(unit->halt.x, unit->halt.y, unit->order_target.pt.x * 256, unit->order_target.pt.y * 256);
				u32 distance_wanted = unit->build_queue[unit->build_queue_slot] == UnitId::ProtossAquifer ? 88 : 70;

				if (distance / 256 > distance_wanted) {
					if (unit->get_movable_state() != MovableState::NotReachedDestination) {
						helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
					}
				} else {
					helper::make_to_hold_position(unit);
					++unit->main_order_state;
				}
			} break;
			case 2: {
				if (!(unit->movement_flags & MovementFlags::Accelerating)) {
					bool is_building_aquifer = unit->build_queue[unit->build_queue_slot] == UnitId::ProtossAquifer;
					u32 distance_wanted = is_building_aquifer ? 88 : 70;

					if (!helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, distance_wanted)) {
						if (unit->get_movable_state() != MovableState::NotReachedDestination) {
							helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
						}
					} else {
						if (helper::begin_building(unit)) {
							Unit* built_unit;

							if (is_building_aquifer) {
								built_unit = helper::begin_gas_building(unit, UnitId::ProtossAquifer);
							} else {
								built_unit = helper::create_unit(unit->build_queue[unit->build_queue_slot], unit->order_target.pt.x, unit->order_target.pt.y, unit->player);
							}

							unit->build_queue[unit->build_queue_slot] = UnitId::None;

							if (built_unit) {
								if (units_dat::shields_enabled[built_unit->id]) {
									built_unit->shields += 3 * 256;
								}

								built_unit->create_origin = unit->id;
								built_unit->sprite->player = unit->sprite->player;
								helper::replace_sprite_images(built_unit->sprite, ImageId::WarpAnchor, 0);

								if (is_building_aquifer) {
									built_unit->sprite->create_underlay(ImageId::VespeneGeyser, 0, 0, 0);
								}

								built_unit->sprite->create_top_underlay(ImageId::WarpAnchorUnderlay, 0, 0, 0);
								unit->order_target.unit = built_unit;
								helper::ai_training_overlord(built_unit, unit);
								built_unit->order_computer(OrderId::BuildSelf2);
								helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld00_wav, unit, 1, 0);
								++unit->main_order_state;
								return;
							} else {
								helper::refund_unit_train_cost(unit->build_queue[unit->build_queue_slot], unit->player);
								helper::display_last_network_error_for_player(unit->player);
							}
						}

						unit->order_to_idle();
					}
				}
			} break;
			case 3: {
				Unit* target = unit->order_target.unit;
				if (!target || target->id != UnitId::ProtossAquifer || unit->order_queue_head) {
					unit->order_to_idle();
				} else {
					unit->order_to(OrderId::CanEnterGasMine, target);
				}

				for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
					image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				}
			} break;
		}
	}

	// Protoss self-building
	void orders_build_self_2(Unit* building) { // 004E4F40
		switch (building->main_order_state) {
			case 0: {
				if (!building->remaining_build_time) {
					for (Image* image = building->sprite->images.head; image; image = image->link.next) {
						if (image->flags & ImageFlags::Flag04) {
							image->animation = IscriptAnimation::SpecialState1;
							image->iscript_offset = *(u16*)(*g_iscript_data + image->iscript_header_offset + 0x22);
							image->wait = 0;
							image->return_address = 0;
							helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
						}
					}

					helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld03_wav, building, 1, 0);
					++building->main_order_state;
				}

				BuildingModifier modifier = get_building_modifier(building, 0, true);
				u16 hp_gain = building->build_repair_hp_gain * modifier.multiplier;
				u16 shield_gain = building->shield_gain * modifier.multiplier;
				s32 max_shields = units_dat::max_shield_points[building->id] << 8;

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

				building->set_hitpoints(building->hitpoints + hp_gain);
				if (building->shields + shield_gain >= max_shields) {
					building->shields = max_shields;
				} else {
					building->shields += shield_gain;
				}
			} break;
			case 1: {
				if (building->order_signal & 1) {
					building->order_signal &= ~1;
					helper::replace_sprite_images(building->sprite, sprites_dat::image_id[building->sprite->id], 0);
					helper::switch_to_warp_texture(building->sprite->main_graphic);
					++building->main_order_state;
				}
			} break;
			case 2: {
				if (building->order_signal & 1) {
					building->order_signal &= ~1;
					helper::replace_sprite_images(building->sprite, sprites_dat::image_id[building->sprite->id], 0);
					building->sprite->main_graphic->flags &= ~ImageFlags::Invisible; // Clear invisible flag if it exists, to prevent silliness with warping cloaked buildings.
					helper::update_unit_stats_and_finish_building(building);
					helper::update_unit_strength(building);
					helper::check_unstack(building);
					
					for (Image* image = building->sprite->images.head; image; image = image->link.next) {
						if (image->flags & ImageFlags::Flag04) {
							image->animation = IscriptAnimation::WarpIn;
							image->iscript_offset = *(u16*)(*g_iscript_data + image->iscript_header_offset + 0x32);
							image->wait = 0;
							image->return_address = 0;
							helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
						}
					}

					if (building->status & UnitStatus::Frozen) {
						for (Image* image = building->sprite->images.head; image; image = image->link.next) {
							if (image->flags & ImageFlags::Flag04) {
								image->animation = IscriptAnimation::Disable;
								image->iscript_offset = *(u16*)(*g_iscript_data + image->iscript_header_offset + 0x38);
								image->wait = 0;
								image->return_address = 0;
								helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
							}							
						}
					}

					helper::update_unit_damage_overlay(building);
				}
			} break;
		}
	}
}