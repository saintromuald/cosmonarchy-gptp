//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/scbwdata.h"

#include <SCBW/UnitFinder.h>
#include "SCBW/api.h"
#include "logger.h"

#include "MathUtils.h"

class vorvalingProc : public scbw::UnitFinderCallbackProcInterface
{
private:
	CUnit* vorvaling;

public:
	vorvalingProc(CUnit* vorvaling)
		: vorvaling(vorvaling) {}
	void proc(CUnit* unit)
	{
		if (scbw::isAlliedTo(unit->playerId, vorvaling->playerId)) {
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
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);
		if (unit->id == UnitId::ZergUltralisk && scbw::getUpgradeLevel(unit->playerId, UpgradeId::KaiserRampage)) {
			if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KaiserRampage, 0) > 0) {

				speed += speed;
			}
		}
		if (increase > 0) {

			if (unit->id == UnitId::ProtossScout || unit->id == UnitId::Hero_Mojo || unit->id == UnitId::Hero_Artanis)
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
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MaledictionSlow, 0) > 1) {
			speed /= 2;
		}
		if (unit->id == UnitId::TerranWyvern && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::ReverseThrust, 0) == 1) {
			if(scbw::get_aise_value(unit,NULL,AiseId::GenericValue,ValueId::ReverseThrust,0)==1)
				speed /= 2;
		}
		if (unit->id == UnitId::ZergVorvaling) {//vorvaling
			vorvalingProc Accelerate = vorvalingProc(unit);
			u32 range = unit->getSightRange(false) * 32;
			scbw::UnitFinder unitFinder(unit->getX() - range,
				unit->getY() - range,
				unit->getX() + range,
				unit->getY() + range);
			unitFinder.forEach(Accelerate);
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::AdrenalSurge)) {

				if (unit->_padding_0x132 & 0x2) {
					//speed += (speed / 2);
					speed *= 2;
					unit->_padding_0x132 &= ~0x2;
				}
				else {
					//speed += (speed / 4);
				}
			}

		}
		else if (unit->id == UnitId::TerranGhost) {
			if (
				unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))
			{
				if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::SomaticImplants)) {
					speed += speed / 2;
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SomaticImplants, 1, 0);
				}
			}
		}
		else if (unit->id == UnitId::ProtossLegionnaire) { // Legionnaire 1.5x speed from Leg Enhancements
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::LegEnhancements)) {
				speed += speed / 4;
			}
		}
		else if (unit->id == UnitId::ProtossDarkTemplar) { // Dark Templar 1.5x speed from Leg Enhancements
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::LegEnhancements)) {
				speed += speed / 4;
			}
		}
		else if (unit->id == UnitId::civilian) {
			if (unit->_padding_0x132 & 0x8) {
				speed /= 2;
			}
		}
		else if (unit->id == UnitId::TerranCyprian) { // cyprian
			if (scbw::get_aise_value(unit, NULL, AiseId::SendCyprianValue, 0, 0) == 1) {
				speed -= (speed / 3); // safety off

			}
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::PlasteelServos)) {
				speed += (speed / 4);
			}
		}
		else if (unit->id == UnitId::ZergOverlord) {
			u32 lair_count = unitCountTable[UnitId::ZergLair].player[unit->playerId];
			u32 hive_count = unitCountTable[UnitId::ZergHive].player[unit->playerId];
			if (hive_count) {
				speed *= 6;

			}
			else if (lair_count) {
				speed *= 4;
			}
		}
		else if (unit->id == UnitId::ZergInfestedTerran) {
			u32 health = unit->hitPoints / 256;
			u32 maxHealth = unit->getMaxHpInGame();
			double multiplier = MathUtils::map(health, maxHealth, maxHealth / 2, 1.0, 1.5, true);
			speed = (u32) ((double)speed * multiplier);
			// GPTP::logger << "Death March: " << '\n';
			// GPTP::logger << "Health: " << health << '\n';
			// GPTP::logger << "Max Health: " << maxHealth << '\n';
			// GPTP::logger << "Multiplier: " << multiplier << '\n';
			// GPTP::logger << "Speed: " << speed << '\n' << '\n';
		}

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
		int increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		int decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);

		if (increase > 0)
			acceleration *= 2;
		//else
		if (decrease > 0)
			acceleration = acceleration - (acceleration / 4);
		if (unit->id == UnitId::TerranDropship && scbw::getUpgradeLevel(unit->playerId, UpgradeId::PassengerInsurance)) {
			acceleration *= 3;
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
