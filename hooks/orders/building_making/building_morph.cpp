#include <SCBW/api.h>
//helper functions def
//from unh
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\building_making\building_helper.h"
#include "logger.h"

namespace {

	void refundBuildingCost(uint16 unitId, uint8 playerId);					//2CE70
	void function_00433FE0(CUnit* unit);								//33FE0
	bool isValidMorph(uint16 buildingId);									//5CC60
	void updateNewUnitVision(uint32 id, int x, int y);				//5CE90
	void zergPlaceBuildingCntd(CUnit* unit);							//5D410
	void function_0047D770(CUnit* unit);								//7D770
	void function_0047DE40(CSprite* sprite, uint32 unitId, int x, int y);	//7DE40
	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId);	//88210
	void incrementUnitScoresEx(CUnit* building, int32 unk1, int32 unk2);	//88D50
	void playBuildingCompleteSound(CUnit* building);					//8F310
	void refreshSpriteData(CSprite* sprite);							//983A0
	void displayLastNetErrForPlayer(uint32 playerId);						//9E530
	void updateUnitStrength(CUnit* unit);								//9FA40
	void replaceUnitWithType(CUnit* unit, uint16 newUnitId);				//9FED0
	void function_004A01F0(CUnit* unit);								//A01F0
	CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId);			//A09D0

} //unnamed namespace

const uint32 Func_Sub4D8500 = 0x004D8500;
void function_004D8500_copy(CImage* image) {

	__asm {
		PUSHAD
		MOV EAX, image
		CALL Func_Sub4D8500
		POPAD
	}

}

;


const uint32 Func_iscript_OpcodeCases = 0x004D74C0;
void iscript_OpcodeCases_copy(CImage* image, uint32 offset_, uint32 unk1, uint32 unk2) {

	__asm {
		PUSHAD
		PUSH unk1
		PUSH unk2
		PUSH offset_
		MOV ECX, image
		CALL Func_iscript_OpcodeCases
		POPAD
	}

}

;
#include "SCBW/api.h"
namespace hooks {

	bool isMorphing(CUnit* building) {

		bool bIsMorphing;

		if (!(building->status & UnitStatus::Completed))
			//was hardcoded instead of using isValidMorph in original code
			bIsMorphing = isValidMorph(building->buildQueue[building->buildQueueSlot]);
		else
			bIsMorphing = false;

		return bIsMorphing;
	};

	const uint32 Func_Sub499BB0 = 0x00499BB0;
	void replaceSpriteImages_copy(CSprite* sprite, uint32 imageId, uint32 imageDirection) {
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	};

