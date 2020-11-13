#include "hooks\hydra_unit_ai.h"


void hydra_unit_ai(CUnit* unit) {
/*
Apostle Lazarus Agent: If 5 or more organic allies (that don't have the buff) are nearby, cast the spell. **DONE**
				AI should also try to position Apostles near organics, but not sure if that's feasible.
Ghost Lobotomy Mine: I think all mines should be reworked in terms of how the AI places them, 
					I would suggest dropping them in positions where enemy ground units 
					have been spotted/attacked/killed within the last few minutes. 
					That way it's like they're responding to ground paths taken by enemies. 
					Vulture Spider Mines could be done in similar ways.
Savant Power Siphon: If 2 or more allied specialists are nearby, target an enemy specialist within 7 range.

Paladin Skyfall: If 6 or more enemies are within 64px of each other while being within range, target terrain between the Paladin and the target group, 64px away from the group.
Science Vessel Observance: If 3 or more enemies are within 96px of each other, and the Vessel has more than 50% HP, target the group and stay within range while attempting to avoid damage.
Azazel Sublime Shepherd: Toggle while 2 or more allied units are within 96px of the Azazel, and stay within 96px of as many allies as possible.
Clarion Phase Link: While 4 or more allied shielded units are within 128px of the Clarion, toggle the ability on.
Arbiter Recall: "Offensive" cast when the Arbiter is added to an attack and a group of 6 or more units are within 160px of one another. Attempt to group as many units as possible during the channel.
Star Sovereign Grief of All Gods: When at least 2000 HP+shields worth of enemy units are within 320px of the Star Sovereign and there are no allied buildings and less than 2000 HP+shields worth of allied units within the same radius, and the Star Sovereign has more than 50% of HP+shields combined, channel the ability and attempt to protect the Star Sovereign with low-cost/low-HP units.
Queen Parasite: When 5 or more supply's worth of units (of any owner) are within 128px of each other, target the group.
Sometimes I get this crash when closing the exe, not sure if this is related to veeq's GPTP config or something else
*/
	if (unit->id == UnitId::TerranApostle) {
		if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent] * 256) &&
			completed_unit_counts[UnitId::TerranScienceFacility].player[unit->playerId] > 0 &&
			scbw::get_generic_timer(unit,ValueId::AI_LazarusTimer)==0) 
		{
			int count = 0;
			hydraFinderLoop(NULL,unit->position,128, CFlags::NoBuilding | CFlags::Ally |
				CFlags::OnlyOrganic, {},[&](CUnit* u)
			{
				if (scbw::get_generic_value(u, ValueId::LazarusAgent) != 1) {
					count++;
				}
			});
			if (count >= 5) {
				scbw::set_generic_value(unit, ValueId::CastLazarusAgent, 1);
				scbw::set_generic_timer(unit, ValueId::AI_LazarusTimer, 24);
			}
		}
	}
	else if (unit->id == UnitId::TerranGhost) {
		//mines
	}
	else if (unit->id == UnitId::TerranSavant) {
		if (scbw::get_generic_timer(unit, ValueId::AI_SiphonTimer) == 0 && unit->mainOrderId!=OrderId::PowerSiphon) {
			int count = 0;
			hydraFinderLoop(unit, unit->position, weapons_dat::OuterSplashRadius[WeaponId::PowerSiphon],
				CFlags::Ally | CFlags::Spellcaster, {}, [&](CUnit* u) {
					if (u->energy < u->getMaxEnergy()) {
						count++;
					}
				});
			if (count >= 2) {
				//Savant Power Siphon: If 2 or more allied specialists are nearby, target an enemy specialist within 7 range.
				auto enemy = rangeMatchUnit(unit, 7 * 32, CFlags::Enemy | CFlags::Spellcaster | CFlags::HasEnergy, {});
				if (enemy != NULL) {
					unit->orderTo(OrderId::PowerSiphon, enemy);
					scbw::set_generic_timer(unit, ValueId::AI_SiphonTimer, 3 * 32);
				}
			}
		}	
	}
}
