#include "cm_behaviors.h"
#include "BW\api.h"

bool is_any_corpse(Thingy* thingy) {
	if (thingy->sprite && SpriteId::is_corpse((SpriteId::Enum)thingy->sprite->id) && !(thingy->sprite->flags & SpriteFlags::RemoveAfterDeath)) {
		ThingyExt* corpse_ext = get_thingy_ext(thingy);
		u32 unit_id = corpse_ext->unit_id;
		if (units_dat::base_property[unit_id] & UnitProperty::Building
		||	unit_id == UnitId::ZergLarva
		||  unit_id == UnitId::ZergOcsalLarva
		||  unit_id == UnitId::ZergEgg
		||  unit_id == UnitId::ZergCocoon
		||  unit_id == UnitId::ProtossPlasmaShell
		||  unit_id == UnitId::ProtossSteward) {
			return false;
		}
		return true;
	}
	return false;
}

static bool internal_is_corpse_active(Thingy* corpse) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	return corpse_ext->activity_timer <= 10 * 24;
}

bool is_inactive_corpse(Thingy* thingy) {
	if (!is_any_corpse(thingy)) {
		return false;
	}
	return !internal_is_corpse_active(thingy);
}

bool is_active_corpse(Thingy* thingy) {
	if (!is_any_corpse(thingy)) {
		return false;
	}
	return internal_is_corpse_active(thingy);
}

void consume_corpse(Thingy* corpse) {
	corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
	corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
}

Thingy* get_closest_active_corpse(Point16 position, u32 radius) {
	return g_corpse_search
		.radial(position, radius)
		.sort_by_distance(position)
		.first([](Thingy* corpse) { return is_active_corpse(corpse); });
}

Unit* reclaim_corpse(Thingy* corpse, u32 player) {
	if (!is_active_corpse(corpse)) {
		return 0;
	}

	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 original_player = corpse_ext->player;
	u32 unit_id = corpse_ext->unit_id;
	Point16 pos = corpse->sprite->position;
	if (units_dat::base_property[unit_id] & UnitProperty::Flyer) {
		if (pos.y > 42) {
			pos.y -= 42;
		} else {
			pos.y = 0;
		}
	}
	Unit* unit = bw::create_unit_in_game(unit_id, player, pos.x, pos.y);
	if (unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		unit->status |= UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
		unit->play_iscript_animation(IscriptAnimation::Unused1);
		unit_ext->values[ValueReviveStarted] = 1;
		unit_ext->timers[TimerReviveFlagClearMin] = 14; // 3;
		unit_ext->timers[TimerReviveFlagClearMax] = 14;
		unit_ext->values[ValueRegressionUnstable] = corpse_ext->create_origin == UnitId::ZergOcsalLarva;
		switch (unit->id) {
			case UnitId::ZergRililisk:
			case UnitId::ZergRilirokor: {
				unit->remove_timer = BroodspawnTimer;
			} break;
		}
		consume_corpse(corpse);

		unit->shields = 0;
		unit->set_hitpoints(5 * 256);
		u32 image_id = ImageId::LazarusAgentProc_Small + bw::get_overlay_offset(unit->id, true);
		unit->sprite->create_top_overlay(image_id);
		helper::play_sound_from_unit(SoundId::LazarusAgentProc, unit);
		unit_ext->timers[TimerReclaiming] = Reclamation::ReviveTimer;
		unit_ext->timers[TimerStun]       = Reclamation::ReviveTimer;
		unit_ext->values[ValueReclaimed]  = 1;
		if (bw::get_game_kind() == GameKind_TheTurning && player == NeutralHostilePlayerId) {
			unit_ext->values[ValueNoCorpse] = true;
		}
	}
	return unit;
}

void avenge_corpse(Thingy* corpse, u32 player) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 corpse_player = corpse_ext->player;
	if (!is_active_corpse(corpse) || !bw::are_mutual_allies(player, corpse_player)) {
		return;
	}

	Point16 pos = corpse->sprite->position;
	bw::create_and_init_thingy(SpriteId::CuirassStonewallProtocolOverlay, pos.x, pos.y, corpse->sprite->player, corpse->sprite->elevation);
	helper::play_sound_at_pos(SoundId::ZealotRevive01, pos);
		
	for (Unit* unit : g_unit_search.radial(corpse->sprite->position, corpse_ext->sight_range_before_death)) {
		if (!bw::is_allied_to(player, unit->player) || unit->player == 11) {
			continue;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->timers[TimerAvenge] = 3 * 24 + 1; // + 1 for update_speed call in game_hooks
		unit->sprite->create_underlay(ImageId::SmallExplosion_Unused);
		unit->update_speed();
	}

	consume_corpse(corpse);
}

