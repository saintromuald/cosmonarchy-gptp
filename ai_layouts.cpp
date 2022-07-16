#include "ai_layouts.h"
#include "hooks/unit_stats/new_requirements.h"
#include "data_ext.h"
#include "trigger_system/trigger_parser.h"

u32 chk_cache_size;
CHKUnit chk_cache[ChkCacheLimit];

bool show_player_layouts[PlayerLimit];
u32 default_ai_expansion_block[PlayerLimit];

AiLayouts player_layouts[PlayerLimit];
AiLayoutsTown layouts_towns[AiTownLimit];

void init_layouts() {
	memset(show_player_layouts, 0, sizeof(show_player_layouts));
	memset(default_ai_expansion_block, 0, sizeof(default_ai_expansion_block));
	memset(player_layouts, 0, sizeof(player_layouts));
	memset(layouts_towns, 0, sizeof(layouts_towns));

	for (u32 i = 0; i < chk_cache_size; ++i) {
		CHKUnit* unit = &chk_cache[i];
		if (unit->player > PlayerLimit && unit->player != 11) {
			continue;
		}
		if (!(units_dat::base_property[unit->unit_id] & UnitProperty::Building) && units_dat::computer_ai_internal[unit->unit_id] & 0x2) { // Don't become guard
			continue;
		}

		if (unit->player == 11) {
			u32 flags = unit->valid_state_flags & unit->state_flags;
			if (flags & ChkUnitFlags_Cloaked) {
				for (u32 i = 0; i < 8; ++i) {
					add_placeholder(unit, i);
				}
			}
		} else {
			add_placeholder(unit, unit->player);
		}
	}

	for (u32 i = 0; i < PlayerLimit; ++i) {
		AiLayouts* layouts = &player_layouts[i];
		sort_layouts(layouts);
	}
}

