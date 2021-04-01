#include "recall_spell.h"
#include <SCBW/api.h>

//helper functions def

namespace {
	//7B720
bool ordersSpell_Sub_4926D0(CUnit* unit, u32 techId, u16* techEnergyCost, u32 sightRange,u32 error_message_index);	//926D0
void MindControlHit(CUnit* attacker, CUnit* target);																//F6910

} //unnamed namespace

namespace hooks {

	void ordersCastMindControl(CUnit* unit) {

		CUnit* target = unit->orderTarget.unit;

		if(target == nullptr) {

			if(unit->orderQueueHead != nullptr) {
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else {
				if(unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}

		}
		else {

			//F69B4

			if(unit->playerId == target->playerId)
				unit->orderToIdle();
			else {

				u16 techEnergyCost = 0;
				bool spellCastSuccess;

				//F69CB

				u32 unitSightRangeForSpell = unit->getSightRange(true);

				unitSightRangeForSpell *= 32;

				//energy cost is initialized in this function
				//Corresponding error message is "Invalid Target"
				spellCastSuccess = ordersSpell_Sub_4926D0(unit, TechId::MindControl, &techEnergyCost, unitSightRangeForSpell, 0x052E);

				if(spellCastSuccess) {

					if(target->status & UnitStatus::IsHallucination)
						target->remove();
					else {

						//overlayId = overlaySize (0,1,2) + MinimumSizeImageId
						u32 overlayId = scbw::hasOverlay(target) + ImageId::MindControlHit_Small;

						////F6A08 (direct implementation of 0x00047B720 hasOverlay)

						//if(units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay)
						//	overlayId = ImageId::MindControlHit_Medium;
						//else
						//if(units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay)
						//	overlayId = ImageId::MindControlHit_Large;
						//else
						//	overlayId = ImageId::MindControlHit_Small;

						if(target->subunit != nullptr)
							target->subunit->sprite->createTopOverlay(overlayId, 0, 0, 0);
						else
							target->sprite->createTopOverlay(overlayId, 0, 0, 0);
						scbw::sendGPTP_aise_cmd(unit, target, GptpId::SetMindControlLink, 0, 0, 0);
						MindControlHit(unit, target);
						//scbw::printText("Set...");
						
						target->orderToIdle();

					}

					//F6A42

//					unit->shields = 0;

					if (!(*CHEAT_STATE & CheatFlags::TheGathering)) {
						unit->spendUnitEnergy(techEnergyCost);
					}
					//F6A5E
					scbw::playSound(SoundId::Protoss_Darchon_mind_wav, target);

					unit->orderToIdle();

				} //if(spellCastSuccess)

			} //if (unit->playerId != target->playerId)

		} //if (target != nullptr)

	} //void ordersCastMindControl(CUnit* unit)

	;

} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Func_Sub_4926D0 = 0x004926D0;
bool ordersSpell_Sub_4926D0(CUnit* unit, u32 techId, u16* techEnergyCost, u32 sightRange,u32 error_message_index) {

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

;

const u32 Func_MindControl_Hit = 0x004F6910;
void MindControlHit(CUnit* attacker, CUnit* target) {

	__asm {
		PUSHAD
		MOV ESI, target
		PUSH attacker
		CALL Func_MindControl_Hit
		POPAD
	}

}

;

} //Unnamed namespace

//End of helper functions