#include "building_protoss.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	void orders_ProbeBuild(Unit* unit)
	{
		if (unit->unit_ai != nullptr)
		{
			unit->status |= UnitStatus::IsGathering;
		}

		if (unit->mainOrderState == 0)
		{
			if (helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y))
			{
				unit->mainOrderState = 1;
			}
		}
		else if (unit->mainOrderState == 1)
		{
			u32 distance, distance_wanted;
			distance = scbw::get_distance_fast(unit->halt.x, unit->halt.y, unit->orderTarget.pt.x * 256, unit->orderTarget.pt.y * 256);
			if (unit->buildQueue[unit->buildQueueSlot] == UnitId::ProtossAquifer)
			{
				distance_wanted = 88;
			}
			else
			{
				distance_wanted = 70;
			}

			if (distance / 256 > distance_wanted)
			{
				if (unit->getMovableState() != 0)
				{
					helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}
			}
			else // 004E4DA9
			{
				helper::make_to_hold_position(unit);
				unit->mainOrderState = 2;
			}
		}
		else if (unit->mainOrderState == 2)
		{
			if (!(unit->movementFlags & MovementFlags::Accelerating))
			{
				u32 distance_wanted;
				bool isBuildingAssimilator = (unit->buildQueue[unit->buildQueueSlot] == UnitId::ProtossAquifer);
				if (isBuildingAssimilator)
				{
					distance_wanted = 88;
				}
				else
				{
					distance_wanted = 70;
				}

				if (!helper::isDistanceGreaterThanHaltDistance(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y, distance_wanted))
				{
					if (unit->getMovableState() != 0)
					{
						helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					}
				}
				else
				{
					if (helper::beginBuilding(unit))
					{
						Unit* builtUnit;
						if (isBuildingAssimilator)
						{
							builtUnit = helper::beginGasBuilding(unit, UnitId::ProtossAquifer);
						}
						else
						{
							builtUnit = helper::create_unit(unit->buildQueue[unit->buildQueueSlot], unit->orderTarget.pt.x, unit->orderTarget.pt.y, unit->player);
						}

						u32 unit_id = unit->buildQueue[unit->buildQueueSlot];
						unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
						if (builtUnit == nullptr)
						{
							helper::refundUnitTrainCost(unit_id, unit->player);
							helper::display_last_network_error_for_player(unit->player);
							unit->orderToIdle();
						}
						else // 004E4E63
						{
							helper::replaceSpriteImages(builtUnit->sprite, ImageId::WarpAnchor, 0);
							if (isBuildingAssimilator)
							{
								builtUnit->sprite->createUnderlay(ImageId::VespeneGeyser, 0, 0, 0);
							}
							builtUnit->sprite->createTopUnderlay(ImageId::WarpAnchorUnderlay, 0, 0, 0);
							unit->orderTarget.unit = builtUnit;
							helper::ai_training_overlord(builtUnit, unit);
							builtUnit->order_computer(OrderId::BuildSelf2);
							helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld00_wav, unit, 1, 0);
							unit->mainOrderState = 3;
						}
					}
					else
					{
						unit->orderToIdle();
					}
				}
			}

		}
		else if (unit->mainOrderState == 3)
		{
			Unit* target = unit->orderTarget.unit;
			Image* current_image;
			if (target == nullptr
				|| target->id != UnitId::ProtossAquifer
				|| unit->orderQueueHead != nullptr)
			{
				unit->orderToIdle();
			}
			else
			{
				unit->orderTo(OrderId::CanEnterGasMine, target);
			}
			current_image = unit->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
	}

	// Protoss self-building
	void orders_BuildSelf2(Unit* building)
	{
		bool jump_to_E50EE = false;
		if (building->mainOrderState > 3)
		{
			jump_to_E50EE = true;
		}
		else if (building->mainOrderState == 0)
		{
			if (building->remainingBuildTime == 0)
			{
				Image* current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::SpecialState1;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x22);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_OpcodeCases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}
				helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld03_wav, building, 1, 0);
				building->mainOrderState = 1;
				jump_to_E50EE = true;
			}
			else
			{
				jump_to_E50EE = true;
			}
		}
		else if (building->mainOrderState == 1)
		{
			if (building->orderSignal & 1)
			{
				building->orderSignal &= ~1;
				helper::replaceSpriteImages(building->sprite, scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId), 0);
				helper::switchToWarpTexture(building->sprite->mainGraphic);
				building->mainOrderState = 3; // DF: Skip state 2 as it's now redundant.
			}
		}
		else if (building->mainOrderState == 2) // DF: Could theorically comment out this entire if block, but I haven't extensively checked/tested if it's truly not used by anything else. Your call.
		{
			if (building->orderSignal & 1)
			{
				Image* current_image;
				building->orderSignal &= ~1;
				helper::replaceSpriteImages(building->sprite, scbw::getSpriteData(SpritesDatEntries::ImageId, building->sprite->spriteId), 0);
				current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::WarpIn;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_OpcodeCases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}
				building->mainOrderState = 3;
				building->orderSignal |= 1; // DF: This is what skipped the warp fadeout animation.
			}
		}
		else if (building->mainOrderState == 3)
		{
			if (building->orderSignal & 1)
			{
				Image* current_image; // DF: Copied over from state 2.
				building->orderSignal &= ~1;
				helper::update_unit_stats_and_finish_building(building); // DF: Copying over replaceSpriteImages is not necessary because of this function.
				helper::update_unit_strength(building);
				helper::checkUnstack(building);
				current_image = building->sprite->images.head; // DF: Copied over from state 2.
				while (current_image != nullptr) // DF: Copied over from state 2.
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::WarpIn;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_OpcodeCases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}

				if (building->status & UnitStatus::Frozen)
				{
					Image* current_image = building->sprite->images.head;
					while (current_image != nullptr)
					{
						if (current_image->flags & ImageFlags::Flag04)
						{
							const u32* u32_006D1200 = (u32*)0x006D1200;
							current_image->animation = IscriptAnimation::Disable;
							current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x38);
							current_image->wait = 0;
							current_image->unknown14 = 0;
							helper::iscript_OpcodeCases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
						}
						current_image = current_image->link.next;
					}
					jump_to_E50EE = true;
				}
				else
				{
					jump_to_E50EE = true;
				}
			}
		}

		if (jump_to_E50EE)
		{
			u32 hpGain, shieldGain;			
			BuildingModifier modifier = get_building_modifier(building, 0);
			if (building->remainingBuildTime != 0)
			{
				if (*CHEAT_STATE & CheatFlags::OperationCwal)
				{
					if (building->remainingBuildTime > 16)
					{
						building->remainingBuildTime -= 16;
					}
					else
					{
						building->remainingBuildTime = 0;
					}
				}
				else
				{
					building->remainingBuildTime--;

					if (modifier.decrease)
					{
						if (building->remainingBuildTime > modifier.decrease)
						{
							building->remainingBuildTime -= modifier.decrease;
						}
						else
						{
							building->remainingBuildTime = 0;
						}
					}
				}
			}

			if (*CHEAT_STATE & CheatFlags::OperationCwal)
			{
				hpGain = building->buildRepairHpGain * 16;
			}
			else
			{
				hpGain = building->buildRepairHpGain * modifier.multiplier;
			}
			building->setHp(building->hitPoints + hpGain);

			if (*CHEAT_STATE & CheatFlags::OperationCwal)
			{
				shieldGain = building->shieldGain * 16;
			}
			else
			{
				shieldGain = building->shieldGain * modifier.multiplier;
			}

			if (building->shields + shieldGain >= (u32)units_dat::MaxShieldPoints[building->id] * 256)
			{
				building->shields = units_dat::MaxShieldPoints[building->id] * 256;
			}
			else
			{
				building->shields += shieldGain;
			}
		}
	}
}