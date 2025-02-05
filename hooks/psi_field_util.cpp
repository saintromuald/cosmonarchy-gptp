// All functions in this file are meant to be hook helpers for psi_field.h.
// Do NOT modify any functions in this file!

#include "psi_field.h"
#include "../BW/bwdata.h"
#include <algorithm>
#include <cassert>
#include "CM/cm_ids.h"
#include "BW/api.h"

// Adds @p unit to the psi provider list and creates the psi field sprite.
// Based on orders_init_psi_provider() @ 00493F70
// This does not affect the unit's current order.
void add_psi_field(Unit* unit) {
	if (!unit) {
		return;
	}
	UnitExt& unit_ext = unit->ext();
	if (*g_first_psi_field_provider == unit || unit_ext.psi_link.next || unit_ext.psi_link.prev) {
		return;
	}
	unit_ext.psi_link.prev = nullptr;
	unit_ext.psi_link.next = *g_first_psi_field_provider;
	if (*g_first_psi_field_provider) {
		UnitExt& provider_ext = (*g_first_psi_field_provider)->ext();
		provider_ext.psi_link.prev = unit;
	}
	*g_first_psi_field_provider = unit;
	helper::add_psi_field_sprite(unit);
	g_can_update_powered_status = true;
}

// Identical to function @ 0x00493100
void remove_from_psi_providers_list(Unit* psi_provider) {
	UnitExt& provider_ext = psi_provider->ext();
	if (provider_ext.psi_link.prev) {
		UnitExt& prev_ext = provider_ext.psi_link.prev->ext();
		prev_ext.psi_link.next = provider_ext.psi_link.next;
	}
	if (provider_ext.psi_link.next) {
		UnitExt& next_ext = provider_ext.psi_link.next->ext();
		next_ext.psi_link.prev = provider_ext.psi_link.prev;
	}
	if (psi_provider == *g_first_psi_field_provider) {
		*g_first_psi_field_provider = provider_ext.psi_link.next;
	}
	provider_ext.psi_link.prev = nullptr;
	provider_ext.psi_link.next = nullptr;
}

// Removes @p unit from the psi provider list and destroys the psi field sprite.
void remove_psi_field(Unit* unit) {
	UnitExt& unit_ext = unit->ext();
	if (unit_ext.power_aura) {
		unit_ext.power_aura->free();
		unit_ext.power_aura = nullptr;
		g_can_update_powered_status = true;   // Might work?
	}
	remove_from_psi_providers_list(unit);
}

// Synchronizes the unit's psi field sprite position with the unit.
void update_psi_field_position(Unit* unit) {
	if (!unit || !unit->sprite) {
		return;
	}
	UnitExt& unit_ext = unit->ext();
	if (!unit_ext.power_aura) {
		return;
	}
	Sprite* psi_field = unit_ext.power_aura;
	if (unit->sprite->position != psi_field->position) {
		psi_field->set_position(unit->get_x(), unit->get_y());
		helper::refresh_sprite_data(psi_field);
		g_can_update_powered_status = true;
	}
}

// Defined in psi_field_inject.cpp
void hooks::hide_all_psi_fields();

namespace hooks {
	void update_psi_field_providers() {
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			 if (unit_id_is_psi_provider(unit->id) && unit->is_completed()) {
				if (hooks::is_ready_to_make_psi_field(unit)) {
					add_psi_field(unit);
					update_psi_field_position(unit);
				} else {
					remove_psi_field(unit);
				}
			}
		}

		if (!g_is_placing_building) {
			Unit* selected_unit;
			for (s32 i = 0; i < SelectionCap; i++) {
				selected_unit = g_client_selection_group->unit[i];
				if (selected_unit != nullptr && hooks::is_ready_to_make_psi_field(selected_unit)) {
					return;
				}
			}
			hide_all_psi_fields();
		}
	}
}
