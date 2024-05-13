#include "unit_destructor_special.h"
#include "../BW/api.h"
#include "../hook_tools.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "logger.h"
#include <BW/UnitFinder.h>
#include "globals.h"
#include <hooks/limits/sfx_extender.h>

void augur_proc(Unit* augur, s32 radius) {
	Unit* target = bw::UnitFinder::get_nearest_target(
		augur->get_x() - radius,
		augur->get_y() - radius,
		augur->get_x() + radius,
		augur->get_y() + radius,
		augur,
	[&](Unit* unit) {
		// Don't attack friendly / allied units
		if (!AssertSprite(unit->sprite)) {
			return false;
		}
		if (unit->player == augur->player) {
			return false;
		}
		if (bw::is_allied_to(unit->player, augur->player)) {
			return false;
		}
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		return true;
	});
	Thingy* thingy = helper::create_thingy(SpriteId::AugurDeathExplosion, augur->get_x(), augur->get_y(), augur->player);
	helper::play_sound_from_unit(bw::rand_between(SoundId::AugurDeath01, SoundId::AugurDeath04), (Unit*)thingy, 1, 0);
	if (target) {
		augur->order_target.unit = target;
		helper::create_bullet(WeaponId::AugurFinalHour, augur, augur->get_x(), augur->get_y(), augur->player, bw::get_angle(augur->get_x(), augur->get_y(), target->get_x(), target->get_y()));
	}
}

void augur_sacrifice(Unit* augur, s32 radius) {
	UnitExt* targetExt = get_unit_ext(augur);
	if (targetExt->values[ValueAugurSafetyCheck] == 0) {
		targetExt->values[ValueAugurSafetyCheck] = 1;
		augur_proc(augur, radius);
		augur->remove();
	}
}

// Identical to 00466170 kill_hangar_units
// Not hooked but used by hooked unit_destructor_special_cases
void kill_hangar_units(Unit* unit) {
	u32 random_value;
	while (unit->unit_with_hangar.in_hangar_count != 0) {
		Unit* child_inside = unit->unit_with_hangar.in_hangar_child;
		unit->unit_with_hangar.in_hangar_child = child_inside->hangar_unit.hangar_link.prev;
		child_inside->hangar_unit.parent = 0;
		child_inside->remove();
		unit->unit_with_hangar.in_hangar_count--;
	}
	Unit* child_outside = unit->unit_with_hangar.out_hangar_child;
	while (unit->unit_with_hangar.out_hangar_count != 0) {
		child_outside->hangar_unit.parent = 0;

		// Kill Stewards only (Plasma Shells will defuse anyway)
		if (child_outside->id != UnitId::ProtossPlasmaShell) {
			u16 random_death_timer;
			// TODO: Remove RNG, make farthest Stewards detonate first
			// hardcoded RNG management
			if (g_is_in_game_loop) {
				static u32* randomnessCounter_0051C6AC = (u32*)0x0051C6AC;
				static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;
				*randomnessCounter_0051C6AC = *randomnessCounter_0051C6AC + 1;
				*randomnessCounter_0051CA18 = *randomnessCounter_0051CA18 + 1;
				g_last_random_number = (g_last_random_number * 0x015A4E35) + 1;
				// number between 0 and 32766
				random_value = (g_last_random_number / 65536) & 0x7FFF;
			} else {
				random_value = 0;
			}
			// should produce a number between 15 and 45 (or maybe 46)
			random_death_timer = ((random_value * 31) / 32768) + 15;
			if (child_outside->remove_timer == 0
				|| random_death_timer < child_outside->remove_timer) {
				child_outside->remove_timer = random_death_timer;
			}
		}
		// 00466239
		Unit* child_outside_old = child_outside;
		child_outside = child_outside->hangar_unit.hangar_link.prev;
		child_outside_old->hangar_unit.hangar_link.prev = 0;
		child_outside_old->hangar_unit.hangar_link.next = 0;
		--unit->unit_with_hangar.out_hangar_count;
	}
	unit->unit_with_hangar.out_hangar_child = 0;
}

