#include "unit_train.h"
#include <SCBW/api.h>
#include "hooks/hydraFinder.h"
#include "globals.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//helper functions def

namespace {

	uint32 getHPGainForRepair(CUnit* unit);										//02C40
	void refundUnitTrainCost(uint32 unitId, uint8 playerId);							//2CEC0
	void function_00432430(CUnit* unit, uint32 buildQueueSlot);					//32430
	void addHangarUnit(CUnit* main_unit, CUnit* added_unit);					//66300
	void orderNewUnitToRally(CUnit* unit, CUnit* factory);						//66F50
	bool32 buildingAddon(CUnit* unit, uint32 hpGain, bool32 canBeAborted);			//679A0
	CUnit* attemptTrainHatchUnit(CUnit* trainer, uint32 builtUnitID, uint32 flag);	//68200
	void AI_CancelStructure(CUnit* unit);										//68280
	void AI_TrainingUnit(CUnit* unit_creator, CUnit* created_unit);				//A2830
	void hideAndDisableUnit(CUnit* unit);										//E6340

} //unnamed namespace




namespace hooks {
	int isQueueSlotActive(CUnit* unit, uint32 slot) {
		//return unit->buildQueue[(slot + unit->buildQueueSlot) % 5] <= UnitId::Spell_DisruptionWeb;
		return unit->buildQueue[(slot + unit->buildQueueSlot) % 5] < UnitId::None;
	}

