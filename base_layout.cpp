#include "base_layout.h"
#include "hooks/unit_stats/new_requirements.h"

static BaseLayouts all_base_layouts[PlayerLimit][ResAreaLimit];
static BaseLayouts* active_base_layouts[ResAreaLimit];

static void add_base_layout(Unit* unit, u32 player) {
	BaseLayout layout = {};
	layout.position = unit->position;
	layout.unit_id = unit->id;
	layout.priority = round(100.0 * unit->hitPoints / unit->getMaxHpRaw());
	if (unit->status & UnitStatus::IsHallucination) {
		layout.is_placeholder = true;
	}
	if (!(unit->status & UnitStatus::Invincible)) {
		layout.should_request = true;
	}

	u32 res_area = scbw::get_closest_resarea(unit->position);
	BaseLayouts* base_layouts = &all_base_layouts[player][res_area];
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Building) {
		if (base_layouts->structures_count < BaseLayoutLimit) {
			base_layouts->structures[base_layouts->structures_count++] = layout;
		}
	} else {
		if (base_layouts->units_count < BaseLayoutLimit) {
			base_layouts->units[base_layouts->units_count++] = layout;
		}
	}
}

static int __cdecl compare_base_layout(const void* a, const void* b) {
	BaseLayout* bl_a = (BaseLayout*)a;
	BaseLayout* bl_b = (BaseLayout*)b;
	u32 cost_a = units_dat::MineralCost[bl_a->unit_id] + units_dat::GasCost[bl_a->unit_id];
	u32 cost_b = units_dat::MineralCost[bl_b->unit_id] + units_dat::GasCost[bl_b->unit_id];
	if (bl_a->unit_id == UnitId::ProtossPylon) {
		cost_a = 10000;
	}
	if (bl_b->unit_id == UnitId::ProtossPylon) {
		cost_b = 10000;
	}
	return cost_b - cost_a;
}

static void init_base_layouts() {
	memset(all_base_layouts, 0, sizeof(all_base_layouts));
	memset(active_base_layouts, 0, sizeof(active_base_layouts));
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
					add_base_layout(unit, i);
				}
			}
		} else {
			add_base_layout(unit, unit->playerId);
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
			qsort(all_base_layouts[i][j].units, all_base_layouts[i][j].units_count, sizeof(BaseLayout), compare_base_layout);
			qsort(all_base_layouts[i][j].structures, all_base_layouts[i][j].structures_count, sizeof(BaseLayout), compare_base_layout);
		}
	}
}

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

static void set_guard(BaseLayouts* base_layouts, AiTown* town, BaseLayout* base_layout) {
	u32 equivalent = find_unit_equivalent(base_layout->unit_id);
	if (equivalent != UnitId::None) {
		BaseLayout copy = *base_layout;
		copy.unit_id = equivalent;
		set_guard(base_layouts, town, &copy);
	}
	scbw::sendGPTP_aise_cmd((Unit*)base_layout, (Unit*)town, GptpId::SetGuard, town->player, 0, 0);
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

static void set_base_layout(BaseLayouts* base_layouts, AiTown* town, BaseLayout* base_layout) {
	u32 equivalent = find_unit_equivalent(base_layout->unit_id);
	if (equivalent != UnitId::None) {
		BaseLayout copy = *base_layout;
		copy.unit_id = equivalent;
		set_base_layout(base_layouts, town, &copy);
	}
	scbw::sendGPTP_aise_cmd((Unit*)base_layout, (Unit*)town, GptpId::SetBaseLayout, town->player, 0, 0);
	if (base_layout->should_request) {
		u32 count = ++base_layouts->preplaced_request_count[base_layout->unit_id];
		if (base_layout->is_placeholder) {
			count = ++base_layouts->request_count[base_layout->unit_id];
		}
		b32 only_if_needed = is_tech_unitid(base_layout->unit_id);
		helper::Ai_Build(base_layout->unit_id, only_if_needed, count, town, base_layout->priority);
	}
}

void tick_base_layouts() {
	if (*elapsedTimeFrames == 0) {
		init_base_layouts();
	}

	for (u32 i = 0; i < ResAreaLimit; ++i) {
		BaseLayouts* base_layouts = active_base_layouts[i];
		if (base_layouts) {
			if (!base_layouts->town->inited) {
				base_layouts->town = 0;
				active_base_layouts[i] = 0;
				continue;
			}
			AiTown* town = base_layouts->town;
			for (u32 j = 0; j < base_layouts->units_count; ++j) {
				BaseLayout* base_layout = &base_layouts->units[j];
				if (!base_layouts->units_active[j] && hooks::createUnitIdAllowed(base_layout->unit_id, town->player)) {
					base_layouts->units_active[j] = true;
					set_guard(base_layouts, town, base_layout);
				}
			}
			for (u32 j = 0; j < base_layouts->structures_count; ++j) {
				BaseLayout* base_layout = &base_layouts->structures[j];
				if (base_layout->unit_id == UnitId::ProtossGateway) {
					int i = 0;
				}
				if (!base_layouts->structures_active[j] && hooks::createUnitIdAllowed(base_layout->unit_id, town->player)) {
					base_layouts->structures_active[j] = true;
					set_base_layout(base_layouts, town, base_layout);
				}
			}
		}
	}

	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &AiTownArray[i];
		if (town->inited && !town->resareaNotSet && !active_base_layouts[town->resarea]) {
			BaseLayouts* base_layouts = &all_base_layouts[town->player][town->resarea];
			base_layouts->town = town;
			memset(base_layouts->units_active, 0, sizeof(base_layouts->units_active));
			memset(base_layouts->structures_active, 0, sizeof(base_layouts->structures_active));
			memset(base_layouts->preplaced_request_count, 0, sizeof(base_layouts->preplaced_request_count));
			memset(base_layouts->request_count, 0, sizeof(base_layouts->request_count));
			active_base_layouts[town->resarea] = base_layouts;
		}
	}
}
