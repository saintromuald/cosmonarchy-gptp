//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/scbwdata.h"
#include "hooks/hydraFinder.h"
#include <SCBW/UnitFinder.h>
#include "SCBW/api.h"
#include "globals.h"
#include "MathUtils.h"
#include "Utils/captivating_claws.h"

namespace hooks {

	double getSpeedMultiplier(CUnit* unit) {
		double speed = 1;

		// Set additives based on status effects and movespeed upgrades
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);

		if (decrease > 0) {
			for (int i = 0; i < decrease; i++) {
				speed *= 0.5;
			}
		}

		// Mind Tyrant Malediction: Apply movespeed slow to non-organic targets
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) > 1) {
			speed *= 0.5;
		}

		// Lobotomy Mine slow
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 1) {
			speed *= 0.5;
		}

		// Vassal Terminal Surge: increase movespeed when any nearby Simulacrum dies
		if (units_dat::BaseProperty[unit->id] & UnitProperty::RoboticUnit) {
			double stacks = MathUtils::clamp<double>(scbw::get_generic_value(unit, ValueId::TerminalSurge), 0, 10) * 1.5;
			if (stacks > 0) {
				speed *= 1.0 + stacks * 0.1;
			}
		}

		// Nathrokor Adrenal Frenzy: increase movespeed of all nearby organic allies after kill
		if (scbw::get_generic_timer(unit, ValueId::NathrokorTimer) > 0) {
			speed *= 1.5;
		}

		// Southpaw Knockout Drivers: reduces movement speed by 5% per stack
		if (scbw::get_generic_value(unit, ValueId::KnockoutDriversStacks) > 0) {
			uint32 knockoutValue = scbw::get_generic_value(unit, ValueId::KnockoutDriversStacks);
			speed -= speed * (0.05 * knockoutValue);
		}

		// Vorvaling Captivating Claws: reduces movement speed by 25% per unique attacker
		if (captivatingClawsGetTimer(unit) > 0) {
			speed -= speed * (0.25 * captivatingClawsGetStacks(unit));
		}

		// Swarming Omen ability: 150% movement speed when moving towards Zoryusthaleth
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic) {
			auto r = 10 * 32;
			auto n = unitRangeCount(unit, CFlags::Ally | CFlags::FrontalArc, {UnitId::ZergZoryusthaleth});
			scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
				unit->position.x + r, unit->position.y + r);
			bool accelerate = false;
			for (int i = 0; i < unitsInFinder.getUnitCount(); i++) {
				auto picked_unit = unitsInFinder.getUnit(i);
				if (n(picked_unit)) {
					if (scbw::get_generic_timer(picked_unit, ValueId::SwarmingOmenTimer) > 0) {
						accelerate = true;
						scbw::set_generic_timer(unit, ValueId::SwarmingOmenStatus, 25);
						break;
					}
				}
			}
			if (accelerate) {
				speed *= 1.5;
			}
		}

		switch (unit->id) {
			case UnitId::ZergUltrakor:
			{
				// Ultrakor Kaiser Rampage: gains 150% movespeed on kill for 1 second
				if (scbw::get_generic_timer(unit, ValueId::KaiserRampage) > 0) {
					speed *= 2;
				}
				break;
			}
			case UnitId::TerranWyvern:
			{
				// Wyvern Reverse Thrust ability: reduce movespeed
				if (scbw::get_generic_rust_value(unit, ValueId::ReverseThrust) == 1) {
					speed *= 0.5;
				}
				break;
			}
			case UnitId::ZergVorvaling:
			{
				// Vorvaling Adrenal Surge: gains double movespeed when moving towards enemies
				if (unit->id == UnitId::ZergVorvaling) {
					uint32 range = unit->getSightRange(false) * 32;
					scbw::UnitFinder unitFinder(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range
					);
					unitFinder.forEach([&](CUnit* target) {
						if (scbw::isAlliedTo(unit->playerId, target->playerId)
							|| units_dat::BaseProperty[target->id] & UnitProperty::Flyer) {
							return;
						}
						uint16 angle = scbw::getAngle(unit->position.x, unit->position.y,
							target->position.x, target->position.y);
						uint16 unit_angle = unit->velocityDirection1;
						int arc = (double)60 / 1.40625;
						// 120 degrees
						int delta = unit_angle - angle;
						delta = abs((delta + 128) % 256 - 128);
						if (delta <= arc) {
							unit->_padding_0x132 |= 0x2;
						}
					});
					if (unit->_padding_0x132 & 0x2) {
						speed *= 2;
						unit->_padding_0x132 &= ~0x2;
					}
				}
				break;
			}
			case UnitId::TerranVulture:
			{
				// Vulture Ion Propulsion: reduce movespeed, gains hover-mover status
				auto tile = scbw::getActiveTileAt(unit->getX(), unit->getY());
				if (tile.isUnwalkable || !tile.isWalkable) {
					if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::IonPropulsion)) {
						speed *= 0.5;
					}
				}
				break;
			}
			case UnitId::TerranEidolon:
			{
				// Eidolon Somatic Implants: gains 125% movespeed while cloaked
				if (unit->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection)) {
					speed *= 1.25;
					scbw::set_generic_value(unit, ValueId::SomaticImplants, 1);
				}
				break;
			}
			case UnitId::ProtossClarion:
			{
				// Clarion Harmony Drive: gains increased movement speed based on shields
				double fraction = (double)unit->shields / ((double)units_dat::MaxShieldPoints[unit->id] * 256);
				speed *= 1.0 + fraction;
				break;
			}
			case UnitId::ProtossManifold:
			{
				// Manifold Phase Rush: gains 150% increased movespeed when moving towards enemies attacked by allied robotics
				auto r = unit->getSightRange(false) * 32;
				auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
				scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
					unit->position.x + r, unit->position.y + r);
				bool accelerate = false;
				for (int i = 0; i < unitsInFinder.getUnitCount(); i++) {
					auto picked_unit = unitsInFinder.getUnit(i);
					if (n(picked_unit)) {
						for (int p = 0; p < 8; p++) {
							if (scbw::get_generic_timer(picked_unit, ValueId::PhaseRushTimerArrayStart + p) > 0 &&
								scbw::isAlliedTo(unit->playerId, p)) {
								accelerate = true;
								break;
							}
						}
					}
				}
				if (accelerate) {
					speed *= 1.5;
				}
				break;
			}
			case UnitId::TerranBlackjack:
			{
				// Blackjack Wear and Tear: reduce movement speed based on missing health
				const uint32 health = unit->hitPoints / 256;
				const uint32 maxHealth = unit->getMaxHpInGame();
				const double multiplier = MathUtils::map(health, (uint32)0, maxHealth, 0.66, 1.0, true); // nice casting tbh
				speed *= multiplier;
				break;
			}
			case UnitId::ProtossGolem:
			{
				// Golem Deathless Procession: gains increased movespeed based on missing shields
				const uint32 health = unit->shields / 256;
				const uint32 maxHealth = units_dat::MaxShieldPoints[unit->id];
				const double multiplier = MathUtils::map(health, maxHealth, maxHealth - maxHealth, 1.0, 1.5, true);
				speed *= multiplier;
				break;
			}
			case UnitId::ProtossExemplar:
			{
				// Khaydarin Eclipse ability: slow Exemplar by 75%
				if (scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch)) {
					speed *= 0.25;
				}
				break;
			}
			case UnitId::ZergZoryusthaleth:
			{
				// Zoryusthaleth Protective Instincts: 150% movement speed when moving towards wounded organic allies
				auto r = 10 * 32;
				auto n = unitRangeCount(unit, CFlags::Ally | CFlags::OnlyOrganic | CFlags::FrontalArc, {});
				scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
					unit->position.x + r, unit->position.y + r);
				bool accelerate = false;
				for (int i = 0; i < unitsInFinder.getUnitCount(); i++) {
					auto picked_unit = unitsInFinder.getUnit(i);
					if (n(picked_unit)) {
						if (picked_unit->getCurrentHpInGame() < picked_unit->getMaxHpInGame()) {
							accelerate = true;
							scbw::set_generic_timer(unit, ValueId::ProtectiveInstinctsStatus, 25);
							break;
						}
					}
				}
				if (accelerate) {
					speed *= 1.5;
				}
				break;
			}
			// Hardcode ideal movement speeds
			case UnitId::TerranCyprian:
			case UnitId::ProtossAmaranth:
			case UnitId::ProtossCabalist:
			{
				speed *= 1.2;
				break;
			}
			case UnitId::ProtossZealot:
			{
				speed *= 1.25;
				break;
			}
			case UnitId::ZergHydralisk:
			case UnitId::ProtossLegionnaire:
			{
				speed *= 1.5;
				break;
			}
		}

		return speed;
	}

	/// Calculates the unit's modified movement speed, factoring in upgrades and status effects.
	///
	/// @return		The modified speed value.
	/// Logically equivalent to GetModifiedUnitSpeed @ 0047B5F0  
	uint32 getModifiedUnitSpeedHook(CUnit* unit, uint32 baseSpeed) {
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);


		switch (unit->id) {
			case UnitId::ProtossPanoptus:
			{
				// Leftover movement speed code
				baseSpeed = 1707;
				break;
			}
			default:
			{
				// Flingy-related code
				if (increase > 0) {
					for (int i = 0; i < increase; i++) {
						baseSpeed *= 1.5;
					}
					if (baseSpeed < 853) {
						baseSpeed = 853;
					}
				}
			}
		}

		return baseSpeed * getSpeedMultiplier(unit);
	}

	/// Calculates the unit's acceleration, factoring in upgrades and status effects.
	///
	/// @return		The modified acceleration value.
	/// Logically equivalent to getModifiedUnitAcceleration @ 0047B8A0 
	uint32 getModifiedUnitAccelerationHook(CUnit* unit) {
		return std::max(scbw::getFlingyData(FlingyDatEntries::Acceleration, scbw::getUnitData(UnitsDatEntries::Flingy, unit->id)) * getSpeedMultiplier(unit), 1.0);
	}

	/// Calculates the unit's turn speed, factoring in upgrades and status effects.
	///
	/// @return		The modified turning speed value.
	/// Logically equivalent to getModifiedUnitTurnRadius @ 0047B850 
	uint32 getModifiedUnitTurnSpeedHook(CUnit* unit) {
		return std::max(scbw::getFlingyData(FlingyDatEntries::TurnRadius, scbw::getUnitData(UnitsDatEntries::Flingy, unit->id)) * getSpeedMultiplier(unit), 1.0);
	}

} //hooks
