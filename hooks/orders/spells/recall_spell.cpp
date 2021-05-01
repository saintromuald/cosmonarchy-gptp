#include "recall_spell.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"

//helper functions def
namespace hooks {

	//Originally known as sub_4942D0
	//Function performed on each target of Recall
	//by IterateUnitsAtLocationTargetProc_RecallTargets
	//that was validated throug spells_Recall_ActionOnTargetInRange
	//Do the actual unit movement
	void spells_Recall_ActionOnValidTargetForRecall(CUnit* target, CUnit* caster) {

		static bool32* const bAtLeast1UnitWasRecalled = (bool32*)(0x0063FF50);

		Point16 previous_pos, new_pos;

		previous_pos.x = target->sprite->position.x;
		previous_pos.y = target->sprite->position.y;

		//identical code to setUnitPosition() but for some reason, is
		//using another function to do exactly the same thing
		helper::function_004EBAE0(target, caster->sprite->position.x, caster->sprite->position.y);

		if (!scbw::checkUnitCollisionPos(target, &(caster->sprite->position), &new_pos, nullptr, false, 0))
			//collision detected, cancel the movement
			scbw::setUnitPosition(target, previous_pos.x, previous_pos.y);
		else {
			CThingy* recall_effect_sprite;

			//finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
			scbw::prepareUnitMove(target, false);
			target->orderTarget.unit = nullptr;
			scbw::setUnitPosition(target, new_pos.x, new_pos.y);
			scbw::refreshUnitAfterMove(target);

			//stop some currently active orders
			if (target->id != UnitId::ZergCocoon)
				target->orderComputerCL(units_dat::ReturnToIdleOrder[target->id]);

			//create the Recall animation over the unit
			recall_effect_sprite = helper::createThingy(SpriteId::Recall_Field, new_pos.x, new_pos.y, 0);
			if (recall_effect_sprite != nullptr) {
				recall_effect_sprite->sprite->elevationLevel = target->sprite->elevationLevel + 1;
				helper::setThingyVisibilityFlags(recall_effect_sprite);
			}

			//disconnect a nuke from a ghost using it
			if (
				target->id == UnitId::TerranEidolon ||
				target->id == UnitId::Hero_SarahKerrigan ||
				target->id == UnitId::Hero_AlexeiStukov ||
				target->id == UnitId::Hero_SamirDuran ||
				target->id == UnitId::Hero_InfestedDuran
				) {

				if (
					target->connectedUnit != nullptr &&
					target->connectedUnit->id == UnitId::TerranNuclearMissile
					) {
					(target->connectedUnit)->connectedUnit = nullptr;
					target->connectedUnit = nullptr;
				}

			}

			*bAtLeast1UnitWasRecalled = 1;

		}

	} //void spells_Recall_ActionOnValidTargetForRecall(CUnit* target, CUnit* caster)

	;

	//Originally known as sub_494400
	//Function performed on each target of Recall
	//by IterateUnitsAtLocationTargetProc_RecallTargets
	//to choose if the unit can have the Recall effect
	//cast on it or no, and if yes, calling another function
	//to do the Recall effect.
	void spells_Recall_ActionOnTargetInRange(CUnit* target, CUnit* caster) {

		if (
			!(target->sprite->flags & CSprite_Flags::Hidden) &&
			(target->playerId == caster->playerId || scbw::isAlliedTo(target->playerId, caster->playerId)) &&
			!(units_dat::BaseProperty[target->id] & UnitProperty::Building) &&
			!(target->status & UnitStatus::Invincible) &&
			!(target->status & UnitStatus::Burrowed) &&
			target->id != UnitId::ZergLarva &&
			target->id != UnitId::ZergEgg &&
			!(target->status & UnitStatus::IsHallucination) &&
			target != caster
			)
			spells_Recall_ActionOnValidTargetForRecall(target, caster);

	}

	;

