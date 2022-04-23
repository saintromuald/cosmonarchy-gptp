#include "base_layouts.h"
#include "hooks/unit_stats/new_requirements.h"

static BaseLayouts all_base_layouts[PlayerLimit][ResAreaLimit];
static BaseLayouts* active_base_layouts[ResAreaLimit];

struct Equivalent {
	u32 from;
	u32 to;
};

Equivalent equivalents[] = {
	{UnitId::ZergHatchery, UnitId::ZergLair},
	{UnitId::ZergLair, UnitId::ZergHive},
	{UnitId::ZergHive, UnitId::ZergSire},

	{UnitId::ZergIrolIris, UnitId::ZergOthstolOviform},
	{UnitId::ZergOthstolOviform, UnitId::ZergAlaszilArbor},

	{UnitId::ZergQuazrokPool, UnitId::ZergLiivralPond},
	{UnitId::ZergHydrokDen, UnitId::ZergGorgralSwamp},
	{UnitId::ZergMuthrokSpire, UnitId::ZergVithrathHaunt},
	{UnitId::ZergMatravilNest, UnitId::ZergAlmakisAntre},
	{UnitId::ZergZorkizShroud, UnitId::ZergUltrokCavern},
	{UnitId::ZergGeszkathGrotto, UnitId::ZergAlkajChasm},

	{UnitId::ZergCreepColony, UnitId::ZergSunkenColony},
	{UnitId::ZergCreepColony, UnitId::ZergSporeColony},
	{UnitId::ZergCreepColony, UnitId::ZergLarvalColony},

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

u32 find_unit_equivalent(u32 unit_id) {
	for (u32 i = 0; i < ARRAY_SIZE(equivalents); ++i) {
		if (unit_id == equivalents[i].to) {
			return equivalents[i].from;
		}
	}
	return UnitId::None;
}

static void add_placeholder_by_unit_id(BaseLayouts* base_layouts, Placeholder placeholder, u32 unit_id) {
	u32 equivalent = find_unit_equivalent(unit_id);
	if (equivalent != UnitId::None) {
		add_placeholder_by_unit_id(base_layouts, placeholder, equivalent);
	}

	for (u32 i = 0; i < base_layouts->placeholders_count; ++i) {
		Placeholder* other = &base_layouts->placeholders[i];
		if (unit_id == base_layouts->placeholders[i].unit_id && placeholder.position.x == other->position.x && placeholder.position.y == other->position.y) {
			return;
		}
	}

	placeholder.unit_id = unit_id;
	if (base_layouts->placeholders_count < PlaceholderLimit) {
		base_layouts->placeholders[base_layouts->placeholders_count++] = placeholder;
	}
}

static void add_placeholder(Unit* unit, u32 player) {
	Placeholder placeholder = {};
	placeholder.position = unit->position;
	placeholder.priority = round(100.0 * unit->hitPoints / unit->getMaxHpRaw());
	if (!(unit->status & UnitStatus::Invincible)) {
		placeholder.should_request = true;
	}

	u32 res_area = scbw::get_closest_resarea(unit->position);
	BaseLayouts* base_layouts = &all_base_layouts[player][res_area];
	add_placeholder_by_unit_id(base_layouts, placeholder, unit->id);
}

static b32 is_tech_unitid(u32 unit_id) {
	if (!units_dat::GroupFlags[unit_id].isBuilding) {
		return false;
	}
	if (units_dat::GroupFlags[unit_id].isFactory) {
		return false;
	}
	if (units_dat::BaseProperty[unit_id] & UnitProperty::Attack) {
		return false;
	}
	if (units_dat::BaseProperty[unit_id] & UnitProperty::Addon) {
		return false;
	}
	if (scbw::unitIdIsResource(unit_id, true)) {
		return false;
	}
	if (unit_id == UnitId::TerranSupplyDepot || unit_id == UnitId::TerranBunker || unit_id == UnitId::TerranTreasury) {
		return false;
	}
	if (unit_id == UnitId::ProtossPylon || unit_id == UnitId::ProtossShieldBattery ||
		unit_id == UnitId::ProtossCrucible || unit_id == UnitId::ProtossPennant) {
		return false;
	}
	if (unit_id == UnitId::ZergCreepColony || unit_id == UnitId::ZergNydusCanal) {
		return false;
	}
	return true;
}

static u32 get_unit_class(u32 unit_id) {
	if (unit_id == UnitId::ProtossPylon) {
		return 3;
	}
	if (!(units_dat::BaseProperty[unit_id] & UnitProperty::Building)) {
		return 2;
	}
	if (units_dat::GroupFlags[unit_id].isFactory || is_tech_unitid(unit_id)) {
		return 1;
	}
	return 0;
} 

static u32 get_unit_cost(u32 unit_id) {
	return units_dat::MineralCost[unit_id] + units_dat::GasCost[unit_id];
}

static int compare_base_layout(void* context, const void* a, const void* b) {
	BaseLayouts* base_layouts = (BaseLayouts*)context;
	Placeholder* pa = (Placeholder*)a;
	Placeholder* pb = (Placeholder*)b;

	s32 priority_diff = pb->priority - pa->priority;
	if (priority_diff) return priority_diff;

	s32 class_diff = get_unit_class(pb->unit_id) - get_unit_class(pa->unit_id);
	if (class_diff) return class_diff;

	s32 cost_diff = get_unit_cost(pa->unit_id) - get_unit_cost(pb->unit_id);
	if (cost_diff) return cost_diff;

	s32 pos_diff = scbw::getDistanceFast(pa->position, base_layouts->resarea_pos) - scbw::getDistanceFast(pb->position, base_layouts->resarea_pos);
	if (pos_diff) return pos_diff;

	return 0;
}

static void init_base_layouts() {
	memset(all_base_layouts, 0, sizeof(all_base_layouts));
	memset(active_base_layouts, 0, sizeof(active_base_layouts));

	for (u32 i = 0; i < PlayerLimit; ++i) {
		for (u32 j = 0; j < ResAreaLimit; ++j) {
			all_base_layouts[i][j].resarea_pos = resource_areas->areas[j].pos;
		}
	}

	Unit* next = 0;
	for (Unit* unit = *firstVisibleUnit; unit; unit = next) {
		next = unit->link.next;
		if (unit->playerId > PlayerLimit && unit->playerId != 11) {
			continue;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building) && units_dat::ComputerAIInternal[unit->id] & 0x2) { // Don't become guard
			if (unit->status & UnitStatus::IsHallucination) {
				unit->userActionFlags |= 0x4;
				unit->remove();
			} else {
				unit->status &= ~UnitStatus::Invincible;
			}
			continue;
		}

		if (unit->playerId == 11) {
			if (unit->status & UnitStatus::IsHallucination) {
				for (u32 i = 0; i < 8; ++i) {
					add_placeholder(unit, i);
				}
			}
		} else {
			add_placeholder(unit, unit->playerId);
		}

		if (unit->status & UnitStatus::IsHallucination) {
			unit->userActionFlags |= 0x4;
			unit->remove();
		} else {
			unit->status &= ~UnitStatus::Invincible;
		}
	}

	for (u32 i = 0; i < PlayerLimit; ++i) {
		for (u32 j = 0; j < ResAreaLimit; ++j) {
			BaseLayouts* base_layouts = &all_base_layouts[i][j];
			qsort_s(base_layouts->placeholders, base_layouts->placeholders_count, sizeof(Placeholder), compare_base_layout, base_layouts);
			for (u32 k = 0; k < base_layouts->placeholders_count; ++k) {
				Placeholder* placeholder = &base_layouts->placeholders[k];
				placeholder->wait_build_start_count = base_layouts->request_count[placeholder->unit_id]++;
			}
		}
	}
}

