#include "hooks\hydra_unit_ai.h"
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
#include "UnitConstants.h"
#include "MathUtils.h"
#include "globals.h"
void hydra_unit_ai(Unit* unit)
{
	UnitExt* unitExt = globals.GetUnitExt(unit);

	/* DONE
	Apostle Lazarus Agent: If 5 or more organic allies (that don't have the buff) are nearby,
	cast the spell. **DONE**
	AI should also try to position Apostles near organics.

	Savant Power Siphon: If 2 or more allied specialists are nearby,
	target an enemy specialist within 7 range. **DONE**

	Matraleth Parasite: When 5 or more supply's worth of units (of any owner) are within 128px of each other,
	target the group. **DONE**

	Star Sovereign Grief of All Gods: When at least 2000 HP+shields worth of enemy units are within 192px of the Star Sovereign,
	and there are no allied town halls and less than 2000 HP+shields worth of allied units within the same radius,
	and the Star Sovereign has more than 50% of HP+shields combined,
	channel the ability. **DONE**
	Attempt to protect the Star Sovereign with low-cost/low-HP units.

	Lanifect Disruption Field:
	When there are at least 3 allies within 128px, and there are at least 2 enemies within 128px,
	and there are no activated Disruption Fields within 96px,
	and the Lanifect is above 50% total HP and shields,
	move towards the most significant enemy threat within 128px (in terms of enemy unit count).
	Recheck conditions to disable the behavior (besides HP/shield checks). **DONE**
	*/

	/* TODO
	Eidolon Lobotomy Mine: I think all mines should be reworked in terms of how the AI places them,
					I would suggest dropping them in positions where enemy ground units
					have been spotted/attacked/killed within the last few minutes.
					That way it's like they're responding to ground paths taken by enemies.
					Vulture Spider Mines could be done in similar ways.

	Madrigal Tempo Change: When 3 or more hostiles within 4 weapon range,
	switch to Tempo Up (Furia);
	else, switch to Tempo Down (Imperioso)

	Paladin Skyfall: If 6 or more enemies are within 64px of each other while being within range,
					target terrain between the Paladin and the target group, 64px away from the group.

	Science Vessel Observance: If 3 or more enemies are within 96px of each other, and the Vessel has more than 50% HP, target
					the group and stay within range while attempting to avoid damage.

	Azazel Sublime Shepherd: Toggle while 2 or more allied units are within 96px of the Azazel, and stay within 96px of as many
					allies as possible.

	Didact Recall: "Offensive" cast when the Didact is added to an attack and a group of 6 or more units are within 160px of
					one another. Attempt to group as many units as possible during the channel.
	*/

	if (unit->id == UnitId::TerranApostle)
	{
		if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent] * 256) && unitExt->timers[TimerAI_Lazarus] == 0)			
		{
			s32 count = 0;
			hydraFinderLoop(nullptr, unit->position, 128, Flags::NoBuilding | Flags::Ally, {}, [&](Unit* u)
			{
				UnitExt* uExt = globals.GetUnitExt(u);
				if (uExt->values[ValueLazarusAgent] != 1)
				{
					count++;
				}
			});
			if (count >= 5)
			{
				scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 1);
				unitExt->timers[TimerAI_Lazarus] = ApostleAILastBreathTimer;
			}
		}
	}
	else if (unit->id == UnitId::TerranEidolon)
	{
		// mines
	}
	else if (unit->id == UnitId::TerranSavant)
	{
		if (unitExt->timers[TimerAI_Siphon] == 0 && unit->mainOrderId != OrderId::PowerSiphon)
		{
			s32 count = 0;
			hydraFinderLoop(unit, unit->position, weapons_dat::OuterSplashRadius[WeaponId::SavantPowerSiphon],
				Flags::Ally | Flags::Spellcaster, {}, [&](Unit* u)
			{
				if (u->energy < u->getMaxEnergy())
				{
					count++;
				}
			});
			if (count >= 2)
			{
				// Savant Power Siphon: If 2 or more allied specialists are nearby, target an enemy specialist within 7 range.
				auto enemy = rangeMatchUnit(unit, 7 * 32, Flags::Enemy | Flags::Spellcaster | Flags::HasEnergy, {});
				if (enemy != nullptr)
				{
					unit->orderTo(OrderId::PowerSiphon, enemy);
					unitExt->timers[TimerAI_Siphon] = 3 * 32;
				}
			}
		}
	}

	else if (unit->id == UnitId::TerranMadrigal)
	{
		u32 furiaDamagePotential = weapons_dat::DamageAmount[WeaponId::MadrigalFuriaMissiles] / weapons_dat::Cooldown[WeaponId::MadrigalFuriaMissiles];
		furiaDamagePotential *= hydraUnitCount(unit, unit->position, 5 * 32, Flags::Enemy, {});
		if (unit->orderTarget.unit)
		{
			UnitExt* targetExt = globals.GetUnitExt(unit->orderTarget.unit);
			furiaDamagePotential += targetExt->values[ValueDirgeStacks] * MadrigalDirgeDamage * 256;
		}
		u32 dirgeDamagePotential = (weapons_dat::DamageAmount[WeaponId::MadrigalDirge] + MadrigalDirgeDamage * 256) / weapons_dat::Cooldown[WeaponId::MadrigalDirge];

		if (furiaDamagePotential > dirgeDamagePotential)
		{
			scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 0);
		}
		else
		{
			scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 1);
		}
	}

	else if (unit->id == UnitId::ZergMatraleth)
	{
		if (unit->mainOrderId != OrderId::CastParasite && unit->energy >= techdata_dat::EnergyCost[TechId::Parasite] * 256 && unitExt->timers[TimerAI_Parasite] == 0)
		{
			hydraFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Parasite],
				Flags::NotSelf, {}, [&](Unit* u)
			{
				std::vector<std::pair<Unit*, u32>> group;
				s32 supply = 0;
				hydraFinderLoop(u, u->position, 128, 0, {}, [&](Unit* u2)
				{
					u32 priority = units_dat::SupplyRequired[u->id] * 100;
					if (u->status & UnitStatus::InAir)
					{
						priority /= 4;
					}
					auto weapon = u->getGroundWeapon();
					if (weapon != WeaponId::None)
					{
						if (weapons_dat::MaxRange[weapon] >= 64)
						{
							priority /= 2;
						}
					}
					group.push_back(std::make_pair(u, priority));
					supply += units_dat::SupplyRequired[u->id];
				});
				std::sort(group.begin(), group.end(), [](auto& left, auto& right)
				{
					return left.second > right.second;
				});
				supply /= 2;
				if (supply >= 5)
				{
					unit->orderTo(OrderId::CastParasite, group[0].first);
					unitExt->timers[TimerAI_Parasite] = MatralethAIParasiteTimer;
					return;
				}
			});
		}

		if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::EnsnaringBrood] * 256) && unitExt->timers[TimerAI_EnsnaringBrood] == 0)
		{
			s32 count = 0;
			hydraFinderLoop(nullptr, unit->position, 128, Flags::NoBuilding | Flags::Ally, {}, [&](Unit* u)
			{
				UnitExt* uExt = globals.GetUnitExt(u);
				if (!uExt->values[TimerEnsnaringBrood])
				{
					count++;
				}
			});
			if (count >= 5)
			{
				scbw::set_generic_rust_value(unit, RustId::CastEnsnaringBrood, 1);
				unitExt->timers[TimerAI_EnsnaringBrood] = MatralethAIEnsnaringBroodTimer;
			}
		}
	}

	else if (unit->id == UnitId::ProtossLanifect)
	{
		auto web_state = scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionWeb);
		if (unitExt->values[ValueExecutingCorsairMoveOrder] != 0)
		{
			if (unit->mainOrderId != OrderId::Move)
			{
				unitExt->values[ValueExecutingCorsairMoveOrder] = 0;
			}
		}
		u32 webs = hydraUnitCount(unit, unit->sprite->position, AIWebSeekRange, 0, {UnitId::Spell_DisruptionWeb});
		u32 allies = hydraUnitCount(unit, unit->sprite->position, AICorsairAllySeekRange, Flags::Ally | Flags::NotSelf | Flags::Combatant, {});
		u32 enemies = hydraUnitCount(unit, unit->sprite->position, AICorsairEnemySeekRange, Flags::Enemy | Flags::Combatant, {});
		f64 percentage = (f64)(unit->hitPoints + unit->shields) / (f64)(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id] * 256.0);
		if (web_state == 0)
		{
			if (webs == 0 && unit->energy >= AIWebMinEnergy * 256
				&& percentage >= AICorsairLifePercentage
				&& allies >= AIWebAllyMinCount && enemies >= AIWebEnemyMinCount)
			{
				Unit* mostSignificant = nullptr;
				auto maxSignificance = 0;
				hydraFinderLoop(unit, unit->sprite->position, 128, Flags::Enemy, {}, [&](Unit* u)
				{
					auto significance = hydraUnitCount(u, u->position, AICorsairEnemySeekRange / 2,
						Flags::NotSelf | Flags::Ally, {});
					if (significance > maxSignificance)
					{
						maxSignificance = significance;
						mostSignificant = u;
					}
				});
				if (mostSignificant != nullptr)
				{
					unit->orderTo(OrderId::Move, mostSignificant->position.x, mostSignificant->position.y);
					scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionWeb, 1);
					unitExt->timers[TimerAI_DisruptionField] = AIWebTimer;
				}
			}
		}
		else
		{
			if (!(allies >= AIWebAllyMinCount && enemies >= AIWebEnemyMinCount))
			{
				if (unitExt->timers[TimerAI_DisruptionField] == 0)
				{
					scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionWeb, 0);
					unitExt->values[ValueExecutingCorsairMoveOrder] = 0;
					if (unit->mainOrderId == OrderId::Move)
					{
						unit->orderTo(OrderId::Stop);
					}
				}
			}
		}
	}
		
	else if (unit->id == UnitId::ProtossStarSovereign && unitExt->timers[TimerAI_GriefOfAllGods] == 0)
	{
		auto enemyUnitsHealth = 0;
		auto alliedUnitsHealth = 0;
		bool alliedTownHalls = false;
		hydraFinderLoop(unit, unit->sprite->position, AIStarsovereignAbilSeekRange,
			Flags::NotSelf | Flags::NoAir, {}, [&](Unit* u)
		{
			if (!scbw::isAlliedTo(unit->playerId, u->playerId))
			{
				enemyUnitsHealth += (u->hitPoints + u->shields) / 256;
			}
			if (scbw::isAlliedTo(unit->playerId, u->playerId))
			{
				if (units_dat::BaseProperty[u->id] & UnitProperty::ResourceDepot)
				{
					alliedTownHalls = true;
					return;
				}
				alliedUnitsHealth += (u->hitPoints + u->shields) / 256;
			}
		});
		if (!alliedTownHalls && enemyUnitsHealth >= 2000
			&& alliedUnitsHealth < 2000
			&& (unit->hitPoints + unit->shields) * 2 >=	(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id]))
		{
			unitExt->timers[TimerAI_GriefOfAllGods] = 612;
			unit->orderComputerClear(OrderId::KhaydarinEclipse);
			unit->orderTarget.pt = unit->sprite->position;
			unit->mainOrderState = 0;
		}
	}
	
	/*	
	else if (unit->id == UnitId::TerranPaladin) {
		if (unit->mainOrderId != OrderId::Skyfall && scbw::get_generic_timer(unit,RustId::AI_PaladinTimer)) {
			bool skyfallCondition = false;
			hydraFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Skyfall],
				CFlags::Enemy, {}, [&](Unit* u) {

				// scbw::print("Loop");
				Unit* tryTarget = u;
				s32 innerCount = 0;
				hydraFinderLoop(u, u->position, weapons_dat::MaxRange[WeaponId::Skyfall] , CFlags::Ally, {},
					[&](Unit* internalUnit) {
					innerCount++;
				});
				// scbw::printf("Count is %d", innerCount);
				if (innerCount >= 6) {
					Point16 source = unit->position;
					Point16 target = u->position;

					auto distance = scbw::getDistanceFast(source.x, source.y, target.x, target.y);
					if (distance >= 64) {
						f64 fraction = (distance - 64) / distance;
						u16 resultX, resultY;
						resultX = MathUtils::lerp(fraction, source.x, target.x, false);
						resultY = MathUtils::lerp(fraction, source.y, target.y, false);
						unit->orderTo(OrderId::Skyfall, u->position.x, u->position.y);
						scbw::set_generic_timer(unit, RustId::AI_PaladinTimer, 50);
					}
				}
			});
		}
	}
	*/
}
