#include "trap_hooks.h"
#include "byte_operations_utils.h"

namespace hooks
{
	bool unit_is_floor_trap(Unit* unit) // 0047B070
	{
		return false;
	}

	bool unit_is_installation_asset(u16 id) // 0047B4B0
	{
		if (id >= UnitId::Special_UpperLevelDoor && id <= UnitId::Special_RightPitDoor)
		{
			return true;
		}
		return false;
	}

	s16 disable_trap(Unit* unit) // 0047B4B0
	{
		u32 status = unit->status;
		if (!unit->is_frozen())
		{
			unit->status = status | UnitStatus::Disabled;
			for (Image* image = unit->sprite->images.head; image; image = image->link.next)
			{
				helper::play_image_iscript(image, IscriptAnimation::AlmostBuilt);
			}
			u32 unit_id = unit->id;
			unit->status |= UnitStatus::NoCollide;

			if (unit_id == UnitId::Special_UpperLevelDoor
			||  unit_id == UnitId::Special_RightUpperLevelDoor
			||  unit_id == UnitId::Special_PitDoor
			||  unit_id == UnitId::Special_RightPitDoor)
			{
				unit->sprite->elevation = 1;
			}
			if (unit->id == UnitId::Special_FloorMissileTrap
			||  unit->id == UnitId::Special_FloorGunTrap
			||  unit->id == UnitId::Special_WallMissileTrap
			||  unit->id == UnitId::Special_WallFlameTrap
			||  unit->id == UnitId::Special_RightWallMissileTrap
			||  unit->id == UnitId::Special_RightWallFlameTrap)
			{
				unit->status |= UnitStatus::Cloaked | UnitStatus::RequiresDetection;
				unit->detection_status = 0x80000000;
				unit->secondary_order_timer = 0;
			}
		}
		return status;
	}
}