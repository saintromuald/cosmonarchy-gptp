#include "make_nydus_exit.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_build5(Unit* nydus_canal) // 0045DC20
	{
		// Fix coordinates if off-map (wasn't present in original function)
		s32 x = units_dat::building_dimensions[UnitId::ZergNydusCesiant].x;
		s32 y = units_dat::building_dimensions[UnitId::ZergNydusCesiant].y;
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
		if (helper::can_build_on_tile_location(nydus_canal, nydus_canal->player, x, y, UnitId::ZergNydusCesiant, 0, 0, 1, 0)) {
			helper::play_worker_error_sound(nydus_canal, 0x358); // "You can't build there"
			nydus_canal->order_to_idle();
		} else {
			scbw::prepare_building_area(nydus_canal->order_target.pt, UnitId::ZergNydusCesiant);
			Unit* nydus_exit = helper::create_unit(UnitId::ZergNydusCesiant, nydus_canal->order_target.pt.x, nydus_canal->order_target.pt.y, nydus_canal->player);
			if (nydus_exit) {
				helper::ai_training_overlord(nydus_exit, nydus_canal);
			}
			
			if (!nydus_exit) {
				helper::display_last_network_error_for_player(nydus_canal->player);
			} else {
				helper::init_building(nydus_exit, 1);
				// logically equivalent to a more complex original calculation
				nydus_exit->hitpoints = (10 * units_dat::max_hitpoints[nydus_exit->id]) / 100;
				nydus_exit->order_computer(OrderId::ZergBuildSelf);
				nydus_exit->current_buttonset = UnitId::Buttons_Blank;
				helper::stop_creep_disappearing_at_building(nydus_exit);
				nydus_canal->building.nydus_exit = nydus_exit;
				nydus_exit->building.nydus_exit = nydus_canal;
				nydus_exit->sprite->player = nydus_canal->sprite->player;

				UnitExt* exit_ext = get_unit_ext(nydus_exit);
				exit_ext->values[ValueIsNydusExit] = 1;
			}
			nydus_canal->order_to_idle();
		}
	}

	u32 adjust_flags(u32 flags, u32 player) {
		return 0xffffffff; // don't check any creep tile flags for Nydus placement
	}
}