void update_layouts() {
	// Tick Layouts Town
	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &g_ai_town_array[i];
		AiLayoutsTown* layouts_town = &layouts_towns[i];
		if (!town->inited) {
			layouts_town->inited = false;
			layouts_town->wants_no_resarea = false;
			layouts_town->worker_limit = 0;
			layouts_town->worker_maynard = 0;
		}
		if (town->inited && !layouts_town->inited && (town->resarea || layouts_town->wants_no_resarea)) {
			layouts_town->inited = true;
			memset(layouts_town->request_count, 0, sizeof(layouts_town->request_count));

			AiLayouts* layouts = &player_layouts[town->player];
			if (!layouts_town->wants_no_resarea) {
				// Claim ownership of invincible placeholders
				bool claimed = false;
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (!placeholder->resarea) {
						placeholder->resarea = town->resarea;
						claimed = true;
					}
				}
				if (claimed) {
					sort_layouts(layouts);
				}
			}

			// Init placeholders
			for (u32 j = 0; j < layouts->placeholders_count; ++j) {
				AiPlaceholder* placeholder = &layouts->placeholders[j];
				if ((!placeholder->town && placeholder->resarea == town->resarea) || (placeholder->town && placeholder->town == town)) {
					placeholder->inited = false;
					placeholder->requested = false;
					placeholder->request_count = 0;
					placeholder->town = town;
					placeholder->placement_timeout = 0;
				}
			}

			if (show_player_layouts[town->player]) {
				scbw::printf("Player %u started Town #%u at resarea %u", town->player + 1, i, town->resarea);
			}
		}
		if (layouts_town->inited && layouts_town->wants_no_resarea) {
			town->worker_limit = layouts_town->worker_limit;
		}
	}

	// Set Layouts Colors
	for (u32 i = 0; i < PlayerLimit; ++i) {
		AiLayouts* layouts = &player_layouts[i];
		for (u32 j = 0; j < layouts->placeholders_count; ++j) {
			AiPlaceholder* placeholder = &layouts->placeholders[j];
			if (placeholder->requested) {
				placeholder->color = graphics::GREEN;
			} else if (placeholder->disabled) {
				placeholder->color = graphics::BLACK;
			} else if (!placeholder->should_request) {
				placeholder->color = graphics::BROWN;
			} else {
				placeholder->color = graphics::WHITE;
			}
		}
	}

	// Tick Layouts
	for (u32 i = 0; i < PlayerLimit; ++i) {
		AiLayouts* layouts = &player_layouts[i];
		for (u32 j = 0; j < layouts->placeholders_count; ++j) {
			AiPlaceholder* placeholder = &layouts->placeholders[j];
			AiTown* town = placeholder->town;
			if (town) {
				if (!town->inited || town->player != i) {
					placeholder->town = 0;
					continue;
				}
			} else {
				continue;
			}
			if (placeholder->disabled) {
				continue;
			}
			if (!placeholder->should_request) {
				if (!placeholder->inited) {
					init_placeholder(town, placeholder);
				}
				continue;
			}
			if (has_expanding_town(town->player)) {
				placeholder->color = graphics::PURPLE;
				continue;
			}
			AiLayoutsTown* layouts_town = get_layouts_town(town);
			u32 base_unit_id = find_unit_base_merge(placeholder->unit_id);
			if (!placeholder->requested && can_build_placeholder(layouts, town, placeholder)) {
				if (can_place_unit(base_unit_id, placeholder->position, i)) {
					placeholder->requested = true;
					if (show_player_layouts[town->player]) {
						scbw::printf("Player %u requested %s #%u at resarea %u", i + 1, Unit::get_name(placeholder->unit_id), j, placeholder->resarea);
					}
					if (!placeholder->inited) {
						init_placeholder(town, placeholder);
					}
					request_placeholder(town, layouts_town, placeholder);
				} else if (placeholder->placement_timeout < 10 * 24) { // Keep in mind that placement_timeout can't be higher than 256 (it's u8)
					placeholder->placement_timeout++;
					break;
				}
			}
			if (placeholder->requested &&
				can_build_placeholder(layouts, town, placeholder) &&
				can_place_unit(base_unit_id, placeholder->position, i) &&
				units_dat::base_property[base_unit_id] & UnitProperty::Building &&
				has_down_equivalent(town, base_unit_id) &&
				number_of_equivalents_up(town, base_unit_id) < placeholder->request_count) {
				// Lock adding new requests until this request is started
				placeholder->color = graphics::YELLOW;
				break;
			}

		}
	}

	// Draws Layouts Preview
	for (u32 i = 0; i < PlayerLimit; ++i) {
		AiLayouts* layouts = &player_layouts[i];
		if (show_player_layouts[i]) {
			for (u32 j = 0; j < layouts->placeholders_count; ++j) {
				AiPlaceholder* placeholder = &layouts->placeholders[j];

				Box16 bounds = units_dat::unit_bounds[placeholder->unit_id];
				Box16 unit_box = {};
				unit_box.left = placeholder->position.x - bounds.left;
				unit_box.top = placeholder->position.y - bounds.top;
				unit_box.right = placeholder->position.x + bounds.right;
				unit_box.bottom = placeholder->position.y + bounds.bottom;

				Box16 camera = {};
				camera.left = *g_move_to_x;
				camera.top = *g_move_to_y;
				camera.right = *g_move_to_x + 640;
				camera.bottom = *g_move_to_y + 400;

				if (unit_box.left <= camera.right && unit_box.right >= camera.left &&
					unit_box.top <= camera.bottom && unit_box.bottom >= camera.top) {
					graphics::draw_box(unit_box.left, unit_box.top, unit_box.right, unit_box.bottom, placeholder->color, graphics::CoordType::ON_MAP);

					char buffer[256] = {};

					Point16 pos = {};
					pos.x = unit_box.left + 5;
					pos.y = unit_box.top + 5;

					snprintf(buffer, 256, "%s (%u)", get_short_unit_name(placeholder->unit_id).data, i + 1);
					graphics::draw_text(pos.x, pos.y, buffer, graphics::FONT_SMALL, graphics::CoordType::ON_MAP);
					pos.y += 10;

					snprintf(buffer, 256, "Prio: %u", placeholder->priority);
					graphics::draw_text(pos.x, pos.y, buffer, graphics::FONT_SMALL, graphics::CoordType::ON_MAP);
					pos.y += 10;
				}
			}
		}
	}
}

AiLayoutsTown* get_layouts_town(AiTown* town) {
	s32 index = town - g_ai_town_array;
	assert(index >= 0 && index <= AiTownLimit);
	return &layouts_towns[index];
}

