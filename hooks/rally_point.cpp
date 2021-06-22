//Contains hooks that control how rally points are used.

#include "rally_point.h"
#include "../SCBW/api.h"

namespace hooks
{
	// Orders newly-produced units to rally, based upon the properties of the building that produced it.
	// @param  unit      The unit that needs to receive rally orders.
	// @param  factory   The unit (building) that created the given unit.
	// Identical to sub_466F50 @ 00466F50

	void orderNewUnitToRallyHook(Unit* unit, Unit* factory)
	{

		// Do nothing if the rally target is the factory itself or the rally target position is 0
		if (factory != nullptr
			&& factory->isFactory()
			&& factory->rally.unit != factory
			&& factory->rally.pt.x != 0)
		{
			// Following should be allowed only on friendly units
			if (factory->rally.unit != nullptr && (factory->rally.unit)->playerId == unit->playerId)
				unit->orderTo(OrderId::Follow, factory->rally.unit);
			else
				unit->orderTo(OrderId::Move, factory->rally.pt.x, factory->rally.pt.y);
		}
	}

	// Called when the player sets the rally point on the ground.
	void setRallyPosition(Unit* unit, u16 x, u16 y)
	{
		unit->rally.unit = nullptr;
		unit->rally.pt.x = x;
		unit->rally.pt.y = y;
	}

	// Called when the player sets the rally point on a unit.
	void setRallyUnit(Unit* unit, Unit* target)
	{
		if (target == nullptr)
			target = unit;

		unit->rally.unit = target;
		unit->rally.pt.x = target->getX();
		unit->rally.pt.y = target->getY();
	}

    // Hooked to allow rally for morphing Larval Colonies - Pr0nogo
    // Possible downstream effect: need to clear rally for the creep colony if the morph cancelled?
    u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit) // 00429520
    {
        *lastInternalError = 0;
        if (orders_dat::RequirementsOffset[OrderId::RallyPoint2] == -1)
        {
            helper::parseOrdersDatReqs();
        }

        u32 result;
        if (player == (u8)unit->playerId)
        {
            if (unit->isFrozen())
            {
                *lastInternalError = 10;
                result = 0;
            }
            else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk) // ?
            {
                *lastInternalError = 11;
                result = 0;
            }
            else
            {
                u16 unitId = unit->id;
                if (unitId == UnitId::TerranMason && unit->mainOrderId == OrderId::ConstructingBuilding
                  || scbw::unitCanNuke(unit) && unit->mainOrderId == OrderId::NukeTrack
                  || unitId == UnitId::ProtossAugur && unit->mainOrderId == OrderId::CompletingArchonSummon)
                {
                    *lastInternalError = 8;
                    result = 0;
                }
                else
                {
                    result = orders_dat::RequirementsOffset[OrderId::RallyPoint2];
                    if (orders_dat::RequirementsOffset[OrderId::RallyPoint2])
                    {
                        result = helper::parseRequirementOpcodes(unit, OrderId::RallyPoint2, player, *ordersRequirementTree, orders_dat::RequirementsOffset[OrderId::RallyPoint2]);
                    }
                    else
                    {
                        *lastInternalError = 23;
                    }
                }
            }
        }
        else
        {
            *lastInternalError = 1;
            result = 0;
        }
        return result;
    }
}