	void orders_ZergBuildSelf(CUnit* building) {

		const int OPCWAL_PROGRESS_INCREASE = 16;

		uint16 buildingId;
		bool jump_to_5D56C = false;
		bool jump_to_5D718 = false;
		bool jump_to_5D720 = false;

		buildingId = building->buildQueue[building->buildQueueSlot];

		if (   !scbw::isZergBuildingMorphResult(buildingId))
			buildingId = building->id;

		if (building->mainOrderState == 0) {
			if (building->remainingBuildTime < (units_dat::TimeCost[buildingId] * 3 / 4))
				building->mainOrderState = 1;

			jump_to_5D56C = true;

		}
		else
			if (building->mainOrderState > 6)
				jump_to_5D56C = true;

		if (jump_to_5D56C) { //5D56C

			jump_to_5D56C = false;

			if (building->remainingBuildTime == 0)
				jump_to_5D720 = true;
			else
				if (!scbw::isCheatEnabled(CheatFlags::OperationCwal))
					jump_to_5D718 = true;
				else {

					if (building->remainingBuildTime >= OPCWAL_PROGRESS_INCREASE)
						building->remainingBuildTime -= OPCWAL_PROGRESS_INCREASE;
					else
						building->remainingBuildTime = 0;

					jump_to_5D720 = true;

				}
		} else if (building->mainOrderState == 1) { //5D5AE
			if (building->id == UnitId::ZergExtractor) {

				CImage* overlay = building->sprite->getOverlay(ImageId::VespeneGeyser2);

				if (overlay != nullptr)
					overlay->free();

			}
				
			building->sprite->playIscriptAnim(IscriptAnimation::SpecialState1, true);
			building->mainOrderState = 2;
			//scbw::printText("Set order state 2");
			jump_to_5D56C = true;

		}  else  if (building->mainOrderState == 2) { //5D5E2
				if (building->remainingBuildTime < units_dat::TimeCost[buildingId] / 2) {
					//scbw::printText("Set order state 3");
					building->mainOrderState = 3;
				}
				jump_to_5D56C = true;

		} else if (building->mainOrderState == 3) { //5D603
				building->sprite->playIscriptAnim(IscriptAnimation::SpecialState2, true);
				building->mainOrderState = 4;
				//scbw::printText("Set order state 4");
				jump_to_5D56C = true;

		} else if (building->mainOrderState == 4) { //5D618

			if (building->remainingBuildTime == 0) {
				//scbw::printText("Set order state 5");								

				building->mainOrderState = 5;
			}
			jump_to_5D56C = true;

		} else if (building->mainOrderState == 5) { //5D62F
			
			building->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
			building->mainOrderState = 6;
			playBuildingCompleteSound(building);
			jump_to_5D56C = true;

			/*bool sEnd = false;
			//if (building->orderSignal & 0x1) {
			building->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
			sEnd = true;

			replaceSpriteImages_copy(building->sprite, scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId), 0);
			function_004D8500_copy(building->sprite->mainGraphic);
			//}
			if (sEnd) {
				//scbw::printText("Set order state 6");
				building->orderSignal |= 4;
				building->mainOrderState = 6;
				playBuildingCompleteSound(building);
			}
			jump_to_5D56C = true;*/
		} else if (building->mainOrderState == 6) { //5D64Bs

			bool ordCondition = false;
			if (building->orderSignal & 0x4) {
				ordCondition = true;
			}
			if (building->orderSignal & 4) {
				//scbw::printText("Get order signal");
				if (building->orderSignal & 4) {
					//scbw::printText("sigorder 4 found");
				}
				if (building->orderSignal & 1) {
					//scbw::printText("sigorder 1 found");
				}
				int32 hpAfterMorph;
				CImage* current_image;
				building->orderSignal -= 4;
				if (isValidMorph(buildingId)) {
					//save hp before some functions modify them
					hpAfterMorph = building->hitPoints;
					uint32 save_shields = building->shields;
					//update score
					building->status |= UnitStatus::Completed;
					incrementUnitScoresEx(building, -1, 0);

					//do the building change
					building->status -= UnitStatus::Completed;
					replaceUnitWithType(building, buildingId);
					//newHp = newHpMax - oldHpMax + oldHp
					hpAfterMorph =
						units_dat::MaxHitPoints[buildingId] -
						units_dat::MaxHitPoints[building->previousUnitType] +
						hpAfterMorph;
					if (hpAfterMorph < 256)
						hpAfterMorph = 256;

					building->setHp(hpAfterMorph);
					building->remainingBuildTime = 0;

				}
				//5d6cd
				//update various stuff (set hp, set shield...) not finished on Morph
				function_004A01F0(building);
				updateUnitStrength(building);
				//relate to building placement / taking room around?
				function_0047D770(building);



				current_image = building->sprite->images.head;
				while (current_image != nullptr) {
					current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
					current_image = current_image->link.next;
				}
				//scbw::printText("Creep!");
				updateNewUnitVision(buildingId, building->sprite->position.x, building->sprite->position.y);

			}
			else {
				//scbw::printText("Wait for order signal");
			}


		}

		if (jump_to_5D56C) { //5D56C

			jump_to_5D56C = false;

			if (building->remainingBuildTime == 0)
				jump_to_5D720 = true;
			else
				if (!scbw::isCheatEnabled(CheatFlags::OperationCwal))
					jump_to_5D718 = true;
				else {

					if (building->remainingBuildTime >= OPCWAL_PROGRESS_INCREASE)
						building->remainingBuildTime -= OPCWAL_PROGRESS_INCREASE;
					else
						building->remainingBuildTime = 0;

					jump_to_5D720 = true;

				}

		}

		if (jump_to_5D718) {
			jump_to_5D718 = false;
			building->remainingBuildTime--;
			jump_to_5D720 = true;
		}

		if (jump_to_5D720) {

			jump_to_5D720 = false;

			if (!scbw::isZergBuildingMorphResult(buildingId))
			{

				int32 hpGain = building->buildRepairHpGain;

				if (scbw::isCheatEnabled(CheatFlags::OperationCwal))
					hpGain *= OPCWAL_PROGRESS_INCREASE;

				building->setHp(building->hitPoints + hpGain);

			}

		}

	} //void orders_ZergBuildSelf(CUnit* building)

