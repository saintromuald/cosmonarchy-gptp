#include "tech_target_check.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>

namespace hooks
{
	// Should be equivalent to getSpellStatString @ 0x00491E80
	u16 getTechUseErrorMessageHook(Unit* target, u8 castingPlayerId, u16 techId)
	{
		u16 stringId;
		if (target->stasisTimer != 0)
		{
			stringId = 900; // Units in stasis can't be targeted.<0>
		}
		else if (techId != TechId::PowerSiphon && (techId < TechId::DefensiveMatrix || techId > TechId::OpticalFlare))
		{
			stringId = 876; // Invalid target.<0>
		}
		else
		{
			// originally handled by switches and internal arrays of id TechId::DisruptionWeb
			// probably changed from switches to this bullshit by GPTP mongomin
			if (techId == TechId::Feedback)
			{
				if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				{
					stringId = 877; // Unable to target structure.<0>
				}
				else if (!target->isValidCaster())
				{
					stringId = 1330; // Must target units with energy.<0>
				}
				else
				{
					stringId = 0; // valid target
				}
			}
			else if (techId == TechId::MindControl)
			{
				if (target->playerId == castingPlayerId)
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
			}
			else if (techId == TechId::Hallucination)
			{
				if (target->id == UnitId::ProtossSteward)
				{
					stringId = 876; // Invalid target.<0>
				}
				else if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				{
					stringId = 877; // Unable to target structure.<0>
				}
				else if (scbw::isAlliedTo(target->playerId, castingPlayerId) || target->playerId == castingPlayerId)
				{
					stringId = 1327; // Must target enemy units<0>
				}
				else
				{
					stringId = 0; // valid target
				}
			}
			else if (techId == TechId::DefensiveMatrix
					|| techId == TechId::Irradiate
					|| techId == TechId::Restoration
					|| techId == TechId::OpticalFlare)
			{
				if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				{
					stringId = 877; // Unable to target structure.<0>
				}
				else
				{
					stringId = 0; // valid target
				}
			}
			else if (techId == TechId::Infestation)
			{
				if (!helper::isInfestableUnit(target))
				{
					stringId = 870;	// Must target severely damaged Terran Command Center.<0>
				}
				else
				{
					stringId = 0; // valid target
				}
			}
			else if (techId == TechId::Consume)
			{
				if (units_dat::BaseProperty[target->id] & UnitProperty::Building
					|| target->playerId != castingPlayerId
					|| target->getRace() != RaceId::Zerg
					|| scbw::isZergEggNew(target))
				{
					stringId = 876; // Invalid target.<0>
				}
				else
				{
					stringId = 0; // valid target
				}

			}
			else if (techId == TechId::PowerSiphon)
				{
					stringId = 0;
				}
			else
			{
				stringId = 876; // Invalid target.<0>
			}
		}
		if (techId == TechId::UnusedTech39)
		{
			stringId = 0;
		}
		if (techId == TechId::Lockdown)
		{
			// removed
		}
		return stringId;
	}
}