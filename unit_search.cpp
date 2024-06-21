#include "unit_search.h"
#include <BW\api.h>

#define CELL_SIZE 8
#define CELL_PX_SIZE (CELL_SIZE * 32)
#define CELL_COUNT (256 / CELL_SIZE)

struct SearchEntry {
	SearchEntry* next;
	Point16 position;
	void* data;
};

struct SearchIndex {
	u32 size;
	SearchEntry entries[EntryLimit];
	SearchEntry* grid[CELL_COUNT][CELL_COUNT];
};

static SearchIndex unit_index;
static SearchIndex seen_unit_index[PlayerLimit];
static SearchIndex bullet_index;
static SearchIndex corpse_index;
static SearchIndex effect_index;

void entry_array_add(EntryArray* array, void* data) {
	if (array->count < EntryLimit) {
		array->entries[array->count++] = data;
	}
}

void entry_array_remove(EntryArray* array, u32 index) {
	if (index < array->count) {
		array->entries[index] = array->entries[--array->count];
	}
}

bool is_point_in_rect(Point16 point, Box32 rect)
{
	return point.x >= rect.left
		&& point.x <= rect.right
		&& point.y >= rect.top
		&& point.y <= rect.bottom;
}

bool is_rect_in_rect(Box32 inner, Box32 outer)
{
	return outer.left <= inner.left
		&& inner.right <= outer.right
		&& outer.top <= inner.top
		&& inner.bottom <= outer.bottom;
}

static void search_index_reset(SearchIndex* index) {
	memset(index, 0, sizeof(SearchIndex));
}

static void search_index_add(SearchIndex* index, Point16 position, void* data) {
	if (index->size < EntryLimit) {
		SearchEntry* entry = &index->entries[index->size++];
		entry->position = position;
		entry->data = data;

		u32 x = position.x / CELL_PX_SIZE;
		u32 y = position.y / CELL_PX_SIZE;
		entry->next = index->grid[x][y];
		index->grid[x][y] = entry;
	}
}

static void index_search_rect(EntryArray* entry_array, SearchIndex* index, Box32 rect) {
	rect = bw::clamp_box32(rect);
	const u32 x1 = rect.left / CELL_PX_SIZE;
	const u32 y1 = rect.top / CELL_PX_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(rect.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++) {
		for (u32 y = y1; y < y2; y++) {
			Box32 tile_rect = {
				x * CELL_PX_SIZE,
				y * CELL_PX_SIZE,
				x * CELL_PX_SIZE + CELL_PX_SIZE,
				y * CELL_PX_SIZE + CELL_PX_SIZE
			};
			bool is_fully_covered = is_rect_in_rect(tile_rect, rect);

			for (SearchEntry* entry = index->grid[x][y]; entry; entry = entry->next) {
				if (is_fully_covered || is_point_in_rect(entry->position, rect) && entry_array->count < EntryLimit) {
					entry_array_add(entry_array, entry->data);
				}
			}
		}
	}
}

static void index_search_square(EntryArray* entry_array, SearchIndex* index, Point16 center, u32 radius) {
	index_search_rect(entry_array, index, {
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	});
}

