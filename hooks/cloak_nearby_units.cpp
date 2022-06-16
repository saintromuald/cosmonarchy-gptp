#include "cloak_nearby_units.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	//Cloak all units near @p cloaker.
	// 1
	void cloak_nearby_units(Unit* cloaker) {
		bool needs_refresh = false;
		static u16* const max_box_right_value  = (u16*)0x00628450; // should usually be map_tile_size->width * 32
		static u16* const max_bot_bottom_value = (u16*)0x006284B4; // should usually be map_tile_size->height * 32

		// Use the unit's air weapon distance
		u32 cloak_radius = cloaker->get_max_weapon_range(cloaker->get_air_weapon());
		Box16 area_of_effect = {};
		area_of_effect.left   = cloaker->sprite->position.x - cloak_radius;
		area_of_effect.right  = cloaker->sprite->position.x + cloak_radius;
		area_of_effect.top    = cloaker->sprite->position.y - cloak_radius;
		area_of_effect.bottom = cloaker->sprite->position.y + cloak_radius;

		if (area_of_effect.left < 0) {
			area_of_effect.left = 0;
		}
		if (area_of_effect.top < 0) {
			area_of_effect.top = 0;
		}
		if (area_of_effect.right > *max_box_right_value) {
			area_of_effect.right = *max_box_right_value;
		}
		if (area_of_effect.bottom > *max_bot_bottom_value) {
			area_of_effect.bottom = *max_bot_bottom_value;
		}

		Unit** units_in_area_of_effect = helper::get_all_units_in_bounds(&area_of_effect);
		Unit* current_unit = *units_in_area_of_effect;
		while (current_unit) {
			UnitExt* unit_ext = globals.get_unit_ext(current_unit);
			if (current_unit != cloaker																	// Don't cloak self
			&& !(units_dat::base_property[current_unit->id] & UnitProperty::NeutralAccessories)		// Don't cloak items
			&& current_unit->id != UnitId::TerranNuclearMissile										// Don't cloak nukes
			&& current_unit->id != UnitId::ResourceMineralField										// Don't cloak resources
			&& current_unit->id != UnitId::ResourceMineralFieldType2								//
			&& current_unit->id != UnitId::ResourceMineralFieldType3								//
			&& current_unit->id != UnitId::ResourceVespeneGeyser									//
			&& current_unit->main_order_id != OrderId::Warpin										// Don't cloak units doing order WarpIn
			&& scbw::is_allied_to(current_unit->player, cloaker->player)							// Only cloak allied units
			&& current_unit->player != 11															// Don't cloak neutral units
			&& cloaker->is_target_within_min_range(current_unit, cloak_radius)						// Check again if unit in distance of cloaking
			&& !current_unit->is_frozen()) {
				if (!unit_ext->timers[TimerDecloak]) {
					helper::secondary_order_cloak(current_unit);
					if (!(current_unit->status & UnitStatus::CloakingForFree)) {
						current_unit->status |= UnitStatus::CloakingForFree;
						needs_refresh = true;
					}
				}
			}
			units_in_area_of_effect++;
			current_unit = *units_in_area_of_effect;
		}
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[*tempUnitsListArraysCountsListLastIndex];
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
		if (needs_refresh) {
			scbw::refresh_console();
		}
	}
}