// Identical to 00468770 remove_harvesters
// Not hooked but used by hooked unit_destructor_special_cases
void free_resource_container(Unit* resource) {
	resource->building.resource.is_mining = 0;
	Unit* worker = resource->building.resource.next_gatherer;
	while (worker) {
		if (worker->worker.harvest_link.prev) { // untie the previous worker from current worker (tie it to next)
			worker->worker.harvest_link.prev->worker.harvest_link.next = worker->worker.harvest_link.next;
		} else {
			resource->building.resource.next_gatherer = worker->worker.harvest_link.next;
		}
		if (worker->worker.harvest_link.next) { // untie the next worker from current worker (tie it to previous)
			worker->worker.harvest_link.next->worker.harvest_link.prev = worker->worker.harvest_link.prev;
		}
		Unit* next_worker = worker->worker.harvest_link.next;
		worker->worker.harvest_target = 0;
		worker->worker.harvest_link.prev = 0;
		worker->worker.harvest_link.next = 0;
		worker = next_worker;
	}
}

void unit_on_death_behaviors(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	// These do not require the unit to be completed
	switch (unit->id) {
		case UnitId::ProtossHerald:
		{
			ascend_corpses_in_area(unit->position, 2 * 32, unit->player);
		} break;
		case UnitId::TerranEidolon:
		case UnitId::TerranSeraph:
		{
			if (unit->building.nuke_missile) {
				(unit->building.nuke_missile)->sprite->play_iscript_animation(IscriptAnimation::Death, true);
				unit->building.nuke_missile = 0;
			}
		} break;
		case UnitId::TerranLobotomyMine:
		{
			EntryArray targets = {};
			unit_search_radial(&targets, unit->position, 2 * 32);
			for (u32 i = 0; i < targets.count; ++i) {
				Unit* victim = targets.units[i];
				UnitExt* victim_ext = get_unit_ext(victim);
				if (victim_ext->timers[TimerLobotomySlow] < 3 * 24 + 1) {
					victim_ext->timers[TimerLobotomySlow] = 3 * 24 + 1;
				}
			}
		} break;
		case UnitId::TerranMorningstar:
		{
			if (unit->connected_unit && unit->connected_unit->id == UnitId::TerranAnvil) {
				unit->connected_unit->building.silo.nuke = 0;
				unit->connected_unit->building.silo.is_ready = false;
			}
		} break;
		case UnitId::TerranAnvil:
		{
			if (unit->building.silo.nuke) {
				unit->building.silo.nuke->remove();
				unit->building.silo.nuke = 0;
			}
		} break;
		case UnitId::ProtossPylon:
		case UnitId::ProtossCrucible:
		{
			// code from hooks::remove_psi_field(Unit* unit)
			// copied because the content is hardcoded here
			if (unit->building.pylon_aura) {
				unit->building.pylon_aura->free();	// should be equivalent to SpriteDestructor @ 00497B40
				unit->building.pylon_aura = 0;
			}

			// begin hardcoding remove_from_psi_providers_list @ 00493100 to avoid use of a helper
			if (unit->psi_link.prev) { // untie the previous pylon from current pylon (tie it to next)
				unit->psi_link.prev->psi_link.next = unit->psi_link.next;
			}
			if (unit->psi_link.next) { // untie the next pylon from current pylon (tie it to previous)
				unit->psi_link.next->psi_link.prev = unit->psi_link.prev;
			}

			if (unit == *first_psi_field_provider) {
				*first_psi_field_provider = unit->psi_link.next;
			}
			unit->psi_link.prev = 0;
			unit->psi_link.next = 0;
			// end hardcoding remove_from_psi_providers_list @ 00493100 to avoid use of a helper
			*g_can_update_powered_status = true;
		} break;
		case UnitId::ZergNydusCesiant:
		{
			Unit* nydus_exit = unit->building.nydus_exit;
			if (nydus_exit) {
				unit->building.nydus_exit = 0;
				nydus_exit->building.nydus_exit = 0;

				if (unit_ext->values[ValueIsNydusExit] == 1) {
					bw::regress(nydus_exit, UnitId::ZergCagrantCesiant); // Lame hardcode
				} else {
					nydus_exit->remove();
				}
			}
		} break;
	}

	if (unit_ext->timers[TimerAurielIncineration] >= 1) {
		iquare_globals.create_effect(EffectId_SalamanderFire, 5 * 24, { unit->position.x, unit->position.y }, 0.5 * 32, SpriteId::SalamanderImpact, 5, unit_ext->values[ValueAurielIncinerationPlayer], 0);
		unit_ext->timers[TimerAurielIncineration] = 0;
		unit_ext->values[ValueNoCorpse] = 1;
	}

	// These require the unit to be completed
	if (unit->status & UnitStatus::Completed) {
		switch (unit->id) {
			case UnitId::TerranAnvil:
			{
				// Anvil - death while armed
				if (unit->building.silo.is_ready && !unit_ext->timers[TimerControlledDemolition] && !unit_ext->timers[TimerSalvageStunTimer]) {
					bw::create_and_init_thingy(SpriteId::Nuke_Hit, unit->position.x, unit->position.y, 11, 12);
					helper::create_bullet(WeaponId::NuclearMissile, unit, unit->position.x, unit->position.y, unit->player, 0);
				}
			} break;
			case UnitId::TerranAspirant:
			{
				iquare_globals.create_effect(EffectId_AspirantCriticality, 5 * 24, { unit->position.x, unit->position.y }, 2 * 32, SpriteId::Criticality, 5, unit->player, unit);
			} break;
			case UnitId::ProtossClarion:
			{
				// Phase Link
				bw::clear_clarion_links(unit);
			} break;
			case UnitId::ProtossAccantorOld:
			case UnitId::ProtossSolarion:
			{
				kill_hangar_units(unit);
			} break;
			case UnitId::ProtossPlasmaShell:
			case UnitId::ProtossSteward:
			{
				if (unit->hangar_unit.parent) {
					if (unit->hangar_unit.hangar_link.next) { // untie the next hangar_unit/scarab from current hangar_unit/scarab (tie it to previous)
						unit->hangar_unit.hangar_link.next->hangar_unit.hangar_link.prev = unit->hangar_unit.hangar_link.prev;
					} else {
						if (unit->hangar_unit.is_outside_hangar) {
							unit->hangar_unit.parent->unit_with_hangar.out_hangar_child = unit->hangar_unit.hangar_link.prev;
						} else {
							unit->hangar_unit.parent->unit_with_hangar.in_hangar_child = unit->hangar_unit.hangar_link.prev;
						}
					}
					if (unit->hangar_unit.is_outside_hangar) {
						unit->hangar_unit.parent->unit_with_hangar.out_hangar_count--;
					} else {
						unit->hangar_unit.parent->unit_with_hangar.in_hangar_count--;
					}

					if (unit->hangar_unit.hangar_link.prev) { // untie the previous hangar_unit/scarab from current hangar_unit/scarab (tie it to next)
						unit->hangar_unit.hangar_link.prev->hangar_unit.hangar_link.next = unit->hangar_unit.hangar_link.next;
					}
				} else {
					unit->hangar_unit.hangar_link.prev = 0;
					unit->hangar_unit.hangar_link.next = 0;
				}
			} break;
			case UnitId::ResourceMineralField:
			case UnitId::ResourceMineralFieldType2:
			case UnitId::ResourceMineralFieldType3:
			case UnitId::ResourceVespeneGeyser:
			case UnitId::TerranReservoir:
			case UnitId::TerranWellbore:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			case UnitId::ZergEzigrisant:
			{
				free_resource_container(unit);
			} break;
			case UnitId::ProtossAugur:
			{
				augur_sacrifice(unit);
			} break;
		}
	}

	{ // Spellcaster - restore energy on enemy death
		EntryArray unit_array = {};
		unit_search_radial(&unit_array, unit->position, unit->get_sight_range() * 32);
		unit_search_filter(&unit_array, unit, FilterEnemy | FilterSpellcaster); // TODO: Remove Rilirokor, Rililisk

		for (u32 i = 0; i < unit_array.count; ++i) {
			Unit* caster = unit_array.units[i];
			if (bw::get_distance_fast(unit->position, caster->position) <= caster->get_sight_range() * 32) {
				caster->heal_energy(5 * 256);
				caster->sprite->create_underlay(ImageId::SmallExplosion_Unused);
			}
		}
	}

	if (!unit_id_is_spell(unit->id)) { // Mortothrokor - Whetted Appetite - Add stacks when in vision
		EntryArray unit_array = {};
		unit_search_radial(&unit_array, unit->position, units_dat::sight_range[UnitId::ZergMortothrokor] * 32);
		unit_search_filter(&unit_array, unit, 0, {UnitId::ZergMortothrokor});

		for (u32 i = 0; i < unit_array.count; ++i) {
			Unit* mortothrokor = unit_array.units[i];
			if (bw::get_distance_fast(unit->position, mortothrokor->position) <= mortothrokor->get_sight_range() * 32) {
				UnitExt* unit_ext = get_unit_ext(mortothrokor);
				unit_ext->values[ValueMortothrokorStacks]++;
				unit_ext->timers[TimerMortothrokorPassive] = 2 * 24;
			}
		}
	}

	// Metastasis - Plague spreads when a unit dies
	if (unit->plague_timer) {
		helper::create_bullet(WeaponId::Plague, unit, unit->position.x, unit->position.y, unit->player, unit->current_direction_1);
		u32 range = 48;
		Box16 coords = {};
		coords.left   = unit->position.x - range;
		coords.top    = unit->position.y - range;
		coords.right  = unit->position.x + range;
		coords.bottom = unit->position.y + range;
		helper::iterate_units_at_location_target_proc_plague(unit, &coords);
	}

	// Gladius - Binding Plasma - proc on death
	if (unit_ext->timers[TimerBindingPlasma]) {
		if (bw::binding_plasma_proc(unit)); {
			helper::play_sound_from_unit(bw::rand_between(SoundId::GladiusBindingPlasmaProc01, SoundId::GladiusBindingPlasmaProc03), unit, 1, 0);
		}
		// TODO: Preserve timer and source position of dying unit, instead of immediately bouncing. -Pr0nogo
	}

	// Banter
	if (unit_ext->values[ValueSignifyStacks] >= 5) {
		// Hierophant
		EntryArray unit_array = {};
		unit_search_radial(&unit_array, unit->position, unit->get_sight_range() * 32);
		unit_search_filter(&unit_array, unit, FilterEnemy, { UnitId::ProtossHierophant });
		bool success = false;
		for (u32 i = 0; i < unit_array.count; ++i) {
			Unit* hierophant = unit_array.units[i];
			if (bw::get_distance_fast(unit->position, hierophant->position) <= hierophant->get_sight_range() * 32) {
				UnitExt* unit_ext = get_unit_ext(hierophant);
				if (success) {
					unit_ext->timers[TimerUnitBanter] = 24 * 60;
					continue;
				} else if (!unit_ext->timers[TimerUnitBanter]) {
					unit_ext->timers[TimerUnitBanter] = 24 * 60;
					u32 banter_sound = bw::rand_between(SoundId::HierophantBanterB01, SoundId::HierophantBanterB03);
					if (helper::play_sound_from_unit(banter_sound, unit, g_options_list->flags & 1, 99)) {
						helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[banter_sound] + 1000 * (sfx_dat::sound_data[banter_sound].duration_ms / 1000), 0, banter_sound);
					}
					success = true;
				}
			}
		}
	}
}