	;



	const uint32 Func_CreateUnit = 0x004A09D0;
	CUnit* CreateUnit_copy(uint32 unitId, int x, int y, uint32 playerId) {

		static CUnit* unit_created;

		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV ECX, unitId
			MOV EAX, x
			CALL Func_CreateUnit
			MOV unit_created, EAX
			POPAD
		}

		return unit_created;

	}

	;

	const uint32 Func_Sub4A01F0 = 0x004A01F0;
	void function_004A01F0_copy(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const uint32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength_copy(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}

	}

	;


	void ZergPlaceBuilding(CUnit* unit) {

		bool bStopThere = false;
		//scbw::printText("ZergPlace");
		if (!(unit->status & UnitStatus::Completed)) {

			uint16 builtUnitId = unit->buildQueue[unit->buildQueueSlot];

			//original code was hardcoded instead of using isValidMorph
			if (isValidMorph(builtUnitId)) {
				if (unit->getRace() == RaceId::Zerg) {
					zergPlaceBuildingCntd(unit);
				}
				else {
					unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
					resources->minerals[unit->playerId] += (units_dat::MineralCost[builtUnitId] * 3) / 4;
					resources->gas[unit->playerId] += (units_dat::GasCost[builtUnitId] * 3) / 4;
					unit->status |= UnitStatus::Completed;
					incrementUnitScoresEx(unit, -1, 0);
					unit->status -= UnitStatus::Completed;
					uint32 hp = unit->hitPoints;
					uint32 shield = unit->shields;
					function_004A01F0(unit);
					updateUnitStrength(unit);
					unit->hitPoints = hp;
					unit->shields = shield;
					CImage* anchor = unit->getOverlay(ImageId::WarpAnchor);
					if (anchor != nullptr) {
						//scbw::printText("Anchor detected");
						anchor->playIscriptAnim(IscriptAnimation::Death);
					}
					else {
						//scbw::printText("No anchor");
						unit->sprite->createTopOverlay(970);
						anchor = unit->getOverlay(970);
						if (anchor != nullptr) {
							//scbw::printText("Anchor2");
							anchor->playIscriptAnim(IscriptAnimation::Death);
						}
						else {
							//scbw::printText("Anchor Error");
						}

					}
					//scbw::refreshConsole();
				}
				bStopThere = true;
				//scbw::printText("ZergPlace - Stop");
			}

		}

		if (!bStopThere) {

			static uint32* const u32_00581E44 = (uint32*)0x00581E44;
			static uint32* const u32_00581ED4 = (uint32*)0x00581ED4;
			static uint16* const u16_0066345A = (uint16*)0x0066345A;

			refundBuildingCost(unit->id, unit->playerId);

			//possibly related to scores?
			u32_00581ED4[unit->playerId] = u32_00581ED4[unit->playerId] - *u16_0066345A;
			u32_00581E44[unit->playerId] = u32_00581E44[unit->playerId] - 1;

			if (unit->id == UnitId::ZergExtractor) {

				CUnit* drone = createUnit(UnitId::ZergDroleth, unit->getX(), unit->getY(), unit->playerId);

				if (drone == nullptr) {
					displayLastNetErrForPlayer(unit->playerId);
					unit->remove();
					bStopThere = true;
				}
				else {	//5DB1E

					//update various stuff (set hp, set shield...) not finished after createUnit
					function_004A01F0(drone);
					updateUnitStrength(drone);

					drone->setHp(unit->previousHp * 256);

					unit->remove();

					bStopThere = true;

				}

			}

			if (!bStopThere) {	//5DB45

				CThingy* thingy = createThingy(SpriteId::Zerg_Building_Spawn_Small, unit->getX(), unit->getY(), 0);

				if (thingy != nullptr) {
					thingy->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
					scbw::setThingyVisibilityFlags(thingy);
				}

				scbw::playSound(SoundId::Misc_ZBldgPlc_wav_1, unit);
				replaceUnitWithType(unit, UnitId::ZergDroleth);

				//probably AI related
				function_00433FE0(unit);

				//update various stuff (set hp, set shield...) not finished after unit change
				function_004A01F0(unit);
				updateUnitStrength(unit);

				if (
					unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW &&
					unit->sprite->images.tail->verticalOffset != 7
					)
				{
					unit->sprite->images.tail->flags |= CImage_Flags::Redraw;
					unit->sprite->images.tail->verticalOffset = 7;
				}

				refreshSpriteData(unit->sprite);

				unit->orderComputerCL(OrderId::ResetCollision1);

				unit->order(
					units_dat::ReturnToIdleOrder[unit->id],
					0,
					0,
					nullptr,
					UnitId::None,
					false
				);

				unit->setHp(unit->previousHp * 256);

				//???
				function_0047DE40(
					unit->sprite,
					unit->id,
					unit->getX(),
					unit->getY()
				);


			}


		}

	}

	;

} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//
#include "SCBW/api.h"
namespace {