static void index_search_radial(EntryArray* entry_array, SearchIndex* index, Point16 center, u32 radius) {
	Box32 dims = {
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	};
	dims = bw::clamp_box32(dims);

	const u32 x1 = dims.left / CELL_PX_SIZE;
	const u32 y1 = dims.top / CELL_PX_SIZE;
	const u32 x2 = ceil(dims.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(dims.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++) {
		for (u32 y = y1; y < y2; y++) {
			Point16 tile_center = {
				x * CELL_PX_SIZE + CELL_PX_SIZE / 2,
				y * CELL_PX_SIZE + CELL_PX_SIZE / 2
			};
			bool is_fully_covered = bw::distance_between(center, tile_center) + CELL_PX_SIZE * 1.414 <= radius;

			for (SearchEntry* entry = index->grid[x][y]; entry; entry = entry->next) {
				if (is_fully_covered || bw::distance_between(center, entry->position) <= radius) {
					entry_array_add(entry_array, entry->data);
				}
			}
		}
	}
}

static void filter_unit_array(EntryArray* entry_array) {
	for (u32 i = 0; i < entry_array->count; ++i) {
		Unit* unit = entry_array->units[i];
		if (unit->is_dead()) {
			entry_array_remove(entry_array, i--);
		}
	}
}

void unit_search_rect(EntryArray* entry_array, Box32 rect) {
	index_search_rect(entry_array, &unit_index, rect);
	filter_unit_array(entry_array);
}

void unit_search_square(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_square(entry_array, &unit_index, center, radius);
	filter_unit_array(entry_array);
}

void unit_search_radial(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_radial(entry_array, &unit_index, center, radius);
	filter_unit_array(entry_array);
}

void seen_unit_search_rect(EntryArray* entry_array, u32 player, Box32 rect) {
	index_search_rect(entry_array, &seen_unit_index[player], rect);
	filter_unit_array(entry_array);
}

void seen_unit_search_square(EntryArray* entry_array, u32 player, Point16 center, u32 radius) {
	index_search_square(entry_array, &seen_unit_index[player], center, radius);
	filter_unit_array(entry_array);
}

void seen_unit_search_radial(EntryArray* entry_array, u32 player, Point16 center, u32 radius) {
	index_search_radial(entry_array, &seen_unit_index[player], center, radius);
	filter_unit_array(entry_array);
}

static void filter_bullet_array(EntryArray* entry_array) {
	for (u32 i = 0; i < entry_array->count; ++i) {
		Bullet* bullet = entry_array->bullets[i];
		if (!bullet->hitpoints) { // TODO: vet this check is correct
			entry_array_remove(entry_array, i--);
		}
	}
}

void bullet_search_rect(EntryArray* entry_array, Box32 rect) {
	index_search_rect(entry_array, &bullet_index, rect);
	filter_bullet_array(entry_array);
}

void bullet_search_square(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_square(entry_array, &bullet_index, center, radius);
	filter_bullet_array(entry_array);
}

void bullet_search_radial(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_radial(entry_array, &bullet_index, center, radius);
	filter_bullet_array(entry_array);
}

static void filter_corpse_array(EntryArray* entry_array) {
	for (u32 i = 0; i < entry_array->count; ++i) {
		Thingy* corpse = entry_array->corpses[i];
		if (corpse->sprite->flags & SpriteFlags::RemoveAfterDeath) { // corpse->sprite and corpse->sprite->id typically doesn't change during a tick
			entry_array_remove(entry_array, i--);
		}
	}
}

void corpse_search_rect(EntryArray* entry_array, Box32 rect) {
	index_search_rect(entry_array, &corpse_index, rect);
	filter_corpse_array(entry_array);
}

void corpse_search_square(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_square(entry_array, &corpse_index, center, radius);
	filter_corpse_array(entry_array);
}

void corpse_search_radial(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_radial(entry_array, &corpse_index, center, radius);
	filter_corpse_array(entry_array);
}

void effect_search_rect(EntryArray* entry_array, Box32 rect) {
	index_search_rect(entry_array, &effect_index, rect);
}

void effect_search_square(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_square(entry_array, &effect_index, center, radius);
}

void effect_search_radial(EntryArray* entry_array, Point16 center, u32 radius) {
	index_search_radial(entry_array, &effect_index, center, radius);
}

void unit_search_filter(EntryArray* unit_array, Unit* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids) {
	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		if (!unit_flag_match(unit, reference, flags, allowed_unit_ids)) {
			entry_array_remove(unit_array, i--);
			continue;
		}
	}
}

void corpse_search_filter(EntryArray* corpse_array, Thingy* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids) {
	for (u32 i = 0; i < corpse_array->count; i++) {
		Thingy* corpse = corpse_array->corpses[i];
		if (!corpse_flag_match(corpse, reference, flags, allowed_unit_ids)) {
			entry_array_remove(corpse_array, i--);
			continue;
		}
	}
}

