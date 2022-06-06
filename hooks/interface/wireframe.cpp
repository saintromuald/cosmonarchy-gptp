#include "wireframe.h"
#include "../../SCBW/api.h"

namespace hooks
{
	// Was initially in building_morph. The rest of the function this fragment is from use too many unknowns, thus this is only a fragment.
	// This allows to change the wireframe displayed for the unit, the special case being that zerg morphing buildings show the wireframe of the new building before construction is complete.
	// -iquare
	u16 stat_data_unit_wireframe_update_fragment(Unit* unit)
	{
		u16 unit_id = UnitId::None;
		bool bUseCurrentUnitId = true;
		if (!(unit->status & UnitStatus::Completed))
		{
			unit_id = unit->build_queue[unit->build_queue_slot];
			if (scbw::is_zerg_building_morph_result(unit_id))
			{
				bUseCurrentUnitId = false;
			}
		}
		if (bUseCurrentUnitId)
		{
			unit_id = unit->id;
		}
		return unit_id;
	}

	u8 wireframe_shields(Unit* unit)
	{
		auto eax = unit->id;
		u8 al = 0;
		if (units_dat::ShieldsEnabled[unit->id])
		{
			auto lastSlot = scbw::get_last_queue_slot_id(unit);
			u32 maxShields = units_dat::MaxShieldPoints[lastSlot];
			if (lastSlot == unit->id)
			{
				maxShields = unit->max_shields(); // for stasis, later
			}
			if (maxShields)
			{
				u32 shields = (u32)unit->shields >> 8;
				shields *= 7;
				shields /= maxShields;
				if (shields * 2 + 1 >= 40)
				{
					return al; // safety check
				}
				auto wireConst = wireframeConstants[shields * 2];
				auto wireConstShift = wireframeConstants[(shields * 2) + 1];
				auto buf1 = charBuffer[wireConst];//CL
				al = charBuffer[wireConstShift];//AL
				*off_50CE81 = buf1;
				*off_50CE82 = al;
			}
		}
		else
		{
			al = *byteShieldStr;
			*off_50CE81 = al;
			*off_50CE82 = al;
		}
		return al;
	}
}