#include "hallucination_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_hallucination1(Unit* unit) {
		s32 sight_range = unit->get_sight_range(true);
		u16 tech_cost = 0;
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueJustCastedHallucination]) {
			unit->order_to_idle();
		} else if (helper::spell_order(unit, TechId::Hallucination, &tech_cost, sight_range * 32, 2201)) {
			Vector32 hit = {unit->order_target.pt.x, unit->order_target.pt.y};
			if (Unit* target = unit->order_target.unit) {
				hit = {target->get_x(), target->get_y()};
			}
			scbw::decloak_passive_proc(unit);
			unit->play_iscript_animation(IscriptAnimation::Unused2);
			unit->spend_unit_energy(tech_cost);

			unit_ext->values[ValueJustCastedHallucination] = 1;
			unit->order_to_idle();
		}
	}

	void apply_hallucination(Unit* unit, Unit* source, u32 time) {
		u16 remove_timer = time;
		helper::apply_hallucination_stats(unit);
		if (source->remove_timer && source->remove_timer < remove_timer) {
			remove_timer = source->remove_timer;
		}
		if (!unit->remove_timer || remove_timer < unit->remove_timer) {
			unit->remove_timer = remove_timer;
		}
		if (source) {
			if (unit->id == UnitId::ProtossAccantorOld) {
				helper::hallucinate_scarabs(unit, source);
				u32 count = source->unit_with_hangar.in_hangar_count;
				for (u32 i = 0; i < count; ++i) {
					Unit* plasma_shell = scbw::create_hangar_unit(unit, UnitId::ProtossPlasmaShell);
					if (plasma_shell) {
						apply_hallucination(plasma_shell, unit, unit->remove_timer);
					}
				}	
			}
			if (unit->id == UnitId::ProtossSolarion) {
				u32 count = source->unit_with_hangar.in_hangar_count + source->unit_with_hangar.out_hangar_count;
				for (u32 i = 0; i < count; ++i) {
					Unit* steward = scbw::create_hangar_unit(unit, UnitId::ProtossSteward);
					if (steward) {
						apply_hallucination(steward, unit, unit->remove_timer);
					}
				}
			}
		}
	}

	// Hooked to rule out AI issues -- Pr0nogo
	Unit* create_hallucination(Unit* source, s32 player, u32 time) {
		if (units_dat::base_property[source->id] & UnitProperty::Building || source->id == UnitId::ProtossSteward || source->id == UnitId::ProtossPlasmaShell) {
			return 0;
		}
		Unit* result = 0;
		Unit* hallucinated_unit = helper::create_unit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		if (!hallucinated_unit) {
			helper::display_last_network_error_for_player(player);
			return 0;
		}
		apply_hallucination(hallucinated_unit, source, time);
		scbw::give_ai_ex(hallucinated_unit);
		return hallucinated_unit;
	}
}