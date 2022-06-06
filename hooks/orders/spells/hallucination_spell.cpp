#include "hallucination_spell.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_Hallucination1(Unit* unit)
	{
		s32 sightRange = unit->get_sight_range(true);
		u16 techCost = 0;
		if (helper::spell_order(unit, TechId::Hallucination, &techCost, sightRange * 32, 0x899))
		{
			s32 counter = 1;

			Vector32 hit;
			if (Unit* target = unit->order_target.unit)
			{
				hit = {target->get_x(), target->get_y()};
			}
			else
			{
				Point16 pt = unit->order_target.pt;
				hit = {pt.x, pt.y};
			}
			
			scbw::decloak_proc(unit);

			helper::create_bullet(WeaponId::CantavisHallucinationCast, unit, unit->get_x(), unit->get_y(), unit->player, unit->current_direction_1);

			if (!(*cheat_state & CheatFlags::TheGathering))
			{
				unit->spend_unit_energy(techCost);
			}

			unit->order_to_idle();
		}
	}

	void apply_hallucination(Unit* unit, Unit* source) {
		u16 remove_timer = unit->id == UnitId::ZergRilirokor || unit->id == UnitId::ZergRililisk ? 1800 : 0;
		helper::apply_hallucination_stats(unit);
		if (unit->status & UnitStatus::IsHallucination) {
			remove_timer = 1350;
		}
		if (source->remove_timer && source->remove_timer < remove_timer) {
			remove_timer = source->remove_timer;
		}
		if (!unit->remove_timer || remove_timer < unit->remove_timer)
		{
			unit->remove_timer = remove_timer;
		}
		if (source) {
			if (unit->id == UnitId::ProtossAccantor) {
				//helper::hallucinate_scarabs(unit, source);

				u32 count = source->unit_with_hangar.in_hangar_count;
				for (u32 i = 0; i < count; ++i) {
					Unit* plasma_shell = scbw::create_hangar_unit(unit, UnitId::ProtossPlasmaShell);
					apply_hallucination(plasma_shell, unit);
				}
			}
			if (unit->id == UnitId::ProtossSolarion) {
				u32 count = source->unit_with_hangar.in_hangar_count + source->unit_with_hangar.out_hangar_count;
				for (u32 i = 0; i < count; ++i) {
					Unit* steward = scbw::create_hangar_unit(unit, UnitId::ProtossSteward);
					apply_hallucination(steward, unit);
				}
			}
		}
	}

	// Hooked to rule out AI issues -- Pr0nogo
	Unit* createHallucination(Unit* source, s32 player) {
		if (units_dat::BaseProperty[source->id] & UnitProperty::Building || source->id == UnitId::ProtossSteward || source->id == UnitId::ProtossPlasmaShell) {
			return 0;
		}
		Unit* result = 0;
		Unit* hallucinated_unit = helper::create_unit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		if (!hallucinated_unit) {
			helper::display_last_network_error_for_player(player);
			return 0;
		}
		apply_hallucination(hallucinated_unit, source);
		scbw::give_ai_ex(hallucinated_unit, true);
		return hallucinated_unit;
	}
}