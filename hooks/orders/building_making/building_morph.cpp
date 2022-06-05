#include <SCBW/api.h>
#include "building_modifier.h"
//helper functions def
//from unh
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\building_making\building_helper.h"
#include "logger.h"
#include <globals.h>

namespace hooks
{
	bool is_morphing(Unit* building)
	{
		bool bIsMorphing;
		if (!(building->status & UnitStatus::Completed))
		{
			// was hardcoded instead of using isValidMorph in original code
			bIsMorphing = helper::isValidMorph(building->build_queue[building->build_queue_slot]);
		}
		else
		{
			bIsMorphing = false;
		}
		return bIsMorphing;
	}

	void orders_zerg_build_self(Unit* building)
	{
		BuildingModifier modifier = get_building_modifier(building, 0);

		const s32 OPCWAL_PROGRESS_INCREASE = 16;
		u16 buildingId;
		bool jump_to_5D56C = false;
		bool jump_to_5D718 = false;
		bool jump_to_5D720 = false;
		buildingId = building->build_queue[building->build_queue_slot];

		if (!scbw::is_zerg_building_morph_result(buildingId))
		{
			buildingId = building->id;
		}

		// Remove statuses if the source Droleth had them before morphing
		if (building->status & UnitStatus::NoCollide)
		{
			building->status &= ~UnitStatus::NoCollide;
		}
		if (building->status & UnitStatus::IsGathering)
		{
			building->status &= ~UnitStatus::IsGathering;
		}

		if (building->main_order_state == 0)
		{
			if (building->remaining_build_time < (units_dat::TimeCost[buildingId] * 3 / 4))
			{
				building->main_order_state = 1;
			}
			jump_to_5D56C = true;
		}
		else if (building->main_order_state > 6)
		{
			jump_to_5D56C = true;
		}

		if (jump_to_5D56C)
		{ // 0045D56C
			jump_to_5D56C = false;
			if (building->remaining_build_time == 0)
			{
				jump_to_5D720 = true;
			}
			else if (!scbw::is_cheat_enabled(CheatFlags::OperationCwal))
			{
				jump_to_5D718 = true;
			}
			else
			{					
				if (building->remaining_build_time >= OPCWAL_PROGRESS_INCREASE)
				{
					building->remaining_build_time -= OPCWAL_PROGRESS_INCREASE;
				}
				else
				{
					building->remaining_build_time = 0;
				}
				jump_to_5D720 = true;
			}
		}
		else if (building->main_order_state == 1)
		{ // 0045D5AE
			if (building->id == UnitId::ZergExcisant)
			{
				Image* overlay = building->sprite->get_overlay(ImageId::VespeneGeyser2);
				if (overlay != nullptr)
				{
					overlay->free();
				}
			}
			building->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);
			building->main_order_state = 2;
			jump_to_5D56C = true;
		}
		else if (building->main_order_state == 2)
		{ // 0045D5E2
			if (building->remaining_build_time < units_dat::TimeCost[buildingId] / 2)
			{
				building->main_order_state = 3;
			}
			jump_to_5D56C = true;
		}
		else if (building->main_order_state == 3)
		{ // 0045D603
			building->sprite->play_iscript_animation(IscriptAnimation::SpecialState2, true);
			building->main_order_state = 4;
			jump_to_5D56C = true;

		}
		else if (building->main_order_state == 4)
		{ // 0045D618
			if (building->remaining_build_time == 0)
			{
				building->main_order_state = 5;
			}
			jump_to_5D56C = true;
		}
		else if (building->main_order_state == 5)
		{ // 005D62F
			building->sprite->play_iscript_animation(IscriptAnimation::AlmostBuilt, true);
			building->main_order_state = 6;
			helper::play_building_completed_sound(building);
			jump_to_5D56C = true;

			// TODO: Cleanup
			/*
			bool sEnd = false;
			if (building->order_signal & 0x1)
			{
				building->sprite->play_iscript_animation(IscriptAnimation::AlmostBuilt, true);
				sEnd = true;
				replaceSpriteImages_copy(building->sprite, scbw::get_sprite_data(SpritesDatEntries::ImageId, building->sprite->sprite_id), 0);
				function_004D8500_copy(building->sprite->main_graphic);
			}

			if (sEnd)
			{
				building->order_signal |= 4;
				building->main_order_state = 6;
				play_building_completed_sound(building);
			}
			jump_to_5D56C = true;
			*/
		}
		else if (building->main_order_state == 6)
		{ // 005D64B
			bool ordCondition = false;
			if (building->order_signal & 0x4)
			{
				ordCondition = true;
			}
			if (building->order_signal & 4)
			{
				if (building->order_signal & 4)
				{
				}
				if (building->order_signal & 1)
				{
				}
				s32 hpAfterMorph;
				Image* current_image;
				building->order_signal -= 4;
				if (helper::isValidMorph(buildingId))
				{
					// save hp before some functions modify them
					hpAfterMorph = building->hitpoints;
					u32 save_shields = building->shields;
					// update score
					building->status |= UnitStatus::Completed;
					helper::increment_unit_scores_ex(building, -1, 0);

					// do the building change
					building->status -= UnitStatus::Completed;
					helper::replace_unit_with_type(building, buildingId);
					// newHp = newHpMax - oldHpMax + oldHp
					hpAfterMorph =
						units_dat::MaxHitPoints[buildingId] -
						units_dat::MaxHitPoints[building->previous_unit_type] +
						hpAfterMorph;
					if (hpAfterMorph < 256)
					{
						hpAfterMorph = 256;
					}
					building->setHp(hpAfterMorph);
					building->remaining_build_time = 0;
				}
				// 0045D6CD
				helper::update_unit_stats_and_finish_building(building);
				helper::update_unit_strength(building);
				// relate to building placement / taking room around?
				;
				current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
					current_image = current_image->link.next;
				}
				helper::SpreadInitialCreep(buildingId, building->sprite->position.x, building->sprite->position.y);
			}
		}

		if (jump_to_5D56C)
		{ // 0045D56C
			jump_to_5D56C = false;
			if (building->remaining_build_time == 0)
			{
				jump_to_5D720 = true;
			}
			else if (!scbw::is_cheat_enabled(CheatFlags::OperationCwal))
			{
				jump_to_5D718 = true;
			}
			else
			{
				if (building->remaining_build_time >= OPCWAL_PROGRESS_INCREASE)
				{
					building->remaining_build_time -= OPCWAL_PROGRESS_INCREASE;
				}
				else
				{
					building->remaining_build_time = 0;
				}
				jump_to_5D720 = true;
			}
		}

		if (jump_to_5D718)
		{
			jump_to_5D718 = false;

			building->remaining_build_time--;

			if (modifier.decrease)
			{
				if (building->remaining_build_time > modifier.decrease)
				{
					building->remaining_build_time -= modifier.decrease;
				}
				else
				{
					building->remaining_build_time = 0;
				}
			}

			jump_to_5D720 = true;
		}

		if (jump_to_5D720)
		{
			jump_to_5D720 = false;
			if (!scbw::is_zerg_building_morph_result(buildingId))
			{
				s32 hpGain = building->build_repair_hp_gain * modifier.multiplier;
				if (scbw::is_cheat_enabled(CheatFlags::OperationCwal))
				{
					hpGain *= OPCWAL_PROGRESS_INCREASE;
				}
				building->setHp(building->hitpoints + hpGain);
			}
		}
	}

	void zerg_place_building(Unit* unit)
	{
		bool bStopThere = false;
		if (!(unit->status & UnitStatus::Completed))
		{
			u16 builtUnitId = unit->build_queue[unit->build_queue_slot];
			// original code was hardcoded instead of using isValidMorph
			if (helper::isValidMorph(builtUnitId))
			{
				if (unit->get_race() == RaceId::Zerg)
				{
					helper::zergPlaceBuildingCntd(unit);
				}
				else
				{ // ALL of this code is no longer run; previously used for protoss building morphs (e.g. gateway -> principality)
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					resources->minerals[unit->player] += (units_dat::MineralCost[builtUnitId] * 3) / 4;
					resources->gas[unit->player] += (units_dat::GasCost[builtUnitId] * 3) / 4;
					unit->status |= UnitStatus::Completed;
					helper::increment_unit_scores_ex(unit, -1, 0);
					unit->status -= UnitStatus::Completed;
					u32 hp = unit->hitpoints;
					u32 shield = unit->shields;
					helper::update_unit_stats_and_finish_building(unit);
					helper::update_unit_strength(unit);
					unit->hitpoints = hp;
					unit->shields = shield;
					Image* anchor = unit->get_overlay(ImageId::WarpAnchor);
					if (anchor != nullptr)
					{
						anchor->play_iscript_animation(IscriptAnimation::Death);
					}
					else
					{
						unit->sprite->create_top_overlay(ImageId::UnusedHeal_Small);
						anchor = unit->get_overlay(ImageId::UnusedHeal_Small);
						if (anchor != nullptr)
						{
							anchor->play_iscript_animation(IscriptAnimation::Death);
						}
					}
					// TODO: Why is this commented?
					//scbw::refresh_console();
				}
				bStopThere = true;
			}
		}

		if (!bStopThere)
		{
			static u32* const u32_00581E44 = (u32*)0x00581E44;
			static u32* const u32_00581ED4 = (u32*)0x00581ED4;
			static u16* const u16_0066345A = (u16*)0x0066345A;
			helper::refundBuildingCost(unit->id, unit->player);

			// possibly related to scores?
			u32_00581ED4[unit->player] = u32_00581ED4[unit->player] - *u16_0066345A;
			u32_00581E44[unit->player] = u32_00581E44[unit->player] - 1;

			if (unit->id == UnitId::ZergExcisant)
			{
				Unit* drone = helper::create_unit(UnitId::ZergDroleth, unit->get_x(), unit->get_y(), unit->player);
				if (drone == nullptr)
				{
					helper::refund_unit_train_cost(UnitId::ZergDroleth, unit->player);
					helper::display_last_network_error_for_player(unit->player);
					unit->remove();
					bStopThere = true;
				}
				else
				{ // 0045DB1E
					helper::update_unit_stats_and_finish_building(drone);
					helper::update_unit_strength(drone);
					drone->setHp(unit->previous_hitpoints * 256);
					unit->remove();
					bStopThere = true;
				}
			}

			if (!bStopThere)
			{ // 0045DB45
				Thingy* thingy = helper::create_thingy(SpriteId::Zerg_Building_Spawn_Small, unit->get_x(), unit->get_y(), 0);
				if (thingy != nullptr)
				{
					thingy->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
					helper::set_thingy_visibility_flags(thingy);
				}
				helper::play_sound_from_unit(SoundId::Misc_ZBldgPlc_wav_1, unit, 1, 0);
				helper::replace_unit_with_type(unit, UnitId::ZergDroleth);
				// probably AI related
				helper::updateBuildingsAI(unit);
				helper::update_unit_stats_and_finish_building(unit);
				helper::update_unit_strength(unit);

				if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW &&
					unit->sprite->images.tail->verticalOffset != 7)
				{
					unit->sprite->images.tail->flags |= ImageFlags::Redraw;
					unit->sprite->images.tail->verticalOffset = 7;
				}

				helper::refreshSpriteData(unit->sprite);
				unit->order_computer(OrderId::ResetCollision1);
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, false);
				unit->setHp(unit->previous_hitpoints * 256);

				helper::update_creep_disappearance_unit_id(unit->id, unit->get_x(), unit->get_y(), false);
			}
		}
	}
}

