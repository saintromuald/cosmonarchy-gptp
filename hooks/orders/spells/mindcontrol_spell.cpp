#include "recall_spell.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks/weapons/weapon_fire.h"

namespace hooks
{
	void orders_cast_tyranny(Unit* unit) {
		bw::decloak_passive_proc(unit);
		UnitExt* unit_ext = get_unit_ext(unit);
		Unit* target = unit->order_target.unit;
		if (!target || unit->player == target->player || bw::is_allied_to(unit->player, target->player)) {
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
			unit->order_to_idle();
		} else {
			u16 tech_energy_cost = 0;
			if (!unit->main_order_state) {
				if (helper::spell_order(unit, TechId::Tyranny, &tech_energy_cost, unit->get_sight_range(true) * 32, 0x052E)) {
					// Start the channel
					unit->play_iscript_animation(IscriptAnimation::Unused2);
					unit->main_order_state = 1;
					unit->main_order_timer = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->spend_unit_energy(tech_energy_cost);
				}
			} else {
				// Cancel when way out of range or lost sight of target
				u32 distance = bw::get_distance_fast(unit->position, target->position);
				if (distance > unit->get_sight_range(true) * 32 * 2 || !target->is_visible_to(unit->player)) {
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