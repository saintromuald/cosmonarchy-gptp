#include "spellcasting.h"
#include "spells/spells.h"
#include <algorithm>

namespace
{
	bool canCastSpellOrder(Unit* unit, u8 techId, u8 orderId);
	bool aiCastSpellOrder(Unit* unit, Unit* target, u8 orderId, u8 aiActionFlag = 1);
	u16 getOrderEnergyCost(u8 orderId);
	bool isNukeTimerReady(u8 playerId);
	Unit* getLoadedSilo(Unit* ghost, b32 checkNukeAvailableForGhost = 1);
}

namespace AI
{
	// Attempts make the @p unit cast a spell.
	bool AI_spellcasterHook(Unit* unit, bool isUnitBeingAttacked)
	{
		if (!isUnitBeingAttacked
			&& AIScriptController[unit->player].spellcaster_timer != 0)
		{
			return false;
		}
		u16 energyReserve;

		switch (unit->id)
		{
			case UnitId::TerranPhobos:
			{
				// Yamato Cannon
				if (canCastSpellOrder(unit, TechId::YamatoGun, OrderId::FireYamatoGun1))
				{
					Unit* target = findBestYamatoGunTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::FireYamatoGun1
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::FireYamatoGun1))
					{
						return true;
					}
				}
				break;
			}
			case UnitId::TerranEidolon:
			{
				// Lobotomy Mine
				if (canCastSpellOrder(unit, TechId::Lockdown, OrderId::MagnaPulse)
					&& unit->mine_carrier.mine_count > 0)
				{
					Unit* target = findBestLockdownTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::MagnaPulse
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::MagnaPulse))
					{
						return true;
					}
				}

				// Launch Nuclear Missile
				if (isNukeTimerReady(unit->player))
				{
					Unit* silo = getLoadedSilo(unit);
					if (!silo)
					{
						return false;
					}
					Unit* target = findBestNukeLaunchTarget(unit, isUnitBeingAttacked);
					if (aiCastSpellOrder(unit, target, OrderId::NukePaint))
					{
						silo->building.silo.nuke->connected_unit = unit;
						AIScriptController[unit->player].ai_last_nuke_time = *elapsed_time_seconds;
						return true;
					}
				}
			} break;
			case UnitId::TerranAzazel:
			{
				// Defensive Matrix
				if (canCastSpellOrder(unit, TechId::DefensiveMatrix, OrderId::DefensiveMatrix))
				{
					Unit* target = findBestDefensiveMatrixTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::DefensiveMatrix
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::DefensiveMatrix))
					{
						return true;
					}
				}
			} break;
			case UnitId::TerranSeraph:				
			{
				// Irradiate
				if (canCastSpellOrder(unit, TechId::Irradiate, OrderId::Irradiate))
				{
					Unit* target = findBestIrradiateTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::Irradiate
						&& unit->order_target.unit == target)
					{
						return false;
					}

					if (aiCastSpellOrder(unit, target, OrderId::Irradiate))
					{
						return true;
					}
				}

				// Launch Nuclear Missile
				if (isNukeTimerReady(unit->player))
				{
					Unit* silo = getLoadedSilo(unit);
					if (!silo)
					{
						return false;
					}
					Unit* target = findBestNukeLaunchTarget(unit, isUnitBeingAttacked);
					if (aiCastSpellOrder(unit, target, OrderId::NukePaint))
					{
						silo->building.silo.nuke->connected_unit = unit;
						AIScriptController[unit->player].ai_last_nuke_time = *elapsed_time_seconds;
						return true;
					}
				}
			} break;
			case UnitId::ZergMatraleth:
			{
				// Parasite
				if (unit->energy >= unit->get_max_energy() / 2)
				{
					if (canCastSpellOrder(unit, TechId::Parasite, OrderId::CastParasite))
					{
						Unit* target = findBestParasiteTarget(unit, isUnitBeingAttacked);

						if (unit->main_order_id == OrderId::CastParasite
							&& unit->order_target.unit == target)
						{
							return false;
						}
						if (aiCastSpellOrder(unit, target, OrderId::CastParasite, 4))
						{
							return true;
						}
						return false;
					}
				}
			} break;
			case UnitId::ZergIsthrathaleth:
			{
				// Plague
				if (canCastSpellOrder(unit, TechId::Plague, OrderId::Plague))
				{
					Unit* target = findBestPlagueTarget(unit, isUnitBeingAttacked);

					if (unit->main_order_id == OrderId::Plague
						&& unit->order_target.unit == target)
						return false;

					if (aiCastSpellOrder(unit, target, OrderId::Plague))
						return true;

				}

				// Dark Swarm
				if (canCastSpellOrder(unit, TechId::DarkSwarm, OrderId::DarkSwarm))
				{
					Unit* target = findBestDarkSwarmTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::DarkSwarm
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::DarkSwarm))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossCantavis:
			{
				// Psionic Storm
				if (canCastSpellOrder(unit, TechId::PsionicStorm, OrderId::PsiStorm))
				{
					Unit* target = findBestPsiStormTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::PsiStorm
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::PsiStorm))
					{
						return true;
					}
				}

				// Hallucination
				if (unit->get_max_energy() == unit->energy
					&& canCastSpellOrder(unit, TechId::Hallucination, OrderId::Hallucination1))
				{
					Unit* target = findBestHallucinationTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::Hallucination1
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::Hallucination1))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossDidact:
			{
				// Stasis Field
				if (canCastSpellOrder(unit, TechId::StasisField, OrderId::StasisField))
				{
					Unit* target = findBestStasisFieldTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::StasisField
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::StasisField))
					{
						return true;
					}
				}

				// Recall
				if (canCastSpellOrder(unit, TechId::Recall, OrderId::Teleport))
				{
					Unit* target = findBestRecallTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::Teleport
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::Teleport))
					{
						return true;
					}
				}
			} break;
			case UnitId::ProtossMindTyrant:
			{
				// Mind Control
				if (canCastSpellOrder(unit, TechId::MindControl, OrderId::CastMindControl))
				{
					Unit* target = findBestMindControlTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::CastMindControl
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::CastMindControl))
					{
						return true;
					}
				}

				// Maelstrom
				energyReserve = getOrderEnergyCost(OrderId::CastMaelstrom)
					+ (getOrderEnergyCost(OrderId::CastMindControl) / 2);
				energyReserve = std::min(energyReserve, unit->get_max_energy());

				if ((unit->energy >= energyReserve || scbw::is_cheat_enabled(CheatFlags::TheGathering))
					&& unit->canUseTech(TechId::Maelstrom, unit->player) != 1)
				{
					Unit* target = findBestMaelstromTarget(unit, isUnitBeingAttacked);
					if (unit->main_order_id == OrderId::CastMaelstrom
						&& unit->order_target.unit == target)
					{
						return false;
					}
					if (aiCastSpellOrder(unit, target, OrderId::CastMaelstrom))
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
	bool canCastSpellOrder(Unit* unit, u8 techId, u8 orderId)
	{
		u16 energyCost = 0;
		if (orders_dat::TechUsed[orderId] < TechId::None)
		{
			energyCost = techdata_dat::EnergyCost[orders_dat::TechUsed[orderId]] * 256;
		}
		if (techId == TechId::Lockdown && unit->mine_carrier.mine_count == 0)
		{
			return false;
		}
		if (scbw::is_cheat_enabled(CheatFlags::TheGathering || unit->energy >= energyCost))
		{
			return unit->canUseTech(techId, unit->player) == 1;
		}
		return false;
	}

	// Logically equivalent to function @ 0x004A1290
	bool aiCastSpellOrder(Unit* unit, Unit* target, u8 orderId, u8 aiActionFlag)
	{
		if (!target || target->ai_action_flags & aiActionFlag)
		{
			return false;
		}
		if (unit->main_order_id != orderId)
		{
			unit->order_to(orderId, target);
			target->ai_action_flags |= aiActionFlag;
		}
		return true;
	}

	// Logically equivalent to function @ 0x0049E1C0
	u16 getOrderEnergyCost(u8 orderId)
	{
		if (orders_dat::TechUsed[orderId] < TechId::None)
		{
			return techdata_dat::EnergyCost[orders_dat::TechUsed[orderId]] * 256;
		}
		else
		{
			return 0;
		}
	}

	// Logically equivalent to function @ 0x00446E50
	bool isNukeTimerReady(u8 playerId)
	{
		return *elapsed_time_seconds >= AIScriptController[playerId].ai_last_nuke_time + 60 * AIScriptController[playerId].ai_nuke_rate;
	}

	// Logically equivalent to function @ 0x00463360
	// checkNukeAvailableForGhost is 1 by default
	// if checkNukeAvailableForGhost is 0, only check for a loaded silo without checking if the nuke is already linked to another ghost
	Unit* getLoadedSilo(Unit* ghost, b32 checkNukeAvailableForGhost)
	{
		Unit* return_value = nullptr;
		Unit* current_unit = firstPlayerUnit->unit[ghost->player];
		while (current_unit != nullptr && return_value == nullptr)
		{
			if (current_unit->id == UnitId::TerranNuclearSilo
				&& current_unit->building.silo.is_ready)
			{
				if (!checkNukeAvailableForGhost)
				{
					Unit* nuke = current_unit->building.silo.nuke;
					if (nuke->connected_unit == nullptr
						|| nuke->connected_unit == ghost
						|| nuke->connected_unit->id == UnitId::TerranNuclearSilo)
					{
						return_value = current_unit;
					}
				}
				else
				{
					return_value = current_unit;
				}
			}
			current_unit = current_unit->player_link.next;
		}

		return return_value;

		/*
		for (Unit* unit = firstPlayerUnit->unit[ghost->player];
		     unit; unit = unit->player_link.next)
		{
		  if (unit->id == UnitId::nuclear_silo && unit->building.silo.is_ready) {
		    Unit* nuke = unit->building.silo.nuke;
		    if (!nuke->connected_unit
		        || nuke->connected_unit == ghost
		        || nuke->connected_unit->id == UnitId::nuclear_silo)
		      return unit;
		  }
		}
		return nullptr;
		*/
	}
}