#include <SCBW/api.h>
//copied from unhooked
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations/TechId.h>
#include <SCBW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "globals.h"
//helper functions def


uint32 Func_Sub46A820 = 0x0046A820;
uint8 setArchonPathing(CUnit* unit) {
	static uint8 result;
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

	void fixTargetLocation(Point16* coords, uint32 unitId);							//01FA0
	bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, bool32 canShowErrorMessage);	//2CF70
	CUnit* function_0045D910(CUnit* unit);											//5D910
	void orderNewUnitToRally(CUnit* unit, CUnit* factory);							//66F50
	bool advanceRemainingBuildTime_Sub466940(CUnit* unit);							//66940
	void refundAllQueueSlots(CUnit* unit);											//66E80
	void actUnitReturnToIdle(CUnit* unit);											//75420
	void incrementUnitScoresEx(CUnit* unit, int32 unk1, int32 unk2);					//88D50
	void playMorphingCompleteSound(CUnit* unit);									//8F440
	void updateUnitStrength(CUnit* unit);											//9FA40
	void replaceUnitWithType(CUnit* unit, uint16 newUnitId);							//9FED0
	void function_004A01F0(CUnit* unit);											//A01F0
	void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit);					//A2830
	void changeUnitButtonSet_Sub4E5D60(CUnit* unit, uint16 unitId);					//E5D60
	void function_004E65E0(CUnit* unit, bool32 flag);								//E65E0

} //unnamed namespace
namespace hooks {

