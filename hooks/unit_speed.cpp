//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/scbwdata.h"

#include <SCBW/UnitFinder.h>
#include "SCBW/api.h"

#include "MathUtils.h"
#include "Utils/captivating_claws.h"

class vorvalingProc : public scbw::UnitFinderCallbackProcInterface
{
private:
	CUnit* vorvaling;

public:
	vorvalingProc(CUnit* vorvaling)
		: vorvaling(vorvaling) {}
	void proc(CUnit* unit)
	{
		if (scbw::isAlliedTo(vorvaling->playerId, unit->playerId)) {
			return;
		}
		u16 angle = scbw::getAngle(vorvaling->position.x, vorvaling->position.y,
			unit->position.x, unit->position.y);
		u16 unit_angle = vorvaling->velocityDirection1;
		int arc = (double)60 / 1.40625;
		//120 degrees
		int delta = unit_angle - angle;
		delta = abs((delta + 128) % 256 - 128);
		if (delta <= arc) {
			vorvaling->_padding_0x132 |= 0x2;
		}
	}
};


namespace hooks {

	/// Calculates the unit's modified movement speed, factoring in upgrades and status effects.
	///
	/// @return		The modified speed value.
	/// Logically equivalent to GetModifiedUnitSpeed @ 0047B5F0  
	u32 getModifiedUnitSpeedHook(CUnit* unit, u32 baseSpeed) {
		u32 speed = baseSpeed;
		/*
		int speedModifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)-(unit->unusedTimer ? 1 : 0)
		+ (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);*/

		// Set additives based on status effects and movespeed upgrades
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);

		// Ultralisk Kaiser Rampage: gains 150% movespeed on kill for 1 second
		if (unit->id == UnitId::ZergUltralisk) {
			if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KaiserRampage, 0) > 0) {
				speed += speed;
			}
		}

		// Flingy-related code
		if (increase > 0) {

			if (unit->id == UnitId::ProtossPanoptus || unit->id == UnitId::Hero_Mojo || unit->id == UnitId::Hero_Artanis)
				speed = 1707;
			else {
				for (int i = 0; i<increase; i++) {
					speed += (speed / 2);
				}
				if (speed < 853)
					speed = 853;
			}
		}

		if (decrease > 0) {
			for (int i = 0; i<decrease; i++) {
				speed /= 2;
			}
		}

		// Dark Archon Malediction: Apply movespeed slow to Maelstrom victims once stun wears off
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MaledictionSlow, 0) > 1) {
			speed /= 2;
		}

		// Lobotomy Mine slow
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::LobotomySlow, 0) > 1) {
			speed /= 2;
		}
		// Wyvern Reverse Thrust ability: reduce movespeed
		if (unit->id == UnitId::TerranWyvern && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::ReverseThrust, 0) == 1) {
			speed /= 2;
		}

		// Vorvaling Adrenal Surge: gains double movespeed when moving towards enemies
		if (unit->id == UnitId::ZergVorvaling) {
			u32 range = unit->getSightRange(false) * 32;
			scbw::UnitFinder unitFinder(unit->getX() - range,
				unit->getY() - range,
				unit->getX() + range,
				unit->getY() + range);
			unitFinder.forEach(vorvalingProc(unit));
			if (unit->_padding_0x132 & 0x2) {
				//speed += (speed / 2);
				speed *= 2;
				unit->_padding_0x132 &= ~0x2;
			}
			else {
				//speed += (speed / 4);
			}
		}

		// Vulture Ion Propulsion: reduce movespeed, gains hover-mover status
		else if (unit->id == UnitId::TerranVulture) {
			auto tile = scbw::getActiveTileAt(unit->getX(), unit->getY());
			if (tile.isUnwalkable || !tile.isWalkable) {
				if (scbw::getUpgradeLevel(unit->playerId,UpgradeId::IonPropulsion)) {
					speed /= 2;
				}
			}
		}

		// Ghost Somatic Implants: gains 125% movespeed while cloaked
		else if (unit->id == UnitId::TerranGhost) {
			if (
				unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))
			{
				speed += speed / 4;
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SomaticImplants, 1, 0);
			}
		}

		// Leftover movement speed code
		else if (unit->id == UnitId::ProtossPanoptus
		|| unit->id == UnitId::Hero_Mojo
		|| unit->id == UnitId::Hero_Artanis)
			speed = 1707;

		// Muscular Augment (Hydralisk): gains 150% movespeed
		else if (unit->id == UnitId::ZergHydralisk) {
			speed += speed / 2;
		}

		// Leg Enhancements (Zealot): gains 125% movespeed
		else if (unit->id == UnitId::ProtossZealot) {
			speed += speed / 4;
		}

		// Leg Enhancements (Legionnaire): gains 150% movespeed
		else if (unit->id == UnitId::ProtossLegionnaire) {
			speed += speed / 2;
		}

		// Leg Enhancements (Dark Templar): gains 120% movespeed
		else if (unit->id == UnitId::ProtossDarkTemplar) {
			speed += speed / 5;
		}

		// Shaman Activate Medstims / Nanite Field abilities: reduce movespeed
		else if (unit->id == UnitId::TerranShaman) {
			if (unit->_padding_0x132 & 0x8 || scbw::get_generic_value(unit, ValueId::NaniteField)) {
				speed /= 2;
			}
		}

		// Cyprian: hardcode ideal movement speed
		else if (unit->id == UnitId::TerranCyprian) {
			speed += (speed / 5);
		}

		// Simulacrum Autovitality: increase movespeed when any nearby Simulacrum dies
		else if (unit->id == UnitId::ProtossSimulacrum) {
			double stacks = MathUtils::clamp<double>(scbw::get_generic_value(unit, ValueId::SimulacrumStacks), 0, 10)*1.5;
			if (stacks > 0) {
				speed = (double)speed * ((10.0 + stacks) / 10.0);
			}
		}

		// Nathrokor Adrenal Frenzy: increase movespeed of all nearby organic allies after kill
		else if (scbw::get_generic_timer(unit, ValueId::NathrokorTimer) > 0) {
			speed += speed / 2;
//			scbw::printFormattedText("SPEED | Unit id: %d", (u32)unit->id);
		}

		// Collective Integration: gains increased movespeed based on tech level
