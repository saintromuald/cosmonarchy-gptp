#include "ai_layouts.h"
#include "hooks/unit_stats/new_requirements.h"
#include "hooks/orders/ai/ai_orders.h"
#include "hooks/harvest.h"
#include "data_ext.h"
#include "trigger_system/trigger_parser.h"

static bool update_townhall_marker(AiPlaceholder& placeholder, u32 player, u32 townhall_index);
static u32 ai_mining_base_count(u32 player);
static u32 get_unit_priority_class(u32 unit_id);
static u32 get_unit_total_cost(u32 unit_id);

void AiLayouts::init() {
	memset(show_player_layouts, 0, sizeof(show_player_layouts));
	memset(default_ai_expansion_block, 0, sizeof(default_ai_expansion_block));
	m_player_layouts = {};
	m_town_layouts = {};

	for (const CHKUnit& chk_unit : g_chk_cache) {
		if (chk_unit.player > PlayerLimit && chk_unit.player != 11) {
			continue;
		}
		if (!(units_dat::base_property[chk_unit.unit_id] & UnitProperty::Building) && units_dat::computer_ai_internal[chk_unit.unit_id] & 0x2) { // Don't become guard
			continue;
		}

		if (chk_unit.player == 11) {
			u32 flags = chk_unit.valid_state_flags & chk_unit.state_flags;
			if (flags & ChkUnitFlags_Cloaked || chk_unit.unit_id == UnitId::TownCenterMarker) {
				for (u32 i = 0; i < 8; ++i) {
					add_placeholder_chk(chk_unit, i);
				}
			}
		} else {
			add_placeholder_chk(chk_unit, chk_unit.player);
		}
	}

	for (u32 player = 0; player < PlayerLimit; ++player) {
		PlayerLayouts& layouts = m_player_layouts[player];
		sort_layouts(layouts);

		for (AiPlaceholder& placeholder : layouts.placeholders) {
			if (!(units_dat::base_property[placeholder.unit_id] & UnitProperty::Building)) {
				for (GuardAi* guard_ai = g_player_guard_list->player[player].first; guard_ai; guard_ai = guard_ai->link.prev) {
					if (guard_ai->original_home.x == placeholder.position.x &&
						guard_ai->original_home.y == placeholder.position.y &&
						guard_ai->unit_id == placeholder.unit_id) {
						placeholder.guard_ai = guard_ai;
					}
				}
			}
		}
	}
}

