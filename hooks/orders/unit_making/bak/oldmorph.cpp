#include <SCBW/api.h>
//copied from unhooked
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations/TechId.h>
#include <SCBW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
//
//helper functions def

namespace {

void fixTargetLocation(Point16* coords, uint32 unitId);							//01FA0
bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, Bool32 canShowErrorMessage);	//2CF70
Unit* function_0045D910(Unit* unit);											//5D910
void orderNewUnitToRallyHook(Unit* unit, Unit* factory);							//66F50
bool advanceRemainingBuildTime_Sub466940(Unit* unit);							//66940
void refundAllQueueSlotsHook(Unit* unit);											//66E80
void actUnitReturnToIdle(Unit* unit);											//75420
void incrementUnitScoresEx(Unit* unit, s32 unk1, s32 unk2);					//88D50
void playMorphingCompleteSound(Unit* unit);									//8F440
void updateUnitStrength(Unit* unit);											//9FA40
void replaceUnitWithType(Unit* unit, u16 newUnitId);							//9FED0
void function_004A01F0(Unit* unit);											//A01F0
void AI_TrainingUnit(Unit* unit_creator, Unit* created_unit);					//A2830
void changeUnitButtonSet_Sub4E5D60(Unit* unit, u16 unitId);					//E5D60
void function_004E65E0(Unit* unit, Bool32 flag);								//E65E0

} //unnamed namespace

namespace hooks {

void orders_ZergBirth(Unit* unit) {

	if(unit->orderSignal & 4) {
		bool bTwinEggOrCocoon = true;
		Unit* unit2;
		Point16 pos;

		unit->orderSignal -= 4;

		if(
			units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg ||
			unit->previousUnitType == UnitId::ZergCocoon || unit->previousUnitType==179
		)
			bTwinEggOrCocoon = false;

		unit2 = nullptr;

		if(units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg) {

			unit2 = function_0045D910(unit);

			if(unit2 != nullptr)
				AI_TrainingUnit(unit,unit2);

		}

		if(
			unit->id != UnitId::ZergDrone &&
			unit->id != UnitId::ZergOverlord
		)
			AI_TrainingUnit(unit,unit);

		function_004E65E0(unit,0);

		//update various stuff (set hp, set shield...) not finished on Morph
		function_004A01F0(unit);
		updateUnitStrength(unit);

		pos.x = unit->sprite->position.x;
		pos.y = unit->sprite->position.y;

		if(bTwinEggOrCocoon) {

			if(units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
				pos.y -= 7;
			else
			if(unit->status & UnitStatus::InAir)
				pos.y -= 42;

		}

		fixTargetLocation(&pos,unit->id);

		if(
			pos.x != unit->sprite->position.x ||
			pos.y != unit->sprite->position.y
		)
			scbw::setUnitPosition(unit,pos.x,pos.y);

		if(
			unit->previousUnitType != UnitId::ZergCocoon &&
			unit->previousUnitType != UnitId::ZergLurkerEgg &&
			unit->previousUnitType != 179 &&
			unit->previousUnitType != 216 &&
			unit->previousUnitType != 102 &&
			unit->previousUnitType != 223
		
		)
		{

			orderNewUnitToRally(unit,unit->connectedUnit);

			if(unit2 != nullptr)
				orderNewUnitToRally(unit2,unit->connectedUnit);

		}

		actUnitReturnToIdle(unit);

	}

}

;

void orders_Morph1(Unit* unit) {

	if(unit->mainOrderState == 0) {

		if(
			unit->id == UnitId::ZergLarva ||
			unit->id == UnitId::ZergHydralisk ||
			unit->id == UnitId::ZergMutalisk ||
			unit->id == UnitId::zergling
		) 
		{

			if(!hasSuppliesForUnit(
						unit->buildQueue[unit->buildQueueSlot],
						unit->playerId,
						1
						)
			)
			{
				refundAllQueueSlots(unit);
				unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}
			else {

				bool bStopThere = false;
				u16 eggId;

				incrementUnitScoresEx(unit,-1,0);

				//was unit->status = unit->status & ~UnitStatus::Completed
				//in original code
				if(unit->status & UnitStatus::Completed)
					unit->status -= UnitStatus::Completed;
				
				switch (unit->id) {
				case UnitId::larva:
					eggId = UnitId::ZergEgg;
					break;
				case UnitId::hydralisk:
					eggId = UnitId::lurker_egg;
					break;
				case UnitId::zergling:
					eggId = UnitId::Powerup_YoungChrysalis;
					break;
				case UnitId::mutalisk:
					eggId = UnitId::cocoon;


					break;
				case UnitId::ultralisk:
					eggId = 102;
					break;
				case UnitId::overlord:
					eggId = 223;
					break;
				default:
					bStopThere = true;
					break;
				}

				if(!bStopThere) {
					replaceUnitWithType(unit,eggId);
					changeUnitButtonSet_Sub4E5D60(unit,unit->id);

					unit->remainingBuildTime = units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]];

					unit->mainOrderState = 1;
					if (unit->id == UnitId::Powerup_YoungChrysalis) {
						unit->sprite->playIscriptAnim(IscriptAnimation::Unused1, true);
					}
				}

			}

		}

	}
	else
	if(unit->mainOrderState == 1) {
		if(advanceRemainingBuildTime_Sub466940(unit)) {
			unit->sprite->playIscriptAnim(IscriptAnimation::SpecialState1,true);
			unit->mainOrderState = 2;
		}
	}
	else
	if(unit->mainOrderState == 2) {

		if(unit->orderSignal & 4) {

			unit->orderSignal -= 4;
			replaceUnitWithType(unit,unit->buildQueue[unit->buildQueueSlot]);

			if(
				unit->id == UnitId::ZergDrone ||
				unit->id == UnitId::ZergOverlord
			)
				AI_TrainingUnit(unit,unit);

			playMorphingCompleteSound(unit);

			unit->buildQueue[unit->buildQueueSlot] = UnitId::None;

			if(units_dat::ConstructionGraphic[unit->id] != 0) {

				Image* current_image;

				function_004E65E0(unit,1);

				current_image = unit->sprite->images.head;

				while(current_image != nullptr) {
					current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
					current_image = current_image->link.next;
				}

				unit->orderComputerCL(OrderId::ZergBirth);

			}
			else { //5DF60

				//update various stuff (set hp, set shield...) not finished on Morph
				function_004A01F0(unit);
				updateUnitStrength(unit);

				if(unit->id == UnitId::ZergEgg)
					orderNewUnitToRally(unit,unit->connectedUnit);

			}

		}

	}

}