// from unhooked
namespace
{
	s32 BuildingHelper::get_unit_id_count(const Unit* unit, u16 unit_id, bool ignoreIncomplete);
}

namespace hooks
{
	// Checks if @p unit_id is a building that can be morphed from another building.
	// Note: This hook affects the behavior of Unit::isRemorphingBuilding().
	bool is_morphed_building(u16 unit_id)
	{
		//Default StarCraft behavior
		if (scbw::is_zerg_building_morph_result(unit_id))
		{
			return true;
		}
		return false;
	}

	// Determines the number of units which have @p unit_id and are being built by the
	// @p unit. This hook is used by StarCraft's AI for counting units and buildings.
	s32 get_morph_building_id_count(const Unit* unit, u16 unit_id, bool ignoreIncomplete)
	{
		s32 unitCount = BuildingHelper::get_unit_id_count(unit, unit_id, ignoreIncomplete);

		switch (unit_id)
		{
			case UnitId::ZergHatchery:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLair, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergHive, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSire, ignoreIncomplete);
			} break;

			case UnitId::ZergLair:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergHive, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSire, ignoreIncomplete);
			} break;

			case UnitId::ZergHive:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSire, ignoreIncomplete);
			} break;

			case UnitId::ZergCreepColony:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSunkenColony, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSporeColony, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLarvalColony, ignoreIncomplete);
			} break;

			case UnitId::ZergIrolIris:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOthstolOviform, ignoreIncomplete);
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlaszilArbor, ignoreIncomplete);
			} break;

			case UnitId::ZergOthstolOviform:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlaszilArbor, ignoreIncomplete);
			} break;

			case UnitId::ZergQuazrokPool:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLiivralPond, ignoreIncomplete);
			} break;

			case UnitId::ZergHydrokDen:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergGorgralSwamp, ignoreIncomplete);
			} break;

			case UnitId::ZergMuthrokSpire:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergVithrathHaunt, ignoreIncomplete);
			} break;

			case UnitId::ZergMatravilNest:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlmakisAntre, ignoreIncomplete);
			} break;

			case UnitId::ZergGeszkathGrotto:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlkajChasm, ignoreIncomplete);
			} break;

			case UnitId::ZergZorkizShroud:
			{
				unitCount += BuildingHelper::get_unit_id_count(unit, UnitId::ZergUltrokCavern, ignoreIncomplete); 
			} break;
		}
		return unitCount;
	}

	void set_special_state_1(Sprite* sprite)
	{
		Image* current_image = sprite->images.head;
		while (current_image != nullptr)
		{
			if (current_image->id != ImageId::StargateOverlay)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
			}
			current_image = current_image->link.next;
		}
	}
}