void AiLayouts::update() {
	// Update Layouts Towns
	for (u32 town_id = 0; town_id < AiTownLimit; ++town_id) {
		AiTown* town = &g_ai_town_array[town_id];
		TownLayouts& layouts_town = m_town_layouts[town_id];
		if (!town->inited) {
			layouts_town.inited = false;
			layouts_town.wants_no_resarea = false;
			layouts_town.worker_limit = 0;
			layouts_town.worker_maynard = 0;
		}
		if (town->inited && !layouts_town.inited && (town->resarea || layouts_town.wants_no_resarea)) {
			layouts_town.inited = true;
			memset(layouts_town.request_count, 0, sizeof(layouts_town.request_count));

			PlayerLayouts& layouts = m_player_layouts[town->player];
			if (!layouts_town.wants_no_resarea) {
				// Claim ownership of invincible placeholders
				for (AiPlaceholder& placeholder : layouts.placeholders) {
					if (!placeholder.resarea) {
						placeholder.resarea = town->resarea;
						layouts.should_sort = true;
					}
				}
			}

			// Init placeholders
			for (AiPlaceholder& placeholder : layouts.placeholders) {
				if ((!placeholder.town && placeholder.resarea == town->resarea) || (placeholder.town && placeholder.town == town)) {
					placeholder.inited = false;
					placeholder.requested = false;
					placeholder.town = town;
					placeholder.placement_timeout = 0;
				}
			}

			if (show_player_layouts[town->player]) {
				bw::printf("Player %u started Town #%u at resarea %u", town->player + 1, town_id, town->resarea);
			}
		}
		if (layouts_town.inited) {
			memset(layouts_town.request_count, 0, sizeof(layouts_town.request_count)); // Have to reset it every frame, but it's pretty costly
			if (town->worker_limit < layouts_town.worker_limit) {
				town->worker_limit = layouts_town.worker_limit;
			}
			if (layouts_town.worker_maynard) {
				helper::ai_get_old_peons(town, 1);
			} else if (town->worker_count < layouts_town.worker_limit) {
				layouts_town.worker_maynard += 1;
			}
		}
	}

	// Update Layouts
	for (u32 player = 0; player < PlayerLimit; ++player) {
		PlayerLayouts& layouts = m_player_layouts[player];

		// Update State
		bool is_expanding = has_expanding_town(player);
		u32 townhall_index = 0;
		for (AiPlaceholder& placeholder : layouts.placeholders) {
			// Update town
			if (placeholder.town) {
				AiTown* town = placeholder.town;
				if (!town->inited || town->player != player) {
					placeholder.town = 0;
				}
			}

			// Update guard
			if (placeholder.guard_ai) {
				placeholder.guard_ai->times_died = 0;
				if (placeholder.guard_ai->parent) {
					placeholder.color = graphics::BLUE;
				}
			}

			// Update townhall markers
			if (placeholder.is_townhall_marker) {
				if (update_townhall_marker(placeholder, player, townhall_index)) {
					townhall_index += 1;
					layouts.should_sort = true;
				}
			}

			// Init if not already
			if (placeholder.town && !placeholder.disabled) {
				init_placeholder(placeholder.town, placeholder);
			}

			// Colors
			if (placeholder.requested) {
				placeholder.color = graphics::GREEN;
			} else if (placeholder.disabled) {
				placeholder.color = graphics::BLACK;
			} else if (!placeholder.should_request) {
				placeholder.color = graphics::BROWN;
			} else if (is_expanding) {
				placeholder.color = graphics::PURPLE;
			} else {
				placeholder.color = graphics::WHITE;
			}
		}

		// Sort Layouts - if there've been any updates
		if (layouts.should_sort) {
			layouts.should_sort = false;
			sort_layouts(layouts);
		}

		// Request Layouts
		s32 idx = -1;
		for (AiPlaceholder& placeholder : layouts.placeholders) {
			idx++;
			if (is_expanding) {
				break;
			}
			if (placeholder.disabled || !placeholder.should_request) {
				continue;
			}
			AiTown* town = placeholder.town;
			if (!town) {
				continue;
			}
			TownLayouts& layouts_town = get_town_layouts(town);
			u32 base_unit_id = find_unit_root_merge(placeholder.unit_id);

			layouts_town.request_count[placeholder.unit_id] += 1;
			if (can_build_placeholder(town, placeholder)) {
				if (can_place_unit(base_unit_id, placeholder.position, player)) {
					if (!placeholder.requested) {
						placeholder.requested = true;
						if (show_player_layouts[town->player]) {
							bw::printf("Player %u requested %s #%u at resarea %u", player + 1, Unit::get_name(placeholder.unit_id), idx, placeholder.resarea);
						}
					}

					bool is_building = units_dat::base_property[placeholder.unit_id] & UnitProperty::Building;
					bool not_guard = units_dat::computer_ai_internal[placeholder.unit_id] & 2;
					if (is_building || not_guard) {
						if (town_count_up_equivalents(town, placeholder.unit_id) < layouts_town.request_count[placeholder.unit_id]) {
							request_placeholder(town, layouts_town, placeholder);
							placeholder.color = graphics::ORANGE;
							if (is_building && town_has_down_equivalent(town, base_unit_id)) {
								// Lock adding new requests until this request is started
								placeholder.color = graphics::YELLOW;
								break;
							}
						}
					} else if (!placeholder.guard_ai || !placeholder.guard_ai->parent) {
						request_placeholder(town, layouts_town, placeholder);
						placeholder.color = graphics::ORANGE;
					}
				} else if (placeholder.placement_timeout < 10 * 24) { // Keep in mind that placement_timeout can't be higher than 256 (it's u8)
					placeholder.placement_timeout++;
					break;
				}
			}
		}

		// Draw layouts
		if (show_player_layouts[player]) {
			s32 idx = -1;
			for (AiPlaceholder& placeholder : layouts.placeholders) {
				idx++;
				if (placeholder.is_townhall_marker) {
					continue;
				}

				Box16 bounds = units_dat::unit_bounds[placeholder.unit_id];
				Box16 unit_box = {};
				unit_box.left = placeholder.position.x - bounds.left;
				unit_box.top = placeholder.position.y - bounds.top;
				unit_box.right = placeholder.position.x + bounds.right;
				unit_box.bottom = placeholder.position.y + bounds.bottom;

				Box16 camera = {};
				camera.left = *g_move_to_x;
				camera.top = *g_move_to_y;
				camera.right = *g_move_to_x + 640;
				camera.bottom = *g_move_to_y + 400;

				if (unit_box.left <= camera.right && unit_box.right >= camera.left &&
					unit_box.top <= camera.bottom && unit_box.bottom >= camera.top) {
					g_graphics.draw_box(unit_box.left, unit_box.top, unit_box.right, unit_box.bottom, placeholder.color, graphics::CoordType::ON_MAP);

					char buffer[256] = {};

					Point16 pos = {};
					pos.x = unit_box.left + 5;
					pos.y = unit_box.top + 5;

					snprintf(buffer, 256, "#%u %s (%u)", (u32)idx, get_short_unit_name(placeholder.unit_id).string().c_str(), player + 1);
					g_graphics.draw_text(pos.x, pos.y, buffer, graphics::FONT_SMALL, graphics::CoordType::ON_MAP);
					pos.y += 10;

					snprintf(buffer, 256, "Prio: %u", placeholder.priority);
					g_graphics.draw_text(pos.x, pos.y, buffer, graphics::FONT_SMALL, graphics::CoordType::ON_MAP);
					pos.y += 10;
				}
			}
		}
	}
}

