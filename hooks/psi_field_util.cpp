// All functions in this file are meant to be hook helpers for psi_field.h.
// Do NOT modify any functions in this file!

#include "psi_field.h"
#include "../SCBW/scbwdata.h"
#include <algorithm>
#include <cassert>

// NOT FULLY REVALIDATED BY UNDEADSTAR

//-------- Unit id safeguard --------//

// Checks if the given unit ID can provide psi (for use in assertion)
// The following unit types cannot make psi fields:
// worker, factories, resource containers, ghosts, nydus canals, silos, hatcheries, and powerups
bool is_valid_psi_provider_type(u16 unit_id)
{
	if (unit_id >= UNIT_TYPE_COUNT)
	{
		return false;
	}
	// Uses memory space of Unit::building for other purposes
	if (units_dat::base_property[unit_id] & (UnitProperty::Worker | UnitProperty::ResourceContainer | UnitProperty::NPCOrAccessories))
	{
		return false;
	}
	// Unit::rally shares memory space with Unit::psi_link
	if (units_dat::group_flags[unit_id].is_factory)
	{
		return false;
	}

	// Rally is now in UnitExt to allow factories to provide psi

	// Uses memory space of Unit::building for other purposes
	if (unit_id == UnitId::TerranEidolon
		|| unit_id == UnitId::ZergNydusCanal
		|| unit_id == UnitId::TerranMissileSilo
		|| unit_id == UnitId::ZergHatchery
		|| unit_id == UnitId::ZergLair
		|| unit_id == UnitId::ZergHive)
	{
		return false;
	}
	return true;
}

// Adds @p unit to the psi provider list and creates the psi field sprite.
// Based on orders_InitPsiProvider() @ 00493F70
// This does not affect the unit's current order.
void add_psi_field(Unit* unit)
{
	if (!unit)
	{
		return;
	}
	if (*first_psi_field_provider == unit || unit->psi_link.next || unit->psi_link.prev)
	{
		return;
	}
	unit->psi_link.prev = nullptr;
	unit->psi_link.next = *first_psi_field_provider;
	if (*first_psi_field_provider)
	{
		(*first_psi_field_provider)->psi_link.prev = unit;
	}
	*first_psi_field_provider = unit;
	helper::add_psi_field_sprite(unit);
	*can_update_powered_status = true;
}

// Identical to function @ 0x00493100
void remove_from_psi_providers_list(Unit* psiProvider)
{
	if (psiProvider->psi_link.prev)
		psiProvider->psi_link.prev->psi_link.next = psiProvider->psi_link.next;
	if (psiProvider->psi_link.next)
		psiProvider->psi_link.next->psi_link.prev = psiProvider->psi_link.prev;

	if (psiProvider == *first_psi_field_provider)
		*first_psi_field_provider = psiProvider->psi_link.next;

	psiProvider->psi_link.prev = nullptr;
	psiProvider->psi_link.next = nullptr;
}

// Removes @p unit from the psi provider list and destroys the psi field sprite.
void remove_psi_field(Unit* unit)
{
	if (unit->building.pylon_aura)
	{
		unit->building.pylon_aura->free();
		unit->building.pylon_aura = nullptr;
		*can_update_powered_status = true;   // Might work?
	}
	remove_from_psi_providers_list(unit);
}

// Synchronizes the unit's psi field sprite position with the unit.
void update_psi_field_position(Unit* unit)
{
	if (!unit || !unit->sprite || !unit->building.pylon_aura)
	{
		return;
	}

	Sprite* psiField = unit->building.pylon_aura;
	if (unit->sprite->position != psiField->position)
	{
		psiField->set_position(unit->get_x(), unit->get_y());
		helper::refresh_sprite_data(psiField);
		*can_update_powered_status = true;
	}
}

// Defined in psi_field_inject.cpp
void hide_all_psi_fields();

namespace hooks
{
	void update_psi_field_providers()
	{
		for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
		{
			if (hooks::can_make_psi_field(unit->id))
			{
				assert(is_valid_psi_provider_type(unit->id));

				if (unit->status & UnitStatus::Completed)
				{
					if (hooks::is_ready_to_make_psi_field(unit))
					{
						add_psi_field(unit);
						update_psi_field_position(unit);
					}
					else
					{
						remove_psi_field(unit);
					}
				}
			}
		}

		if (!(*is_placing_building))
		{
			Unit* selUnit;
			for (s32 i = 0; i < SelectionCap; i++)
			{
				selUnit = client_selection_group->unit[i];

				if (selUnit != nullptr && hooks::is_ready_to_make_psi_field(selUnit))
				{
					return;
				}
			}
			hide_all_psi_fields();
		}
	}
}
