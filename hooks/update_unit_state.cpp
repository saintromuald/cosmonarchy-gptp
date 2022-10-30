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
		if (unit_ext->values[ValueGodsMadeFleshActive]) {
			unit->spend_unit_energy(GodsMadeFleshEnergyDrain);
			dont_regen = true;
		}

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
		if (unit->id == UnitId::ProtossMindTyrant
			&& unit->main_order_id == OrderId::CompletingArchonSummon
			&& unit->main_order_state == 0) {
			max_energy = 100 * 256; // Identical to energy amount on spawn
		}

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
				unit->sprite->create_overlay(overlay_id);
				helper::play_sound_from_unit(SoundId::FlashShieldingStart, unit, 1, 0);
			}
		} else if (unit->sprite->get_overlay(overlay_id)) {
			unit->sprite->remove_overlay(overlay_id);
			if (unit->energy >= max_energy) {
				helper::play_sound_from_unit(SoundId::FlashShieldingEnd, unit, 1, 0);
			} else {
				helper::play_sound_from_unit(SoundId::FlashShieldingInterrupt, unit, 1, 0);
			}
		}
	}

	static bool wyvern_can_fire_at(Unit* unit, Unit* target)
	{
		const u32 unitOffset = std::max(unit->get_right() - unit->get_left(), unit->get_bottom() - unit->get_top()) + 32;
		return scbw::can_weapon_target_unit(WeaponId::WyvernJacksonCannon, target, unit)
			&& scbw::get_distance_fast(target->sprite->position.x, target->sprite->position.y, unit->sprite->position.x, unit->sprite->position.y) <= weapons_dat::max_range[WeaponId::WyvernJacksonCannon] + unitOffset
			&& unit->can_attack_target(target);
	}

	// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	void update_unit_state(Unit* unit) // 004EC290
	{
		UnitExt* unit_ext = get_unit_ext(unit);

		// Timers
		if (unit->main_order_timer)
		{
			unit->main_order_timer--;
		}
		if (unit->ground_weapon_cooldown)
		{
			unit->ground_weapon_cooldown--;
		}
		if (unit->air_weapon_cooldown)
		{
			unit->air_weapon_cooldown--;
		}
		if (unit->spell_cooldown)
		{
			unit->spell_cooldown--;
		}

		// Wyvern clear target when changed owner to allied
		if (unit->id == UnitId::TerranWyvern
			&& unit_ext->units[UnitWyvernTargetLock])
		{
			Unit* target = unit_ext->units[UnitWyvernTargetLock];
			UnitExt* targetExt = get_unit_ext(target);
			if (targetExt->timers[TimerOwnerChangedRecently]
				&& scbw::is_allied_to(unit->player, target->player))
			{
				unit_ext->units[UnitWyvernTargetLock] = nullptr;
			}
		}

		// Wyvern Reverse Thrust - fire while moving
		if (unit->id == UnitId::TerranWyvern
		&&  unit_ext->timers[TimerWyvernAttackCooldown] == 0)
		{
			auto range = weapons_dat::max_range[WeaponId::WyvernJacksonCannon];
			s32 forwardOffset = weapons_dat::forward_offset[WeaponId::WyvernJacksonCannon];
			auto x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
			auto y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1);

			bool seek_unit = true;
			if (unit_ext->values[ValueWyvernLandGrabOn] == 1)
			{
				auto targ_x = unit_ext->values[ValueWyvernLandGrabX];
				auto targ_y = unit_ext->values[ValueWyvernLandGrabY];
				auto direction = scbw::get_angle(x, y, targ_x, targ_y);
				if (scbw::get_distance_fast(unit->position.x, unit->position.y, targ_x, targ_y) <= range)
				{
					u32 sound = scbw::rand_between(SoundId::WyvernAttack01, SoundId::WyvernAttack02);
					helper::play_sound_from_unit(sound, unit, 1, 0);
					scbw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, unit->player, unit, nullptr, targ_x, targ_y);
					unit_ext->timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
					seek_unit = false;
				}
				else
				{
					unit_ext->values[ValueWyvernLandGrabOn] = 0;
				}
			}

			if (seek_unit)
			{
				if (unit->order_target.unit
					&& unit->main_order_id != OrderId::Follow
					&& unit->main_order_id != OrderId::Patrol
					&& unit->main_order_id != OrderId::WyvernLandGrab)
				{
					Unit* newTarget = unit->order_target.unit;
					if (wyvern_can_fire_at(unit, newTarget))
					{
						unit_ext->units[UnitWyvernTargetLock] = newTarget;
					}
				}
				Unit* target = unit_ext->units[UnitWyvernTargetLock];
				if (!target || !wyvern_can_fire_at(unit, target))
				{
					target = scbw::UnitFinder::get_nearest_target(
						unit->get_x() - range,
						unit->get_y() - range,
						unit->get_x() + range,
						unit->get_y() + range,
						unit, [&](Unit* target)
					{
						if (target == unit)
						{
							return false;
						}
						if (scbw::is_allied_to(unit->player, target->player))
						{
							return false;
						}
						if (!wyvern_can_fire_at(unit, target))
						{
							return false;
						}
						return true;
					});	
				}
				
				if (target)
				{
					auto direction = scbw::get_angle(x, y, target->get_x(), target->get_y());
					helper::play_sound_from_unit(scbw::rand_between(SoundId::WyvernAttack01, SoundId::WyvernAttack02), unit, 1, 0);
					scbw::artificial_bullet_from(WeaponId::WyvernJacksonCannon, unit->player, unit, target, target->position);
					unit_ext->timers[TimerWyvernAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::WyvernJacksonCannon);
				}
			}
		}

		// Clear Cleric healing field every tick
		unit->is_being_healed = 0;

		// Accantor Plasma Shells - gain shields equal to the source Accantor
		if (unit->id == UnitId::ProtossAccantorOld)
		{
			Unit* plasma_shell = unit->unit_with_hangar.in_hangar_child;
			while (plasma_shell)
			{
				if (plasma_shell->id == UnitId::ProtossPlasmaShell)
				{
					plasma_shell->shields = unit->shields;
				}
				plasma_shell = plasma_shell->hangar_unit.hangar_link.prev;
			}
		}
		
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

		// Add mining status, decrease the regeneration rate
		/*
		if (unit->main_order_id == OrderId::MiningMinerals
		||	unit->main_order_id == OrderId::HarvestGas3)
		{
			if (unit->order_target.unit)
			{
				unit->order_target.unit->cmbw_resource_regen_mining_delay = 5 * 24;
			}
		}
		*/

		/*
		// Resource regeneration
		if (units_dat::base_property[unit->id] & UnitProperty::ResourceContainer)
		{
			// Decrement resource regen timers
			if (unit->cmbw_resource_regen_mining_delay > 0)
			{
				unit->cmbw_resource_regen_mining_delay--;
			}
			if (!unit->cmbw_resource_regen_mining_delay || unit_ext->timers[TimerResourceBoostedRegen]) {
				if (unit->cmbw_resource_regen_cooldown > 0)
				{
					unit->cmbw_resource_regen_cooldown--;
				}
				else
				{
					// Set regen timers
					unit->cmbw_resource_regen_cooldown = 36;
					if (unit_ext->timers[TimerResourceBoostedRegen]) {
						unit->cmbw_resource_regen_cooldown /= 3;
					}

					// Actual resource regeneration
					bool update_image = true;
					if (unit->id == UnitId::ResourceVespeneGeyser || (scbw::unit_id_is_capped_gas_structure(unit->id) && unit_ext->timers[TimerResourceBoostedRegen]))
					{
						if (unit->building.resource.resource_amount < 7500) {
							unit->building.resource.resource_amount++;
							scbw::refresh_console();
						}
					}
					else if (unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3)
					{
						if (unit->building.resource.resource_amount < 2500) {
							unit->building.resource.resource_amount++;
							helper::update_mineral_patch_image(unit);
							scbw::refresh_console();
						}
					}
				}
			}
		}
		*/

		// Flash shielding (regeneration)
		if (units_dat::shields_enabled[unit->id] && unit->status & UnitStatus::Completed && scbw::unit_id_can_regenerate_shields(unit->id)) {
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
				unit->heal_shields((10 * 256 / 24) * (1 + 0.15 * unit_ext->unit_vectors[UnitVectorCrucibles].size()), unit, false);
				if (unit->shields > max_shields) {
					if (unit->shields > max_shields) {
						unit->shields = max_shields;
					}

					if (unit->sprite->flags & SpriteFlags::Selected) { // If the unit is currently selected, redraw its graphics
						helper::set_all_image_group_flags_palette_11(unit->sprite);
					}
				}

				if (!unit->get_overlay(overlay_id)) {
					unit->sprite->create_top_overlay(overlay_id);
					helper::play_sound_from_unit(SoundId::FlashShieldingStart, unit, 1, 0);
				}
			} else if (unit->sprite->get_overlay(overlay_id)) {
				unit->sprite->remove_overlay(overlay_id);
				if (unit->shields >= max_shields) {
					helper::play_sound_from_unit(SoundId::FlashShieldingEnd, unit, 1, 0);
				} else {
					helper::play_sound_from_unit(SoundId::FlashShieldingInterrupt, unit, 1, 0);
				}
			}
		}

		// Vagrant Nihilism - suffer 2 damage per second until death
		if (unit->id == UnitId::ProtossVagrantShade)
		{
			unit->inflict_damage(21);
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

		// only for fully-constructed units and buildings
		if (unit->status & UnitStatus::Completed)
		{
			bool bStopHere = false;

			// remove units with timed lives
			if (unit->remove_timer != 0)
			{
				unit->remove_timer -= 1;
				if (unit->remove_timer == 0)
				{
					unit->remove();
					bStopHere = true;
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
				bStopHere = true;
			}

			// Cleanse debuffs on initial Defensive Matrix application
			if (unit->defensive_matrix_timer == AzazelDefensiveMatrixTimer)
			{
				scbw::restore_func(unit);
			}

			// HP regeneration
			if ((units_dat::base_property[unit->id] & UnitProperty::RegeneratesHP || unit_ext->values[ValueBlissfulEmbraceOriginalPlayer])
			&&	 unit->hitpoints > 0
			&&	 unit->hitpoints != unit->get_max_hp_raw()
			&&	 unit_ext->damage_taken_timer > 2 * 24)
			{
				s32 regen = 11; // 1 HP per second
				if (unit->status & UnitStatus::Burrowed) {
					regen *= 2;
				}
				if (unit->id == UnitId::TerranMatador) {
					regen *= 5;
				}

				regen *= 1 + 0.15 * unit_ext->unit_vectors[UnitVectorCrucibles].size();
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

				// Azazel Innervating Matrices - heal over time
				if (unit->defensive_matrix_hp != 0)
				{
					if (unit->hitpoints > 0
					&&  unit->hitpoints / 256 != unit->get_max_hp_ingame())
					{
						auto caster = scbw::get_aise_value(unit, nullptr, AiseId::GetMatrixCasterPlayer, 0, 0);
						if (caster < 8)
						{
							tick_heal += AzazelDefensiveMatrixHeal;
						}
					}
				}

				if (tick_heal)
				{
					unit->heal_hp(tick_heal);
				}
			}

			// Last Orders - Aurora passive
			if (unit->id == UnitId::ProtossAurora)
			{
				if (unit_ext->values[ValueLastOrdersInit] == 1)
				{
					auto distance = scbw::get_distance_fast(unit->position.x, unit->position.y, unit->order_target.pt.x, unit->order_target.pt.y);
					if (distance < 32)
					{
						unit->status |= UnitStatus::Invincible;

						auto match = cosmonarchy_finder_match(unit, Flags::NoAir | Flags::Enemy, {});
						const u32 radius = 64;
						scbw::UnitFinder(
							unit->get_x() - radius,
							unit->get_y() - radius,
							unit->get_x() + radius,
							unit->get_y() + radius
						).for_each_in_match(match, [&](Unit* target)
						{
							u32 dist = scbw::get_distance_fast(unit->position, target->position);
							u32 damage = 20 * 256;
							if (dist > 48)
							{
								damage /= 4;
							}
							else if (dist > 32)
							{
								damage /= 2;
							}
							target->damage_with_custom(damage, damage_type::Explosive, unit, unit->player, 0, 1, false);
						});
						unit->remove();
						bStopHere = true;
					}
				}
			}

			// Terran building burn-down
			if (!bStopHere && !unit_ext->values[ValueBlissfulEmbraceOriginalPlayer])
			{
				if (unit->get_race() == RaceId::Terran)
				{
					if (unit->status & UnitStatus::GroundedBuilding
					||  units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding)
					{
						if (helper::unit_hp_is_in_redzone(unit))
						{
							unit->damage_hp(20, nullptr, unit->last_attacking_player, true);
						}
					}
				}
			}
		}
	};

	// Hooked to help with the cloak rewrite - Pr0nogo
	void update_units() // 004ED000
	{
		*g_interceptor_focused_this_frame = 0;
		if (helper::is_selected_unit_group_enabled())
		{
			if (*g_is_placing_building)
			{
				helper::refresh_layers_3_and_4();
				helper::end_building_placement();
			}
			if (*g_is_targeting)
			{
				helper::cancel_target_order();
			}
		}
		Unit* first_visible_unit = *g_first_visible_unit;
		if (!--(*g_order_timer_reassign_count))
		{
			char visible_timer = 0;
			if (first_visible_unit)
			{
				do
				{
					first_visible_unit->order_queue_timer = visible_timer++;
					if (visible_timer == 8)
					{
						visible_timer = 0;
					}
					first_visible_unit = first_visible_unit->link.next;
				}
				while (first_visible_unit);
				first_visible_unit = *g_first_visible_unit;
			}
			*g_order_timer_reassign_count = 150;
		}
		if (!--(*g_secondary_order_timer_reassign_count))
		{
			for (char i = 0; first_visible_unit; first_visible_unit = first_visible_unit->link.next)
			{
				first_visible_unit->secondary_order_timer = i++;
				if (i == 30)
				{
					i = 0;
				}
			}
			*g_secondary_order_timer_reassign_count = 300;
		}
		// Now handled in global effect
	//	helper::update_disruption_web_status();
		Unit* dying_unit = *g_first_dying_unit;
		if (*g_first_dying_unit)
		{
			Unit* next_dying_unit = dying_unit->link.next;
			do
			{
				next_dying_unit = dying_unit->link.next;
				*g_active_iscript_entity = dying_unit;
				*g_active_iscript_unit = dying_unit;
				helper::progress_dying_unit_frame(dying_unit);
				if (dying_unit->sprite)
				{
					if (*visionUpdated)
					{
						if (g_player_table[dying_unit->player].type == 2) // PLAYER_OCCUPIED
						{
							helper::reveal_sight_at_location(1, player_vision->flags[dying_unit->player], dying_unit->sprite->position.x, dying_unit->sprite->position.y, (dying_unit->status >> 2) & 1); // is air unit
						}
					}
					helper::update_visibility(dying_unit);
				}
				dying_unit = next_dying_unit;
			}
			while (next_dying_unit);
		}
		first_visible_unit = *g_first_visible_unit;
		if (first_visible_unit)
		{
			do
			{
				*g_active_iscript_entity = first_visible_unit;
				*g_active_iscript_unit = first_visible_unit;
				helper::progress_active_unit_frame(first_visible_unit);
				first_visible_unit = first_visible_unit->link.next;
			}
			while (first_visible_unit);
			first_visible_unit = *g_first_visible_unit;
		}
		Unit* scanner_sweep = *first_scanner_sweep;
		if (*visionUpdated)
		{
			if (scanner_sweep)
			{
				Unit* next_scanner_sweep = scanner_sweep->link.next;
				do
				{
					next_scanner_sweep = scanner_sweep->link.next;
					*g_active_iscript_entity = scanner_sweep;
					*g_active_iscript_unit = scanner_sweep;
					helper::refresh_unit_vision(scanner_sweep);
					scanner_sweep = next_scanner_sweep;
				}
				while (next_scanner_sweep);
				first_visible_unit = *g_first_visible_unit;
			}
		}
		if (first_visible_unit)
		{
			UnitExt* unitExt = get_unit_ext(first_visible_unit);
			do
			{
				helper::update_visibility(first_visible_unit);
				if (first_visible_unit->status & 0x300 && unitExt->timers[TimerDecloak] == 0)
				{
					first_visible_unit->is_cloaked = 0;
					if (first_visible_unit->secondary_order_timer)
					{
						first_visible_unit->secondary_order_timer = first_visible_unit->secondary_order_timer - 1;
					}
					else
					{
						helper::check_unit_visibility(first_visible_unit);
						first_visible_unit->secondary_order_timer = 30;
					}
				}
				first_visible_unit = first_visible_unit->link.next;
			}
			while (first_visible_unit);
			first_visible_unit = *g_first_visible_unit;
		}
		if (first_visible_unit)
		{
			Unit* next_visible_unit = first_visible_unit->link.next;
			do
			{
				next_visible_unit = first_visible_unit->link.next;
				*g_active_iscript_entity = first_visible_unit;
				*g_active_iscript_unit = first_visible_unit;
				helper::progress_unit_frame(first_visible_unit);
				first_visible_unit = next_visible_unit;
			}
			while (next_visible_unit);
		}
		Unit* hidden_unit = *g_first_hidden_unit;
		if (hidden_unit)
		{
			Unit* next_hidden_unit = hidden_unit->link.next;
			do
			{
				next_hidden_unit = hidden_unit->link.next;
				if (hidden_unit->status & 0x300)
				{
					hidden_unit->is_cloaked = 0;
				}
				*g_active_iscript_entity = hidden_unit;
				*g_active_iscript_unit = hidden_unit;
				helper::unit_update(hidden_unit);
				hidden_unit = next_hidden_unit;
			}
			while (next_hidden_unit);
		}
		helper::update_burrowing_cloaking_units();
		helper::update_unit_power();
		s32 lurker_splash_index = (*g_lurker_splash_list_index)++ + 1;
		if (*g_lurker_splash_list_index == 32)
		{
			lurker_splash_index = 0;
			*g_lurker_splash_list_index = 0;
		}
		memset(&g_lurker_splash_list[32 * lurker_splash_index], 0, 0x80u);
		*g_last_bullet_index = 0;
		scanner_sweep = *first_scanner_sweep;
		if (scanner_sweep)
		{
			Unit* next_scanner_sweep_2 = scanner_sweep->link.next;
			do
			{
				next_scanner_sweep_2 = scanner_sweep->link.next;
				*g_active_iscript_entity = scanner_sweep;
				*g_active_iscript_unit = scanner_sweep;
				helper::progress_unit_frame(scanner_sweep);
				scanner_sweep = next_scanner_sweep_2;
			}
			while (next_scanner_sweep_2);
		}
		*g_active_iscript_entity = 0;
		*g_active_iscript_unit = 0;
	}

	// Hooked to help with the cloak rewrite - Pr0nogo
	void update_burrowing_cloaking_units() // 00492EB0
	{
		Unit* v0;
		s32 v1;
		Unit* v2;
		s32 v3;
		s32 v4;
		s32 v5;

		v0 = *first_invisible_unit;
		v1 = 0;
		if (*first_invisible_unit)
		{
			while (1)
			{
				v2 = v0->burrow_link.next;
				if (helper::is_unit_cloaking(v0))
				{
					do_cloak(v0, SoundId::Terran_PHOENIX_TPhClo01_WAV);
				}
				else
				{
					v3 = v0->status;
					if (v3 & UnitStatus::Burrowed || v0->secondary_order_id != OrderId::Cloak || v0->is_cloaked != 1 || !(v3 & 0x800))
					{
						v4 = 0;
					}
					else
					{
						v0->status = v3 & 0xFFFFF7FF;
						v4 = 1;
					}
					v1 |= v4;
					v5 = v0->status;
					if (v5 & 0x100)
						goto LABEL_13;
					helper::play_sound_from_direction(v0, 273);
				}
				v1 = 1;
		LABEL_13:
				v0 = v2;
				if (!v2)
				{
					if (v1)
					{
						*force_button_refresh = 1;
						portrait_redraw[0] = 1;
						*status_screen_redraw = 1;
						*control_under_mouse = 0;
						*control_under_mouse_value = 0;
					}
					return;
				}
			}
		}
	}

	// Hooked for cloak rewrite animations - Pr0nogo
	void do_cloak(Unit* unit, s32 soundId) // 0049B440
	{
		Unit* unitVar = unit;
		if (!(unit->status & UnitStatus::Burrowed))
		{
			helper::play_sound_from_unit(soundId, unit, 1, 0);
		}
		if (!g_is_in_replay && unitVar->player == *g_local_nation_id)
		{
			*force_button_refresh = 1;
			portrait_redraw[0] = 1;
			*status_screen_redraw = 1;
			*control_under_mouse = 0;
			*control_under_mouse_value = 0;
		}
		if (unitVar->status & UnitStatus::Burrowed)
		{
			helper::update_sprite_flag_on_palette(unitVar->sprite);
			helper::show_images_hidden_by_cloak(unitVar->sprite);
			unitVar->status &= 0xFFFFFCFF;
		}
		else
		{
			helper::update_sprite_images_for_cloak(unitVar->sprite);
			unitVar->status = unitVar->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			Unit* unitSubunit = unitVar->subunit;
			if (unitSubunit)
			{
				helper::update_sprite_images_for_cloak(unitSubunit->sprite);
				unitSubunit->status = unitSubunit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
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
		if (image)
		{
			do
			{
				next_image = image->link.next;
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
					}
					else
					{
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
					}
					else
					{
						g_free_sprites->head = sprite;
						g_free_sprites->tail = sprite;
					}
				}
				image = next_image;
			} while (next_image);
		}
	}
}