void AiLayouts::add_placeholder_unit_id(PlayerLayouts& layouts, AiPlaceholder& placeholder, u32 unit_id) {
	if (unit_id == UnitId::TownCenterMarker) {
		placeholder.is_townhall_marker = true;
		placeholder.was_preplaced = false;

		// Initial values, will be updated later
		// Priority needs to be the same for correct sorting
		placeholder.should_request = true;
		placeholder.priority = 180;
		placeholder.yield_score = 0;

		u32 resarea = bw::get_closest_resarea(placeholder.position);
		EntryArray found = {};
		unit_search_radial(&found, placeholder.position, 12 * 32);
		for (u32 i = 0; i < found.count; ++i) {
			Unit* unit = found.units[i];
			if (!unit_id_is_resource_node(unit->id, false)) {
				continue;
			}
			if (unit->building.resource.resarea != resarea) {
				continue;
			}

			u32 score = get_harvest_amount(unit);
			if (unit_id_is_uncapped_gas_node(unit->id)) {
				score *= 3;
			}
			u32 distance = bw::get_distance_fast(placeholder.position, unit->position) / 32;
			if (distance) {
				score /= distance;
			}
			placeholder.yield_score += score;
		}
	}

	AiUnitEquivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.from != UnitId::None) {
		add_placeholder_unit_id(layouts, placeholder, equivalent.from);
	}

	for (AiPlaceholder& other : layouts.placeholders) {
		if (unit_id == other.unit_id && placeholder.position == other.position) {
			if (other.priority < placeholder.priority) {
				other.priority = placeholder.priority;
			}
			placeholder.priority -= equivalent.priority_decrease;
			return;
		}
	}

	placeholder.unit_id = unit_id;
	layouts.placeholders.push_back(placeholder);
	placeholder.priority -= equivalent.priority_decrease;
}

void AiLayouts::add_placeholder_chk(const CHKUnit& unit, u32 player) {
	AiPlaceholder placeholder = {};
	placeholder.position.x = unit.x;
	placeholder.position.y = unit.y;
	placeholder.priority = unit.hp;
	placeholder.guard_rebuilds = unit.energy == 100 ? 0 : unit.energy + 1;
	u32 flags = unit.valid_state_flags & unit.state_flags;
	placeholder.was_preplaced = !(flags & ChkUnitFlags_Cloaked);
	placeholder.should_request = !(flags & ChkUnitFlags_Hallucinated);
	if (!(flags & ChkUnitFlags_Invincible)) {
		placeholder.resarea = bw::get_closest_resarea(placeholder.position);
	}
	add_placeholder_unit_id(m_player_layouts[player], placeholder, unit.unit_id);
}

