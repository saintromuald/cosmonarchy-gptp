#include "base_layouts.h"
#include "hooks/unit_stats/new_requirements.h"

u32 chk_cache_size;
CHKUnit chk_cache[ChkCacheLimit];

u32 default_ai_expansion_block[PlayerLimit];

static BaseLayouts mainarea_base_layouts[PlayerLimit];
static BaseLayouts resarea_base_layouts[PlayerLimit][ResAreaLimit];
static BaseLayouts* active_base_layouts[ResAreaLimit];

struct Equivalent {
	u32 from;
	u32 to;
	s32 priority_decrease;
	b32 merge_requests;
};

static Equivalent equivalents[] = {
	{UnitId::ZergHatchery, UnitId::ZergLair, 1},
	{UnitId::ZergLair, UnitId::ZergHive, 1},
	{UnitId::ZergHive, UnitId::ZergSire, 1},

	{UnitId::ZergIrolIris, UnitId::ZergOthstolOviform, 2},
	{UnitId::ZergOthstolOviform, UnitId::ZergAlaszilArbor, 2},

	{UnitId::ZergQuazrokPool, UnitId::ZergLiivralPond, 1},
	{UnitId::ZergHydrokDen, UnitId::ZergGorgralSwamp, 1},
	{UnitId::ZergMuthrokSpire, UnitId::ZergVithrathHaunt, 1},
	{UnitId::ZergMatravilNest, UnitId::ZergAlmakisAntre, 1},
	{UnitId::ZergZorkizShroud, UnitId::ZergUltrokCavern, 1},
	{UnitId::ZergGeszkathGrotto, UnitId::ZergAlkajChasm, 1},

	{UnitId::ZergCreepColony, UnitId::ZergSunkenColony, 0, true},
	{UnitId::ZergCreepColony, UnitId::ZergSporeColony, 0, true},
	{UnitId::ZergCreepColony, UnitId::ZergLarvalColony, 0, true},

	{UnitId::ZergOthstoleth, UnitId::ZergAlaszileth},
	{UnitId::ZergOvileth, UnitId::ZergIroleth},

	{UnitId::ZergZethrokor, UnitId::ZergVorvrokor},
	{UnitId::ZergQuazilisk, UnitId::ZergLiiralisk},
	{UnitId::ZergHydralisk, UnitId::ZergBactalisk},
	{UnitId::ZergSkithrokor, UnitId::ZergGorgrokor},
	{UnitId::ZergMutalisk, UnitId::ZergVithrilisk},
	{UnitId::ZergNathrokor, UnitId::ZergKalkalisk},
	{UnitId::ZergKonvilisk, UnitId::ZergAlmaksalisk},
	{UnitId::ZergMatraleth, UnitId::ZergAkistrokor},
	{UnitId::ZergZoryusthaleth, UnitId::ZergUltrokor},
	{UnitId::ZergKeskathalor, UnitId::ZergAlkajelisk},

	{UnitId::ProtossCantavis, UnitId::ProtossAugur},
};

static Equivalent find_unit_equivalent_down(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && !equivalents[i].merge_requests) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

static Equivalent find_unit_equivalent_up(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].from) {
			return equivalents[i];
		}
	}
	return {UnitId::None, UnitId::None, 0};
}

static u32 find_unit_merge(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
			return equivalents[i].from;
		}
	}
	return UnitId::None;
}

static u32 find_unit_base_merge(u32 unit_id) {
    for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
        if (unit_id == equivalents[i].to && equivalents[i].merge_requests) {
            return find_unit_base_merge(equivalents[i].from);
        }
    }
    return unit_id;
}

static void add_placeholder_by_unit_id(BaseLayouts* base_layouts, Placeholder* placeholder, u32 unit_id) {
	if (unit_id == UnitId::SpecialControlNode) {
		if (!placeholder->was_preplaced) {
			placeholder->should_request = false;
			add_placeholder_by_unit_id(base_layouts, placeholder, UnitId::TerranCommandCenter);
			add_placeholder_by_unit_id(base_layouts, placeholder, UnitId::ProtossNexus);
			add_placeholder_by_unit_id(base_layouts, placeholder, UnitId::ZergHatchery);
		}
		return;
	}

	Equivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.from != UnitId::None && !equivalent.merge_requests) {
		add_placeholder_by_unit_id(base_layouts, placeholder, equivalent.from);
	}

	placeholder->priority -= equivalent.priority_decrease;
	for (u32 i = 0; i < base_layouts->placeholders_count; ++i) {
		Placeholder* other = &base_layouts->placeholders[i];
		if (unit_id == base_layouts->placeholders[i].unit_id &&
			placeholder->position.x == other->position.x && placeholder->position.y == other->position.y) {
			return;
		}
	}

	placeholder->unit_id = unit_id;
	if (base_layouts->placeholders_count < PlaceholderLimit) {
		base_layouts->placeholders[base_layouts->placeholders_count++] = *placeholder;
	}
}

