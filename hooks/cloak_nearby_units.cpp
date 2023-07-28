#include "cloak_nearby_units.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	//Cloak all units near @p cloaker.
	void cloak_nearby_units(Unit* cloaker) {
		bool needs_refresh = false;
		u32 cloak_radius = 4 * 32;
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
		if (area_of_effect.right > *g_map_width_pixels) {
			area_of_effect.right = *g_map_width_pixels;
		}
		if (area_of_effect.bottom > *g_map_height_pixels) {
			area_of_effect.bottom = *g_map_height_pixels;
		}

		Unit** units_in_area_of_effect = helper::get_all_units_in_bounds(&area_of_effect);
		Unit* current_unit = *units_in_area_of_effect;
		while (current_unit) {
			UnitExt* unit_ext = get_unit_ext(current_unit);
			if  (current_unit != cloaker															// Don't cloak self
			&& !(units_dat::base_property[current_unit->id] & UnitProperty::NeutralAccessories)		// Don't cloak items
			&&   current_unit->id != UnitId::TerranMorningstar									// Don't cloak nukes
			&&   current_unit->main_order_id != OrderId::Warpin										// Don't cloak warping units
			&&   scbw::are_mutual_allies(current_unit->player, cloaker->player)						// Only cloak allied units
			&&   current_unit->player != 11															// Don't cloak neutral units
			&&   cloaker->is_target_within_min_range(current_unit, cloak_radius)					// Check again if unit in distance of cloaking
			&&  !current_unit->is_frozen())
			{
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
		*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list_last_index[*g_temp_units_list_arrays_counts_list_last_index];
		*g_temp_units_list_arrays_counts_list_last_index = *g_temp_units_list_arrays_counts_list_last_index - 1;
		if (needs_refresh) {
			scbw::refresh_console();
		}
	}
}