void ascend_corpse(Thingy* corpse, u32 player) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 corpse_player = corpse_ext->player;
	if (!is_active_corpse(corpse) || bw::are_mutual_allies(player, corpse_player)) {
		return;
	}

	Point16 pos = corpse->sprite->position;
	bw::create_and_init_thingy(SpriteId::CuirassStonewallProtocolOverlay, pos.x, pos.y, corpse->sprite->player, corpse->sprite->elevation);
	ascend_in_area(player, pos, corpse_ext->sight_range_before_death);
	consume_corpse(corpse);
}

void ascend_in_area(u32 player, Point16 position, u32 range) {
	helper::play_sound_at_pos(SoundId::Protoss_ZEALOT_PZeRag00_wav, position);

	for (Unit* unit : g_unit_search.radial(position, range)) {
		if (!bw::is_allied_to(player, unit->player) || unit->player == 11) {
			continue;
		}
		if (!unit_is_combatant(unit)) {
			continue;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->timers[TimerAscend] = 3 * 24;
	}
}

void avenge_or_ascend_corpse(Thingy* corpse, u32 player) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 corpse_player = corpse_ext->player;
	if (bw::are_mutual_allies(player, corpse_player)) {
		avenge_corpse(corpse, player);
	} else {
		ascend_corpse(corpse, player);
	}
}

void avenge_corpses_in_area(Point16 position, u32 radius, u32 player) {
	for (Thingy* corpse : g_corpse_search.radial(position, radius)) {
		avenge_corpse(corpse, player);
	}
}

void ascend_corpses_in_area(Point16 position, u32 radius, u32 player) {
	for (Thingy* corpse : g_corpse_search.radial(position, radius)) {
		ascend_corpse(corpse, player);
	}
}

void avenge_or_ascend_corpses_in_area(Point16 position, u32 radius, u32 player) {
	for (Thingy* corpse : g_corpse_search.radial(position, radius)) {
		avenge_or_ascend_corpse(corpse, player);
	}
}

u32 unit_avenges_or_ascends(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);

	// Avenge
	bool avenges = false;
	switch (unit->id) {
		case UnitId::ProtossEcclesiast:
		case UnitId::ProtossVassal:
		case UnitId::ProtossServitor:
		case UnitId::ProtossArchon: {
			avenges = true;
		} break;
		default: {
			if (unit_ext->values[ValueIsZombified]) {
				avenges = true;
			} else if (unit_ext->timers[TimerHeraldAvenge]) {
				avenges = true;
			}
		} break;
	}
	if (avenges) {
		return unit->player + 1;
	}

	// Ascend
	s32 ascend_player = -1;
	if (unit_ext->values[ValueSignifyStacks] >= 5) {
		ascend_player = unit_ext->values[ValueSignifyPlayer];
	} else if (unit_ext->timers[TimerPariahHit]) {
		ascend_player = unit_ext->values[ValuePariahHitPlayer];
	}

	if (ascend_player >= 0 && !bw::are_mutual_allies(unit->player, ascend_player)) {
		return ascend_player + 1;
	}
	return 0;
}

void incubate(Unit* target, u32 count, u32 unit_id, u32 player) {
	UnitExt* target_ext = get_unit_ext(target);
	if (target_ext->timers[TimerIncubationArm]
		||  units_dat::base_property[target->id] & UnitProperty::Invincible
		||  unit_id_is_broodspawn(target->id)) {
		return;
	}
	target_ext->values[ValueIncubationCount] = count; 
	target_ext->values[ValueIncubationUnitId] = unit_id; 
	target_ext->values[ValueIncubationPlayer] = player;
	target_ext->timers[TimerIncubationArm] = 2 * 24 + 1;
	bw::add_unit_overlay(target, ImageId::ParasiteOverlay);
}

void set_custom_egg(Unit* egg, u32 unit_id, u32 time, u32 hp) {
	egg->order_to_idle();
	egg->sprite->play_iscript_animation(IscriptAnimation::Walking, true);
	egg->build_queue[egg->build_queue_slot] = unit_id;
	egg->set_hitpoints(hp);

	UnitExt* egg_ext = get_unit_ext(egg);
	egg_ext->values[ValueCustomEggActive]   = true;
	egg_ext->timers[TimerCustomEggHatch]    = time;
	egg_ext->values[ValueCustomEggHatchMax] = time;
	egg_ext->values[ValueCustomEggHpMax]    = hp;
}

void fire_secondary_melee_weapon_if_needed(Unit* unit, u32 weapon_id) {
	Unit* target = unit->order_target.unit;
	if (unit->order_signal & 0x20 && unit->ground_weapon_cooldown <= 1) {
		unit->order_signal &= ~0x20;
		if (target && !(units_dat::base_property[target->id] & UnitProperty::Flyer) && unit->get_distance_to_target(target) <= 30) {
			unit->play_iscript_animation(IscriptAnimation::SpecialState1);
		}
	}
}

