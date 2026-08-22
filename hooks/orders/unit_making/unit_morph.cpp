#include <BW/api.h>
// copied from unhooked
#include <BW/bwdata.h>
#include <BW/enumerations/TechId.h>
#include <cassert>
#include "hooks\orders\building_making\building_zerg.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\ai\ai_orders.h"
#include "hooks\unit_stats\max_energy.h"
#include "globals.h"
#include "hooks\interface\uifeatures\hotkeys.h"
#include "hooks\orders\building_making\building_modifier.h"

namespace hooks {
	void orders_zerg_birth(Unit* unit) {
		// DF - Fixes hatching eggs not playing a death animation
		if (unit->hitpoints == 0) {
			helper::unit_completion_init(unit, 0);
			helper::orders_self_destructing(unit);
			return;
		}

		auto& unit_ext = unit->ext();
		if (!(unit->order_signal & 4)) {
			Image* current_image = unit->sprite->main_graphic;
			IscriptAnimation::Enum animation = IscriptAnimation::SpecialState1;
			if (unit_ext.values[ValueSpecialSovrolethBirth]) {
				animation = IscriptAnimation::Unused2;
			}
			if (current_image->animation != animation) {
				current_image->play_iscript_animation(animation);
			}
			if (!unit_ext.values[ValuePlayingHatchSound]) {
				unit_ext.values[ValuePlayingHatchSound] = true;
				bw::play_sound(SoundId::Zerg_Egg_ZEgRdy00_WAV, unit);
			}
		}

		if (unit->order_signal & 4) {
			unit->order_signal &= ~4;

			v2u16 pos = {};
			bool no_two_units = false;
			if (units_dat::base_property[unit->create_origin] & UnitProperty::TwoUnitsIn1Egg // Fixes crash when cancelling a unit morphed from a dual birth source. -Pr0nogo
			||  unit_ext.values[ValueSpecialSovrolethBirth]
			||  unit_ext.values[ValueCustomEggActive])
			{
				no_two_units = true;
			}

			bool offset_unit = true;
			if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg || unit->previous_unit_type == UnitId::ZergCocoon) {
				offset_unit = false;
			}
			if (unit_ext.values[ValueSpecialSovrolethBirth]) {
				offset_unit = true;
			}
			unit_ext.values[ValueSpecialSovrolethBirth] = false;

			if (unit_id_is_broodspawn(unit->id)) {
				unit->remove_timer = Broodspawn::LifeTimer;
			}

			Unit* factory = 0;
			if ((unit->create_origin == UnitId::ZergLarva && unit->id != UnitId::ZergLarva)
			||  (unit->create_origin == UnitId::ZergOcsalLarva && unit->id != UnitId::ZergOcsalLarva)) {
				factory = unit->connected_unit;
			}

			u32 extra_count = 0;
			extra_count += unit_id_is_larva(unit->id) ? 0 : unit_ext.values[ValueSwarmhold]; // Done this way to prevent double Larvae on Egg cancel. -Pr0nogo
			if (unit->id != UnitId::ZergLarva && game_kind_doubles_units()) {
				extra_count *= Massacre::UnitMultiplier;
				extra_count++;
			}
			if (unit_id_is_morphed_from_double_unit(unit->id) && unit->create_origin == UnitId::ZergOcsalLarva) { // Kinda stupid. -Pr0nogo
			//	extra_count++;
			}

			u32 stable_count = extra_count;
			Unit* twin_unit = 0;
			if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg && !no_two_units) {
				twin_unit = helper::zerg_dual_birth(unit);
				if (twin_unit) {
					auto& twin_unit_ext = twin_unit->ext();
					if (stable_count) {
						stable_count--;
						twin_unit_ext.values[ValueRegressionUnstable] = unit_ext.values[ValueRegressionUnstable];
					} else {
						twin_unit_ext.values[ValueRegressionUnstable] = bw::get_zerg_regression(twin_unit) != UnitId::None ? true : false;
					}
					if (unit_id_is_broodspawn(twin_unit->id)) {
						twin_unit->remove_timer = Broodspawn::LifeTimer;
					}
					spread_debuffs(unit, twin_unit);
					twin_unit->rally = unit->rally;
					helper::ai_training_unit(twin_unit, unit);
					inherit_selection(unit, twin_unit);
				}
				extra_count *= 2;
			}