/*		else if (unit->id == UnitId::ZergOverlord || unit->id == UnitId::ZergIroleth || unit->id==UnitId::ZergOthstoleth) {
			u32 irisCountSpeed = unitCountTable[UnitId::ZergOverlordIris].player[unit->playerId];
			u32 quayCountSpeed = unitCountTable[UnitId::ZergIrolethQuay].player[unit->playerId];
			u32 berthCountSpeed = unitCountTable[UnitId::ZergOthstolethBerth].player[unit->playerId];
			if (berthCountSpeed) {
				speed *= 6;
			}
			else if (quayCountSpeed) {
				speed *= 5;
			}
			else if (irisCountSpeed) {
				speed *= 4;
			}
		}
*/
		// Infested Terran Death March: gains increased movespeed based on missing health
		else if (unit->id == UnitId::ZergInfestedTerran) {
			u32 health = unit->hitPoints / 256;
			u32 maxHealth = unit->getMaxHpInGame();
			double multiplier = MathUtils::map(health, maxHealth, maxHealth / 2, 1.0, 1.5, true);
			speed = (u32) ((double)speed * multiplier);
		}

		// Golem Deathless Procession: gains increased movespeed based on missing shields
		else if (unit->id == UnitId::ProtossGolem) {
			u32 health = unit->shields / 256;
			u32 maxHealth = units_dat::MaxShieldPoints[unit->id];
			double multiplier = MathUtils::map(health, maxHealth, maxHealth - maxHealth, 1.0, 1.5, true);
			speed = (u32) ((double)speed * multiplier);
		}

		// Southpaw Knockout Drivers: reduces movement speed by 5% per stack
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::KnockoutDriversValue, 0) > 0) {
			u32 knockoutValue = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::KnockoutDriversValue, 0);
			speed -= speed * (0.05 * knockoutValue);
		}

		// Vorvaling Captivating Claws: reduces movement speed by 25% per unique attacker
		u32 captivatingClawsStacks = captivating_claws::getCaptivatingClawsStacks(unit);
		if (captivatingClawsStacks > 0) {
			speed -= speed * (0.25 * captivatingClawsStacks);
		}

		// Khaydarin Eclipse ability: slow Exemplar by 75%
		if (unit->id == UnitId::ProtossExemplar && scbw::get_generic_value(unit, ValueId::ExemplarSwitch)) {
			speed = speed / 4;
		}

		// DEBUG
		// Speed logger
		/*
		if (unit->id == UnitId::probe) {
			GPTP::logger << "Speed comparison: " << baseSpeed << " - " << speed << ", Unit Id: " << unit->id << ", increase: " << increase << ", decrease: " << decrease << std::endl;
			GPTP::logger << "Ensnare timer: " << unit->ensnareTimer << '\n';
		}
		if (unit->id == UnitId::ProtossDarkTemplar) {
			GPTP::logger << "DT Speed comparison: " << baseSpeed << " - " << speed << ", Unit Id: " << unit->id << ", increase: " << increase << ", decrease: " << decrease << std::endl;

		}
		if (unit->id == UnitId::dark_archon) {
			GPTP::logger << "DA Speed comparison: " << baseSpeed << " - " << speed << ", Unit Id: " << unit->id << ", increase: " << increase << ", decrease: " << decrease << std::endl;

		}*/

		return speed;

	}

	/// Calculates the unit's acceleration, factoring in upgrades and status effects.
	///
	/// @return		The modified acceleration value.
	/// Logically equivalent to getModifiedUnitAcceleration @ 0047B8A0 
	u32 getModifiedUnitAccelerationHook(CUnit* unit) {


		//u32 acceleration = flingy_dat::Acceleration[units_dat::Graphic[unit->id]];
//		u32 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, units_dat::Graphic[unit->id]);
		u32 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, scbw::getUnitData(UnitsDatEntries::Flingy,unit->id));
		/*
		int modifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
		+ (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);*/

		// Set additives based on status effects and movespeed upgrades
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);

		// 
		if (increase > 0)
			acceleration *= 2;

		//else
		//
		if (decrease > 0)
			acceleration = acceleration - (acceleration / 4);

		// Collective Integration: gains increased acceleration based on tech level
