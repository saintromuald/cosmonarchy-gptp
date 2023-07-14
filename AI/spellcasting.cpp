#include "spellcasting.h"
#include "spells/spells.h"
#include <algorithm>

namespace
{
	bool can_cast_spell_order(Unit* unit, u8 tech_id, u8 order_id);
	bool ai_cast_spell_order(Unit* unit, Unit* target, u8 order_id, u8 ai_action_flag = 1);
	bool ai_cast_spell_order_pos(Unit* unit, Point16 pos, u8 order_id, u8 ai_action_flag);
	u16 get_order_energy_cost(u8 order_id);
	bool is_nuke_timer_ready(u8 player);
	Unit* get_loaded_silo(Unit* nuker, b32 check_nuke_available_for_nuker = 1);
}

namespace AI
{
	// Attempts make the @p unit cast a spell.
	bool ai_spellcaster_hook(Unit* unit, bool is_unit_being_attacked)
	{
		u16 energy_reserve = 0;
		switch (unit->id)
		{
			case UnitId::TerranPhobos:
			{
				// Yamato Cannon
				if (can_cast_spell_order(unit, TechId::YamatoGun, OrderId::FireYamatoGun1))
				{
					Unit* target = find_best_yamato_gun_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::FireYamatoGun1 && unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::FireYamatoGun1))
					{
						return true;
					}
				}
				break;
			}
			case UnitId::TerranEidolon:
			{
				// Launch Nuclear Missile
				if (is_nuke_timer_ready(unit->player))
				{
					if (is_unit_being_attacked) {
						return false;
					}
					Unit* silo = get_loaded_silo(unit);
					if (!silo)
					{
						return false;
					}
					Unit* target = find_best_nuke_launch_target(unit, is_unit_being_attacked);
					if (ai_cast_spell_order(unit, target, OrderId::NukePaint))
					{
						silo->building.silo.nuke->connected_unit = unit;
						g_aiscript_controller[unit->player].ai_last_nuke_time = *g_elapsed_time_seconds;
						return true;
					}
				}
			} break;
			case UnitId::TerranAzazel:
			{
				// Defensive Matrix
				if (can_cast_spell_order(unit, TechId::DefensiveMatrix, OrderId::DefensiveMatrix))
				{
					Unit* target = find_best_defensive_matrix_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::DefensiveMatrix && unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::DefensiveMatrix))
					{
						return true;
					}
				}
			} break;
			case UnitId::TerranSeraph:				
			{
				// Irradiate
				if (can_cast_spell_order(unit, TechId::Irradiate, OrderId::Irradiate))
				{
					Unit* target = find_best_irradiate_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::Irradiate && unit->order_target.unit == target)
					{
						return false;
					}

					if (ai_cast_spell_order(unit, target, OrderId::Irradiate))
					{
						return true;
					}
				}

				// Launch Nuclear Missile
				if (is_nuke_timer_ready(unit->player))
				{
					if (is_unit_being_attacked) {
						return false;
					}
					Unit* silo = get_loaded_silo(unit);
					if (!silo)
					{
						return false;
					}
					Unit* target = find_best_nuke_launch_target(unit, is_unit_being_attacked);
					if (ai_cast_spell_order(unit, target, OrderId::NukePaint))
					{
						silo->building.silo.nuke->connected_unit = unit;
						g_aiscript_controller[unit->player].ai_last_nuke_time = *g_elapsed_time_seconds;
						return true;
					}
				}
			} break;
			case UnitId::ZergMatraleth:
			{
				// Parasite
				if (unit->energy >= unit->get_max_energy() / 2) {
					if (can_cast_spell_order(unit, TechId::MatralethParasite, OrderId::CastParasite)) {
						Thingy* target = 0;
						u32 radius = is_unit_being_attacked ? 9 * 32 : 64 * 32;

						EntryArray corpses = {};
						corpse_search_radial(&corpses, unit->position, radius);
						for (u32 i = 0; i < corpses.count; ++i) {
							Thingy* corpse = corpses.corpses[i];
							if (is_active_corpse(corpse)) {
								target = corpse;
								break;
							}
						}
						if (!target) {
							return false;
						}

						if (unit->main_order_id == OrderId::CastParasite && unit->order_target.pt == target->sprite->position) {
							return false;
						}
						if (ai_cast_spell_order_pos(unit, target->sprite->position, OrderId::CastParasite, 4)) {
							return true;
						}
						return false;
					}
				}
			} break;
			case UnitId::ZergIsthrathaleth:
			{
				// Plague
				if (can_cast_spell_order(unit, TechId::Plague, OrderId::Plague))
				{
					Unit* target = find_best_plague_target(unit, is_unit_being_attacked);

					if (unit->main_order_id == OrderId::Plague
						&& unit->order_target.unit == target)
						return false;

					if (ai_cast_spell_order(unit, target, OrderId::Plague))
						return true;

				}

				// Carapace Swarm
				if (can_cast_spell_order(unit, TechId::CarapaceSwarm, OrderId::CarapaceSwarm))
				{
					Unit* target = find_best_carapace_swarm_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::CarapaceSwarm
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::CarapaceSwarm))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossCantavis:
			{
				// Psionic Storm
				if (can_cast_spell_order(unit, TechId::PsionicStorm, OrderId::PsiStorm))
				{
					Unit* target = find_best_psi_storm_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::PsiStorm && unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::PsiStorm))
					{
						return true;
					}
				}

				// Hallucination
				if (unit->get_max_energy() == unit->energy && can_cast_spell_order(unit, TechId::Hallucination, OrderId::Hallucination1))
				{
					Unit* target = find_best_hallucination_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::Hallucination1 && unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::Hallucination1))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossDidact:
			{
				// Stasis Field
				if (can_cast_spell_order(unit, TechId::StasisField, OrderId::StasisField))
				{
					Unit* target = find_best_stasis_field_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::StasisField
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::StasisField))
					{
						return true;
					}
				}

				// DimensionalRecall
				if (can_cast_spell_order(unit, TechId::DimensionalRecall, OrderId::Teleport))
				{
					Unit* target = find_best_recall_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::Teleport
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::Teleport))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossMindTyrant:
			{
				// Tyranny
				if (can_cast_spell_order(unit, TechId::Tyranny, OrderId::CastTyranny))
				{
					Unit* target = find_best_tyranny_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::CastTyranny
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::CastTyranny))
					{
						return true;
					}
				}

				// Maelstrom
				energy_reserve = get_order_energy_cost(OrderId::CastMaelstrom) + (get_order_energy_cost(OrderId::CastTyranny) / 2);
				energy_reserve = std::min(energy_reserve, unit->get_max_energy());

				if ((unit->energy >= energy_reserve)
					&& unit->can_use_tech(TechId::MindTyrantMaelstrom, unit->player) != 1)
				{
					Unit* target = find_best_maelstrom_target(unit, is_unit_being_attacked);
					if (unit->main_order_id == OrderId::CastMaelstrom
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (ai_cast_spell_order(unit, target, OrderId::CastMaelstrom))
					{
						return true;
					}
				}
			} break;
		}
		return false;
	}
}

