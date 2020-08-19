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

		// Kaiser Rampage upgrade: gains 150% movespeed on kill for 1 second
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

		// Malediction upgrade: Apply movespeed slow to Maelstrom victims once stun wears off
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MaledictionSlow, 0) > 1) {
			speed /= 2;
		}

		// Reverse Thrust ability: reduce movespeed
		if (unit->id == UnitId::TerranWyvern && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::ReverseThrust, 0) == 1) {
			if(scbw::get_aise_value(unit,NULL,AiseId::GenericValue,ValueId::ReverseThrust,0)==1)
				speed /= 2;
		}

		// Adrenal Surge upgrade: gains double movespeed when moving towards enemies
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

		// Ion Propulsion upgrade: reduce movespeed, gains hover-mover status
		else if (unit->id == UnitId::TerranVulture) {
			auto tile = scbw::getActiveTileAt(unit->getX(), unit->getY());
			if (tile.isUnwalkable || !tile.isWalkable) {
				if (scbw::getUpgradeLevel(unit->playerId,UpgradeId::IonPropulsion)) {
					speed /= 2;
				}
			}
		}

		// Somatic Implants: gains 125% movespeed while cloaked
		else if (unit->id == UnitId::TerranGhost) {
			if (
				unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))
			{
				speed += speed / 4;
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SomaticImplants, 1, 0);
			}
		}

		// Leg Enhancements upgrade (Zealot): gains 125% movespeed
		else if (unit->id == UnitId::ProtossZealot) {
			speed += speed / 2;
		}

		// Leg Enhancements upgrade (Legionnaire): gains 150% movespeed
		else if (unit->id == UnitId::ProtossLegionnaire) {
			speed += speed / 2;
		}

	#ifdef HYDRA
		// Leg Enhancements upgrade (Dark Templar): gains 120% movespeed
		else if (unit->id == UnitId::ProtossDarkTemplar) {
			speed += speed / 5;
		}
	#endif

		// Activate Medstims ability: reduce movespeed
		else if (unit->id == UnitId::TerranShaman) {
			if (unit->_padding_0x132 & 0x8) {
				speed /= 2;
			}
		}

		// Safety Off ability: reduce movespeed
		else if (unit->id == UnitId::TerranCyprian) {
			if (scbw::get_aise_value(unit, NULL, AiseId::SendCyprianValue, 0, 0) == 1) {
				speed -= (speed / 3); // safety off
			}
		}

		// Collective Integration passive: gains increased movespeed based on tech level
		else if (unit->id == UnitId::ZergOverlord || unit->id == UnitId::ZergIroleth || unit->id==UnitId::ZergOthstoleth) {
			u32 irisCountSpeed = unitCountTable[UnitId::ZergOverlordIris].player[unit->playerId];
			u32 quayCountSpeed = unitCountTable[UnitId::ZergIrolethQuay].player[unit->playerId];
			u32 berthCountSpeed = unitCountTable[UnitId::ZergOthstolethBerth].player[unit->playerId];
			if (berthCountSpeed) {
				speed *= 6;

			}
			if (quayCountSpeed) {
				speed *= 5;

			}
			else if (irisCountSpeed) {
				speed *= 4;
			}
		}

		// Death March passive: gains increased movespeed based on missing health
		else if (unit->id == UnitId::ZergInfestedTerran) {
			u32 health = unit->hitPoints / 256;
			u32 maxHealth = unit->getMaxHpInGame();
			double multiplier = MathUtils::map(health, maxHealth, maxHealth / 2, 1.0, 1.5, true);
			speed = (u32) ((double)speed * multiplier);
		}

		// Knockout Drivers upgrade: reduces movement speed by 5% per stack
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::KnockoutDriversValue, 0) > 0) {
			u32 knockoutValue = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::KnockoutDriversValue, 0);
			speed -= speed * (0.05 * knockoutValue);
		}

		// Captivating Claws upgrade: reduces movement speed by 25% per unique attacker
		u32 captivatingClawsStacks = captivating_claws::getCaptivatingClawsStacks(unit);
		if (captivatingClawsStacks > 0) {
			speed -= speed * (0.25 * captivatingClawsStacks);
		}

	#ifdef DELIVERANCE
		// Deathless Procession upgrade: gains 125% movespeed, increasing to 175% when above 50% shields
		if unit->id == UnitId::ProtossAmaranth) {
			if (unit->getCurrentShieldsInGame() <= units_dat::MaxShieldPoints[unit->id] / 2)
				speed += speed / 4;
			else
				speed += (speed / 2) + speed / 4;
		}

		u32 astral_tempest_stacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::AstralTempestStacks, 0);
		else if (astral_tempest_stacks) {
			speed *= 1 + astral_tempest_stacks * 0.05;
		}
	#endif

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
		u32 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, units_dat::Graphic[unit->id]);
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

		// Collective Integration passive: gains increased acceleration based on tech level
		else if (unit->id == UnitId::ZergOverlord || unit->id == UnitId::ZergIroleth) {
			u32 irisCountAccel = unitCountTable[UnitId::ZergOverlordIris].player[unit->playerId];
			u32 quayCountAccel = unitCountTable[UnitId::ZergIrolethQuay].player[unit->playerId];
			u32 berthCountAccel = unitCountTable[UnitId::ZergOthstolethBerth].player[unit->playerId];
			if (berthCountAccel) {
				acceleration *= 4;

			}
			if (quayCountAccel) {
				acceleration *= 3;

			}
			else if (irisCountAccel) {
				acceleration *= 2;
			}
		}

		return acceleration;


	}

	/// Calculates the unit's turn speed, factoring in upgrades and status effects.
	///
	/// @return		The modified turning speed value.
	/// Logically equivalent to getModifiedUnitTurnRadius @ 0047B850 
	u32 getModifiedUnitTurnSpeedHook(CUnit* unit) {


		//u32 turnSpeed = flingy_dat::TurnSpeed[units_dat::Graphic[unit->id]];
		u32 turnSpeed = scbw::getFlingyData(FlingyDatEntries::TurnRadius, units_dat::Graphic[unit->id]);
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
