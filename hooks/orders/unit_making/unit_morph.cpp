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


u32 Func_Sub46A820 = 0x0046A820;
u8 setArchonPathing(CUnit* unit) {
	static u8 result;
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub46A820
		MOV result, AL
		POPAD
	}
	return result;
	//
}
;

namespace {

	void fixTargetLocation(Point16* coords, u32 unitId);							//01FA0
	bool hasSuppliesForUnit(u32 unitId, u32 playerId, Bool32 canShowErrorMessage);	//2CF70
	CUnit* function_0045D910(CUnit* unit);											//5D910
	void orderNewUnitToRally(CUnit* unit, CUnit* factory);							//66F50
	bool advanceRemainingBuildTime_Sub466940(CUnit* unit);							//66940
	void refundAllQueueSlots(CUnit* unit);											//66E80
	void actUnitReturnToIdle(CUnit* unit);											//75420
	void incrementUnitScoresEx(CUnit* unit, s32 unk1, s32 unk2);					//88D50
	void playMorphingCompleteSound(CUnit* unit);									//8F440
	void updateUnitStrength(CUnit* unit);											//9FA40
	void replaceUnitWithType(CUnit* unit, u16 newUnitId);							//9FED0
	void function_004A01F0(CUnit* unit);											//A01F0
	void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit);					//A2830
	void changeUnitButtonSet_Sub4E5D60(CUnit* unit, u16 unitId);					//E5D60
	void function_004E65E0(CUnit* unit, Bool32 flag);								//E65E0

} //unnamed namespace
namespace hooks {