static void add_placeholder(CHKUnit* unit, u32 player) {
	Placeholder placeholder = {};
	placeholder.position.x = unit->x;
	placeholder.position.y = unit->y;
	placeholder.priority = unit->hp;
	placeholder.guard_rebuilds = unit->energy == 100 ? 0 : unit->energy + 1;
	u32 flags = unit->validStateFlags & unit->stateFlags;
	placeholder.was_preplaced = !(flags & ChkUnitFlags_Hallucinated);
	placeholder.should_request = !(flags & ChkUnitFlags_Cloaked);
	if (flags & ChkUnitFlags_Invincible) {
		add_placeholder_by_unit_id(&mainarea_base_layouts[player], &placeholder, unit->unit_id);
	} else {
		u32 res_area = scbw::get_closest_resarea(placeholder.position);
		add_placeholder_by_unit_id(&resarea_base_layouts[player][res_area], &placeholder, unit->unit_id);
	}
}

static u32 get_unit_class(u32 unit_id) {
	if (unit_id == UnitId::ProtossPylon || !(units_dat::BaseProperty[unit_id] & UnitProperty::Building)) {
		return 3;
	}
	if (unit_id == UnitId::ZergCreepColony || unit_id == UnitId::ZergLarvalColony || unit_id == UnitId::ZergSunkenColony || unit_id == UnitId::ZergSporeColony) {
		return 2;
	}
	if (units_dat::GroupFlags[unit_id].isFactory || scbw::unit_id_is_tech(unit_id)) {
		return 1;
	}
	return 0;
} 

static u32 get_unit_cost(u32 unit_id) {
	return units_dat::MineralCost[unit_id] + units_dat::GasCost[unit_id];
}

static s32 compare_base_layout(void* context, const void* a, const void* b) {
	BaseLayouts* base_layouts = (BaseLayouts*)context;
	Placeholder* pa = (Placeholder*)a;
	Placeholder* pb = (Placeholder*)b;

	s32 priority_diff = pb->priority - pa->priority;
	if (priority_diff) return priority_diff;

	s32 class_diff = get_unit_class(pb->unit_id) - get_unit_class(pa->unit_id);
	if (class_diff) return class_diff;

	s32 cost_diff = get_unit_cost(pa->unit_id) - get_unit_cost(pb->unit_id);
	if (cost_diff) return cost_diff;

	s32 pos_diff = scbw::get_distance_fast(pa->position, base_layouts->resarea_pos) - scbw::get_distance_fast(pb->position, base_layouts->resarea_pos);
	if (pos_diff) return pos_diff;

	return 0;
}

static void sort_base_layouts(BaseLayouts* base_layouts) {
	qsort_s(base_layouts->placeholders, base_layouts->placeholders_count, sizeof(Placeholder), compare_base_layout, base_layouts);
}

static void init_base_layouts() {
	memset(default_ai_expansion_block, 0, sizeof(default_ai_expansion_block));
	memset(mainarea_base_layouts, 0, sizeof(mainarea_base_layouts));
	memset(resarea_base_layouts, 0, sizeof(resarea_base_layouts));
	memset(active_base_layouts, 0, sizeof(active_base_layouts));

	for (u32 i = 0; i < chk_cache_size; ++i) {
		CHKUnit* unit = &chk_cache[i];
		if (unit->playerId > PlayerLimit && unit->playerId != 11) {
			continue;
		}
		if (!(units_dat::BaseProperty[unit->unit_id] & UnitProperty::Building) && units_dat::ComputerAIInternal[unit->unit_id] & 0x2) { // Don't become guard
			continue;
		}

		if (unit->playerId == 11) {
			u32 flags = unit->validStateFlags & unit->stateFlags;
			if (flags & ChkUnitFlags_Hallucinated) {
				for (u32 i = 0; i < 8; ++i) {
					add_placeholder(unit, i);
				}
			}
		} else {
			add_placeholder(unit, unit->playerId);
		}
	}

	for (u32 i = 0; i < PlayerLimit; ++i) {
		for (u32 j = 0; j < ResAreaLimit; ++j) {
			BaseLayouts* base_layouts = &resarea_base_layouts[i][j];
			base_layouts->resarea_pos = resource_areas->areas[j].pos;
			sort_base_layouts(base_layouts);

			/*
			if (base_layouts->placeholders_count) {
				char buffer[256] = {};
				sprintf_s(buffer, "player_%u_resarea_%u.txt", i, j);
				FILE* file = fopen(buffer, "wb");

				for (u32 k = 0; k < base_layouts->placeholders_count; ++k) {
					Placeholder* placeholder = &base_layouts->placeholders[k];
					if (units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
						fprintf(file, "%s\n", Unit::getName(placeholder->unit_id));
						fprintf(file, "Priority: %u\n", placeholder->priority);
						fprintf(file, "Was Preplaced: %u\n", placeholder->was_preplaced);
						fprintf(file, "Should Request: %u\n", placeholder->should_request);
						fprintf(file, "\n");
					}
				}

				fclose(file);
			}
			*/
		}
	}
}

