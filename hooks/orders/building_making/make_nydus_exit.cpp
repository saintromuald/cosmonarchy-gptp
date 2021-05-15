#include "make_nydus_exit.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_Build5(Unit* nydus_canal)
	{
		s32 x, y;
		u32 result_f00473FB0;
		x = units_dat::BuildingDimensions[UnitId::ZergNydusCanal].x;
		y = units_dat::BuildingDimensions[UnitId::ZergNydusCanal].y;
		if (x < 0)
		{
			x++;
		}
		if (y < 0)
		{
			y++;
		}
		x = nydus_canal->orderTarget.pt.x - (x / 2);
		y = nydus_canal->orderTarget.pt.y - (y / 2);
		if (x < 0)
		{
			x += 31;
		}
		if (y < 0)
		{
			y += 31;
		}
		x /= 32;
		y /= 32;

		// check if the nydus exit can be placed
		result_f00473FB0 = helper::canBuildOnTileLocation(nydus_canal, nydus_canal->playerId, x, y, UnitId::ZergNydusCanal, 0, 0, 1, 0);
		if (result_f00473FB0 != 0)
		{
			helper::playWorkerErrorSound(nydus_canal, 0x358); // "You can't build there"
			if (nydus_canal->orderQueueHead != nullptr)
			{
				nydus_canal->userActionFlags |= 1;
				prepareForNextOrder(nydus_canal);
			}
			else
				if (nydus_canal->pAI != nullptr)
				{
					nydus_canal->orderComputerClear(OrderId::ComputerAI);
				}
				else
				{
					nydus_canal->orderComputerClear(units_dat::ReturnToIdleOrder[nydus_canal->id]);
				}
		}
		else
		{ // 0045DCDB

			Unit* nydusExit = helper::createUnit(UnitId::ZergNydusCanal, nydus_canal->orderTarget.pt.x, nydus_canal->orderTarget.pt.y, nydus_canal->playerId);
			if (nydusExit == nullptr)
			{
				helper::displayLastNetErrorForPlayer(nydus_canal->playerId);
				nydus_canal->orderToIdle();
			}
			else if (nydusExit->id != UnitId::ZergNydusCanal)
			{
				helper::displayLastNetErrorForPlayer(nydus_canal->playerId);
				nydus_canal->orderToIdle();
				nydusExit->remove();
			}
			else
			{
				helper::initBuilding(nydusExit, 1);
				// logically equivalent to a more complex original calculation
				nydusExit->hitPoints = (10 * units_dat::MaxHitPoints[nydusExit->id]) / 100;
				nydusExit->orderComputerClear(OrderId::ZergBuildSelf);
				nydusExit->currentButtonSet = UnitId::Buttons_Blank;
				helper::stopCreepDisappearingAtBuilding(nydusExit);
				nydus_canal->building.nydusExit = nydusExit;
				nydusExit->building.nydusExit = nydus_canal;
				nydus_canal->orderToIdle();
			}
		}
	}

	u32 adjustFlags(u32 flags, u32 player)
	{
		return 0xffffffff; // don't check any creep tile flags for nydus placement
	}
}