	void orders_ZergBirth(CUnit* unit) {

		if (unit->orderSignal & 4) {

			bool bTwinEggOrCocoon = true;
			CUnit* unit2;
			Point16 pos;

			unit->orderSignal -= 4;
			bool vorvaling = false;
			if (unit->previousUnitType == UnitId::ZergVorvaling || unit->previousUnitType==UnitId::ZergNathrokor || 
				unit->previousUnitType == UnitId::EggVorvaling || unit->previousUnitType==UnitId::EggNathrokor) {
				vorvaling = true;
			}

			if (
				units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg ||
				unit->previousUnitType == UnitId::ZergCocoon || unit->previousUnitType == 179
				)
				bTwinEggOrCocoon = false;

			unit2 = NULL;

			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg && !vorvaling) {

				unit2 = function_0045D910(unit);//zerg dual birth

				if (unit2 != NULL)
					AI_TrainingUnit(unit, unit2);

			}

			if (
				unit->id != UnitId::ZergDrone &&
				unit->id != UnitId::ZergOverlord
				)
				AI_TrainingUnit(unit, unit);

			function_004E65E0(unit, 0);

			//update various stuff (set hp, set shield...) not finished on Morph
			function_004A01F0(unit);
			updateUnitStrength(unit);

			pos.x = unit->sprite->position.x;
			pos.y = unit->sprite->position.y;

			if (bTwinEggOrCocoon) {

				if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
					pos.y -= 7;
				else
					if (unit->status & UnitStatus::InAir)
						pos.y -= 42;

			}

			fixTargetLocation(&pos, unit->id);

			if (
				pos.x != unit->sprite->position.x ||
				pos.y != unit->sprite->position.y
				)
				scbw::setUnitPosition(unit, pos.x, pos.y);

			if (
				!scbw::isZergEgg(unit->previousUnitType,false,false)
				)
			{

				orderNewUnitToRally(unit, unit->connectedUnit);

				if (unit2 != NULL)
					orderNewUnitToRally(unit2, unit->connectedUnit);

			}

			actUnitReturnToIdle(unit);

		}

	}

	;

	void orders_Morph1(CUnit* unit) {
		//scbw::printText("Morph...");
		if (unit->mainOrderState == 0) {

			if (scbw::isConventionalUnitMorphSource(unit->id,true)
				
				)
			{
				//scbw::printText("Is Conventional");
				if (!hasSuppliesForUnit(
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
					//scbw::printText("Setup egg");
					bool bStopThere = false;
					u16 eggId;
					switch (unit->id) {
					case UnitId::ZergOverlord:
					case UnitId::ZergIroleth:
						break;
					default:
						incrementUnitScoresEx(unit, -1, 0);
						break;
					}
//					incrementUnitScoresEx(unit, -1, 0);
					//was unit->status = unit->status & ~UnitStatus::Completed
					//in original code
					if (unit->status & UnitStatus::Completed)
						unit->status -= UnitStatus::Completed;

					switch (unit->id) {
					case UnitId::ZergLarva:
						eggId = UnitId::ZergEgg;
						break;
					case UnitId::ZergHydralisk:
						if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ZergLurker) {
							eggId = UnitId::ZergLurkerEgg;
						}
						else {
							eggId = UnitId::EggBasilisk;
						}
						break;
					case UnitId::ZergZergling:
						if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ZergNathrokor) {
							eggId = UnitId::EggNathrokor;
						}
						else {
							eggId = UnitId::EggVorvaling;
						}						
						break;
					case UnitId::ZergNathrokor:
						eggId = UnitId::EggKalkaleth;
						break;
					case UnitId::ZergMutalisk:
						eggId = UnitId::ZergCocoon;
						break;
					case UnitId::ZergUltralisk:
						if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ZergAlkajelisk) {
							eggId = UnitId::EggAlkajelisk;
						}
						else {
							eggId = UnitId::EggKeskathalor;
						}
						break;
					case UnitId::ZergOverlord:
						eggId = UnitId::EggIroleth;
						break;
					case UnitId::ZergIroleth:
						eggId = UnitId::EggOthstoleth;
						break;
					default:
						if (unit->id != UnitId::ProtossHighTemplar) {
							bStopThere = true;
						}
						
						break;
					}
					//scbw::printFormattedText("Stop status: %d", (int)bStopThere);
					if (!bStopThere) {
						if (unit->id == UnitId::ProtossHighTemplar) {
							//scbw::printText("Augur morph");
							replaceUnitWithType(unit, UnitId::ProtossAugur);
							unit->sprite->flags &= ~CSprite_Flags::IscriptCode;
							if(unit->sprite->images.head != NULL) {
								CImage* current_image = unit->sprite->images.head;
								do {
									current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
									current_image = current_image->link.next;
								}while (current_image != NULL);
							}
							changeUnitButtonSet_Sub4E5D60(unit,UnitId::None);
							scbw::refreshConsole();
							unit->status &= ~UnitStatus::IsBuilding; //disable the flag if set
							setArchonPathing(unit);
							unit->orderComputerCL(OrderId::CompletingArchonSummon);
							return;
						}
						else {

							replaceUnitWithType(unit, eggId);
						}
						
						changeUnitButtonSet_Sub4E5D60(unit, unit->id);
						unit->remainingBuildTime = units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]];
						unit->mainOrderState = 1;
						if (unit->id == UnitId::EggVorvaling) {
							unit->sprite->playIscriptAnim(IscriptAnimation::Unused1, true);
						}

					}

				}

			}

		}
		else
			if (unit->mainOrderState == 1) {
				//advance
				if (advanceRemainingBuildTime_Sub466940(unit)) {
					unit->sprite->playIscriptAnim(IscriptAnimation::SpecialState1, true);
					unit->mainOrderState = 2;
				}
			}
			else
				if (unit->mainOrderState == 2) {

					if (unit->orderSignal & 4) {

						unit->orderSignal -= 4;
						switch (unit->id) {
						case UnitId::EggIroleth:
							scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 1);
							break;
						case UnitId::EggOthstoleth:
							scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 2);
							break;
						default:
							break;
						}
						replaceUnitWithType(unit, unit->buildQueue[unit->buildQueueSlot]);
						if (
							unit->id == UnitId::ZergDrone ||
							unit->id == UnitId::ZergOverlord
							)
							AI_TrainingUnit(unit, unit);

						playMorphingCompleteSound(unit);
						unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
						if (units_dat::ConstructionGraphic[unit->id] != 0) {

							CImage* current_image;

							function_004E65E0(unit, 1);

							current_image = unit->sprite->images.head;

							while (current_image != NULL) {
								current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
								current_image = current_image->link.next;
							}

							unit->orderComputerCL(OrderId::ZergBirth);

						}
						else { //5DF60

							//update various stuff (set hp, set shield...) not finished on Morph

							function_004A01F0(unit);

							updateUnitStrength(unit);

							if (unit->id == UnitId::ZergEgg)
								orderNewUnitToRally(unit, unit->connectedUnit);

						}

					}

				}

	}

	;

	//Check if @p unit can morph into @p morphUnitId.
	bool unitCanMorphHook(CUnit* unit, u16 morphUnitId) {
		//Default StarCraft behavior
		if (unit->id == UnitId::ZergHydralisk) {
			if (morphUnitId == UnitId::ZergBasilisk) {
				return true;
			}
			if (morphUnitId == UnitId::ZergLurker) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergZergling) {
			if (morphUnitId == UnitId::ZergVorvaling || morphUnitId == UnitId::ZergNathrokor) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergNathrokor) {
			if (morphUnitId == UnitId::ZergKalkaleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergUltralisk) {
			if (morphUnitId == UnitId::ZergKeskathalor || morphUnitId == UnitId::ZergAlkajelisk) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergOverlord) {
			if (morphUnitId == UnitId::ZergIroleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergIroleth) {
			if (morphUnitId == UnitId::ZergOthstoleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ProtossHighTemplar) {
			if (morphUnitId == UnitId::ProtossAugur) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergLarva || unit->id == UnitId::ZergMutalisk) {
			if (unit->canMakeUnit(morphUnitId, *ACTIVE_NATION_ID) == 1) {
				return true;
			}
		}
		return false;
	}

	//Check if @p unitId is an egg unit that can be rallied
	bool isRallyableEggUnitHook(u16 unitId) {
		//Default StarCraft behavior
		if (scbw::isZergEgg(unitId,false,false))
			return false;

		return true;
	}

	//Return the ID of the egg unit to use when morphing @p unitId.
	//If the unit cannot morph, return UnitId::None.
	u16 getUnitMorphEggTypeHook(u16 unitId, CUnit* unit, u16 resultId) {
		//Default StarCraft behavior
		if (unitId == UnitId::ZergLarva)
			return UnitId::ZergEgg;

		if (unitId == UnitId::ZergMutalisk) {
			if (resultId == UnitId::ZergDevourer)
				return UnitId::EggDevourer;
			else if (resultId == UnitId::ZergGorgoleth)
				return UnitId::EggGorgoleth;
			else
				return UnitId::ZergCocoon;
		}

		if (unitId == UnitId::ZergHydralisk) {
			if (resultId == UnitId::ZergLurker) {
				return UnitId::ZergLurkerEgg;
			}
			return UnitId::EggBasilisk;

		}

		if (unitId == UnitId::ZergZergling)
		{
			if (resultId == UnitId::ZergNathrokor) {
				return UnitId::EggNathrokor;
			}
			return UnitId::EggVorvaling;
		}

		if (unitId == UnitId::ZergNathrokor) {
			return UnitId::EggKalkaleth;
		}

		if (unitId == UnitId::ZergUltralisk)
		{
			if (resultId == UnitId::ZergAlkajelisk) {
				return UnitId::EggAlkajelisk;
			}
			else {
				return UnitId::EggKeskathalor;
			}
		}

		if (unitId == UnitId::ZergOverlord) {
			return UnitId::EggIroleth;
		}

		if (unitId == UnitId::ZergIroleth) {
			return UnitId::ZergOthstoleth;
		}


		return UnitId::None;
	}

	//Determine the type (unit ID) of the unit to revert to when cancelling an
	//@p eggUnit while it is morphing.
	u16 getCancelMorphRevertTypeHook(CUnit* eggUnit) {
		//Default StarCraft behavior

		if (eggUnit->id == UnitId::ZergCocoon
		||	eggUnit->id == UnitId::EggGorgoleth
		||	eggUnit->id == UnitId::EggDevourer) {
			return UnitId::ZergMutalisk;
		}
		if (eggUnit->id == UnitId::ZergLurkerEgg) {
			return UnitId::ZergHydralisk;
		}
		if (eggUnit->id == UnitId::EggBasilisk) {
			return UnitId::ZergHydralisk;
		}
		if (eggUnit->id == UnitId::EggVorvaling) {
			return UnitId::ZergZergling;
		}
		if (eggUnit->id == UnitId::EggNathrokor) {
			return UnitId::ZergZergling;
		}
		if (eggUnit->id == UnitId::EggKalkaleth) {
			return UnitId::ZergNathrokor;
		}
		if (eggUnit->id == UnitId::EggIroleth) {
			return UnitId::ZergOverlord;
		}
		if (eggUnit->id == UnitId::EggOthstoleth) {
			return UnitId::ZergIroleth;
		}
		if (eggUnit->id == UnitId::EggKeskathalor) {
			return UnitId::ZergUltralisk;
		}
		if (eggUnit->id == UnitId::EggAlkajelisk) {
			return UnitId::ZergUltralisk;
		}

		//  || unitId==223 || unitId==102 || unitIds==179
		return UnitId::None;  //Default (no revert for larvae)
	}

	//Determines the vertical (Y) offset by which the @p unit will be shifted to
	//when it finishes morphing.
	s16 getUnitVerticalOffsetOnBirth(CUnit* unit) {
		//Default StarCraft behavior

		//No offset, birth offset is handled elsewhere
		if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg)
			return 0;

		//No offset, since the morphed unit should stay where it is
		if (unit->previousUnitType == UnitId::ZergCocoon)
			return 0;

		//Hovering units (?) float 7 pixels above ground
		if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
			return -7;

		//Air units float 42 pixels above ground
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

		if (unitId == UnitId::ZergLurker)
			supplyCost -= units_dat::SupplyRequired[UnitId::ZergHydralisk];

		aiSupplyReserved[playerId] = supplyCost;

		//No supply cost check needed
		if (supplyCost == 0 || units_dat::BaseProperty[unitId] & UnitProperty::MorphFromOtherUnit)
			return true;

		const RaceId::Enum raceId = CUnit::getRace(unitId);
		assert(raceId <= 2);
		const u32 supplyUsed = raceSupply[raceId].used[playerId];

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

	const u32 Func_fixTargetLocation = 0x00401FA0;
	void fixTargetLocation(Point16* coords, u32 unitId) {

		__asm {
			PUSHAD
			MOV EAX, unitId
			MOV EDX, coords
			CALL Func_fixTargetLocation
			POPAD
		}

	}

	;

	const u32 Func_hasSuppliesForUnit = 0x0042CF70;
	bool hasSuppliesForUnit(u32 unitId, u32 playerId, Bool32 canShowErrorMessage) {

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

	const u32 Func_Sub45D910 = 0x0045D910;
	CUnit* function_0045D910(CUnit* unit) {

		static CUnit* rValue;

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

	u32 Func_Sub466940 = 0x00466940;
	//return true if remainingBuildTime was 0 when called,
	//remainingBuildTime change affected by cheating
	bool advanceRemainingBuildTime_Sub466940(CUnit* unit) {

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

	const u32 Func_RefundAllQueueSlots = 0x00466E80;
	void refundAllQueueSlots(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}

	}

	;

	const u32 Func_Sub466F50 = 0x00466F50;
	void orderNewUnitToRally(CUnit* unit, CUnit* factory) {

		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, factory
			CALL Func_Sub466F50
			POPAD
		}

	}

	;

	const u32 Func_ActUnitReturnToIdle = 0x00475420;
	void actUnitReturnToIdle(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ActUnitReturnToIdle
			POPAD
		}

	}

	;

	const u32 Func_incrementUnitScoresEx = 0x00488D50;
	//unk1 is ECX, unk2 is pushed value
	void incrementUnitScoresEx(CUnit* unit, s32 count, s32 unk2) {

		__asm {
			PUSHAD
			MOV EDI, unit
			MOV ECX, count
			PUSH unk2
			CALL Func_incrementUnitScoresEx
			POPAD
		}

	}

	;

	const u32 Func_PlayMorphingCompleteSound = 0x0048F440;
	void playMorphingCompleteSound(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayMorphingCompleteSound
			POPAD
		}

	}

	;

	const u32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}

	}

	;

	const u32 Func_ReplaceUnitWithType = 0x0049FED0;
	void replaceUnitWithType(CUnit* unit, u16 newUnitId) {
		u32 newUnitId_ = newUnitId;

		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ReplaceUnitWithType
			POPAD
		}

	}

	;

	const u32 Func_Sub4A01F0 = 0x004A01F0;
	void function_004A01F0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const u32 Func_AI_TrainingUnit = 0x004A2830;
	void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit) {
		__asm {
			PUSHAD
			MOV EAX, created_unit
			MOV ECX, unit_creator
			CALL Func_AI_TrainingUnit
			POPAD
		}
	}

	;

	const u32 Func_Sub4E5D60 = 0x004E5D60;
	void changeUnitButtonSet_Sub4E5D60(CUnit* unit, u16 buttonSetId) {

		__asm {
			PUSHAD
			MOV EAX, unit
			MOV CX, buttonSetId
			CALL Func_Sub4E5D60
			POPAD
		}

	}

	;

	const u32 Func_Sub4E65E0 = 0x004E65E0;
	void function_004E65E0(CUnit* unit, Bool32 flag) {

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