void unit_destructor_special_cases(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	unit->reset_supply_provided();

	// Cancel units that required an addon
	if (units_dat::base_property[unit->id] & UnitProperty::Addon) {
		Unit* parent = unit_ext->units[UnitAddonParent];
		if (parent) {
			u32 offset = 0;
			for (u32 i = 1; i < 5; ++i) {
				u32 slot = i + parent->build_queue_slot - offset;
				u32 unit_id = parent->build_queue[slot];
				if (unit_id == UnitId::None) {
					continue;
				}
				if (!bw::unit_id_is_available_without_an_addon(unit_id)) {
					helper::refund_queue_slot(parent, slot);
					offset++;
				}
			}
		}
	}

	// Blissful Embrace - Cleanup & Visual Effects
	if (unit_ext->values[ValueInfested]) {
		// Update creep
		bw::update_custom_creep_disappearance(unit);

		// Make this a sprite
		u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
		if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
			effect_image_id = ImageId::ZergBuildingExplosion;
		} else if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
			effect_image_id = ImageId::ZergAirDeathExplosion_Large;
		}
		unit->sprite->create_overlay(effect_image_id);
		helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZBldgDth_WAV, unit, true, 0);
	}

	// Proc on death behaviors
	unit_on_death_behaviors(unit);

	// Update custom creep disappearance
	bw::update_custom_creep_disappearance(unit);

	// Call kill all loaded units before UnitExt is cleared. Note that the original call is nops'ed
	helper::kill_all_loaded_units(unit);

	// Globals memory management
	iquare_globals.remove_unit_from_line_attack_table(unit, false);
	iquare_globals.unit_death(unit);
	unit_death_start(unit);
}