;

//Check if @p unit can morph into @p morphUnitId.
bool unitCanMorphHook(Unit* unit, u16 morphUnitId) {
	//Default StarCraft behavior

	if (unit->id == UnitId::hydralisk) {
		/*
		if (morphUnitId == UnitId::lurker
		&& unit->canUseTech(TechId::LurkerAspect, *ACTIVE_NATION_ID) == 1) {
		return true;
		}*/
		if (morphUnitId == UnitId::Powerup_KhalisCrystal) {
			return true;
		}
	}


	if (unit->id == UnitId::zergling) {
		if (morphUnitId == UnitId::Powerup_UrajCrystal) {
			return true;
		}
		if (morphUnitId == UnitId::devouring_one) {
			return true;
		}
	}
	if (unit->id == UnitId::larva || unit->id == UnitId::mutalisk) {
		if (unit->canMakeUnit(morphUnitId, *ACTIVE_NATION_ID) == 1) {
			return true;
		}
	}
	return false;
}

//Check if @p unitId is an egg unit.
bool isEggUnitHook(u16 unitId) {
	//Default StarCraft behavior

	if (unitId == UnitId::egg
		|| unitId == UnitId::cocoon
		|| unitId == UnitId::lurker_egg ||
		unitId == UnitId::Powerup_YoungChrysalis  || unitId==223 || unitId==102 || unitId==179)
		return true;

	return false;
}

//Check if @p unitId is an egg unit that can be rallied
bool isRallyableEggUnitHook(u16 unitId) {
	//Default StarCraft behavior

	if (unitId == UnitId::cocoon || unitId == UnitId::lurker_egg || unitId == UnitId::Powerup_YoungChrysalis  || unitId==223 || unitId==102 || unitId==179)
		return false;

	return true;
}