	const uint32 Func_refundBuildingCost = 0x0042CE70;
	void refundBuildingCost(uint16 unitId, uint8 playerId) {

		__asm {
			PUSHAD
			MOV AX, unitId
			MOV CL, playerId
			CALL Func_refundBuildingCost
			POPAD
		}

	}

	;

	const uint32 Func_Sub433FE0 = 0x00433FE0;
	void function_00433FE0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub433FE0
			POPAD
		}

	}

	;

	//Identical to isValidMorph @ 0x0045CC60;
	bool isValidMorph(uint16 buildingId) {

		bool result;

		if (scbw::isZergBuildingMorphResult(buildingId))
			result = true;
		else
			result = false;

		return result;

	}

	;

	const uint32 Func_updateNewUnitVision = 0x0045CE90;
	void updateNewUnitVision(uint32 id, int x, int y) {
		__asm {
			PUSHAD
			MOV ECX, id
			PUSH y
			PUSH x
			CALL Func_updateNewUnitVision
			POPAD
		}

	}

	;

	const uint32 Func_ZergPlaceBuildingCntd = 0x0045D410;
	void zergPlaceBuildingCntd(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ZergPlaceBuildingCntd
			POPAD
		}

	}

	;

	const uint32 Func_Sub47D770 = 0x0047D770;
	void function_0047D770(CUnit* unit) {

		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub47D770
			POPAD
		}

	}

	;

	const uint32 Func_Sub47DE40 = 0x0047DE40;
	void function_0047DE40(CSprite* sprite, uint32 unitId, int x, int y) {

		__asm {
			PUSHAD
			MOV ECX, y
			MOV EDX, x
			MOV ESI, sprite
			PUSH unitId
			CALL Func_Sub47DE40
			POPAD
		}

	}

	;

	//original referenced name was replaceSprite (but this one is probably
	//more accurate since it does create something rather than replacing)
	const uint32 Func_CreateThingy = 0x00488210;
	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId) {

		static CThingy* thingy;
		int32 x_ = x;

		__asm {
			PUSHAD
			PUSH playerId
			MOVSX EDI, y
			PUSH x_
			PUSH spriteId
			CALL Func_CreateThingy
			MOV thingy, EAX
			POPAD
		}

		return thingy;

	}

	;

	const uint32 Func_incrementUnitScoresEx = 0x00488D50;
	//unk1 is ECX, unk2 is pushed value
	void incrementUnitScoresEx(CUnit* building, int32 unk1, int32 unk2) {

		__asm {
			PUSHAD
			MOV EDI, building
			MOV ECX, unk1
			PUSH unk2
			CALL Func_incrementUnitScoresEx
			POPAD
		}

	}

	;

	const uint32 Func_PlayBuildingCompleteSound = 0x0048F310;
	void playBuildingCompleteSound(CUnit* building) {

		__asm {
			PUSHAD
			MOV EAX, building
			CALL Func_PlayBuildingCompleteSound
			POPAD
		}

	}

	;

	const uint32 Func_refreshSpriteData = 0x004983A0;
	void refreshSpriteData(CSprite* sprite) {

		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refreshSpriteData
			POPAD
		}

	}

	;

	const uint32 Func_displayLastNetErrForPlayer = 0x0049E530;
	void displayLastNetErrForPlayer(uint32 playerId) {
		__asm {
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrForPlayer
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

	const uint32 Func_CreateUnit = 0x004A09D0;
	CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId) {

		static CUnit* unit_created;

		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV ECX, unitId
			MOV EAX, x
			CALL Func_CreateUnit
			MOV unit_created, EAX
			POPAD
		}

		return unit_created;

	}

	;

} //Unnamed namespace