Point16 AiLayouts::get_ai_town_position(AiTown* town) {
	PlayerLayouts& layouts = m_player_layouts[town->player];
	for (AiPlaceholder& placeholder : layouts.placeholders) {
		if (placeholder.resarea == town->resarea && placeholder.is_townhall_marker) {
			return placeholder.position;
		}
	}
	return town->position;
}

PlayerLayouts& AiLayouts::get_player_layouts(u32 player) {
	assert(player >= 0 && player < PlayerLimit);
	return m_player_layouts[player];
}

TownLayouts& AiLayouts::get_town_layouts(AiTown* town) {
	s32 index = town - g_ai_town_array;
	assert(index >= 0 && index <= AiTownLimit);
	return m_town_layouts[index];
}

u32 AiLayouts::ai_get_next_custom_expo(AiTown* main_town) {
	u32 best_priority = 0;
	u32 best_distance = UINT_MAX;
	u32 best_resarea = 0;

	bool has_transport = bw::player_has_transport(main_town->player);

	PlayerLayouts& layouts = m_player_layouts[main_town->player];
	for (AiPlaceholder& placeholder : layouts.placeholders) {
		// Check if resarea is available
		if (!hooks::ai_resarea_valid_for_expo(main_town->player, placeholder.resarea, main_town, has_transport, 0)) {
			continue;
		}
		// Skip disabled placeholders
		if (placeholder.disabled) {
			continue;
		}
		// See if it's an optimal expansion
		if (placeholder.should_request && !placeholder.is_townhall_marker && units_dat::base_property[placeholder.unit_id] & UnitProperty::Building) {
			u32 distance = bw::get_distance_fast(main_town->position, g_resource_areas->areas[placeholder.resarea].position);
			if (placeholder.priority > best_priority || (placeholder.priority == best_priority && distance < best_distance)) {
				best_priority = placeholder.priority;
				best_distance = distance;
				best_resarea = placeholder.resarea;
			} else if (placeholder.priority < best_priority) {
				break;
			}
		}
	}

	return best_resarea;
}

bool AiLayouts::has_expanding_town(u32 player) {
	if (ai_mining_base_count(player) > 2) {
		return false;
	}
	for (u32 i = 0; i <= AiTownLimit; ++i) {
		AiTown* town = &g_ai_town_array[i];
		TownLayouts& layouts_town = m_town_layouts[i];
		if (town->player == player && !layouts_town.wants_no_resarea && !town->main_building && layouts_town.inited) {
			return true;
		}
	}
	return false;
}

bool AiLayouts::can_build_placeholder(AiTown* town, AiPlaceholder& placeholder) {
	u32 unit_id = placeholder.unit_id;

	// Check tech availability
	u32 base_unit_id = find_unit_root_merge(unit_id);
	if (!hooks::create_unit_id_allowed(base_unit_id, town->player, false) ||
		(unit_id != base_unit_id && !hooks::create_unit_id_allowed(unit_id, town->player, true))) {
		return false;
	}

	return true;
}

void AiLayouts::init_placeholder(AiTown* town, AiPlaceholder& placeholder) {
	if (placeholder.inited) {
		return;
	}

	u32 merge = find_unit_merge(placeholder.unit_id);
	if (merge != UnitId::None) {
		AiPlaceholder copy = placeholder;
		copy.unit_id = merge;
		init_placeholder(town, copy);
	}

	placeholder.inited = true;
	if (units_dat::base_property[placeholder.unit_id] & UnitProperty::Building) {
		bw::send_gptp_aise_command((Unit*)&placeholder, (Unit*)town, GptpId::SetBaseLayout, town->player, 0, 0);
	}
}

