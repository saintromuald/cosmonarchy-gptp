//Source file for the Tech Target Check hook module.
#include "tech_target_check.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
//-------- Helper function declarations. Do NOT modify! --------//
namespace {
bool isInfestableUnit(CUnit* unit);
} //Unnamed namespace

namespace hooks {

//Should be equivalent to getSpellStatString @ 0x00491E80
u16 getTechUseErrorMessageHook(CUnit* target, u8 castingPlayerId, u16 techId) {


	u16 stringId;
	if(target->stasisTimer != 0)
		stringId = 900; //Units in stasis can't be targeted.<0>
	else
	if(techId!=TechId::PowerTransfer && techId!=TechId::PowerTransfer && (techId < TechId::DefensiveMatrix || techId > TechId::OpticalFlare))
		stringId = 876; //Invalid target.<0>
	else {

		//originally handled by switchs and internal arrays of id
		//TechId::DisruptionWeb
		
		if(techId == TechId::Feedback) {

			if(units_dat::BaseProperty[target->id] & UnitProperty::Building)
				stringId = 877; //Unable to target structure.<0>
			else
			if(!target->isValidCaster())
				stringId = 1330; //Must target units with energy.<0>
			else
				stringId = 0; //valid target


		}
		else
		if(techId == TechId::MindControl) {

			if (target->playerId == castingPlayerId) {
				stringId = 1327; // Must target enemy units<0>
			}
			else
			if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
				stringId = 877; // Unable to target structure.<0>
			}
			else
			if (target->id == UnitId::TerranSpiderMine
			||	target->id == UnitId::TerranLobotomyMine
			||	target->id == UnitId::ZergLarva
			||	target->id == UnitId::ZergEgg
			||	target->id == UnitId::ZergCocoon
			||	target->id == UnitId::ProtossSteward
			||	target->id == UnitId::ProtossPlasmaShell
					) {
				stringId = 876; //Invalid target.<0>
			}
			else {
				stringId = 0; //valid target
			}
		}
		else
		if(techId == TechId::Hallucination) {
			if(target->id == UnitId::ProtossSteward)
				stringId = 876; // Invalid target.<0>
			else
			if(units_dat::BaseProperty[target->id] & UnitProperty::Building)
				stringId = 877; // Unable to target structure.<0>
			else
			if(scbw::isAlliedTo(target->playerId, castingPlayerId) || target->playerId == castingPlayerId)
				stringId = 1327; // Must target enemy units<0>
			else
				stringId = 0; //valid target
		}
		else
		if(	techId == TechId::DefensiveMatrix ||
			techId == TechId::Irradiate ||
			techId == TechId::Restoration ||
			techId == TechId::OpticalFlare
		)
		{
			if(units_dat::BaseProperty[target->id] & UnitProperty::Building)
				stringId = 877; //Unable to target structure.<0>
			else
				stringId = 0; //valid target
		}
		else
		if(techId == TechId::Infestation) {
			if(!isInfestableUnit(target))
				stringId = 870;	//Must target severely damaged Terran Command Center.<0>
			else
				stringId = 0; //valid target
		}
		else
		if(techId == TechId::Consume) {
			if(
				units_dat::BaseProperty[target->id] & UnitProperty::Building
			||	target->playerId != castingPlayerId
			||	target->getRace() != RaceId::Zerg
			||	scbw::isZergEggNew(target)
			)
				stringId = 876; //Invalid target.<0>
			else
				stringId = 0; //valid target

		}
		else if (techId == TechId::PowerTransfer) {
			stringId = 0;
		}
		else
			stringId = 876; //Invalid target.<0>

	}
	if (techId == TechId::UnusedTech39) {
		stringId = 0;
	}
	if (techId == TechId::Lockdown) {
		
	}
	return stringId;

}

} //hooks

//-------- Helper function definitions. Do NOT modify! --------//
namespace {

const u32 Func_IsInfestable = 0x00402210;
bool isInfestableUnit(CUnit* unit) {

	static Bool32 result;

	__asm {
		PUSHAD
		MOV EDX, unit
		CALL Func_IsInfestable
		MOV result, EAX
		POPAD
	}

	return result != 0;

}

}