static u32 has_down_equivalent(AiTown* town, u32 unit_id) {
	Equivalent equivalent = find_unit_equivalent_down(unit_id);
	if (equivalent.to != UnitId::None) {
		return helper::Ai_CountUnitsInTown(town, equivalent.to, false);
	}
	return true;
}

static u32 number_of_equivalents_up(AiTown* town, u32 unit_id) {
	u32 i = helper::Ai_CountUnitsInTown(town, unit_id, false);
	if (unit_id == UnitId::ZergCreepColony) {
		i += number_of_equivalents_up(town, UnitId::ZergSunkenColony);
		i += number_of_equivalents_up(town, UnitId::ZergSporeColony);
		i += number_of_equivalents_up(town, UnitId::ZergLarvalColony);
	} else {
		Equivalent equivalent = find_unit_equivalent_up(unit_id);
		if (equivalent.to != UnitId::None) {
			i += number_of_equivalents_up(town, equivalent.to);
		}
	}
	return i;
}

static bool unit_has_creep(u32 unit_id, u32 x, u32 y, u32 player) {
	Box32 area = {};
	area.w = units_dat::BuildingDimensions[unit_id].x;
	area.h = units_dat::BuildingDimensions[unit_id].y;
	area.x = x - area.w / 2;
	area.y = y - area.h / 2;
	for (u32 xx = 0; xx < area.w / 32; ++xx) {
		for (u32 yy = 0; yy < area.h / 32; ++yy) {
			if (!scbw::get_active_tile_at(area.x + xx * 32, area.y + yy * 32).hasCreep) {
				return false;
			}
		}
	}
	return true;
}

static bool is_basic_townhall(u32 unit_id) {
	return unit_id == UnitId::TerranCommandCenter ||
		unit_id == UnitId::ProtossNexus || 
		unit_id == UnitId::ZergHatchery;
}

static bool can_build_placeholder(BaseLayouts* base_layouts, Placeholder* placeholder) {
	AiTown* town = base_layouts->town;
	u32 unit_id = placeholder->unit_id;

	// Check tech availability
	u32 base_unit_id = find_unit_base_merge(unit_id);
	if (!hooks::createUnitIdAllowed(base_unit_id, town->player, false) ||
		(unit_id != base_unit_id && !hooks::createUnitIdAllowed(unit_id, town->player, true))) {
		return false;
	}

	// Prevent building non town halls when town hall is missing
	if ((!town->main_building || !(town->main_building->status & UnitStatus::Completed)) && units_dat::BaseProperty[unit_id] & UnitProperty::Building) {
		// Ignore all non townhalls
		if (!is_basic_townhall(unit_id)) {
			return false;
		}

		// Ignore any town halls that aren't the first town hall
		for (u32 i = 0; i < base_layouts->placeholders_count; ++i) {
			Placeholder* other = &base_layouts->placeholders[i];
			if (other == placeholder) {
				break;
			}
			if (is_basic_townhall(placeholder->unit_id)) {
				return false;
			}
		}

		// TODO: maybe add custom get old peons handling
	}

	// Check gas availability
	if (units_dat::GasCost[unit_id] > resources->gas[town->player] &&
		!Player::numberOfCompletedUnitsOfType(town->player, UnitId::TerranReservoir) &&
		!Player::numberOfCompletedUnitsOfType(town->player, UnitId::ProtossAquifer) &&
		!Player::numberOfCompletedUnitsOfType(town->player, UnitId::ZergExcisant)) {
		return false;
	}
	
	// Check placeability
	if (units_dat::BaseProperty[unit_id] & UnitProperty::CreepBuilding &&
		!unit_has_creep(unit_id, placeholder->position.x, placeholder->position.y, town->player)) {
		return false;
	}
	if (units_dat::BaseProperty[unit_id] & UnitProperty::RequiredPsi &&
		!helper::unit_has_power(unit_id, placeholder->position.x, placeholder->position.y, town->player)) {
		return false;
	}

	return true;
}

static bool has_expanding_town(u32 player) {
	for (u32 i = 0; i < ResAreaLimit; ++i) {
		BaseLayouts* base_layouts = active_base_layouts[i];
		if (base_layouts && base_layouts->town->player == player && !base_layouts->town->main_building) {
			return true;
		}
	}
	return false;
}