void AiLayouts::request_placeholder(AiTown* town, TownLayouts& layouts_town, AiPlaceholder& placeholder) {
	if (units_dat::base_property[placeholder.unit_id] & UnitProperty::Building) {
		// Requesting the base unit currently handled in town_add_spending_request
		/*
		u32 merge = find_unit_merge(unit_id);
		if (merge != UnitId::None) {
			AiPlaceholder copy = *placeholder;
			copy.unit_id = merge;
			request_placeholder(town, layouts_town, &copy);
		}
		*/

		bw::town_add_spending_request(town, 1, placeholder.unit_id, placeholder.priority);
	} else if (units_dat::computer_ai_internal[placeholder.unit_id] & 2) {
		bw::military_add_spending_request(town->player, placeholder.unit_id, placeholder.position, placeholder.priority);
	} else {
		if (!placeholder.guard_ai) {
			placeholder.guard_ai = bw::create_guard_ai(town->player, placeholder.unit_id, placeholder.position);
		}
		if (placeholder.guard_ai && !placeholder.guard_ai->parent) {
			bw::guard_add_spending_request(town->player, placeholder.guard_ai, placeholder.priority);
		}
		// We no longer use Aise for guards...
	//	bw::send_gptp_aise_command((Unit*)placeholder, (Unit*)town, GptpId::SetGuard, town->player, 0, 0);
	}
}

static bool update_townhall_marker(AiPlaceholder& placeholder, u32 player, u32 townhall_index) {
	switch (g_player_table[player].race) {
		case RaceId::Terran: {
			if (townhall_index && hooks::create_unit_id_allowed(UnitId::TerranTreasury, player, true)) {
				placeholder.unit_id = UnitId::TerranTreasury;
			} else {
				placeholder.unit_id = UnitId::TerranMinistry;
			}
		} break;
		case RaceId::Protoss: {
			placeholder.unit_id = UnitId::ProtossNexus;
		} break;
		case RaceId::Zerg: {
			placeholder.unit_id = UnitId::ZergHachirosk;
		} break;
	}

	u32 old_priority = placeholder.priority;
	if (townhall_index) {
		placeholder.should_request = true;
		placeholder.priority = 170; // this should be lower (probably 130 or 140), but it needs to be high due to what I assume is some aise bug with spending requests -Veeq7
	} else {
		placeholder.should_request = false;
		placeholder.priority = 180;
	}
	return old_priority != placeholder.priority;
}

u32 find_unit_base(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to) {
			return equivalents[i].from;
		}
	}
	return UnitId::None;
}

AiUnitEquivalent find_unit_equivalent_down(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && !equivalents[i].merge_requests) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

AiUnitEquivalent find_unit_equivalent_up(u32 unit_id, u32* index) {
	for (u32 i = index ? *index : 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (index) *index += 1;
		if (unit_id == equivalents[i].from) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

u32 find_unit_merge(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
			return equivalents[i].from;
		}
	}
	return UnitId::None;
}

u32 find_unit_root_merge(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
			return find_unit_root_merge(equivalents[i].from);
		}
	}
	return unit_id;
}

u32 town_count_units(AiTown* town, u32 unit_id, bool completed) {
	u32 count = 0;
	for (Unit* unit = first_player_unit->unit[town->player]; unit; unit = unit->player_link.next) {
		if (completed) {
			if (unit->id == unit_id && unit->status & UnitStatus::Completed && town == bw::get_closest_town(unit->player, unit->position)) {
				count += 1;
			}
		} else {
			if (unit->id == unit_id && town == bw::get_closest_town(unit->player, unit->position)) {
				count += 1;
			}
			UnitAi* unit_ai = unit->unit_ai;
			if (unit_ai && (unit_ai->type == AiType::Worker || unit_ai->type == AiType::Building) && unit_ai->worker_ai.town == town) {
				count += helper::ai_get_queued_count(unit, unit_id);
			}
		}
	}
	return count;
}

u32 town_has_down_equivalent(AiTown* town, u32 unit_id) {
	AiUnitEquivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.from != UnitId::None) {
		return town_count_units(town, equivalent.from, true);
	}
	return true;
}

u32 town_count_up_equivalents(AiTown* town, u32 unit_id) {
	u32 count = town_count_units(town, unit_id, false);
	u32 index = 0;
	AiUnitEquivalent equivalent = find_unit_equivalent_up(unit_id, &index);
	while (equivalent.to != UnitId::None) {
		count += town_count_up_equivalents(town, equivalent.to);
		equivalent = find_unit_equivalent_up(unit_id, &index);
	}
	return count;
}

u32 player_count_units(u32 player, u32 unit_id, bool completed) {
	u32 count = 0;
	for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
		if (completed) {
			if (unit->id == unit_id && unit->status & UnitStatus::Completed) {
				count += 1;
			}
		} else {
			if (unit->id == unit_id) {
				count += 1;
			}
			UnitAi* unit_ai = unit->unit_ai;
			if (unit_ai && (unit_ai->type == AiType::Worker || unit_ai->type == AiType::Building)) {
				count += helper::ai_get_queued_count(unit, unit_id);
			}
		}
	}
	return count;
}