			if (UnitId::is_triple_birth(static_cast<UnitId::Enum>(unit->id))) {
				if (!unit_ext.values[ValueRegressionUnstable]) {
					extra_count += unit_ext.values[ValueSwarmhold] ? 2 : 1;
				}
			}
			for (u32 i = 0; i < extra_count; ++i) {
				Unit* new_unit = bw::create_unit_in_game(unit->id, unit->player, unit->position.x, unit->position.y);
				if (new_unit) {
					spread_debuffs(unit, new_unit);
					new_unit->rally = unit->rally;
					auto& new_ext = new_unit->ext();
					if (stable_count) {
						stable_count--;
						new_ext.values[ValueRegressionUnstable] = unit_ext.values[ValueRegressionUnstable];
					} else {
						new_ext.values[ValueRegressionUnstable] = bw::get_zerg_regression(twin_unit) != UnitId::None ? true : false;
					}
					helper::order_new_unit_to_rally(new_unit, factory);
					if (g_active_player_id == g_local_human_id && unit->sprite->flags & SpriteFlags::Selected) {
						helper::select_new_unit(new_unit);
					}
					inherit_selection(unit, new_unit);
				}
			}

			// This unit has collisionless proxy eggs.
			if (unit->id == UnitId::ZergSovrokor) {
				unit->status &= ~UnitStatus::NoCollide;
			}
			unit_ext.values[ValueSwarmhold] = 0;

			if (!UnitId::is_zerg_worker(static_cast<UnitId::Enum>(unit->id)) && unit->id != UnitId::ZergOvileth && unit->id != UnitId::ZergYblyleth) {
				helper::ai_training_unit(unit, unit);
			}
			helper::unit_completion_init(unit, 0);
			if (unit_ext.values[ValueCustomEggActive]) {
				unit_ext.values[ValueCustomEggActive] = false;
				u32 hp = unit->hitpoints;
				helper::update_unit_stats_and_finish_building(unit);
				unit->set_hitpoints(hp);
			} else {
				helper::update_unit_stats_and_finish_building(unit);
			}
			helper::update_unit_strength(unit);

			pos.x = unit->sprite->position.x;
			pos.y = unit->sprite->position.y;
			if (offset_unit) {
				if (units_dat::movement_flags[unit->id] == MovementFlags_HoverUnit) {
					pos.y -= 7;
				} else if (unit->is_in_air()) {
					pos.y -= AirToGroundDistance;
				}
			}
			helper::fix_target_location(&pos, unit->id);
			if (pos.x != unit->sprite->position.x || pos.y != unit->sprite->position.y) {
				helper::move_unit(unit, pos.x, pos.y);
			}
			if (!bw::is_zerg_egg_new(unit, true, false)) {
				helper::order_new_unit_to_rally(unit, factory);
				unit->rally = {};
				if (twin_unit) {
					helper::order_new_unit_to_rally(twin_unit, factory);
					twin_unit->rally = {};
				}
			}
			helper::force_order_done(unit);
			unit->connected_unit = unit_ext.units[UnitMorphConnectedUnit];

