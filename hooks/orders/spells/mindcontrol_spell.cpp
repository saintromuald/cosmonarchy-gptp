#include "recall_spell.h"
#include <BW/api.h>
#include "globals.h"
#include "hooks/weapons/weapon_fire.h"

namespace hooks {
void orders_cast_maledictus(Unit* unit) {
	auto& unit_ext = unit->ext();
	Unit* target = unit->order_target.unit;
	if (!target || (unit->player != target->player && !bw::are_mutual_allies(unit->player, target->player))) {
		unit->status &= ~UnitStatus::CanNotReceiveOrders;
		unit->order_to_idle();
		return;
	}
	bw::decloak_passive_proc(unit);
	u16 unused_tech_energy_cost = 0; // unused now
	if (!unit->main_order_state) {
		if (helper::spell_order(unit, TechId::MindTyrantMaledictus, &unused_tech_energy_cost, weapons_dat::max_range[orders_dat::order_weapon_id[OrderId::CastMaledictus]], 1326)) {
			// Start the channel
			unit->sprite->create_top_overlay(ImageId::MindTyrantTyrannyCastOverlay);
			unit->play_iscript_animation(IscriptAnimation::Unused2);
			unit->main_order_state = 1;
			unit->main_order_timer = ProtossMindTyrant::MaledictusCastTime;
			unit->status |= UnitStatus::CanNotReceiveOrders;
		}
	} else {
		// Cancel when way out of range or lost sight of target
		u32 distance = bw::distance_between(unit->position, target->position);
		if (distance > weapons_dat::max_range[orders_dat::order_weapon_id[OrderId::CastMaledictus]] * 2 || !target->is_visible_to(unit->player)) {
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
			unit->order_to_idle();
			return;
		}

		switch (unit->main_order_timer) {
			case 95:
			case 71:
			case 46:
			case 22: {
				unit->sprite->create_top_overlay(ImageId::MindTyrantTyrannyCastOverlay);
			}
		}

		// Wait for channel
		if (unit->main_order_timer) {
			return;
		}
		unit->spend_energy(techdata_dat::energy_cost[TechId::MindTyrantMaledictus] * 256);
		fire_bullet(unit, WeaponId::MindTyrantMaledictus, {unit->order_target.pt, target});
		unit->status &= ~UnitStatus::CanNotReceiveOrders;
		unit->order_to_idle();
	}
}
} // namespace hooks