#include "update_unit_state.h"
#include "update_status_effects.h"
#include <BW/api.h>
#include <algorithm>
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapon_effect.h"
#include "hooks/tactical_ai.h"
#include "globals.h"
#include "BW\UnitFinder.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/unit_destructor_special.h"
#include "hooks/update_unit_sight.h"
#include <hooks\unit_stats\supply_hooks.h>

namespace hooks {
	void update_unit_energy(Unit* unit) { // 004EB4B0
		auto& unit_ext = unit->ext();
		bool dont_regen = !unit->is_valid_caster() || !unit->is_completed() || unit_ext.energy_regen_timer <= 2 * 24 || unit->is_frozen();

		// Behaviors code -Veeq7
		// Block regen for channeling Facades
		if (unit->id == UnitId::TerranFacade && unit_ext.timers[TimerFacadePryingEyesChannel]) {
			dont_regen = true;
		}

		// Regen code -Veeq7
		s32 max_energy = unit->get_max_energy();

		u32 overlay_id = ImageId::EnergyRegenOverlaySmall + bw::get_overlay_offset(unit->id, true);
		if (!dont_regen && unit->energy < max_energy) {
			unit->heal_energy(10 * 256 / 24, unit);
			if (unit->energy > max_energy) {
				unit->energy = max_energy;
			}

			// If the unit is currently selected, redraw its graphics
			if (unit->sprite->flags & SpriteFlags::Selected) {
				helper::set_all_image_group_flags_palette_11(unit->sprite);
			}

			if (!unit->get_overlay(overlay_id)) {
				bw::add_unit_overlay(unit, overlay_id);
				helper::play_sound_from_unit(SoundId::EnergyRegenStart, unit, 1, 0);
			}
		} else if (unit->get_overlay(overlay_id)) {
			Image* overlay = unit->get_overlay(overlay_id);
			overlay->play_iscript_animation(IscriptAnimation::Death);
			helper::play_sound_from_unit(SoundId::EnergyRegenEnd, unit, 1, 0);
		}
	}

	static bool wyvern_can_fire_at(Unit* unit, Unit* target) {
		if (!target->sprite || !bw::can_weapon_target_unit(WeaponId::WyvernJacksonCannon, target, unit) || !unit->can_attack_target(target)) {
			return false;
		}
        if (unit->get_distance_to_target(target) > weapons_dat::max_range[WeaponId::WyvernJacksonCannon]) {
			return false;
		}

		const Vec32f facing_dir = bw::facing_dir(unit);
		const Vec32f dir_to_target = bw::dir_between(unit->position, target->position);
		const f32 dot = bw::dot_product(facing_dir, dir_to_target);
		if (dot <= bw::deg_to_dot(225.0 / 2)) { // 225 in total, 112.5 degrees in both directions 
			return false;
		}

		return true;
	}

	// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	void update_unit_state(Unit* unit) { // 004EC290
		// Timers
		if (unit->main_order_timer) {
			unit->main_order_timer--;
		}
		if (unit->ground_weapon_cooldown) {
			unit->ground_weapon_cooldown--;
		}
		if (unit->air_weapon_cooldown) {
			unit->air_weapon_cooldown--;
		}
		if (unit->spell_cooldown) {
			unit->spell_cooldown--;
		}

