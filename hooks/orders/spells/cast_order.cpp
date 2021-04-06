#include "cast_order.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
//helper functions def

namespace {

	uint32 getUpgradedWpnCooldown(CUnit* unit, uint8 weaponId);													//0x00475DC0
	bool function_00476640(CUnit* unit, uint32 weaponId);														//0x00476640
	void setUnitStatTxtErrorMsg(char* message);																//0x0048CCB0
	uint32 get_statTxt_Str_0(CUnit* unit, uint32 playerId, char* message);										//0x0048EF30
	bool canCastSpell_0(CUnit* unit);																		//0x00492140
	uint32 function_00492620(CUnit* unit, bool32 wasMainOrderStateZero);										//0x00492620
	uint32 RandomizeShort(uint32 unknown_index);																	//0x004DC4A0
	void setNextWaypoint_Sub4EB290(CUnit* unit);															//0x004EB290

} //unnamed namespace

namespace hooks {

	void orders_Spell(CUnit* unit) {

		int32 spellCost;
		uint8 orderWeaponId = orders_dat::OrderWeaponId[unit->mainOrderId];
		CUnit* target = unit->orderTarget.unit;
		if (unit->id == UnitId::TerranPaladin) {
			if (unit->orderSignal & 0x20) {
				unit->orderSignal &= ~0x20;
				scbw::set_generic_timer(unit, ValueId::SkyfallTimer, 3);
				scbw::set_generic_value(unit, ValueId::SkyfallStacks, 0);
				auto pt = unit->orderTarget.pt;
				if (unit->orderTarget.unit != nullptr) {
					pt = unit->orderTarget.unit->position;
					unit->orderTarget.unit = nullptr;
				}
				//scbw::printFormattedText("Fire skyfall %d %d -> %d %d",unit->getX(),unit->getY(),pt.x,pt.y);
				scbw::set_generic_value(unit, ValueId::SkyfallX, unit->orderTarget.pt.x);
				scbw::set_generic_value(unit, ValueId::SkyfallY, unit->orderTarget.pt.y);
			}
		}
		if (unit->id == UnitId::TerranWyvern) {
			if (unit->orderSignal & 0x40) {
				unit->orderSignal &= ~0x40;
				if (scbw::get_generic_rust_value(unit, ValueId::ReverseThrust) == 1) {
					scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabOn, 1);
					scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabX, unit->orderTarget.pt.x);
					scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabY, unit->orderTarget.pt.y);
				} else {
					scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabOn, 0);
				}

			}
			if (unit->orderSignal & 0x80) {
				unit->orderSignal &= ~0x80;
				unit->orderTo(OrderId::CTFCOPInit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			}
		}
		if (unit->mainOrderState == 2) {
			//		scbw::printText("State 2");
			if (unit->orderSignal & 2) {
				//			scbw::printText("Signal & 2");

				unit->orderSignal -= 2;
				unit->orderToIdle();
			} else { //92886
				//scbw::printText("No Signal 2 yet");
				if (target != nullptr) {
					//scbw::printText("Target exists");

					unit->orderTarget.pt.x = target->sprite->position.x;
					unit->orderTarget.pt.y = target->sprite->position.y;

					if (
						target->sprite->position.x != unit->nextTargetWaypoint.x ||
						target->sprite->position.y != unit->nextTargetWaypoint.y
						) { //928C1
						unit->nextTargetWaypoint.x = target->sprite->position.x;
						unit->nextTargetWaypoint.y = target->sprite->position.y;
					}

				}

			}

		} else { //928CF
			//scbw::printText("State != 2");
			if (!canCastSpell_0(unit)) {
				char* message = (char*)statTxtTblNew->getString(876); //Invalid Target

				//do the actual transmission corresponding to the message
				get_statTxt_Str_0(unit, unit->playerId, message);

				//make the unit stop since the order failed
				setNextWaypoint_Sub4EB290(unit);
				unit->orderToIdle();

			} else { //9291A
				//scbw::printText("Continue...");

				uint8 techUsed = orders_dat::TechUsed[unit->mainOrderId];

				if (techUsed >= TechId::None)
					spellCost = 0;
				else
					spellCost = techdata_dat::EnergyCost[techUsed] * 256;

				if (
					!(*CHEAT_STATE & CheatFlags::TheGathering) &&
					unit->energy < spellCost
					) {
					//scbw::printText("energy < spellcost");
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

					message = (char*)statTxtTblNew->getString(864 + raceId); //Not Enough Energy

					if (message != nullptr) {

						//display an error message only for the owner
						//of the unit
						if (*LOCAL_NATION_ID == unit->playerId &&
							*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID
							)
							setUnitStatTxtErrorMsg(message);

					}

					unit->orderToIdle();

				} else { //929C2

					uint32 result_00492620;
					//scbw::printText("energy >= spellcost");
					//may initiate movement from unit depending on
					//mainOrderState
					//scbw::printFormattedText("Order state: %d", unit->mainOrderState);
					if (unit->mainOrderState == 0)
						result_00492620 = function_00492620(unit, 1);
					else
						result_00492620 = function_00492620(unit, 0);

					if (unit->mainOrderState == 0)
						unit->mainOrderState = 1;
					bool gcd_condition = true;
					if (unit->id == UnitId::TerranWyvern && unit->groundWeaponCooldown > 0) {
						gcd_condition = false;//new
					}

					if (result_00492620 == 0) {

						if (unit->getMovableState() == 2) //unmovable
						{
							//scbw::printText("to Idle");
							unit->orderToIdle();
						}

					} else { //92A01

						if (
							unit->getSpellCountdown() == 0 &&
							gcd_condition && //new
							function_00476640(unit, orderWeaponId) && //is ready to fire weapon (angle,range...) maybe?
							!(unit->movementFlags & MovementFlags::Accelerating) &&
							unit->getMovableState() != 0
							) { //92A38
								//scbw::printText("Conditions good");
							bool bStopHere = false;

							//in case it was modified by a previous function,
							//relinking
							target = unit->orderTarget.unit;

							if (unit->pAI != nullptr && target != nullptr) {

								if (orderWeaponId <= WeaponId::Maelstrom) { //Maelstrom is the last spell-related weapon

									//array bound to the function
									const uint8* const array_00492B44 = (uint8*)(0x00492B44);

									//this code would not work for weapons id of NuclearMissile, YamatoGun 
									//and weapons id before those, they must be excluded somewhere else
									//Maybe the switch and array use could be removed in a future version
									//since enough elements use default value (switch 6)
									uint32 switch_value = array_00492B44[orderWeaponId - 0x20];
									//scbw::printText("Get switch value");

									if (switch_value != 6) {

										//no idea why irradiate is so important
										uint32 analyzed_target_status = target->irradiateTimer;

										if (switch_value == 2)
											analyzed_target_status |= target->parasiteFlags;

										if (switch_value == 3)
											analyzed_target_status |= target->ensnareTimer;

										if (switch_value == 4)
											analyzed_target_status |= target->plagueTimer;

										if (switch_value == 5)
											analyzed_target_status |= target->isBlind;

										if (switch_value == 0 && unit->isFrozen())
											analyzed_target_status |= 1;

										if (analyzed_target_status != 0) {
											unit->orderToIdle();
											//scbw::printText("Bad analysis");
											bStopHere = true;
										}

									}

								} //if(orderWeaponId <= WeaponId::Maelstrom)

							} //if(unit->pAI != nullptr && target != nullptr)

							if (!bStopHere) { //92AA3

	//							scbw::printText("Spend energy");
								uint8 cooldown;
								uint32 random_value;

								unit->spendUnitEnergy(spellCost);
								cooldown = getUpgradedWpnCooldown(unit, orderWeaponId);

								//a complicated way to add between -1 and 2 to the cooldown
								random_value = RandomizeShort(49);
								cooldown += (((uint8)random_value) & 3) - 1;

								unit->spellCooldown = cooldown;
								if (unit->id == UnitId::TerranWyvern) {
									unit->groundWeaponCooldown = cooldown;//new
								}


								//disable flag 2 if enabled, else has no effect
								unit->orderSignal &= ~2;	//&= -3

								unit->movementFlags |= MovementFlags::StartingAttack;

								if (unit->sprite->images.head != nullptr) {

									CImage* current_image = unit->sprite->images.head;
									IscriptAnimation::Enum anim = (IscriptAnimation::Enum)orders_dat::OrderIscriptAnim[unit->mainOrderId];
									if (techUsed == TechId::Lockdown && unit->vulture.spiderMineCount > 0) {
										unit->vulture.spiderMineCount--;
									}
									while (current_image != nullptr) {
										current_image->playIscriptAnim(anim);
										current_image = current_image->link.next;
									}

								}

								unit->mainOrderState = 2;

								if (unit->pAI != nullptr)
									AIScriptController[unit->playerId].spellcasterTimer = 1;

							}

						}

					} //if(result_00492620 != 0)

				} //got enough energy

			} //canCastSpell_0(unit)

		} //unit->mainOrderState != 2

	} //void orders_Spell(CUnit* unit)

	;

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const uint32 Func_getUpgradedWpnCooldown = 0x00475DC0;
	uint32 getUpgradedWpnCooldown(CUnit* unit, uint8 weaponId) {

		static uint32 return_value;

		__asm {
			PUSHAD
			MOV AL, weaponId
			MOV ESI, unit
			CALL Func_getUpgradedWpnCooldown
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_Sub476640 = 0x00476640;
	bool function_00476640(CUnit* unit, uint32 weaponId) {

		static bool32 pre_result;

		__asm {
			PUSHAD
			PUSH weaponId
			MOV EAX, unit
			CALL Func_Sub476640
			MOV pre_result, EAX
			POPAD
		}

		return (pre_result != 0);

	}

	;

	const uint32 Func_setUnitStatTxtErrorMsg = 0x0048CCB0;
	void setUnitStatTxtErrorMsg(char* message) {

		__asm {
			PUSHAD
			MOV EAX, message
			CALL Func_setUnitStatTxtErrorMsg
			POPAD
		}

	}

	;

	const uint32 Func_get_statTxt_Str_0 = 0x0048EF30;
	uint32 get_statTxt_Str_0(CUnit* unit, uint32 playerId, char* message) {

		static uint32 return_value;

		__asm {
			PUSHAD
			MOV EAX, message
			MOV ECX, playerId
			MOV EDX, unit
			CALL Func_get_statTxt_Str_0
			MOV EAX, return_value
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_canCastSpell_0 = 0x00492140;
	bool canCastSpell_0(CUnit* unit) {

		static bool32 bPreResult;

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_canCastSpell_0
			MOV bPreResult, EAX
			POPAD
		}

		//0 means "can cast spell"
		return (bPreResult == 0);

	}

	;

	const uint32 Func_Sub492620 = 0x00492620;
	uint32 function_00492620(CUnit* unit, bool32 wasMainOrderStateZero) {

		static uint32 return_value;

		__asm {
			PUSHAD
			PUSH wasMainOrderStateZero
			MOV EAX, unit
			CALL Func_Sub492620
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_RandomizeShort = 0x004DC4A0;
	uint32 RandomizeShort(uint32 unknown_index) {

		static uint32 return_value;

		__asm {
			PUSHAD
			MOV EAX, unknown_index
			CALL Func_RandomizeShort
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	//Related to path/movement decision
	const uint32 Func_sub_4EB290 = 0x004EB290;
	void setNextWaypoint_Sub4EB290(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_sub_4EB290
			POPAD
		}
	}


	;

} //Unnamed namespace

//End of helper functions