void clear_tile_flags_from_area(u32 flags, u32 tile_width, u32 tile_height, s32 tile_x, s32 tile_y) {
	for (s32 y = tile_height - 1; y >= 0; --y) {
		for (s32 x = tile_width - 1; x >= 0; --x) {
			s32 fx = tile_x + x;
			s32 fy = tile_y + y;
			if (fx >= 0 && fx < g_map_tile_size->width && fy >= 0 && fy < g_map_tile_size->height) {
				(*g_active_tile_array_int)[fx + fy * g_map_tile_size->width] &= ~flags;
			}
		}
	}
}

// Hooked to allow for player-colored corpses. -Pr0nogo
Thingy* iscript_create_sprite(Image* image, u16 id, u32 x, u32 y, u8 elevation) { // 004D7120
	Sprite* sprite = image->parent_sprite;
	u32 pos_x = x + image->horizontal_offset + sprite->position.x;
	u32 pos_y = y + image->vertical_offset   + sprite->position.y;

	if (bw::sprite_id_is_corpse(id)) {
		Unit* unit = *g_active_iscript_unit;
		if (unit) {
			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit_ext->values[ValueNoCorpse]) {
				return 0;
			}

			if (unit->id == UnitId::ZergVilgoleth) { // Currently does not leave a corpse
				sprite->create_overlay(ImageId::ZergAirDeathExplosion_Small);
				return 0;
			}
		}
	}

	Thingy* thingy = helper::create_thingy(id, pos_x, pos_y, 0);
	if (thingy) {
		if (id == SpriteId::Egg_Spawn) {
			thingy->sprite->player = sprite->player;
		} else if (bw::sprite_id_is_corpse(id)) {
			thingy->sprite->player = sprite->player;

			Unit* unit = *g_active_iscript_unit;
			if (unit) {
				ThingyExt* thingy_ext = get_thingy_ext(thingy);
				thingy_ext->player = unit->player;
				thingy_ext->unit_id = unit->id;
				thingy_ext->sight_range_before_death = unit->get_sight_range() * 32;
				thingy_ext->avenge_or_ascend_player = unit_avenges_or_ascends(unit);
				thingy_ext->create_origin = unit->create_origin;

				UnitExt* unit_ext = get_unit_ext(unit);
				if (unit_ext->timers[TimerAutocratControlledOpposition]) {
					reclaim_corpse(thingy, unit_ext->values[ValueAutocratControlledOppositionPlayer]);
				}
			}
		}
		thingy->sprite->elevation = elevation;
		helper::set_thingy_visibility_flags(thingy);
	}
	return thingy;
}

