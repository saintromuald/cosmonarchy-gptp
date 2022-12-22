#include "weapon_spellhit.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "globals.h"
#include <algorithm>
#include <hooks/byte_operations_utils.h>
#include <cassert>
#include "weapon_fire.h"

namespace hooks {
	// Hooked to allow for replacing proc functions - Pr0nogo
	Unit* iterate_units_at_location_target_proc(Box16* location, Unit*(__fastcall* proc)(Unit*, Unit*), Unit* unit) { // 004E8280
		Unit** unitsInBounds = helper::get_all_units_in_bounds(location);
		Unit** unitsInBounds2 = unitsInBounds;
		Unit* aUnitInBound;
		if (proc || (aUnitInBound = *unitsInBounds) == 0) {
			aUnitInBound = *unitsInBounds;
			if (*unitsInBounds) {
				do {
					if (proc(aUnitInBound, unit)) {
						break;
					}
					aUnitInBound = unitsInBounds2[1];
					++unitsInBounds2;
				}
				while (aUnitInBound);
			}
		}
		s32 index = g_temp_units_list_arrays_counts_list_last_index[0] - 1;
		g_temp_units_list_arrays_counts_list_last_index[0] = index;
		*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list[index];
		return aUnitInBound;
	}

	// Hooked to clean up lardcode - Pr0nogo
	// Seems to be double-called from Maelstrom, no idea why
	char disable_unit(Unit* unit) { // 00492CC0
		Unit* subunit = unit->subunit;
		if (subunit) {
			subunit->status |= UnitStatus::Frozen;
		}
		helper::set_resource_target(unit);
		s16 unit_id = unit->id;
		if (scbw::unit_can_nuke(unit)) {
			Unit* nuke = unit->connected_unit;
			if (nuke) {
				if (nuke->id == UnitId::TerranNuclearMissile) {
					unit->connected_unit = 0;
					nuke->connected_unit = 0;
				}
			}
		}
		if (unit_id == UnitId::ProtossSolarion) {
			helper::return_all_interceptors(unit);
		}
		u8 order_id = unit->main_order_id;
		if (order_id == OrderId::Repair1 || order_id == OrderId::DroneLand) {
			unit->main_order_state = 0;
		}
		if (!orders_dat::disabling_keeps_target[order_id]) {
			unit->order_target.unit = 0;
		}
		char result;
		result = helper::ignore_all_scripts_and_go_to_idle(unit);
		Sprite* sprite = unit->sprite;
		if (sprite->flags & 8) {
			if (*g_client_selection_count > 1u) {
				result = helper::update_building_land_unit_selection(*(u32*)sprite, unit);
			}
		}
		unit->current_buttonset = 228;
		*control_under_mouse = 0;
		*control_under_mouse_value = 0;
		*force_button_refresh = 1;
		portrait_redraw[0] = 1;
		*status_screen_redraw = 1;
		return result;
	}

	void irradiate_hit(Unit* attacker, Point16 target, u8 attacking_player_id) {
		globals.create_effect(EffectId_Irradiate, 10 * 24, {target.x, target.y}, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, attacking_player_id, attacker);
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void optical_flare_hit(Unit* unused, u32 attacking_player_id, Bullet* bullet) {
		if (!bullet) {
			return;
		}

		const u32 radius = 2 * 32;
		helper::play_sound_from_unit(SoundId::Terran_Medic_Miopia1_wav, (Unit*)bullet, 1, 0);
		scbw::UnitFinder(
			bullet->position.x - radius,
			bullet->position.y - radius,
			bullet->position.x + radius,
			bullet->position.y + radius
		).for_each([&](Unit* target) {
			UnitExt* targetExt = get_unit_ext(target);
			targetExt->timers[TimerAuralAssaultBlind] = 2 * 24 + 1;
		});
	}

	// Bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void restore_hit(Unit* target, Bullet* bullet) {
		if (target->status & UnitStatus::IsHallucination) {
			target->remove();
		} else {
			// Create impact overlay
			scbw::restore_func(target);
		}
	}

	// Bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	// the timer is reset before this call 2 times out of 3 (with previousLockdownTimer holding the pre-reset value), and the 3rd case, previousLockdownTimer is set to 131 (0x83)
	void lockdown_hit(Unit* target, u8 timer, Bullet* bullet) {
		// Add lockdown overlay if no ongoing lockdown already
		if (target->lockdown_timer == 0) {
			u32 image_id = ImageId::LockdownField_Small + scbw::get_overlay_offset(target->id);
			Unit* overlay_unit = target->subunit ? target->subunit : target;
			overlay_unit->sprite->create_top_overlay(image_id, 0, 0, 0);
		}

		// This, added to the fact that target->lockdown_timer is set to 0 before this function, probably prevents lockdown to reset the duration of a previous lock
		if (target->lockdown_timer < timer) {
			target->lockdown_timer = timer;
		}

		// Should stop movements/orders/scripts of the unit, give it the status DoodadStatesThing.
		// May have an effect on selection. Performs scbw::refresh_console() code at the end
		disable_unit(target);
	}

	void weapon_bullet_hit(Bullet* bullet, Unit* target, u32 damage_divisor) { // 00479AE0
		u32 base_damage = helper::get_unit_bullet_damage(bullet, target);
		if (bullet->hit_flags & 2) {
			if (bullet->source_unit) {
				target->damage_with(bullet->weapon_id, base_damage, bullet->source_unit, bullet->player, bullet->direction1, damage_divisor);
			} else {
				helper::attack_overlay_and_notify(bullet->source_unit, target, bullet->weapon_id, bullet->direction1); // no damage, but still ask for retaliation
			}
		} else {
			target->damage_with(bullet->weapon_id, base_damage, bullet->source_unit, bullet->player, bullet->direction1, damage_divisor);
		}
	}

	u16 get_unit_width(Unit* unit) {
		return unit->get_right() - unit->get_left();
	}

	u16 get_unit_height(Unit* unit) {
		return unit->get_bottom() - unit->get_top();
	}

	bool is_in_range(Unit* source, Unit* target, u32 range) {
		u16 source_x = source->position.x;
		u16 source_y = source->position.y;
		u16 target_x = target->position.x;
		u16 target_y = target->position.y;

		u16 h_diff = abs(source_x - target_x);
		u16 v_diff = abs(source_y - target_y);
		u16 h_range = range + get_unit_width(source) / 2 + get_unit_width(target) / 2 + 1;
		u16 v_range = range + get_unit_height(source) / 2 + get_unit_height(target) / 2 + 1;

		if (h_diff > h_range || v_diff > v_range) {
			return false;
		}
		return true;
	}

