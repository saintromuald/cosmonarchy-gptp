#include "cmbw_behaviors.h"
#include "SCBW\api.h"

bool is_any_corpse(Thingy* thingy) {
	if (thingy->sprite && scbw::sprite_id_is_corpse(thingy->sprite->id) && !(thingy->sprite->flags & SpriteFlags::RemoveAfterDeath)) {
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
	EntryArray corpses = {};
	corpse_search_radial(&corpses, position, radius);
	corpse_search_sort_by_distance(&corpses, position);
	for (u32 i = 0; i < corpses.count; ++i) {
		Thingy* corpse = corpses.corpses[i];
		if (is_active_corpse(corpse)) {
			return corpse;
		}
	}
	return 0;
}

Unit* reclaim_corpse(Thingy* corpse, u32 player) {
	if (!is_active_corpse(corpse)) {
		return 0;
	}

	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 unit_id = corpse_ext->unit_id;
	Point16 pos = corpse->sprite->position;
	if (units_dat::base_property[unit_id] & UnitProperty::Flyer) {
		if (pos.y > 42) {
			pos.y -= 42;
		} else {
			pos.y = 0;
		}
	}
	Unit* unit = scbw::create_unit_in_game(unit_id, player, pos.x, pos.y);
	if (unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		unit->status |= UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
		unit->play_iscript_animation(IscriptAnimation::Unused1);
		unit_ext->values[ValueReviveStarted] = 1;
		unit_ext->timers[TimerReviveFlagClearMin] = 14; // 3;
		unit_ext->timers[TimerReviveFlagClearMax] = 14;
		switch (unit->id) {
			case UnitId::ZergRililisk:
			case UnitId::ZergRilirokor:
			{
				unit->remove_timer = BroodspawnTimer;
			}
		}
		consume_corpse(corpse);

		unit->shields = 0;
		unit->set_hitpoints(25 * 256);
		unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
		helper::play_sound_from_unit(SoundId::LazarusAgentProc, unit);
		unit_ext->timers[TimerReclaiming] = 2 * 24;
		unit_ext->timers[TimerStun]       = 2 * 24;
		unit_ext->timers[TimerDecaySafe]  = 3;
		unit_ext->values[ValueDecaying] = true;
		if (scbw::get_game_kind() == GameKind_TheTurning && player == TheTurningZombiePlayerId) {
			unit_ext->values[ValueDecaying] = false;
			unit_ext->values[ValueNoCorpse] = true;
		}
	}
	return unit;
}

void avenge_corpse(Thingy* corpse, u32 player) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 corpse_player = corpse_ext->player;
	if (!is_active_corpse(corpse) || !scbw::are_mutual_allies(player, corpse_player)) {
		return;
	}

	Point16 pos = corpse->sprite->position;
	scbw::create_and_init_thingy(SpriteId::CuirassStonewallProtocolOverlay, pos.x, pos.y, corpse->sprite->player, corpse->sprite->elevation);
	helper::play_sound_at_pos(SoundId::ZealotRevive01, pos);
		
	EntryArray units = {};
	unit_search_radial(&units, corpse->sprite->position, corpse_ext->sight_range_before_death);
	for (u32 i = 0; i < units.count; ++i) {
		Unit* unit = units.units[i];
		if (!scbw::is_allied_to(player, unit->player)) {
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
	if (!is_active_corpse(corpse) || scbw::are_mutual_allies(player, corpse_player)) {
		return;
	}

	Point16 pos = corpse->sprite->position;
	scbw::create_and_init_thingy(SpriteId::CuirassStonewallProtocolOverlay, pos.x, pos.y, corpse->sprite->player, corpse->sprite->elevation);
	ascend_in_area(player, pos, corpse_ext->sight_range_before_death);
	consume_corpse(corpse);
}

void ascend_in_area(u32 player, Point16 position, u32 range) {
	helper::play_sound_at_pos(SoundId::Protoss_ZEALOT_PZeRag00_wav, position);

	EntryArray units = {};
	unit_search_radial(&units, position, range);
	for (u32 i = 0; i < units.count; ++i) {
		Unit* unit = units.units[i];
		if (!scbw::is_allied_to(player, unit->player)) {
			continue;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->timers[TimerAscend] = 3 * 24;
		unit->sprite->create_underlay(ImageId::SmallExplosion_Unused);
	}
}

void avenge_or_ascend_corpse(Thingy* corpse, u32 player) {
	ThingyExt* corpse_ext = get_thingy_ext(corpse);
	u32 corpse_player = corpse_ext->player;
	if (scbw::are_mutual_allies(player, corpse_player)) {
		avenge_corpse(corpse, player);
	} else {
		ascend_corpse(corpse, player);
	}
}

void avenge_corpses_in_area(Point16 position, u32 radius, u32 player) {
	EntryArray corpses = {};
	corpse_search_radial(&corpses, position, radius);
	for (u32 i = 0; i < corpses.count; ++i) {
		Thingy* corpse = corpses.corpses[i];
		avenge_corpse(corpse, player);
	}
}

void ascend_corpses_in_area(Point16 position, u32 radius, u32 player) {
	EntryArray corpses = {};
	corpse_search_radial(&corpses, position, radius);
	for (u32 i = 0; i < corpses.count; ++i) {
		Thingy* corpse = corpses.corpses[i];
		ascend_corpse(corpse, player);
	}
}

void avenge_or_ascend_corpses_in_area(Point16 position, u32 radius, u32 player) {
	EntryArray corpses = {};
	corpse_search_radial(&corpses, position, radius);
	for (u32 i = 0; i < corpses.count; ++i) {
		Thingy* corpse = corpses.corpses[i];
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
	} else if (unit_ext->timers[TimerPatriarchAstralRule]) {
		ascend_player = unit_ext->values[ValuePatriarchAstralRulePlayer];
	} else if (unit_ext->timers[TimerPariahHit]) {
		ascend_player = unit_ext->values[ValuePariahHitPlayer];
	}

	if (ascend_player >= 0 && !scbw::are_mutual_allies(unit->player, ascend_player)) {
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
	scbw::add_unit_overlay(target, ImageId::ParasiteOverlay);
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