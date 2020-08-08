#include "max_energy.h"
#include <SCBW/api.h>

const int ENERGY_INGAME_250 = 64000;
const int ENERGY_INGAME_200 = 51200;

namespace hooks {

/// Replaces the CUnit::getMaxEnergy() function.
/// Return the amount of maximum energy that a unit can have.
/// Note: 1 energy displayed in-game equals 256 energy.
/// Should be equivalent to GetUnitMaxEnergy @ 00491870  
u16 getUnitMaxEnergyHook(CUnit* const unit) {

	if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
		return ENERGY_INGAME_250;

	switch (unit->id) {
		case UnitId::TerranScienceVessel:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::TitanReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranAzazel:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::AchernusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranGhost:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::MoebiusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranWraith:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ApolloReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranBattlecruiser:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ColossusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ZergQueen:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::GameteMeiosis))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ZergDefiler:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::MetasynapticNode))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ProtossHighTemplar:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::Ascendancy))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ProtossArbiter:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinCore))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ProtossCorsair:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ArgusJewel))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranMedic:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::CaduceusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::TerranComsatStation:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::OdysseusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ProtossShieldBattery:
		case UnitId::ProtossBulwark:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinGenerator))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ZergChasmalColony:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::SynapticGrowth))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ProtossDarkArchon:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ArgusTalisman))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ZergGorgoleth:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::VitalEncephalon))
				return ENERGY_INGAME_250;
			break;			
	}

	return ENERGY_INGAME_200;
}

} //hooks
