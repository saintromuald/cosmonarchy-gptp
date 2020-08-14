#include <SCBW/api.h>
//helper functions def
//from unh
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "logger.h"

namespace {

	void refundBuildingCost(u16 unitId, u8 playerId);					//2CE70
	void function_00433FE0(CUnit* unit);								//33FE0
	bool isValidMorph(u16 buildingId);									//5CC60
	void updateNewUnitVision(u32 id, int x, int y);				//5CE90
	void zergPlaceBuildingCntd(CUnit* unit);							//5D410
	void function_0047D770(CUnit* unit);								//7D770
	void function_0047DE40(CSprite* sprite, u32 unitId, int x, int y);	//7DE40
	CThingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId);	//88210
	void incrementUnitScoresEx(CUnit* building, s32 unk1, s32 unk2);	//88D50
	void playBuildingCompleteSound(CUnit* building);					//8F310
	void refreshSpriteData(CSprite* sprite);							//983A0
	void displayLastNetErrForPlayer(u32 playerId);						//9E530
	void updateUnitStrength(CUnit* unit);								//9FA40
	void replaceUnitWithType(CUnit* unit, u16 newUnitId);				//9FED0
	void function_004A01F0(CUnit* unit);								//A01F0
	CUnit* createUnit(u32 unitId, int x, int y, u32 playerId);			//A09D0

} //unnamed namespace

const u32 Func_Sub4D8500 = 0x004D8500;
void function_004D8500_copy(CImage* image) {

	__asm {
		PUSHAD
		MOV EAX, image
		CALL Func_Sub4D8500
		POPAD
	}

}

;


