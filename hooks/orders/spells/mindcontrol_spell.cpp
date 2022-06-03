#include "recall_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void ordersCastMindControl(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		Unit* target = unit->orderTarget.unit;
		if (!target || unit->player == target->player) {
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
			unit->order_to_idle();
		} else {
			u16 techEnergyCost = 0;
			// Energy cost is initialized in this function
			// Corresponding error message is "Invalid Target"
			if (helper::spell_order(unit, TechId::MindControl, &techEnergyCost, unit->get_sight_range(true) * 32, 0x052E)) {
				// Channel
				if (!unit->main_order_state) {
					unit->play_iscript_animation(IscriptAnimation::Unused2);
					unit->main_order_state = 1;
					unit->mainOrderTimer = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
				}
				if (unit->mainOrderTimer) {
					return;
				}
				unit->status &= ~UnitStatus::CanNotReceiveOrders;

				if (target->status & UnitStatus::IsHallucination) {
					target->remove();
				} else {
					u32 overlayId = scbw::has_overlay(target) + ImageId::MindControlHit_Small;
					if (target->subunit) {
						target->subunit->sprite->create_top_overlay(overlayId, 0, 0, 0);
					} else {
						target->sprite->create_top_overlay(overlayId, 0, 0, 0);
					}
					helper::MindControlHit(unit, target);
					target->order_to_idle();
				}
				if (!(*CHEAT_STATE & CheatFlags::TheGathering)) {
					unit->spend_unit_energy(techEnergyCost);
				}
				helper::play_sound_from_unit(SoundId::Protoss_Darchon_mind_wav, target, 1, 0);
				unit->order_to_idle();
			} else {
				// Reset timer if target out of range
				unit->main_order_state = 0;
			}
		}
	}
}