	void iscript_attackmelee(Unit* attacker) { // 00479B40
		UnitExt* attackerExt = get_unit_ext(attacker);
		Unit* target = attacker->order_target.unit;
		if (target) {
			u8 weapon_id;
			// Lakizilisks can only attack while burrowed even if given a melee attack, according to this code.
			if (attacker->id != UnitId::ZergLakizilisk
			|| (attacker->status & UnitStatus::Burrowed
			||  attacker->status & UnitStatus::InBuilding)) { // Attempt to allow Lakizilisks to attack from within Bunkers. Didn't work, but may still be part of the solution. -Pr0nogo
				weapon_id = units_dat::ground_weapon[attacker->id];
			} else {
				weapon_id = WeaponId::None;
			}
			attackerExt->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::damage_factor[weapon_id] + 1;
			attackerExt->values[ValueAI_PreventTargeting] = 0;

			// Previously checked for Hallucination status, but there was no functional difference in the code!
			// Refactored 15 Sep 2022. -Pr0nogo
			u32 damage = helper::get_unit_damage_bonus(attacker, weapon_id);
			damage = ((u16)damage + weapons_dat::damage_amount[weapon_id]) * 256;
			if (attackerExt->values[ValueDeconstruction]) {
				scbw::do_deconstruction_damage(attacker, weapon_id);
			}
			target->damage_with(weapon_id, damage, attacker, attacker->player, attacker->current_direction_1, 1);

			// Cloaked units - decloak
			// Cabalist uses silent strike, so the check has to be in weapon_damage. -Veeq7
			if (attacker->id != UnitId::ProtossCabalist) {
				scbw::decloak_proc(attacker);
			}
			apply_weapon_cooldown(attacker, weapon_id);
		}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	// In the original code, the parameters are taken from Bullet as:
	//	attacker from bullet->source_unit
	//	x and y from bullet->sprite->position
	void emp_shockwave_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet) {
		scbw::create_and_init_thingy(ImageId::DefensiveMatrixFront_Small, x, y, 11, 19);
		static u16* const maxBoxRightValue = (u16*)0x00628450;	// should usually be g_map_tile_size->width * 32
		static u16* const maxBoxBottomValue = (u16*)0x006284B4;	// should usually be g_map_tile_size->height * 32

		// define the base box of effect
		Box16 aoe = {};
		aoe.left = (u16)x - weapons_dat::inner_splash_radius[WeaponId::EMP_Shockwave];
		aoe.right = (u16)x + weapons_dat::inner_splash_radius[WeaponId::EMP_Shockwave];
		aoe.top = (u16)y - weapons_dat::inner_splash_radius[WeaponId::EMP_Shockwave];
		aoe.bottom = (u16)y + weapons_dat::inner_splash_radius[WeaponId::EMP_Shockwave];

		// check and fix effect if beyond width of map
		if (aoe.left < 0) {
			aoe.left = 0;
		} else if (aoe.right > *maxBoxRightValue) {
			aoe.right = *maxBoxRightValue;
		}
		// check and fix effect if beyond height of map
		if (aoe.top < 0) {
			aoe.top = 0;
		} else if (aoe.bottom > *maxBoxBottomValue) {
			aoe.bottom = *maxBoxBottomValue;
		}

		// find all units in box of effect and pick the first
		Unit** units_in_aoe = helper::get_all_units_in_bounds(&aoe);
		Unit* current_unit = *units_in_aoe;
		while (current_unit) {
			if (current_unit != attacker // EMP doesn't affect the attacker
				&& (!attacker || current_unit != attacker->subunit)) {
				if (current_unit->status & UnitStatus::IsHallucination) {
					current_unit->remove();
				} else if (current_unit->stasis_timer == 0) { // don't affect units in stasis
					current_unit->energy -= 100;
					current_unit->shields -= 100;
					if (current_unit->sprite->flags & SpriteFlags::Selected && current_unit->sprite) { // Follows original code, though this order of tests doesn't make sense
						helper::set_all_image_group_flags_palette_11(current_unit->sprite); // redraw the unit
					}
				}
			}
			units_in_aoe++; // go on next unit of the list (or null)
			current_unit = *units_in_aoe;
		}
		// reload the previous temporary unit list from before the call to get_all_units_in_bounds
		*g_temp_units_list_arrays_counts_list_last_index = *g_temp_units_list_arrays_counts_list_last_index - 1;
		*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list[*g_temp_units_list_arrays_counts_list_last_index];
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void corrosive_acid_hit(u32 attacking_player_id, s32 x, s32 y, Bullet* bullet) {
		Box16 coords = {};
		coords.left = x - 64;
		coords.bottom = y + 64;
		coords.top = y - 64;
		coords.right = x + 64;

		// IterateUnitsAtLocationTargetProc_CorrosiveAcid(attacking_player_id,&coords);
		// Originally spreads.
		if (bullet->attack_target.unit) {
			auto vec = scbw::get_clarion_links(bullet->attack_target.unit);
			if (vec.size() != 0) {
				for (auto unit : vec) {
					helper::apply_spores(unit);
				}
			} else {
				helper::apply_spores(bullet->attack_target.unit);
			}
		}
	}

	// No bullet because the code normally calling this function loses easily-available information (and getting it the hard way seems overkill)
	void consume_hit(Unit* attacker, Unit* target) {
		if (!attacker || !target) {
			return;
		}

		if (target && !(target->status & UnitStatus::Invincible) && helper::get_spell_stat_string(target, TechId::Consume, attacker->player) == 0) { // returning a value != 0 means there was an error
			helper::increment_unit_death_scores(target, attacker->player);
			if (!(target->status & UnitStatus::IsHallucination)) {
				EntryArray units = {};
				unit_search_radial(&units, attacker->position, 3 * 32);
				unit_search_filter(&units, attacker, FilterAlly | FilterIncludingSelf);
				for (u32 i = 0; i < units.count; ++i) {
					Unit* unit = units.units[i];
					unit->heal_hp_then_shields(target->hitpoints, attacker);
					unit->sprite->create_underlay(ImageId::SmallExplosion_Unused);
				}
			}
			target->remove();
		}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void ensnare_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet) {
		Thingy* ensnare_effect_sprite = helper::create_thingy(SpriteId::Ensnare, x, y, 0);
		if (ensnare_effect_sprite) {
			ensnare_effect_sprite->sprite->elevation = 19; // 0x13
			helper::set_thingy_visibility_flags(ensnare_effect_sprite);
		}
		Box16 coords;
		coords.top = y - 64;
		coords.right = x + 64;
		coords.left = x - 64;
		coords.bottom = y + 64;
		helper::IterateUnitsAtLocationTargetProc_Ensnare(attacker, &coords);
	}

	s32 ensnare_proc(Unit* target, Unit* attacker) {
		if (target != attacker
			&& !(units_dat::base_property[target->id] & UnitProperty::Building)
			&& target->id != UnitId::ZergInfestedTerran) {
			if (!(target->status & UnitStatus::Invincible) && !(target->status & UnitStatus::Burrowed)) {
				helper::createEnsnareOverlay(target);
			}
		}
		return 0;
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void dark_swarm_hit(s32 x, s32 y, u32 attacking_player_id, Bullet* bullet) {
		// Adjust location where Dark Swarm will be created if necessary (by modifying coords)
		Point16 coords = {x, y};
		helper::fix_target_location(&coords, UnitId::Spell_DarkSwarm);

		// The dark swarm is created for player 11 (neutral), not for the owner
		Unit* dark_swarm_unit = helper::create_unit(UnitId::Spell_DarkSwarm, coords.x, coords.y, 11); 
		if (!dark_swarm_unit) {
			helper::display_last_network_error_for_player(attacking_player_id);
		} else {
			dark_swarm_unit->status |= UnitStatus::NoCollide;
			dark_swarm_unit->sprite->elevation = 11;

			// update various stuff (set hp, set shield...) not finished on create_unit
			helper::update_unit_stats_and_finish_building(dark_swarm_unit);
			helper::update_unit_strength(dark_swarm_unit);
			if (dark_swarm_unit->remove_timer == 0 || dark_swarm_unit->remove_timer > 900) {
				dark_swarm_unit->remove_timer = 900;
			}
		}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void broodling_hit(Unit* attacker, Unit* target, Bullet* bullet) {
		u32 broodlings_to_spawn = 3;
		ActiveTile* tile = scbw::get_active_tile_at_coords(bullet->sprite->position.x, bullet->sprite->position.y);
		if (tile->has_creep || tile->creep_receding) {
			broodlings_to_spawn = 5;
		}
		Point16 current = bullet->sprite->position;
		current.x -= 2;
		current.y -= 2;
		while (broodlings_to_spawn > 0) {
			// Check various conditions before attempting to create a broodling
			if (helper::can_unit_id_fit_at_xy(current.x, current.y, UnitId::ZergRilirokor)) {
				Point16 in_pos = current;
				Point16 out_pos = {};

				Box16 move_area = {};
				move_area.left = current.x - 32;
				move_area.top = current.y - 32;
				move_area.right = current.x + 32;
				move_area.bottom = current.y + 32;

				if (scbw::check_unit_collision_pos(attacker, &in_pos, &out_pos, &move_area, false, 0)) {
					current = out_pos;
				}
			}

			// Adjust the position to welcome the unit...I think
			Point16 pos = current;
			helper::fix_target_location(&pos, UnitId::ZergRilirokor);

			// Create the broodling
			Unit* created_broodling = helper::create_unit(UnitId::ZergRilirokor, pos.x, pos.y, attacker->player);
			UnitExt* broodlingExt = get_unit_ext(created_broodling);
			broodlingExt->units[UnitBroodlingMother] = attacker;

			if (!created_broodling) {
				helper::display_last_network_error_for_player(attacker->player);
			} else {
				helper::play_sound_from_unit(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, created_broodling, 1, 0);

				// Update various stuff (set hp, set shield...) not finished on create_unit
				helper::update_unit_stats_and_finish_building(created_broodling);
				helper::update_unit_strength(created_broodling);
				u32 timer = 720;
				if (created_broodling->remove_timer == 0 || created_broodling->remove_timer > timer) {
					created_broodling->remove_timer = timer;
				}

				if (attacker->unit_ai) {
					helper::ai_training_unit(created_broodling, attacker);
				}
			}

			current.y += 4;
			current.x += 4;
			broodlings_to_spawn--;
		}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void plague_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet) {
		if (attacker) {
			globals.create_effect(EffectId_PlagueField, 10 * 32, {(u16)x, (u16)y}, 3 * 32, SpriteId::PlagueField, 5, attacker->player, attacker);
		}
		//if (attacker) {
		//	u32 distance = 64;
		//	Box16 coords = {};
		//	coords.left   = x - distance;
		//	coords.top    = y - distance;
		//	coords.right  = x + distance;
		//	coords.bottom = y + distance;
		//	helper::iterate_units_at_location_target_proc_plague(attacker, &coords);
		//
		//	UnitExt* attackerExt = get_unit_ext(attacker);
		//	scbw::UnitFinder(
		//		x - distance,
		//		y - distance,
		//		x + distance,
		//		y + distance
		//	).for_each([&](Unit* unit) {
		//		if (attacker && attackerExt->timers[TimerEnsnaringBrood] > 0
		//			&& !(units_dat::base_property[unit->id] & UnitProperty::Building))
		//		{
		//			scbw::apply_ensnare(unit);
		//		}
		//	});
		//}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void maelstrom_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet) {
		Thingy* maelstrom_effect_sprite = helper::create_thingy(SpriteId::Maelstrom_Hit, x, y, 0);
		if (maelstrom_effect_sprite) {
			maelstrom_effect_sprite->sprite->elevation = 17;
			helper::set_thingy_visibility_flags(maelstrom_effect_sprite);
		}
		helper::play_sound_from_unit(SoundId::Protoss_Darchon_Parahit_wav, attacker, 1, 0);

		// TODO: Unit search
		const u32 radius = 48;
		scbw::UnitFinder(
			x - radius,
			y - radius,
			x + radius,
			y + radius
		).for_each([attacker](Unit* unit) {
			maelstrom_proc(unit, attacker);
		});
	}

	// Hooked to modify maelstrom timer and conditions; however, this hook is not called yet, we may need to hook iterate_units_at_location_target_proc - 004E8280 - Pr0nogo
	bool maelstrom_proc(Unit* target, Unit* source) { // 004F6760, always returns 0
		s32 unit_status = target->status;
		if (unit_status & UnitStatus::IsHallucination) {
			helper::orders_self_destructing(target);
		} else {
			if (target != source && !(unit_status & UnitStatus::Invincible)) {
				if (!(unit_status & UnitStatus::Burrowed)) {
					helper::set_maelstrom_timer(target, MindTyrantMaelstromTimer);
				}
			}
		}
		return 0;
	}

	// No bullet because this function is called in another context without the bullet.
	void mind_control_hit(Unit* attacker, Unit* target) {
		if (scbw::unit_is_stunned(target)) {
			attacker->heal_energy(techdata_dat::energy_cost[TechId::MindControl] * 256, attacker);
		}
		scbw::decloak_proc(attacker);
		if (target->status & UnitStatus::IsHallucination) {
			target->remove();
		} else {
			scbw::mind_control(attacker, target);
		}
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void stasis_field_hit(Unit* attacker, s32 x, s32 y, Bullet* bullet) {
		scbw::create_and_init_thingy(SpriteId::Stasis_Field_Hit, x, y, 11, 17);

		Box16 coords = {};
		coords.left = x - 48;
		coords.top = y - 48;
		coords.right = x + 48;
		coords.bottom = y + 48;
		helper::IterateUnitsAtLocationTargetProc_StasisField(attacker, &coords);
	}

	// Bullet is not normally a part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	Unit* create_disruption_web(u32 attacking_player_id, s32 x, s32 y) {
		Point16 coords;
		Unit* disruption_web_unit;
		coords.x = x;
		coords.y = y;

		// adjust location where Disruption Web will be created if necessary (by modifying coords)
		helper::fix_target_location(&coords, UnitId::Spell_DisruptionWeb);
		disruption_web_unit = helper::create_unit(UnitId::Spell_DisruptionWeb, coords.x, coords.y, 11);
		if (!disruption_web_unit) {
			helper::display_last_network_error_for_player(attacking_player_id);
		} else {
			disruption_web_unit->status |= UnitStatus::NoCollide;
			disruption_web_unit->sprite->elevation = 11;
			// update various stats (set hp, set shield...) not finished on create_unit
			helper::update_unit_stats_and_finish_building(disruption_web_unit);
			helper::update_unit_strength(disruption_web_unit);
			if (disruption_web_unit->remove_timer == 0
				|| disruption_web_unit->remove_timer > 360) {
				disruption_web_unit->remove_timer = 360; // 0x0168
			}
		}
		return disruption_web_unit;
	}

	void disruption_web_hit(u32 attacking_player_id, s32 x, s32 y, Bullet* bullet) {
		create_disruption_web(attacking_player_id, x, y);
	}

	u8 stasis_field_overlay(Unit* unit, u8 timer) { // 004F67B0
		if (!unit->stasis_timer) {
			Unit* final_unit = unit->subunit;
			if (!final_unit) {
				final_unit = unit;
			}
			u16 image_id = ImageId::StasisField_Small + scbw::get_overlay_offset(unit->id);
			unit->sprite->create_overlay(image_id, 0, 0, 0);
			unit->status |= UnitStatus::Invincible; // remove
		}
		if (timer > unit->stasis_timer) {
			unit->stasis_timer = timer;
		}
		return disable_unit(unit);
	}

	void create_scanner(Unit* unit) { // 00464E40
		Unit* v1; // esi
		u16 v2; // ax
		bool v3; // zf
		GroupFlag targetsFound2; // al
		u8 iterate; // al
		s16 v6; // cx
		s16 v7; // ax
		char* v8; // eax
		s16 v9; // cx
		Unit* v10; // eax
		Unit* v11; // ebx
		s16 v12; // ax
		Point16 a2; // [esp+4h] [ebp-4h]

		v1 = unit;
		if (!(*g_cheat_state & 0x800)) {
			v2 = techdata_dat::energy_cost[4] * 256;
			v3 = v1->energy >= v2;
			//			a2 = (Point32)(v1->energy < v2);		// check this out
			if (!v3) {
				targetsFound2 = units_dat::group_flags[(u16)v1->id];
				if (targetsFound2.is_zerg) {
					iterate = 0;
				} else if (targetsFound2.is_terran) {
					iterate = 1;
				} else {
					iterate = (targetsFound2.is_protoss) != 0 ? 2 : 4;
				}
				v6 = iterate + 864;
				v7 = iterate + 863;
				if (!v6) {
					goto LABEL_15;
				}
				if (v7 < *g_stat_txt_tbl) {
					v8 = (char*)g_stat_txt_tbl + g_stat_txt_tbl[v7 + 1];
					if (!v8) {
				LABEL_15:
						if (v1->order_queue_head) {
							v1->user_action_flags |= 1u;
							helper::orders_nothing_2(v1);
						} else if (v1->unit_ai) {
							helper::issue_order_targeting_nothing(v1, 0x9Cu);
						} else {
							helper::issue_order_targeting_nothing(v1, units_dat::return_to_idle_order[(u16)v1->id]);
						}
						return;
					}
				} else {
					v8 = "";
				}
				if ((u8)v1->player == *g_local_nation_id && *g_active_player_id == *g_local_human_id) {
					helper::set_unit_stat_txt_error_message(v8);
				}
				goto LABEL_15;
			}
		}
		v9 = v1->order_target.pt.x;
		a2.y = v1->order_target.pt.y;
		a2.x = v9;
		helper::fix_target_location(&a2, UnitId::Spell_ScannerSweep);
		v10 = helper::create_unit(0x21u, a2.x, a2.y, (u8)v1->player);
		v11 = v10;
		if (v10) {
			helper::update_unit_stats_and_finish_building(v10);
			helper::update_unit_strength(v11);
			helper::play_sound_from_unit(388, v1, 1, 0);
			if (!(*g_cheat_state & 0x800))
			{
				v1->spend_unit_energy(techdata_dat::energy_cost[4] * 256);
			}
			helper::to_idle(v1);
		} else {
			helper::display_last_network_error_for_player(v1->player);
			helper::to_idle(v1);
		}
	}

	// Hooked to add Nydus handling, except it already existed! - Pr0nogo
	b32 give_unit_to_player(Unit* unit, u32 player) { // 004C8040
		u32 player_id_var = player;
		if (player == 13) {
			player_id_var = *g_current_trigger_player_id;
		}
		if (!(player_id_var < 0xC ? -(unit != 0) : 0)) {
			helper::SErrSetLastError(87);
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination) {
			return 0;
		}
		s16 unit_id = unit->id;
		if (unit_id == UnitId::ResourceMineralField
		||  unit_id == UnitId::ResourceMineralFieldType2
		||  unit_id == UnitId::ResourceMineralFieldType3
		||  unit_id == UnitId::ResourceVespeneGeyser
		||  unit_id == UnitId::ProtossSteward
		||  unit_id == UnitId::ProtossPlasmaShell
		||  units_dat::base_property[unit_id] & UnitProperty::Addon
		||  unit->player == player_id_var)
		{
			return 0;
		}
		if (helper::unit_is_geyser(unit)) {
			Unit* hidden_miner = helper::get_hidden_miner(unit);
			if (hidden_miner) {
				helper::give_all_unit_properties_to_player(hidden_miner, player_id_var);
			}
		}
		helper::give_all_unit_properties_to_player(unit, player_id_var);
		const u32 give_units_proc = 0x004C7CE0;
		if (helper::unit_is_active_transport(unit)) {
			helper::iterateLoadedUnits(unit, (Unit*(__fastcall*)(Unit*, Unit*))give_units_proc, player_id_var);
		}
		if (helper::unitHasHanger(unit)) {
			helper::forHangarUnits(unit, (Unit*(__fastcall*)(Unit*, Unit*))give_units_proc, player_id_var);
		}
		if (unit->status & UnitStatus::GroundedBuilding) {
			Unit* connected_unit = unit->building.addon;
			if (connected_unit) {
				helper::give_all_unit_properties_to_player(connected_unit, player_id_var);
			}
			if (helper::is_constructing_addon(unit)) {
				Unit* creating_unit = helper::getCreatingUnit(unit);
				helper::give_all_unit_properties_to_player(creating_unit, player_id_var);
			}
			if (unit->id == UnitId::ZergNydusCesiant) {
				Unit* nydus_exit = unit->building.nydus_exit;
				if (nydus_exit) {
					helper::give_all_unit_properties_to_player(nydus_exit, player_id_var);
				}
			}
		}
		return 1;
	}

	// Hooked to investigate Maelstrom double-calling disable_unit. -Pr0nogo
	char set_maelstrom_timer(Unit* unit, u8 duration) // 004553F0
	{
		if (!unit->maelstrom_timer)
		{
			u32 overlay_offset = scbw::get_overlay_offset(unit->id);
			Unit* final_unit = unit->subunit;
			if (!final_unit)
			{
				final_unit = unit;
			}
			final_unit->sprite->create_overlay(ImageId::MaelstromOverlay_Small + overlay_offset, 0, 0, 0);
		}
		u32 timer = unit->maelstrom_timer;
		if (duration > timer)
		{
			timer = duration;
		}
		unit->maelstrom_timer = timer;
		return hooks::disable_unit(unit);
	}
}