u32 player_count_up_equivalents(u32 player, u32 unit_id) {
	u32 count = player_count_units(player, unit_id, false);
	u32 index = 0;
	AiUnitEquivalent equivalent = find_unit_equivalent_up(unit_id, &index);
	while (equivalent.to != UnitId::None) {
		count += player_count_up_equivalents(player, equivalent.to);
		equivalent = find_unit_equivalent_up(unit_id, &index);
	}
	return count;
}

static u32 get_unit_priority_class(u32 unit_id) {
	if (unit_id == UnitId::ProtossPylon || !(units_dat::base_property[unit_id] & UnitProperty::Building)) {
		return 3;
	}
	if (unit_id == UnitId::ZergCagrantCesiant
	||  unit_id == UnitId::ZergLarvoskCesiant
	||  unit_id == UnitId::ZergSurkithCesiant
	||  unit_id == UnitId::ZergSpraithCesiant
	||  unit_id == UnitId::ZergSkortrithCesiant
	||  unit_id == UnitId::ZergSpirtithCesiant
	||	unit_id == UnitId::ZergNydusCesiant) {
		return 2;
	}
	if (units_dat::group_flags[unit_id].is_factory || unit_id_is_tech(unit_id)) {
		return 1;
	}
	return 0;
} 

static u32 get_unit_total_cost(u32 unit_id) {
	return units_dat::mineral_cost[unit_id] + units_dat::vespene_cost[unit_id];
}

void sort_layouts(PlayerLayouts& layouts) {
	std::ranges::sort(layouts.placeholders, [](AiPlaceholder& a, AiPlaceholder& b) {
		s32 priority_diff = b.priority - a.priority;
		if (priority_diff) return priority_diff < 0;

		if (a.is_townhall_marker && b.is_townhall_marker) {
			s32 yield_diff = b.yield_score - a.yield_score;
			if (yield_diff) return yield_diff < 0;
		}

		s32 class_diff = get_unit_priority_class(b.unit_id) - get_unit_priority_class(a.unit_id);
		if (class_diff) return class_diff < 0;

		s32 cost_diff = get_unit_total_cost(a.unit_id) - get_unit_total_cost(b.unit_id);
		if (cost_diff) return cost_diff < 0;

		if (a.resarea && b.resarea) {
			Point16 point_a = g_resource_areas->areas[a.resarea].position;
			Point16 point_b = g_resource_areas->areas[b.resarea].position;

			s32 pos_diff = bw::get_distance_fast(a.position, point_a) - bw::get_distance_fast(b.position, point_b);
			if (pos_diff) return pos_diff < 0;
		}

		return false;
	});
}

bool unit_has_creep(u32 unit_id, u32 x, u32 y) {
	Box32 area = {};
	area.w = units_dat::building_dimensions[unit_id].x;
	area.h = units_dat::building_dimensions[unit_id].y;
	area.x = x - area.w / 2;
	area.y = y - area.h / 2;
	for (u32 xx = 0; xx < area.w / 32; ++xx) {
		for (u32 yy = 0; yy < area.h / 32; ++yy) {
			ActiveTile* tile = bw::get_active_tile_at_coords(area.x + xx * 32, area.y + yy * 32);
			if (!tile->has_cagra) {
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
			ActiveTile* tile = bw::get_active_tile_at_coords(area.x + xx * 32, area.y + yy * 32);
			TileExt* tile_ext = get_tile_ext_at_coords(area.x + xx * 32, area.y + yy * 32);
			if (tile->has_cagra && !can_place_on_creep_tile(tile_ext, player)) {
				return false;
			}
		}
	}
	return true;
}

bool is_basic_townhall(u32 unit_id) {
	return unit_id == UnitId::TerranMinistry
		|| unit_id == UnitId::ProtossNexus
		|| unit_id == UnitId::ZergHachirosk;
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

static u32 ai_mining_base_count(u32 player) {
	u32 count = 0;
	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &g_ai_town_array[i];
		if (town->player == player && town->resarea) {
			count++;
		}
	}
	return count;
}
