#include "wireframe.h"
#include "../../SCBW/api.h"

namespace hooks
{

	//was initially in a building_morph file
	//the rest of the function this fragment is from use
	//too many unknowns, thus this is only a fragment.
	//This allow to change the wireframe displayed for
	//the unit, the special case being that zerg morphing
	//buildings show the wireframe of the new building
	//before construction is complete
	u16 statdata_UnitWireframeUpdate_Fragment(Unit* unit)
	{

		u16 unit_id;
		bool bUseCurrentUnitId = true;

		if (!(unit->status & UnitStatus::Completed))
		{

			unit_id = unit->build_queue[unit->build_queue_slot];

			if (
				unit_id == UnitId::ZergHive ||
				unit_id == UnitId::ZergLair ||
				unit_id == UnitId::ZergGeszithRoost ||
				unit_id == UnitId::ZergSporeColony ||
				unit_id == UnitId::ZergSunkenColony ||
				unit_id == 191
				)
				bUseCurrentUnitId = false;

		}

		//5694F
		if (bUseCurrentUnitId)
			unit_id = unit->id;

		return unit_id;
	}
	u16 get_last_queue_slot_id(Unit* unit)
	{
		u16 unit_id;
		if (unit->status & UnitStatus::Completed)
			unit_id = unit->id;
		else
		{
			unit_id = unit->build_queue[unit->build_queue_slot];
			if (!(scbw::is_zerg_building_morph_result))
				unit_id = unit->id;
		}
		return unit_id;
	};

	u8 wireframeShields(Unit* unit)
	{
		auto eax = unit->id;
		u8 al = 0;
		if (units_dat::ShieldsEnabled[unit->id])
		{
			auto lastSlot = scbw::get_last_queue_slot_id(unit);
			u32 maxShields = units_dat::MaxShieldPoints[lastSlot];
			if (lastSlot == unit->id)
			{
				maxShields = unit->maxShields();//for stasis, later
			}
			if (maxShields)
			{
				u32 shields = (u32)unit->shields >> 8;
				shields *= 7;
				shields /= maxShields;
				if (shields * 2 + 1 >= 40)
					return al; // safety check
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
} //hooks
