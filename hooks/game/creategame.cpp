#include "creategame.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>

namespace hooks {
	// Hooked to eventually extend weapon ids past 255. -Pr0nogo
	void generate_strength_table() { // 004313C0
		for (u32 unit_id = 0; unit_id < UnitId::Count; unit_id++) {
			u32 air_strength = 0;
			u32 ground_strength = 0;
			if (!unit_id_is_larva(unit_id) && !unit_id_is_zerg_egg(unit_id)) {
				u32 final_unit = units_dat::subunit[unit_id];
				if (unit_id == UnitId::ProtossSolarion) {
					final_unit = UnitId::ProtossSteward;
				} else if (unit_id == UnitId::ProtossAccantorOld) {
					final_unit = UnitId::ProtossPlasmaShell;
				} else {
					if (final_unit == UnitId::None) {
						final_unit = unit_id;
					}
				}

				u32 air_weapon_id = units_dat::air_weapon[final_unit];
				if (air_weapon_id == WeaponId::None) {
					air_strength = 1;
				} else {
					u32 air_strength_calc = helper::calculate_unit_strength(unit_id, air_weapon_id);
					air_strength = helper::filter_unit_strength(unit_id, air_strength_calc);
				}

				u32 ground_weapon_id = units_dat::ground_weapon[final_unit];
				if (ground_weapon_id == WeaponId::None) {
					ground_strength = 1;
				} else {
					u32 ground_strength_calc = helper::calculate_unit_strength(unit_id, ground_weapon_id);
					ground_strength = helper::filter_unit_strength(unit_id, ground_strength_calc);
				}
			}
			u32 final_air_strength = 0;
			if (air_strength != 1 || ground_strength <= 1) {
				final_air_strength = air_strength;
			}
			g_ai_strength_air[unit_id] = final_air_strength;

			u32 final_ground_strength = ground_strength;
			if (ground_strength == 1 && air_strength > 1) {
				final_ground_strength = 0;
			}
			g_ai_strength_ground[unit_id] = final_ground_strength;
		}
	}

	// Hooked to correct unit id hardcodes. -Pr0nogo
	u32 filter_unit_strength(u32 unit_id, u32 strength) { // 00431150
		switch (unit_id) {
			case UnitId::TerranMason:
			case UnitId::ProtossScribe:
			case UnitId::ZergDroleth: {
				return strength >> 2;
			}
			case UnitId::TerranSpiderMine:
			case UnitId::TerranLobotomyMine:
			case UnitId::ProtossSteward:
			case UnitId::ProtossPlasmaShell: {
				return 0;
			}
			case UnitId::TerranHarakan:
			case UnitId::ProtossZealot:
			case UnitId::ZergMutalisk: {
				return 2 * strength;
			}
			case UnitId::ZergSkryrokor: {
				return strength >> 4;
			}
			case UnitId::ProtossAccantorOld: {
				return strength / 10;
			}
		}
		return strength;
	}
}