bool can_infest(u32 player, Unit* target, bool force) {
	// Spawning cagra wouldn't work properly on units. Spawning cagra on units or addons could corrupt/crash the game. Addons don't transfer properly anyways
	UnitExt* target_ext = get_unit_ext(target);
	return target->status & UnitStatus::Completed && !(units_dat::base_property[target->id] & UnitProperty::Addon)
		&& (force || is_hostile_or_neutral(player, target->player));
}

void mark_infesting(u32 player, Unit* target) {
	if (!can_infest(player, target)) {
		return;
	}
	UnitExt* target_ext = get_unit_ext(target);
	target_ext->timers[TimerInfesting] = 24;
	target_ext->values[ValueInfestingPlayer] = player;
}

static u32 blissful_embrace_overlay_image_id(Unit* target) {
	if (units_dat::base_property[target->id] & UnitProperty::Building) {
		if (unit_id_is_smaller_building(target->id)) {
			return ImageId::InfestedOverlay_Small;
		} else {
			return ImageId::InfestedOverlay_Large;
		}
	} else {
		if (units_dat::base_property[target->id] & UnitProperty::LargeOverlay) {
			return ImageId::InfestedOverlay_Large;
		} else if (units_dat::base_property[target->id] & UnitProperty::MediumOverlay) {
			return ImageId::InfestedOverlay_Small;
		} else {
			return ImageId::OcsalRegressionOverlay;
		}
	}
}

void blissful_embrace_tick(Unit* target) {
	// Spawn infestation overlay
	u32 overlay_image_id = blissful_embrace_overlay_image_id(target);
	if (!target->get_overlay(overlay_image_id)) {
		target->sprite->create_overlay(overlay_image_id);
	}

	// Init
	UnitExt* target_ext = get_unit_ext(target);
	if (!target_ext->values[ValueInfestedInit]) {
		target_ext->values[ValueInfestedInit] = true;
		
		// Play sound
		helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZBldgDth_WAV, target, true, 0);

		// Spawn effect overlay
		u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
		if (units_dat::base_property[target->id] & UnitProperty::LargeOverlay) {
			effect_image_id = ImageId::ZergBuildingExplosion;
		} else if (units_dat::base_property[target->id] & UnitProperty::MediumOverlay) {
			effect_image_id = ImageId::ZergAirDeathExplosion_Large;
		}
		target->sprite->create_overlay(effect_image_id);

		// Spread initial creep
		if (unit_spreads_custom_creep(target)) {
			bw::spawn_custom_creep(target);
		}
	}
}

void blissful_embrace(Unit* target, u32 player, bool force) {
	// Sanity check.
	if (!can_infest(player, target, force)) {
		return;
	}

	// Eject loaded units if possible, kill them otherwise
	bw::unload_or_kill_loaded_units(target);
	UnitExt* target_ext = get_unit_ext(target);

	// Remove connected larva
	u16 larva_id = UnitId::ZergLarva;
	if (target->id == UnitId::ZergCaphrolosk || target->id == UnitId::ZergOcsalCesiant) {
		larva_id = UnitId::ZergOcsalLarva;
	}
	for (Unit* larva : g_unit_search.radial_ex(target, 4 * 32, 0, {larva_id})) {
		if (larva->connected_unit == target) {
			larva->connected_unit = 0;
		}
	}

	// Obtain control
	target->give_to(player);
	target_ext->values[ValueInfested] = true;

	Unit* matraleth = target_ext->units[UnitInfestingMatraleth];
	if (matraleth) {
		matraleth->order_to_idle();
	}

	// Init infestation
	blissful_embrace_tick(target);
}

void empress_zombify(u32 player, Unit* source, Unit* target, u32 stacks) {
	UnitExt* target_ext = get_unit_ext(target);
	target_ext->values[ValueMindControlOriginalPlayer] = 0;

	// Recurse
	for (u32 i = 0; i < TransportCap; ++i) {
		Unit* unit = target->get_loaded_unit(i);
		if (unit) {
			empress_zombify(player, source, unit, stacks);
		}
	}

	// Obtain control
	target->give_to(player);

	target->hitpoints = 0;
	target->shields = 0;
	target->heal_hp_then_shields(stacks * 256 * 5, source);
	target->remove_timer = 1 + stacks * 24 * 1;

	target->give_to(player);
	target_ext->values[ValueEmpressAnodyneAnnexStacks] = 0;
	target_ext->values[ValueIsZombified] = true;

	// Audiovisuals
	target->sprite->create_overlay(ImageId::MindControlHit_Small + bw::get_overlay_offset(target->id));
	u32 sound_id = bw::rand_between(SoundId::EmpressZombify01, SoundId::EmpressZombify03);
	helper::play_sound_from_unit(sound_id, target);
}