//Return the ID of the egg unit to use when morphing @p unitId.
//If the unit cannot morph, return UnitId::None.
u16 getUnitMorphEggTypeHook(u16 unitId, Unit* unit, u16 resultId) {
	//Default StarCraft behavior
	if (unitId == UnitId::larva)
		return UnitId::egg;

	if (unitId == UnitId::mutalisk){
		  if (resultId == UnitId::devourer)
			  return 179;
		  else
			  return UnitId::cocoon;
	}

	if (unitId == UnitId::hydralisk)
		return UnitId::lurker_egg;

	if (unitId == UnitId::zergling)
		return UnitId::Powerup_YoungChrysalis;
	if(unitId == UnitId::ultralisk)
		return 102;
	if(unitId == UnitId::overlord)
		return 223;
	

	return UnitId::None;
}

//Determine the type (unit ID) of the unit to revert to when cancelling an
//@p eggUnit while it is morphing.
u16 getCancelMorphRevertTypeHook(Unit* eggUnit) {
	//Default StarCraft behavior

	if (eggUnit->id == UnitId::cocoon || eggUnit->id==179)
		return UnitId::mutalisk;

	if (eggUnit->id == UnitId::lurker_egg)
	{
		return UnitId::hydralisk;
	}

	if (eggUnit->id == UnitId::Powerup_YoungChrysalis)
	{
		return UnitId::devouring_one;
		//return UnitId::zergling;
		//replace for different unit
	}
	if (eggUnit->id == 222)
	{
		return UnitId::overlord;
	}
	if (eggUnit->id == 102)
	{
		return UnitId::ultralisk;
	}

	
//  || unitId==223 || unitId==102 || unitIds==179
	return UnitId::None;  //Default (no revert for larvae)
}

//Determines the vertical (Y) offset by which the @p unit will be shifted to
//when it finishes morphing.
s16 getUnitVerticalOffsetOnBirth(Unit* unit) {
	//Default StarCraft behavior

	//No offset, birth offset is handled elsewhere
	if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg)
		return 0;

	//No offset, since the morphed unit should stay where it is
	if (unit->previousUnitType == UnitId::cocoon)
		return 0;

	//Hovering units (?) f32 7 pixels above ground
	if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
		return -7;

	//Air units f32 42 pixels above ground
	if (unit->status & UnitStatus::InAir)
		return -42;

	//Default for ground units
	return 0;
}

//Check if @p playerId has enough supplies to build @p unitId.
bool hasSuppliesForUnitHook(u8 playerId, u16 unitId, bool canShowErrorMessage) {
	//Default StarCraft behavior
	s32 supplyCost = units_dat::SupplyRequired[unitId];

	if (units_dat::BaseProperty[unitId] & UnitProperty::TwoUnitsIn1Egg)
		supplyCost *= 2;

	if (unitId == UnitId::lurker)
		supplyCost -= units_dat::SupplyRequired[UnitId::hydralisk];

	aiSupplyReserved[playerId] = supplyCost;

	//No supply cost check needed
	if (supplyCost == 0 || units_dat::BaseProperty[unitId] & UnitProperty::MorphFromOtherUnit)
		return true;

	const RaceId::Enum raceId = Unit::getRace(unitId);
	assert(raceId <= 2);
	const uint32 supplyUsed = raceSupply[raceId].used[playerId];

	//Must construct additional pylons
	if (supplyUsed + supplyCost > raceSupply[raceId].max[playerId]) {
		if (canShowErrorMessage)
			scbw::showErrorMessageWithSfx(playerId, 847 + raceId, 1 + raceId);
		return false;
	}

	//Supply limit exceeded
	if (supplyCost > scbw::getSupplyRemaining(playerId, raceId)) {
		if (canShowErrorMessage)
			scbw::showErrorMessageWithSfx(playerId, 844 + raceId, 153 + raceId);
		return false;
	}

	return true;
}


} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const uint32 Func_fixTargetLocation = 0x00401FA0;
void fixTargetLocation(Point16* coords, uint32 unitId) {

	__asm {
		PUSHAD
		MOV EAX, unitId
		MOV EDX, coords
		CALL Func_fixTargetLocation
		POPAD
	}

}