bool can_build_placeholder(BaseLayouts* base_layouts, Placeholder* placeholder) {
	AiTown* town = base_layouts->town;
	if (!hooks::createUnitIdAllowed(placeholder->unit_id, town->player)) {
		return false;
	}
	if (Player::numberOfUnitsOfType(town->player, placeholder->unit_id) < placeholder->wait_build_start_count) {
		return false;
	}
	if (!town->mainBuilding && units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
		if (placeholder->unit_id != UnitId::TerranCommandCenter &&
			placeholder->unit_id != UnitId::ProtossNexus && 
			placeholder->unit_id != UnitId::ZergHatchery) {
			return false;
		}
	}
	return true;
}

void tick_base_layouts() {
	if (*elapsedTimeFrames == 0) {
		init_base_layouts();
	}

	// Deinit, Tick
	for (u32 i = 0; i < ResAreaLimit; ++i) {
		BaseLayouts* base_layouts = active_base_layouts[i];
		if (base_layouts) {
			if (!base_layouts->town->inited) {
				base_layouts->town = 0;
				active_base_layouts[i] = 0;
				continue;
			}
			AiTown* town = base_layouts->town;
			for (u32 j = 0; j < base_layouts->placeholders_count; ++j) {
				Placeholder* placeholder = &base_layouts->placeholders[j];
				if (!base_layouts->placeholders_requested[j] && can_build_placeholder(base_layouts, placeholder)) {
					base_layouts->placeholders_requested[j] = true;
					u32 count = ++base_layouts->request_count[placeholder->unit_id];
					if (units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
						b32 only_if_needed = is_tech_unitid(placeholder->unit_id);
						helper::Ai_Build(placeholder->unit_id, only_if_needed, only_if_needed ? 1 : count, town, placeholder->priority);
					} else {
						scbw::sendGPTP_aise_cmd((Unit*)placeholder, (Unit*)town, GptpId::SetGuard, town->player, 0, 0);
					}
				}
			}
		}
	}

	// Init
	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &AiTownArray[i];
		if (town->inited && !town->resareaNotSet && !active_base_layouts[town->resarea]) {
			BaseLayouts* base_layouts = &all_base_layouts[town->player][town->resarea];
			base_layouts->town = town;
			memset(base_layouts->placeholders_requested, 0, sizeof(base_layouts->placeholders_requested));
			memset(base_layouts->request_count, 0, sizeof(base_layouts->request_count));
			for (u32 j = 0; j < base_layouts->placeholders_count; ++j) {
				Placeholder* placeholder = &base_layouts->placeholders[j];
				base_layouts->placeholders_requested[j] = !placeholder->should_request;
				if (units_dat::BaseProperty[placeholder->unit_id] & UnitProperty::Building) {
					scbw::sendGPTP_aise_cmd((Unit*)placeholder, (Unit*)town, GptpId::SetBaseLayout, town->player, 0, 0);
				}
			}
			active_base_layouts[town->resarea] = base_layouts;
		}
	}
}