// Hooked to make new refineries revert to their previous node type. -Pr0nogo
s32 remove_subunit_or_gas_container(Unit* unit) // 004A0080
{
	u32 id = unit->id;
	if (units_dat::base_property[id] & UnitProperty::Subunit) {
		Unit* first_dying_unit = *g_first_dying_unit;
		if (first_dying_unit) {
			if (*g_last_dying_unit == first_dying_unit) {
				*g_last_dying_unit = unit;
			}
			unit->link.prev = first_dying_unit;
			unit->link.next = first_dying_unit->link.next;
			Unit* next_dying_u = first_dying_unit->link.next;
			if (next_dying_u) {
				next_dying_u->link.prev = unit;
			}
			first_dying_unit->link.next = unit;
		} else {
			*g_last_dying_unit = unit;
			*g_first_dying_unit = unit;
		}

		Unit* previous_player_u = unit->player_link.prev;
		if (previous_player_u) {
			previous_player_u->player_link.next = unit->player_link.next;
		}
		Unit* next_player_u = unit->player_link.next;
		if (next_player_u) {
			next_player_u->player_link.prev = unit->player_link.prev;
		}

		s32 player = unit->player;
		if (unit == first_player_unit->unit[player]) {
			first_player_unit->unit[player] = unit->player_link.next;
		}
		unit->player_link.prev = 0;
		unit->player_link.next = 0;
		helper::increment_unit_scores(unit, -1);
		unit->subunit = 0;
		return 1;
	}

	Sprite* sprite = unit->sprite;
	if (!sprite || sprite->flags & SpriteFlags::Hidden) {
		return 0;
	}
	if (id != UnitId::TerranReservoir && id != UnitId::TerranWellbore && id != UnitId::ZergEzigrisant) {
		if (id == UnitId::ZergExcisant) {
			helper::remove_creep_at_unit(sprite->position.x, sprite->position.y, id);
		} else if (id != UnitId::ProtossAquifer) {
			return 0;
		}
	}

	if (unit->status & UnitStatus::Completed) {
		helper::increment_unit_scores_ex(unit, -1, 0);
	}

	unit->status &= ~UnitStatus::Completed;
	u32 node_id = UnitId::ResourceVespeneGeyser;
	if (unit_id_is_ridge_refinery(unit->id)) {
		node_id = UnitId::ResourceVespeneRidge;
	}
	helper::replace_unit_with_id(unit, node_id);
	id = unit->id;
	unit->main_order_id = units_dat::human_idle_order[id];
	unit->order_target.unit = 0;
	unit->order_target.pt.y = 0;
	unit->order_target.pt.x = 0;
	unit->main_order_state = 0;
	unit->order_unit_id = UnitId::None;
	unit->hitpoints = units_dat::max_hitpoints[id];
	helper::give_unit(unit, 11, 0);
	helper::give_sprite(unit, 11);
	unit->status |= UnitStatus::Completed;
	helper::increment_unit_scores_ex(unit, 1, 1);

	if (g_is_placing_building) {
		helper::place_building_refinery_check(unit);
	}
	return 1;
}

