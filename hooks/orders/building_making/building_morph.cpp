#include <SCBW/api.h>
//helper functions def
//from unh
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\building_making\building_helper.h"
#include "logger.h"

namespace hooks {
	void orders_ZergBuildSelf(CUnit* building) {
		const int OPCWAL_PROGRESS_INCREASE = 16;

		uint16 buildingId;
		bool jump_to_5D56C = false;
		bool jump_to_5D718 = false;
		bool jump_to_5D720 = false;

		buildingId = building->buildQueue[building->buildQueueSlot];

		if (!scbw::isZergBuildingMorphResult(buildingId))
			buildingId = building->id;

		if (building->mainOrderState == 0) {
			if (building->remainingBuildTime < (units_dat::TimeCost[buildingId] * 3 / 4))
				building->mainOrderState = 1;

			jump_to_5D56C = true;

		} else
			if (building->mainOrderState > 6)
				jump_to_5D56C = true;

		if (jump_to_5D56C) { // 005D56C
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
		} else if (building->mainOrderState == 1) { // 005D5AE
			if (building->id == UnitId::ZergExtractor) {
				CImage* overlay = building->sprite->getOverlay(ImageId::VespeneGeyser2);
				if (overlay != nullptr)
					overlay->free();
			}
			building->sprite->playIscriptAnim(IscriptAnimation::SpecialState1, true);
			building->mainOrderState = 2;
			//scbw::print("Set order state 2");
			jump_to_5D56C = true;

		} else if (building->mainOrderState == 2) { // 005D5E2
			if (building->remainingBuildTime < units_dat::TimeCost[buildingId] / 2) {
				//scbw::print("Set order state 3");
				building->mainOrderState = 3;
			}
			jump_to_5D56C = true;

		} else if (building->mainOrderState == 3) { // 005D603
			building->sprite->playIscriptAnim(IscriptAnimation::SpecialState2, true);
			building->mainOrderState = 4;
			//scbw::print("Set order state 4");
			jump_to_5D56C = true;

		} else if (building->mainOrderState == 4) { // 005D618

			if (building->remainingBuildTime == 0) {
				//scbw::print("Set order state 5");								

				building->mainOrderState = 5;
			}
			jump_to_5D56C = true;

		} else if (building->mainOrderState == 5) { // 005D62F

			building->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
			building->mainOrderState = 6;
			playBuildingCompleteSound(building);
			jump_to_5D56C = true;

			// TODO: Cleanup
			/*bool sEnd = false;
			//if (building->orderSignal & 0x1) {
			building->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
			sEnd = true;

			replaceSpriteImages_copy(building->sprite, scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId), 0);
			function_004D8500_copy(building->sprite->mainGraphic);
			//}
			if (sEnd) {
				//scbw::print("Set order state 6");
				building->orderSignal |= 4;
				building->mainOrderState = 6;
				playBuildingCompleteSound(building);
			}
			jump_to_5D56C = true;*/
		} else if (building->mainOrderState == 6) { // 005D64B

			bool ordCondition = false;
			if (building->orderSignal & 0x4) {
				ordCondition = true;
			}
			if (building->orderSignal & 4) {
				if (building->orderSignal & 4) {
				}
				if (building->orderSignal & 1) {
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
				// 005D6CD
				// update various stuff (set hp, set shield...) not finished on Morph
				function_004A01F0(building);
				updateUnitStrength(building);
				// relate to building placement / taking room around?
				function_0047D770(building);

				current_image = building->sprite->images.head;
				while (current_image != nullptr) {
					current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
					current_image = current_image->link.next;
				}
				updateNewUnitVision(buildingId, building->sprite->position.x, building->sprite->position.y);
			}
		}

		if (jump_to_5D56C) { // 005D56C
			jump_to_5D56C = false;

			if (building->remainingBuildTime == 0) {
				jump_to_5D720 = true;
			} else
				if (!scbw::isCheatEnabled(CheatFlags::OperationCwal)) {
					jump_to_5D718 = true;
				} else {

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
			if (!scbw::isZergBuildingMorphResult(buildingId)) {
				int32 hpGain = building->buildRepairHpGain;
				if (scbw::isCheatEnabled(CheatFlags::OperationCwal)) {
					hpGain *= OPCWAL_PROGRESS_INCREASE;
				}
				building->setHp(building->hitPoints + hpGain);
			}
		}

	} //void orders_ZergBuildSelf(CUnit* building)

	void ZergPlaceBuilding(CUnit* unit) {
		bool bStopThere = false;
		if (!(unit->status & UnitStatus::Completed)) {
			uint16 builtUnitId = unit->buildQueue[unit->buildQueueSlot];

			// original code was hardcoded instead of using isValidMorph
			if (isValidMorph(builtUnitId)) {
				if (unit->getRace() == RaceId::Zerg) {
					zergPlaceBuildingCntd(unit);
				} else {
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
						anchor->playIscriptAnim(IscriptAnimation::Death);
					} else {
						unit->sprite->createTopOverlay(970);
						anchor = unit->getOverlay(970);
						if (anchor != nullptr) {
							anchor->playIscriptAnim(IscriptAnimation::Death);
						}
					}
					//scbw::refreshConsole();
				}
				bStopThere = true;
			}
		}

		if (!bStopThere) {
			static uint32* const u32_00581E44 = (uint32*)0x00581E44;
			static uint32* const u32_00581ED4 = (uint32*)0x00581ED4;
			static uint16* const u16_0066345A = (uint16*)0x0066345A;
			refundBuildingCost(unit->id, unit->playerId);

			// possibly related to scores?
			u32_00581ED4[unit->playerId] = u32_00581ED4[unit->playerId] - *u16_0066345A;
			u32_00581E44[unit->playerId] = u32_00581E44[unit->playerId] - 1;

			if (unit->id == UnitId::ZergExtractor) {
				CUnit* drone = createUnit(UnitId::ZergDroleth, unit->getX(), unit->getY(), unit->playerId);
				if (drone == nullptr) {
					displayLastNetErrorForPlayer(unit->playerId);
					unit->remove();
					bStopThere = true;
				} else {	// 005DB1E
					// update various stuff (set hp, set shield...) not finished after createUnit
					function_004A01F0(drone);
					updateUnitStrength(drone);
					drone->setHp(unit->previousHp * 256);
					unit->remove();
					bStopThere = true;
				}
			}

			if (!bStopThere) {	// 005DB45
				CThingy* thingy = createThingy(SpriteId::Zerg_Building_Spawn_Small, unit->getX(), unit->getY(), 0);
				if (thingy != nullptr) {
					thingy->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
					setThingyVisibilityFlags(thingy);
				}

				scbw::playSound(SoundId::Misc_ZBldgPlc_wav_1, unit);
				replaceUnitWithType(unit, UnitId::ZergDroleth);

				//probably AI related
				function_00433FE0(unit);

				//update various stuff (set hp, set shield...) not finished after unit change
				function_004A01F0(unit);
				updateUnitStrength(unit);

				if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW &&
					unit->sprite->images.tail->verticalOffset != 7) {
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

				// ???
				function_0047DE40(
					unit->sprite,
					unit->id,
					unit->getX(),
					unit->getY()
				);
			}
		}
	}
}

//from unhooked

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