#include <SCBW/api.h>
#include "building_modifier.h"
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
			bIsMorphing = helper::is_valid_building_morph(building->build_queue[building->build_queue_slot]);
		}
		else
		{
			bIsMorphing = false;
		}
		return bIsMorphing;
	}

	void orders_zerg_build_self(Unit* building) {
		bool is_nydus = building->id == UnitId::ZergNydusCesiant && building->building.nydus_exit;
		BuildingModifier modifier = get_building_modifier(building, 0, (helper::is_valid_building_morph(building->id) || is_nydus) ? false : true);
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
			if (helper::is_valid_building_morph(buildingId)) {
				building->create_origin = building->id;
			}
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
				replaceSpriteImages_copy(building->sprite, scbw::get_sprite_data(SpritesDatEntries::ImageId, building->sprite->id), 0);
				helper::switch_to_warp_texture(building->sprite->main_graphic);
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
				if (helper::is_valid_building_morph(buildingId)) {
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
					hpAfterMorph = units_dat::max_hitpoints[buildingId] - units_dat::max_hitpoints[building->previous_unit_type] + hpAfterMorph;
					if (hpAfterMorph < 256) {
						hpAfterMorph = 256;
					}
					building->set_hitpoints(hpAfterMorph);
					building->remaining_build_time = 0;
				} else if (building->create_origin == UnitId::ZergGosvileth) {
					Unit* worker = scbw::create_unit_in_game(UnitId::ZergGosvileth, building->player, building->position.x, building->position.y);
					if (worker) {
						worker->set_hitpoints(building->previous_hitpoints * 256);
					}
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
				// Originally had code for operation cwal cheat
				building->set_hitpoints(building->hitpoints + hpGain);
			}
		}
	}

	void cancel_zerg_building(Unit* unit) // 0045DA40
	{
		bool bStopThere = false;
		if (!(unit->status & UnitStatus::Completed)) {
			u16 built_unit_id = unit->build_queue[unit->build_queue_slot];
			// original code was hardcoded instead of using is_valid_morph
			if (helper::is_valid_building_morph(built_unit_id)) {
				if (unit->get_race() == RaceId::Zerg) {
					helper::cancel_zerg_building_morph(unit);
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
				Unit* worker = helper::create_unit(unit->create_origin, unit->get_x(), unit->get_y(), unit->player);
				if (!worker) {
					helper::refund_unit_train_cost(unit->create_origin, unit->player);
					helper::display_last_network_error_for_player(unit->player);
					unit->remove();
					bStopThere = true;
				} else {
					helper::update_unit_stats_and_finish_building(worker);
					helper::update_unit_strength(worker);
					worker->set_hitpoints(unit->previous_hitpoints * 256);
					worker->sprite->player = unit->sprite->player;
					unit->remove();
					bStopThere = true;
				}
			}

			if (!bStopThere) {
				Thingy* thingy = helper::create_thingy(SpriteId::Zerg_Building_Spawn_Small, unit->get_x(), unit->get_y(), 0);
				if (thingy) {
					thingy->sprite->elevation = unit->sprite->elevation + 1;
					helper::set_thingy_visibility_flags(thingy);
				}
				helper::play_sound_from_unit(SoundId::Misc_ZBldgPlc_wav_1, unit, 1, 0);

				u32 previous_unit_id = unit->id;
				u32 previous_hitpoints = unit->previous_hitpoints * 256;
				helper::replace_unit_with_id(unit, unit->create_origin);
				helper::update_building_ai(unit);
				helper::update_unit_stats_and_finish_building(unit);
				helper::update_unit_strength(unit);

				if (unit->sprite->images.tail->palette_type == PaletteType::RLE_SHADOW && unit->sprite->images.tail->vertical_offset != 7) {
					unit->sprite->images.tail->flags |= ImageFlags::Redraw;
					unit->sprite->images.tail->vertical_offset = 7;
				}

				helper::refresh_sprite_data(unit->sprite);
				unit->order_computer(OrderId::ResetCollision1);
				unit->order(units_dat::return_to_idle_order[unit->id], 0, 0, nullptr, UnitId::None, false);
				unit->set_hitpoints(previous_hitpoints);

				helper::update_creep_disappearance_unit_id(previous_unit_id, unit->get_x(), unit->get_y(), false);
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
	// Note: This hook affects the behavior of Unit::is_remorphing_building().
	bool is_morphed_building(u16 unit_id)
	{
		if (scbw::is_zerg_building_morph_result(unit_id))
		{
			return true;
		}
		return false;
	}

	// Determines the number of units which have @p unit_id and are being built by the
	// @p unit. This hook is used by StarCraft's AI for counting units and buildings.
	s32 get_morph_building_id_count(const Unit* unit, u16 unit_id, bool ignore_incomplete)
	{
		s32 unit_count = BuildingHelper::get_unit_id_count(unit, unit_id, ignore_incomplete);

		switch (unit_id)
		{
			case UnitId::ZergHachirosk:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergCaphrolosk, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergGathtelosk, ignore_incomplete);
			} break;

			case UnitId::ZergCagrantColony:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSurkithColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpraithColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLarvoskColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOcsalColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergKazitrithColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSkortrithColony, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpirtithColony, ignore_incomplete);
			} break;

			case UnitId::ZergSurkithColony:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSkortrithColony, ignore_incomplete);
			}

			case UnitId::ZergSpraithColony:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergSpirtithColony, ignore_incomplete);
			}

			case UnitId::ZergIrolIris:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergIrulentIvoa, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOthstolOviform, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOrbothOmlia, ignore_incomplete);
			} break;

			case UnitId::ZergIrulentIvoa:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOthstolOviform, ignore_incomplete);
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOrbothOmlia, ignore_incomplete);
			} break;

			case UnitId::ZergOthstolOviform:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergOrbothOmlia, ignore_incomplete);
			} break;

			case UnitId::ZergQuazrokPool:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergLiivralPond, ignore_incomplete);
			} break;

			case UnitId::ZergHydrokDen:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergGorgralSwamp, ignore_incomplete);
			} break;

			case UnitId::ZergMuthrokSpire:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergVithrathHaunt, ignore_incomplete);
			} break;

			case UnitId::ZergMatravilNest:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlmakisAntre, ignore_incomplete);
			} break;

			case UnitId::ZergGeszkathGrotto:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergAlkajChasm, ignore_incomplete);
			} break;

			case UnitId::ZergZorkizShroud:
			{
				unit_count += BuildingHelper::get_unit_id_count(unit, UnitId::ZergUltrokCavern, ignore_incomplete); 
			} break;
		}
		return unit_count;
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
	void drone_morph_standard(Unit* unit, u32 id) { // 0045D890
		helper::increment_unit_scores_ex(unit, -1, 0);
		unit->status &= ~UnitStatus::Completed;
		unit->create_origin = unit->id;
		helper::replace_unit_with_id(unit, id);
		if (g_player_table[unit->player].type == PlayerType::Computer) {
			if (unit->unit_ai) {
				WorkerAi* worker_ai = (WorkerAi*)unit->unit_ai;
				AiTown* unit_town = worker_ai->town;
				unit->unit_ai = 0;
				helper::delete_worker_ai(worker_ai, &unit_town->workers);
				--unit_town->worker_count;
				helper::add_worker_structure_ai(unit, unit_town);
			}
		}
		helper::drone_build_execute(unit);
	}

	// Hooked to help Veeq7 remember to go to sleep. -Pr0nogo
	Unit* begin_gas_building(Unit* unit, u16 unit_id) { // 004678A0
		Unit* target = unit->order_target.unit;
		if (!target || target->id != UnitId::ResourceVespeneGeyser) {
			target = helper::find_unit_at_tile(unit->order_target.pt.y, unit->order_target.pt.x, OrderId::Fatal);
		}
		if (!target) {
			return 0;
		}
		if (*can_draw_placement_preview) {
			helper::inherit_gas_overlays(target->sprite); // Guessed helper name, probably wrong
		}
		helper::increment_unit_scores_ex(target, -1, 1);
		target->status &= ~UnitStatus::Completed;
		helper::give_unit(target, unit->player, 0);
		helper::give_sprite(target, unit->player);
		target->create_origin = unit->id;
		helper::replace_unit_with_id(target, unit_id);
		if (!scbw::unit_is_stunned(target) || units_dat::base_property[target->id] & UnitProperty::Building) {
			target->current_buttonset = UnitId::Buttons_CancelConstruction;
		}
		helper::add_overlay_below_main(target->sprite, ImageId::VespeneGeyser2, 0, 0, 0);
		target->hitpoints = units_dat::max_hitpoints[unit_id] / 10;
		return target;
	}

	// Hooked to clear placement area (and maybe fix the AI stacked Droleth bug). -Pr0nogo
	void orders_drone_build(Unit* builder) // 0045E090
	{
		Sprite* builder_sprite = builder->sprite;
		Image* underlay = builder_sprite->images.tail;
		if (builder_sprite->position.x != builder->order_target.pt.x
		||  builder_sprite->position.y != builder->order_target.pt.y)
		{
			// Change shadow pos
			if (underlay->palette_type == 10 && underlay->vertical_offset != 7)
			{
				underlay->flags |= ImageFlags::Redraw;
				underlay->vertical_offset = 7;
			}
			helper::refresh_sprite_data(builder->sprite);
			Order* order_head = builder->order_queue_head;
			if (order_head)
			{
				helper::insert_order_before(builder, order_head, OrderId::ResetCollision1, 0, 0, 0, 228);
			}
			else
			{
				helper::perform_another_order(OrderId::ResetCollision1, 0, builder, 228, {}, 0);
			}
			helper::force_order_done(builder);
		}
		else
		{
			u32 building_id = builder->build_queue[builder->build_queue_slot];
			u16 pos_x = builder_sprite->position.x;
			u16 pos_y = builder_sprite->position.y;
			u32 dims_x = units_dat::building_dimensions[building_id].x;
			u32 dims_y = units_dat::building_dimensions[building_id].y;
			u32 tile_y = (pos_y - (dims_y >> 1)) / 32;
			if (helper::can_build_on_tile_location(builder, builder->player, (pos_x - (dims_x >> 1)) / 32, tile_y, building_id, 0, 0, 1, 1) // This is probably WRONG
			|| builder_sprite->position.x != builder->order_target.pt.x
			|| builder_sprite->position.y != builder->order_target.pt.y)
			{
				helper::play_worker_error_sound(builder, 856);
				if (underlay->palette_type == 10 && underlay->vertical_offset != 7)
				{
					underlay->flags |= ImageFlags::Redraw;
					underlay->vertical_offset = 7;
				}
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
				return;
			}

			u32 player = builder->player;
			g_ai_minerals_reserve[player] = units_dat::mineral_cost[building_id];
			g_ai_vespene_reserve[player] = units_dat::vespene_cost[building_id];

			// @Todo: Tidy this into not being inlined
			if (g_game_minerals[player] < g_ai_minerals_reserve[player])
			{
				helper::display_error_message_id_with_sfx(player, 850, *g_console_index + 147);
				helper::set_shadow_y(builder->sprite, 7);
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
			}
			if (g_game_vespene[player] < g_ai_vespene_reserve[player])
			{
				helper::display_error_message_id_with_sfx(player, 851, *g_console_index + 150);
				helper::set_shadow_y(builder->sprite, 7);
				helper::refresh_sprite_data(builder->sprite);
				helper::order_immediate(builder, OrderId::ResetCollision1);
				helper::force_order_done(builder);
			}

			helper::subtract_queue_cost(builder->player);
			Unit* powerup = helper::get_unit_powerup(builder);
			if (powerup)
			{
				helper::drop_powerup(builder);
			}
			if (building_id == UnitId::ZergExcisant)
			{
				helper::droleth_build_excisant(builder);
			}
			else
			{
				drone_morph_standard(builder, building_id);
			}
			if (powerup)
			{
				helper::move_unit(powerup, powerup->powerup_origin.x, powerup->powerup_origin.y);
			}
		}
	}

	// Hooked to clear placement area (and maybe fix the AI stacked Droleth bug). -Pr0nogo
	void orders_drone_land(Unit* unit) // 004E9AA0
	{
		if (unit->unit_ai)
		{
			unit->status |= UnitStatus::IsGathering;
		}
		u32 order_state = unit->main_order_state;
		if (unit->main_order_state)
		{
			order_state -= 1;
			if (order_state)
			{
				if (order_state == 1)
				{
					Sprite* unit_sprite = unit->sprite;
					if (unit->move_target.pt.x == unit_sprite->position.x && unit->move_target.pt.y == unit_sprite->position.y)
					{
						u32 unmovable = ((unit->status & UnitStatus::Unmovable) != 0) + 1;
						if (((unit->status & UnitStatus::Unmovable) != 0) != -1)
						{
							Image* underlay = unit_sprite->images.tail;
							if (underlay->palette_type == 10)
							{
								if (underlay->vertical_offset)
								{
									underlay->flags |= ImageFlags::Redraw;
									underlay->vertical_offset = 0;
								}
							}
							if (unit->sprite->images.tail->palette_type == 10) // Deliberately not using underlay var
							{
								unit->sprite->images.tail->flags &= 0xFBu;
							}
							if (unmovable == 2)
							{
								helper::play_building_land_sound(unit);
								Order* order_queue = unit->order_queue_head;
								if (order_queue)
								{
									if (order_queue->order_id == OrderId::DroneBuild)
									{
										helper::remove_order_from_unit_queue(order_queue, unit);
									}
								}
								underlay = unit->sprite->images.tail;
								if (underlay->palette_type == 10 && underlay->vertical_offset != 7)
								{
									underlay->flags |= ImageFlags::Redraw;
									underlay->vertical_offset = 7;
								}
								helper::refresh_sprite_data(unit->sprite);
								helper::order_immediate(unit, OrderId::ResetCollision1);
								helper::force_order_done(unit);
							}
							else
							{
								Order* order_queue = unit->order_queue_head;
								if (order_queue && order_queue->order_id == OrderId::DroneBuild)
								{
									helper::move_unit(unit, unit->order_target.pt.x, unit->order_target.pt.y);
								}
								else
								{
									underlay = unit->sprite->images.tail;
									if (underlay->palette_type == 10 && underlay->vertical_offset != 7)
									{
										underlay->flags |= ImageFlags::Redraw;
										underlay->vertical_offset = 7;
									}
									helper::refresh_sprite_data(unit->sprite);
									helper::order_immediate(unit, OrderId::ResetCollision1);
								}
								*control_under_mouse = 0;
								*control_under_mouse_value = 0;
								*g_force_button_refresh = 1;
								portrait_redraw[0] = 1;
								*status_screen_redraw = 1;
								helper::force_order_done(unit);
							}
						}
					}
				}
				return;
			}
			if (unit->move_target.pt.x == unit->sprite->position.x && unit->move_target.pt.y == unit->sprite->position.y)
			{
				u32 unmovable = ((unit->status & UnitStatus::Unmovable) != 0) + 1;
				if (((unit->status & UnitStatus::Unmovable) != 0) != -1)
				{
					u32 queue_slot_id = unit->build_queue[unit->build_queue_slot];
					u32 can_build = helper::can_build_on_tile_location(unit, unit->player,  (unit->order_target.pt.x - units_dat::building_dimensions[queue_slot_id].x / 2) / 32,
																							(unit->order_target.pt.y - units_dat::building_dimensions[queue_slot_id].y / 2) / 32,
																							 queue_slot_id, 0, 0, 1, 1);
					if (queue_slot_id != UnitId::ZergExcisant || unmovable != 1)
					{
						if (!can_build && unmovable != 2)
						{
							if (helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128))
							{
								if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y) || unit->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) // Was BYTE1(unit->status)
								{
									Image* underlay = unit->sprite->images.tail;
									if (underlay->palette_type == 10)
									{
										underlay->flags |= ImageFlags::DontUpdate;
									}
									u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
									unit->main_order_state = 2;
									unit->status = status;
								}
								return;
							}
						}
					}
					else if (!can_build)
					{
						Unit* unit_at_tile = helper::find_unit_at_tile(unit->order_target.pt.y, unit->order_target.pt.x, 188);
						if (unit_at_tile && helper::is_target_within_min_range(unit, 32, unit_at_tile))
						{
							u32 builder_id = unit->id;
							unit->status &= 0xFFEFFFFF;
							unit->sprite->elevation = units_dat::elevation[builder_id] + 1;
							helper::order_targeting_nothing(0x96u, unit, 0);
							helper::order_targeting_nothing(units_dat::return_to_idle_order[unit->id], unit, 0);
							if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
							{
								u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
								unit->main_order_state = 2;
								unit->status = status;
							}
							return;
						}
						if (helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 128))
						{
							if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y) || unit->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) // Was BYTE1(unit->status)
							{
								Image* underlay = unit->sprite->images.tail;
								if (underlay->palette_type == 10)
								{
									underlay->flags |= ImageFlags::DontUpdate;
								}
								u32 status = unit->status | UnitStatus::CanNotReceiveOrders;
								unit->main_order_state = 2;
								unit->status = status;
							}
							return;
						}
						helper::play_building_land_sound(unit);
						Order* order_queue = unit->order_queue_head;
						if (order_queue)
						{
							if (order_queue->order_id == OrderId::DroneBuild)
							{
								helper::remove_order_from_unit_queue(order_queue, unit);
							}
						}
						helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
						return;
					}
					helper::play_building_land_sound(unit);
					Order* order_queue = unit->order_queue_head;
					if (order_queue)
					{
						if (order_queue->order_id == OrderId::DroneBuild)
						{
							helper::remove_order_from_unit_queue(order_queue, unit);
						}
					}
					helper::issue_order_targeting_nothing(unit, units_dat::return_to_idle_order[unit->id]);
					return;
				}
			}
		}
		else
		{
			u32 tgt_x = unit->order_target.pt.x;
			u32 tgt_y = unit->order_target.pt.y - 7;
			bool move_target = helper::set_move_target_xy(unit, tgt_x, tgt_y);
			if (tgt_x != unit->next_target_waypoint.x || tgt_y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = tgt_x;
				unit->next_target_waypoint.y = tgt_y;
			}
			if (move_target)
			{
				unit->main_order_state = 1;
			}
		}
	}

	// Hooked to troubleshoot multi-building select queuing building mutation orders. -Pr0nogo
	void orders_morph2(Unit* unit) // 0045D0D0
	{
		u32 player = unit->player;
		u32 queue_id = unit->build_queue[unit->build_queue_slot];
		g_ai_minerals_reserve[player] = units_dat::mineral_cost[queue_id];
		g_ai_vespene_reserve[player] = units_dat::vespene_cost[queue_id];
		if (!helper::has_supplies_for_unit(player, queue_id, 1))
		{
			helper::order(0, unit, units_dat::return_to_idle_order[unit->id], {}, 0, UnitId::None);
			return;
		}

		s32 err_string_id;
		s32 err_reason;
		if (g_game_minerals[player] < g_ai_minerals_reserve[player])
		{
			err_string_id = 850;
			err_reason = *(s32*)g_console_index + 0x93;
			helper::display_error_message_id_with_sfx(player, err_string_id, err_reason);
			helper::order(0, unit, units_dat::return_to_idle_order[unit->id], {}, 0, UnitId::None);
			return;
		}
		if (g_game_vespene[player] < g_ai_vespene_reserve[player])
		{
			err_string_id = 851;
			err_reason = *(s32*)g_console_index + 0x96;
			helper::display_error_message_id_with_sfx(player, err_string_id, err_reason);
			helper::order(0, unit, units_dat::return_to_idle_order[unit->id], {}, 0, UnitId::None);
			return;
		}

		g_game_minerals[player] -= g_ai_minerals_reserve[player];
		g_game_vespene[player] -= g_ai_vespene_reserve[player];
		unit->status = unit->status & 0xFFFFFFFE;
		if (!unit->is_frozen() || units_dat::base_property[unit->id] & UnitProperty::Building)
		{
			unit->current_buttonset = 236;
		}

		helper::init_building(unit, 1);
		u32 order_id = unit->main_order_id;
		unit->remaining_build_time = units_dat::time_cost[queue_id];
		unit->user_action_flags |= 1u;
		if (order_id)
		{
			while (1)
			{
				Order* order_tail = unit->order_queue_tail;
				if (!order_tail)
				{
					break;
				}
				u32 tail_order_id = order_tail->order_id;
				if (!orders_dat::can_be_interrupted[tail_order_id] && tail_order_id != OrderId::ZergBuildSelf)
				{
					break;
				}
				helper::remove_order_from_unit_queue(order_tail, unit);
			}
			helper::perform_another_order(OrderId::ZergBuildSelf, 0, unit, UnitId::None, {}, 0);
			unit = unit;
		}
		helper::orders_nothing_2(unit);
		for (Image* unit_image = unit->sprite->images.head; unit_image; unit_image = unit_image->link.next)
		{
			play_image_iscript(unit_image, 13);
		}
		scbw::refresh_console();
	}
}