// Hooked to troubleshoot Map Revealer crash. -Pr0nogo
void unit_die(Unit* unit) // 004A0740
{
	if (units_dat::base_property[(unsigned __int16)unit->id] & UnitProperty::Building) {
		*g_ignore_unit_flag80_clear_subcount = 1;
	}
	helper::unit_destructor_special_cases(unit);
	helper::kill_all_loaded_units(unit);
	helper::remove_references(unit);
	helper::clear_bullet_targets(unit);
	helper::remove_from_ai_structs(unit, (UnitAi*)1);
	helper::remove_worker_or_building_ai(unit, 0);

	UnitAi* unit_ai = unit->unit_ai;
	if (unit_ai) {
		if (unit_ai->type == AiType::Guard) {
			u32 times_died = unit_ai->guard_ai.times_died;
			unit_ai->guard_ai.parent = 0;
			if (times_died < 100) {
				unit_ai->guard_ai.times_died = times_died + 1;
			}
		}
	}
	helper::ai_remove_cleric(unit);
	unit->unit_ai = 0;

	if (!remove_subunit_or_gas_container(unit)) {
		helper::set_resource_target(unit);
		Sprite* sprite = unit->sprite;
		if (sprite && !(sprite->flags & SpriteFlags::Hidden)) {
			helper::end_collision_tracking(unit);
		}
		if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
			unit->is_cloaked = 0;
			helper::is_unit_cloaking(unit);
			if (unit->status & UnitStatus::RequiresDetection) {
				helper::end_cloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);
			}
		}

		helper::building_death(unit, sprite);
		if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding && unit->building.upgrade_id) {
			helper::clear_building_from_tile_flag(unit, unit->order_target.pt.x, unit->order_target.pt.y);
		}
		if (unit->path) {
			helper::reassign_path(unit);
		}

		Unit* building_unit = unit->current_build_unit;
		if (building_unit) {
			u32 secondary_id = unit->secondary_order_id;
			if (secondary_id == OrderId::Train || secondary_id == OrderId::TrainFighter) {
				helper::orders_self_destructing(building_unit);
			}
		}
		if (unit->secondary_order_id != OrderId::Nothing2) {
			unit->secondary_order_id = OrderId::Nothing2;
			unit->secondary_order_pos.x = 0;
			unit->secondary_order_pos.y = 0;
			unit->current_build_unit = 0;
			unit->secondary_order_state = 0;
		}
		helper::drop_powerup(unit);
		helper::remove_from_selections(unit);
		if (sprite) {
			helper::remove_from_client_selection_3(unit);
			helper::remove_selection_circle(sprite);
		}
		helper::increment_unit_scores(unit, -1);
		if (unit->status & UnitStatus::Completed) {
			helper::increment_unit_scores_ex(unit, -1, 0);
		}
		if (!g_is_in_replay && unit->player == *g_local_nation_id && g_is_placing_building
			&& !helper::CMDRECV_place_building_allowed(*g_active_portrait_unit, *g_place_building_order_id, *g_place_building_unit_id))
		{
			helper::refresh_layers_3_and_4();
			helper::end_building_placement();
		}
		helper::remove_unit_from_lists(unit);
		bw::refresh_console();
	}
}