			// TODO: Run this function only for whoever selects this unit. Confirm that this function is actually the right function to run.
			// Fixes the issue where the cancel button persists on a unit when you select an egg through the birth sequence. -Pr0nogo
		//	if (!unit_id_is_larva(unit->id)) {
		//		helper::update_buttonset();
		//	}
		}
	}

	// TODO: Use switch -Veeq7
	void orders_morph_1(Unit* unit) { // 0045DEA0
		auto& unit_ext = unit->ext();
		if (unit->main_order_state == 0) {
			if (bw::is_conventional_unit_morph_source(unit->id, true)) {
				if (!helper::has_supplies_for_unit(unit->build_queue[unit->build_queue_slot], unit->player, 1)) {
					helper::refund_all_queue_slots(unit);
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				} else {
					switch (unit->id) {
						case UnitId::ZergOvileth:
						case UnitId::ZergIroleth:
						case UnitId::ZergOthstoleth: {
							break;
						}
						default: {
							helper::increment_unit_scores_ex(unit, -1, 0);
						} break;
					}
					unit->status &= ~UnitStatus::Completed;
					
					u16 egg_id = UnitId::None;
					switch (unit->id) {
						case UnitId::ZergLarva:
						case UnitId::ZergOcsalLarva:
						case UnitId::ZergDroleth:
						case UnitId::ZergYblyleth:
						case UnitId::ZergZethrokor:
						case UnitId::ZergSovrokor:
						case UnitId::ZergQuazilisk:
						case UnitId::ZergQuazrokor:
						case UnitId::ZergCikralisk:
						case UnitId::ZergProtathalor:
						case UnitId::ZergAxitrilisk:
						case UnitId::ZergMortothrokor:
						case UnitId::ZergHydralisk:
						case UnitId::ZergHydrokor:
						case UnitId::ZergHydraliskOrkoti:
						case UnitId::ZergIzirokor:
						case UnitId::ZergRozirokor:
						case UnitId::ZergIziralisk:
						case UnitId::ZergKagralisk:
						case UnitId::ZergEvigrilisk:
						case UnitId::ZergKeskathalor:
						case UnitId::ZergZoryusthaleth:
						case UnitId::ZergInzorthrokor:
						case UnitId::ZergLakizilisk:
						case UnitId::ZergKonvilisk:
						case UnitId::ZergVilgorokor:
						case UnitId::ZergIsthrathaleth:
						case UnitId::ZergElthrazalisk: {
							egg_id = UnitId::ZergEgg;
						} break;
						case UnitId::ZergOvileth:
						case UnitId::ZergOthstoleth:
						case UnitId::ZergNathrokor:
						case UnitId::ZergNathrelisk:
						case UnitId::ZergMutalisk:
						case UnitId::ZergSkithrokor:
						case UnitId::ZergAlmaksalisk:
						case UnitId::ZergGeszithalor:
						case UnitId::ZergGorgrokor:
						case UnitId::ZergAzanthrokor:
						case UnitId::ZergMatraleth: {
							egg_id = UnitId::ZergCocoon;
						} break;
						default: {
							return;
						}  break;
					}

					if (units_dat::base_property[unit->id] & UnitProperty::Spellcaster) {
						unit_ext.values[ValuePreMorphEnergy] = unit->energy;
					} else {
						unit_ext.values[ValuePreMorphEnergy] = hooks::get_unit_max_energy(unit); // New starting amount
					}
					unit->create_origin = unit->id;
					unit_ext.units[UnitMorphConnectedUnit] = 0;
					if (unit->connected_unit) {
						unit_ext.values[ValueMorphConnectedUnitId] = unit->connected_unit->id;
					} else { 
						unit_ext.values[ValueMorphConnectedUnitId] = UnitId::ZergHachirosk; // Dunno if connected_unit will ever be null at this time. -Pr0nogo
					}

					if (unit->unit_ai && unit->unit_ai->type == AiType::Worker) {
						AiTown* town = unit->unit_ai->worker_ai.town;
						helper::remove_worker_or_building_ai(unit, false);
						helper::replace_unit_with_id(unit, egg_id); // morph into egg from source unit
						if (egg_id == UnitId::ZergEgg) {
							helper::add_worker_structure_ai(unit, town);
						} else {
							bw::printf("There is no code for cocoon building AI in internal game code, this shouldn't be called!");
						}
					} else {
						helper::replace_unit_with_id(unit, egg_id); // morph into egg from source unit
					}
					helper::change_unit_buttonset(unit, unit->id);
					unit->remaining_build_time = units_dat::time_cost[unit->build_queue[unit->build_queue_slot]];
					unit->main_order_state = 1;

					u32 origin = unit->create_origin;
					if (egg_id == UnitId::ZergEgg || egg_id == UnitId::ZergCocoon) {
						u32 egg_image_id = get_mutate_image_id(origin);
						if (egg_image_id != ImageId::None) {
							helper::replace_sprite_images(unit->sprite, egg_image_id, 0);
						}
					}

					// TODO: Replace the switch statement with unit->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);
					// Requires all units to use separate grps for mutation animations. -Pr0nogo
					IscriptAnimation::Enum animation = IscriptAnimation::Init;
					switch (origin) {
						case UnitId::ZergLarva:
						case UnitId::ZergOcsalLarva: {
							animation = IscriptAnimation::Walking;
						} break;
						case UnitId::ZergDroleth: {
							animation = IscriptAnimation::AirAttkInit;
						} break;
						case UnitId::ZergZethrokor:
						case UnitId::ZergSovrokor: {
							animation = IscriptAnimation::Unused1;
						} break;
						// New mutation animation handling! Everything will eventually use this. -Pr0nogo
						case UnitId::ZergYblyleth:
						case UnitId::ZergQuazilisk:
						case UnitId::ZergQuazrokor:
						case UnitId::ZergNathrokor:
						case UnitId::ZergCikralisk:
						case UnitId::ZergProtathalor:
						case UnitId::ZergHydralisk:
						case UnitId::ZergHydrokor:
						case UnitId::ZergIzirokor:
						case UnitId::ZergRozirokor:
						case UnitId::ZergIziralisk:
						case UnitId::ZergSkithrokor:
						case UnitId::ZergKagralisk:
						case UnitId::ZergEvigrilisk:
						case UnitId::ZergAlmaksalisk:
						case UnitId::ZergZoryusthaleth:
						case UnitId::ZergInzorthrokor:
						case UnitId::ZergLakizilisk:
						case UnitId::ZergGorgrokor:
						case UnitId::ZergAzanthrokor:
						case UnitId::ZergVilgorokor:
						case UnitId::ZergGeszithalor: {
							animation = IscriptAnimation::SpecialState1;
						} break;
						case UnitId::ZergUltrokor:
						case UnitId::ZergAlzurokor:
						case UnitId::ZergOvileth: {
							animation = IscriptAnimation::AlmostBuilt;
						} break;
						case UnitId::ZergKonvilisk: {
							animation = IscriptAnimation::Landing;
						} break;
						case UnitId::ZergKeskathalor: {
							animation = IscriptAnimation::LiftOff;
						} break;
						default: {
							animation = IscriptAnimation::Init;
						} break;
					}
					unit->sprite->play_iscript_animation(animation, true);
				}
			}
		} else if (unit->main_order_state == 1) {
			if (unit->id == UnitId::ZergEgg && unit->remaining_build_time < 19) {
				unit->play_iscript_animation(IscriptAnimation::GndAttkInit);
				unit->remaining_build_time = 0;
			}
			if (unit->id == UnitId::ZergCocoon && unit->remaining_build_time < 12) {
				unit->play_iscript_animation(IscriptAnimation::SpecialState1);
				unit->remaining_build_time = 0;
			}
			if (cm::progress_build(unit)) {
				unit->main_order_state = 2;
			}
		} else if (unit->main_order_state == 2) {
			if (unit->order_signal & 4) {
				unit->order_signal &= ~4;
				helper::replace_unit_with_id(unit, unit->build_queue[unit->build_queue_slot]);
				if (units_dat::base_property[unit->id] & UnitProperty::Spellcaster) {
					unit->energy = unit_ext.values[ValuePreMorphEnergy];
				} else {
					unit->energy = 0;
				}

				if (UnitId::is_zerg_worker(static_cast<UnitId::Enum>(unit->id)) || unit->id == UnitId::ZergOvileth) {
					helper::ai_training_unit(unit, unit);
				}

				helper::play_unit_ready_sound(unit);
				unit->build_queue[unit->build_queue_slot] = UnitId::None;
				if (get_construction_graphic(unit) != 0) {
					Image* current_image;
					helper::unit_completion_init(unit, 1);
					current_image = unit->sprite->images.head;
					while (current_image) {
						auto anim = IscriptAnimation::SpecialState1;
						current_image->play_iscript_animation(anim);
						current_image = current_image->link.next;
					}
					unit->order_computer(OrderId::ZergBirth);
				} else {
					helper::update_unit_stats_and_finish_building(unit);
					helper::update_unit_strength(unit);
					if (unit->id == UnitId::ZergEgg && (unit->create_origin == UnitId::ZergLarva || unit->create_origin == UnitId::ZergOcsalLarva)) {
						helper::order_new_unit_to_rally(unit, unit->connected_unit);
					}
				}
			}
		}
	}

	bool unit_can_morph(Unit* unit, u16 morph_unit_id) {
		switch (unit->id) {
			case UnitId::ZergLarva:
			case UnitId::ZergOcsalLarva: {
				if (unit->can_make_unit(morph_unit_id, *g_active_nation_id) == 1) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergDroleth: {
				if (morph_unit_id == UnitId::ZergGosvileth) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergOvileth: {
				if (morph_unit_id == UnitId::ZergIroleth) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergZethrokor: {
				if (morph_unit_id == UnitId::ZergVorvrokor || morph_unit_id == UnitId::ZergZethalor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergSovrokor: {
				if (morph_unit_id == UnitId::ZergGarvrokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergQuazilisk:
			case UnitId::ZergQuazrokor: {
				if (morph_unit_id == UnitId::ZergLiiralisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergNathrokor:
			case UnitId::ZergNathrelisk: {
				if (morph_unit_id == UnitId::ZergKalkalisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergCikralisk: {
				if (morph_unit_id == UnitId::ZergGhitorokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergProtathalor: {
				if (morph_unit_id == UnitId::ZergSovroleth) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMutalisk: {
				if (morph_unit_id == UnitId::ZergTosgrilisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergAxitrilisk: {
				if (morph_unit_id == UnitId::ZergBegrothilisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMortothrokor: {
				if (morph_unit_id == UnitId::ZergOztukrokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergHydralisk: 
			case UnitId::ZergHydrokor: 
			case UnitId::ZergHydraliskOrkoti: {
				if (morph_unit_id == UnitId::ZergBactalisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergIzirokor:
			case UnitId::ZergRozirokor:
			case UnitId::ZergIziralisk: {
				if (morph_unit_id == UnitId::ZergTethzorokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergSkithrokor: {
				if (morph_unit_id == UnitId::ZergVithrilisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKagralisk: {
				if (morph_unit_id == UnitId::ZergCherbrathalor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergEvigrilisk: {
				if (morph_unit_id == UnitId::ZergOkrimnilisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergAlmaksalisk: {
				if (morph_unit_id == UnitId::ZergAlgoztalisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKeskathalor: {
				if (morph_unit_id == UnitId::ZergKethrostalor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergGeszithalor: {
				if (morph_unit_id == UnitId::ZergAlkajelisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergZoryusthaleth:
			case UnitId::ZergInzorthrokor: {
				if (morph_unit_id == UnitId::ZergUltrokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergLakizilisk: {
				if (morph_unit_id == UnitId::ZergZobriolisk) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergGorgrokor:
			case UnitId::ZergAzanthrokor: {
				if (morph_unit_id == UnitId::ZergZarcavrokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKonvilisk: {
				if (morph_unit_id == UnitId::ZergTetcaerokor) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergVilgorokor: {
				if (morph_unit_id == UnitId::ZergIsthrathaleth) {
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMatraleth: {
				if (morph_unit_id == UnitId::ZergAkistrokor) {
					return true;
				}
				return false;
			} break;
			default: {
				return false;
			}
		}
	}

	bool is_rallyable_egg_unit(u16 unit_id) {
		if (bw::is_zerg_egg_new(0, false, false, unit_id)) {
			return false;
		}
		return true;
	}

	// Return the ID of the egg unit to use when morphing @p unit_id. If the unit cannot morph, return UnitId::None.
	u16 get_unit_morph_egg_id(u16 unit_id, Unit* unit, u16 resultId) {
		if (units_dat::base_property[unit_id] & UnitProperty::Flyer) {
			return UnitId::ZergCocoon;
		} else {
			return UnitId::ZergEgg;
		}
		return UnitId::None;
	}

	// Determine the type (unit ID) of the unit to revert to when cancelling an @p egg_unit while it is morphing.
	u16 get_cancel_morph_revert_id(Unit* egg_unit) {
		if (egg_unit->get_race() != RaceId::Zerg || (units_dat::base_property[egg_unit->id] & UnitProperty::Building)) {
			return UnitId::None;
		}
		u32 source_id = egg_unit->create_origin;
		auto& unit_ext = egg_unit->ext();
		bool is_ocsal = unit_id_get_larva_type(unit_ext.values[ValueMorphConnectedUnitId]) == UnitId::ZergOcsalLarva;
		if (unit_id_is_larva(source_id)) {
			if (is_ocsal) {
				source_id = UnitId::ZergOcsalLarva; // For cases where you cancel eggs after ocsal mutation completes. -Pr0nogo
			}
			if (egg_unit->connected_unit) {
				egg_unit->connected_unit->count_connected_larva();
				if (*g_larva_counter < 3 && !egg_unit->is_in_air()) {
					return source_id;
				}
				return UnitId::None;
			}
		}
		if (!is_ocsal) {
			return source_id;
		}
		return UnitId::None;
	}

	// Determines the vertical (Y) offset by which the @p unit will be shifted to when it finishes morphing.
	s16 get_unit_vertical_offset_on_birth(Unit* unit) {
		// No offset, birth offset is handled elsewhere
		if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg) {
			return 0;
		}
		// No offset, since the morphed unit should stay where it is
		if (unit->previous_unit_type == UnitId::ZergCocoon) {
			return 0;
		}
		// Hovering units (?) f32 7 pixels above inside
		if (units_dat::movement_flags[unit->id] == MovementFlags_HoverUnit) {
			return -7;
		}
		// Air units f32 42 pixels above inside
		if (unit->status & UnitStatus::InAir) {
			return -AirToGroundDistance;
		}
		// Default for inside units
		return 0;
	}

	Unit* begin_train(Unit* unit, u16 unit_id, bool notify_error) {
		if (unit->status & UnitStatus::GroundedBuilding || unit->id == UnitId::ProtossSolarion || bw::is_larval_egg(unit)) {
			if (helper::has_supplies_for_unit(unit_id, unit->player, notify_error)) {
				Unit* result = helper::create_unit(unit_id, unit->get_x(), unit->get_y(), unit->player);
				if (result) {
					return result;
				}
				helper::display_last_network_error_for_player(unit->player);
			}
		}
		return 0;
	}

	// Hooked to change construction animations contextually, for Zerg morphs. -Pr0nogo
	Image* unit_completion_init(Unit* unit, u32 has_construction_anim) { // 004E65E0
		u32 status = ((unit->status) >> 8) & 3; // UnitStatus::RequiresDetection | UnitStatus::Cloaked
		u8 draw_func1 = 0;
		u8 draw_func2 = 0;
		Image* main_graphic;
		if (status) {
			main_graphic = unit->sprite->main_graphic;
			draw_func1 = main_graphic->draw_fxn.step0;
			draw_func2 = main_graphic->draw_fxn.step1;
		}

		Unit* iscript_unit = *g_active_iscript_unit;
		*g_active_iscript_unit = unit;
		u32 construction_id = get_construction_graphic(unit);
		if (!has_construction_anim || construction_id == 0) {
			construction_id = sprites_dat::image_id[unit->sprite->id];
		}

		helper::replace_sprite_images(unit->sprite, construction_id, unit->current_direction_1);
		*g_active_iscript_unit = iscript_unit;

		if (status) {
			helper::make_cloaked(draw_func2, unit->sprite, status, draw_func1);
			if (*g_player_visions & unit->detection_status) {
				helper::make_sprite_visible(unit->sprite);
			}
		}
		helper::apply_unit_effects(unit);
		main_graphic = unit->sprite->main_graphic;
		unit->damage_overlay_state = g_building_overlay_state_max[main_graphic->id];
		return main_graphic;
	}

	// TODO: Eventually, hook 0045D910 zerg_dual_birth to fix initial facing direction of non-Zethrokor dual birth units. -Pr0nogo
}