	void orders_Recall(CUnit* unit) {

		static bool32* const bAtLeast1UnitWasRecalled = (bool32*)(0x0063FF50);

		//unit has started casting Recall (== went through the other condition)
		if (unit->mainOrderState == 1) {
			//execute the recall effect when timer reach 0
			if (unit->mainOrderTimer == 0) {
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				Box16 spell_location;

				spell_location.left = unit->orderTarget.pt.x - 64;
				spell_location.top = unit->orderTarget.pt.y - 64;
				spell_location.right = unit->orderTarget.pt.x + 64;
				spell_location.bottom = unit->orderTarget.pt.y + 64;

				*bAtLeast1UnitWasRecalled = 0;

				//apply the recal effect on each valid target found in spell_location
				helper::IterateUnitsAtLocationTargetProc_RecallTargets(unit, &spell_location);

				if (*bAtLeast1UnitWasRecalled != 0) {

					//original code added (RandomizeShort(0x12) & 1) to the base sound
					//id Protoss_ARBITER_PAbFol02_WAV then called the playSound function
					//using that value, but since there are only 2 sounds, it's easier to 
					//see what sound is used that way.
					//The compiler may make it back like the original anyway.

					if ((helper::RandomizeShort(0x12) & 1) == 0)
						scbw::playSound(SoundId::Protoss_ARBITER_PAbFol02_WAV, unit);
					else
						scbw::playSound(SoundId::Protoss_ARBITER_PAbFol03_WAV, unit);

				}

				unit->orderToIdle();

			}

		} else { //94505

			if (
				!scbw::isCheatEnabled(CheatFlags::TheGathering) &&
				unit->energy < techdata_dat::EnergyCost[TechId::Recall] * 256
				) {

				uint8 raceId;
				char* message;

				if (units_dat::GroupFlags[unit->id].isZerg)
					raceId = RaceId::Zerg;
				else
					if (units_dat::GroupFlags[unit->id].isTerran)
						raceId = RaceId::Terran;
					else
						if (units_dat::GroupFlags[unit->id].isProtoss)
							raceId = RaceId::Protoss;
						else
							raceId = RaceId::Neutral;

				message = (char*)statTxtTblNew->getString(864 + raceId);	//"Not enough energy" text

				if (
					message != nullptr &&
					unit->playerId == *LOCAL_NATION_ID &&
					*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID
					)
					helper::setUnitStatTxtErrorMsg(message);

				unit->orderToIdle();

			} else { //945B9

				CThingy* recall_effect_sprite;
				uint32 random_value;

				if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
					unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::Recall] * 256);

				if (unit->orderTarget.unit != nullptr) {
					unit->orderTarget.pt.x = unit->orderTarget.unit->sprite->position.x;
					unit->orderTarget.pt.y = unit->orderTarget.unit->sprite->position.y;
				}

				recall_effect_sprite =
					helper::createThingy(
						SpriteId::Recall_Field,
						unit->orderTarget.pt.x,
						unit->orderTarget.pt.y,
						0
					);

				if (recall_effect_sprite != nullptr) {
					recall_effect_sprite->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
					helper::setThingyVisibilityFlags(recall_effect_sprite);
				}

				//94617
				if (*IS_IN_GAME_LOOP) {

					//hardcoded (not using a function) RNG use, like
					//the original code do

					static uint32* const u32_0051C654 = (uint32*)0x0051C654;
					static uint32* const u32_0051CA18 = (uint32*)0x0051CA18;

					*u32_0051C654 = *u32_0051C654 + 1;
					*u32_0051CA18 = *u32_0051CA18 + 1;
					*lastRandomNumber = (*lastRandomNumber * 0x015A4E35) + 1;

					random_value = ((*lastRandomNumber) / 65536) & 0x7FFF;

				} else
					random_value = 0;

				//original code added (random_value & 1) to the base sound
				//id Protoss_ARBITER_PAbFol00_WAV then called the playSound function
				//using that value, but since there are only 2 sounds, it's easier to 
				//see what sound is used that way.
				//The compiler may make it back like the original anyway.

				if ((random_value & 1) == 0)
					helper::playUnitSFX(
						SoundId::Protoss_ARBITER_PAbFol00_WAV,
						unit->orderTarget.pt.x,
						unit->orderTarget.pt.y,
						1,
						0
					);
				else
					helper::playUnitSFX(
						SoundId::Protoss_ARBITER_PAbFol01_WAV,
						unit->orderTarget.pt.x,
						unit->orderTarget.pt.y,
						1,
						0
					);

				//				unit->mainOrderTimer = 22;
				unit->status |= UnitStatus::CanNotReceiveOrders;
				unit->mainOrderTimer = 3 * 24;
				unit->mainOrderState = 1;

			}

		}

	} //void orders_Recall(CUnit* unit)

	;

} //namespace hooks
