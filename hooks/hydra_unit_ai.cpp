#include "hooks\hydra_unit_ai.h"
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
#include "UnitConstants.h"
#include "MathUtils.h"
#include "globals.h"
void hydra_unit_ai(CUnit* unit)
{
	UnitExt* unitExt = globals.GetUnitExt(unit);

	/*
	Apostle Lazarus Agent: If 5 or more organic allies (that don't have the buff) are nearby, cast the spell. **DONE**
					AI should also try to position Apostles near organics, but not sure if that's feasible.
	Savant Power Siphon: If 2 or more allied specialists are nearby, target an enemy specialist within 7 range. **DONE**
	Clarion Phase Link: While 4 or more allied shielded units are within 128px of the Clarion, toggle the ability on. **DONE**
	Queen Parasite: When 5 or more supply's worth of units (of any owner) are within 128px
					of each other, target the group.	**DONE**
	Star Sovereign Grief of All Gods: When at least 2000 HP+shields worth of enemy units are within 320px of the Star Sovereign
					and there are no allied buildings and less than 2000 HP+shields worth of allied units within the same radius,
					and the Star Sovereign has more than 50% of HP+shields combined, channel the ability and attempt to protect the
					Star Sovereign with low-cost/low-HP units.	**DONE**
	Corsair Disruption Web:
					When there are at least 3 allies within 128px, and there are at least 2 enemies within 128px, and there are no
					activated Disruption Webs within 96px, and the Corsair is above 50% total HP and shields, and the Corsair has
					at least 30 energy,
					move towards the most significant enemy threat within 128px (in terms of enemy unit count). Recheck conditions
					to disable the behavior (besides HP/shields and energy checks). **DONE**

	Ghost Lobotomy Mine: I think all mines should be reworked in terms of how the AI places them,
					I would suggest dropping them in positions where enemy ground units
					have been spotted/attacked/killed within the last few minutes.
					That way it's like they're responding to ground paths taken by enemies.
					Vulture Spider Mines could be done in similar ways.
	Paladin Skyfall: If 6 or more enemies are within 64px of each other while being within range,
					target terrain between the Paladin and the target group, 64px away from the group.
	Science Vessel Observance: If 3 or more enemies are within 96px of each other, and the Vessel has more than 50% HP, target
					the group and stay within range while attempting to avoid damage.
	Azazel Sublime Shepherd: Toggle while 2 or more allied units are within 96px of the Azazel, and stay within 96px of as many
					allies as possible.
	Arbiter Recall: "Offensive" cast when the Arbiter is added to an attack and a group of 6 or more units are within 160px of
					one another. Attempt to group as many units as possible during the channel.
	*/

	if (unit->id == UnitId::TerranApostle)
	{
		if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent] * 256) && unitExt->timers[TimerAI_Lazarus] == 0)			
		{
			s32 count = 0;
			hydraFinderLoop(nullptr, unit->position, 128, CFlags::NoBuilding | CFlags::Ally |
				CFlags::OnlyOrganic, {}, [&](CUnit* u)
			{
				UnitExt* uExt = globals.GetUnitExt(u);
				if (uExt->values[ValueLazarusAgent] != 1)
				{
					count++;
				}
			});
			if (count >= 5)
			{
				unitExt->values[ValueCastLazarusAgent] = 1;
				unitExt->timers[TimerAI_Lazarus] = 24;
			}
		}
	}

	else if (unit->id == UnitId::TerranEidolon)
	{
		//mines
	}

	else if (unit->id == UnitId::TerranSavant)
	{
		if (unitExt->timers[TimerAI_Siphon] == 0 && unit->mainOrderId != OrderId::PowerSiphon)
		{
			s32 count = 0;
			hydraFinderLoop(unit, unit->position, weapons_dat::OuterSplashRadius[WeaponId::SavantPowerSiphon],
				CFlags::Ally | CFlags::Spellcaster, {}, [&](CUnit* u)
			{
				if (u->energy < u->getMaxEnergy())
				{
					count++;
				}
			});
			if (count >= 2)
			{
				// Savant Power Siphon: If 2 or more allied specialists are nearby, target an enemy specialist within 7 range.
				auto enemy = rangeMatchUnit(unit, 7 * 32, CFlags::Enemy | CFlags::Spellcaster | CFlags::HasEnergy, {});
				if (enemy != nullptr)
				{
					unit->orderTo(OrderId::PowerSiphon, enemy);
					unitExt->timers[TimerAI_Siphon] = 3 * 32;
				}
			}
		}
	}

	else if (unit->id == UnitId::ZergMatraleth)
	{
		if (unit->mainOrderId != OrderId::CastParasite
			&& unit->energy >= techdata_dat::EnergyCost[TechId::Parasite] * 256
			&& unitExt->timers[TimerAI_Queen] == 0)
		{
			hydraFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Parasite],
				CFlags::NotSelf, {}, [&](CUnit* u)
			{
				std::vector<std::pair<CUnit*, u32>> group;
				s32 supply = 0;
				hydraFinderLoop(u, u->position, 128, 0, {}, [&](CUnit* u2)
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
					unitExt->timers[TimerAI_Queen] = 75;
					return;
				}
			});
		}
	}

	else if (unit->id == UnitId::ProtossStarSovereign && unitExt->timers[TimerAI_GriefOfAllGods] == 0)
	{
		auto enemyUnitsHealth = 0;
		auto alliedUnitsHealth = 0;
		bool alliedBuildings = false;
		hydraFinderLoop(unit, unit->sprite->position, AIStarsovereignAbilSeekRange,
			CFlags::NotSelf | CFlags::NoAir, {}, [&](CUnit* u)
		{
			if (!scbw::isAlliedTo(unit->playerId, u->playerId))
			{
				enemyUnitsHealth += (u->hitPoints + u->shields) / 256;
			}
			if (scbw::isAlliedTo(unit->playerId, u->playerId))
			{
				if (units_dat::BaseProperty[u->id] & UnitProperty::Building)
				{
					alliedBuildings = true;
					return;
				}
				alliedUnitsHealth += (u->hitPoints + u->shields) / 256;
			}
		});
		if (!alliedBuildings && enemyUnitsHealth >= 2000
			&& alliedUnitsHealth < 2000
			&& (unit->hitPoints + unit->shields) * 2 >=	(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id]))
		{
			unitExt->timers[TimerAI_GriefOfAllGods] = 612;
			unit->orderComputerCL(OrderId::KhaydarinEclipse);
			unit->orderTarget.pt = unit->sprite->position;
			unit->mainOrderState = 0;
		}
	}

	else if (unit->id == UnitId::ProtossLanifect)
	{
		auto web_state = scbw::get_generic_rust_value(unit, RustId::DisruptionWeb);
		if (unitExt->values[ValueExecutingCorsairMoveOrder] != 0)
		{
			if (unit->mainOrderId != OrderId::Move)
			{
				unitExt->values[ValueExecutingCorsairMoveOrder] = 0;
			}
		}
		auto webs = hydraUnitCount(unit, unit->sprite->position, AIWebSeekRange, 0, {UnitId::Spell_DisruptionWeb});
		auto allies = hydraUnitCount(unit, unit->sprite->position, AICorsairAllySeekRange, CFlags::Ally | CFlags::NotSelf, {});
		auto enemies = hydraUnitCount(unit, unit->sprite->position, AICorsairEnemySeekRange, CFlags::Enemy, {});
		auto percentage = (f64)(unit->hitPoints + unit->shields) /
			(f64)(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id] * 256);
		if (web_state == 0)
		{
			if (webs == 0 && unit->energy >= AIWebMinEnergy * 256
				&& percentage >= AICorsairLifePercentage
				&& allies >= AIWebAllyMinCount && enemies >= AIWebEnemyMinCount)
			{
				CUnit* mostSignificant = nullptr;
				auto maxSignificance = 0;
				hydraFinderLoop(unit, unit->sprite->position, 128, CFlags::Enemy, {}, [&](CUnit* u)
				{
					auto significance = hydraUnitCount(u, u->position, AICorsairEnemySeekRange / 2,
						CFlags::NotSelf | CFlags::Ally, {});
					if (significance > maxSignificance)
					{
						maxSignificance = significance;
						mostSignificant = u;
					}
				});
				if (mostSignificant != nullptr)
				{
					unit->orderTo(OrderId::Move, mostSignificant->position.x, mostSignificant->position.y);
					scbw::set_generic_rust_value(unit, RustId::DisruptionWeb, 1);
					unitExt->timers[TimerAI_CorsairDweb] = AIWebTimer;
				}
			}
		}
		else
		{
			if (!(allies >= AIWebAllyMinCount && enemies >= AIWebEnemyMinCount))
			{
				if (unitExt->timers[TimerAI_CorsairDweb] == 0)
				{
					scbw::set_generic_rust_value(unit, RustId::DisruptionWeb, 0);
					unitExt->values[ValueExecutingCorsairMoveOrder] = 0;
					if (unit->mainOrderId == OrderId::Move)
					{
						unit->orderTo(OrderId::Stop);
					}
				}
			}
		}
	}

	/*	else if (unit->id == UnitId::TerranPaladin) {
			if (unit->mainOrderId != OrderId::Skyfall && scbw::get_generic_timer(unit,RustId::AI_PaladinTimer)) {
				bool skyfallCondition = false;
				hydraFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Skyfall],
					CFlags::Enemy, {}, [&](CUnit* u) {

	//				scbw::print("Loop");
					CUnit* tryTarget = u;
					s32 innerCount = 0;
					hydraFinderLoop(u, u->position, weapons_dat::MaxRange[WeaponId::Skyfall] , CFlags::Ally, {},
						[&](CUnit* internalUnit) {
						innerCount++;
					});
					//scbw::printf("Count is %d", innerCount);
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
		}*/
}