// Hooked to troubleshoot Map Revealer crash. -Pr0nogo
Unit* remove_unit_from_lists(Unit* unit) // 0049E3F0
{
	Unit* prev_unit = unit->player_link.prev;
	if (prev_unit) {
		prev_unit->player_link.next = unit->player_link.next;
	}

	Unit* next_unit = unit->player_link.next;
	if (next_unit) {
		next_unit->player_link.prev = unit->player_link.prev;
	}

	u32 player = unit->player;
	if (unit == first_player_unit->unit[player]) {
		first_player_unit->unit[player] = unit->player_link.next;
	}
	unit->player_link.prev = 0;
	unit->player_link.next = 0;

	if (unit->id == UnitId::Spell_ScannerSweep || unit->id == UnitId::Special_MapRevealer) {
		if (*g_first_scanner_sweep == unit) {
			*g_first_scanner_sweep = unit->link.next;
		}
		if (*g_last_scanner_sweep == unit) {
			*g_last_scanner_sweep = unit->link.prev;
		}
		if (unit->link.prev) {
			unit->link.prev->link.next = unit->link.next;
		}
		if (unit->link.next) {
			unit->link.next->link.prev = unit->link.prev;
		}
		unit->link.prev = 0;
		unit->link.next = 0;
	} else if (unit->sprite->flags & SpriteFlags::Hidden)
	{
		unit = helper::clist_remove_hidden_unit_entry(unit);
	} else {
		unit = helper::clist_remove_used_unit_entry(unit);
	}

	Unit* dying_unit = *g_first_dying_unit;
	if (*g_first_dying_unit)
	{
		if (*g_last_dying_unit == *g_first_dying_unit) {
			*g_last_dying_unit = unit;
		}
		unit->link.prev = *g_first_dying_unit;
		unit->link.next = dying_unit->link.next;
		Unit* linked_dying_unit = dying_unit->link.next;
		if (linked_dying_unit) {
			linked_dying_unit->link.prev = unit;
		}
		dying_unit->link.next = unit;
	} else {
		*g_last_dying_unit = unit;
		*g_first_dying_unit = unit;
	}
	return unit;
}

// Hooked to fix abandon addon transmission playing when structure dies. -Pr0nogo
void building_death(Unit* unit, Sprite* sprite) // 0049FD00
{
	if (unit->status & UnitStatus::GroundedBuilding) {
		if (unit->secondary_order_id == OrderId::BuildAddon) {
			Unit* current_build_unit = unit->current_build_unit;
			if (current_build_unit) {
				if (!(current_build_unit->status & UnitStatus::Completed)) {
					helper::cancel_unit(current_build_unit);
				}
			}
		}
		if (unit->building.addon) {
			disconnect_addon_no_event(unit);
		}
		if (unit->building.tech_id != TechId::None) {
			helper::unit_refund_partial_technology(unit);
		}
		if (unit->building.upgrade_id != UpgradeId::None) {
			helper::unit_refund_partial_upgrade(unit);
		}
		u32 build_queue_id = unit->build_queue[unit->build_queue_slot % 5];
		if (build_queue_id != UnitId::None && !(units_dat::base_property[build_queue_id] & UnitProperty::Building)) {
			helper::refund_all_queue_slots(unit);
		}
		if (g_is_placing_building) {
			helper::place_building_proc();
		}
		if (units_dat::group_flags[unit->id].is_zerg) {
			if (helper::unit_is_morphing(unit)) {
				unit->status |= UnitStatus::Completed;
			}
			if (!helper::update_creep_disappearance_unit(unit)) {
				unit->status |= UnitStatus::UNKNOWN2;
			}
		}
	}
}

//-------- Actual hooking --------//

void __declspec(naked) unit_destructor_special_cases_wrapper()
{
	static Unit* unit;
	__asm {
		MOV unit, EAX
		PUSHAD
	}
	unit_destructor_special_cases(unit);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) clear_tile_flags_from_area_wrapper() {
	static u32 flags;
	static u32 tile_width;
	static u32 tile_height;
	static s32 tile_x;
	static s32 tile_y;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x8]
		MOV tile_x, EAX
		MOV EAX, [EBP + 0xC]
		MOV tile_y, EAX
		POP EAX
		MOV flags, EAX
		MOV tile_width, EDI
		MOV tile_height, ECX
		PUSHAD
	}
	clear_tile_flags_from_area(flags, tile_width, tile_height, tile_x, tile_y);
	__asm {
		POPAD
		MOV ESP, EBP
		POP EBP
		RETN 0x8
	}
}

