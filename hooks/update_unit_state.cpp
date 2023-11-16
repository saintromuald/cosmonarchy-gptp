#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/api.h>
#include <algorithm>
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapon_effect.h"
#include "hooks/tactical_ai.h"
#include "globals.h"
#include "SCBW\UnitFinder.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/unit_destructor_special.h"
#include "hooks/update_unit_sight.h"

namespace hooks
{
	// This function regenerates energy for spellcasters and drains energy for cloaking units.
	void update_unit_energy(Unit* unit) { // 004EB4B0
		UnitExt* unit_ext = get_unit_ext(unit);
		bool dont_regen = !unit->is_valid_caster() || !(unit->status & UnitStatus::Completed) || unit_ext->energy_regen_timer <= 2 * 24;

		// Behaviors code -Veeq7
		// Spend energy for cloaked units
		// Judging by the cost set to zero I assume this this block is not actually used, so I am commenting it out -Veeq7
		/*
		if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection) // If the unit is cloaked and must consume energy to stay cloaked (i.e. not under a Didact)
			&& !(unit->status & UnitStatus::CloakingForFree)) {
			u16 cloaking_energy_cost = 0;
			if (unit->has_energy(cloaking_energy_cost)) {
				unit->spend_unit_energy(cloaking_energy_cost); // The function prevents energy consumption if THE GATHERING is enabled
				dont_regen = true;
			} else if (unit->secondary_order_id == OrderId::Cloak) {
				unit->set_secondary_order(OrderId::Nothing2);  // Supposedly, immediately decloaks the unit.
			}
		}
		*/

		// Spend energy for Gods Made Flesh
	//	if (unit_ext->values[ValueGodsMadeFleshActive]) {
	//		unit->spend_unit_energy(GodsMadeFleshEnergyDrain);
	//		dont_regen = true;
	//	}

		// Spend energy for Phobos Discharge
		if (unit->id == UnitId::TerranPhobos && unit_ext->values[ValuePhobosDischarge] > 0) {
			unit->spend_unit_energy((200 * 256) / (5 * 24));
			dont_regen = true;
			if (unit->energy == 0) {
				unit_ext->values[ValuePhobosDischarge] = 0;
			}
		}

		// Regen code -Veeq7
		s32 max_energy = unit->get_max_energy();

		/*
		// Stupid special case for specialists summoned in this manner. No longer applies since Mind Tyrant was revised. -Pr0nogo
		if (unit->id == UnitId::ProtossMindTyrant
			&& unit->main_order_id == OrderId::CompletingArchonSummon
			&& unit->main_order_state == 0) {
			max_energy = 100 * 256; // Identical to energy amount on spawn
		}
		*/

		u32 overlay_id = ImageId::EnergyRegenOverlaySmall + scbw::get_overlay_offset(unit->id, true);
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
				scbw::add_unit_overlay(unit, overlay_id);
				helper::play_sound_from_unit(SoundId::EnergyRegenStart, unit, 1, 0);
			}
		} else if (unit->sprite->get_overlay(overlay_id)) {
			Image* overlay = unit->get_overlay(overlay_id);
			overlay->play_iscript_animation(IscriptAnimation::Death);
			helper::play_sound_from_unit(SoundId::EnergyRegenEnd, unit, 1, 0);
		}
	}

	static bool wyvern_can_fire_at(Unit* unit, Unit* target) {
		if (target->sprite) {
			const u32 unit_offset = std::max(unit->get_right() - unit->get_left(), unit->get_bottom() - unit->get_top()) + 32;
			return scbw::can_weapon_target_unit(WeaponId::WyvernJacksonCannon, target, unit)
				&& scbw::get_distance_fast(target->position.x, target->position.y, unit->position.x, unit->position.y) <= weapons_dat::max_range[WeaponId::WyvernJacksonCannon] + unit_offset
				&& unit->can_attack_target(target);
		}
		return false;
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

		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->id == UnitId::TerranWyvern) {
			Unit* turret = unit->subunit;

			// Wyvern - clear target if owner becomes allied
			if (unit_ext->units[UnitWyvernTargetLock]) {
				Unit* target = unit_ext->units[UnitWyvernTargetLock];
				UnitExt* target_ext = get_unit_ext(target);
				if (target_ext->timers[TimerOwnerChangedRecently]
				&&  scbw::is_allied_to(unit->player, target->player)) {
					unit_ext->units[UnitWyvernTargetLock] = 0;
				}
			}

			// Wyvern Reverse Thrust - fire while moving
			if (unit->status & UnitStatus::Completed && unit_ext->timers[TimerWyvernAttackCooldown] == 0) {
				u32 range = weapons_dat::max_range[WeaponId::WyvernJacksonCannon];
				s32 forward_offset = weapons_dat::forward_offset[WeaponId::WyvernJacksonCannon];
				u32 x = unit->get_x() + scbw::get_polar_x(forward_offset, unit->current_direction_1);
				u32 y = unit->get_y() + scbw::get_polar_y(forward_offset, unit->current_direction_1);

				bool seek_unit = true;
				if (unit_ext->values[ValueWyvernLandGrabOn] == 1) {
					u32 targ_x = unit_ext->values[ValueWyvernLandGrabX];
					u32 targ_y = unit_ext->values[ValueWyvernLandGrabY];
					s32 direction = scbw::get_angle(x, y, targ_x, targ_y);
					if (scbw::get_distance_fast(unit->position.x, unit->position.y, targ_x, targ_y) <= range) {
						u32 sound = scbw::rand_between(SoundId::WyvernAttack01, SoundId::WyvernAttack02);
						helper::play_sound_from_unit(sound, unit, 1, 0);
						scbw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, turret->player, turret, 0, targ_x, targ_y);
						unit_ext->timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
						turret->order_target.pt.x = targ_x;
						turret->order_target.pt.y = targ_y;
						turret->order_target.unit = 0;
						seek_unit = false;
					} else {
						unit_ext->values[ValueWyvernLandGrabOn] = 0;
					}
				}

				if (seek_unit) {
					if (turret->order_target.unit
					&&  turret->main_order_id != OrderId::Follow
					&&  turret->main_order_id != OrderId::Patrol
					&&  turret->main_order_id != OrderId::WyvernLandGrab) {
						Unit* new_target = turret->order_target.unit;
						if (wyvern_can_fire_at(unit, new_target)) {
							unit_ext->units[UnitWyvernTargetLock] = new_target;
						}
					}
					Unit* target = unit_ext->units[UnitWyvernTargetLock];
					if (!target || !wyvern_can_fire_at(unit, target)) {
						EntryArray units = {};
						unit_search_radial(&units, unit->position, range);
						unit_search_sort_by_distance(&units, unit->position);

						for (u32 i = 0; i < units.count; ++i) {
							Unit* it = units.units[i];
							if (it == unit) {
								continue;
							}
							if (scbw::is_allied_to(unit->player, it->player)) {
								continue;
							}
							if (!wyvern_can_fire_at(unit, it)) {
								continue;
							}
							target = unit;
							break;
						}
					}
					
					if (target && wyvern_can_fire_at(unit, target)) {
						s32 direction = scbw::get_angle(x, y, target->get_x(), target->get_y());
						helper::play_sound_from_unit(scbw::rand_between(SoundId::WyvernAttack01, SoundId::WyvernAttack02), unit, 1, 0);
						scbw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, turret->player, turret, target, target->position);
						unit_ext->timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
					}
				}
			}
		}

		// Clear Cleric healing field every tick
		unit->is_being_healed = 0;

		// Cowardice slow flag
		if (unit->new_flags_3 & 0x10)
		{
			unit->new_flags_3 &= ~0x10;
			unit->update_speed();
		}

		// Something to do with player-specific Signify overlays?
		if (unit->new_flags_3 & 0x20 && unit == players_selections->unit[*g_active_nation_id][0])
		{
			unit->new_flags_3 &= ~0x20;
			scbw::refresh_console();
		}

		// Flash shielding (regeneration)
		if (units_dat::shields_enabled[unit->id] && unit->status & UnitStatus::Completed) {
			if (unit_id_can_regenerate_shields(unit->id)) {
				s32 max_shields = (s32)(units_dat::max_shield_points[unit->id]) * 256;

				u32 overlay_id = ImageId::RechargeShields_Small;
				switch (scbw::get_overlay_offset(unit->id, true)) {
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

				if (unit->shields < max_shields && unit->id != UnitId::ProtossPlasmaShell && unit_ext->shield_regen_timer > 2 * 24) {
					unit->heal_shields((10 * 256 / 24) * (1 + 0.25 * unit_ext->unit_vectors[UnitVectorCrucibles].size()), unit, false);
					if (unit->shields > max_shields) {
						if (unit->shields > max_shields) {
							unit->shields = max_shields;
						}
						if (unit->sprite->flags & SpriteFlags::Selected) { // If the unit is currently selected, redraw its graphics
							helper::set_all_image_group_flags_palette_11(unit->sprite);
						}
					}

					if (!unit->get_overlay(overlay_id)) {
						scbw::add_unit_overlay(unit, overlay_id);
						helper::play_sound_from_unit(SoundId::FlashShieldingStart, unit, 1, 0);
					}
				}
				else if (unit->sprite->get_overlay(overlay_id)) {
					scbw::remove_unit_overlay(unit, overlay_id);
					if (unit->shields >= max_shields) {
						helper::play_sound_from_unit(SoundId::FlashShieldingEnd, unit, 1, 0);
					}
					else {
						helper::play_sound_from_unit(SoundId::FlashShieldingInterrupt, unit, 1, 0);
					}
				}
			} else if (unit->id == UnitId::TerranAspirant) {
				if (unit_ext->damage_taken_timer > 2 * 24) {
					s32 regen = 1 + 0.25 * unit_ext->unit_vectors[UnitVectorCrucibles].size();
					unit->heal_shields(regen, 0, false, false);
				}
			}
		}

		// Vagrant Nihilism - suffer 2 damage per second until death
		if (unit->id == UnitId::ProtossVagrantShade) {
			unit->inflict_damage(21, unit, unit->player, false, (*g_elapsed_time_frames) % 6 == 0);
		}

		// Supposedly, allows Zethrokors to attack after unburrowing without delay
		if (unit->id == UnitId::ZergZethrokor)
		{
			if (unit->ground_weapon_cooldown == 0)
			{
				unit->order_queue_timer = 0;
			}
		}

		// update unit status effects (stim, maelstrom, plague, etc.)
		if (unit->status & UnitStatus::Completed || !(unit->sprite->flags & SpriteFlags::Hidden))
		{
			unit->cycle_counter++;
			if (unit->cycle_counter >= 8)
			{
				unit->cycle_counter = 0;
				helper::restore_all_unit_stats(unit); // use update_status_effects hook if available
			}
		}

		// Only for fully-constructed units and buildings. -GPTP
		// Modified 24/01/23 to allow Zerg building morph sources to regenerate while mutating. -Pr0nogo
		if (unit->status & UnitStatus::Completed
		||	unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon
		||	scbw::is_zerg_building_morph_result(unit->build_queue[unit->build_queue_slot]))
		{
			bool stop_here = false;

			// remove units with timed lives
			if (unit->remove_timer != 0)
			{
				unit->remove_timer -= 1;
				if (unit->remove_timer == 0)
				{
					unit->remove();
					stop_here = true;
				}
			}

			// update unit energy (energy regen/drain)
			update_unit_energy(unit);

			if (unit->recent_order_timer != 0)
			{
				unit->recent_order_timer--;
			}

			// Augur - Final Hour
			if (unit->id == UnitId::ProtossAugur && unit->shields == 0)
			{
				augur_sacrifice(unit);
				stop_here = true;
			}

			// HP regeneration
			if ((units_dat::base_property[unit->id] & UnitProperty::RegeneratesHP || unit_ext->values[ValueBlissfulEmbraceOriginalPlayer])
			&&	 unit->hitpoints > 0
			&&	 unit->hitpoints != unit->get_max_hp_raw()
			&&	 unit_ext->damage_taken_timer > 2 * 24 || unit_regens_in_combat(unit))
			{
				s32 regen = 11; // 1 HP per second
				if (unit->status & UnitStatus::Burrowed) {
					regen *= 2;
				}

				regen *= 1 + 0.25 * unit_ext->unit_vectors[UnitVectorCrucibles].size();
				unit->heal_hp(regen, 0, false);
			}

			// Deliberate healing
			{
				s32 tick_heal = 0;
				// Alaszileth - Dermal Accelerant - 500% life regen
				if (unit_ext->timers[TimerAlaszilethRegenBoost] > 0)
				{
					tick_heal += 11 * 5;
				}

				// Azazel Defensive Matrix - heal over time
				if (unit->defensive_matrix_hp != 0)
				{
					if (unit->hitpoints > 0 && unit->hitpoints / 256 != unit->get_max_hp_ingame())
					{
						tick_heal += AzazelDefensiveMatrixHeal;
					}
				}

				if (tick_heal)
				{
					unit->heal_hp(tick_heal);
				}
			}

			// Last Orders - Aurora passive
			if (unit->id == UnitId::ProtossAurora) {
				if (unit_ext->values[ValueLastOrdersInit] == 1) {
					auto distance = scbw::get_distance_fast(unit->position.x, unit->position.y, unit->order_target.pt.x, unit->order_target.pt.y);
					if (distance < 32) {
						unit->status |= UnitStatus::Invincible;
						// TODO: unit search radial
						auto match = cosmonarchy_finder_match(unit, Flags::NoAir | Flags::Enemy, {});
						const u32 radius = 64;
						scbw::UnitFinder(
							unit->get_x() - radius,
							unit->get_y() - radius,
							unit->get_x() + radius,
							unit->get_y() + radius
						).for_each_in_match(match, [&](Unit* target) {
							u32 dist = scbw::get_distance_fast(unit->position, target->position);
							u32 damage = 25 * 256;
							if (dist > 48) {
								damage /= 4;
							} else if (dist > 32) {
								damage /= 2;
							}
							target->damage_with_custom(damage, -1, unit, unit->player);
						});
						unit->remove();
						avenge_or_ascend_corpses_in_area(unit->position, 2 * 32, unit->player);
						stop_here = true;
					}
				}
			}

			// Terran building burn-down
			if (!stop_here && !unit_ext->values[ValueBlissfulEmbraceOriginalPlayer])
			{
				if (unit->get_race() == RaceId::Terran)
				{
					if (unit->status & UnitStatus::GroundedBuilding
					||  units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding)
					{
						if (helper::unit_hp_is_in_redzone(unit))
						{
							unit->damage_hp(20, 0, unit->last_attacking_player, true);
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
			if (*g_is_placing_building) {
				helper::refresh_layers_3_and_4();
				helper::end_building_placement();
			}
			if (*g_is_targeting) {
				helper::cancel_target_order();
			}
		}

		// Originally had code related to order timers and Disruption Web; removed former, handled latter in globals

		Unit* next_unit; // caching next unit instead of a direct read, since some functions might actually modify the link.next
						 // (especially when the unit dies or gets hidden etc)
		for (Unit* unit = *g_first_dying_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit   = unit;
			helper::progress_dying_unit_frame(unit);
			if (unit->sprite) {
				if (*g_vision_updated) {
					if (g_player_table[unit->player].type == PlayerType::Human) {
						hooks::reveal_sight_at_location(1, player_vision->flags[unit->player], unit->position.x, unit->position.y, unit->status & UnitStatus::InAir);
					}
				}
			//	helper::update_visibility(unit);
			} else {
				unit_death_end(unit);
			}
		}

		for (Unit* unit = *g_first_visible_unit; unit; unit = next_unit) {
			next_unit = unit->link.next;
			*g_active_iscript_entity = unit;
			*g_active_iscript_unit = unit;
			helper::progress_active_unit_frame(unit);
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
	void update_burrowing_cloaking_units() // 00492EB0
	{
		Unit* invisible_unit = *g_first_invisible_unit;
		bool redraw = 0;
		if (*g_first_invisible_unit)
		{
			while (1)
			{
				Unit* next_burrowed = invisible_unit->invisible_link.next;
				if (helper::is_unit_cloaking(invisible_unit))
				{
					do_cloak(invisible_unit, scbw::get_cloak_sound_id(invisible_unit));
				} else {
					u32 status_1 = invisible_unit->status;
					bool cloak_or_burrow;
					if (status_1 & UnitStatus::Burrowed || invisible_unit->secondary_order_id != OrderId::Cloak || invisible_unit->is_cloaked != 1 || !(status_1 & UnitStatus::CloakingForFree))
					{
						cloak_or_burrow = 0;
					} else {
						invisible_unit->status = status_1 & 0xFFFFF7FF;
						cloak_or_burrow = 1;
					}
					redraw |= cloak_or_burrow;
					if (invisible_unit->status & UnitStatus::RequiresDetection)
					{
						goto Next_Or_Redraw;
					}
					helper::begin_cloak(invisible_unit, scbw::get_cloak_sound_id(invisible_unit));
				}
				redraw = 1;
			Next_Or_Redraw:
				invisible_unit = next_burrowed;
				if (!next_burrowed)
				{
					if (redraw)
					{
						scbw::refresh_console();
					}
					return;
				}
			}
		}
	}

	// Hooked for cloak rewrite animations - Pr0nogo
	void do_cloak(Unit* unit, s32 sound_id) // 0049B440
	{
		if (!(unit->status & UnitStatus::Burrowed))
		{
			helper::play_sound_from_unit(sound_id, unit, 1, 0);
		}
		if (!*g_is_in_replay && unit->player == *g_local_nation_id)
		{
			scbw::refresh_console();
		}

		if (unit->status & UnitStatus::Burrowed)
		{
			helper::update_sprite_flag_on_palette(unit->sprite);
			helper::show_images_hidden_by_cloak(unit->sprite);
			unit->status &= 0xFFFFFCFF;
		} else {
			helper::update_sprite_images_for_cloak(unit->sprite);
			if (unit->sprite->main_graphic->id != ImageId::WarpAnchor) {
				unit->status = unit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			}
			Unit* subunit = unit->subunit;
			if (subunit)
			{
				helper::update_sprite_images_for_cloak(subunit->sprite);
				if (subunit->sprite->main_graphic->id != ImageId::WarpAnchor) {
					subunit->status = subunit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
				}
			}
		}
	}

	// Hooked to add nullcheck - Pr0nogo
	void progress_unit_frame(Unit* unit) // 004ECF70
	{
		Unit* unitVar = unit;
		if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit) && unit->sprite)
		{
			if (!(unit->sprite->flags & 0x20))
			{
				helper::refresh_sprite(unit->sprite, g_selection_circle_color[unit->player]);
			}
		}
		update_unit_state(unit);
		helper::progress_order(unit);
		helper::perform_secondary_orders(unit, unit->player); // ECX at the time of calling this function

		if (unit->subunit)
		{
			if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit))
			{
				*g_active_iscript_unit = unit->subunit;
				progress_unit_frame(unit->subunit);
				*g_active_iscript_unit = unit;
			}
		}
		if (unit->sprite)
		{
			helper::sprite_to_iscript_loop(unit->sprite);
			if (!unit->sprite->images.head)
			{
				unit->sprite = 0;
			}
		}
	}

	// Hooked to investigate crashes. -Pr0nogo
	void sprite_to_iscript_loop(Sprite* sprite) // 00497920
	{
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
				if (!sprite->images.head)
				{
					u32 pos_y = sprite->position.y / 32;
					if (pos_y >= 0)
					{
						if (pos_y >= *g_map_tile_height)
						{
							pos_y = *g_map_tile_height - 1;
						}
					} else {
						pos_y = 0;
					}

					if (g_horizontal_sprite_lines_begin[pos_y] == sprite)
					{
						g_horizontal_sprite_lines_begin[pos_y] = sprite->link.next;
					}
					if (g_horizontal_sprite_lines_end[pos_y] == sprite)
					{
						g_horizontal_sprite_lines_end[pos_y] = sprite->link.prev;
					}
					if (sprite->link.prev)
					{
						sprite->link.prev->link.next = sprite->link.next;
					}

					Sprite* next_sprite = sprite->link.next;
					if (next_sprite)
					{
						next_sprite->link.prev = sprite->link.prev;
					}

					sprite->link.prev = 0;
					sprite->link.next = 0;
					Sprite* first_free = g_free_sprites->head;
					if (g_free_sprites->head)
					{
						if (g_free_sprites->tail == g_free_sprites->head)
						{
							g_free_sprites->tail = sprite;
						}
						sprite->link.prev = g_free_sprites->head;
						sprite->link.next = first_free->link.next;
						Sprite* next_free = first_free->link.next;
						if (next_free)
						{
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
	u8 replace_unit_with_id(Unit* unit, u32 new_id) // 0049FED0
	{
		if (!unit->sprite) {
			return 0;
		}

		if (!(unit->sprite->flags & SpriteFlags::Hidden))
		{
			helper::sort_unit(unit);
			if (unit->status & UnitStatus::GroundedBuilding)
			{
				helper::clear_building_from_tile_flag(unit, unit->position.x, unit->position.y);
			}
			if (unit->status & UnitStatus::InAir)
			{
				helper::remove_repulse_tile(unit);
			}
		}
		helper::transform_unit_sprite(unit, new_id);

		if (units_dat::base_property[unit->id] & UnitProperty::Building)
		{
			if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection))
			{
				if (unit->secondary_order_id != OrderId::Nothing2)
				{
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

		if (unit->sprite->flags & SpriteFlags::Selected)
		{
			if (!helper::unit_is_standard_and_movable(unit) && *g_client_selection_count > 1 && new_id != UnitId::ZergEgg)
			{
				helper::update_building_land_unit_selection(0, unit);
				helper::remove_from_client_selection_3(unit);
			}
		}
		scbw::refresh_console();
		u32 pos_x = unit->position.x;
		u32 pos_y = unit->position.y;
		Point16 position = {pos_x, pos_y};
		helper::fix_target_location(&position, new_id);
		if (pos_x != position.x || pos_y != position.y)
		{
			helper::move_unit(unit, position.x, position.y);
		}
		if (!(unit->sprite->flags & SpriteFlags::Hidden))
		{
			helper::update_unit_order_finder(unit);
			if (unit->status & UnitStatus::GroundedBuilding)
			{
				helper::set_building_tile_flag(unit, unit->position.x, unit->position.y);
			}
			helper::check_unstack(unit);
			if (unit->status & UnitStatus::InAir)
			{
				helper::set_repulse_angle(unit);
			}
		}

		Unit* subunit = unit->subunit;
		if (subunit)
		{
			helper::transform_subunit(subunit, units_dat::subunit[new_id]);
		}
		return helper::apply_unit_effects(unit);
	}

	// Hooked to change the sound id for permacloaked units on initial creation. -Pr0nogo
	u32 update_unit_strength_and_apply_default_orders(Unit* unit) // 0049FA40
	{
		if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg)
		{
			helper::increment_unit_scores(unit, -1);
			unit->status |= UnitStatus::Completed;
			helper::increment_unit_scores(unit, 1);
		} else {
			unit->status |= UnitStatus::Completed;
		}
		helper::increment_unit_scores_ex(unit, 1, 1);
		if (units_dat::base_property[unit->id] & UnitProperty::PermanentCloak)
		{
			u32 sound_id = scbw::get_cloak_sound_id(unit);
			helper::begin_cloak(unit, sound_id);
		}
		if (unit->id == UnitId::Spell_ScannerSweep || unit->id == UnitId::Special_MapRevealer)
		{
			if (*g_first_hidden_unit == unit)
			{
				*g_first_hidden_unit = unit->link.next;
			}
			if (*g_last_hidden_unit == unit)
			{
				*g_last_hidden_unit = unit->link.prev;
			}
			if (unit->link.prev)
			{
				unit->link.prev->link.next = unit->link.next;
			}
			Unit* next_unit = unit->link.next;
			if (next_unit)
			{
				next_unit->link.prev = unit->link.prev;
			}
			unit->link.prev = 0;
			unit->link.next = 0;
			Unit* first_scanner = *g_first_scanner_sweep;
			if (*g_first_scanner_sweep)
			{
				if (*g_last_scanner_sweep == *g_first_scanner_sweep)
				{
					*g_last_scanner_sweep = unit;
				}
				unit->link.prev = *g_first_scanner_sweep;
				unit->link.next = first_scanner->link.next;
				Unit* next_scanner = first_scanner->link.next;
				if (next_scanner)
				{
					next_scanner->link.prev = unit;
				}
				first_scanner->link.next = unit;
				helper::refresh_unit_vision(unit);
			} else {
				*g_last_scanner_sweep = unit;
				*g_first_scanner_sweep = unit;
				helper::refresh_unit_vision(unit);
			}
		} else if (unit->sprite->flags & SpriteFlags::Hidden && unit->id != UnitId::ProtossSteward && unit->id != UnitId::ProtossPlasmaShell) {
			helper::show_and_enable_unit(unit);
		}
		// Originally had trap code
		u32 player_type = g_player_table[(unsigned __int8)unit->player].type;
		if (player_type == PlayerType::Rescuable)
		{
			u32 main_order_id = unit->main_order_id;
			unit->user_action_flags |= UserActionFlags::UserActionFlags_IscriptGoToIdle;
			if (main_order_id)
			{
				while (1)
				{
					Order* tail = unit->order_queue_tail;
					if (!tail)
					{
						break;
					}
					u32 tail_order_id = tail->order_id;
					if (!orders_dat::can_be_interrupted[tail_order_id] && tail_order_id != OrderId::RescuePassive)
					{
						break;
					}
					helper::remove_order_from_unit_queue(tail, unit);
				}
				helper::perform_another_order(OrderId::RescuePassive, 0, unit, UnitId::None, {}, 0);
			}
			helper::do_next_queued_order(unit);
		} else {
			u32 new_order_id;
			if (player_type == PlayerType::Neutral)
			{
				new_order_id = OrderId::Neutral;
			} else if (player_type == PlayerType::Computer) {
				new_order_id = units_dat::computer_idle_order[unit->id];
			} else {
				new_order_id = units_dat::human_idle_order[unit->id];
			}
			helper::issue_order_targeting_nothing(unit, new_order_id);
		}
		if (units_dat::base_property[unit->id] & UnitProperty::NeutralAccessories)
		{
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
	u32 initialize_unit_base(Unit* unit, u32 unit_id, u32 pos_x, u32 pos_y, u32 player) // 004A0320
	{
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
		if (initialized)
		{
			*(u32*)unit->build_queue = 0xE400E4;
			*(u32*)&unit->build_queue[2] = 0xE400E4;
			unit->build_queue[4] = UnitId::None;
			u8 unique_id_something = unit->secondary_order_id & OrderId::BuildProtoss1;// secondary_order_special
			unit->build_queue_slot = 0;
			unit->unique_id = unique_id_something;
			if (!units_dat::group_flags[unit->id].is_factory)
			{
				if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building)
				{
					unit->current_buttonset = UnitId::Buildings;
				}
			} else {
				if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building)
				{
					unit->current_buttonset = UnitId::Factories;
				}
			}

			u32 random;
			if (*g_is_in_game_loop)
			{
				++g_random_counts[15];
				random = (22695477 * *g_last_random_number + 1) >> 16;
				++(*g_total_random_counts);
				*g_last_random_number = 22695477 * *g_last_random_number + 1;
			} else {
				random = 0;
			}

			s32 current_hp;
			unit->wireframe_randomizer = random;
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit)
			{
				current_hp = 1;
			} else {
				current_hp = units_dat::max_hitpoints[unit_id] / 10;
			}
			unit->hitpoints = current_hp;

			u32 main_order_id;
			if ((unit->status & UnitStatus::GroundedBuilding) == 0)
			{
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
			if (unit->secondary_order_id != OrderId::Nothing2)
			{
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
			if (first_unit)
			{
				first_unit->player_link.prev = unit;
			}
			first_player_unit->unit[player] = unit;
			helper::increment_unit_scores(unit, 1);

			if (unit->status & UnitStatus::GroundedBuilding)
			{
				helper::update_unit_order_finder(unit);
				if (unit->status & UnitStatus::GroundedBuilding)
				{
					helper::set_building_tile_flag(unit, unit->sprite->position.x, unit->sprite->position.y);
				}
				helper::check_unstack(unit);
				if (unit->status & UnitStatus::InAir)
				{
					helper::set_repulse_angle(unit);
				}
				helper::init_building(unit, 1);
				helper::refresh_all_visible_images_at_screen_pos(unit->sprite, 0);
				if (*g_is_placing_building)
				{
					helper::place_building_proc();
				}
			} else {
				if (unit->id == UnitId::TerranVulture)
				{
					unit->mine_carrier.mine_count = 2;
				}
				unit->sprite->flags |= SpriteFlags::Hidden;
				helper::refresh_all_visible_images_at_screen_pos(unit->sprite, 0);
			}

			unit->detection_status = -1;
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit)
			{
				unit->sprite->flags |= SpriteFlags::RemoveAfterDeath;
				return 1;
			} else {
				if (!(unit->sprite->flags & SpriteFlags::Hidden))
				{
					helper::refresh_unit_vision(unit);
				}
				return 1;
			}
		}
		return initialized;
	}

	// Hooked to investigate crashes. -Pr0nogo
	b32 set_thingy_visibility_flags(Thingy* thingy) // 004878F0
	{
		Sprite* thingy_sprite = thingy->sprite;
		if (thingy_sprite)
		{
			if (!(thingy_sprite->flags & SpriteFlags::Hidden))
			{
				u8 status = ~(g_active_tile_array[thingy_sprite->position.x / 32 + g_map_tile_size->width * (thingy_sprite->position.y / 32)]->visibility_flags);
				if (thingy_sprite->visibility_status != status)
				{
					helper::refresh_all_visible_images_at_screen_pos(thingy_sprite, status);
					if ((u8)*g_player_visions & thingy_sprite->visibility_status)
					{
						if (!(*g_player_visions & thingy->sprite->visibility_status))
						{
							return 1;
						}
					}
				}
			}
		}
		return 0;
	}
}