#include "max_energy.h"
#include <BW/api.h>

namespace hooks {
	// Replaces the Unit::get_max_energy() function. Returns the amount of maximum energy that a unit can have.
	// Note: 1 energy displayed in-game equals 256 energy, and energy cannot be more than 255.
	u16 get_unit_max_energy(Unit* const unit) { // 00491870 
		u32 energy = 200;
		switch (unit->id) {
			case UnitId::AskosiEyelet:
			case UnitId::AskosiVicar: {
				energy = 50;
			} break;
			case UnitId::AskosiZonguard: {
				energy = 75;
			} break;
			case UnitId::TerranPietyWard:
			case UnitId::TerranAutocrat:
			case UnitId::TerranPhobos:
			case UnitId::TerranFacade:
			case UnitId::ProtossDemiurge:
			case UnitId::AskosiCardinal:
			case UnitId::AskosiRove:
			case UnitId::AskosiElgird:
			case UnitId::AskosiHistrion: {
				energy = 100;
			} break;
			case UnitId::TerranAncile:
			case UnitId::ProtossXenolith:
			case UnitId::AskosiParaclete: {
				energy = 125;
			} break;
			case UnitId::TerranApostle:
			case UnitId::TerranHypnagogue:
			case UnitId::TerranSeraph:
			case UnitId::TerranLuminary:
			case UnitId::ProtossAnalogue:
			case UnitId::AskosiProselyte: {
				energy = 150;
			} break;
			case UnitId::TerranTriton: {
				energy = 175;
			} break;
			case UnitId::TerranAnticthon:
			case UnitId::TerranAion:
			case UnitId::TerranSerathiel:
			case UnitId::ProtossMindTyrant: {
				energy = 200;
			} break;
			case UnitId::TerranArchbishop:
			case UnitId::ProtossCantavis:
			case UnitId::ProtossSaladius:
			case UnitId::ExtTassadar:
			case UnitId::Goska: {
				energy = 250;
			} break;
		}
		return std::min(energy * 256, (u32)USHRT_MAX);
	}
}