void __declspec(naked) iscript_create_sprite_wrapper() {
	static Thingy* result;
	static Image* image;
	static u16 id;
	static u32 x;
	static u32 y;
	static u8 elevation;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x08]
		MOV id, AX
		MOV EAX, [EBP + 0x0C]
		MOV x, EAX
		MOV EAX, [EBP + 0x10]
		MOV y, EAX
		MOV EAX, [EBP + 0x14]
		MOV elevation, AL
		POP EAX
		MOV image, EAX
		PUSHAD
	}
	result = iscript_create_sprite(image, id, x, y, elevation);
	__asm {
		POPAD
		MOV EAX, result
		MOV ESP, EBP
		POP EBP
		RETN 0x10
	}
}

void __declspec(naked) remove_subunit_or_gas_container_wrapper() {
	static s32 result;
	static Unit* unit;
	__asm {
		MOV unit, EAX
		PUSHAD
	}
	result = remove_subunit_or_gas_container(unit);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

void __declspec(naked) unit_die_wrapper() {
	static Unit* unit;
	__asm {
		MOV unit, EAX
		PUSHAD
	}
	unit_die(unit);
	__asm {
		POPAD
		MOV EAX, 0 // unused return value
		RETN
	}
}

void __declspec(naked) remove_unit_from_lists_wrapper() {
	static Unit* unit;
	static Unit* result;
	__asm {
		MOV unit, EAX
		PUSHAD
	}
	result = remove_unit_from_lists(unit);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

void __declspec(naked) building_death_wrapper() {
	static Unit* unit;
	static Sprite* sprite;
	__asm {
		MOV unit, EAX
		MOV sprite, EDX
		PUSHAD
	}
	building_death(unit, sprite);
	__asm {
		POPAD
		RETN
	}
}

	// PATCHES \\

void __declspec(naked) clear_bullet_targets_patch_8AACC() {
	__asm {
		loc_48AAD0:
		CMP[EAX + 0x5C], ECX//bullet->attack_target.unit != unit
		JNE		offset loc_48AAEE
		MOV		ESI, [ECX + 0xC]
		MOV		DX, [ESI + 0x14]
		MOV[EAX + 0x58], DX//bullet->attack_target.pt.x = unit->sprite->position.x
		MOV		DX, [ESI + 0x16]
		MOV[EAX + 0x5A], DX//bullet->attack_target.pt.y = unit->sprite->position.y
		XOR		EDX, EDX
		MOV[EAX + 0x5C], EDX//bullet->attack_target.unit = NULL

		loc_48AAEE :
		CMP[EAX + 0x64], ECX//bullet->source_unit != unit
		JNE		offset loc_48AAF6
		CMP[ECX + 0x64], 103//unit->id == UnitId::ZergLakizilisk
		JE		offset loc_48AAF6
		MOV[EAX + 0x64], EDX//bullet-source_unit = NULL

		loc_48AAF6 :
		MOV		EAX, [EAX + 4]//bullet = bullet->link.next
		CMP		EAX, EDX//bullet != NULL
		JNE		offset loc_48AAD0
		POP		ESI
		RET
	}
}	

namespace hooks {
	void inject_unit_destructor_special_hooks() {
		jump_patch(unit_destructor_special_cases_wrapper,	0x0049F4A0, 2);
		jump_patch(clear_tile_flags_from_area_wrapper,		0x00472E50, 2);
		jump_patch(iscript_create_sprite_wrapper,			0x004D7120, 1);
		jump_patch(remove_subunit_or_gas_container_wrapper,	0x004A0080, 3);
	//	jump_patch(unit_die_wrapper,						0x004A0740, 3); // Not tested yet
	//	jump_patch(remove_unit_from_lists_wrapper,			0x0049E3F0, 0); // Not tested yet
		jump_patch(building_death_wrapper,					0x0049FD00, 5);

		nops(0x004A0766, 5); // Disable kill all loaded units so we can call it before UnitExt is cleared
		jump_patch(clear_bullet_targets_patch_8AACC,		0x0048AACC, 2); // DF: Fix bug where dying Lakizilisks sometimes deal double damage
	}
}