void mind_control(u32 player, u32 mind_controller_unit_id, Unit* target) {
	if (!target || player == target->player || !is_hostile_or_neutral(player, target->player)) {
		return;
	}

	UnitExt* target_ext = get_unit_ext(target);
	if (!target_ext->values[ValueMindControlOriginalPlayer]) {
		target_ext->values[ValueMindControlOriginalPlayer] = target->player + 1;
	}
	target_ext->values[ValueMindControllerUnitId] = mind_controller_unit_id;

	// Recurse
	for (u32 i = 0; i < TransportCap; ++i) {
		Unit* unit = target->get_loaded_unit(i);
		if (unit) {
			mind_control(player, mind_controller_unit_id, unit);
		}
	}

	// Cancel in-progress units
	if (units_dat::group_flags[target->id].is_factory) {
		u32 offset = 0;
		for (u32 i = 1; i < 5; ++i) {
			u32 slot = i + target->build_queue_slot - offset;
			u32 unit_id = target->build_queue[slot];
			if (unit_id == UnitId::None) {
				continue;
			}
			helper::refund_queue_slot(target, slot);
			offset++;
		}
		helper::cancel_last_training(target);
	}

	// Global recede timer for all control abilities
	target_ext->values[ValueMindControlPlayer] = player + 1;
	target_ext->timers[TimerMindControlRecede] = 5 * 24;

	// Obtain control
	target->give_to(player);

	// Audiovisuals
	if (mind_controller_unit_id == UnitId::ProtossDemiurge) {
		bw::remove_unit_overlay(target, ImageId::Malediction_Small + bw::get_overlay_offset(target->id));
		bw::add_unit_overlay(target, ImageId::DemiurgeWorshipProc_Small + bw::get_overlay_offset(target->id, true));
		helper::play_sound_from_unit(SoundId::DemiurgeWorship01, target, 1, 0);
		target_ext->values[ValueControlledByDemiurge] = 1;
	} else if (mind_controller_unit_id == UnitId::ProtossMindTyrant) {
		bw::add_unit_overlay(target, ImageId::MindControlHit_Small + bw::get_overlay_offset(target->id));
	}
}

void clear_tyranny(Unit* tyrant, Unit* victim) {
	UnitExt* tyrant_ext = get_unit_ext(tyrant);
	UnitExt* victim_ext = get_unit_ext(victim);
	victim_ext->units[UnitMindTyrantLink] = 0;
	tyrant_ext->units[UnitMindTyrantVictim] = 0;
}

void recede_mind_control(Unit* target) {
	if (!target) {
		return;
	}

	UnitExt* target_ext = get_unit_ext(target);
	if (target_ext->values[ValueMindControlOriginalPlayer]) {
		target->give_to(target_ext->values[ValueMindControlOriginalPlayer] - 1);
	}
	if (target_ext->values[ValueControlledByDemiurge]) {
		target_ext->values[ValueControlledByDemiurge] = 0;
		target_ext->timers[TimerDemiurgeWorshipOverlayDelay] = 0;
	}
	target_ext->values[ValueMindControlOriginalPlayer] = 0;
	target_ext->values[ValueMindControlPlayer] = 0;
	target_ext->values[ValueMindControllerUnitId] = UnitId::None; // Or should it be 0? -Pr0nogo
}

void spread_debuffs(Unit* from, Unit* to) {
	UnitExt* from_ext = get_unit_ext(from);
	UnitExt* to_ext = get_unit_ext(to);

	if (from_ext->values[ValueReclaimed]) {
		to_ext->values[ValueReclaimed] = true;
	}
	if (from_ext->values[ValueInfested]) {
		to_ext->values[ValueInfested] = true;
	}
	if (from_ext->values[ValueIsZombified]) {
		empress_zombify(from->player, from, to, 1);
		to->remove_timer = from->remove_timer;
	}
	if (from_ext->values[ValueMindControlPlayer]) {
		to->give_to(from_ext->values[ValueMindControlOriginalPlayer] - 1);
		mind_control(from_ext->values[ValueMindControlPlayer] - 1, from_ext->values[ValueMindControllerUnitId], to);
	}
}

AttackTargetingType attack_targeting_type_of(Unit* unit) {
    switch (unit->id) {
        case UnitId::TerranDiadem:
            return AttackTargetingType::Diadem;

        case UnitId::ProtossOptecton:
            return AttackTargetingType::FireGround;
    }
    return AttackTargetingType::RequiresTarget;
}

bool can_pick_new_target(Unit* unit) {
	switch (attack_targeting_type_of(unit)) {
		case AttackTargetingType::FireGround:
		case AttackTargetingType::RemembersTarget: {
            if (unit->movement_flags & MovementFlags_StartingAttack) {
                return false;
            }
		} break;
	}
	return true;
}