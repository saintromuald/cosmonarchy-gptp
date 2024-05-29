#include "recall_spell.h"
#include <BW/api.h>
#include "globals.h"
#include "hooks/weapons/weapon_fire.h"

namespace hooks
{
	void orders_cast_tyranny(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		Unit* target = unit->order_target.unit;
		if (!target || unit->player == target->player || bw::is_allied_to(unit->player, target->player)) {
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
			unit->order_to_idle();
		} else {
			bw::decloak_passive_proc(unit);
			u16 tech_energy_cost = 0;
			if (!unit->main_order_state) {
				if (helper::spell_order(unit, TechId::Tyranny, &tech_energy_cost, weapons_dat::max_range[orders_dat::order_weapon_id[OrderId::CastTyranny]], 0x052E)) {
					// Start the channel
					bw::add_unit_overlay(unit, ImageId::MindTyrantTyrannyCastOverlay);
					unit->play_iscript_animation(IscriptAnimation::Unused2);
					unit->main_order_state = 1;
					unit->main_order_timer = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->spend_unit_energy(tech_energy_cost);
				}
			} else {
				// Cancel when way out of range or lost sight of target
				u32 distance = bw::distance_between(unit->position, target->position);
				if (distance > weapons_dat::max_range[orders_dat::order_weapon_id[OrderId::CastTyranny]] * 2 || !target->is_visible_to(unit->player)) {
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->order_to_idle();
					return;
				}

				// Wait for channel
				if (unit->main_order_timer) {
					return;
				}
				fire_bullet(unit, WeaponId::MindTyrantTyranny, {unit->order_target.pt, target});
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				unit->order_to_idle();
			}
		}
	}
}