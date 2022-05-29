#include "cloak_nearby_units.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	//Cloak all units near @p cloaker.
	// 1
	void cloakNearbyUnitsHook(Unit* cloaker)
	{
		bool needsRefresh = false;
		static u16* const maxBoxRightValue = (u16*)0x00628450;	//should usually be mapTileSize->width * 32
		static u16* const maxBoxBottomValue = (u16*)0x006284B4;	//should usually be mapTileSize->height * 32
		Box16 area_of_effect;
		Unit** unitsInAreaOfEffect;
		Unit* current_unit;

		// Use the unit's air weapon range
		u32 cloakRadius = cloaker->getMaxWeaponRange(cloaker->getAirWeapon());
		area_of_effect.left = cloaker->sprite->position.x - cloakRadius;
		area_of_effect.right = cloaker->sprite->position.x + cloakRadius;
		area_of_effect.top = cloaker->sprite->position.y - cloakRadius;
		area_of_effect.bottom = cloaker->sprite->position.y + cloakRadius;

		if (area_of_effect.left < 0)
		{
			area_of_effect.left = 0;
		}
		if (area_of_effect.top < 0)
		{
			area_of_effect.top = 0;
		}
		if (area_of_effect.right > *maxBoxRightValue)
		{
			area_of_effect.right = *maxBoxRightValue;
		}
		if (area_of_effect.bottom > *maxBoxBottomValue)
		{
			area_of_effect.bottom = *maxBoxBottomValue;
		}

		unitsInAreaOfEffect = helper::getAllUnitsInBounds(&area_of_effect);
		current_unit = *unitsInAreaOfEffect;
		while (current_unit != nullptr)
		{
			UnitExt* unitExt = globals.get_unit_ext(current_unit);

			if (current_unit != cloaker																	// Don't cloak self
				&& !(units_dat::BaseProperty[current_unit->id] & UnitProperty::NeutralAccessories)		// Don't cloak items
				&& current_unit->id != UnitId::TerranNuclearMissile										// Don't cloak nukes
				&& current_unit->id != UnitId::ResourceMineralField										// Don't cloak resources
				&& current_unit->id != UnitId::ResourceMineralFieldType2								//
				&& current_unit->id != UnitId::ResourceMineralFieldType3								//
				&& current_unit->id != UnitId::ResourceVespeneGeyser									//
				&& current_unit->mainOrderId != OrderId::Warpin											// Don't cloak units doing order WarpIn
				&& scbw::isAlliedTo(current_unit->player, cloaker->player)							// Only cloak allied units
				&& current_unit->player != 11															// Don't cloak neutral units
				&& cloaker->isTargetWithinMinRange(current_unit, cloakRadius)							// Check again if unit in range of cloaking
				&& !current_unit->isFrozen())
			{
				if (!unitExt->timers[TimerDecloak])
				{
					helper::secondaryOrder_Cloak(current_unit);
				}
				if (!(current_unit->status & UnitStatus::CloakingForFree))
				{
					current_unit->status |= UnitStatus::CloakingForFree;
					needsRefresh = true;
				}
			}
			unitsInAreaOfEffect++;
			current_unit = *unitsInAreaOfEffect;
		}
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[*tempUnitsListArraysCountsListLastIndex];
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
		if (needsRefresh)
		{
			scbw::refresh_console();
		}
	}
}