//End of helper functions

//from unhooked
//V241 for VS2008

//-------- Helper function declarations. Do NOT modify! ---------//

namespace {
	int BuildingHelper::getNumberOfUnitType(const CUnit* unit, uint16 unitId, bool ignoreIncomplete);
} //unnamed namespace


  //-------- Actual hook functions --------//

namespace hooks {

	//Checks if @p unitId is a building that can be morphed from another building.
	//Note: This hook affects the behavior of CUnit::isRemorphingBuilding().
	bool isMorphedBuildingHook(uint16 unitId) {
		//Default StarCraft behavior

		if (scbw::isZergBuildingMorphResult(unitId))
			return true;

		return false;
	}

	//Determines the number of units which have @p unitId and are being built by the
	//@p unit. This hook is used by StarCraft's AI for counting units and buildings.
	//For example, Greater Spires are counted as Spires, so that the AI would not
	//try to rebuild another Spire after morphing one into a Greater Spire.
	int getMorphBuildingTypeCountHook(const CUnit* unit, uint16 unitId, bool ignoreIncomplete) {
		//Default StarCraft behavior

		int unitCount = BuildingHelper::getNumberOfUnitType(unit, unitId, ignoreIncomplete);

		switch (unitId) {
		case UnitId::ZergHatchery:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergLair, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergHive, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergSire, ignoreIncomplete);
			break;

		case UnitId::ZergLair:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergHive, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergSire, ignoreIncomplete);
			break;

		case UnitId::ZergHive:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergSire, ignoreIncomplete);
			break;

		case UnitId::ZergCreepColony:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergSunkenColony, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergSporeColony, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergLarvalColony, ignoreIncomplete);
			break;

		case UnitId::ZergIrolIris:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergOthstolOviform, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergAlazilArbor, ignoreIncomplete);
			break;

		case UnitId::ZergOthstolOviform:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergAlazilArbor, ignoreIncomplete);
			break;

		case UnitId::ZergSpawningPool:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergVorvalPond, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergNathrokLake, ignoreIncomplete);
			break;

		case UnitId::ZergHydralDen:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergBactalDen, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergLakizDen, ignoreIncomplete);
			break;

		case UnitId::ZergMutalSpire:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergGorgrokApiary, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergVithrilHaunt, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergGeszithRoost, ignoreIncomplete);
			break;

		case UnitId::ZergGorgrokApiary:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergGeszithRoost, ignoreIncomplete);
			break;

		case UnitId::ZergQuazilQuay:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergKalkathBloom, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergAlmakAntre, ignoreIncomplete);
			break;

		case UnitId::ZergMatralNest:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergAkisNest, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergKonvilNest, ignoreIncomplete);
			break;

		case UnitId::ZergUltrakCavern:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergAlkajChasm, ignoreIncomplete);
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergKeskathGrotto, ignoreIncomplete);
			break;

		case UnitId::ZergZoryusShroud:
			unitCount += BuildingHelper::getNumberOfUnitType(unit, UnitId::ZergIsthrathMound, ignoreIncomplete);
			break;
		}
		return unitCount;
	}

	void SetSpecialState1(CSprite* sprite) {
		CImage* current_image = sprite->images.head;
		while (current_image != nullptr) {
			if (current_image->id != ImageId::StargateOverlay) {
				current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
			}
			current_image = current_image->link.next;
		}
	}
} //hooks