u32 ai_get_next_custom_expo(AiTown* main_town) {
	u32 best_priority = 0;
	u32 best_distance = UINT_MAX;
	u32 best_resarea = 0;

	AiLayouts* layouts = &player_layouts[main_town->player];
	for (u32 j = 0; j < layouts->placeholders_count; ++j) {
		AiPlaceholder* placeholder = &layouts->placeholders[j];
		// Check if resarea is available
		if (g_resource_areas->areas[placeholder->resarea].flags & 3) {
			continue;
		}
		// Skip disabled placeholders
		if (placeholder->disabled) {
			continue;
		}
		// See if it's an optimal expansion
		if (placeholder->should_request && units_dat::base_property[placeholder->unit_id] & UnitProperty::Building) {
			u32 distance = scbw::get_distance_fast(main_town->position, g_resource_areas->areas[placeholder->resarea].position);
			if (placeholder->priority > best_priority || (placeholder->priority == best_priority && distance < best_distance)) {
				best_priority = placeholder->priority;
				best_distance = distance;
				best_resarea = placeholder->resarea;
			} else if (placeholder->priority < best_priority) {
				break;
			}
		}
	}

	return best_resarea;
}

AiUnitEquivalent find_unit_equivalent_down(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && !equivalents[i].merge_requests) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

AiUnitEquivalent find_unit_equivalent_up(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].from) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

u32 has_down_equivalent(AiTown* town, u32 unit_id) {
	AiUnitEquivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.from != UnitId::None) {
		return count_units_in_town(town, equivalent.from);
	}
	return true;
}

u32 number_of_equivalents_up(AiTown* town, u32 unit_id) {
	u32 i = count_units_in_town(town, unit_id);
	if (unit_id == UnitId::ZergCreepColony) {
		i += number_of_equivalents_up(town, UnitId::ZergSunkenColony);
		i += number_of_equivalents_up(town, UnitId::ZergSporeColony);
		i += number_of_equivalents_up(town, UnitId::ZergLarvalColony);
	} else {
		AiUnitEquivalent equivalent = find_unit_equivalent_up(unit_id);
		if (equivalent.to != UnitId::None) {
			i += number_of_equivalents_up(town, equivalent.to);
		}
	}
	return i;
}

u32 find_unit_merge(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
			return equivalents[i].from;
		}
	}
	return UnitId::None;
}

u32 find_unit_base_merge(u32 unit_id) {
    for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
        if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
            return find_unit_base_merge(equivalents[i].from);
        }
    }
    return unit_id;
}

void add_placeholder_by_unit_id(AiLayouts* layouts, AiPlaceholder* placeholder, u32 unit_id) {
	if (unit_id == UnitId::SpecialControlNode) {
		if (!placeholder->was_preplaced) {
			placeholder->should_request = false;
			add_placeholder_by_unit_id(layouts, placeholder, UnitId::TerranCommandCenter);
			add_placeholder_by_unit_id(layouts, placeholder, UnitId::ProtossNexus);
			add_placeholder_by_unit_id(layouts, placeholder, UnitId::ZergHatchery);
		}
		return;
	}

	AiUnitEquivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.from != UnitId::None) {
		add_placeholder_by_unit_id(layouts, placeholder, equivalent.from);
	}

	for (u32 i = 0; i < layouts->placeholders_count; ++i) {
		AiPlaceholder* other = &layouts->placeholders[i];
		if (unit_id == layouts->placeholders[i].unit_id &&
			placeholder->position.x == other->position.x && placeholder->position.y == other->position.y) {
			if (other->priority < placeholder->priority) {
				other->priority = placeholder->priority;
			}
			placeholder->priority -= equivalent.priority_decrease;
			return;
		}
	}

	placeholder->unit_id = unit_id;
	if (layouts->placeholders_count < PlaceholderLimit) {
		layouts->placeholders[layouts->placeholders_count++] = *placeholder;
	}
	placeholder->priority -= equivalent.priority_decrease;
}