	void secondaryOrd_TrainFighter(CUnit* unit) {
		if (
			unit->id == UnitId::ProtossSolarion ||
			unit->id == UnitId::Hero_Gantrithor ||
			unit->id == UnitId::ProtossAccantor ||
			unit->id == UnitId::Hero_Warbringer
			) {
			if (unit->secondaryOrderState == 0 || unit->secondaryOrderState == 1) {

				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::None) {
					unit->secondaryOrderState = 3;
					unit->currentBuildUnit = nullptr;
				} else {

					CUnit* builtUnit;

					if (unit->secondaryOrderState == 0)
						builtUnit = attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 1);
					else
						builtUnit = attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 0);

					unit->currentBuildUnit = builtUnit;

					if (builtUnit == nullptr)
						unit->secondaryOrderState = 1;
					else {
						builtUnit->interceptor.parent = unit;
						unit->secondaryOrderState = 2;
					}

				}

			} else
				if (unit->secondaryOrderState == 2) {

					CUnit* builtUnit = unit->currentBuildUnit;
					uint32 hpGain;

					if (builtUnit != nullptr) {

						hpGain = getHPGainForRepair(builtUnit);

						buildingAddon(builtUnit, hpGain, FALSE);

						if (builtUnit->status & UnitStatus::Completed) {

							addHangarUnit(unit, builtUnit);

							unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
							unit->buildQueueSlot++;

							if (unit->buildQueueSlot >= 5)
								unit->buildQueueSlot = 0;

							scbw::refreshConsole();

							unit->currentBuildUnit = nullptr;
							unit->secondaryOrderState = 0;

						}

					} else {

						scbw::refreshConsole();

						unit->currentBuildUnit = nullptr;
						unit->secondaryOrderState = 0;

					}

				} else
					if (unit->secondaryOrderState == 3) {

						if (unit->id == UnitId::ProtossAccantor ||
							unit->id == UnitId::Hero_Warbringer
							)
							unit->secondaryOrderState = 4;
						else {

							CUnit* inHangarChild = unit->carrier.inHangarChild;

							if (inHangarChild != nullptr) {

								int32 hpInterceptor = units_dat::MaxHitPoints[UnitId::ProtossSteward];

								while (inHangarChild != nullptr && inHangarChild->hitPoints >= hpInterceptor)
									inHangarChild = inHangarChild->interceptor.hangar_link.next;

								if (inHangarChild != nullptr)
									inHangarChild->setHp(inHangarChild->hitPoints + 128);

							}

						}

					}

		}

	}

	//generic unit training function
	void function_00468420(CUnit* unit) {

		if (
			!(unit->status & UnitStatus::DoodadStatesThing) &&
			unit->lockdownTimer == 0 &&
			unit->stasisTimer == 0 &&
			unit->maelstromTimer == 0 &&
			(!units_dat::GroupFlags[unit->id].isZerg ||
				unit->id == UnitId::ZergInfestedCommandCenter
				)
			) {

			bool jump_to_685D8 = false;

			if (unit->secondaryOrderState <= 1) {

				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::None) {
					unit->setSecondaryOrder(OrderId::Nothing2);
					unit->sprite->playIscriptAnim(IscriptAnimation::WorkingToIdle, true);
				} else {

					CUnit* builtUnit;

					if (unit->secondaryOrderState == 0)
						builtUnit = attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 1);
					else
						builtUnit = attemptTrainHatchUnit(unit, unit->buildQueue[unit->buildQueueSlot], 0);

					unit->currentBuildUnit = builtUnit;

					if (builtUnit == nullptr)
						unit->secondaryOrderState = 1;
					else {

						CImage* current_image = unit->sprite->images.head;

						unit->secondaryOrderState = 2;

						while (current_image != nullptr) {
							current_image->playIscriptAnim(IscriptAnimation::IsWorking);
							current_image = current_image->link.next;
						}

					}

				}

			} else
				if (unit->secondaryOrderState == 2) {

					CUnit* builtUnit = unit->currentBuildUnit;

					if (builtUnit != nullptr) {
						uint32 hpGain = getHPGainForRepair(builtUnit);
						bool extraBuild = false;
						if (scbw::get_generic_value(builtUnit, ValueId::Crucible) == 0) {
							auto r = 256;
							auto n = unitRangeCount(nullptr, CFlags::NoInvincible | CFlags::Ally | CFlags::Powered | CFlags::Completed,
								{UnitId::ProtossCrucible});
							scbw::UnitFinder unitsInSplash(builtUnit->position.x - r, builtUnit->position.y - r,
								builtUnit->position.x + r, builtUnit->position.y + r);
							for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
								auto unit = unitsInSplash.getUnit(i);
								if (n(unit)) {
									scbw::set_generic_value(builtUnit, ValueId::Crucible, 5);//6 is ~100/15
									buildingAddon(builtUnit, hpGain, TRUE);
									if (builtUnit->status & UnitStatus::Completed) {
										extraBuild = true;
									}
									break;
								}
							}
						} else {
							scbw::add_generic_value(builtUnit, ValueId::Crucible, -1);
						}
						if (extraBuild || buildingAddon(builtUnit, hpGain, TRUE)) {

							if (builtUnit->status & UnitStatus::Completed) {

								AI_TrainingUnit(unit, builtUnit);

								if (builtUnit->id == UnitId::TerranNuclearMissile)
									hideAndDisableUnit(builtUnit);
								else
									orderNewUnitToRally(builtUnit, unit);



								function_00432430(unit, unit->buildQueueSlot);

								unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
								unit->buildQueueSlot = (unit->buildQueueSlot + 1) % 5;

								jump_to_685D8 = true;

							}

						} else { //unit was canceled due to lack of space around the building

							uint32 queueSlot = unit->buildQueueSlot % 5;

							if (unit->buildQueue[queueSlot] == UnitId::None)
								jump_to_685D8 = true;
							else {

								uint8 counter = 4;

								if (unit->currentBuildUnit != nullptr)
									AI_CancelStructure(unit->currentBuildUnit);
								else {

									if (!units_dat::BaseProperty[unit->buildQueue[unit->buildQueueSlot % 5]] & UnitProperty::Building)
										refundUnitTrainCost(unit->buildQueue[unit->buildQueueSlot % 5], unit->playerId);

								}

								do {

									counter--;
									unit->buildQueue[unit->buildQueueSlot] = unit->buildQueue[(unit->buildQueueSlot + 1) % 5];

									if (unit->pAI != nullptr && *(uint8*)((uint32)unit->pAI + 8) == 3) {
										*(uint8*)((uint32)unit->pAI + unit->buildQueueSlot + 9) = *(uint8*)((uint32)unit->pAI + (unit->buildQueueSlot + 1) % 5 + 9);
										*(uint32*)((uint32)unit->pAI + unit->buildQueueSlot * 4 + 0x18) = *(uint32*)((uint32)unit->pAI + ((unit->buildQueueSlot + 1) % 5) * 4 + 0x18);
									}


								} while (counter != 0);

								unit->buildQueue[(unit->buildQueueSlot + 1) % 5] = UnitId::None;

								jump_to_685D8 = true;

							}

						}

					} else
						jump_to_685D8 = true;

					if (jump_to_685D8) {
						if (builtUnit != nullptr && builtUnit->id == UnitId::ProtossSolarion) {
							int interceptors = 12;

							uint8 buf[0xc] = {0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,    0x0,0x0,0x0,0x0};//12
							buf[0] = 0x99;//new general command
							buf[1] = NewGameCommand::CarrierPrebuild;
							buf[2] = interceptors;
							buf[3] = *ACTIVE_PLAYER_ID;
							memcpy(&buf[4], &builtUnit, 4);

							scbw::SendCommand(buf, 0xc);
						}

						scbw::refreshConsole();
						unit->currentBuildUnit = nullptr;
						unit->secondaryOrderState = 0;
					}

				}


		}

	}

	char getMaxHangerSpace(CUnit* a1) { // 0x004653D0
		uint16 id; // ax
		int32 hangarSpace; // eax

		id = a1->id;
		if (id != UnitId::ProtossSolarion) {
			if (id != UnitId::ProtossAccantor) {
				hangarSpace = 0;
			} else {
				hangarSpace = 5;
			}
		} else {
			hangarSpace = 12;
		}
		return hangarSpace;
	}

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	//Identical to getHPGainForRepair @ 0x00402C40
	uint32 getHPGainForRepair(CUnit* unit) {

		if (*CHEAT_STATE & CheatFlags::OperationCwal)
			return unit->buildRepairHpGain * 16;
		else
			return unit->buildRepairHpGain;

	}

	const uint32 Func_refundUnitTrainCost = 0x0042CEC0;
	void refundUnitTrainCost(uint32 unitId, uint8 playerId) {

		__asm {
			PUSHAD
			MOV ECX, unitId
			MOV AL, playerId
			CALL Func_refundUnitTrainCost
			POPAD
		}

	}

	const uint32 Func_Sub432430 = 0x00432430;
	void function_00432430(CUnit* unit, uint32 buildQueueSlot) {

		__asm {
			PUSHAD
			MOV EAX, unit
			MOV ECX, buildQueueSlot
			CALL Func_Sub432430
			POPAD
		}

	}

	const uint32 Func_addHangerUnit = 0x00466300;
	void addHangarUnit(CUnit* main_unit, CUnit* added_unit) {

		__asm {
			PUSHAD
			MOV EDI, main_unit
			MOV ESI, added_unit
			CALL Func_addHangerUnit
			POPAD
		}

	}

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

	const uint32 Func_buildingAddon = 0x004679A0;
	//if canBeAborted is TRUE, the unit can be aborted because there's
	//no room around the constructing building
	bool32 buildingAddon(CUnit* unit, uint32 hpGain, bool32 canBeAborted) {

		static bool32 bResult;

		__asm {
			PUSHAD
			MOV EDX, hpGain
			MOV EAX, unit
			PUSH canBeAborted
			CALL Func_buildingAddon
			MOV bResult, EAX
			POPAD
		}

		return bResult;

	}

	const uint32 Func_attemptTrainHatchUnit = 0x00468200;
	CUnit* attemptTrainHatchUnit(CUnit* trainer, uint32 builtUnitID, uint32 flag) {

		static CUnit* builtUnit;

		__asm {
			PUSHAD
			MOV ESI, trainer
			MOV EDI, builtUnitID
			PUSH flag
			CALL Func_attemptTrainHatchUnit
			MOV builtUnit, EAX
			POPAD
		}

		return builtUnit;

	}

	const uint32 Func_AI_CancelStructure = 0x00468280;
	void AI_CancelStructure(CUnit* unit) {

		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_AI_CancelStructure
			POPAD
		}

	}

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

	const uint32 Func_unitDeathSomething_0 = 0x004E6340;
	void hideAndDisableUnit(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}

	}

} //Unnamed namespace

//End of helper functions
