#include "hallucination_spell.h"
#include <SCBW/api.h>

//helper functions def

namespace {

	bool ordersSpell_Sub_4926D0(CUnit* unit, u32 techId, u16* techEnergyCost, u32 sightRange,u32 error_message_index);	// 926D0
	void UnitDestructor(CUnit* unit);																					// A0990
	bool function_004F66D0(CUnit* unit);																				// 004F66D0	// called PlaceHallucination in Neiv's list

	// for createHallucination
	CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId);														// 004A09D0
	void applyHallucinationStats(CUnit* halUnit);																		// 004F6180
	s32 hallucinateScarabs(CUnit* a1, CUnit* a2);																		// 004F65B0
	void AI_TrainingUnit(CUnit* result, CUnit* parent);																	// 004A2830
	void displayLastNetErrForPlayer(u8 a1);																				// 0049E530

} //unnamed namespace

namespace hooks {

	void orders_Hallucination1(CUnit* unit) {
		int sightRange = unit->getSightRange(true);
		u16 techCost;
		if(
			ordersSpell_Sub_4926D0(
				unit,
				TechId::Hallucination,
				&techCost,
				sightRange * 32,
				0x899 //Invalid Target
			)
		) {
			int counter = 1;
			// TODO: cleanup
			//while(counter > 0) {
			//
			//	counter--;
			//
			//	CUnit* hal_unit = hallucinationHelpers::createHallucinationUnit(unit->orderTarget.unit,unit->playerId);
			//
			//	if(hal_unit == NULL)
			//		counter = 0;
			//	else
			//	if(!hallucinationHelpers::function_004F66D0(hal_unit)) {
			//		counter = 0;
			//		hallucinationHelpers::UnitDestructor(hal_unit);
			//	}
			//
			//}
			CUnit* target = unit->orderTarget.unit;
			scbw::sendGPTP_aise_cmd(target, 0, GptpId::SetGenericValue, ValueId::HallucinatingSourcePlayer, unit->playerId, 0);
			scbw::sendGPTP_aise_cmd(target, 0, GptpId::SetGenericTimer, ValueId::HallucinatingTimer, (30 * 24) + 1, 0);

			if(!(*CHEAT_STATE & CheatFlags::TheGathering))
				unit->spendUnitEnergy(techCost);

			scbw::playSound(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, target);

			if(unit->orderTarget.unit->subunit != NULL)
				target->subunit->sprite->createOverlay(ImageId::HallucinationHit,0,0,0);
			else
				target->sprite->createOverlay(ImageId::HallucinationHit,0,0,0);

			if(unit->orderQueueHead != NULL) {
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			if(unit->pAI != NULL)
				unit->orderComputerCL(OrderId::ComputerAI);
			else
				unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
		}
	}

	// Hooked to rule out AI issues -- Pr0nogo
	CUnit* createHallucination(CUnit* source, s32 player) {
		CUnit* halUnit; // eax
		CUnit* v3; // edi
		CUnit* halUnit2; // esi
		CUnit* result; // eax
		u16 halTimer; // cx
		u16 currentHalTimer; // ax
		s16 unitType; // ax

		v3 = source;
		halUnit = createUnit(source->id, source->sprite->position.x, source->sprite->position.y, player);
		halUnit2 = halUnit;
		if (halUnit) {
			applyHallucinationStats(halUnit);
			if (halUnit2->status & UnitStatus::IsHallucination)
				halTimer = 1350;
			else
				halTimer = halUnit2->id != UnitId::ZergBroodling ? 0 : 1800;
			currentHalTimer = halUnit2->removeTimer;
			if (!currentHalTimer || halTimer < currentHalTimer)
				halUnit2->removeTimer = halTimer;
			unitType = halUnit2->id;
			if (unitType == UnitId::ProtossAccantor)	// Originally also ran for Warbringer ID
				hallucinateScarabs(halUnit2, v3);
			AI_TrainingUnit(halUnit2, halUnit2);
			result = halUnit2;
		}
		else {
			displayLastNetErrForPlayer(player);
			result = 0;
		}
		return result;
	}

} //namespace hooks

namespace {

	const u32 Func_Sub_4926D0 = 0x004926D0;
	bool ordersSpell_Sub_4926D0(CUnit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index) {
		static Bool32 bPreResult;
		__asm {
			PUSHAD
			PUSH techEnergyCost
			PUSH sightRange
			PUSH techId
			MOV EAX, error_message_index
			MOV EBX, unit
			CALL Func_Sub_4926D0
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	const u32 Func_UnitDestructor = 0x004A0990;
	void UnitDestructor(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_UnitDestructor
			POPAD
		}
	}

	const u32 Func_sub_4F66D0 = 0x004F66D0;
	bool function_004F66D0(CUnit* unit) {
		static Bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_sub_4F66D0
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	// for createHallucination

	const u32 Func_createUnit = 0x004A09D0;
	CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId) {
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV EAX, x
			MOVZX CX, unitType
			CALL Func_createUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_applyHallucinationStats = 0x004F6180;
	void applyHallucinationStats(CUnit* halUnit) {
		__asm {
			PUSHAD
			MOV EAX, halUnit
			CALL Func_applyHallucinationStats
			POPAD
		}
	}

	const u32 Func_hallucinateScarabs = 0x004F65B0;
	s32 hallucinateScarabs(CUnit* a1, CUnit* a2) {
		static s32 result;
		__asm {
			PUSHAD
			PUSH a2
			MOV ESI, a1
			CALL Func_hallucinateScarabs
			MOV EAX, result
			POPAD
		}
		return result;
	}

	const u32 Func_AI_TrainingUnit = 0x004A2830;
	void AI_TrainingUnit(CUnit* result, CUnit* parent) {
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingUnit
			POPAD
		}
	}

	const u32 Func_displayLastNetErrForPlayer = 0x0049E530;
	void displayLastNetErrForPlayer(u8 a1) {
		__asm {
			PUSHAD
			PUSH a1
			CALL Func_displayLastNetErrForPlayer
			POPAD
		}
	}
}