void add_placeholder(CHKUnit* unit, u32 player) {
	AiPlaceholder placeholder = {};
	placeholder.position.x = unit->x;
	placeholder.position.y = unit->y;
	placeholder.priority = unit->hp;
	placeholder.guard_rebuilds = unit->energy == 100 ? 0 : unit->energy + 1;
	u32 flags = unit->valid_state_flags & unit->state_flags;
	placeholder.was_preplaced = !(flags & ChkUnitFlags_Cloaked);
	placeholder.should_request = !(flags & ChkUnitFlags_Hallucinated);
	if (!(flags & ChkUnitFlags_Invincible)) {
		placeholder.resarea = scbw::get_closest_resarea(placeholder.position);
	}
	add_placeholder_by_unit_id(&player_layouts[player], &placeholder, unit->unit_id);
}

static u32 get_unit_class(u32 unit_id) {
	if (unit_id == UnitId::ProtossPylon || !(units_dat::base_property[unit_id] & UnitProperty::Building)) {
		return 3;
	}
	if (unit_id == UnitId::ZergCreepColony || unit_id == UnitId::ZergLarvalColony || unit_id == UnitId::ZergSunkenColony || unit_id == UnitId::ZergSporeColony) {
		return 2;
	}
	if (units_dat::group_flags[unit_id].is_factory || scbw::unit_id_is_tech(unit_id)) {
		return 1;
	}
	return 0;
} 

static u32 get_unit_cost(u32 unit_id) {
	return units_dat::mineral_cost[unit_id] + units_dat::gas_cost[unit_id];
}

static s32 compare_base_layout(const void* a, const void* b) {
	AiPlaceholder* pa = (AiPlaceholder*)a;
	AiPlaceholder* pb = (AiPlaceholder*)b;

	s32 priority_diff = pb->priority - pa->priority;
	if (priority_diff) return priority_diff;

	s32 class_diff = get_unit_class(pb->unit_id) - get_unit_class(pa->unit_id);
	if (class_diff) return class_diff;

	s32 cost_diff = get_unit_cost(pa->unit_id) - get_unit_cost(pb->unit_id);
	if (cost_diff) return cost_diff;

	if (pa->resarea && pb->resarea) {
		Point16 point_a = g_resource_areas->areas[pa->resarea].position;
		Point16 point_b = g_resource_areas->areas[pb->resarea].position;
		s32 pos_diff = scbw::get_distance_fast(pa->position, point_a) - scbw::get_distance_fast(pb->position, point_b);
		if (pos_diff) return pos_diff;
	}

	return 0;
}

void sort_layouts(AiLayouts* layouts) {
	qsort(layouts->placeholders, layouts->placeholders_count, sizeof(AiPlaceholder), compare_base_layout);
}

u32 count_units_in_town(AiTown* town, u32 unit_id) {
	u32 count = 0;
	for (Unit* unit = first_player_unit->unit[town->player]; unit; unit = unit->player_link.next) {
		UnitAi* unit_ai = unit->unit_ai;
		if (unit_ai && (unit_ai->type == AiType::Worker || unit_ai->type == AiType::Building)) {
			if (unit_ai->worker_ai.town == town) {
				if (unit->id == unit_id) {
					count += 1;
				} else {
					count += helper::ai_get_queued_count(unit, unit_id);
				}
			}
		} else if (unit->id == unit_id) {
			// Could add closest town check, but for now we will just count any unit that doesn't belong to a town
			count += 1;
		}
	}
	return count;
}

bool unit_has_creep(u32 unit_id, u32 x, u32 y) {
	Box32 area = {};
	area.w = units_dat::building_dimensions[unit_id].x;
	area.h = units_dat::building_dimensions[unit_id].y;
	area.x = x - area.w / 2;
	area.y = y - area.h / 2;
	for (u32 xx = 0; xx < area.w / 32; ++xx) {
		for (u32 yy = 0; yy < area.h / 32; ++yy) {
			ActiveTile* tile = scbw::get_active_tile_at_coords(area.x + xx * 32, area.y + yy * 32);
			if (!tile->has_creep) {
				return false;
			}
		}
	}
	return true;
}