void unit_search_custom_filter(EntryArray* unit_array, SearchFilterFunc filter) {
	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units.data[i];
		if (!filter(unit)) {
			entry_array_remove(unit_array, i--);
			continue;
		}
	}
}

void unit_search_sort(EntryArray* unit_array, SearchScoreFunc score_func) {
	std::ranges::stable_sort(unit_array->units, [&](Unit* a, Unit* b) {
		return score_func(a) > score_func(b);	
	});
}

static Point16 temp_center;
static s32 unit_distance_compare_func(void const* a, void const* b) {
	return bw::distance_between(temp_center, ((Unit*)a)->position) > bw::distance_between(temp_center, ((Unit*)b)->position);	
}

static s32 corpse_distance_compare_func(void const* a, void const* b) {
	if (!((Thingy*)a)->sprite || !((Thingy*)b)->sprite) return 0;
	return bw::distance_between(temp_center, ((Thingy*)a)->sprite->position) > bw::distance_between(temp_center, ((Thingy*)b)->sprite->position);	
}

void unit_search_sort_by_distance(EntryArray* unit_array, Point16 center) {
	temp_center = center;
	qsort(unit_array->units.data, unit_array->count, sizeof(Unit*), unit_distance_compare_func);
}

void corpse_search_sort_by_distance(EntryArray* corpse_array, Point16 center) {
	temp_center = center;
	qsort(corpse_array->corpses.data, corpse_array->count, sizeof(Thingy*), corpse_distance_compare_func);
}

Unit* unit_search_find_best(EntryArray* unit_array, SearchScoreFunc score_func) {
	s32 best_score = ScoreInvalidUnit;
	Unit* best_unit = 0;

	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		s32 score = score_func(unit);
		if (score > best_score) {
			best_unit = unit;
			best_score = score;
		}
	}

	return best_unit;
}

Unit* unit_search_get_first(EntryArray* unit_array, SearchFilterFunc match_func) {
	if (!match_func) {
		if (unit_array->count) {
			return unit_array->units[0];
		}
		return 0;
	}

	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		if (match_func(unit)) {
			return unit;
		}
	}
	return 0;
}

void reset_search_index() {
	search_index_reset(&unit_index);
	for (u32 i = 0; i < 8; ++i) {
		search_index_reset(&seen_unit_index[i]);
	}
	search_index_reset(&bullet_index);
	search_index_reset(&corpse_index);
	search_index_reset(&effect_index);
}

void update_search_index() {
	// Add units
	for (Unit* unit = g_first_visible_unit; unit; unit = unit->link.next) {
		search_index_add(&unit_index, unit->position, unit);

		UnitExt* unit_ext = get_unit_ext(unit);
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (unit_ext->seen_by_flags & (1 << i)) {
				search_index_add(&seen_unit_index[i], unit_ext->last_seen_position[i], unit);
			}
		}
	}
	
	// Add bullets
	for (Bullet* bullet = *g_first_active_bullet; bullet; bullet = bullet->next) {
		search_index_add(&bullet_index, bullet->position, bullet);
	}

	// Add corpses
	for (Thingy* thingy = *g_first_thingy; thingy; thingy = thingy->next) {
		if (thingy->sprite && !(thingy->sprite->flags & SpriteFlags::RemoveAfterDeath) && bw::sprite_id_is_corpse(thingy->sprite->id)) {
			search_index_add(&corpse_index, thingy->sprite->position, thingy);
		}
	}
}

void update_search_index_effects() {
	// Add effects
	for (EffectField& effect : g_iquare_globals->m_effects) {
		Point16 position = {};
		position.x = effect.position.x;
		position.y = effect.position.y;
		search_index_add(&effect_index, position, &effect);
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
			if (!bw::unit_is_facing_position(center, target->position)) {
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

void unit_search_add_units_from_connected_transport(EntryArray* unit_array, Unit* unit) {
	if (unit->status & UnitStatus::InTransport && unit->connected_unit) {
		Unit* transport = unit->connected_unit;
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* cargo_unit = transport->get_loaded_unit(i);
			if (cargo_unit) {
				entry_array_add(unit_array, cargo_unit);
			}
		}
	}
}