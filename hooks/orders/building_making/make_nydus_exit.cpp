#include "make_nydus_exit.h"
#include <BW/api.h>

namespace hooks {
    void orders_build5(Unit* nydus) { // 0045DC20
        // Fix coordinates if off-map (wasn't present in original function)
        s32 x = units_dat::building_dimensions[nydus->id].x;
        s32 y = units_dat::building_dimensions[nydus->id].y;
        if (x < 0) {
            x++;
        }
        if (y < 0) {
            y++;
        }

        x = nydus->order_target.pt.x - (x / 2);
        y = nydus->order_target.pt.y - (y / 2);
        if (x < 0) {
            x += 31;
        }
        if (y < 0) {
            y += 31;
        }
        x /= 32;
        y /= 32;

        // check if the nydus exit can be placed
        if (helper::tile_location_is_unbuildable(nydus, nydus->player, x, y, nydus->id, 0, 0, 1, 0)) {
            helper::play_worker_error_sound(nydus, 0x358); // "You can't build there"
            nydus->order_to_idle();
            return;
        }

        bw::prepare_building_area(nydus->order_target.pt, nydus->id);
        Unit* nydus_exit = helper::create_unit(nydus->id, nydus->order_target.pt.x, nydus->order_target.pt.y, nydus->player);
        if (nydus_exit) {
            helper::ai_training_overlord(nydus_exit, nydus);
        }		
        if (!nydus_exit) {
            helper::display_last_network_error_for_player(nydus->player);
            return;
        }
        helper::unit_completion_init(nydus_exit, 1);

        // logically equivalent to a more complex original calculation
        nydus_exit->hitpoints = (10 * units_dat::max_hitpoints[nydus_exit->id]) / 100;
        nydus_exit->order_computer(OrderId::ZergBuildSelf);
        nydus_exit->current_buttonset = UnitId::Buttons_Blank;
        if (units_dat::base_property[nydus->id] & UnitProperty::CreepBuilding) {
            helper::stop_creep_disappearing_at_building(nydus_exit);
        }
        nydus->building.nydus_exit = nydus_exit;
        nydus_exit->building.nydus_exit = nydus;
        nydus_exit->sprite->player = nydus->sprite->player;
        nydus->ext().values[ValueIsNydusEntry] = true;
        nydus_exit->ext().values[ValueIsNydusExit] = true;
        nydus->order_to_idle();
    }

    u32 adjust_flags(u32 flags, u32 player) {
        return 0xffffffff; // don't check any creep tile flags for Nydus placement
    }
} // namespace hooks