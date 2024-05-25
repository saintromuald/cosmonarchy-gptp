#include "tech_target_check.h"
#include <BW/bwdata.h>
#include <BW/enumerations.h>
#include <BW/api.h>

namespace hooks
{
	u16 get_tech_use_error_message_hook(Unit* target, u8 player, u16 tech_id) // 00491E80
	{
		if (target->stasis_timer != 0) {
			return 900; // Units in stasis can't be targeted.<0>
		} else {
			// GPTP mongomin were here
			switch (tech_id) {
				case TechId::Tyranny: {
					if (target->player == player) {
						return 1327; // Must target enemy units<0>
					} else if (units_dat::base_property[target->id] & UnitProperty::Building) {
						return 877; // Unable to target structure.<0>
					} else if (unit_id_is_trap(target->id) || !unit_id_is_valid_for_most_abilities(target->id)) {
						return 876; // Invalid target.<0>
					} else {
						return 0; // valid target
					}
				} break;
				case TechId::DefensiveMatrix:
				case TechId::LuminaryAuralAssault: {
					if (units_dat::base_property[target->id] & UnitProperty::Building) {
						return 877; // Unable to target structure.<0>
					} else {
						return 0; // valid target
					}
				} break;
				case TechId::Consume: {
					if (units_dat::base_property[target->id] & UnitProperty::Building || !bw::is_allied_to(target->player, player) || bw::is_zerg_egg_new(target)) {
						return 876; // Invalid target.<0>
					} else {
						return 0; // valid target
					}
				} break;
				case TechId::TalosSalvage: {
					if (!target
						|| !bw::is_allied_to(target->player, player)
						|| !(target->status & UnitStatus::Completed)) {
						return 876; // Invalid target.<0>
					} else {
						return 0; // valid target
					}
				} break;
				case TechId::ControlledDemolition:
				case TechId::EmergencyDetonation:
				case TechId::Irradiate:
				case TechId::Hallucination:
				case TechId::PsionicStorm:
				{
					return 0;
				} break;
				case TechId::ZergBlissfulEmbrace:
				{
					if (target->player == player) {
						return 1327; // Must target enemy units<0>
					}
					if (unit_id_is_trap(target->id) || !unit_id_is_valid_for_most_abilities(target->id)) {
						return 876; // Invalid target.<0>
					} else {
						return 0; // valid target
					}
				} break;
				default: {
					return 876; // Invalid target.<0>
				}
			}
		}
		return 876; // Invalid target.<0>
	}
}