static void init_placeholder(BaseLayouts* base_layouts, Placeholder* placeholder) {
	u32 merge = find_unit_merge(placeholder->unit_id);
	if (merge != UnitId::None) {
		Placeholder copy = *placeholder;
		copy.unit_id = merge;
		init_placeholder(base_layouts, &copy);
	}

	if (units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
		scbw::sendGPTP_aise_cmd((Unit*)placeholder, (Unit*)base_layouts->town, GptpId::SetBaseLayout, base_layouts->town->player, 0, 0);
	}
}

static void request_placeholder(BaseLayouts* base_layouts, Placeholder* placeholder) {
	u32 merge = find_unit_merge(placeholder->unit_id);
	if (merge != UnitId::None) {
		Placeholder copy = *placeholder;
		copy.unit_id = merge;
		request_placeholder(base_layouts, &copy);
	}

	u32 count = ++base_layouts->request_count[placeholder->unit_id];
	placeholder->wait_build_start_count = count;
	if (units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
		helper::Ai_Build(placeholder->unit_id, false, count, base_layouts->town, placeholder->priority);
	} else {
		scbw::sendGPTP_aise_cmd((Unit*)placeholder, (Unit*)base_layouts->town, GptpId::SetGuard, base_layouts->town->player, 0, 0);
	}
}

u32 ai_get_next_custom_expo(AiTown* main_town) {
	u32 best_priority = 0;
	u32 best_distance = UINT_MAX;
	u32 best_resarea = 0;
	for (u32 i = 0; i < ResAreaLimit; ++i) {
		// Check if resarea is available
		if (resource_areas->areas[i].flags & 3) {
			continue;
		}

		// See if it's an optimal expansion
		BaseLayouts* base_layouts = &resarea_base_layouts[main_town->player][i];
		for (u32 j = 0; j < base_layouts->placeholders_count; ++j) {
			Placeholder* placeholder = &base_layouts->placeholders[j];
			if (placeholder->should_request && units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
				u32 distance = scbw::get_distance_fast(main_town->position, resource_areas->areas[i].pos);
				if (placeholder->priority > best_priority || (placeholder->priority == best_priority && distance < best_distance)) {
					best_priority = placeholder->priority;
					best_distance = distance;
					best_resarea = i;
					break;
				}
			}
		}
	}
	return best_resarea;
}

void tick_base_layouts() {
	if (*elapsedTimeFrames == 0) {
		init_base_layouts();
	}

	// Deinit, Tick
	for (u32 i = 0; i < ResAreaLimit; ++i) {
		BaseLayouts* base_layouts = active_base_layouts[i];
		if (base_layouts) {
			AiTown* town = base_layouts->town;
			if (!town->inited) {
				base_layouts->town = 0;
				active_base_layouts[i] = 0;
				continue;
			}
			if (town->main_building && has_expanding_town(town->player)) {
				continue;
			}
			for (u32 j = 0; j < base_layouts->placeholders_count; ++j) {
				Placeholder* placeholder = &base_layouts->placeholders[j];
				if (placeholder->should_request) {
					if (!base_layouts->placeholders_requested[j] && can_build_placeholder(base_layouts, placeholder)) {
						base_layouts->placeholders_requested[j] = true;
						request_placeholder(base_layouts, placeholder);
					}
					if (base_layouts->placeholders_requested[j] &&
						units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building &&
						has_down_equivalent(town, placeholder->unit_id) &&
						number_of_equivalents_up(town, find_unit_base_merge(placeholder->unit_id)) < placeholder->wait_build_start_count) {
						// Lock adding new requests until this request is started
						break;
					}
				}
			}
		}
	}

	// Init
	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &AiTownArray[i];
		if (town->inited && !active_base_layouts[town->resarea]) {
			BaseLayouts* base_layouts = &resarea_base_layouts[town->player][town->resarea];

			BaseLayouts* mainarea_layouts = &mainarea_base_layouts[town->player];
			if (mainarea_layouts->placeholders_count) {
				for (u32 j = 0; j < mainarea_layouts->placeholders_count; ++j) {
					if (base_layouts->placeholders_count < PlaceholderLimit) {
						base_layouts->placeholders[base_layouts->placeholders_count++] = mainarea_layouts->placeholders[j];
					}
				}
				sort_base_layouts(base_layouts);
				memset(mainarea_layouts, 0, sizeof(BaseLayouts));
			}

			base_layouts->town = town;
			memset(base_layouts->placeholders_requested, 0, sizeof(base_layouts->placeholders_requested));
			memset(base_layouts->request_count, 0, sizeof(base_layouts->request_count));
			for (u32 j = 0; j < base_layouts->placeholders_count; ++j) {
				Placeholder* placeholder = &base_layouts->placeholders[j];
				init_placeholder(base_layouts, placeholder);
			}
			active_base_layouts[town->resarea] = base_layouts;
		}
	}
}
