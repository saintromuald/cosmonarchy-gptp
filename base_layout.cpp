#include "base_layout.h"

static BaseLayouts all_base_layouts[PlayerLimit][ResAreaLimit];
static BaseLayouts* active_base_layouts[ResAreaLimit];

static void add_base_layout(Unit* unit, u32 player) {
	BaseLayout layout = {};
	layout.position = unit->position;
	layout.unit_id = unit->id;
	layout.priority = round(100.0 * unit->hitPoints / unit->getMaxHpRaw());

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

static void init_base_layouts() {
	memset(all_base_layouts, 0, sizeof(all_base_layouts));
	memset(active_base_layouts, 0, sizeof(active_base_layouts));
	Unit* next = 0;
	for (Unit* unit = *firstVisibleUnit; unit; unit = next) {
		next = unit->link.next;
		if (unit->playerId > PlayerLimit && unit->playerId != 11) {
			continue;
		}
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building) &&
			units_dat::ComputerAIInternal[unit->id] & 0x2) { // Don't become guard
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
		}
	}
}

void tick_base_layouts() {
	if (*elapsedTimeFrames == 0) {
		init_base_layouts();
	}

	for (u32 i = 0; i < ResAreaLimit; ++i) {
		BaseLayouts* base_layouts = active_base_layouts[i];
		if (base_layouts && !base_layouts->town->inited) {
			base_layouts->town = 0;
			active_base_layouts[i] = 0;
		}
	}

	for (u32 i = 0; i < AiTownLimit; ++i) {
		AiTown* town = &AiTownArray[i];
		if (town->inited && !town->resareaNotSet && !active_base_layouts[town->resarea]) {
			BaseLayouts* base_layouts = &all_base_layouts[town->player][town->resarea];
			base_layouts->town = town;
			active_base_layouts[town->resarea] = base_layouts;
			for (u32 j = 0; j < base_layouts->units_count; ++j) {
				BaseLayout* base_layout = &base_layouts->units[j];
				scbw::sendGPTP_aise_cmd((Unit*)base_layout, (Unit*)town, GptpId::SetGuard, town->player, 0, 0);
			}
			for (u32 j = 0; j < base_layouts->structures_count; ++j) {
				BaseLayout* base_layout = &base_layouts->structures[j];
				scbw::sendGPTP_aise_cmd((Unit*)base_layout, (Unit*)town, GptpId::SetBaseLayout, town->player, 0, 0);
			}
		}
	}
}
