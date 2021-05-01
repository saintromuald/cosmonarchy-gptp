#include "building_protoss.h"
#include <SCBW/api.h>

//helper functions def

namespace {

	

} //unnamed namespace

namespace hooks {

	void orders_ProbeBuild(CUnit* unit) {

		if (unit->pAI != nullptr)
			unit->status |= UnitStatus::IsGathering;

		if (unit->mainOrderState == 0) {
			if (function_004EB9C0(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y))
				unit->mainOrderState = 1;
		} else
			if (unit->mainOrderState == 1) {

				uint32 distance, distance_wanted;

				distance = scbw::getDistanceFast(unit->halt.x, unit->halt.y, unit->orderTarget.pt.x * 256, unit->orderTarget.pt.y * 256);

				if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ProtossAquifer)
					distance_wanted = 88;
				else
					distance_wanted = 70;

				if (distance / 256 > distance_wanted) {

					if (unit->getMovableState() != 0)
						function_004EB9C0(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

				} else { //E4DA9
					makeToHoldPosition(unit);
					unit->mainOrderState = 2;
				}

			} else
				if (unit->mainOrderState == 2) {

					if (!(unit->movementFlags & MovementFlags::Accelerating)) {

						uint32 distance_wanted;
						bool isBuildingAssimilator = (unit->buildQueue[unit->buildQueueSlot] == UnitId::ProtossAquifer);

						if (isBuildingAssimilator)
							distance_wanted = 88;
						else
							distance_wanted = 70;

						if (!isDistanceGreaterThanHaltDistance(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y, distance_wanted)) {

							if (unit->getMovableState() != 0)
								function_004EB9C0(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

						} else {

							if (function_00467030(unit)) {

								CUnit* builtUnit;

								if (isBuildingAssimilator)
									builtUnit = function_004678A0(unit, UnitId::ProtossAquifer);
								else
									builtUnit = createUnit(
										unit->buildQueue[unit->buildQueueSlot],
										unit->orderTarget.pt.x,
										unit->orderTarget.pt.y,
										unit->playerId
									);

								unit->buildQueue[unit->buildQueueSlot] = UnitId::None;

								if (builtUnit == nullptr) {
									displayLastNetErrorForPlayer(unit->playerId);
									unit->orderToIdle();
								} else { //E4E63

									replaceSpriteImages(builtUnit->sprite, ImageId::WarpAnchor, 0);

									if (isBuildingAssimilator)
										function_00498D70(builtUnit->sprite, ImageId::VespeneGeyser, 0, 0, 0);

									unit->orderTarget.unit = builtUnit;

									AI_TrainingOverlord(unit, builtUnit);

									builtUnit->orderComputerCL(OrderId::BuildSelf2);

									scbw::playSound(SoundId::Protoss_SHUTTLE_pshBld00_wav, unit);
									unit->mainOrderState = 3;

								}

							} else
								unit->orderToIdle();

						}

					}

				} else
					if (unit->mainOrderState == 3) {

						CUnit* target = unit->orderTarget.unit;
						CImage* current_image;

						if (target == nullptr ||
							target->id != UnitId::ProtossAquifer ||
							unit->orderQueueHead != nullptr
							)
							unit->orderToIdle();
						else
							unit->orderTo(OrderId::HarvestGas2, target);

						current_image = unit->sprite->images.head;

						while (current_image != nullptr) {
							current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
							current_image = current_image->link.next;
						}

					}

	}

	;

	//Protoss building self-building
	void orders_BuildSelf2(CUnit* building) {

		bool jump_to_E50EE = false;

		if (building->mainOrderState > 3)
			jump_to_E50EE = true;
		else
			if (building->mainOrderState == 0) {

				if (building->remainingBuildTime == 0) {

					CImage* current_image = building->sprite->images.head;

					while (current_image != nullptr) {

						if (current_image->flags & CImage_Flags::Flag04) {

							const uint32* u32_006D1200 = (uint32*)0x006D1200;

							current_image->animation = IscriptAnimation::SpecialState1;
							current_image->iscriptOffset = *(uint16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x22);
							current_image->wait = 0;
							current_image->unknown14 = 0;
							iscript_OpcodeCases(current_image, (uint32)&current_image->iscriptHeaderOffset, 0, 0);

						}

						current_image = current_image->link.next;

					}

					scbw::playSound(SoundId::Protoss_SHUTTLE_pshBld03_wav, building);
					building->mainOrderState = 1;
					jump_to_E50EE = true;

				} else
					jump_to_E50EE = true;

			} else
				if (building->mainOrderState == 1) {

					if (building->orderSignal & 1) {

						building->orderSignal -= 1;

						replaceSpriteImages(
							building->sprite,
							//					sprites_dat::ImageId[building->sprite->spriteId],
							scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId),
							0
						);

						function_004D8500(building->sprite->mainGraphic);

						building->mainOrderState = 2;

					}

				} else
					if (building->mainOrderState == 2) {

						if (building->orderSignal & 1) {

							CImage* current_image;

							building->orderSignal -= 1;

							replaceSpriteImages(
								building->sprite,
								//					sprites_dat::ImageId[building->sprite->spriteId],
								scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId),
								0
							);

							current_image = building->sprite->images.head;

							while (current_image != nullptr) {

								if (current_image->flags & CImage_Flags::Flag04) {

									const uint32* u32_006D1200 = (uint32*)0x006D1200;

									current_image->animation = IscriptAnimation::WarpIn;
									current_image->iscriptOffset = *(uint16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
									current_image->wait = 0;
									current_image->unknown14 = 0;
									iscript_OpcodeCases(current_image, (uint32)&current_image->iscriptHeaderOffset, 0, 0);

								}

								current_image = current_image->link.next;

							}

							building->mainOrderState = 3;
							building->orderSignal |= 1;

						}

					} else
						if (building->mainOrderState == 3) {

							if (building->orderSignal & 1) {

								building->orderSignal -= 1;

								function_004A01F0(building);
								updateUnitStrength(building);
								function_0042D9A0(building);

								if (building->status & UnitStatus::DoodadStatesThing) {

									CImage* current_image = building->sprite->images.head;

									while (current_image != nullptr) {

										if (current_image->flags & CImage_Flags::Flag04) {

											const uint32* u32_006D1200 = (uint32*)0x006D1200;

											current_image->animation = IscriptAnimation::Disable;
											current_image->iscriptOffset = *(uint16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x38);
											current_image->wait = 0;
											current_image->unknown14 = 0;
											iscript_OpcodeCases(current_image, (uint32)&current_image->iscriptHeaderOffset, 0, 0);

										}

										current_image = current_image->link.next;

									}

									jump_to_E50EE = true;

								} else
									jump_to_E50EE = true;

							}

						}

					if (jump_to_E50EE) {

						uint32 hpGain, shieldGain;

						if (building->remainingBuildTime != 0) {
							if (*CHEAT_STATE & CheatFlags::OperationCwal) {
								if (building->remainingBuildTime > 16)
									building->remainingBuildTime -= 16;
								else
									building->remainingBuildTime = 0;
							} else
								building->remainingBuildTime--;
						}

						if (*CHEAT_STATE & CheatFlags::OperationCwal)
							hpGain = building->buildRepairHpGain * 16;
						else
							hpGain = building->buildRepairHpGain;

						building->setHp(building->hitPoints + hpGain);

						if (*CHEAT_STATE & CheatFlags::OperationCwal)
							shieldGain = building->shieldGain * 16;
						else
							shieldGain = building->shieldGain;

						if (building->shields + shieldGain >= (uint32)units_dat::MaxShieldPoints[building->id] * 256)
							building->shields = units_dat::MaxShieldPoints[building->id] * 256;
						else
							building->shields += shieldGain;

					}

	}

	;

} //namespace hooks