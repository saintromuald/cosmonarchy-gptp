#include "sight_range.h"
#include <SCBW/api.h>

const s32 DEFAULT_SIGHT_RANGE = 4;
const s32 BLIND_SIGHT_RANGE = 2;
const s32 MAX_SIGHT_RANGE = 11;

namespace hooks
{
	/// Returns the modified sight distance of the unit, measured in matrices.
	/// StarCraft passes 1 for is_for_spellcasting when using Feedback, Mind Control, and Hallucination (but not when launching Nukes).
	/// Note: sight ranges cannot exceed 11, unless extended.
	/// Equivalent to getUpgradedSightRange @ 004E5B40  
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting)
	{
		u32 sight_range;

		// Check if the unit is a constructing building (exclude remorphing buildings)
		if  (unit->status & UnitStatus::GroundedBuilding
		&& !(unit->status & UnitStatus::Completed)
		&&  !unit->is_remorphing_building())
		{
			sight_range = DEFAULT_SIGHT_RANGE;
		}
		else if (!is_for_spellcasting && unit->is_blind) // Check if the unit is blinded (don't bother if this is for spellcasting)
		{
			sight_range = BLIND_SIGHT_RANGE;
		}
		else if (unit->status & UnitStatus::Burrowed && !scbw::unit_is_trap(unit)) {
			sight_range = units_dat::sight_range[unit->id] / 2;
		}
		else
		{
			sight_range = units_dat::sight_range[unit->id];
		}

		return sight_range;
	}

	// Hooked to implement vision advantage from height. -Pr0nogo
	void reveal_sight_at_location(s32 sight_range, u32 flags, s32 x_pos, s32 y_pos, bool is_air_unit) // 004806F0
	{
		u32 vision_flags = flags; // Dunno if it can be replaced with the arg only
		if (flags <= 0xFF)
		{
			SightRange* sight_vals = &g_sight_values[sight_range];
			u32 new_vision_flags = ~flags & ~(flags << 8);
			u32 relative_flags = flags;
			void (__fastcall * *callbacks)(s32, s32, s32, s32, s32, s32) = (void(__fastcall**)(s32, s32, s32, s32, s32, s32))g_sight_callbacks;
			if (is_air_unit)
			{
				callbacks = g_sight_callbacks[0].air;
			}
			else
			{
				u32 ground_height = helper::get_ground_height_at_px(y_pos, x_pos);
				u32 ground_height_vision_flags;
				if (ground_height == 2)
				{
					ground_height_vision_flags = vision_flags | 0x1000000;
				}
				else if (ground_height == 1)
				{
					ground_height_vision_flags = vision_flags | 0x5000000;
				}
				else
				{
					ground_height_vision_flags = vision_flags | 0x7000000;
				}
				relative_flags = ground_height_vision_flags;
			}
			s32 tile_x = x_pos / 32;
			s32 tile_y = y_pos / 32;
			s32 tile_sight_x_radius = sight_vals->tile_sight_width / 2;
			s32 tile_sight_height_radius = sight_vals->tile_sight_height;
			if (x_pos / 32 - tile_sight_x_radius < 0
			||  tile_x + tile_sight_x_radius >= g_map_tile_size->width
			||  tile_y - tile_sight_height_radius / 2 < 0
			||  tile_y + tile_sight_height_radius / 2 >= *g_map_tile_height)
			{
				callbacks += 2;
			}
			(*callbacks)(tile_x, y_pos / 32, (s32)sight_vals, (s32)*g_active_tile_array_int[tile_x + tile_y * g_map_tile_size->width], relative_flags, new_vision_flags);
		}
	}
}