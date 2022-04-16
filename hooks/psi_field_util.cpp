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
bool isValidPsiProviderType(u16 unitId)
{
	if (unitId >= UNIT_TYPE_COUNT)
	{
		return false;
	}
	// Uses memory space of Unit::building for other purposes
	if (units_dat::BaseProperty[unitId] & (UnitProperty::Worker | UnitProperty::ResourceContainer | UnitProperty::NPCOrAccessories))
	{
		return false;
	}
	// Unit::rally shares memory space with Unit::psi_link
	if (units_dat::GroupFlags[unitId].isFactory)
	{
		return false;
	}
	// Uses memory space of Unit::building for other purposes
	if (unitId == UnitId::TerranEidolon ||
		unitId == UnitId::ZergNydusCanal ||
		unitId == UnitId::TerranNuclearSilo ||
		unitId == UnitId::ZergHatchery ||
		unitId == UnitId::ZergLair ||
		unitId == UnitId::ZergHive)
	{
		return false;
	}
	return true;
}

// Adds @p unit to the psi provider list and creates the psi field sprite.
// Based on orders_InitPsiProvider() @ 00493F70
// But this does not affect the unit's current order.
void addPsiField(Unit* unit)
{
	if (!unit)
	{
		return;
	}
	if (*firstPsiFieldProvider == unit || unit->psi_link.next || unit->psi_link.prev)
	{
		return;
	}
	unit->psi_link.prev = nullptr;
	unit->psi_link.next = *firstPsiFieldProvider;
	if (*firstPsiFieldProvider)
	{
		(*firstPsiFieldProvider)->psi_link.prev = unit;
	}
	*firstPsiFieldProvider = unit;
	helper::addPsiFieldSprite(unit);
	*canUpdatePoweredStatus = true;
}

// Identical to function @ 0x00493100
void removeFromPsiProviderList(Unit* psiProvider)
{
	if (psiProvider->psi_link.prev)
		psiProvider->psi_link.prev->psi_link.next = psiProvider->psi_link.next;
	if (psiProvider->psi_link.next)
		psiProvider->psi_link.next->psi_link.prev = psiProvider->psi_link.prev;

	if (psiProvider == *firstPsiFieldProvider)
		*firstPsiFieldProvider = psiProvider->psi_link.next;

	psiProvider->psi_link.prev = nullptr;
	psiProvider->psi_link.next = nullptr;
}

// Removes @p unit from the psi provider list and destroys the psi field sprite.
void removePsiField(Unit* unit)
{
	if (unit->building.pylonAura)
	{
		unit->building.pylonAura->free();
		unit->building.pylonAura = nullptr;
		*canUpdatePoweredStatus = true;   // Might work?
	}
	removeFromPsiProviderList(unit);
}

// Synchronizes the unit's psi field sprite position with the unit.
void updatePsiFieldPosition(Unit* unit)
{
	if (!unit || !unit->sprite || !unit->building.pylonAura)
	{
		return;
	}

	Sprite* psiField = unit->building.pylonAura;
	if (unit->sprite->position != psiField->position)
	{
		psiField->setPosition(unit->getX(), unit->getY());
		helper::refreshSpriteData(psiField);
		*canUpdatePoweredStatus = true;
	}
}

// Defined in psi_field_inject.cpp
void hideAllPsiFields();

namespace hooks
{
	void updatePsiFieldProviders()
	{
		for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
		{
			if (hooks::canMakePsiField(unit->id))
			{
				assert(isValidPsiProviderType(unit->id));

				if (unit->status & UnitStatus::Completed)
				{
					if (hooks::isReadyToMakePsiField(unit))
					{
						addPsiField(unit);
						updatePsiFieldPosition(unit);
					}
					else
					{
						removePsiField(unit);
					}
				}
			}
		}

		if (!(*IS_PLACING_BUILDING))
		{
			Unit* selUnit;
			for (s32 i = 0; i < SELECTION_ARRAY_LENGTH; i++)
			{
				selUnit = clientSelectionGroup->unit[i];

				if (selUnit != nullptr && hooks::isReadyToMakePsiField(selUnit))
				{
					return;
				}
			}
			hideAllPsiFields();
		}
	}
}
