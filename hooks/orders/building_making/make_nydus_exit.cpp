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
		if (x < 0) {
			x++;
		}
		if (y < 0) {
			y++;
		}
		x = nydus_canal->order_target.pt.x - (x / 2);
		y = nydus_canal->order_target.pt.y - (y / 2);
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
		if (helper::can_build_on_tile_location(nydus_canal, nydus_canal->player, x, y, UnitId::ZergNydusCanal, 0, 0, 1, 0)) {
			helper::play_worker_error_sound(nydus_canal, 0x358); // "You can't build there"
			nydus_canal->order_to_idle();
		} else { // 0045DCDB
			Unit* nydus_exit = helper::create_unit(UnitId::ZergNydusCanal, nydus_canal->order_target.pt.x, nydus_canal->order_target.pt.y, nydus_canal->player);
			if (nydus_exit) {
				helper::ai_training_overlord(nydus_exit, nydus_canal);
			}
			
			if (!nydus_exit) {
				helper::display_last_network_error_for_player(nydus_canal->player);
			} else if (nydus_exit->id != UnitId::ZergNydusCanal) {
				// TODO: should probably remove, since we are already handling CCMU properly here
				// we should test first though
				helper::display_last_network_error_for_player(nydus_canal->player);
				nydus_exit->remove();
			} else {
				helper::init_building(nydus_exit, 1);
				// logically equivalent to a more complex original calculation
				nydus_exit->hitpoints = (10 * units_dat::MaxHitPoints[nydus_exit->id]) / 100;
				nydus_exit->order_computer(OrderId::ZergBuildSelf);
				nydus_exit->current_button_set = UnitId::Buttons_Blank;
				helper::stopCreepDisappearingAtBuilding(nydus_exit);
				nydus_canal->building.nydus_exit = nydus_exit;
				nydus_exit->building.nydus_exit = nydus_canal;
			}
			nydus_canal->order_to_idle();
		}
	}

	u32 adjustFlags(u32 flags, u32 player) {
		return 0xffffffff; // don't check any creep tile flags for nydus placement
	}
}