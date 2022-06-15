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
			// was hardcoded instead of using is_valid_morph in original code
			bIsMorphing = helper::is_valid_morph(building->build_queue[building->build_queue_slot]);
		}
		else
		{
			bIsMorphing = false;
		}
		return bIsMorphing;
	}

	void orders_zerg_build_self(Unit* building) {
		bool is_nydus = building->id == UnitId::ZergNydusCanal && building->building.nydus_exit;
		BuildingModifier modifier = get_building_modifier(building, 0, (helper::is_valid_morph(building->id) || is_nydus) ? false : true);
		if (is_nydus) {
			BuildingModifier nydus_modifier = get_building_modifier(building->building.nydus_exit, 0, false);
			modifier.decrease += nydus_modifier.decrease;
			modifier.multiplier *= nydus_modifier.multiplier;
		}

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
			if (building->remaining_build_time < (units_dat::time_cost[buildingId] * 3 / 4))
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
				if (overlay)
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
			if (building->remaining_build_time < units_dat::time_cost[buildingId] / 2)
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
				s32 hpAfterMorph;
				Image* current_image;
				building->order_signal &= ~4;
				if (helper::is_valid_morph(buildingId))
				{
					// save hp before some functions modify them
					hpAfterMorph = building->hitpoints;
					u32 save_shields = building->shields;
					// update score
					building->status |= UnitStatus::Completed;
					helper::increment_unit_scores_ex(building, -1, 0);

					// do the building change
					building->status &= ~UnitStatus::Completed;
					helper::replace_unit_with_id(building, buildingId);

					// newHp = newHpMax - oldHpMax + oldHp
					hpAfterMorph = units_dat::MaxHitPoints[buildingId] - units_dat::MaxHitPoints[building->previous_unit_type] + hpAfterMorph;
					if (hpAfterMorph < 256) {
						hpAfterMorph = 256;
					}
					building->set_hitpoints(hpAfterMorph);
					building->remaining_build_time = 0;
				} else if (building->morph_origin == UnitId::ZergGosvileth && building->previous_hitpoints >= 50) {
					Unit* worker = scbw::create_unit_in_game(UnitId::ZergGosvileth, building->player, building->position.x, building->position.y);
					worker->set_hitpoints((building->previous_hitpoints - 50) * 256);
				}
				// 0045D6CD
				helper::update_unit_stats_and_finish_building(building);
				helper::update_unit_strength(building);
				current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
					current_image = current_image->link.next;
				}
				helper::spread_initial_creep(buildingId, building->sprite->position.x, building->sprite->position.y);
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
			if (modifier.decrease) {
				if (building->remaining_build_time > modifier.decrease) {
					building->remaining_build_time -= modifier.decrease;
				} else {
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
				building->set_hitpoints(building->hitpoints + hpGain);
			}
		}
	}

	void zerg_place_building(Unit* unit) {
		bool bStopThere = false;
		if (!(unit->status & UnitStatus::Completed)) {
			u16 builtUnitId = unit->build_queue[unit->build_queue_slot];
			// original code was hardcoded instead of using is_valid_morph
			if (helper::is_valid_morph(builtUnitId)) {
				if (unit->get_race() == RaceId::Zerg) {
					helper::zergPlaceBuildingCntd(unit);
				} else { // ALL of this code is no longer run; previously used for protoss building morphs (e.g. gateway -> principality)
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					resources->minerals[unit->player] += (units_dat::mineral_cost[builtUnitId] * 3) / 4;
					resources->gas[unit->player] += (units_dat::gas_cost[builtUnitId] * 3) / 4;
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
					if (!anchor) {
						anchor->play_iscript_animation(IscriptAnimation::Death);
					} else {
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

		if (!bStopThere) {
			static u32* const u32_00581E44 = (u32*)0x00581E44;
			static u32* const u32_00581ED4 = (u32*)0x00581ED4;
			static u16* const u16_0066345A = (u16*)0x0066345A;
			helper::refund_building_cost(unit->id, unit->player);

			// possibly related to scores?
			u32_00581ED4[unit->player] = u32_00581ED4[unit->player] - *u16_0066345A;
			u32_00581E44[unit->player] = u32_00581E44[unit->player] - 1;

			if (unit->id == UnitId::ZergExcisant) {
				Unit* worker = helper::create_unit(unit->morph_origin, unit->get_x(), unit->get_y(), unit->player);
				if (!worker) {
					helper::refund_unit_train_cost(unit->morph_origin, unit->player);
					helper::display_last_network_error_for_player(unit->player);
					unit->remove();
					bStopThere = true;
				} else { // 0045DB1E
					helper::update_unit_stats_and_finish_building(worker);
					helper::update_unit_strength(worker);
					worker->set_hitpoints(unit->previous_hitpoints * 256);
					unit->remove();
					bStopThere = true;
				}
			}

			if (!bStopThere) { // 0045DB45
				Thingy* thingy = helper::create_thingy(SpriteId::Zerg_Building_Spawn_Small, unit->get_x(), unit->get_y(), 0);
				if (thingy) {
					thingy->sprite->elevation_level = unit->sprite->elevation_level + 1;
					helper::set_thingy_visibility_flags(thingy);
				}
				helper::play_sound_from_unit(SoundId::Misc_ZBldgPlc_wav_1, unit, 1, 0);


				u32 previous_hitpoints = unit->previous_hitpoints * 256;
				helper::replace_unit_with_id(unit, unit->morph_origin);
				helper::update_building_ai(unit);
				helper::update_unit_stats_and_finish_building(unit);
				helper::update_unit_strength(unit);

				if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW && unit->sprite->images.tail->verticalOffset != 7) {
					unit->sprite->images.tail->flags |= ImageFlags::Redraw;
					unit->sprite->images.tail->verticalOffset = 7;
				}

				helper::refresh_sprite_data(unit->sprite);
				unit->order_computer(OrderId::ResetCollision1);
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, false);
				unit->set_hitpoints(previous_hitpoints);

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

	// Hooked to help Veeq7 remember to go to sleep. -Pr0nogo
	void drone_morph_standard(Unit* unit, u32 id) // 0045D890
	{
		helper::increment_unit_scores_ex(unit, -1, 0);
		unit->status &= 0xFFFFFFFE;
		helper::replace_unit_with_id(unit, id);
		if (playerTable[unit->player].type == PlayerType::Computer)
		{
			if (unit->unit_ai)
			{
				WorkerAi* worker_ai = (WorkerAi*)unit->unit_ai;
				AiTown* unit_town = worker_ai->town;
				unit->unit_ai = 0;
				helper::delete_worker_ai(worker_ai, &unit_town->free_workers);
				--unit_town->worker_count;
				helper::add_worker_structure_ai(unit, unit_town);
			}
		}
		helper::drone_build_execute(unit);
	}

	// Hooked to help Veeq7 remember to go to sleep. -Pr0nogo
	Unit* begin_gas_building(Unit* unit, u8 unit_id) // 004678A0
	{
		Unit* target = unit->order_target.unit;
		if (!target || target->id != UnitId::ResourceVespeneGeyser)
		{
			target = helper::find_unit_at_tile(unit->order_target.pt.y, unit->order_target.pt.x, OrderId::Fatal);
		}
		if (!target)
		{
			return 0;
		}
		if (*unk_sprite_related)
		{
			helper::inherit_gas_overlays(target->sprite); // Guessed name, probably wrong
		}
		helper::increment_unit_scores_ex(target, -1, 1);
		target->status &= 0xFFFFFFFE;
		helper::give_unit(target, unit->player, 0);
		helper::give_sprite(target, unit->player);
		helper::replace_unit_with_id(target, unit_id);

		if (!(target->status & 0x400) && !target->lockdown_timer && !target->stasis_timer && !target->maelstrom_timer
			|| units_dat::BaseProperty[target->id] & UnitProperty::Building)
		{
			target->current_buttonset = 231;
		}
		helper::add_overlay_below_main(target->sprite, ImageId::VespeneGeyser2, 0, 0, 0);
		target->hitpoints = units_dat::MaxHitPoints[unit_id] / 10;
		return target;
	}
}