		auto& unit_ext = unit->ext();
		if (unit->id == UnitId::TerranWyvern) {
			Unit* turret = unit->subunit;

			// Wyvern - clear target if owner becomes allied
			if (unit_ext.units[UnitWyvernTargetLock]) {
				Unit* target = unit_ext.units[UnitWyvernTargetLock];
				if (target->ext().timers[TimerOwnerChangedRecently] && bw::is_allied_to(unit->player, target->player)) {
					unit_ext.units[UnitWyvernTargetLock] = 0;
				}
			}

			// Wyvern Reverse Thrust - fire while moving
			if (unit->status & UnitStatus::Completed && unit_ext.timers[TimerWyvernAttackCooldown] == 0) {
				u32 range = weapons_dat::max_range[WeaponId::WyvernJacksonCannon];
				s32 forward_offset = weapons_dat::forward_offset[WeaponId::WyvernJacksonCannon];
				u32 x = unit->get_x() + bw::get_polar_x(forward_offset, unit->current_direction_1);
				u32 y = unit->get_y() + bw::get_polar_y(forward_offset, unit->current_direction_1);

				bool seek_unit = true;
				if (unit_ext.values[ValueWyvernLandGrabOn] == 1) {
					u32 targ_x = unit_ext.values[ValueWyvernLandGrabX];
					u32 targ_y = unit_ext.values[ValueWyvernLandGrabY];
					s32 direction = bw::get_angle(x, y, targ_x, targ_y);
					if (bw::distance_between(unit->position.x, unit->position.y, targ_x, targ_y) <= range) {
						u32 sound = bw::rand_between(SoundId::WyvernWeaponfire01, SoundId::WyvernWeaponfire02);
						helper::play_sound_from_unit(sound, unit, 1, 0);
						bw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, turret->player, turret, 0, targ_x, targ_y);
						unit_ext.timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
						turret->order_target.pt.x = targ_x;
						turret->order_target.pt.y = targ_y;
						turret->order_target.unit = 0;
						seek_unit = false;
					} else {
						unit_ext.values[ValueWyvernLandGrabOn] = 0;
					}
				}

				if (seek_unit) {
					if (turret->order_target.unit
					&&  turret->main_order_id != OrderId::Follow
					&&  turret->main_order_id != OrderId::Patrol
					&&  turret->main_order_id != OrderId::WyvernLandGrab) {
						Unit* new_target = turret->order_target.unit;
						if (wyvern_can_fire_at(unit, new_target)) {
							unit_ext.units[UnitWyvernTargetLock] = new_target;
						}
					}
					Unit* target = unit_ext.units[UnitWyvernTargetLock];
					if (!target || !wyvern_can_fire_at(unit, target)) {
						auto targets = g_unit_search.radial_ex(unit, range, FilterEnemy)
                            .sort_by_distance(unit->position);

						for (Unit* it : targets) {
							if (it == unit) {
								continue;
							}
							if (!wyvern_can_fire_at(unit, it)) {
								continue;
							}
							// TODO: add direction check
							target = unit;
							break;
						}
					}
					
					if (target && wyvern_can_fire_at(unit, target)) {
						if (unit_ext.values[ValueWyvernWeaponCharging]) {
							if (!unit_ext.timers[TimerWyvernWeaponCharge]) {
								unit_ext.values[ValueWyvernWeaponCharging] = false;

								s32 direction = bw::get_angle(x, y, target->get_x(), target->get_y());
								bw::play_sound({SoundId::WyvernWeaponfire01, SoundId::WyvernWeaponfire02}, unit);
								bw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, turret->player, turret, target, target->position);
								unit_ext.timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
							}
						} else {
							unit_ext.values[ValueWyvernWeaponCharging] = true;
							unit_ext.timers[TimerWyvernWeaponCharge] = 12;
							bw::play_sound({SoundId::WyvernWeaponCharge01, SoundId::WyvernWeaponCharge03}, unit);
						}
					}
				}
			}
		}

		// Clear Cleric healing field every tick
		unit->is_being_healed = 0;

		// Cowardice slow flag
		if (unit->new_flags_3 & 0x10) {
			unit->new_flags_3 &= ~0x10;
			unit->update_speed();
		}

		// Something to do with player-specific Signify overlays?
		if (unit->new_flags_3 & 0x20 && unit == players_selections->unit[*g_active_nation_id][0]) {
			unit->new_flags_3 &= ~0x20;
			bw::refresh_console();
		}

		// Flash shielding (regeneration)
		if (units_dat::shields_enabled[unit->id] && unit->is_completed()) {
			if (!unit_ext.values[ValueIsShade] && unit_id_has_flash_shielding(unit->id)) {
				s32 max_shields = (s32)(units_dat::max_shield_points[unit->id]) * 256;

				u32 overlay_id = ImageId::RechargeShields_Small;
				switch (bw::get_overlay_offset(unit->id, true)) {
					case 1: {
						overlay_id = ImageId::RechargeShieldsCustom_Medium;
					} break;
					case 2: {
						overlay_id = ImageId::RechargeShields_Large;
					} break;
					case 3: {
						overlay_id = ImageId::RechargeShieldsCustom_XLarge;
					} break;
				}

				static constexpr u32 start_time = 2 * 24;
				if (unit->shields < max_shields && unit->id != UnitId::ProtossPlasmaShell && unit_ext.shield_regen_timer > start_time && !unit->is_frozen()) {
					static constexpr u32 start_heal_amount = 10 * 256 / 24;
					static constexpr u32 end_heal_amount = 2 * 256 / 24;
					static constexpr u32 change_duration = 5 * 24;
					u32 heal_amount = Map(unit_ext.shield_regen_timer, start_time, start_time + change_duration, start_heal_amount, end_heal_amount, true);
					if (!unit_ext.unit_vectors[UnitVectorCrucibles].empty()) {
                        heal_amount *= 1.25;
					}

					unit->heal_shields(heal_amount, unit, false);
					if (unit->shields > max_shields) {
						if (unit->shields > max_shields) {
							unit->shields = max_shields;
						}
						if (unit->sprite->flags & SpriteFlags::Selected) { // If the unit is currently selected, redraw its graphics
							helper::set_all_image_group_flags_palette_11(unit->sprite);
						}
					}

					if (!unit->get_overlay(overlay_id)) {
						bw::add_unit_overlay(unit, overlay_id);
						bw::play_sound(SoundId::FlashShieldingStart, unit);
					}
				} else if (unit->get_overlay(overlay_id)) {
					bw::remove_unit_overlay(unit, overlay_id);
					if (unit->shields >= max_shields) {
						helper::play_sound_from_unit(SoundId::FlashShieldingEnd, unit, 1, 0);
					} else {
						helper::play_sound_from_unit(SoundId::FlashShieldingInterrupt, unit, 1, 0);
					}
				}
			} else if (unit_id_has_shield_regeneration(unit->id)) {
				if (unit_ext.damage_taken_timer > 2 * 24) {
					unit->heal_shields(21, 0, false, false);
				}
			}
		}

		// Vagrant Nihilism - suffer 2 damage per second until death
		if (unit->id == UnitId::ProtossVagrantShade || unit_ext.values[ValueIsShade]) {
			unit->inflict_damage(21, unit, unit->player, false, (g_elapsed_time_frames) % 6 == 0);
		}

		// Supposedly, allows Zethrokors to attack after unburrowing without delay
		if (unit->id == UnitId::ZergZethrokor) {
			if (unit->ground_weapon_cooldown == 0) {
				unit->order_queue_timer = 0;
			}
		}

		// update unit status effects (stim, maelstrom, plague, etc.)
		if (unit->is_completed() || !unit->sprite->is_hidden()) {
			unit->cycle_counter++;
			if (unit->cycle_counter >= 8) {
				unit->cycle_counter = 0;
				helper::restore_all_unit_stats(unit); // use update_status_effects hook if available
			}
		}

		// Only for fully-constructed units and buildings. -GPTP
		// Modified 24/01/23 to allow Zerg building morph sources to regenerate while mutating. -Pr0nogo
		if (unit->is_completed() ||	unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon || bw::is_zerg_building_morph_result(unit->build_queue[unit->build_queue_slot])) {
			bool stop_here = false;
			if (unit->remove_timer != 0) {
				if (unit->id != UnitId::ZergRilirokor || !bw::get_active_tile_at_coords(unit->position)->has_cagra) {
					unit->remove_timer -= 1;
					if (unit->remove_timer == 0) {
						unit->bwfunc_remove();
						stop_here = true;
					}
				}
			}

			update_unit_energy(unit);

			if (unit->recent_order_timer != 0) {
				unit->recent_order_timer--;
			}

			// Augur - Final Hour
			if (unit->id == UnitId::ProtossAugur && unit->shields == 0) {
				augur_sacrifice(unit);
				stop_here = true;
			}

			// Passive HP regeneration
			if (bw::unit_regenerates_hp(unit) && unit->hitpoints > 0 && unit->hitpoints != unit->get_max_hp_raw()) {
				u32 factor = bw::get_regeneration_factor(unit);
				if (unit_ext.damage_taken_timer > 2 * 24) {
					factor++;
				}

				s32 regen = 11 * factor; // 1 HP per second, before factor.
				if (unit->is_burrowed()) {
					regen *= 2;
				}
				if (unit->id == UnitId::TerranEidolonNydus && unit->is_cloaked) {
					regen *= 1.33;
				}

				bool has_crucible = unit_ext.unit_vectors[UnitVectorCrucibles].size() != 0;
				regen *= 1 + (has_crucible * 0.25);
				unit->heal_hp(regen, 0, false);
			}

			// Deliberate healing
			{
				s32 tick_heal = 0;
				// Alaszileth - Dermal Accelerant - 500% life regen
				if (unit_ext.timers[TimerAlaszilethRegenBoost] > 0) {
					tick_heal += 11 * 5;
				}

				// Cannibal Reintegration - 5 HP per sec
				if (unit_ext.timers[TimerCannibalHealing]) {
					tick_heal += 11 * 5;
				}

				// Azazel Defensive Matrix - heal over time
				if (unit->defensive_matrix_hp != 0) {
					if (unit->hitpoints > 0 && unit->hitpoints / 256 != unit->get_max_hp_ingame()) {
						tick_heal += Azazel::DefensiveMatrixHeal;
					}
				}

				if (tick_heal) {
					unit->heal_hp(tick_heal);
				}
			}

			// Last Orders - Aurora passive
			if (unit->has_last_orders() && unit_ext.values[ValueLastOrdersInit]) {
				auto distance = bw::distance_between(unit->position.x, unit->position.y, unit->order_target.pt.x, unit->order_target.pt.y);
				if (distance < 32) {
					unit->status |= UnitStatus::Invincible;
					// TODO: unit search radial
					auto match = cosmonarchy_finder_match(unit, Flags::Enemy, {});
					const u32 radius = 64;
					bw::UnitFinder(
						unit->get_x() - radius,
						unit->get_y() - radius,
						unit->get_x() + radius,
						unit->get_y() + radius
					).for_each_in_match(match, [&](Unit* target) {
						u32 dist = bw::distance_between(unit->position, target->position);
						u32 damage = 25 * 256;
						if (dist > 48) {
							damage /= 4;
						} else if (dist > 32) {
							damage /= 2;
						}
						target->damage_with_custom(damage, -1, unit, unit->player);
					});
					unit->bwfunc_remove();
					avenge_or_ascend_corpses_in_area(unit->position, 2 * 32, unit->player);
					stop_here = true;
				}
			}

			// Terran building burn-down
			if (!stop_here && !unit_ext.values[ValueInfested] && !unit_ext.timers[TimerBurnDownDelay]) {
				if (unit->get_race() == RaceId::Terran) {
					if (unit->is_grounded_building() || units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding) {
						if (helper::unit_hp_is_in_redzone(unit)) {
							// Manual handling of on-death behaviors. -Pr0nogo
							if (unit->hitpoints <= 20 && unit_ext.timers[TimerInfesting]) {
								blissful_embrace(unit, unit_ext.values[ValueInfestingPlayer]);
								unit->set_hitpoints(100 * 256 + units_dat::max_hitpoints[unit->id] / 3);
							} else {
								unit->damage_hp(20, 0, unit->last_attacking_player, true);
								if (unit->plague_timer && unit->hitpoints <= 256) {
									unit->bwfunc_remove();
								}
							}
						}
					}
				}
			}
		}
	};

	// Hooked to help with the cloak rewrite - Pr0nogo
	void update_units() { // 004ED000
		*g_interceptor_focused_this_frame = 0;

		if (helper::is_selected_unit_group_enabled()) {
			bw::cancel_targeting_and_place_building();
		}

		// Originally had code related to order timers and Disruption Web; removed former, handled latter in globals
		stat_res_refresh_production_arrays();

		Unit* next_unit; // caching next unit instead of a direct read, since some functions might actually modify the link.next
						 // (especially when the unit dies or gets hidden etc)
		for (Unit* unit = *g_first_visible_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::progress_active_unit_frame(unit);
			stat_res_get_production(unit);
		}

		for (Unit* unit = *g_first_dying_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::progress_dying_unit_frame(unit);
			if (unit->sprite) {
				if (*g_vision_updated) {
					if (g_player_table[unit->player].type == PlayerType::Human) {
						hooks::reveal_sight_at_location(1, player_vision->flags[unit->player], unit->position.x, unit->position.y, unit->status & UnitStatus::InAir);
					}
				}
				helper::update_visibility(unit);
			} else {
				unit_death_end(unit);
			}
		}

		if (*g_vision_updated) {
			for (Unit* unit = *g_first_scanner_sweep; unit; unit = next_unit) {
				next_unit = unit->link.next;
				*g_active_iscript_entity = unit;
				*g_active_iscript_unit = unit;
				helper::refresh_unit_vision(unit);
			}
		}

		for (Unit* unit = *g_first_visible_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			UnitExt* unit_ext = get_unit_ext(unit);
			helper::update_visibility(unit);
			if (unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked) || unit_ext->timers[TimerDecloak]) {
				unit->is_cloaked = 0;
				helper::check_unit_visibility(unit);
			}
		}

		for (Unit* unit = *g_first_visible_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::progress_unit_frame(unit);
		}

		for (Unit* unit = *g_first_hidden_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			if (unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked)) {
				unit->is_cloaked = 0;
			}
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::unit_update(unit);
		}

		helper::update_burrowing_cloaking_units();
		helper::update_unit_power();
		s32 lurker_splash_index = (*g_lurker_splash_list_index)++ + 1;
		if (*g_lurker_splash_list_index == 32) {
			lurker_splash_index = 0;
			*g_lurker_splash_list_index = 0;
		}
		memset(&g_lurker_splash_list[32 * lurker_splash_index], 0, 0x80u);
		*g_last_bullet_index = 0;

		for (Unit* unit = *g_first_scanner_sweep; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::progress_unit_frame(unit);
		}

		*g_active_iscript_entity = 0;
		*g_active_iscript_unit = 0;
	}

	// Originally meant to help with the cloak rewrite. Hooked 22 Oct 2023. -Pr0nogo
	void update_burrowing_cloaking_units() { // 00492EB0
		Unit* invisible_unit = *g_first_invisible_unit;
		bool redraw = 0;
		if (*g_first_invisible_unit) {
			while (1) {
				Unit* next_burrowed = invisible_unit->invisible_link.next;
				if (helper::is_unit_cloaking(invisible_unit)) {
					do_cloak(invisible_unit, bw::get_cloak_sound_id(invisible_unit));
				} else {
					u32 status_1 = invisible_unit->status;
					bool cloak_or_burrow;
					if (invisible_unit->is_burrowed() || invisible_unit->secondary_order_id != OrderId::Cloak || invisible_unit->is_cloaked != 1 || !(status_1 & UnitStatus::CloakingForFree)) {
						cloak_or_burrow = 0;
					} else {
						invisible_unit->status = status_1 & 0xFFFFF7FF;
						cloak_or_burrow = 1;
					}
					redraw |= cloak_or_burrow;
					if (invisible_unit->status & UnitStatus::RequiresDetection) {
						goto Next_Or_Redraw;
					}
					helper::begin_cloak(invisible_unit, bw::get_cloak_sound_id(invisible_unit));
				}
				redraw = 1;
			Next_Or_Redraw:
				invisible_unit = next_burrowed;
				if (!next_burrowed) {
					if (redraw) {
						bw::refresh_console();
					}
					return;
				}
			}
		}
	}

	// Hooked for cloak rewrite animations - Pr0nogo
	void do_cloak(Unit* unit, s32 sound_id) { // 0049B440
		bool is_burrowed = unit->is_burrowed();
		if (!is_burrowed) {
			helper::play_sound_from_unit(sound_id, unit, 1, 0);
		}
		if (!g_is_in_replay && unit->player == g_local_nation_id) {
			bw::refresh_console();
		}

		if (is_burrowed) {
			helper::update_sprite_flag_on_palette(unit->sprite);
			helper::show_images_hidden_by_cloak(unit->sprite);
			unit->status &= 0xFFFFFCFF;
		} else {
			helper::update_sprite_images_for_cloak(unit->sprite);
			unit->status = unit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			Unit* subunit = unit->subunit;
			if (subunit) {
				helper::update_sprite_images_for_cloak(subunit->sprite);
				subunit->status = subunit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			}
		}
	}

	// Hooked to add nullcheck - Pr0nogo
	void progress_unit_frame(Unit* unit) { // 004ECF70
		if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit) && AssertSprite(unit->sprite)) {
			if (!(unit->sprite->flags & SpriteFlags::Hidden)) {
				helper::refresh_sprite(unit->sprite, g_selection_circle_color[unit->player]);
			}
		}
		helper::update_unit_state(unit);
		helper::progress_order(unit);
		helper::perform_secondary_orders(unit, unit->player); // ECX at the time of calling this function

		if (unit->subunit) {
			if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit)) {
				*g_active_iscript_unit = unit->subunit;
				progress_unit_frame(unit->subunit);
				*g_active_iscript_unit = unit;
			}
		}
		if (unit->sprite) {
			helper::sprite_to_iscript_loop(unit->sprite);
			if (!unit->sprite->images.head) {
				unit->sprite = 0;
			}
		}
	}

	// Hooked to investigate crashes. -Pr0nogo
	void sprite_to_iscript_loop(Sprite* sprite) { // 00497920
		Image* image = sprite->images.head;
		Image* next_image = 0;
		u32 data = 0; // unused arg of iscript_opcode_cases?
		if (image) {
			do {
				next_image = image->link.next;
				if (!image->update_fxn) { // Resolve null crash, presumably
					return;
				}
				((void(__thiscall*)(Image*))image->update_fxn)(image);
				helper::iscript_opcode_cases(image, &image->iscript_header_offset, 0, 0);
				if (!sprite->images.head) {
					u32 pos_y = sprite->position.y / 32;
					if (pos_y >= 0) {
						if (pos_y >= *g_map_tile_height) {
							pos_y = *g_map_tile_height - 1;
						}
					} else {
						pos_y = 0;
					}

					if (g_horizontal_sprite_lines_begin[pos_y] == sprite) {
						g_horizontal_sprite_lines_begin[pos_y] = sprite->link.next;
					}
					if (g_horizontal_sprite_lines_end[pos_y] == sprite) {
						g_horizontal_sprite_lines_end[pos_y] = sprite->link.prev;
					}
					if (sprite->link.prev) {
						sprite->link.prev->link.next = sprite->link.next;
					}

					Sprite* next_sprite = sprite->link.next;
					if (next_sprite) {
						next_sprite->link.prev = sprite->link.prev;
					}

					sprite->link.prev = 0;
					sprite->link.next = 0;
					Sprite* first_free = g_free_sprites->head;
					if (g_free_sprites->head) {
						if (g_free_sprites->tail == g_free_sprites->head) {
							g_free_sprites->tail = sprite;
						}
						sprite->link.prev = g_free_sprites->head;
						sprite->link.next = first_free->link.next;
						Sprite* next_free = first_free->link.next;
						if (next_free) {
							next_free->link.prev = sprite;
						}
						first_free->link.next = sprite;
					} else {
						g_free_sprites->head = sprite;
						g_free_sprites->tail = sprite;
					}
				}
				image = next_image;
			} while (next_image);
		}
	}

	// Hooked to improve functionality. -Pr0nogo
	u8 replace_unit_with_id(Unit* unit, u32 new_id) { // 0049FED0
		if (!AssertSprite(unit->sprite)) {
			return false;
		}

		if (!unit->sprite->is_hidden()) {
			helper::sort_unit(unit);
			if (unit->is_grounded_building()) {
				helper::clear_building_from_tile_flag(unit, unit->position.x, unit->position.y);
			}
			if (unit->is_in_air()) {
				helper::remove_repulse_tile(unit);
			}
		}
		helper::transform_unit_sprite(unit, new_id);

		if (unit->is_building()) {
			if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
				if (unit->secondary_order_id != OrderId::Nothing2) {
					unit->secondary_order_id = OrderId::Nothing2;
					unit->secondary_order_pos = { 0, 0 };
					unit->current_build_unit = 0;
					unit->secondary_order_state = 0;
				}
				unit->status &= ~(UnitStatus::Cloaked | UnitStatus::RequiresDetection);
			}
		} else {
			u8 direction = unit->current_direction_1 = unit->velocity_direction_1 = unit->current_direction_2;
			helper::set_image_direction(unit->sprite->main_graphic, direction);
		}

		if (unit->sprite->is_selected()) {
			if (!helper::unit_is_standard_and_movable(unit) && g_client_selection_count > 1 && new_id != UnitId::ZergEgg) {
				helper::update_building_land_unit_selection(0, unit);
				helper::remove_from_client_selection_3(unit);
			}
		}
		bw::refresh_console();
		u32 pos_x = unit->position.x;
		u32 pos_y = unit->position.y;
		Point16 position = {pos_x, pos_y};
		helper::fix_target_location(&position, new_id);
		if (pos_x != position.x || pos_y != position.y) {
			helper::move_unit(unit, position.x, position.y);
		}
		if (!unit->sprite->is_hidden()) {
			helper::update_unit_order_finder(unit);
			if (unit->is_grounded_building()) {
				helper::set_building_tile_flag(unit, unit->position.x, unit->position.y);
			}
			helper::check_unstack(unit);
			if (unit->is_in_air()) {
				helper::set_repulse_angle(unit);
			}
		}

		Unit* subunit = unit->subunit;
		if (subunit) {
			helper::transform_subunit(subunit, units_dat::subunit[new_id]);
		}
		return helper::apply_unit_effects(unit);
	}

	// Hooked to change the sound id for permacloaked units on initial creation. -Pr0nogo
	u32 update_unit_strength_and_apply_default_orders(Unit* unit) { // 0049FA40
		if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg) {
			helper::increment_unit_scores(unit, -1);
			unit->status |= UnitStatus::Completed;
			helper::increment_unit_scores(unit, 1);
		} else {
			unit->status |= UnitStatus::Completed;
		}
		helper::increment_unit_scores_ex(unit, 1, 1);

		UnitExt* unit_ext = get_unit_ext(unit);
		if (!unit_ext->values[ValueNoMassacreResource]) {
			if (game_kind_doubles_units() && unit_should_give_gas_in_massacre_mode(unit)) {
				bw::add_massacre_resource(unit->player);
			}
		} else {
			unit_ext->values[ValueNoMassacreResource] = 0;
		}

		if (units_dat::base_property[unit->id] & UnitProperty::PermanentCloak
		||	unit_ext->values[ValueTriggerBehaviorProximityCloak]
		||	unit_ext->values[ValueTriggerBehaviorSteadyShroud])
		{
			helper::begin_cloak(unit, bw::get_cloak_sound_id(unit));
		}
		if (unit->id == UnitId::Spell_ScannerSweep || unit->id == UnitId::Special_MapRevealer) {
			if (*g_first_hidden_unit == unit) {
				*g_first_hidden_unit = unit->link.next;
			}
			if (*g_last_hidden_unit == unit) {
				*g_last_hidden_unit = unit->link.prev;
			}
			if (unit->link.prev) {
				unit->link.prev->link.next = unit->link.next;
			}
			Unit* next_unit = unit->link.next;
			if (next_unit) {
				next_unit->link.prev = unit->link.prev;
			}
			unit->link.prev = 0;
			unit->link.next = 0;
			Unit* first_scanner = *g_first_scanner_sweep;
			if (*g_first_scanner_sweep) {
				if (*g_last_scanner_sweep == *g_first_scanner_sweep) {
					*g_last_scanner_sweep = unit;
				}
				unit->link.prev = *g_first_scanner_sweep;
				unit->link.next = first_scanner->link.next;
				Unit* next_scanner = first_scanner->link.next;
				if (next_scanner) {
					next_scanner->link.prev = unit;
				}
				first_scanner->link.next = unit;
				helper::refresh_unit_vision(unit);
			} else {
				*g_last_scanner_sweep = unit;
				*g_first_scanner_sweep = unit;
				helper::refresh_unit_vision(unit);
			}
		} else if (unit->sprite->is_hidden() && !unit_id_is_hangar_unit((UnitId::Enum)unit->id)) {
			helper::show_and_enable_unit(unit);
		}
		// Originally had trap code

		u32 player_type = g_player_table[unit->player].type;
		if (player_type == PlayerType::Rescuable) {
			u32 main_order_id = unit->main_order_id;
			unit->user_action_flags |= UserActionFlags::UserActionFlags_IscriptGoToIdle;
			if (main_order_id) {
				while (1) {
					Order* tail = unit->order_queue_tail;
					if (!tail) {
						break;
					}
					u32 tail_order_id = tail->order_id;
					if (!orders_dat::can_be_interrupted[tail_order_id] && tail_order_id != OrderId::RescuePassive) {
						break;
					}
					helper::remove_order_from_unit_queue(tail, unit);
				}
				helper::perform_another_order(OrderId::RescuePassive, 0, unit, UnitId::None, {}, 0);
			}
			helper::do_next_queued_order(unit);
		} else {
			u32 new_order_id;
			if (player_type == PlayerType::Neutral) {
				new_order_id = OrderId::Neutral;
			} else if (player_type == PlayerType::Computer) {
				new_order_id = units_dat::computer_idle_order[unit->id];
			} else {
				new_order_id = units_dat::human_idle_order[unit->id];
			}
			helper::issue_order_targeting_nothing(unit, new_order_id);
		}

		if (units_dat::base_property[unit->id] & UnitProperty::NeutralAccessories) {
			Sprite* sprite = unit->sprite;
			unit->powerup_origin.x = sprite->position.x;
			unit->powerup_origin.y = sprite->position.y;
			u32 idle_order = units_dat::human_idle_order[unit->id];
			unit->user_action_flags |= 1;
			helper::order(1, unit, idle_order, {}, 0, 228);
			helper::do_next_queued_order(unit);
		}
		unit->air_strength = helper::get_unit_strength(unit, unit->id, 0);
		u32 strength = helper::get_unit_strength(unit, 1);
		unit->ground_strength = strength;
		return strength;
	}

	// Hooked to investigate crashes. -Pr0nogo
	// Not tested yet. Low-priority, because the only time it crashes is when you spawn units for a player that has no start location.
	u32 initialize_unit_base(Unit* unit, u32 unit_id, u32 pos_x, u32 pos_y, u32 player) { // 004A0320
		unit->powerup_origin.x = 0;
		unit->powerup_origin.y = 0;
		unit->rally.pt.x = 0;
		unit->rally.pt.y = 0;
		unit->rally.unit = 0;
		unit->invisible_link.prev = 0;
		unit->invisible_link.next = 0;
		unit->order_queue_head = 0;
		unit->order_queue_tail = 0;
		unit->unused_86 = 0;
		unit->remove_timer = 0;
		unit->defensive_matrix_hp = 0;
		unit->defensive_matrix_timer = 0;
		unit->stim_timer = 0;
		unit->ensnare_timer = 0;
		unit->lockdown_timer = 0;
		unit->irradiate_timer = 0;
		unit->stasis_timer = 0;
		unit->plague_timer = 0;
		unit->status = 0;
		unit->user_action_flags = 0;
		unit->pathing_flags = 0;
		unit->previous_hitpoints = 1;
		unit->unit_ai = 0;

		u32 initialized = helper::initialize_unit(unit, pos_y, unit_id, pos_x, player);
		if (initialized) {
			*(u32*)unit->build_queue = 0xE400E4;
			*(u32*)&unit->build_queue[2] = 0xE400E4;
			unit->build_queue[4] = UnitId::None;
			u8 unique_id_something = unit->secondary_order_id & OrderId::BuildProtoss1;// secondary_order_special
			unit->build_queue_slot = 0;
			unit->unique_id = unique_id_something;
			if (!units_dat::group_flags[unit->id].is_factory) {
				if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building) {
					unit->current_buttonset = UnitId::Buildings;
				}
			} else {
				if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building) {
					unit->current_buttonset = UnitId::Factories;
				}
			}

			u32 random = bw::random();
			s32 current_hp = units_dat::max_hitpoints[unit_id] / 10;
			unit->wireframe_randomizer = random;
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
				current_hp = 1;
			}
			unit->hitpoints = current_hp;
			
			u32 main_order_id;
			if ((unit->status & UnitStatus::GroundedBuilding) == 0) {
				main_order_id = OrderId::Nothing2;
			} else {
				main_order_id = units_dat::human_idle_order[unit_id];
			}

			unit->main_order_id = main_order_id;
			unit->order_target.unit = 0;
			unit->order_target.pt.y = 0;
			unit->order_target.pt.x = 0;
			unit->main_order_state = 0;
			unit->order_unit_id = UnitId::None;
			if (unit->secondary_order_id != OrderId::Nothing2) {
				unit->secondary_order_id = OrderId::Nothing2;
				*(u32*)(unit+0xEA) = 0; // secondary_order_unk in Neiv's list
				unit->secondary_order_pos.x = 0;
				unit->secondary_order_pos.y = 0;
				unit->current_build_unit = 0;
				unit->secondary_order_state = 0;
			}
			unit->finder_index.left = -1;
			unit->finder_index.right = -1;
			unit->finder_index.top = -1;
			unit->finder_index.bottom = -1;
			unit->player_link.prev = 0;
			unit->player_link.next = first_player_unit->unit[player];

			Unit* first_unit = first_player_unit->unit[player];
			if (first_unit) {
				first_unit->player_link.prev = unit;
			}
			first_player_unit->unit[player] = unit;
			helper::increment_unit_scores(unit, 1);

			if (unit->status & UnitStatus::GroundedBuilding) {
				helper::update_unit_order_finder(unit);
				if (unit->status & UnitStatus::GroundedBuilding) {
					helper::set_building_tile_flag(unit, unit->sprite->position.x, unit->sprite->position.y);
				}
				helper::check_unstack(unit);
				if (unit->status & UnitStatus::InAir)
				{
					helper::set_repulse_angle(unit);
				}
				helper::unit_completion_init(unit, 1);
				helper::refresh_all_visible_images_at_screen_pos(unit->sprite, 0);
				if (g_is_placing_building) {
					helper::place_building_proc();
				}
			} else {
				if (unit_id_is_mine_layer((UnitId::Enum)unit->id)) {
					set_up_mines(unit);
				}
				unit->sprite->flags |= SpriteFlags::Hidden;
				helper::refresh_all_visible_images_at_screen_pos(unit->sprite, 0);
			}

			unit->detection_status = -1;
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
				unit->sprite->flags |= SpriteFlags::RemoveAfterDeath;
				return true;
			} else {
				if (!(unit->sprite->flags & SpriteFlags::Hidden)) {
					helper::refresh_unit_vision(unit);
				}
				return true;
			}
		}
		return initialized;
	}

	// Hooked to investigate crashes. -Pr0nogo
	b32 set_thingy_visibility_flags(Thingy* thingy) { // 004878F0
		Sprite* thingy_sprite = thingy->sprite;
		if (thingy_sprite && !(thingy_sprite->flags & SpriteFlags::Hidden)) {
			u8 status = ~(g_active_tile_array[thingy_sprite->position.x / 32 + g_map_tile_size->width * (thingy_sprite->position.y / 32)]->visibility_flags);
			if (thingy_sprite->visibility_status != status) {
				helper::refresh_all_visible_images_at_screen_pos(thingy_sprite, status);
				if ((u8)*g_player_visions & thingy_sprite->visibility_status) {
					if (!(*g_player_visions & thingy->sprite->visibility_status)) {
						return 1;
					}
				}
			}
		}
		return 0;
	}

	// Hooked to improve functionality and fix a few CM-specific bugs. -DF
	b32 initialize_unit(Unit* unit, s32 pos_y, u16 unit_id, s32 pos_x, u8 player_id) { // 0049ECF0
		Unit* old_active_iscript_unit = *g_active_iscript_unit;
		*g_active_iscript_unit = unit;

		if (helper::initialize_flingy(units_dat::graphic[unit_id], pos_y, pos_x, (Flingy*)unit, player_id, 0)) {
			unit->player = player_id;
			unit->main_order_id = OrderId::Fatal;
			unit->main_order_state = 0;
			unit->order_signal = 0;
			unit->order_unit_id = UnitId::None;
			*(u32*)(&unit->main_order_timer) = 0; // Zero-inits the cooldown timers as well.
			unit->order_target = {0, 0, 0};
			u32* union_0xC0 = (u32*)(&unit->mine_carrier); // Not specifically mine carriers, just unit layout offset 0xC0.
			*union_0xC0 = 0;
			*(union_0xC0 + 1) = 0; // 0xC4
			*(union_0xC0 + 2) = 0; // 0xC8
			*(union_0xC0 + 3) = 0; // 0xCC
			unit->id = unit_id;
			unit->resource_type = 0;
			unit->secondary_order_timer = 0;

			if (unit->sprite) {
				helper::sprite_to_iscript_loop(unit->sprite);
				if (!unit->sprite->images.head) {
					unit->sprite = 0;
				}
			}

			*g_active_iscript_unit = old_active_iscript_unit;
			if (!AssertSprite(unit->sprite)) {
				return false; // New
			}

			unit->last_attacking_player = 8;
			unit->shields = units_dat::max_shield_points[unit->id] << 8;
			unit->energy = helper::get_unit_max_energy(unit) / 4;

			unit->ai_action_flags = 0;
			unit->sprite->elevation = units_dat::elevation[unit_id];
			u32 base_property = units_dat::base_property[unit_id];

			if (base_property & UnitProperty::Building) {
				unit->building.tech_id = TechId::None;
				unit->building.upgrade_id = UpgradeId::None;
			}

			u32 unit_status = unit->status;
			unit_status ^= ((base_property << 1) ^ unit_status) & UnitStatus::GroundedBuilding;			//  UnitProperty::Building
			unit_status ^= (base_property ^ unit_status) & UnitStatus::InAir;							//  UnitProperty::Flyer
			unit_status ^= ((base_property >> 12) ^ unit_status) & UnitStatus::CanTurnAroundToAttack;	//  UnitProperty::Attack
			unit_status ^= ((base_property >> 10) ^ unit_status) & UnitStatus::Reacts;					//  UnitProperty::AutoAttackAndMove
			unit_status ^= (~(unit_status << 18) ^ unit_status) & UnitStatus::IsNormal;					// !UnitProperty::Flyer
			unit_status ^= ((base_property >> 3) ^ unit_status) & UnitStatus::Invincible;				//  UnitProperty::Invincible

			unit->status = unit_status;
			unit->pathing_flags ^= (unit->pathing_flags ^ (units_dat::elevation[unit_id] < 12)) & PathingFlags::PathingFlags_ProvidesCollision;
			unit->path = 0;
			unit->movement_state = 0;
			unit->recent_order_timer = 0;
			unit->damage_overlay_state = g_building_overlay_state_max[unit->sprite->main_graphic->id];
			u16 time_cost = units_dat::time_cost[unit_id];
			unit->remaining_build_time = time_cost | !time_cost; // Set time_cost to 1 if it's 0, without branching.
			helper::update_hp_gain(unit);
			helper::update_building_construct_shields_amount(unit);
			helper::apply_speed_upgrade_from_unit_type(unit);
			unit->update_speed();

			// Preserve player color from unit transforms, even after the player leaves or is defeated. -Pr0nogo
			// TODO: Find out how to assign player color to a sprite.
		//	UnitExt* unit_ext = get_unit_ext(unit);
		//	unit_ext->values[ValuePlayerColor];
			return true;
		}
		*g_active_iscript_unit = old_active_iscript_unit;
		return false;
	}
}