/*		else if (unit->id == UnitId::ZergOverlord || unit->id == UnitId::ZergIroleth) {
			u32 irisCountAccel = unitCountTable[UnitId::ZergOverlordIris].player[unit->playerId];
			u32 quayCountAccel = unitCountTable[UnitId::ZergIrolethQuay].player[unit->playerId];
			u32 berthCountAccel = unitCountTable[UnitId::ZergOthstolethBerth].player[unit->playerId];
			if (berthCountAccel) {
				acceleration *= 4;

			}
			else if (quayCountAccel) {
				acceleration *= 3;

			}
			else if (irisCountAccel) {
				acceleration *= 2;
			}
		}
*/
		// Simulacrum Autovitality
		else if (unit->id == UnitId::ProtossSimulacrum) {
			double stacks = MathUtils::clamp<double>(scbw::get_generic_value(unit, ValueId::SimulacrumStacks), 0, 10)*1.5;
			if (stacks > 0) {
				acceleration = (double)acceleration * ((10.0 + stacks) / 10.0);
			}
		}

		// Nathrokor Adrenal Frenzy: increase acceleration of all nearby organic allies after kill
		else if (scbw::get_generic_timer(unit, ValueId::NathrokorTimer) > 0) {
			acceleration += acceleration / 2;
//			scbw::printFormattedText("ACCEL | Unit id: %d", (u32)unit->id);
		}

		// Khaydarin Eclipse ability: slow Exemplar by 75%
		if (unit->id == UnitId::ProtossExemplar && scbw::get_generic_value(unit, ValueId::ExemplarSwitch)) {
			acceleration = acceleration / 4;
		}
		return acceleration;


	}

	/// Calculates the unit's turn speed, factoring in upgrades and status effects.
	///
	/// @return		The modified turning speed value.
	/// Logically equivalent to getModifiedUnitTurnRadius @ 0047B850 
	u32 getModifiedUnitTurnSpeedHook(CUnit* unit) {


		//u32 turnSpeed = flingy_dat::TurnSpeed[units_dat::Graphic[unit->id]];
//		u32 turnSpeed = scbw::getFlingyData(FlingyDatEntries::TurnRadius, units_dat::Graphic[unit->id]);
		u32 turnSpeed = scbw::getFlingyData(FlingyDatEntries::TurnRadius, scbw::getUnitData(UnitsDatEntries::Flingy, unit->id));
		/*
		int modifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
		+ (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);*/
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);
		if (increase > 0)
			turnSpeed *= 2;

		//else if (modifier < 0)
		if (decrease > 0)
			turnSpeed = turnSpeed - (turnSpeed / 4);

		return turnSpeed;

	}

} //hooks