namespace
{
	// Logically equivalent to function @ 0x004A11E0
	bool can_cast_spell_order(Unit* unit, u8 tech_id, u8 order_id)
	{
		u16 energy_cost = 0;
		if (orders_dat::tech_used[order_id] != TechId::None)
		{
			energy_cost = techdata_dat::energy_cost[orders_dat::tech_used[order_id]] * 256;
		}
		if (unit->energy >= energy_cost)
		{
			return unit->can_use_tech(tech_id, unit->player) == 1;
		}
		return false;
	}

	// Logically equivalent to function @ 0x004A1290
	bool ai_cast_spell_order(Unit* unit, Unit* target, u8 order_id, u8 ai_action_flag) {
		if (!target || target->ai_action_flags & ai_action_flag) {
			return false;
		}
		if (unit->main_order_id != order_id) {
			unit->order_to(order_id, target);
			target->ai_action_flags |= ai_action_flag;
		}
		return true;
	}

	bool ai_cast_spell_order_pos(Unit* unit, Point16 pos, u8 order_id, u8 ai_action_flag) {
		if (unit->main_order_id != order_id) {
			unit->order_to(order_id, pos.x, pos.y);
		}
		return true;
	}

	// Logically equivalent to function @ 0x0049E1C0
	u16 get_order_energy_cost(u8 order_id)
	{
		if (orders_dat::tech_used[order_id] != TechId::None)
		{
			return techdata_dat::energy_cost[orders_dat::tech_used[order_id]] * 256;
		}
		else
		{
			return 0;
		}
	}

	// Logically equivalent to function @ 0x00446E50
	bool is_nuke_timer_ready(u8 player)
	{
		return *g_elapsed_time_seconds >= g_aiscript_controller[player].ai_last_nuke_time + 60 * g_aiscript_controller[player].ai_nuke_rate;
	}

	// Logically equivalent to function @ 0x00463360
	// check_nuke_available_for_nuker is 1 by default
	// if check_nuke_available_for_nuker is 0, only check for a loaded silo without checking if the nuke is already linked to another nuker
	Unit* get_loaded_silo(Unit* nuker, b32 check_nuke_available_for_nuker)
	{
		Unit* silo = 0;
		Unit* current_unit = first_player_unit->unit[nuker->player];
		while (current_unit && !silo)
		{
			if (current_unit->id == UnitId::TerranAnvil
				&& current_unit->building.silo.is_ready)
			{
				if (!check_nuke_available_for_nuker)
				{
					Unit* nuke = current_unit->building.silo.nuke;
					if (!nuke->connected_unit
					||   nuke->connected_unit == nuker
					||   nuke->connected_unit->id == UnitId::TerranAnvil)
					{
						silo = current_unit;
					}
				}
				else
				{
					silo = current_unit;
				}
			}
			current_unit = current_unit->player_link.next;
		}
		return silo;
	}
}