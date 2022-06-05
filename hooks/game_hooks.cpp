#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "hooks/interface/uifeatures/screen_hotkeys.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/interface/hpbars/hpbars.h"
#include "Utils/captivating_claws.h"
#include "hooks/cosmonarchy_finder.h"
#include "globals.h"
#include "target_related.h"
#include "hooks/orders/ai/ai_orders.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_fire.h"
#include "logger.h"
#include "limits/sfx_extender.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "globals.h"
#include "unit_search.h"
#include "player_color.h"
#include "unit_ai.h"
#include "tactical_ai.h"
#include "scenario.h"
#include "trigger_system.h"
#include "base_layouts.h"
#include "token_parser.h"
#include "math_utils.h"
#include "graphics/Bitmap.h"
#include "data_ext.h"

const u32 hyperTrigger = 0x006509A0;

#include "SCBW\UnitFinder.h"
void shaman_nanite_field_proc(Unit* shaman, u32 heal, Unit* target)
{
	if (target->player != shaman->player)
	{
		if (!scbw::is_allied_to(target->player, shaman->player))
		{
			return;
		}
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
	{
		return;
	}
	if (target->stasis_timer)
	{
		return;
	}
	if (target == shaman)
	{
		return;
	}
	u32 dist = scbw::get_distance_fast(target->position, shaman->position);
	if (dist > 128) {
		return;
	}

	bool reqHeal = false;
	if (target->get_current_hp_ingame() < target->get_max_hp_ingame())
	{
		reqHeal = true;
		u32 overlayImageId = ImageId::ShamanNaniteField;
		Unit* tgt = nullptr;
		if (target->subunit != nullptr)
		{
			tgt = target->subunit;
		}
		else
		{
			tgt = target;
		}
		(tgt->sprite)->create_top_overlay(overlayImageId, 0, 0, 0);
	}

	u32 new_heal = heal;
	if (dist <= 32) {
		new_heal *= 2.5;
	}
	else if (dist <= 64) {
		new_heal *= 2;
	}
	else if (dist <= 96) {
		new_heal *= 1.5;
	}
	target->heal(new_heal * 256, shaman);
}

namespace hooks
{
	void tick_unit(Unit* unit);
	static bool game_inited;
	void next_ui_frame() {
		if (!game_inited) {
			game_inited = true;
			/*
			Parser parser = {};
			parser.at = ...;
			while (parser_get_word(&parser)) {
				if (parser_eq(&parser, "-map")) {
					Token token = parser_get_string(&parser);
					helper::load_scenario_singleplayer(token.data, 0, "Some Error Message", "Player Name", 6);
				}
			}
			*/
		}
		tick_trigger_system(false);
	}

	/// This hook is called every frame
	bool next_frame()
	{
		tick_trigger_system(true);
		tick_base_layouts();
		// This block is executed once every game
		if (*elapsedTimeFrames == 0)
		{
#ifdef CM_DEBUG
			helper::play_sound_global(SoundId::Misc_Buzz_wav_1);
			scbw::print("\tGPTP was compiled in debug!\n\tThis will make the game run slower.", GameTextColor::Yellow);
#endif
			hooks::initalize_screen_locations();
			hooks::clear_actions();
			globals.reset();
			reset_ext_variables();

			for (s32 i = 0; i < 8; i++)
			{
				scbw::set_upgrade_level(i, UpgradeId::VentralSacs, 1); // Zerg AI may require this to function properly
			}
		}

		if (*elapsedTimeFrames % 2 == 0)
		{
			for (u32 i = 0; i < 16; ++i)
			{
				Ping* ping = ping_array + i;
				if (ping->timer < 8)
				{
					ping->timer++;
				}
			}
		}

		// Map Features
		scbw::set_ingame_loop_state(true);
		if (*elapsedTimeFrames == 1) {
			load_player_colors();
		}
		if (*elapsedTimeFrames == 0 || *elapsedTimeFrames == 24) {
			load_player_names();
		}
		scbw::set_ingame_loop_state(false);

		if (*elapsedTimeFrames == 2) {
			u32 local_flags = unit_deaths[UnitId::ResourceVespeneGeyser].player[*LOCAL_NATION_ID];
			if (!(local_flags & VespeneDeaths_DisableQuasifog)) {
				// Default maps to "semi-explored", from DarkenedFantasies
				for (s32 i = 0; i < 4092; i++) {
					if (fog_of_war_mask->unexplored[i] < 5) {
						fog_of_war_mask->unexplored[i] = 5;
					}
				}
				scbw::refresh_screen();
			}

			// Check deaths
			u8 reveal_flag = 0x00;
			bool do_ping[PLAYABLE_PLAYER_COUNT] = {};
			for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id)
			{
				u32 flags = unit_deaths[UnitId::ResourceVespeneGeyser].player[player_id];
				if (flags & VespeneDeaths_Reveal)
				{
					reveal_flag |= 1 << player_id;
				}
				if (flags & VespeneDeaths_Ping)
				{
					do_ping[player_id] = true;
				}
			}

			// Explores the entire map, requires a trigger that sets 1 death of Vespene Geyser for players in question
			if (reveal_flag)
			{
				// Explore map
				/*
				for (u32 x = 0; x < mapTileSize->width; x++)
				{
					for (u32 y = 0; y < mapTileSize->height; y++)
					{
						ActiveTile& tile = scbw::get_active_tile_at(x * 32, y * 32);
						tile.exploredFlags &= ~reveal_flag;
					}
				}
				*/

				// Reveal resources
				for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
				{
					if (unit->player == 11 && units_dat::BaseProperty[unit->id] & UnitProperty::Building)
					{
						unit->sprite->visibilityFlags |= reveal_flag;
					}
				}
			}

			for (u32 target = 0; target < PLAYABLE_PLAYER_COUNT; ++target)
			{
				if (do_ping[target])
				{
					u32 color = 0; // red
					if (scbw::is_allied_to(*LOCAL_NATION_ID, target))
					{
						color = 8; // green
					}

					scbw::minimap_ping(startPositions[target].x, startPositions[target].y, color, *LOCAL_NATION_ID);
				}
			}
		}

		options_list->speed = 6; // Lock game speed at fastest
		if (!scbw::is_game_paused()) // If the game is not paused
		{
			*ai_update_cooldown = 0;
			for (u32 i = 0; i < 8; ++i) {
				raceSupply[RaceId::Terran].provided[i] = 60000;
			}

			update_search_index();
			scbw::set_ingame_loop_state(true); // Needed for scbw::random() to work
			graphics::reset_all_graphics();
			hooks::update_psi_field_providers();
			*(s32*)hyperTrigger = 0; // Single frame hypertriggers

			// Scenario Features
			if (*elapsedTimeFrames == 1) {
				init_scenario();
			}
			if (*elapsedTimeFrames >= 1) {
				tick_scenario();
			}

			hooks::handle_screen_hotkeys();
			scbw::refresh_screen();

			// Needed for extended tooltips
			static bool lastShift = 0;
			bool newShift = GetAsyncKeyState(VK_SHIFT) & 0x1;
			if (lastShift != newShift)
			{
				scbw::refresh_console();
			}
			lastShift = newShift;

			globals.frame_hook();
			update_search_index_effects();
			update_ext_variables();

			// Loop through bullets
			for (Bullet* bullet = *first_bullet; bullet; bullet = bullet->next)
			{
				if (bullet->weaponId == WeaponId::KalkaliskViscousAcid)
				{
					if (bullet->order_signal & 0x80)
					{
						bullet->order_signal &= ~0x80;
						globals.remove_multihit_bullet(bullet);
					}
				}
			}

			// Loop through all visible units in the game.
			for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
			{
				tick_unit(unit);
			}
			scbw::set_ingame_loop_state(false);
		}

		if (*activePortraitUnit)
		{
			// TODO: update to work with multi building select
			Unit* selected = *activePortraitUnit;

			// scbw::printf("%u", selected->main_order_id);

			if (selected
				&& selected->id == UnitId::ZergNydusCanal
				&& selected->building.nydus_exit
				&& (scbw::is_allied_to(selected->player, *LOCAL_HUMAN_ID) || helper::get_detecting_players(selected) & (1 << *LOCAL_HUMAN_ID)))
			{
				Unit* exit = selected->building.nydus_exit;
				graphics::draw_line(selected->get_x(), selected->get_y(), exit->get_x(), exit->get_y(), graphics::GREEN, graphics::ON_MAP);
			}
		}
		return true;
	}

	void tick_unit(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		tactical_ai(unit, unitExt);

		u32 creep_spreader_id = scbw::unit_get_creep_spreader_id(unit);
		if (creep_spreader_id != UnitId::None && unit->player < PlayerLimit) {
			Box32 creep_area = {};
			helper::get_creep_spread_area(creep_spreader_id, unit->position.x, unit->position.y, unit->status & UnitStatus::Completed, &creep_area);
			for (u32 y = creep_area.top; y <= creep_area.bottom; ++y) {
				for (u32 x = creep_area.left; x <= creep_area.right; ++x) {
					s32 delta_x = unit->position.x - (x * 32 + 16);
					s32 delta_y = unit->position.y - (y * 32 + 16);
					if (delta_x * delta_x * 100 + delta_y * delta_y * 256 <= 10240000) {
						ActiveTile* tile = scbw::get_active_tile_at_tile(x, y);
						if (tile->has_creep || tile->temporary_creep) {
							TileExt* tile_ext = get_tile_ext_at_tile(x, y);
							tile_ext->creep_owners |= 1 << unit->player;
						}
					}
				}
			}
		}

		if ((unit->id == UnitId::ProtossPariah || unit->id == UnitId::SharedStriga) && unit->order_signal & 0x20)
		{
			unit->order_signal &= ~0x20;
			globals.remove_unit_from_pariah_table(unit, true);
		}
		//hooks::ShowHealthBar(unit);

		if (unit->unit_ai) {
			if (unit->main_order_id == OrderId::ComputerAI) {
				switch (unit->unit_ai->type) {
					case AiType::Building:
					case AiType::Worker: {
						unit->main_order_timer = 0;
					} break;
				}
			}
		}

		switch (unit->id)
		{
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			{
				// Fixes an issue where geyser would freeze, when upgrading the geyser while a harvester was mining
				if (!(unit->status & UnitStatus::Completed)) {
					helper::remove_harvesters(unit);
				}
			} break;
			case UnitId::TerranTalos:
			{
				if (unit->main_order_id == OrderId::TalosSalvage) {
					Unit* target = unit->order_target.unit;
					if (target) {
						if (unit->is_target_within_min_range(target, unit->get_max_weapon_range(unit->get_ground_weapon()))) {
							UnitExt* targetExt = globals.get_unit_ext(target);
							targetExt->timers[TimerSalvageStunTimer] = 5 * 24;
							
							u32 time = units_dat::TimeCost[target->id] / 2;
							targetExt->salvage_time += 1;
							
							targetExt->salvage_minerals += (f32)units_dat::MineralCost[target->id] / time;
							while (targetExt->salvage_minerals > 1) {
								targetExt->salvage_minerals -= 1;
								resources->minerals[unit->player] += 1;
							}
							
							targetExt->salvage_gas += (f32)units_dat::GasCost[target->id] / time;
							while (targetExt->salvage_gas > 1) {
								targetExt->salvage_gas -= 1;
								resources->gas[unit->player] += 1;
							}
							
							if (targetExt->salvage_time >= time) {
								resources->minerals[unit->player] += ceilf(targetExt->salvage_minerals);
								resources->gas[unit->player] += ceilf(targetExt->salvage_gas);
								target->remove();
							}

							if (!(unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving)
								&& unitExt->timers[TimerTalosAnimationSubtimer] == 0) {

								u32 width  = units_dat::BuildingDimensions[target->id].x * 0.75;
								u32 height = units_dat::BuildingDimensions[target->id].y * 0.75;

								s32 x_offset = scbw::random() % width  - width  / 2;
								s32 y_offset = scbw::random() % height - height / 2;

								x_offset = MathUtils::clamp(x_offset, -127, 128);
								y_offset = MathUtils::clamp(y_offset, -127, 128);

								target->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
								unitExt->timers[TimerTalosAnimationSubtimer] = scbw::rand_between(3, 7);
							}

							if (unitExt->values[ValueTalosAnimationTimer] == 0
							||  unitExt->values[ValueTalosAnimationTimer] % 128 == 0) {
								Point16 pos = target->sprite->position;
								switch (scbw::random() % 4) {
									case 0: {
										pos.x -= units_dat::UnitBounds[target->id].left * 0.75;
										pos.y -= units_dat::UnitBounds[target->id].top * 0.75;
									} break;
									case 1: {
										pos.x += units_dat::UnitBounds[target->id].right * 0.75;
										pos.y -= units_dat::UnitBounds[target->id].top * 0.75;
									} break;
									case 2: {
										pos.x -= units_dat::UnitBounds[target->id].left * 0.75;
										pos.y += units_dat::UnitBounds[target->id].bottom * 0.75;
									} break;
									case 3: {
										pos.x += units_dat::UnitBounds[target->id].right * 0.75;
										pos.y += units_dat::UnitBounds[target->id].bottom * 0.75;
									} break;
								}							
								helper::set_move_target_xy(pos.x, pos.y, unit);
							}
							unitExt->values[ValueTalosAnimationTimer] += 1;
						} else {
							unitExt->values[ValueTalosAnimationTimer] = 0;
						}
					}
				}
			} break;
			case UnitId::ZergZoryusthaleth:
			{
				// Zoryusthaleth - Swarming Omen
				if (unit->get_current_hp_ingame() < unit->get_max_hp_ingame() / 2)
				{
					unitExt->timers[TimerSwarmingOmen] = 15 * 24;
				}
				// Zoryusthaleth - Reconstitution
				if (unitExt->timers[TimerReconstitutionCooldown] == 0)
				{
					u32 radius = 5 * 32;
					EntryArray corpses = corpse_search_radial(unit->position, radius);
					for (u32 i = 0; i < corpses.size; ++i)
					{
						Thingy* corpse = corpses.corpses[i];
						if (unit->energy < 5 * 256)
						{
							break;
						}
						if (corpse->sprite->flags & SpriteFlags::Flag04)
						{
							continue;
						}
						unit->spend_unit_energy(5 * 256);

						EntryArray targets = unit_search_radial(unit->position, radius);
						unit_search_filter(&targets, unit, FilterAlly);
						for (u32 i = 0; i < targets.size; ++i) {
							Unit* target = targets.units[i];
							target->heal(ZoryusthalethReconstitutionHealing, unit);
							//scbw::create_and_init_thingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
						}
						scbw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= SpriteFlags::Flag04;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
						unitExt->timers[TimerReconstitutionCooldown] = 1 * 24;
						break;
					}
				}
			} break;
			case UnitId::ZergSovroleth:
			{
				// Update Speed
				unit->update_speed();

				// Corpse Cocoon
				EntryArray corpses = corpse_search_rect({unit->get_left(), unit->get_top(), unit->get_right(), unit->get_bottom()});
				for (u32 i = 0; i < corpses.size; ++i) {
					Thingy* corpse = corpses.corpses[i];
					if (!(corpse->sprite->flags & SpriteFlags::Flag04)) {
						Unit* egg = scbw::create_unit_in_game(UnitId::ZergEgg, unit->player, corpse->sprite->position.x, corpse->sprite->position.y, 0, false);
						if (egg) {
							egg->sprite->play_iscript_animation(IscriptAnimation::Walking, true);
							egg->build_queue[egg->build_queue_slot] = UnitId::ZergSovroleth;

							UnitExt* eggExt = globals.get_unit_ext(egg);
							eggExt->timers[TimerCorpseCocoonHatch] = 2 * 24;
						}
						corpse->sprite->flags |= SpriteFlags::Flag04;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
					}
				}
			} break;
			case UnitId::ZergEgg:
			case UnitId::ZergCocoon:
			case UnitId::ZergQuazilisk:
			{
				// Gimmick to allow morph cancellation animations to play correctly
				if (unitExt->timers[TimerQuaziliskCancel])
				{
					if (unit->sprite->main_graphic->animation != IscriptAnimation::CastSpell)
					{
						unit->play_iscript_animation(IscriptAnimation::CastSpell);
					}
				}
				if ((unit->id == UnitId::ZergEgg && !scbw::is_larval_egg(unit)) || unit->id == UnitId::ZergCocoon) {
					if (scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y).has_creep
						&& !unit->get_overlay(ImageId::CreepMorphOverlay)) {
						unit->sprite->create_underlay(ImageId::CreepMorphOverlay, 0, 2);
					}
				}
				if (unitExt->timers[TimerCorpseCocoonHatch] == 1 + 19) {
					unit->play_iscript_animation(IscriptAnimation::GndAttkInit);
				}
				if (unitExt->timers[TimerCorpseCocoonHatch] == 1) {
					helper::replace_unit_with_type(unit, UnitId::ZergSovroleth);
					scbw::give_ai_ex(unit, true);
					helper::play_morphing_complete_sound(unit);
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					helper::init_building(unit, 1);
					unitExt->values[ValueSpecialSovrolethBirth] = true;
					unit->order_computer(OrderId::ZergBirth);
				}
			} break;
			case UnitId::ProtossStarSovereign:
			{
				// Star Sovereign - Intervention Missiles
				if (unitExt->timers[TimerStarSovereignInterventionMissiles] == 0)
				{
					auto r = 5 * 32;
					auto n = cosmonarchy_finder_match(unit, Flags::Enemy | Flags::FrontalArc, {});
					scbw::UnitFinder unitsInFinder(unit->get_x() - r, unit->get_y() - r,
						unit->position.x + r, unit->position.y + r);
					bool enemiesDetected = false;
					for (s32 i = 0; i < unitsInFinder.get_unit_count(); i++)
					{
						auto picked_unit = unitsInFinder.get_unit(i);
						if (n(picked_unit))
						{
							enemiesDetected = true;
							break;
						}
					}
					if (enemiesDetected)
					{
						unitExt->timers[TimerStarSovereignInterventionMissiles] = weapons_dat::Cooldown[WeaponId::StarSovereignInterventionMissiles];
						for (s32 arc = 0; arc < 6; arc++)
						{
							const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlaySpecialPtr, unit->sprite->main_graphic->id));
							u8 frameDirection = unit->sprite->main_graphic->direction;
							Point8 offset = loFile->get_offset(frameDirection, 5 - arc);
							if (unit->sprite->main_graphic->flags & ImageFlags::Mirrored)
							{
								offset.x = -offset.x;
							}
							auto X = unit->get_x() + offset.x;
							auto Y = unit->get_y() + offset.y;
							auto target_unit = unit->order_target.unit;
							auto target_pos = unit->order_target.pt;
							auto offset_direction = -40 + arc * 16;
							auto maxRange = weapons_dat::MaxRange[WeaponId::StarSovereignInterventionMissiles];
							auto polar_x = unit->get_x() + scbw::get_polar_x(maxRange, scbw::dir_saturate(unit->current_direction_1, offset_direction));
							auto polar_y = unit->get_y() + scbw::get_polar_y(maxRange, scbw::dir_saturate(unit->current_direction_1, offset_direction));
							unit->order_target.unit = nullptr;
							unit->order_target.pt.x = polar_x;
							unit->order_target.pt.y = polar_y;
							helper::create_bullet(WeaponId::StarSovereignInterventionMissiles, unit, X, Y, unit->player, scbw::dir_saturate(unit->current_direction_1, offset_direction));
							helper::play_sound_from_unit(80, unit, 1, 0);
							unit->order_target.unit = target_unit;
							unit->order_target.pt = target_pos;
						}
					}
				}

				// Star Sovereign - Grief of All Gods
				if (unitExt->timers[TimerPlanetCrackerChannel] == 1)
				{
					globals.create_effect(EffectId_StarSovereignTheGriefOfAllGods, 360, {unit->position.x, unit->position.y}, 8 * 32, -1, unit->sprite->elevationLevel, unit->player, unit, unit);
				}

				// blast effects
				u32 player = unit->player;
				u16 X = unit->position.x;
				u16 Y = unit->position.y;
				if (unit->order_signal & 8)
				{
					unit->order_signal &= ~8;
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y + 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y + 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y - 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y - 32, player);
				}
				else if (unit->order_signal & 16)
				{
					unit->order_signal &= ~16;
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y - 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y + 80, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 80, player);
				}
				else if (unit->order_signal & 32)
				{
					unit->order_signal &= ~32;
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y + 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 144, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 0, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 208, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 208, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 208, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 208, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 208, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 208, player);
				}
				else if (unit->order_signal & 64)
				{
					unit->order_signal &= ~64;
				}
			} break;
			case UnitId::TerranApostle:
			{
				// Apostle - Lazarus Agent cast
				if (scbw::get_generic_rust_value(unit, RustId::CastLazarusAgent) == 1)
				{
					scbw::decloak_proc(unit);
					scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 0);
					// Cast Lazarus Agent						
					unit->spend_unit_energy(techdata_dat::EnergyCost[TechId::ApostleLazarusAgent] * 256);
					auto r = 4 * 32;
					auto n = cosmonarchy_finder_match(unit, Flags::NoBuilding | Flags::Ally | Flags::IncludingSelf | Flags::NoTrap | Flags::NoHallucination | Flags::NoEgg | Flags::NoLarva | Flags::NoHangar, {});
					scbw::UnitFinder unitsInSplash(unit->get_x() - r, unit->get_y() - r,
						unit->position.x + r, unit->position.y + r);
					for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
					{
						auto picked_unit = unitsInSplash.get_unit(i);
						if (n(picked_unit))
						{
							// Apply Lazarus Agent
							UnitExt* pickedUnitExt = globals.get_unit_ext(picked_unit);
							pickedUnitExt->values[ValueLazarusAgent] += 1;
							picked_unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
							helper::play_sound_from_unit(SoundId::LazarusAgentCast, unit, 1, 0);
						}
					}
				}
			} break;
			case UnitId::ProtossLanifect:
			{
				// Disruption Field - Lanifect ability
				if (scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField))
				{
					if (!unitExt->values[ValueDisruptionFieldOn])
					{
						unitExt->values[ValueDisruptionFieldOn] = 1;
						globals.create_effect(EffectId_DisruptionField, -1, {unit->position.x, unit->position.y}, 0, SpriteId::Disruption_Web, 0, unit->player, unit, unit);
					}
					else if (unit->is_frozen())
					{
						scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
					}
					scbw::decloak_proc(unit);
				}
				else
				{
					unitExt->values[ValueDisruptionFieldOn] = 0;
				}
			} break;
			case UnitId::TerranMadcap:
			{
				// Madcap Cabeirus Feeders - subtract stacks while moving
				if (unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving)
				{
					if (unitExt->values[ValueMadcapStacks] > 0)
					{
						unitExt->values[ValueMadcapStacks]--;
					}
				}
			} break;
			case UnitId::TerranHeracles:
			{
				// Heracles Nemean Reactors - remove stacks when out of combat
				if (unitExt->values[ValueNemeanReactorsStack] > 0
					&& unitExt->timers[TimerNemeanReactors] <= 1)
				{
					unitExt->values[ValueNemeanReactorsStack] = 0;
				}
			} break;
			case UnitId::TerranCuirass:
			{
				if (unitExt->values[ValueIsSelfReviving])
				{
					if (unit->hitpoints + CuirassSelfHeal >= unit->get_max_hp_raw())
					{
						unitExt->values[ValueIsSelfReviving] = 0;
						unit->hitpoints = unit->get_max_hp_raw();
						scbw::restore_func(unit);
						unit->order_computer(OrderId::Stop);
						unit->status &= ~(UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
						helper::play_sound_from_unit(SoundId::Terran_GOLIATH_TGoWht03_WAV, unit, 1, 0);
					}
					else
					{
						unit->heal(CuirassSelfHeal);
						if (!unit->sprite->get_overlay(ImageId::StonewallProtocolOverlay))
						{
							unit->sprite->create_top_overlay(ImageId::StonewallProtocolOverlay, -3, -8);
						}
					}
				}
			}
			case UnitId::SharedStriga:
			case UnitId::ProtossLegionnaire:
			{
				// Striga - Transient Travels
				// Legionnaire - Blade Vortex
				if (unit->order_target.unit != nullptr
					&& (unit->main_order_id == OrderId::Attack1 || unit->main_order_id == OrderId::AttackUnit))
				{
					Unit* target = unit->order_target.unit;
					u32 distance = unit->get_distance_to_target(target);
					bool doTeleport = false;
					u32 imageId;
					Point16 position;

					if (unit->id == UnitId::ProtossLegionnaire)
					{
						if (distance > 15 && distance <= 3 * 32 && scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0)
						{
							doTeleport = true;
							imageId = ImageId::LegionnaireTeleportEffect;
							position = target->sprite->position;
						}
					}
					else if (unit->id == UnitId::SharedStriga)
					{
						if (distance > 3.5 * 32 && distance <= 5 * 32 && scbw::is_unit_isolated(target))
						{
							doTeleport = true;
							imageId = ImageId::LegionnaireTeleportEffect;
							Vector32 diff = scbw::get_vector_between_points(target->sprite->position, unit->sprite->position);
							Vector32f displacement = scbw::get_normalized_vector(diff);
							displacement.x *= 3 * 32;
							displacement.y *= 3 * 32;
							position = target->position;
							position.x += displacement.x;
							position.y += displacement.y;
						}
					}
					if (doTeleport)
					{
						position = scbw::clamp_point(position);
						bool teleport = scbw::teleport(unit, position, 0, true);
						if (!unit->get_overlay(imageId) && teleport)
						{
							unit->sprite->create_top_overlay(imageId);
						}
					}
				}
			} break;
			case UnitId::TerranPaladin:
			{
				// Paladin - Skyfall
				if (unitExt->timers[TimerSkyfall] == 1
					&& !unit->is_frozen())
				{
					u8 missile = unitExt->values[ValueSkyfallStacks];
					unitExt->values[ValueSkyfallStacks]++;
					if (missile == 20)
					{
						unitExt->values[ValueSkyfallStacks] = 0;
					}
					else
					{
						unitExt->timers[TimerSkyfall] = 2 + 1;
						u32 tgt_x = unitExt->values[ValueSkyfallX];
						u32 tgt_y = unitExt->values[ValueSkyfallY];

						auto column = missile / 4;
						auto row = missile % 4;
						auto distance = scbw::get_distance_fast(unit->get_x(), unit->get_y(), tgt_x, tgt_y);
						auto angle = scbw::bw_angle_to_normal(scbw::get_angle(unit->get_x(), unit->get_y(), tgt_x, tgt_y));
						Vector32 rotation_pt = Vector32{ 0,0 };
						rotation_pt.x += 16 + (row - 2) * 32;
						rotation_pt.y -= (column - 2) * 32;
						auto product = scbw::rotate_point(Vector32{ 0,0 }, rotation_pt, angle);
						Point16 missile_target;
						missile_target.x = tgt_x + product.x;
						missile_target.y = tgt_y + product.y;

						s32 forwardOffset = weapons_dat::ForwardOffset[10];
						auto source_x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
						auto source_y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1) - weapons_dat::VerticalOffset[10];
						unit->order_target.unit = nullptr;
						unit->order_target.pt = missile_target;
						auto range = 64;

						auto targ = scbw::UnitFinder::get_nearest_target(
							missile_target.x - range,
							missile_target.y - range,
							missile_target.x + range,
							missile_target.y + range,
							unit,
							[&](Unit* target)
							{
								if (target == unit)
								{
									return false;
								}
								if (scbw::is_allied_to(unit->player, target->player))
								{
									return false;
								}
								if (target->status & UnitStatus::Invincible)
								{
									return false;
								}
								return true;
							}
						);
						if (targ != nullptr)
						{
							unit->order_target.unit = targ;
						}
						helper::create_bullet(WeaponId::PaladinCrusaderMissiles, unit, source_x, source_y, unit->player, unit->current_direction_1);
					}
				}
			} break;
			case UnitId::ZergLakizilisk:
			{
				// Lakizilisk - play sound on attack, hardcoded here to facilitate a different sound on return
				if (unit->order_signal & 0x20) {
					unit->order_signal &= ~0x20;
					helper::play_sound_from_unit(SoundId::Bullet_ZLrkFir1_wav, unit, 1, 0);
				}
				// Lakizilisk - Seismic Spines
				if (unit->sprite != nullptr && unit->main_order_id != OrderId::Die
					&& unitExt->timers[TimerLurkerETA] == 1)
				{
					auto position = unit->position; // new target
					auto spr_position = unit->sprite->position;
					Point16 pos;
					pos.x = unitExt->values[ValueSpineX];
					pos.y = unitExt->values[ValueSpineY];
					unit->position = pos;
					unit->sprite->position = pos;
					scbw::artificial_bullet_from(WeaponId::SubterraneanSpines, unit->player, unit, nullptr, spr_position);
					helper::play_sound_from_unit(SoundId::LakiziliskProjectileReturn, unit, 1, 0);
					unit->position = position;
					unit->sprite->position = spr_position;
					auto spineCount = 0;
					globals.clear_lakizilisk(unit);
				}
			} break;
			case UnitId::TerranShaman:
			{
				// Shaman - Nanite Field
				if (unit->is_frozen() || unitExt->values[ValueReviveStarted])
				{
					break;
				}
				auto timer = unitExt->timers[TimerNaniteField];
				if (timer == 0)
				{
					unitExt->timers[TimerNaniteField] = 24;
					unit->sprite->play_iscript_animation(IscriptAnimation::CastSpell);
					scbw::UnitFinder(
						unit->get_x() - 128,
						unit->get_y() - 128,
						unit->get_x() + 128,
						unit->get_y() + 128
					).for_each([&](Unit* target)
						{
							shaman_nanite_field_proc(unit, ShamanHeal, target);
						});
				}
			} break;
			case UnitId::ProtossBarghest:
			{
				// Barghest - Repentent Chorus
				unitExt->values[ValueRepententChorus] = cosmonarchyUnitCount(unit, unit->position, 5 * 32, Flags::Ally | Flags::NotSelf, { UnitId::ProtossBarghest });
			} break;
			case UnitId::TerranCentaur:
			{
				// Centaur - Sunchaser Missiles
				if (unit->order_signal & 0x20)
				{
					unit->order_signal &= ~0x20;
					if (unit->order_target.unit != nullptr)
					{
						if (unit->get_distance_to_target(unit->order_target.unit) <= CentaurMissileDistance)
						{
							unit->play_iscript_animation(IscriptAnimation::Unused2);
						}
					}
				}
			} break;
			case UnitId::TerranOlympian:
			{
				if (unit->order_signal & 0x20)
				{
					unit->order_signal &= ~0x20;
					if (unit->order_target.unit != nullptr && !(units_dat::BaseProperty[unit->order_target.unit->id] & UnitProperty::Flyer))
					{
						if (unit->get_distance_to_target(unit->order_target.unit) <= OlympianMeleeDistance)
						{
							unit->play_iscript_animation(IscriptAnimation::SpecialState1);
						}
					}
				}
			}
			case UnitId::ZergMatraleth:
			{
				// Matraleth - Ensnaring Brood
				if (scbw::get_generic_rust_value(unit, RustId::CastEnsnaringBrood) == 1)
				{
					scbw::decloak_proc(unit);
					Point16 pos = unit->position;
					s32 radius = 6 * 32;
					scbw::set_generic_rust_value(unit, RustId::CastEnsnaringBrood, 0);
					scbw::UnitFinder(
						pos.x - radius,
						pos.y - radius,
						pos.x + radius,
						pos.y + radius
					).for_each([&](Unit* target)
					{
						if (!(scbw::is_allied_to(target->player, unit->player)))
						{
							return;
						}
						if (units_dat::BaseProperty[target->id] & UnitProperty::Building
							|| !(target->status & UnitStatus::Completed))
						{
							return;
						}
						// Overlays
						if (target->get_overlay(ImageId::EnsnaringBroodOverlay))
						{
							target->remove_overlay(ImageId::EnsnaringBroodOverlay);
						}
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->timers[TimerEnsnaringBrood] = 12 * 24;
						target->sprite->create_top_overlay(ImageId::EnsnaringBroodOverlay);
					});
					unit->spend_unit_energy(techdata_dat::EnergyCost[TechId::MatralethEnsnaringBrood] * 256);
				}
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Phase Link
				if (scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 0)
				{
					scbw::send_gptp_aise_command(unit, nullptr, GptpId::SetPhaseLinkStatus, 1, 0, 0);
				}
			} break;
			case UnitId::ProtossDemiurge:
			{
				// Demiurge - Gods Made Flesh - initial cast
				if (scbw::get_generic_rust_value(unit, RustId::CastGodsMadeFlesh) && !unitExt->values[ValueGodsMadeFleshActive])
				{
					scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 0);
					scbw::decloak_proc(unit);
					unitExt->values[ValueGodsMadeFleshActive] = 1;
					helper::play_sound_from_unit(scbw::rand_between(SoundId::DemiurgeGodsMadeFleshCast01, SoundId::DemiurgeGodsMadeFleshCast02), unit, 1, 0);
					unit->sprite->create_top_overlay(ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(unit->id));
				}
				// Demiurge - Gods Made Flesh - apply link
				if (unitExt->values[ValueGodsMadeFleshActive])
				{
					for (Unit* target : unitExt->unit_vectors[UnitVectorWorshipersDemiurge]) {
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->timers[TimerGodsMadeFlesh] = 3;
						u32 overlay = ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(target->id);
						if (!target->get_overlay(overlay)) {
							target->sprite->create_top_overlay(overlay);
						}
					}
					if (!unit->energy) {
						unitExt->values[ValueGodsMadeFleshActive] = 0;
					}
				}
			} break;
			case UnitId::TerranMaverick:
			{
				// Decloak on Stimpack activation
				if (unit->stim_timer == StimpackTimer)
				{
					scbw::decloak_proc(unit);
				}
			} break;
			case UnitId::TerranWyvern:
			{
				if (scbw::get_generic_rust_value(unit, RustId::CancelLandGrab))
				{
					scbw::set_generic_rust_value(unit, RustId::CancelLandGrab, 0);
					unitExt->values[ValueWyvernLandGrabOn] = 0;
				}
			} break;
			case UnitId::SharedTarasque:
			{
				if (unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] == 0)
				{
					if (fire_multiple(unit, unitExt, WeaponId::TarasqueAcceleratedPlasmid, 5, Flags::Enemy, true))
					{
						unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] = weapons_dat::Cooldown[WeaponId::TarasqueAcceleratedPlasmid];
					}
					else
					{
						unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::SharedAnticthon:
			{
				// Revive Status
				u32 flags = Flags::NotSelf | Flags::Ally | Flags::NoBuilding | Flags::NoTrap | Flags::NoHallucination | Flags::NoEgg | Flags::NoLarva | Flags::NoHangar;
				u32 radius = 4 * 32;
				if (!unitExt->values[ValueReviveStarted])
				{
					auto match = cosmonarchy_finder_match(unit, flags, {});
					scbw::UnitFinder(
						unit->get_x() - radius,
						unit->get_y() - radius,
						unit->get_x() + radius,
						unit->get_y() + radius
					).for_each_in_match(match, [&](Unit* target) {
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->timers[TimerAnticthonRevive] = 2;
					});
				}
			} break;
			case UnitId::ProtossAnthelion:
			{
				u32 attackRadius = 10 * 32;
				auto match = cosmonarchy_finder_match(unit, Flags::Enemy, {});
				bool attacked = false;
				scbw::UnitFinder(
					unit->get_x() - attackRadius,
					unit->get_y() - attackRadius,
					unit->get_x() + attackRadius,
					unit->get_y() + attackRadius
				).for_each_in_match(match, [&](Unit* target)
				{
					u32 dist = scbw::get_distance_fast(unit->position, target->position);
					if (dist <= weapons_dat::OuterSplashRadius[WeaponId::AnthelionEventHorizon])
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!(units_dat::BaseProperty[target->id] & UnitProperty::Building)
							&& targetExt->timers[TimerAnthelionGravityWell])
						{
							targetExt->timers[TimerAnthelionGravityWell] = 2;
							targetExt->units[UnitAnthelionGravityWellSource] = unit;
						}

						if (unitExt->timers[TimerAnthelionAttackCooldown] == 0)
						{
							attacked = true;

							u32 divider = 1;
							if (dist > weapons_dat::MediumSplashRadius[WeaponId::AnthelionEventHorizon])
							{
								divider = 4;
							}
							else if (dist > weapons_dat::InnerSplashRadius[WeaponId::AnthelionEventHorizon])
							{
								divider = 2;
							}
							target->damage_with(WeaponId::AnthelionEventHorizon, -1, unit, unit->player, 0, divider);
							helper::create_thingy(SpriteId::NeutronFlareOrange_Damageless, target->sprite->position.x, target->sprite->position.y, unit->player);
						}
					}
				});
				if (attacked)
				{
					unitExt->timers[TimerAnthelionAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::AnthelionEventHorizon);
					unit->sprite->create_underlay(ImageId::OpticalFlareHit_Large);
					helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionWeaponFire01, SoundId::AnthelionWeaponFire04), unit, 1, 0);
				}

				if (unit->main_order_id == OrderId::Follow || unit->main_order_id == OrderId::AttackUnit)
				{
					unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
				}

				if (unit->main_order_id == OrderId::Move)
				{
					if (unitExt->anthelion_last_move_target != unit->move_target.pt)
					{
						if (unitExt->timers[TimerAnthelionEndlessEclipse] > 1)
						{
							helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionMoveCharge01, SoundId::AnthelionMoveCharge02), unit, 1, 0);
						}
						unitExt->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
					}
					unitExt->anthelion_last_move_target = unit->move_target.pt;
					s16 oldX = unit->sprite->position.x;
					s16 oldY = unit->sprite->position.y;

					if (unitExt->timers[TimerAnthelionEndlessEclipse] == 1)
					{
						bool teleport = scbw::teleport(unit, unit->move_target.pt, 0, true);
						if (teleport)
						{
							scbw::create_and_init_thingy(SpriteId::AnthelionTeleportDepart, oldX, oldY, unit->player, 11);
							helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionArrive01, SoundId::AnthelionArrive03), unit, 1, 0);
							unit->sprite->create_top_overlay(ImageId::AnthelionTeleportArrival, 0, 0, 0);
						}
					}
				}
				else
				{
					unitExt->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
				}
			} break;
			case UnitId::TerranSeraph:
			{
				// Observance - Seraph ability
				u32 radius = 6 * 32;
				auto match = cosmonarchy_finder_match(unit, Flags::Enemy, {});
				bool attacked = false;
				scbw::UnitFinder(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius
				).for_each_in_match(match, [&](Unit* target)
				{
					if ((!scbw::is_allied_to(unit->player, target->player)))
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!targetExt->timers[TimerObservanceDebuff])
						{
							unitExt->timers[TimerObservanceOverlay] = 2;
							targetExt->timers[TimerObservanceDebuff] = 5 * 24 + 1;
							targetExt->timers[TimerObservancePeriod] = 5 * 24 + 1;
							if (targetExt->values[ValueObservanceRend] == 0)
							{
								s32 overlay = ImageId::ObservanceOverlaySmall - scbw::get_overlay_offset(target->id);
								target->sprite->create_overlay(overlay);
							}
							targetExt->values[ValueObservanceRend] += 1;
						}
					}
				});
			} break;
			case UnitId::TerranAzazel:
			{
				// Tag nearby units for Sublime Shepherd
				scbw::UnitFinder(
					unit->get_x() - 128,
					unit->get_y() - 128,
					unit->get_x() + 128,
					unit->get_y() + 128
				).for_each([&](Unit* target)
					{
						if (target == unit)
						{
							return;
						}
						if (scbw::unit_id_is_resource_node(target->id, false))
						{
							return;
						}
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->values[ValueHasSublimeShepherd] = 1;
					});
			} break;
			case UnitId::SharedAxitrilisk:
			{
				// Axitrilisk - play sound on shield loss, timer of 5 seconds per unit
				if (unit->shields < 1
					&& !unitExt->timers[TimerAxitriliskShieldLossSFX])
				{
					unitExt->timers[TimerAxitriliskShieldLossSFX] = AxitriliskShieldLossSFXTimer;
					helper::play_sound_from_unit(scbw::rand_between(SoundId::AxitriliskShieldLoss01, SoundId::AxitriliskShieldLoss03), unit, 1, 0);
				}
			} break;
			case UnitId::TerranPhalanxSiegeMode:
			{
				// Fix AI-controlled Phalanxes sometimes failing to acquire targets while in siege mode, originally from DarkenedFantasies.
				// TODO: Find a solution that works, the code below does not. -Pr0nogo
				/*
				if (playerTable[unit->player].type == PlayerType::Computer && unit->main_order_id == OrderId::WatchTarget) {
					helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
				}
				*/
			} break;
			case UnitId::ZergIsthrathaleth:
			{
				if (unit->get_overlay(ImageId::PlagueChannelOverlay)) {
					if (unit->sprite->images.head->animation != IscriptAnimation::Unused2) {
						unit->remove_overlay(ImageId::PlagueChannelOverlay);
					}
				}
				else {
					if (unit->sprite->images.head->animation == IscriptAnimation::Unused2) {
						unit->sprite->create_top_overlay(ImageId::PlagueChannelOverlay); // TODO: Use the Isthrathaleth's attack overlay for origin coordinates.
					}
				}
			} break;
		}

		// Tyranny
		if (unitExt->values[ValueTyrannyPlayer]) {

			EntryArray mind_tyrants = unit_search_radial(unit->position, 8 * 32);
			unit_search_filter(&mind_tyrants, unit, 0, {UnitId::ProtossMindTyrant});
			unit_search_custom_filter(&mind_tyrants, [=](Unit* mind_tyrant) {
				if (scbw::is_allied_to(mind_tyrant->player, unitExt->values[ValueTyrannyPlayer] - 1)) {
					return true;
				}
				return false;
			});

			if (mind_tyrants.size) {
				if (unitExt->timers[TimerTyrannyRecede] <= 4 * 24) {
					if (unit->status & UnitStatus::Frozen) {
						unit->unfreeze();
					}
					unit->remove_overlay(ImageId::MaelstromOverlay_Small, ImageId::MaelstromOverlay_Large);
				}
				unitExt->timers[TimerTyrannyRecede] = 5 * 24;
			}
			if (unitExt->timers[TimerTyrannyRecede] <= 4 * 24) {
				if (!(unit->status & UnitStatus::Frozen)) {
					unit->freeze();
				}
				u32 offset = scbw::get_overlay_offset(unit->id);
				if (!unit->get_overlay(ImageId::MaelstromOverlay_Small + offset)) {
					unit->sprite->create_top_overlay(ImageId::MaelstromOverlay_Small + offset);
				}
			}
			if (!unitExt->timers[TimerTyrannyRecede]) {
				if (unit->status & UnitStatus::Frozen) {
					unit->unfreeze();
				}
				unit->remove_overlay(ImageId::MaelstromOverlay_Small, ImageId::MaelstromOverlay_Large);
				scbw::recede_mind_control(unit);
				unitExt->values[ValueTyrannyPlayer] = 0;
			}
		}

		// Fix animation bug with acceleration movement flag
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
			&& unit->movement_flags & MovementFlags::Accelerating
			&& unit->sprite->main_graphic->animation != IscriptAnimation::Walking
			&& unit->sprite->main_graphic->animation != IscriptAnimation::WalkingToIdle)
		{
			unit->movement_flags &= ~MovementFlags::Accelerating;
		}

		// Sublime Shepherd - Check tagged units for nearby Azazels
		unitExt->values[ValueSublimeShepherd] = 0;
		if (unitExt->values[ValueHasSublimeShepherd])
		{
			unitExt->values[ValueHasSublimeShepherd] = 0;
			scbw::UnitFinder(
				unit->get_x() - 128,
				unit->get_y() - 128,
				unit->get_x() + 128,
				unit->get_y() + 128
			).for_each([&](Unit* target)
				{
					if (target == unit)
					{
						return;
					}
					if (target->id != UnitId::TerranAzazel)
					{
						return;
					}
					unitExt->values[ValueSublimeShepherd]++;
				});
		}

		// Azazel - Sublime Shepherd overlay
		if (unitExt->values[ValueSublimeShepherd] && !unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id)))
		{
			unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		}
		else if (unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id)) && !unitExt->values[ValueSublimeShepherd])
		{
			unit->sprite->remove_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		}

		// Fix target clearing bug
		/*
		if (unit->order_target.unit
			&& !(unit->order_target.unit->visibility_status & (1 << unit->player)))
		{
			scbw::print("lard");
			unit->order_to_idle();
		}
		*/

		// Play harvesting animations when a worker harvests from a resource
		// Requires more involved anim/graphics work
		/*
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			Unit* harvest_target = unit->worker.harvest_target;
			// Eventually make it happen for vespene geysers and mineral fields, not just refineries
			if (harvest_target && scbw::unit_id_is_gas_structure(harvest_target->id, true) && harvest_target->sprite->main_graphic->animation != IscriptAnimation::IsWorking)
			{
				harvest_target->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			}
		}
		*/

		// Phase Link Overlay
		if (*elapsedTimeFrames % 10 == 0
			&& unitExt->unit_vectors[UnitVectorPhaseLinkClarions].size())
		{
			unit->sprite->create_overlay(ImageId::PhaseLinkSmall + scbw::get_overlay_offset(unit->id), 0, 0, 0);
		}

		// Disruption Web Clear
		if (unitExt->timers[TimerUnderDisruptionWeb] == 1)
		{
			unit->status &= ~UnitStatus::CanNotAttack;
		}

		// Madrigal - Dirge
		if (unitExt->timers[TimerDirgeFuse] == 1)
		{
			scbw::madrigal_dirge_fuse(unit, unitExt);
		}

		// Hierophant Signify Reset
		if (unitExt->timers[TimerSignify] == 1)
		{
			unitExt->values[ValueSignifyStacks] = 0;
			unitExt->value_vectors[ValueVectorSignifySourcePlayer].clear();
			scbw::update_signify_status(unit, unitExt);
		}

		// Decloak on stun
		if (unit->is_frozen() || unitExt->timers[TimerStrigaRepulsion] || unitExt->timers[TimerAnthelionGravityWell])
		{
			scbw::decloak_proc(unit);
		}

		// Play addon work animations when related units are being trained by parent structure
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->building.addon && unit->building.addon->status & UnitStatus::Completed)
		{
			u16 unit_id = unit->build_queue[unit->build_queue_slot];
			if (unit_id != UnitId::None && !scbw::unit_id_is_available_without_an_addon(unit_id))
			{
				unitExt->values[ValuePlayingAddonWorkingAnim] = 1;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			}
			else if (unitExt->values[ValuePlayingAddonWorkingAnim])
			{
				unitExt->values[ValuePlayingAddonWorkingAnim] = 0;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle);
			}
		}

		// Correct vanilla timers - THIS IS TEMPORARY!
		if (unit->defensive_matrix_timer > AzazelDefensiveMatrixTimer)
		{
			unit->defensive_matrix_timer = AzazelDefensiveMatrixTimer;
		}
		if (unit->stasis_timer > DidactStasisFieldTimer)
		{
			unit->stasis_timer = DidactStasisFieldTimer;
		}
		if (unit->maelstrom_timer > MindTyrantMaelstromTimer)
		{
			unit->maelstrom_timer = MindTyrantMaelstromTimer;
		}
		if (unit->plague_timer > IsthrathalethPlagueTimer)
		{
			unit->plague_timer = IsthrathalethPlagueTimer;
		}

		// Striga - Fleeting Focus
		if (unitExt->timers[TimerStrigaRepulsion])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unitExt->repulsion_vector.x;
			finalPos.y += unitExt->repulsion_vector.y;
			finalPos = scbw::clamp_point(finalPos);

			if (!scbw::move_unit_improved(unit, finalPos.x, finalPos.y))
			{
				unitExt->timers[TimerStrigaRepulsion] = 0;
			}
		}

		// Magnetar - Law of Attraction
		// Todo - fix lag time between multiple pulls
		// Todo - fix delay before moved unit is able to do stuff
		if (unitExt->timers[TimerMagnetarAttraction])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unitExt->repulsion_vector.x;
			finalPos.y += unitExt->repulsion_vector.y;
			finalPos = scbw::clamp_point(finalPos);

			if (!scbw::move_unit_improved(unit, finalPos.x, finalPos.y))
			{
				unitExt->timers[TimerMagnetarAttraction] = 0;
			}
		}

		// Anthelion - Gravity Well
		if (unitExt->timers[TimerAnthelionGravityWell] && unitExt->units[UnitAnthelionGravityWellSource])
		{
			Point16 targetPos = unitExt->units[UnitAnthelionGravityWellSource]->position;
			if (unit->position != targetPos)
			{
				Vector32 diff = scbw::get_vector_between_points(unit->position, targetPos);

				if (scbw::get_vector_distance(diff) < 2)
				{
					scbw::move_unit_improved(unit, targetPos.x, targetPos.y);
				}
				else
				{
					Vector32f displacement = scbw::get_normalized_vector(diff);
					displacement.x *= 2;
					displacement.y *= 2;

					Point16 finalPos = unit->position;
					finalPos.x += displacement.x;
					finalPos.y += displacement.y;
					finalPos = scbw::clamp_point(finalPos);

					scbw::move_unit_improved(unit, finalPos.x, finalPos.y);
				}
			}
			s32 angle = scbw::get_angle(unit->position, targetPos);
			unit->current_direction_1 = angle;
		}

		// Cyclops - Flak Cannon Fuse
		for (u32 fuse_timer : unitExt->timer_vectors[TimerVectorCyclopsFuse])
		{
			if (fuse_timer == 1)
			{
				// Todo: make thingy
				helper::create_thingy(SpriteId::FragmentaryExplosion, unit->get_x(), unit->get_y(), 11);
				helper::play_sound_from_unit(scbw::rand_between(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04), unit, 1, 0);
				const u32 radius = 1.5 * 32;
				scbw::UnitFinder(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius
				).for_each_in_match(cosmonarchy_finder_match(unit, Flags::NotSelf, {}), [&](Unit* target)
				{
					target->damage_with_custom(5 * 256, DamageType::Explosive, nullptr, 0, 0, 1, false);
				});
			}
		}

		// Gorgon - Petrification
		if (unitExt->timers[TimerPetrification] == 1) {
			unitExt->values[ValuePetrificationStacks] = 0;
		}

		// Solarion - Auto Train
		if (unit->id == UnitId::ProtossSolarion && unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count < 12) {
			unit->build_queue[unit->build_queue_slot] = UnitId::ProtossSteward;
			unit->set_secondary_order(OrderId::TrainFighter);
		}

		// Blissful Embrace - Creep Spread
		if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
			EntryArray akistrokors = unit_search_radial(unit->position, 6 * 32);
			unit_search_filter(&akistrokors, unit, FilterAlly, {UnitId::ZergAkistrokor});
			unit->hitpoints += akistrokors.size * 256.0 / 24.0;

			if (!units_dat::GroupFlags[unit->id].isZerg && units_dat::GroupFlags[unit->id].is_factory) {
				helper::secondary_order_spawning_larva(unit);
			}

			if (*elapsedTimeFrames % 15 == 0 && !(unit->status & UnitStatus::InAir) && !helper::spreads_creep(unit->id, true)) {
				scbw::spread_custom_creep(unit);
			}
		}

		// Akistrokor - Blissful Embrace ability
		if (unit->id == UnitId::ZergAkistrokor) {
			if (unit->main_order_id == OrderId::BlissfulEmbrace) {
				Unit* target = unit->order_target.unit;
				if (target && target->hitpoints <= target->get_max_hp_raw() / 2 && target->status & UnitStatus::Completed
					&& units_dat::BaseProperty[target->id] & UnitProperty::Building && !(units_dat::BaseProperty[target->id] & UnitProperty::Addon
					&& unit->player != target->player && !scbw::is_allied_to(unit->player, target->player))) {
					UnitExt* target_ext = globals.get_unit_ext(target);

					for (u32 i = 0; i < 8; ++i) {
						if (scbw::is_allied_to(i, unit->player)) {
							target_ext->ai_timer_avoid_targeting[i] = 2;
						}
					}
					if (unit->sprite->main_graphic->animation == IscriptAnimation::CastSpell) {
						if (unit->order_signal & 0x2) {
							unit->order_signal &= ~0x2;
							helper::force_order_done(unit);
							scbw::blissful_embrace(target, unit->player);
						}
					}
				} else {
					helper::force_order_done(unit);
				}
			} else {
				unit->order_signal &= ~0x2;
			}
		}

		// Reviving
		if (((unitExt->timers[TimerReviveFlagClearMin] == 0 && !(unit->status & UnitStatus::NoBrkCodeStart)) || unitExt->timers[TimerReviveFlagClearMax] == 0)
			&& unitExt->values[ValueReviveStarted])
		{
			unit->status &= ~(UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
			unitExt->timers[TimerReviveFlagClearMin] = 0;
			unitExt->timers[TimerReviveFlagClearMax] = 0;
			unitExt->values[ValueReviveStarted] = 0;
			scbw::give_ai_ex(unit, true);
		}

		// Alaszileth - Dermal Accelerant
		Point16 pos = unit->position;
		s32 radius = 7 * 32;
		scbw::UnitFinder(
			pos.x - radius,
			pos.y - radius,
			pos.x + radius,
			pos.y + radius
		).for_each([&](Unit* foundUnit) {
			if (!(scbw::is_allied_to(foundUnit->player, unit->player)))
			{
				return;
			}
			if (foundUnit->id != UnitId::ZergAlaszileth)
			{
				return;
			}
			if (scbw::unit_id_is_resource_node(unit->id, false))
			{
				return;
			}
			unitExt->timers[TimerAlaszilethRegenBoost] = 2;

			// Create overlay
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::get_overlay_offset(unit->id, true);
			if (!unit->get_overlay(overlay))
			{
				unit->sprite->create_top_overlay(overlay);
			}
		});

		// Seraph - Observance
		if (unitExt->timers[TimerObservanceDebuff] > 1)
		{
			const auto range = 6 * 32;
			auto seraphs = scbw::UnitFinder(
				unit->get_x() - range,
				unit->get_y() - range,
				unit->get_x() + range,
				unit->get_y() + range
			).filter([&](Unit* target)
			{
				if (unit->player == target->player
					|| scbw::is_allied_to(unit->player, target->player)
					|| target->id != UnitId::TerranSeraph)
				{
					return false;
				}
				return true;
			}).getAllUnits();

			if (!seraphs.empty())
			{
				// Seraph - Observance period
				unitExt->timers[TimerObservanceDebuff] = 5 * 24 + 1;

				// Seraph - Observance ability
				if (unitExt->timers[TimerObservancePeriod] == 1)
				{
					unitExt->timers[TimerObservancePeriod] = 5 * 24 + 1;
					unitExt->values[ValueObservanceRend] += 1;
				}

				// Seraph - Observance persistant overlay creation
				for (auto seraph : seraphs)
				{
					if (!seraph)
					{
						continue;
					}
					UnitExt* seraphExt = globals.get_unit_ext(seraph);
					if (seraphExt->timers[TimerObservanceOverlay] == 0)
					{
						seraph->sprite->create_overlay(ImageId::SeraphRedOverlay);
						seraphExt->timers[TimerObservanceOverlay] = 18;
					}
				}
			}
		}
		else if (unitExt->timers[TimerObservanceDebuff] == 1)
		{
			unit->sprite->remove_overlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
		}

		// Vithrilisk - Corrosive Touch Clear
		if (unitExt->timers[TimerCorrosiveTouch] == 1)
		{
			unitExt->values[ValueCorrosiveTouchStacks] = 0;
			unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Zergling - Adrenal Glands
		if (unitExt->timers[TimerAdrenal] == 1)
		{
			unitExt->values[ValueAdrenalStacks] = 0;
		}

		// Vorvaling - Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws] == 1)
		{
			unitExt->unit_vectors[UnitVectorCaptivatingClaws] = {};
		}

		// Game Rule -  Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
		if (unit->main_order_id == OrderId::Harvest1
			|| unit->main_order_id == OrderId::HarvestGas1
			|| unit->main_order_id == OrderId::ReturnMinerals
			|| unit->main_order_id == OrderId::ReturnGas)
		{
			if (unit->status & UnitStatus::IsGathering)
			{
				unit->status |= UnitStatus::NoCollide;
			}
			else
			{
				unit->status &= ~UnitStatus::NoCollide;
			}
		}

		// Hallucinating Visual Effect
		u32 hallucinatingTimer = unitExt->timers[TimerHallucinating];
		if (unit != nullptr && unit->sprite != nullptr)
		{
			if (hallucinatingTimer % 24 == 1)
			{
				Sprite* sprite;
				if (unit->subunit != nullptr)
				{
					sprite = unit->subunit->sprite;
				}
				else
				{
					sprite = unit->sprite;
				}
				sprite->create_overlay(ImageId::HallucinationOverlay, 0, 0, 0);
			}
		}

		// Luminary - Wax Wings Clear
		if (unitExt->timers[TimerWaxWingsDebuff] == 1)
		{
			unit->update_speed();
		}

		// Luminary - Aural Assault
		if (unitExt->timers[TimerAuralAssaultBlind] > 1)
		{
			if (unit->main_order_id != OrderId::AttackUnit)
			{
				auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
				const u32 radius = unit->get_seek_range() * 32;
				Unit* target = scbw::UnitFinder::get_nearest_target(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius,
					unit, match
				);
				if (target)
				{
					unit->order_to(OrderId::AttackUnit, target);
				}
			}
		}

		// Talos - Salvage Stun
		if (unitExt->timers[TimerSalvageStunTimer] > 1) {
			unit->freeze();

			u32 offset = scbw::get_overlay_offset(unit->id);
			if (!unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->create_top_overlay(ImageId::SalvageDisableSmall + offset);
			}
			if (unitExt->timers[TimerTalosSoundLoopTimer] == 0) {
				helper::play_sound_from_unit(SoundId::TalosSalvageLoop, unit, 1, 0);
				unitExt->timers[TimerTalosSoundLoopTimer] = 63;
			}
		}
		if (unitExt->timers[TimerSalvageStunTimer] == 1) {
			unit->unfreeze();

			u32 offset = scbw::get_overlay_offset(unit->id);
			if (unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->remove_overlay(ImageId::SalvageDisableSmall + offset);
			}
		}

		// Cloaked units recloak after 2 seconds of being out of combat
		if (unitExt->timers[TimerDecloak] == 1)
		{
			if (scbw::is_decloakable(unit) && units_dat::BaseProperty[unit->id] & UnitProperty::PermanentCloak)
			{
				helper::anim_cloak(unit, SoundId::Terran_PHOENIX_TPhClo00_WAV);
			}
			else
			{
				unit->visibility_status = 0xff;
				helper::update_visibility(unit); // didact cloak needs special handling
			}
		}

		// Remove all Nemean Reactors stacks after not attacking for 5 seconds
		if (unitExt->timers[TimerNemeanReactors] == 1)
		{
			unitExt->values[ValueNemeanReactorsStack] = 0;
			// remove overlay later
		}

		// Update flingy speed when Adrenal Frenzy period expires
		if (unitExt->timers[TimerNathrokorBuff] == 1)
		{
			unitExt->timers[TimerNathrokorBuff] = 0;
			unitExt->values[ValueNathrokorBuffStacks] = 0;
			unit->update_speed();
		}

		// Autovitality - Simulacrum behavior
		if (unitExt->timers[TimerSimulacrumClone] == 1)
		{
			scbw::create_unit_from_ability(UnitId::ProtossSimulacrum, 1, unit->player, unit->position.x, unit->position.y, 0, unit);
		}

		// Remove Ensnaring Brood overlay
		if (unitExt->timers[TimerEnsnaringBrood] == 1)
		{
			unit->remove_overlay(ImageId::EnsnaringBroodOverlay);
		}

		// Remove Swarming Omen overlay
		if (unitExt->timers[TimerSwarmingOmen] == 1)
		{
			// unit->remove_overlay();
		}

		// Crucible Stacks Clear
		{
			auto& crucibles = unitExt->unit_vectors[UnitVectorCrucibles];
			for (auto it = crucibles.begin(); it != crucibles.end(); ++it)
			{
				Unit* target = *it;
				if (scbw::get_distance_fast(unit->position, target->position) > 8 * 32)
				{
					target->remove_overlay(ImageId::CrucibleKhaydarinChargeOverlay);
					crucibles.erase(it);
					--it;
				}
			}
		}

		// Crucible - Khaydarin Charge stacks
		if (unit->id == UnitId::ProtossCrucible && unit->status & UnitStatus::Completed)
		{
			EntryArray arr = unit_search_radial(unit->position, 8 * 32);
			unit_search_filter(&arr, unit, FilterAlly | FilterBuilding);
			for (u32 i = 0; i < arr.size; i++)
			{
				Unit* target = arr.units[i];
				if (scbw::unit_id_is_tech(target->id)) {
					continue;
				}
				switch (target->id) {
					case UnitId::ProtossPylon:
					case UnitId::ProtossPennant:
					case UnitId::ProtossCrucible:
					case UnitId::TerranSupplyDepot:
					case UnitId::TerranBunker:
					{
						continue;
					}
				}
				if (scbw::unit_id_is_gas_structure(target->id)) {
					continue;
				}
				if (target->id == UnitId::ZergNydusCanal && target->building.nydus_exit) {
					continue;
				}

				UnitExt* targetExt = globals.get_unit_ext(target);
				auto& vec = targetExt->unit_vectors[UnitVectorCrucibles];
				if (std::find(vec.begin(), vec.end(), unit) == vec.end())
				{
					vec.push_back(unit);
				}
				if (!target->get_overlay(ImageId::CrucibleKhaydarinChargeOverlay)) {
					target->sprite->create_underlay(ImageId::CrucibleKhaydarinChargeOverlay, 0, 0, 0);
				}
			}
		}

		// Controlled Demolition - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastControlledDemolition)
			&& !unitExt->timers[TimerEmergencyDetonation] && !unitExt->timers[TimerControlledDemolition])
		{
			scbw::set_generic_rust_value(unit, RustId::CastControlledDemolition, 0);
			unitExt->timers[TimerControlledDemolition] = ControlledDemolitionTimer;
			unit->sprite->create_overlay(ImageId::ControlledDemolition);
		}

		if (unitExt->timers[TimerControlledDemolition] == 7) {
			Image* overlay = unit->get_overlay(ImageId::ControlledDemolition);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Controlled Demolition - destroy structure
		if (unitExt->timers[TimerControlledDemolition] == 1)
		{
			unit->remove();
		}

		// Emergency Detonation - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation))
		{
			scbw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
			unitExt->timers[TimerEmergencyDetonation] = (u32)(0.5 * unitExt->timers[TimerControlledDemolition]);
			unitExt->timers[TimerControlledDemolition] = 0;
			unit->sprite->remove_overlay(ImageId::ControlledDemolition);
			unit->sprite->create_top_overlay(ImageId::EmergencyDetonation);
		}

		if (unitExt->timers[TimerEmergencyDetonation] == 7) {
			Image* overlay = unit->get_overlay(ImageId::EmergencyDetonation);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Emergency Detonation - destroy structure
		if (unitExt->timers[TimerEmergencyDetonation] == 1)
		{
			auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
			const u32 radius = 3 * 32;
			scbw::UnitFinder(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius
			).for_each_in_match(match, [&](Unit* target)
			{
				if (scbw::get_distance_fast(unit->position, target->position) <= radius)
				{
					target->damage_with_custom(250 * 256, DamageType::Explosive, unit, unit->player, 0, 1, false);
				}
			});
			helper::create_thingy(SpriteId::ArcliteShockCannon_Damageless, unit->sprite->position.x, unit->sprite->position.y, unit->player);
			unit->remove();
		}

		// Demiurge - Gods Made Flesh - return units to original owner
		if (unitExt->values[ValueWorshipStack]
			&& unitExt->units[UnitMindController] == nullptr)
		{
			helper::play_sound_from_unit(SoundId::DemiurgeWorship01, unit, 1, 0);
			unit->sprite->create_top_overlay(ImageId::Worship_Small + scbw::get_overlay_offset(unit->id), 0, 0, 0);
			unitExt->values[ValueWorshipStack] = 0;
			unitExt->timers[TimerGodsMadeFlesh] = 0;
			scbw::recede_mind_control(unit);
		}

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
		if (unitExt->timers[TimerGodsMadeFlesh] == 1 && unitExt->units[UnitMindController])
		{
			unit->remove();
		}

		// Alaszileth - Dermal Accelerant - remove overlays
		if (unitExt->timers[TimerAlaszilethRegenBoost] == 0)
		{
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::get_overlay_offset(unit->id, true);
			if (unit->get_overlay(overlay))
			{
				unit->remove_overlay(overlay);
			}
		}

		if (unitExt->values[ValueAI_WorkerChosenForExpansion] && unit->resource_type == ResourceType::None) {
			unit->order_to(OrderId::Move, unitExt->values[ValueAI_WorkerChosenX], unitExt->values[ValueAI_WorkerChosenY]);
			unitExt->values[ValueAI_WorkerChosenForExpansion] = 0;
			unitExt->values[ValueAI_WorkerChosenX] = 0;
			unitExt->values[ValueAI_WorkerChosenY] = 0;
		}
	}

	bool game_on()
	{
		chk_cache_size = 0;
		set_max_sight_range<20>();
		return true;
	}

	bool game_end()
	{
		end_scenario();
		return true;
	}
}