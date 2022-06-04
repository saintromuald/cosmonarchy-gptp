#include "tech_target_check.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>

namespace hooks
{
	u16 getTechUseErrorMessageHook(Unit* target, u8 castingPlayerId, u16 techId) // 00491E80
	{
		u16 stringId;
		if (target->stasis_timer != 0)
		{
			stringId = 900; // Units in stasis can't be targeted.<0>
		}
		else if (techId == TechId::EmergencyDetonation)
		{
			return 0;
		}
		else if (techId < TechId::DefensiveMatrix || techId > TechId::OpticalFlare)
		{
			stringId = 876; // Invalid target.<0>
		}
		else
		{
			// GPTP mongomin were here
			switch (techId) {
				case TechId::Feedback: {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					{
						stringId = 877; // Unable to target structure.<0>
					}
					else if (!target->is_valid_caster())
					{
						stringId = 1330; // Must target units with energy.<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::MindControl: {
					if (target->player == castingPlayerId)
					{
						stringId = 1327; // Must target enemy units<0>
					}
					else if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					{
						stringId = 877; // Unable to target structure.<0>
					}
					else if (target->id == UnitId::TerranSpiderMine
						|| target->id == UnitId::TerranLobotomyMine
						|| target->id == UnitId::ZergLarva
						|| target->id == UnitId::ZergEgg
						|| target->id == UnitId::ZergCocoon
						|| target->id == UnitId::ProtossSteward
						|| target->id == UnitId::ProtossPlasmaShell)
					{
						stringId = 876; //Invalid target.<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::Hallucination: {
					if (target->id == UnitId::ProtossSteward)
					{
						stringId = 876; // Invalid target.<0>
					}
					else if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					{
						stringId = 877; // Unable to target structure.<0>
					}
					else if (scbw::is_allied_to(target->player, castingPlayerId) || target->player == castingPlayerId)
					{
						stringId = 1327; // Must target enemy units<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::DefensiveMatrix:
				case TechId::Irradiate:
				case TechId::Restoration:
				case TechId::OpticalFlare: {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					{
						stringId = 877; // Unable to target structure.<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::Infestation:
				{
					if (!helper::is_infestable_unit(target))
					{
						stringId = 870;	// Must target severely damaged Terran Command Center.<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::Consume: {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building
						|| target->player != castingPlayerId
						|| target->get_race() != RaceId::Zerg
						|| scbw::is_zerg_egg_new(target))
					{
						stringId = 876; // Invalid target.<0>
					}
					else
					{
						stringId = 0; // valid target
					}
				} break;
				case TechId::TalosSalvage: {
					if (!target
						|| !scbw::is_allied_to(target->player, castingPlayerId)
						|| !(target->status & UnitStatus::Completed)) {
						stringId = 876; // Invalid target.<0>
					}
					else {
						stringId = 0;
					}
				} break;
				default: {
					stringId = 876; // Invalid target.<0>
				}
			}
		}
		return stringId;
	}
}