bool unit_has_no_or_allied_creep(u32 unit_id, u32 x, u32 y, u32 player) {
	Box32 area = {};
	area.w = units_dat::building_dimensions[unit_id].x;
	area.h = units_dat::building_dimensions[unit_id].y;
	area.x = x - area.w / 2;
	area.y = y - area.h / 2;
	for (u32 xx = 0; xx < area.w / 32; ++xx) {
		for (u32 yy = 0; yy < area.h / 32; ++yy) {
			ActiveTile* tile = scbw::get_active_tile_at_coords(area.x + xx * 32, area.y + yy * 32);
			TileExt* tile_ext = get_tile_ext_at_coords(area.x + xx * 32, area.y + yy * 32);
			if (tile->has_creep && !can_place_on_creep_tile(tile_ext, player)) {
				return false;
			}
		}
	}
	return true;
}

bool is_basic_townhall(u32 unit_id) {
	return unit_id == UnitId::TerranCommandCenter ||
		unit_id == UnitId::ProtossNexus || 
		unit_id == UnitId::ZergHatchery;
}

bool can_place_unit(u32 unit_id, Point16 position, u32 player) {
	if (find_unit_equivalent_down(unit_id).from != UnitId::None) {
		return true; // If we are requesting an structure upgrade that is not merged into one request, we don't actually place it
	}
	if (units_dat::base_property[unit_id] & UnitProperty::CreepBuilding &&
		!unit_has_creep(unit_id, position.x, position.y)) {
		return false;
	}
	if (units_dat::base_property[unit_id] & UnitProperty::Building && !units_dat::group_flags[unit_id].is_zerg &&
		!unit_has_no_or_allied_creep(unit_id, position.x, position.y, player)) {
		return false;
	}
	if (units_dat::base_property[unit_id] & UnitProperty::RequiredPsi &&
		!helper::unit_has_power(unit_id, position.x, position.y, player)) {
		return false;
	}
	return true;
}

bool can_build_placeholder(AiLayouts* layouts, AiTown* town, AiPlaceholder* placeholder) {
	u32 unit_id = placeholder->unit_id;

	// Check tech availability
	u32 base_unit_id = find_unit_base_merge(unit_id);
	if (!hooks::create_unit_id_allowed(base_unit_id, town->player, false) ||
		(unit_id != base_unit_id && !hooks::create_unit_id_allowed(unit_id, town->player, true))) {
		return false;
	}

	return true;
}

u32 ai_mining_base_count(u32 player) {
	u32 count = 0;
	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &g_ai_town_array[i];
		if (town->player == player && town->resarea) {
			count++;
		}
	}
	return count;
}

bool has_expanding_town(u32 player) {
	if (ai_mining_base_count(player) > 2) {
		return false;
	}
	for (u32 i = 0; i <= AiTownLimit; ++i) {
		AiTown* town = &g_ai_town_array[i];
		AiLayoutsTown* layouts_town = &layouts_towns[i];
		if (town->player == player && !layouts_town->wants_no_resarea && !town->main_building && layouts_town->inited) {
			return true;
		}
	}
	return false;
}

void init_placeholder(AiTown* town, AiPlaceholder* placeholder) {
	placeholder->inited = true;

	u32 merge = find_unit_merge(placeholder->unit_id);
	if (merge != UnitId::None) {
		AiPlaceholder copy = *placeholder;
		copy.unit_id = merge;
		init_placeholder(town, &copy);
	}

	if (units_dat::base_property[placeholder->unit_id] & UnitProperty::Building) {
		scbw::send_gptp_aise_command((Unit*)placeholder, (Unit*)town, GptpId::SetBaseLayout, town->player, 0, 0);
	}
}

void request_placeholder(AiTown* town, AiLayoutsTown* layouts_town, AiPlaceholder* placeholder) {
	u32 merge = find_unit_merge(placeholder->unit_id);
	if (merge != UnitId::None) {
		AiPlaceholder copy = *placeholder;
		copy.unit_id = merge;
		request_placeholder(town, layouts_town, &copy);
	}

	u32 count = ++layouts_town->request_count[placeholder->unit_id];
	placeholder->request_count = count;
	if (units_dat::base_property[placeholder->unit_id] & UnitProperty::Building) {
		helper::ai_build(placeholder->unit_id, false, count, town, placeholder->priority);
	} else {
		scbw::send_gptp_aise_command((Unit*)placeholder, (Unit*)town, GptpId::SetGuard, town->player, 0, 0);
	}
}