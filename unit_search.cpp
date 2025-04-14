#include "unit_search.h"
#include "BW/api.h"

bool is_point_in_rect(Point16 point, Box32 rect) {
	return point.x >= rect.left
		&& point.x <= rect.right
		&& point.y >= rect.top
		&& point.y <= rect.bottom;
}

bool is_rect_in_rect(Box32 inner, Box32 outer) {
	return outer.left <= inner.left
		&& inner.right <= outer.right
		&& outer.top <= inner.top
		&& inner.bottom <= outer.bottom;
}

void refresh_search_index() {
	g_unit_search.reset();
	for (u32 player = 0; player < PlayerLimit; ++player) {
		g_seen_unit_search[player].reset();
	}
	g_bullet_search.reset();
	g_corpse_search.reset();
	g_effect_search.reset();
	g_search_arena.reset();

	// Add units
	for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
		g_unit_search.add(unit, unit->position);

		UnitExt* unit_ext = get_unit_ext(unit);
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (unit_ext->seen_by_flags & (1 << player)) {
				g_seen_unit_search[player].add(unit, unit_ext->last_seen_position[player]);
			}
		}
	}
	
	// Add bullets
	for (Bullet* bullet = *g_first_active_bullet; bullet; bullet = bullet->next) {
		g_bullet_search.add(bullet, bullet->position);
	}

	// Add corpses
	for (Thingy* thingy = *g_first_thingy; thingy; thingy = thingy->next) {
		if (thingy->sprite && !(thingy->sprite->flags & SpriteFlags::RemoveAfterDeath) && SpriteId::is_corpse((SpriteId::Enum)thingy->sprite->id)) {
			g_corpse_search.add(thingy, thingy->sprite->position);
		}
	}
}

void update_search_index_effects() {
	// Add effects
	for (EffectField& effect : g_iquare_globals->m_effects) {
		g_effect_search.add(&effect, effect.position);
	}
}

bool unit_flag_match(Unit* target, Unit* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids) {
	if (!(flags & FilterIncludingSelf) && target == center) {
		return false;
	}
	if (flags & FilterNotSelf && target == center) {
		return false;
	}
	if (!(flags & FilterInvincible) && target->status & UnitStatus::Invincible) {
		return false;
	}
	if (flags & FilterNoAir && target->status & UnitStatus::InAir) {
		return false;
	}
	if (flags & FilterNoGround && !(target->status & UnitStatus::InAir)) {
		return false;
	}
	if (flags & FilterSpellcaster) {
		if (!(units_dat::base_property[target->id] & UnitProperty::Spellcaster)) {
			return false;
		}
	}
	if (flags & FilterHasEnergy) {
		if (target->energy < 256) {
			return false;
		}
	}
	if (flags & FilterShielded) {
		if (!units_dat::shields_enabled[target->id]) {
			return false;
		}
		if (target->shields < 256) {
			return false;
		}
	}
	if (flags & FilterPowered) {
		if (target->status & UnitStatus::Frozen) {
			return false;
		}
	}
	if (flags & FilterCompleted) {
		if (!(target->status & UnitStatus::Completed)) {
			return false;
		}
	}
	if (flags & FilterNoTrap) {
		if (unit_id_is_trap(target->id)) {
			return false;
		}
	}
	if (flags & FilterNoHallucination) {
		if (target->status & UnitStatus::IsHallucination) {
			return false;
		}
	}
	if (center) {
		if (flags & FilterFrontalArc) {
			if (!bw::unit_is_facing(center, target->position)) {
				return false;
			}
		}
		if (flags & FilterOwn && target->player != center->player) {
			return false;
		}
		if (bw::is_allied_to(target->player, center->player) && flags & FilterEnemy) {
			return false;
		}
		if (!bw::is_allied_to(target->player, center->player) && flags & FilterAlly) {
			return false;
		}
		if (flags & FilterVisible && !target->is_visible_to(center->player)) {
			return false;
		}
	}
	if (allowed_unit_ids.size() && allowed_unit_ids.find(target->id) == allowed_unit_ids.end()) {
		return false;
	}
	if (units_dat::base_property[target->id] & UnitProperty::Organic && flags & FilterNoOrganic) {
		return false;
	}
	if (!(units_dat::base_property[target->id] & UnitProperty::Organic) && flags & FilterOnlyOrganic) {
		return false;
	}
	if (units_dat::base_property[target->id] & UnitProperty::Mechanical && flags & FilterNoMechanical) {
		return false;
	}
	if (!(units_dat::base_property[target->id] & UnitProperty::Mechanical) && flags & FilterOnlyMechanic) {
		return false;
	}
	if (units_dat::base_property[target->id] & UnitProperty::Building && flags & FilterNoBuilding) {
		return false;
	}
	if (!(units_dat::base_property[target->id] & UnitProperty::Building) && flags & FilterBuilding) {
		return false;
	}
	if (!(bw::is_enabled(target)) && flags & FilterEnabled) {
		return false;
	}
	if (flags & FilterCanAttackOrCast && !target->has_weapon() && !(units_dat::base_property[target->id] & UnitProperty::Spellcaster)) {
		return false;
	}
	if (flags & FilterNotWorker && units_dat::base_property[target->id] & UnitProperty::Worker) {
		return false;
	}
	if (flags & FilterNotSubunit && units_dat::base_property[target->id] & UnitProperty::Subunit) {
		return false;
	}
	if (flags & FilterNoEgg && (target->id == UnitId::ZergEgg || target->id == UnitId::ZergCocoon)) {
		return false;
	}
	if (flags & FilterNoLarva && (target->id == UnitId::ZergLarva || target->id == UnitId::ZergOcsalLarva)) {
		return false;
	}
	if (flags & FilterNoHangar && (target->id == UnitId::ProtossPlasmaShell || target->id == UnitId::ProtossSteward)) {
		return false;
	}
	if (flags & FilterTownCenter && !unit_id_is_town_center(target->id)) {
		return false;
	}
	if (flags & FilterLifted && target->is_building() && target->is_in_air()) {
		return false;
	}
	if (flags & FilterComputer && !target->is_computer_controlled()) {
		return false;
	}
	return true;
}