const u32 Func_iscript_OpcodeCases = 0x004D74C0;
void iscript_OpcodeCases_copy(CImage* image, u32 offset_, u32 unk1, u32 unk2) {

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

	const u32 Func_Sub499BB0 = 0x00499BB0;
	void replaceSpriteImages_copy(CSprite* sprite, u32 imageId, u32 imageDirection) {
		__asm {
			PUSHAD
			PUSH imageDirection
			PUSH imageId
			MOV EAX, sprite
			CALL Func_Sub499BB0
			POPAD
		}
	};

	bool isProtossMorph(u16 unitId) {
		if (   unitId == UnitId::ProtossPrincipality		// morphs begin
	    	|| unitId == UnitId::ProtossVicarage
	    #ifdef DELIVERANCE
	    	|| unitId == UnitId::ProtossPrelacy
	    #endif
	    	|| unitId == UnitId::ProtossRoboticsAuthority
	    	|| unitId == UnitId::ProtossArgosy
	    	|| unitId == UnitId::ProtossStarfield			// morphs end
	    	|| unitId == UnitId::ProtossGateway				// original structures begin
	    	|| unitId == UnitId::ProtossStargate
	    	|| unitId == UnitId::ProtossRoboticsFacility	// original structures end
			) {
			return true;
		}
		return false;
	}

	void orders_ZergBuildSelf(CUnit* building) {

		const int OPCWAL_PROGRESS_INCREASE = 16;

		u16 buildingId;
		bool jump_to_5D56C = false;
		bool jump_to_5D718 = false;
		bool jump_to_5D720 = false;

		buildingId = building->buildQueue[building->buildQueueSlot];

		if (   buildingId != UnitId::ZergHive
			&& buildingId != UnitId::ZergLair
			&& buildingId != UnitId::ZergGuardianRoost
			&& buildingId != UnitId::ZergDevourerHaunt
			&& buildingId != UnitId::ZergGorgolethApiary
			&& buildingId != UnitId::ZergSporeColony
			&& buildingId != UnitId::ZergSunkenColony
			&& buildingId != UnitId::ZergLurkerDen
			&& buildingId != UnitId::ZergBasiliskDen
			&& buildingId != UnitId::ZergMutationPit
			&& buildingId != UnitId::ZergVorvalPond
			&& !isProtossMorph(buildingId)
			)
			buildingId = building->id;

		if (building->mainOrderState == 0) {
			//scbw::printText("State 0...");
			if (building->remainingBuildTime < (units_dat::TimeCost[buildingId] * 3 / 4))
				building->mainOrderState = 1;
			if (isProtossMorph(buildingId)) {
				if (!building->getOverlay(ImageId::WarpAnchor)) {
					building->sprite->createTopOverlay(ImageId::WarpAnchor);
				}
			}

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
		}
		else
			if (building->mainOrderState == 1) { //5D5AE

				if (building->id == UnitId::ZergExtractor) {

					CImage* overlay = building->sprite->getOverlay(ImageId::VespeneGeyser2);

					if (overlay != NULL)
						overlay->free();

				}

				//5D5D0
				if (!(isProtossMorph(buildingId))) {

					building->sprite->playIscriptAnim(IscriptAnimation::SpecialState1, true);
				}

				building->mainOrderState = 2;
				//scbw::printText("Set order state 2");
				jump_to_5D56C = true;

			}
			else
				if (building->mainOrderState == 2) { //5D5E2

					if (building->remainingBuildTime < units_dat::TimeCost[buildingId] / 2) {
						//scbw::printText("Set order state 3");
						building->mainOrderState = 3;
					}
					jump_to_5D56C = true;

				}
				else
					if (building->mainOrderState == 3) { //5D603


						if (!(isProtossMorph(buildingId))) {
							building->sprite->playIscriptAnim(IscriptAnimation::SpecialState2, true);
						}
						building->mainOrderState = 4;
						//scbw::printText("Set order state 4");
						jump_to_5D56C = true;

					}
					else
						if (building->mainOrderState == 4) { //5D618

							if (building->remainingBuildTime == 0) {
								//scbw::printText("Set order state 5");

								if (isProtossMorph(buildingId)) {
									CImage* current_image = building->sprite->images.head;
									while (current_image != NULL) {
										if (current_image->flags & CImage_Flags::Flag04) {
											const u32* u32_006D1200 = (u32*)0x006D1200;
											current_image->animation = IscriptAnimation::SpecialState1;
											current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x22);
											current_image->wait = 0;
											current_image->unknown14 = 0;
											iscript_OpcodeCases_copy(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
										}
										current_image = current_image->link.next;

									}
								}

								building->mainOrderState = 5;
							}
							jump_to_5D56C = true;

						}
						else
							if (building->mainOrderState == 5) { //5D62F
								bool sEnd = false;
								if (!(isProtossMorph(buildingId))) {

									building->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
									sEnd = true;
								}
								else {
									if (building->orderSignal & 0x1) {
										//scbw::printText("Special state order signal 0x1");
										building->orderSignal -= 1;
										if (building->getOverlay(ImageId::WarpAnchor)) {
											building->removeOverlay(ImageId::WarpAnchor);
										}

										replaceSpriteImages_copy(
											building->sprite,
//											sprites_dat::ImageId[building->sprite->spriteId],
											scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId),
											0
										);
										char buf[32];
										sprintf(buf, "Sprite id is %d", building->sprite->spriteId);
										//scbw::printText(buf);
										function_004D8500_copy(building->sprite->mainGraphic);
										sEnd = true;
									}
								}
								if (sEnd) {
									building->mainOrderState = 6;
									playBuildingCompleteSound(building);
								}
								jump_to_5D56C = true;
							}
							else
								if (building->mainOrderState == 6) { //5D64Bs

									bool ordCondition = false;
									if (isProtossMorph(buildingId)) {
										if (building->orderSignal & 0x1) {
											ordCondition = true;
										}
									}
									else {
										if (building->orderSignal & 0x4) {
											ordCondition = true;
										}
									}
									if (((building->orderSignal & 4) && building->getRace() == RaceId::Zerg) || (building->orderSignal & 0x1)) {
										//scbw::printText("Get order signal");
										if (building->orderSignal & 4) {
											//scbw::printText("sigorder 4 found");
										}
										if (building->orderSignal & 1) {
											//scbw::printText("sigorder 1 found");
										}
										s32 hpAfterMorph;
										CImage* current_image;
										building->orderSignal -= 4;
										if (isValidMorph(buildingId)) {
											//save hp before some functions modify them
											hpAfterMorph = building->hitPoints;
											u32 save_shields = building->shields;
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
											if (isProtossMorph(buildingId)) {
												building->shields = save_shields;
											}
											if (hpAfterMorph < 256)
												hpAfterMorph = 256;

											building->setHp(hpAfterMorph);
											building->remainingBuildTime = 0;

										}
										else {
											//scbw::printText("Invalid morph");
										}
										//5d6cd
										//update various stuff (set hp, set shield...) not finished on Morph
										function_004A01F0(building);
										updateUnitStrength(building);
										//relate to building placement / taking room around?
										function_0047D770(building);



										current_image = building->sprite->images.head;
										while (current_image != NULL) {
											if (building->getRace() == RaceId::Zerg) {
												current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
											}
											if (isProtossMorph(buildingId)) {
												if (current_image->flags & CImage_Flags::Flag04) {
													const u32* u32_006D1200 = (u32*)0x006D1200;
													current_image->animation = IscriptAnimation::WarpIn;
													current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
													current_image->wait = 0;
													current_image->unknown14 = 0;
													iscript_OpcodeCases_copy(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
													scbw::playSound(SoundId::Protoss_SHUTTLE_pshBld03_wav, building);
													//scbw::printText("Play Warp Flash (or at least attempt to)");
												}
											}
											current_image = current_image->link.next;
										}
										if (building->getRace() == RaceId::Zerg) {
											//scbw::printText("Creep!");
											updateNewUnitVision(buildingId, building->sprite->position.x, building->sprite->position.y);
										}
										else {
											//function_004D8500_copy(building->sprite->mainGraphic);
											//scbw::printText("Warp overlay 2");
											/*
											CImage* anchor = building->getOverlay(ImageId::WarpAnchor);
											if (anchor==NULL) {
												scbw::printText("End - No anchor");
												building->sprite->createTopOverlay(970);
												anchor = building->getOverlay(970);
												if(anchor!=NULL){
													scbw::printText("Anchor2");
													anchor->playIscriptAnim(IscriptAnimation::Death);
												}
												else {
													scbw::printText("Anchor Error");
												}

											}
											else {
												scbw::printText("Bug: Anchor still exists");
											}*/
										}

									}
									else {
										//scbw::printText("Wait for order signal");
									}
									if (isProtossMorph(buildingId)) {
										//scbw::printText("protoss morph!");
										if (!(building->_unknown_0x026 & 0x1)) {
											building->orderSignal |= 0x4;
											building->_unknown_0x026 |= 0x1;
										}
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

			if (
				   buildingId != UnitId::ZergHive
				&& buildingId != UnitId::ZergLair
				&& buildingId != UnitId::ZergGuardianRoost
				&& buildingId != UnitId::ZergDevourerHaunt
				&& buildingId != UnitId::ZergGorgolethApiary
				&& buildingId != UnitId::ZergSporeColony
				&& buildingId != UnitId::ZergSunkenColony
				&& buildingId != UnitId::ZergBasiliskDen
				&& buildingId != UnitId::ZergLurkerDen
				&& buildingId != UnitId::ZergMutationPit
				&& buildingId != UnitId::ZergVorvalPond
				&& !isProtossMorph(buildingId)
				)
			{

				s32 hpGain = building->buildRepairHpGain;

				if (scbw::isCheatEnabled(CheatFlags::OperationCwal))
					hpGain *= OPCWAL_PROGRESS_INCREASE;

				building->setHp(building->hitPoints + hpGain);

			}

		}

	} //void orders_ZergBuildSelf(CUnit* building)

	;



	const u32 Func_CreateUnit = 0x004A09D0;
	CUnit* CreateUnit_copy(u32 unitId, int x, int y, u32 playerId) {

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

	const u32 Func_Sub4A01F0 = 0x004A01F0;
	void function_004A01F0_copy(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const u32 Func_UpdateUnitStrength = 0x0049FA40;
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

			u16 builtUnitId = unit->buildQueue[unit->buildQueueSlot];

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
					u32 hp = unit->hitPoints;
					u32 shield = unit->shields;
					function_004A01F0(unit);
					updateUnitStrength(unit);
					unit->hitPoints = hp;
					unit->shields = shield;
					CImage* anchor = unit->getOverlay(ImageId::WarpAnchor);
					if (anchor != NULL) {
						//scbw::printText("Anchor detected");
						anchor->playIscriptAnim(IscriptAnimation::Death);
					}
					else {
						//scbw::printText("No anchor");
						unit->sprite->createTopOverlay(970);
						anchor = unit->getOverlay(970);
						if (anchor != NULL) {
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

			static u32* const u32_00581E44 = (u32*)0x00581E44;
			static u32* const u32_00581ED4 = (u32*)0x00581ED4;
			static u16* const u16_0066345A = (u16*)0x0066345A;

			refundBuildingCost(unit->id, unit->playerId);

			//possibly related to scores?
			u32_00581ED4[unit->playerId] = u32_00581ED4[unit->playerId] - *u16_0066345A;
			u32_00581E44[unit->playerId] = u32_00581E44[unit->playerId] - 1;

			if (unit->id == UnitId::ZergExtractor) {

				CUnit* drone = createUnit(UnitId::ZergDrone, unit->getX(), unit->getY(), unit->playerId);

				if (drone == NULL) {
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

				if (thingy != NULL) {
					thingy->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
					scbw::setThingyVisibilityFlags(thingy);
				}

				scbw::playSound(SoundId::Misc_ZBldgPlc_wav_1, unit);
				replaceUnitWithType(unit, UnitId::ZergDrone);

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
					NULL,
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

namespace {

	const u32 Func_refundBuildingCost = 0x0042CE70;
	void refundBuildingCost(u16 unitId, u8 playerId) {

		__asm {
			PUSHAD
			MOV AX, unitId
			MOV CL, playerId
			CALL Func_refundBuildingCost
			POPAD
		}

	}

	;

	const u32 Func_Sub433FE0 = 0x00433FE0;
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
	bool isValidMorph(u16 buildingId) {

		bool result;

		if (
			   buildingId == UnitId::ZergHive
			|| buildingId == UnitId::ZergLair
			|| buildingId == UnitId::ZergGuardianRoost
			|| buildingId == UnitId::ZergDevourerHaunt
			|| buildingId == UnitId::ZergGorgolethApiary
			|| buildingId == UnitId::ZergLurkerDen
			|| buildingId == UnitId::ZergBasiliskDen
			|| buildingId == UnitId::ZergSporeColony
			|| buildingId == UnitId::ZergSunkenColony
			|| buildingId == UnitId::ZergMutationPit
			|| buildingId == UnitId::ZergVorvalPond
			|| hooks::isProtossMorph(buildingId)

			)
			result = true;
		else
			result = false;

		return result;

	}

	;

	const u32 Func_updateNewUnitVision = 0x0045CE90;
	void updateNewUnitVision(u32 id, int x, int y) {
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

	const u32 Func_ZergPlaceBuildingCntd = 0x0045D410;
	void zergPlaceBuildingCntd(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ZergPlaceBuildingCntd
			POPAD
		}

	}

	;

	const u32 Func_Sub47D770 = 0x0047D770;
	void function_0047D770(CUnit* unit) {

		__asm {
			PUSHAD
			PUSH unit
			CALL Func_Sub47D770
			POPAD
		}

	}

	;

	const u32 Func_Sub47DE40 = 0x0047DE40;
	void function_0047DE40(CSprite* sprite, u32 unitId, int x, int y) {

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
	const u32 Func_CreateThingy = 0x00488210;
	CThingy* createThingy(u32 spriteId, s16 x, s16 y, u32 playerId) {

		static CThingy* thingy;
		s32 x_ = x;

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

	const u32 Func_incrementUnitScoresEx = 0x00488D50;
	//unk1 is ECX, unk2 is pushed value
	void incrementUnitScoresEx(CUnit* building, s32 unk1, s32 unk2) {

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

	const u32 Func_PlayBuildingCompleteSound = 0x0048F310;
	void playBuildingCompleteSound(CUnit* building) {

		__asm {
			PUSHAD
			MOV EAX, building
			CALL Func_PlayBuildingCompleteSound
			POPAD
		}

	}

	;

	const u32 Func_refreshSpriteData = 0x004983A0;
	void refreshSpriteData(CSprite* sprite) {

		__asm {
			PUSHAD
			MOV EAX, sprite
			CALL Func_refreshSpriteData
			POPAD
		}

	}

	;

	const u32 Func_displayLastNetErrForPlayer = 0x0049E530;
	void displayLastNetErrForPlayer(u32 playerId) {
		__asm {
			PUSHAD
			PUSH playerId
			CALL Func_displayLastNetErrForPlayer
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

	const u32 Func_CreateUnit = 0x004A09D0;
	CUnit* createUnit(u32 unitId, int x, int y, u32 playerId) {

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
	int getNumberOfUnitType(const CUnit* unit, u16 unitId, bool ignoreIncomplete);
} //unnamed namespace


  //-------- Actual hook functions --------//

namespace hooks {

	//Checks if @p unitId is a building that can be morphed from another building.
	//Note: This hook affects the behavior of CUnit::isRemorphingBuilding().
	bool isMorphedBuildingHook(u16 unitId) {
		//Default StarCraft behavior

		if (unitId == UnitId::ZergLair
			|| unitId == UnitId::ZergHive
			|| unitId == UnitId::ZergGuardianRoost
			|| unitId == UnitId::ZergDevourerHaunt
			|| unitId == UnitId::ZergGorgolethApiary
			|| unitId == UnitId::ZergLurkerDen
			|| unitId == UnitId::ZergBasiliskDen
			|| unitId == UnitId::ZergSunkenColony
			|| unitId == UnitId::ZergSporeColony
			|| unitId == UnitId::ZergMutationPit
			|| isProtossMorph(unitId))
			return true;

		return false;
	}

	//Determines the number of units which have @p unitId and are being built by the
	//@p unit. This hook is used by StarCraft's AI for counting units and buildings.
	//For example, Greater Spires are counted as Spires, so that the AI would not
	//try to rebuild another Spire after morphing one into a Greater Spire.
	int getMorphBuildingTypeCountHook(const CUnit* unit, u16 unitId, bool ignoreIncomplete) {
		//Default StarCraft behavior

		int unitCount = getNumberOfUnitType(unit, unitId, ignoreIncomplete);

		switch (unitId) {
		case UnitId::ZergHatchery:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergLair, ignoreIncomplete);
		case UnitId::ZergLair:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergHive, ignoreIncomplete);
			break;

		case UnitId::ZergCreepColony:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergSunkenColony, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ZergSporeColony, ignoreIncomplete);
			break;

		case UnitId::ZergSpawningPool:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergVorvalPond, ignoreIncomplete);
			break;

		case UnitId::ZergSpire:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergGuardianRoost, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ZergDevourerHaunt, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ZergGorgolethApiary, ignoreIncomplete);
			break;

		case UnitId::ZergHydraliskDen:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergBasiliskDen, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ZergLurkerDen, ignoreIncomplete);
			break;

		case UnitId::ZergEvolutionChamber:
			unitCount += getNumberOfUnitType(unit, UnitId::ZergMutationPit, ignoreIncomplete);
			break;

		case UnitId::ProtossGateway:
			unitCount += getNumberOfUnitType(unit, UnitId::ProtossPrincipality, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ProtossVicarage, ignoreIncomplete);

			break;
		case UnitId::ProtossRoboticsFacility:
			unitCount += getNumberOfUnitType(unit, UnitId::ProtossRoboticsAuthority, ignoreIncomplete);
			break;
		case UnitId::ProtossStargate:
			unitCount += getNumberOfUnitType(unit, UnitId::ProtossArgosy, ignoreIncomplete);
			unitCount += getNumberOfUnitType(unit, UnitId::ProtossStarfield, ignoreIncomplete);

			break;

		}
		return unitCount;
	}

	void SetSpecialState1(CSprite* sprite) {
		CImage* current_image = sprite->images.head;
		while (current_image != NULL) {
			if (current_image->id != ImageId::StargateOverlay) {
				current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
			}
			current_image = current_image->link.next;
		}
	}
} //hooks


  //-------- Helper function definitions. Do NOT modify! --------//

namespace {

	//Check if @p unit is warping in a Protoss building.
	//Identical to function @ 0x004E4C40
	bool isWarpingInProtossBuilding(const CUnit* probe) {
		return probe->mainOrderId == OrderId::BuildProtoss1
			&& probe->status & UnitStatus::GroundedBuilding
			&& probe->orderTarget.unit != NULL
			&& !(probe->orderTarget.unit->status & UnitStatus::Completed);
	}

	//Check if @p building is in the process of constructing an add-on.
	//Identical to function @ 0x004E66B0
	bool isConstructingAddon(const CUnit* building) {
		return building->secondaryOrderId == OrderId::BuildAddon
			&& building->status & UnitStatus::GroundedBuilding
			&& building->currentBuildUnit != NULL
			&& !(building->currentBuildUnit->status & UnitStatus::Completed);
	}

	//Return the building or add-on that the @p unit is constructing.
	//Identical to function @ 0x00466A30
	CUnit* getConstructRepairTarget(const CUnit* unit) {
		if (unit->mainOrderId == OrderId::BuildTerran
			|| unit->mainOrderId == OrderId::Repair1
			|| unit->mainOrderId == OrderId::ConstructingBuilding
			|| isWarpingInProtossBuilding(unit))
			return unit->orderTarget.unit;

		if (unit->secondaryOrderId == OrderId::Train
			|| unit->secondaryOrderId == OrderId::TrainFighter
			|| isConstructingAddon(unit))
			return unit->currentBuildUnit;

		return NULL;
	}

	//Identical to function @ 0x00466B70
	int getNumberOfUnitTypeInBuildQueue(const CUnit* unit, u16 unitId) {
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
			if (getConstructRepairTarget(unit) == NULL
				&& unit->buildQueue[unit->buildQueueSlot % 5] == unitId
				&& (unit->mainOrderId == OrderId::BuildTerran
					|| unit->mainOrderId == OrderId::BuildProtoss1
					|| unit->mainOrderId == OrderId::DroneLand
					|| unit->mainOrderId == OrderId::DroneBuild
					|| unit->mainOrderId == OrderId::DroneStartBuild))
				return 1;
			else
				return 0;
		}

		if (scbw::isZergEgg(unit->id,true,false)
			|| unit->mainOrderId == OrderId::Morph2
			|| unit->mainOrderId == OrderId::ZergBuildSelf)
			return (unit->buildQueue[unit->buildQueueSlot % 5] == unitId) ? 1 : 0;

		int unitCount = 0;
		for (int i = 0; i < 5; ++i) {
			if (unit->buildQueue[(unit->buildQueueSlot + i) % 5] == unitId)
				++unitCount;
		}
		return unitCount;
	}

	int getNumberOfUnitType(const CUnit* unit, u16 unitId, bool ignoreIncomplete) {
		if (!ignoreIncomplete || unit->status & UnitStatus::Completed) {
			if (unit->id == unitId)
				return 1;

			if (!ignoreIncomplete)
				return getNumberOfUnitTypeInBuildQueue(unit, unitId);
		}

		return 0;
	}

} //unnamed namespace