	void orders_ZergBirth(CUnit* unit) {

		if (unit->orderSignal & 4) {

			bool bTwinEggOrCocoon = true;
			CUnit* unit2;
			Point16 pos;

			unit->orderSignal -= 4;
			bool vorvaling = false;
			auto origin = unit->eggOrigin;
			if (origin == UnitId::ZergZergling) {
				vorvaling = true;
			}

			if (
				units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg ||
				unit->previousUnitType == UnitId::ZergCocoon || unit->previousUnitType == 179
				)
				bTwinEggOrCocoon = false;

			unit2 = nullptr;

			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg && !vorvaling) {
				unit2 = function_0045D910(unit);//zerg dual birth

				if (unit2 != nullptr)
					AI_TrainingUnit(unit, unit2);

			}

			if (
				unit->id != UnitId::ZergDroleth &&
				unit->id != UnitId::ZergOvileth
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
				!scbw::isZergEggNew(unit,true,false,unit->previousUnitType)
				)
			{
				auto origin = unit->eggOrigin;
				if (origin == UnitId::ZergLarva) {
					orderNewUnitToRally(unit, unit->connectedUnit);

					if (unit2 != nullptr)
						orderNewUnitToRally(unit2, unit->connectedUnit);
				}

			}

			actUnitReturnToIdle(unit);

		}

	}

	;

	void orders_Morph1(CUnit* unit) {
		if (unit->mainOrderState == 0) {

			if (scbw::isConventionalUnitMorphSource(unit->id,true,true))
			{
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
					bool bStopThere = false;
					uint16 eggId;
					switch (unit->id) {
					case UnitId::ZergOvileth:
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
					case UnitId::ZergZergling:
					case UnitId::ZergVorvaling:
					case UnitId::ZergHydralisk:
					case UnitId::ZergBactalisk:
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergUltrakor:
					case UnitId::ZergQuazilisk:
						eggId = UnitId::ZergEgg;
						break;
					case UnitId::ZergNathrokor:
					case UnitId::ZergMutalisk:
					case UnitId::ZergGorgrokor:
					case UnitId::ZergOvileth:
					case UnitId::ZergIroleth:
						eggId = UnitId::ZergCocoon;
						break;
					case UnitId::ZergMatraleth:
						if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ZergKonvilisk) {
							eggId = UnitId::ZergEgg;
						}
						else {
							eggId = UnitId::ZergCocoon;
						}
					default:
						if (unit->id != UnitId::ProtossCantavis) {
							bStopThere = true;
						}
						break;
					}
					if (!bStopThere) {
						if (unit->id == UnitId::ProtossCantavis) {
							replaceUnitWithType(unit, UnitId::ProtossAugur);
							unit->sprite->flags &= ~CSprite_Flags::IscriptCode;
							if(unit->sprite->images.head != nullptr) {
								CImage* current_image = unit->sprite->images.head;
								do {
									
									current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
									current_image = current_image->link.next;
								}while (current_image != nullptr);
							}
							changeUnitButtonSet_Sub4E5D60(unit,UnitId::None);
							scbw::refreshConsole();
							unit->status &= ~UnitStatus::IsBuilding; //disable the flag if set
							setArchonPathing(unit);
							unit->orderComputerCL(OrderId::CompletingArchonSummon);
							return;
						}
						else {
							unit->eggOrigin = unit->id;
							//morph into egg from source unit
							replaceUnitWithType(unit, eggId);
						}
						
						changeUnitButtonSet_Sub4E5D60(unit, unit->id);
						unit->remainingBuildTime = units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]];
						unit->mainOrderState = 1;
						auto origin = unit->eggOrigin;
						if (origin == UnitId::ZergZergling) {
							unit->sprite->playIscriptAnim(IscriptAnimation::Unused1, true);
						}
						else if (origin == UnitId::ZergUltrakor 
							|| origin==UnitId::ZergOvileth) { // Iroleth uses air cocoon
							unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
						}
						else if (origin == UnitId::ZergLarva) {
							unit->sprite->playIscriptAnim(IscriptAnimation::Walking, true);
						}
					}

				}

			}

		}
		else
			if (unit->mainOrderState == 1) {
				//advance
				if (advanceRemainingBuildTime_Sub466940(unit)) {
					auto origin = unit->eggOrigin;
					auto anim = IscriptAnimation::SpecialState1;
					/*if (origin == UnitId::ZergLarva) {
						anim = IscriptAnimation::GndAttkInit;
					}*/
					unit->sprite->playIscriptAnim(anim, true);
					unit->mainOrderState = 2;
				}
			}
			else
				if (unit->mainOrderState == 2) {

					if (unit->orderSignal & 4) {

						unit->orderSignal -= 4;
						auto origin = unit->eggOrigin;
						switch (origin) {
						case UnitId::ZergOvileth:
							scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 1);
							break;
						case UnitId::ZergIroleth:
							scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 2);
							break;
						default:
							break;
						}

						replaceUnitWithType(unit, unit->buildQueue[unit->buildQueueSlot]);
						if (
							unit->id == UnitId::ZergDroleth ||
							unit->id == UnitId::ZergOvileth
							)
							AI_TrainingUnit(unit, unit);

						playMorphingCompleteSound(unit);
						unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
						if (units_dat::ConstructionGraphic[unit->id] != 0) {

							CImage* current_image;

							function_004E65E0(unit, 1);

							current_image = unit->sprite->images.head;

							while (current_image != nullptr) {
								auto anim = IscriptAnimation::SpecialState1;
								/*if (origin == UnitId::ZergLarva) {
									anim = IscriptAnimation::GndAttkInit;
								}*/
								current_image->playIscriptAnim(anim);
								current_image = current_image->link.next;
							}

							unit->orderComputerCL(OrderId::ZergBirth);

						}
						else { //5DF60

							//update various stuff (set hp, set shield...) not finished on Morph

							function_004A01F0(unit);

							updateUnitStrength(unit);
							auto origin = unit->eggOrigin;
							if (unit->id == UnitId::ZergEgg && origin==UnitId::ZergLarva)
								orderNewUnitToRally(unit, unit->connectedUnit);

						}

					}

				}

	}

	;

	//Check if @p unit can morph into @p morphUnitId.
	bool unitCanMorphHook(CUnit* unit, uint16 morphUnitId) {
		//Default StarCraft behavior

		if (unit->id == UnitId::ZergZergling) {
			if (morphUnitId == UnitId::ZergVorvaling
			||	morphUnitId == UnitId::ZergNathrokor) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergHydralisk) {
			if (morphUnitId == UnitId::ZergBactalisk
				|| morphUnitId == UnitId::ZergLakizilisk) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergGorgrokor) {
			if (morphUnitId == UnitId::ZergGeszithalor) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergQuazilisk) {
			if (morphUnitId == UnitId::ZergKalkalisk
				|| morphUnitId == UnitId::ZergAlmaksalisk) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergUltrakor) {
			if (morphUnitId == UnitId::ZergKeskathalor
			||	morphUnitId == UnitId::ZergAlkajelisk) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergZoryusthaleth) {
			if (morphUnitId == UnitId::ZergIsthrathaleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergOvileth) {
			if (morphUnitId == UnitId::ZergIroleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ZergIroleth) {
			if (morphUnitId == UnitId::ZergOthstoleth) {
				return true;
			}
		}

		if (unit->id == UnitId::ProtossCantavis) {
			if (morphUnitId == UnitId::ProtossAugur) {
				return true;
			}
		}
		if (unit->id == UnitId::ZergMutalisk) {
			if (morphUnitId == UnitId::ZergVithrilisk) {
				return true;
			}
			if (morphUnitId == UnitId::ZergGorgrokor) {
				return true;
			}
		}
		if (unit->id == UnitId::ZergLarva) {
			if (unit->canMakeUnit(morphUnitId, *ACTIVE_NATION_ID) == 1) {
				return true;
			}
		}
		return false;
	}

	//Check if @p unitId is an egg unit that can be rallied
	bool isRallyableEggUnitHook(uint16 unitId) {
		//Default StarCraft behavior
		if (scbw::isZergEggNew(nullptr,false,false,unitId))
			return false;

		return true;
	}

	const uint32 Func_RemoveGasRefs = 0x00432760;
	void removeAITownGasReferences(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ECX,unit
			CALL Func_RemoveGasRefs
			POPAD
		}
	}
	const uint32 Func_DeleteBuildingAi = 0x00404500;
	void deleteBuildingAi(void* building_ai_list, BuildingAi* ai) {
		__asm {
			PUSHAD
			MOV EDX,building_ai_list
			MOV EAX,ai
			CALL Func_DeleteBuildingAi
			POPAD
		}
	}
	const uint32 Func_DeleteWorkerAi = 0x00404470;
	void deleteWorkerAi(void* worker_ai_list, WorkerAi* ai) {
		__asm {
			PUSHAD
			MOV EDX,worker_ai_list
			MOV EAX,ai
			CALL Func_DeleteWorkerAi
			POPAD
		}
	}
	const uint32 Func_DeleteAiTownIfNeeded = 0x004347E0;
	void deleteAiTownIfNeeded(AiTown* town) {
		__asm {
			PUSHAD
			MOV EAX,town
			CALL Func_DeleteAiTownIfNeeded
			POPAD
		}
	}
/*	uint32 remove_worker_or_building_ai(CUnit* unit, uint8 assert_is_building) {
		BuildingAi* ai;
		AiTown* town;
		removeAITownGasReferences(unit);
		ai = (BuildingAi*)unit->pAI;
		auto result = 0;
		if (ai)
		{
			if (assert_is_building
				|| units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->id != UnitId::ResourceVespeneGeyser
				|| unit->id == UnitId::ZergLarva
				|| unit->id == UnitId::ZergEgg
				//|| scbw::isLarvalEgg(unit)
				|| unit->id == UnitId::ZergOverlord)
			{
				town = ai->town;
				auto free = town->free_buildings;
				deleteBuildingAi(free, ai);
			}
			else
			{
				
				if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Worker))
					return 0;
				town = ai->town;
				deleteWorkerAi(town->free_workers, (WorkerAi*)ai);
				town->worker_count--;
			}
			if (town)
			{
				if (unit == town->building_scv)
					town->building_scv = nullptr;
				if (unit == town->main_building)
					town->main_building = nullptr;
			}
			if (!assert_is_building)
			{
				if (town)
					deleteAiTownIfNeeded(town);
			}
			unit->pAI = nullptr;
			result = 1;
		}
		return result;
	}
	*/

	//Return the ID of the egg unit to use when morphing @p unitId.
	//If the unit cannot morph, return UnitId::None.
	uint16 getUnitMorphEggTypeHook(uint16 unitId, CUnit* unit, uint16 resultId) {
		//Default StarCraft behavior

		if (units_dat::BaseProperty[unitId] & UnitProperty::Flyer) {
			return UnitId::ZergCocoon;
		}
		else {
			return UnitId::ZergEgg;
		}
		return UnitId::None;
	}

	//Determine the type (unit ID) of the unit to revert to when cancelling an
	//@p eggUnit while it is morphing.
	uint16 getCancelMorphRevertTypeHook(CUnit* eggUnit) {
		//Default StarCraft behavior

		auto source = eggUnit->eggOrigin;
		if (eggUnit->getRace() == RaceId::Zerg
		&&  eggUnit->id != UnitId::ZergInfestedTerran) {
			if (!(units_dat::BaseProperty[eggUnit->id] & UnitProperty::Building)
			   && source != UnitId::ZergLarva) {
				return source;
			}
		}
		return UnitId::None;  //Default (no revert for larvae)
	}

	//Determines the vertical (Y) offset by which the @p unit will be shifted to
	//when it finishes morphing.
	int16 getUnitVerticalOffsetOnBirth(CUnit* unit) {
		//Default StarCraft behavior

		//No offset, birth offset is handled elsewhere
		if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg)
			return 0;

		//No offset, since the morphed unit should stay where it is
		if (unit->previousUnitType == UnitId::ZergCocoon) {
			return 0;
		}

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
	bool hasSuppliesForUnitHook(uint8 playerId, uint16 unitId, bool canShowErrorMessage) {
		//Default StarCraft behavior
		int32 supplyCost = units_dat::SupplyRequired[unitId];

		if (units_dat::BaseProperty[unitId] & UnitProperty::TwoUnitsIn1Egg)
			supplyCost *= 2;

		if (unitId == UnitId::ZergLakizilisk)
			supplyCost -= units_dat::SupplyRequired[UnitId::ZergHydralisk];

		aiSupplyReserved[playerId] = supplyCost;

		//No supply cost check needed
		if (supplyCost == 0 || units_dat::BaseProperty[unitId] & UnitProperty::MorphFromOtherUnit)
			return true;

		const RaceId::Enum raceId = CUnit::getRace(unitId);
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

	CUnit* beginTrain(CUnit* unit, uint16 unit_id, bool notify_error) {
		if (unit->status & UnitStatus::GroundedBuilding
			|| unit->id==UnitId::ProtossSolarion
//			|| unit->id==UnitId::Hero_Gantrithor
			|| unit->id==UnitId::ProtossAccantor
//			|| unit->id==UnitId::Hero_Warbringer
			|| scbw::isLarvalEgg(unit))//just egg previously
		{
			if (hasSuppliesForUnit(unit_id,unit->playerId,notify_error))
			{
				CUnit* result = scbw::CreateUnit(unit_id, unit->getX(), unit->getY(), unit->playerId);
				if (result)
					return result;
				scbw::displayLastNetErrForPlayer(unit->playerId);
			}
		}
		return 0;
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
	bool hasSuppliesForUnit(uint32 unitId, uint32 playerId, bool32 canShowErrorMessage) {

		static bool32 bPreResult;

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

	uint32 Func_Sub466940 = 0x00466940;
	//return true if remainingBuildTime was 0 when called,
	//remainingBuildTime change affected by cheating
	bool advanceRemainingBuildTime_Sub466940(CUnit* unit) {

		static bool32 bTimeWasZero;

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
	void refundAllQueueSlots(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RefundAllQueueSlots
			POPAD
		}

	}

	;

	const uint32 Func_Sub466F50 = 0x00466F50;
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

	const uint32 Func_ActUnitReturnToIdle = 0x00475420;
	void actUnitReturnToIdle(CUnit* unit) {

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
	void incrementUnitScoresEx(CUnit* unit, int32 count, int32 unk2) {

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

	const uint32 Func_PlayMorphingCompleteSound = 0x0048F440;
	void playMorphingCompleteSound(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_PlayMorphingCompleteSound
			POPAD
		}

	}

	;

	const uint32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}

	}

	;

	const uint32 Func_ReplaceUnitWithType = 0x0049FED0;
	void replaceUnitWithType(CUnit* unit, uint16 newUnitId) {
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
	void function_004A01F0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const uint32 Func_AI_TrainingUnit = 0x004A2830;
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

	const uint32 Func_Sub4E5D60 = 0x004E5D60;
	void changeUnitButtonSet_Sub4E5D60(CUnit* unit, uint16 buttonSetId) {

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
	void function_004E65E0(CUnit* unit, bool32 flag) {

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