bool corpse_flag_match(Thingy* target, Thingy* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids) {
	if (!(flags & CorpseFilterIncludingSelf) && target == center) {
		return false;
	}
	if (flags & CorpseFilterNotSelf && target == center) {
		return false;
	}
	ThingyExt* target_ext = get_thingy_ext(target);
	u32 target_unit_id = target_ext->unit_id;
	u32 target_properties = units_dat::base_property[target_unit_id];
	if (flags & CorpseFilterNoAir && target_properties & UnitProperty::Flyer) {
		return false;
	}
	if (flags & CorpseFilterNoGround && !(target_properties & UnitProperty::Flyer)) {
		return false;
	}
	if (flags & CorpseFilterSpellcaster) {
		if (!(target_properties & UnitProperty::Spellcaster)) {
			return false;
		}
	}
	if (flags & CorpseFilterShielded) {
		if (!units_dat::shields_enabled[target_unit_id]) {
			return false;
		}
	}
	if (flags & CorpseFilterNoTrap) {
		if (unit_id_is_trap(target_unit_id)) {
			return false;
		}
	}
	if (center) {
		u32 target_player = target_ext->player;
		ThingyExt* center_ext = get_thingy_ext(center);
		u32 center_player = center_ext->player;
		if (flags & CorpseFilterOwn && target_player != center_player) {
			return false;
		}
		if (bw::is_allied_to(target_player, center_player) && flags & CorpseFilterEnemy) {
			return false;
		}
		if (!bw::is_allied_to(target_player, center_player) && flags & CorpseFilterAlly) {
			return false;
		}
		if (flags & CorpseFilterVisible && !target->sprite->is_visible_to(center_player)) { // Dunno if this is even legal for Thingy sprites... -Pr0nogo
			return false;
		}
	}
	if (allowed_unit_ids.size() && allowed_unit_ids.find(target_unit_id) == allowed_unit_ids.end()) {
		return false;
	}
	if (units_dat::base_property[target_unit_id] & UnitProperty::Building && flags & CorpseFilterNoBuilding) {
		return false;
	}
	if (!(units_dat::base_property[target_unit_id] & UnitProperty::Building) && flags & CorpseFilterBuilding) {
		return false;
	}
	if (flags & CorpseFilterNotWorker && units_dat::base_property[target_unit_id] & UnitProperty::Worker) {
		return false;
	}
	if (flags & CorpseFilterNotSubunit && units_dat::base_property[target_unit_id] & UnitProperty::Subunit) {
		return false;
	}
	if (flags & CorpseFilterNoEgg && (target_unit_id == UnitId::ZergEgg || target_unit_id == UnitId::ZergCocoon)) {
		return false;
	}
	if (flags & CorpseFilterNoLarva && (target_unit_id == UnitId::ZergLarva || target_unit_id == UnitId::ZergOcsalLarva)) {
		return false;
	}
	if (flags & CorpseFilterNoHangar && (target_unit_id == UnitId::ProtossPlasmaShell || target_unit_id == UnitId::ProtossSteward)) {
		return false;
	}
	return true;
}

u16 bw_random() {
	return bw::random();
}
