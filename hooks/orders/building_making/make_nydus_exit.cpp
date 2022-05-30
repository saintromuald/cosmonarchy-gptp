#include "make_nydus_exit.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_Build5(Unit* nydus_canal) // 0045DC20
	{
		// Fix coordinates if off-map (wasn't present in original function)
		s32 x, y;
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
		if (helper::can_build_on_tile_location(nydus_canal, nydus_canal->player, x, y, UnitId::ZergNydusCanal, 0, 0, 1, 0))
		{
			helper::playWorkerErrorSound(nydus_canal, 0x358); // "You can't build there"
			if (nydus_canal->orderQueueHead != nullptr)
			{
				nydus_canal->userActionFlags |= 1;
				prepareForNextOrder(nydus_canal);
			}
			else if (nydus_canal->unit_ai != nullptr)
			{
				nydus_canal->order_computer(OrderId::ComputerAI);
			}
			else
			{
				nydus_canal->order_computer(units_dat::ReturnToIdleOrder[nydus_canal->id]);
			}
		}
		else
		{ // 0045DCDB
			Unit* nydusExit = helper::create_unit(UnitId::ZergNydusCanal, nydus_canal->orderTarget.pt.x, nydus_canal->orderTarget.pt.y, nydus_canal->player);
			if (nydusExit)
			{
				helper::ai_training_overlord(nydusExit, nydus_canal);
			}
			
			if (nydusExit == nullptr)
			{
				helper::display_last_network_error_for_player(nydus_canal->player);
			}
			else if (nydusExit->id != UnitId::ZergNydusCanal)
			{
				// TODO: should probably remove, since we are already handling CCMU properly here
				// we should test first though
				helper::display_last_network_error_for_player(nydus_canal->player);
				nydusExit->remove();
			}
			else
			{
				helper::init_building(nydusExit, 1);
				// logically equivalent to a more complex original calculation
				nydusExit->hitPoints = (10 * units_dat::MaxHitPoints[nydusExit->id]) / 100;
				nydusExit->order_computer(OrderId::ZergBuildSelf);
				nydusExit->currentButtonSet = UnitId::Buttons_Blank;
				helper::stopCreepDisappearingAtBuilding(nydusExit);
				nydus_canal->building.nydusExit = nydusExit;
				nydusExit->building.nydusExit = nydus_canal;
			}
			nydus_canal->order_to_idle();
		}
	}

	u32 adjustFlags(u32 flags, u32 player)
	{
		return 0xffffffff; // don't check any creep tile flags for nydus placement
	}
}