;

const uint32 Func_hasSuppliesForUnit = 0x0042CF70;
bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, Bool32 canShowErrorMessage) {

	static Bool32 bPreResult;

	__asm {
		PUSHAD
		PUSH canShowErrorMessage
		PUSH unitId
		PUSH playerId
		CALL Func_hasSuppliesForUnit
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

const uint32 Func_Sub45D910 = 0x0045D910;
Unit* function_0045D910(Unit* unit) {

	static Unit* rValue;

	__asm {
		PUSHAD
		PUSH unit
		CALL Func_Sub45D910
		MOV rValue, EAX
		POPAD
	}
	
	return rValue;

}

;

uint32 Func_Sub466940 = 0x00466940;
//return true if remainingBuildTime was 0 when called,
//remainingBuildTime change affected by cheating
bool advanceRemainingBuildTime_Sub466940(Unit* unit) {

	static Bool32 bTimeWasZero;

	__asm {
		PUSHAD
		MOV ECX, unit
		CALL Func_Sub466940
		MOV bTimeWasZero, EAX
		POPAD
	}

	return (bTimeWasZero != 0);

}

;

const uint32 Func_RefundAllQueueSlots = 0x00466E80;
void refundAllQueueSlotsHook(Unit* unit) {

  __asm {
    PUSHAD
	MOV EAX, unit
	CALL Func_RefundAllQueueSlots
    POPAD
  }

}

;

const uint32 Func_Sub466F50 = 0x00466F50;
void orderNewUnitToRallyHook(Unit* unit, Unit* factory) {

	__asm {
		PUSHAD
		MOV EAX, unit
		MOV ECX, factory
		CALL Func_Sub466F50
		POPAD
	}

}

;

const uint32 Func_ActUnitReturnToIdle = 0x00475420;
void actUnitReturnToIdle(Unit* unit) {

  __asm {
    PUSHAD
    MOV EAX, unit
    CALL Func_ActUnitReturnToIdle
    POPAD
  }

}

;

const uint32 Func_incrementUnitScoresEx = 0x00488D50;
//unk1 is ECX, unk2 is pushed value
void incrementUnitScoresEx(Unit* unit, s32 unk1, s32 unk2) {

	__asm {
		PUSHAD
		MOV EDI, unit
		MOV unk1, ECX
		PUSH unk2
		CALL Func_incrementUnitScoresEx
		POPAD
	}

}

;

const uint32 Func_PlayMorphingCompleteSound = 0x0048F440;
void playMorphingCompleteSound(Unit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_PlayMorphingCompleteSound
		POPAD
	}

}

;

const uint32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrength(Unit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}

;

const uint32 Func_ReplaceUnitWithType = 0x0049FED0;
void replaceUnitWithType(Unit* unit, u16 newUnitId) {
	uint32 newUnitId_ = newUnitId;

  __asm {
    PUSHAD
    PUSH newUnitId_
    MOV EAX, unit
    CALL Func_ReplaceUnitWithType
    POPAD
  }

}

;

const uint32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0(Unit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}

;

const uint32 Func_AI_TrainingUnit = 0x004A2830;
void AI_TrainingUnit(Unit* unit_creator, Unit* created_unit) {
	__asm {
		PUSHAD
		MOV EAX, created_unit
		MOV ECX, unit_creator
		CALL Func_AI_TrainingUnit
		POPAD
	}
}

;

const uint32 Func_Sub4E5D60 = 0x004E5D60;
void changeUnitButtonSet_Sub4E5D60(Unit* unit, u16 buttonSetId) {

	__asm {
		PUSHAD
		MOV EAX, unit
		MOV CX, buttonSetId
		CALL Func_Sub4E5D60
		POPAD
	}

}

;

const uint32 Func_Sub4E65E0 = 0x004E65E0;
void function_004E65E0(Unit* unit, Bool32 flag) {

	__asm {
		PUSHAD
		MOV EDI, unit
		PUSH flag
		CALL Func_Sub4E65E0
		POPAD
	}

}

;

} //Unnamed namespace

//End of helper functions
