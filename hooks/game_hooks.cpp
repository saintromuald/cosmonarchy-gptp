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
#include "ai_layouts.h"
#include "trigger_system/trigger_system.h"
#include "trigger_system/trigger_parser.h"
#include "math_utils.h"
#include "graphics/Bitmap.h"
#include "data_ext.h"
#include "config.h"
#include "color_palette.h"
#include "custom_ai.h"
#include "SCBW\UnitFinder.h"
#include "hooks/interface/buttonsets.h"

void shaman_nanite_field_proc(Unit* shaman, u32 heal, Unit* target) {
	if (target->get_current_hp_ingame() < target->get_max_hp_ingame()) {
		u32 overlay = ImageId::ShamanNaniteField;
		Unit* target_context = target;
		if (target->subunit) {
			target_context = target->subunit;
		}
		target_context->sprite->create_top_overlay(overlay, 0, 0, 0);
	}

	u32 dist = scbw::get_distance_fast(target->position, shaman->position);
	if (dist <= 1 * 32) {
		heal *= 2.5;
	} else if (dist <= 2 * 32) {
		heal *= 2;
	} else if (dist <= 3 * 32) {
		heal *= 1.5;
	}
	target->heal_hp(heal * 256, shaman);
}

#pragma pack(push, 8)
#include <discord_game_sdk.h>
#pragma pack(pop)

#define DiscordClientId 972072419013558326
namespace hooks
{
	HMODULE discord_dll;
	typedef enum EDiscordResult discord_create_t(DiscordVersion version, struct DiscordCreateParams* params, struct IDiscordCore** result);
	discord_create_t* discord_create;

	static u32 discord_init_timeout;
	static bool discord_in_game;
	static IDiscordCore* discord;

	static void init_discord(bool in_game) {
		if (discord) {
			return;
		}

		if (discord_init_timeout) {
			discord_init_timeout--;
			return;
		}

		if (!discord_dll) {
			discord_dll = LoadLibraryA("discord_game_sdk.dll");
			if (!discord_dll) {
				discord_init_timeout = 30 * 24; // Let's forget about discord for a while, if this fails
				return;
			}
		}

		if (!discord_create) {
			discord_create = (discord_create_t*)GetProcAddress(discord_dll, "DiscordCreate");
			if (!discord_create) {
				discord_init_timeout = 30 * 24; // Let's forget about discord for a while, if this fails
				return;
			}
		}

		DiscordCreateParams params = {};
		params.client_id = DiscordClientId;
		params.flags = DiscordCreateFlags_NoRequireDiscord;
		EDiscordResult result = discord_create(DISCORD_VERSION, &params, &discord);
		if (result != DiscordResult_Ok) {
			discord_init_timeout = 30 * 24; // Let's forget about discord for a while, if this fails
			return;
		}

		discord_in_game = !in_game; // Let's force an update to activity, since we just init discord api
	}

	static void update_discord(bool in_game) {
		init_discord(in_game);
		if (!discord) {
			return;
		}

		if (discord_in_game != in_game) {
			discord_in_game = in_game;
			IDiscordActivityManager* activity_manager = discord->get_activity_manager(discord);
			if (activity_manager) {
				DiscordActivity activity = {};
				activity.type = DiscordActivityType_Playing;
				sprintf_s(activity.name, "Cosmonarchy BW");
				sprintf_s(activity.assets.large_text, "Cosmonarchy BW");
				sprintf_s(activity.assets.large_image, "cmbw");
				activity.timestamps.start = time(0);
				if (in_game) {
					sprintf_s(activity.details, "%s", current_map_name);
					sprintf_s(activity.state, "In Game");
					activity.instance = true;
				} else {
					sprintf_s(activity.details, "");
					sprintf_s(activity.state, "In Menu");
					activity.instance = false;
				}
				activity_manager->update_activity(activity_manager, &activity, 0, 0);
			}
		}

		discord->run_callbacks(discord);
	}

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
		update_discord(false);
		update_trigger_system(false);
	}

	void ui_button_click(BinDlg* dialog) {
		if (dialog->parent && dialog->parent->psz_text && strcmp(dialog->parent->psz_text, "StatBtn") == 0) {
			hooks::quickcasting_timeout  = 5;
			hooks::quickbuilding_timeout = 5;
		}
	}

	static bool process_quickcasting() {
		if (config.quick_casting && *g_is_targeting && !quickcasting_timeout) {
			if (PtInRect(g_minimap_rect, *(POINT*)g_mouse)) {
				u16 x = g_mouse->x;
				u16 y = g_mouse->y;
				helper::get_minimap_cursor_pos(&x, &y);
				helper::do_targeted_order(x, y, 0, UnitId::None);
			} else {
				Event event = {};
				event.x_pos = g_mouse->x;
				event.y_pos = g_mouse->y;
				helper::input_target_order_left_mouse_click(&event);
			}
			if (*g_is_targeting) {
				helper::input_target_order_right_mouse_click();
			}
			return true;
		}
		return false;
	}

	static bool process_quickbuilding() {
		if (config.quick_building && *g_is_placing_building && !quickbuilding_timeout) {
			bool not_held = true;
			for (char c = 'A'; c <= 'Z'; ++c) {
				if (g_is_keycode_used[c]) {
					not_held = false;
					break;
				}
			}
			if (not_held) {
				Point16 position = {};
				position.x = g_mouse->x;
				position.y = g_mouse->y;

				Event event = {};
				event.x_pos = position.x;
				event.y_pos = position.y;

				helper::input_place_building_left_mouse_click(&event, 0, position);
				if (*g_is_placing_building) {
					helper::input_place_building_right_mouse_click();
				}
			}
			return true;
		}
		return false;
	}

	u8 quickcasting_timeout;
	u8 quickbuilding_timeout;
	bool quickcasting_signal;
	u32 quickcasting_command_card;
	Unit* transport_ui_unit;

	int transport_ui_click() {
		helper::cmdact_unload(transport_ui_unit);
		return 0;
	}

	// Hooked to remove the 1-frame delay from quick-casting. -Pr0nogo
	u32 __cdecl set_ingame_input_procs() { // 00484CC0
		helper::initialize_input_procs();

		*common_accelerator = *accelerator_tables;

		g_input_procs->funcs[4]  = (InputProcsFunc*)0x00484380; // game_input_left_mouse_button_down;
		g_input_procs->funcs[6]  = (InputProcsFunc*)0x00484380; // game_input_left_mouse_button_down;
		g_input_procs->funcs[7]  = (InputProcsFunc*)0x00484620; // game_input_right_mouse_button_down;
		g_input_procs->funcs[9]  = (InputProcsFunc*)0x00484620; // game_input_right_mouse_button_down;

		g_input_procs->funcs[10] = (InputProcsFunc*)0x00484520; // game_input_middle_mouse_button_down;
		g_input_procs->funcs[11] = (InputProcsFunc*)0x004843A0; // game_input_middle_mouse_button_up;

		g_input_procs->funcs[13] = (InputProcsFunc*)0x00484390; // game_input_idle;
		g_input_procs->funcs[15] = (InputProcsFunc*)0x004846B0; // game_input_user_key_press;

		g_input_procs->funcs[16] = (InputProcsFunc*)0x004846E0; // cmdact_hotkey;
		g_input_procs->funcs[0]  = (InputProcsFunc*)0x00484350; // null_input;

		g_input_procs->funcs[3]  = 0;
		g_input_procs->funcs[17] = 0;
		g_input_procs->funcs[18] = 0;

		if (process_quickcasting()) {
			quickcasting_signal = true;
			quickcasting_command_card = *orders_buttonset;
		}

		if (transport_ui_unit) {
			*left_click_proc  = (InputProcsFunc*)transport_ui_click;
			*right_click_proc = (InputProcsFunc*)transport_ui_click;
		} else if (*g_is_placing_building) {
			*left_click_proc  = (InputProcsFunc*)0x0048E5D0; // input_place_building_left_mouse_click;
			*right_click_proc = (InputProcsFunc*)0x00484600; // input_place_building_right_mouse_click;
		} else if (*g_is_targeting) {
			*left_click_proc  = (InputProcsFunc*)0x004BD500; // input_target_order_left_mouse_click;
			*right_click_proc = (InputProcsFunc*)0x004845F0; // input_target_order_right_mouse_click;
		} else {
			*left_click_proc  = (InputProcsFunc*)0x0046FF70; // input_game_left_mouse_click;
			*right_click_proc = (InputProcsFunc*)0x004564E0; // input_game_right_mouse_click;
		}
		return 0;
	}

	void tick_unit(Unit* unit, bool unit_is_hidden);

	/// This hook is called every frame
	bool next_frame() {
		// Force disable cpu throttling, since on modern CPU it hurts more than helps
		*cpu_throttle = false;

		if (quickcasting_timeout && !*g_is_targeting) {
			quickcasting_timeout--;
		}
		if (quickbuilding_timeout && !*g_is_placing_building) {
			quickbuilding_timeout--;
		}

		s32* trigger_cooldown = (s32*)0x006509A0;
		*trigger_cooldown = 0; // Single frame hypertriggers

		update_discord(true);

		// This block is executed once every game
		if (*g_elapsed_time_frames == 0) {
#ifdef CM_DEBUG
			helper::play_sound_global(SoundId::Misc_Buzz_wav_1);
			scbw::print("\tGPTP was compiled in debug!\n\tThis will make the game run slower.", GameTextColor::Yellow);
#endif
			hooks::init_screen_locations();
			hooks::init_actions();
			globals.reset();
			init_ext_variables();

			for (s32 i = 0; i < 8; i++) {
				scbw::set_upgrade_level(i, UpgradeId::VentralSacs, 1); // Zerg AI may require this to function properly
			}

			// Connect addons
			for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
				if (units_dat::base_property[unit->id] & UnitProperty::Building && !unit->building.addon) {
					Unit* addon = helper::connect_addon_check(unit);
					if (addon) {
						helper::complete_addon(unit, addon);
					}
				}
			}
			
			// Init Extended Features
			reset_search_index();
			init_player_colors();
			init_trigger_system();
			init_layouts();
			init_custom_ai();

			// Execute Triggers before frame 0
			scbw::set_ingame_loop_state(true);
			helper::execute_all_game_triggers(g_game_speed_replay_modifiers[g_options_list->speed]);
			scbw::set_ingame_loop_state(false);
		}

		if (!scbw::is_game_paused()) { // If the game is not paused
			graphics::reset_all_graphics();
		}

		// Update extended features
		scbw::set_ingame_loop_state(true);
		update_trigger_system(true);
		if (!scbw::is_game_paused()) {
			update_layouts();
			update_custom_ai();
			hooks::update_actions();
		}
		scbw::set_ingame_loop_state(false);

		if (*g_elapsed_time_frames % 2 == 0)
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
		if (*g_elapsed_time_frames == 0 || *g_elapsed_time_frames == 15) {
			scbw::set_ingame_loop_state(true);
			load_player_colors();
			load_player_names();
			scbw::set_ingame_loop_state(false);
		}

		if (*g_elapsed_time_frames == 1) {
			u32 local_flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[*g_local_nation_id];
			if (!(local_flags & VespeneDeaths_DisableQuasifog)) {
				// Default maps to "semi-explored", from DarkenedFantasies
				for (s32 i = 0; i < 4092; i++) {
					if (g_fog_of_war_mask->unexplored[i] < 8) {
						g_fog_of_war_mask->unexplored[i] = 8;
					}
				}
				scbw::refresh_screen();
			}

			// Check deaths
			u8 reveal_flag = 0x00;
			bool do_ping[PLAYABLE_PLAYER_COUNT] = {};
			for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id)
			{
				u32 flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player_id];
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
				for (u32 x = 0; x < g_map_tile_size->width; x++)
				{
					for (u32 y = 0; y < g_map_tile_size->height; y++)
					{
						ActiveTile& tile = scbw::get_active_tile_at(x * 32, y * 32);
						tile.exploredFlags &= ~reveal_flag;
						tile.exploredFlags &= ~reveal_flag;
					}
				}
				*/

				// Reveal resources
				for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next)
				{
					if (unit->player == 11 && units_dat::base_property[unit->id] & UnitProperty::Building)
					{
						unit->sprite->visibility_status |= reveal_flag;
					}
				}
			}

			for (u32 target = 0; target < PLAYABLE_PLAYER_COUNT; ++target)
			{
				if (do_ping[target])
				{
					u32 color = 0; // red
					if (scbw::is_allied_to(*g_local_nation_id, target))
					{
						color = 8; // green
					}

					scbw::minimap_ping(g_start_positions[target].x, g_start_positions[target].y, color, *g_local_nation_id);
				}
			}
		}

		g_options_list->speed = 6; // Lock game speed at fastest
		if (!scbw::is_game_paused()) // If the game is not paused
		{
			*g_ai_update_cooldown = 0; // We don't want AI to be handicapped with request handling and whatnot
			*g_can_update_powered_status = true; // We want to have dynamic power updates, so we update power every frame
			for (u32 i = 0; i < 8; ++i) {
				raceSupply[RaceId::Terran].provided[i] = 60000;
			}

			scbw::set_ingame_loop_state(true); // Needed for scbw::random() to work
			hooks::update_psi_field_providers();

			reset_search_index();
			update_search_index();
			globals.frame_hook();
			update_search_index_effects();
			update_ext_variables();

			// Scenario Features
			if (*g_elapsed_time_frames == 1) {
				init_scenario();
			}
			if (*g_elapsed_time_frames >= 1) {
				tick_scenario();
			}

			hooks::handle_screen_hotkeys();
			scbw::refresh_screen();

			// Needed for extended tooltips
			static bool last_shift = 0;
			bool new_shift = *g_is_holding_shift;
			if (last_shift != new_shift) {
				scbw::refresh_console();
			}
			last_shift = new_shift;

			// Implict ESC for closing submenus. -Pr0nogo
			// @Veeq7 unsure if this is actually safe and/or sane?
			if (g_is_keycode_used[VK_ESCAPE] && scbw::buttonset_id_is_submenu(*g_active_portrait_buttonset_id) && !*g_is_game_paused)
			{
				hooks::update_buttonset();
			}

			// Draw transport UI
			{
				Point16 start_pos = {179, 333}; // {441, 203};
				Point16 pos = start_pos;

				if (transport_ui_unit) {
					transport_ui_unit = 0;
					helper::set_ingame_input_procs();
				}

				u32 x_count = 0;
				for (u32 i = 0; i < *g_client_selection_count; ++i) {
					Unit* transport = g_client_selection_group->unit[i];
					if (scbw::unit_is_allied_or_detected(transport, *g_local_nation_id)) {
						for (u32 j = 0; j < TransportCap; ++j) {
							Unit* cargo_unit = transport->get_loaded_unit(j);
							if (cargo_unit) {
								char buffer[128] = {};

								bool own_unit = cargo_unit->player == *g_local_nation_id;
								if (own_unit) {
									snprintf(buffer, 128, BWTC_GREEN "%s", get_short_unit_name(cargo_unit->id).data);
								} else if (scbw::is_allied_to(*g_local_nation_id, cargo_unit->player)) {
									snprintf(buffer, 128, BWTC_YELLOW "%s", get_short_unit_name(cargo_unit->id).data);
								} else {
									snprintf(buffer, 128, BWTC_RED "%s", get_short_unit_name(cargo_unit->id).data);
								}
								u32 width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(buffer);

								RECT rect = {};
								rect.left = pos.x;
								rect.top = pos.y;
								rect.right = rect.left + width;
								rect.bottom = rect.top + 13;
								bool hover = own_unit && PtInRect(&rect, *(POINT*)g_mouse);
								if (hover) {
									snprintf(buffer, 128, BWTC_WHITE "%s", get_short_unit_name(cargo_unit->id).data);
									width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(buffer);
								}

								graphics::draw_text(pos.x, pos.y, buffer, graphics::FONT_SMALL, graphics::CoordType::ON_SCREEN);
								if (hover && !transport_ui_unit) {
									transport_ui_unit = cargo_unit;
									helper::set_ingame_input_procs();
								}

								pos.x += width + 5;
								x_count++;
								if (x_count > 4) {
									x_count = 0;
									pos.x = start_pos.x;
									pos.y -= 15; // += 15;
								}
							}
						}
					}
				}
			}

			// Loop through bullets
			for (Bullet* bullet = *first_bullet; bullet; bullet = bullet->next)
			{
				if (bullet->weapon_id == WeaponId::KalkaliskViscousAcid)
				{
					if (bullet->order_signal & 0x80)
					{
						bullet->order_signal &= ~0x80;
						globals.remove_multihit_bullet(bullet);
					}
				}
			}

			process_quickbuilding();

			// Loop through all visible units in the game.
			for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
				tick_unit(unit, false);
			}
			for (Unit* unit = *g_first_hidden_unit; unit; unit = unit->link.next) {
				if (unit->status & UnitStatus::Completed) {
					tick_unit(unit, true);
				}
			}
			scbw::set_ingame_loop_state(false);
		}

		if (*g_active_portrait_unit)
		{
			/*
			Unit* selected = *g_active_portrait_unit;
			// For checking cargo-related elements. -Pr0nogo
			if (selected) {
				for (u32 i = 0; i < TransportCap; ++i) {
					Unit* cargo_unit = selected->get_loaded_unit(i);
					if (!cargo_unit)
					{
						continue;
					}
					u32 unit_status = cargo_unit->status;
					scbw::printf("0x%X", unit_status);
				}
			}
			*/
			/*
			if (selected->sprite && selected->sprite->main_graphic) {
				scbw::printf("%u", selected->sprite->main_graphic->animation);
			}
			*/
		//	scbw::printf("%u", selected->main_order_id);
		//	scbw::printf("0x%X", selected->status);

 			*unit_iterator_pos = 0;
			while (Unit* selection = get_active_player_next_selection()) {
				if (selection->id == UnitId::ZergNydusCesiant && selection->building.nydus_exit
				&& (scbw::is_allied_to(selection->player, *g_local_human_id) || helper::get_detecting_players(selection) & (1 << *g_local_human_id)))
				{
					Unit* exit = selection->building.nydus_exit;
					graphics::draw_line(selection->get_x(), selection->get_y(), exit->get_x(), exit->get_y(), graphics::GREEN, graphics::ON_MAP);
				}
			}
		}
		return true;
	}

	// Fix facing angles for specific units
	static void init_unit(Unit* unit, UnitExt* unit_ext) {
		switch (unit->id) {
			case UnitId::TerranGoliath: {
				helper::turn_unit(unit, 96);
				if (unit->subunit) {
					helper::turn_unit(unit->subunit, 96);
				}
			} break;
			case UnitId::TerranSentinel: {
				helper::turn_unit(unit, 96);
			} break;
		}
	}

	void tick_unit(Unit* unit, bool unit_is_hidden) {
		Unit* overlay_unit = unit;
		if (unit->status & UnitStatus::InTransport && unit->connected_unit) {
			Unit* transport = unit->connected_unit;

			if (unit->position.x != transport->position.x || unit->position.y != transport->position.y) {
				scbw::move_unit_force(unit, transport->position.x, transport->position.y);
			}

			if (unit_is_hidden) {
				overlay_unit = transport;
			}
		}

		if (unit->sprite) {
			SpriteExt* sprite_ext = get_sprite_ext(unit->sprite);
			sprite_ext->owner = unit;
			sprite_ext->should_consume_corpse = scbw::unit_get_corpse_consume(unit) != 0;
		}

		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->status & UnitStatus::Completed) {
			if (unit_ext->values[ValueUnitInit] != unit->id + 1) {
				unit_ext->values[ValueUnitInit] = unit->id + 1;
				init_unit(unit, unit_ext);
			}
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon) {
			UnitExt* addon_ext = get_unit_ext(unit->building.addon);
			addon_ext->units[UnitAddonParent] = unit;
		}

		if (!unit_is_hidden && unit->sprite) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (unit->is_visible_to(i)) {
					unit_ext->seen_by_flags |= (1 << i);
					unit_ext->last_seen_position[i] = unit->position;
					unit_ext->last_seen_timer[i] = 0;
				}
			}
		}

		if (!unit_is_hidden) {
			tactical_ai(unit, unit_ext);
		}

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

		if ((unit->id == UnitId::ProtossPariah || unit->id == UnitId::TerranStriga) && unit->order_signal & 0x20)
		{
			unit->order_signal &= ~0x20;
			globals.remove_unit_from_pariah_table(unit, true);
		}
		//hooks::show_health_bar(unit);

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
			case UnitId::ZergEvigrilisk: {
				bool is_moving = unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving;
				bool can_fire = !unit->ground_weapon_cooldown;
				u32 weapon_id = WeaponId::EvigriliskBrutishTendrils;
				if (is_moving && can_fire) {
					if (hooks::fire_multiple(unit, unit_ext, weapon_id, 4, FilterEnemy, true, false)) {
						scbw::decloak_proc(unit);
						unit->ground_weapon_cooldown = hooks::get_modified_weapon_cooldown(unit, weapon_id);
						helper::play_sound_from_unit(scbw::rand_between(SoundId::EvigriliskFire01, SoundId::EvigriliskFire03), unit, 1, 0);
					}
				}
			} break;
			case UnitId::TerranTalos:
			{
				if (unit->main_order_id == OrderId::TalosSalvage) {
					Unit* target = unit->order_target.unit;
					if (target) {
						if (unit->is_target_within_min_range(target, unit->get_max_weapon_range(unit->get_ground_weapon()))) {
							UnitExt* targetExt = get_unit_ext(target);
							targetExt->timers[TimerSalvageStunTimer] = 2 * 24;
							
							u32 time = units_dat::time_cost[target->id] / 2;
							targetExt->salvage_time += 1;
							
							targetExt->salvage_minerals += (f32)units_dat::mineral_cost[target->id] / time;
							while (targetExt->salvage_minerals > 1) {
								targetExt->salvage_minerals -= 1;
								resources->minerals[unit->player] += 1;
							}
							
							targetExt->salvage_gas += (f32)units_dat::gas_cost[target->id] / time;
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
								&& unit_ext->timers[TimerTalosAnimationSubtimer] == 0) {

								u32 width  = units_dat::building_dimensions[target->id].x * 0.75;
								u32 height = units_dat::building_dimensions[target->id].y * 0.75;

								s32 x_offset = scbw::random() % width  - width  / 2;
								s32 y_offset = scbw::random() % height - height / 2;

								x_offset = MathUtils::clamp(x_offset, -127, 128);
								y_offset = MathUtils::clamp(y_offset, -127, 128);

								target->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
								unit_ext->timers[TimerTalosAnimationSubtimer] = scbw::rand_between(3, 7);
							}

							if (unit_ext->values[ValueTalosAnimationTimer] == 0
							||  unit_ext->values[ValueTalosAnimationTimer] % 128 == 0) {
								Point16 pos = target->sprite->position;
								switch (scbw::random() % 4) {
									case 0: {
										pos.x -= units_dat::unit_bounds[target->id].left * 0.75;
										pos.y -= units_dat::unit_bounds[target->id].top * 0.75;
									} break;
									case 1: {
										pos.x += units_dat::unit_bounds[target->id].right * 0.75;
										pos.y -= units_dat::unit_bounds[target->id].top * 0.75;
									} break;
									case 2: {
										pos.x -= units_dat::unit_bounds[target->id].left * 0.75;
										pos.y += units_dat::unit_bounds[target->id].bottom * 0.75;
									} break;
									case 3: {
										pos.x += units_dat::unit_bounds[target->id].right * 0.75;
										pos.y += units_dat::unit_bounds[target->id].bottom * 0.75;
									} break;
								}							
								helper::set_move_target_xy(pos.x, pos.y, unit);
							}
							unit_ext->values[ValueTalosAnimationTimer] += 1;
						} else {
							unit_ext->values[ValueTalosAnimationTimer] = 0;
						}
					}
				}
			} break;
			case UnitId::ZergZoryusthaleth: {
				// Zoryusthaleth - Swarming Omen
				if (unit->get_current_hp_ingame() < unit->get_max_hp_ingame() / 2)
				{
					unit_ext->timers[TimerSwarmingOmen] = 5 * 24;
					if (unit_ext->timers[TimerSwarmingOmenSFX] == 0) {
						unit_ext->timers[TimerSwarmingOmenSFX] = 10 * 24;
						helper::play_sound_from_unit(scbw::rand_between(SoundId::SwarmingOmenProc01, SoundId::SwarmingOmenProc02), unit, 1, 0);
					}
					if (!unit->sprite->get_overlay(ImageId::EnsnaringBroodOverlay)) {
						unit->sprite->create_top_overlay(ImageId::EnsnaringBroodOverlay);
					}
				}
				else {
					if (unit->sprite->get_overlay(ImageId::EnsnaringBroodOverlay) && unit_ext->timers[TimerSwarmingOmen] == 0) {
						unit->sprite->remove_overlay(ImageId::EnsnaringBroodOverlay);
					}
				}

				// Zoryusthaleth - Reconstitution
				if (unit_ext->timers[TimerReconstitutionCooldown] == 0) {
					u32 radius = 5 * 32;
					EntryArray corpses = {};
					corpse_search_radial(&corpses, unit->position, radius);
					for (u32 i = 0; i < corpses.count; ++i) {
						Thingy* corpse = corpses.corpses[i];
						EntryArray targets = {};
						unit_search_radial(&targets, unit->position, radius);
						unit_search_filter(&targets, unit, FilterAlly);
						for (u32 i = 0; i < targets.count; ++i) {
							Unit* target = targets.units[i];
							target->heal_hp(ZoryusthalethReconstitutionHealing, unit);
							target->sprite->create_underlay(ImageId::SmallExplosion_Unused);
							//scbw::create_and_init_thingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
						}
						scbw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= SpriteFlags::Flag04;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
						unit_ext->timers[TimerReconstitutionCooldown] = 1 * 24;
						break;
					}
				}
			} break;
			case UnitId::ZergUltrokor: {
				// Kaiser Rampage - Ultrokor behavior
				if (!unit_ext->timers[TimerKaiserRampageTimeout]) {
					u32 radius = 1 * 32;
					EntryArray corpses = {};
					corpse_search_radial(&corpses, unit->position, radius);
					for (u32 i = 0; i < corpses.count; ++i) {
						Thingy* corpse = corpses.corpses[i];
						unit_ext->timers[TimerKaiserRampageTimeout] = 0.5 * 24;
						unit_ext->timers[TimerKaiserRampage] = 1 + (2 * 24);
						unit->update_speed();

						scbw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= SpriteFlags::Flag04;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
						break;
					}
				}
			} break;
			case UnitId::ZergSovroleth:
			{
				{	
					EntryArray sovroleths = {};
					unit_search_radial(&sovroleths, unit->position, 8 * 32);
					unit_search_filter(&sovroleths, unit, FilterAlly, {UnitId::ZergSovroleth}); 
					unit_ext->values[ValueSovrolethLinkCount] = sovroleths.count;
				}

				// Update Speed
				unit->update_speed();

				// Corpse Cocoon
				EntryArray corpses = {};
				corpse_search_rect(&corpses, {unit->get_left(), unit->get_top(), unit->get_right(), unit->get_bottom()});
				for (u32 i = 0; i < corpses.count; ++i) {
					Thingy* corpse = corpses.corpses[i];
					if (!(corpse->sprite->flags & SpriteFlags::Flag04)) {
						Unit* egg = scbw::create_unit_in_game(UnitId::ZergEgg, unit->player, corpse->sprite->position.x, corpse->sprite->position.y, 0, false);
						if (egg) {
							egg->sprite->play_iscript_animation(IscriptAnimation::Walking, true);
							egg->build_queue[egg->build_queue_slot] = UnitId::ZergSovroleth;

							UnitExt* eggExt = get_unit_ext(egg);
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
				if (unit_ext->timers[TimerQuaziliskCancel])
				{
					if (unit->sprite->main_graphic->animation != IscriptAnimation::CastSpell)
					{
						unit->play_iscript_animation(IscriptAnimation::CastSpell);
					}
				}
				if ((unit->id == UnitId::ZergEgg && !scbw::is_larval_egg(unit)) || unit->id == UnitId::ZergCocoon) {
					if (scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y).has_creep && !unit->get_overlay(ImageId::CreepMorphOverlay)) {
						unit->sprite->create_underlay(ImageId::CreepMorphOverlay, 0, 2);
					}
				}
				if (unit_ext->timers[TimerCorpseCocoonHatch] == 1 + 19) {
					unit->play_iscript_animation(IscriptAnimation::GndAttkInit);
				}
				if (unit_ext->timers[TimerCorpseCocoonHatch] == 1) {
					helper::replace_unit_with_id(unit, UnitId::ZergSovroleth);
					scbw::give_ai_ex(unit);
					helper::play_morphing_complete_sound(unit);
					unit->build_queue[unit->build_queue_slot] = UnitId::None;
					helper::init_building(unit, 1);
					unit_ext->values[ValueSpecialSovrolethBirth] = true;
					unit->order_computer(OrderId::ZergBirth);
				}
			} break;
			case UnitId::ProtossStarSovereign:
			{
				// Star Sovereign - Intervention Missiles
				if (unit_ext->timers[TimerStarSovereignInterventionMissiles] == 0)
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
						unit_ext->timers[TimerStarSovereignInterventionMissiles] = weapons_dat::cooldown[WeaponId::StarSovereignInterventionMissiles];
						for (s32 arc = 0; arc < 6; arc++)
						{
							const LO_Header* loFile = lo_files->special_overlays[unit->sprite->main_graphic->id];
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
							auto maxRange = weapons_dat::max_range[WeaponId::StarSovereignInterventionMissiles];
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
				if (unit_ext->timers[TimerPlanetCrackerChannel] == 1)
				{
					globals.create_effect(EffectId_StarSovereignTheGriefOfAllGods, 360, {unit->position.x, unit->position.y}, 8 * 32, -1, unit->sprite->elevation, unit->player, unit, unit);
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
					unit->spend_unit_energy(techdata_dat::energy_cost[TechId::ApostleLazarusAgent] * 256);
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
							UnitExt* pickedUnitExt = get_unit_ext(picked_unit);
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
					if (!unit_ext->values[ValueDisruptionFieldOn])
					{
						unit_ext->values[ValueDisruptionFieldOn] = 1;
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
					unit_ext->values[ValueDisruptionFieldOn] = 0;
				}
			} break;
			case UnitId::TerranMadcap:
			{
				// Madcap Cabeirus Feeders - subtract stacks while moving
				if (unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving)
				{
					if (unit_ext->values[ValueMadcapStacks] > 0)
					{
						unit_ext->values[ValueMadcapStacks]--;
					}
				}
			} break;
			case UnitId::TerranHeracles:
			{
				// Heracles Nemean Reactors - remove stacks when out of combat
				if (unit_ext->values[ValueNemeanReactorsStack] > 0
				&&  unit_ext->timers[TimerNemeanReactors] <= 1)
				{
					unit_ext->values[ValueNemeanReactorsStack] = 0;
				}
			} break;
			case UnitId::TerranCuirass:
			{
				if (unit_ext->values[ValueCuirassIsSelfReviving])
				{
					if (unit->hitpoints + CuirassSelfHeal >= unit->get_max_hp_raw())
					{
						unit_ext->values[ValueCuirassIsSelfReviving] = 0;
						unit->hitpoints = unit->get_max_hp_raw();
						scbw::restore_func(unit);
						unit->order_computer(OrderId::Stop);
						unit->status &= ~(UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
						helper::play_sound_from_unit(SoundId::Terran_GOLIATH_TGoWht03_WAV, unit, 1, 0);
					}
					else
					{
						unit->heal_hp(CuirassSelfHeal);
						if (!unit->sprite->get_overlay(ImageId::StonewallProtocolOverlay))
						{
							unit->sprite->create_top_overlay(ImageId::StonewallProtocolOverlay, -3, -8);
						}
					}
				}
			} break;
			case UnitId::TerranStriga:
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
					else if (unit->id == UnitId::TerranStriga)
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
				if (unit_ext->timers[TimerSkyfall] == 1
					&& !unit->is_frozen())
				{
					u8 missile = unit_ext->values[ValueSkyfallStacks];
					unit_ext->values[ValueSkyfallStacks]++;
					if (missile == 20)
					{
						unit_ext->values[ValueSkyfallStacks] = 0;
					}
					else
					{
						unit_ext->timers[TimerSkyfall] = 2 + 1;
						u32 tgt_x = unit_ext->values[ValueSkyfallX];
						u32 tgt_y = unit_ext->values[ValueSkyfallY];

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

						s32 forwardOffset = weapons_dat::forward_offset[10];
						auto source_x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
						auto source_y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1) - weapons_dat::vertical_offset[10];
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
				// Lakizilisk - play new_sound on attack, hardcoded here to facilitate a different new_sound on return
				if (unit->order_signal & 0x20) {
					unit->order_signal &= ~0x20;
					helper::play_sound_from_unit(SoundId::Bullet_ZLrkFir1_wav, unit, 1, 0);
				}
				// Lakizilisk - Seismic Spines
				if (unit->sprite != nullptr && unit->main_order_id != OrderId::Die
					&& unit_ext->timers[TimerLurkerETA] == 1)
				{
					auto position = unit->position; // new target
					auto spr_position = unit->sprite->position;
					Point16 pos;
					pos.x = unit_ext->values[ValueSpineX];
					pos.y = unit_ext->values[ValueSpineY];
					unit->position = pos;
					unit->sprite->position = pos;
					scbw::artificial_bullet_from(WeaponId::LakiziliskSubterraneanSpines, unit->player, unit, nullptr, spr_position);
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
				if (unit->is_frozen() || unit_ext->values[ValueReviveStarted]) {
					break;
				}
				auto timer = unit_ext->timers[TimerNaniteField];
				if (timer == 0) {
					unit_ext->timers[TimerNaniteField] = 24;

					u32 overlay = ImageId::RestorationHit_Small + scbw::get_overlay_offset(unit->id);
					overlay_unit->sprite->create_overlay(overlay);

					EntryArray unit_array = {};
					unit_search_radial(&unit_array, unit->position, 4 * 32);
					unit_search_add_units_from_connected_transport(&unit_array, unit);
					unit_search_filter(&unit_array, unit, FilterAlly | FilterNoBuilding);

					for (u32 i = 0; i < unit_array.count; ++i) {
						Unit* target = unit_array.units[i];
						shaman_nanite_field_proc(unit, ShamanHeal, target);
					}
				}
			} break;
			case UnitId::TerranCentaur:
			{
				// Centaur - Sunchaser Missiles
				if (unit->order_signal & 0x20)
				{
					unit->order_signal &= ~0x20;
					Unit* target = unit->order_target.unit;
					if (target && unit->get_distance_to_target(target) <= CentaurMissileDistance)
					{
						unit->play_iscript_animation(IscriptAnimation::Unused2);
					}
				}
			} break;
			case UnitId::TerranOlympian:
			{
				Unit* target = unit->order_target.unit;
				if (unit->order_signal & 0x20 && unit->ground_weapon_cooldown <= 1)
				{
					unit->order_signal &= ~0x20;
					if (target && !(units_dat::base_property[target->id] & UnitProperty::Flyer) && unit->get_distance_to_target(target) <= OlympianMeleeDistance)
					{
						unit->play_iscript_animation(IscriptAnimation::SpecialState1);
					}
				}
			} break;
			case UnitId::ZergMatraleth:
			case UnitId::InfestedKerriganExt:
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
						if (units_dat::base_property[target->id] & UnitProperty::Building
							|| !(target->status & UnitStatus::Completed))
						{
							return;
						}
						// Overlays
						if (target->get_overlay(ImageId::EnsnaringBroodOverlay))
						{
							target->remove_overlay(ImageId::EnsnaringBroodOverlay);
						}
						UnitExt* targetExt = get_unit_ext(target);
						targetExt->timers[TimerEnsnaringBrood] = 12 * 24;
						target->sprite->create_top_overlay(ImageId::EnsnaringBroodOverlay);
					});
					unit->spend_unit_energy(techdata_dat::energy_cost[TechId::MatralethEnsnaringBrood] * 256);
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
				if (scbw::get_generic_rust_value(unit, RustId::CastGodsMadeFlesh) && !unit_ext->values[ValueGodsMadeFleshActive])
				{
					scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 0);
					scbw::decloak_proc(unit);
					unit_ext->values[ValueGodsMadeFleshActive] = 1;
					helper::play_sound_from_unit(scbw::rand_between(SoundId::DemiurgeGodsMadeFleshCast01, SoundId::DemiurgeGodsMadeFleshCast02), unit, 1, 0);
					unit->sprite->create_top_overlay(ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(unit->id));
				}
				// Demiurge - Gods Made Flesh - apply link
				if (unit_ext->values[ValueGodsMadeFleshActive])
				{
					for (Unit* target : unit_ext->unit_vectors[UnitVectorWorshipersDemiurge]) {
						UnitExt* targetExt = get_unit_ext(target);
						targetExt->timers[TimerGodsMadeFlesh] = 3;
						u32 overlay = ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(target->id);
						if (!target->get_overlay(overlay)) {
							target->sprite->create_top_overlay(overlay);
						}
					}
					if (!unit->energy) {
						unit_ext->values[ValueGodsMadeFleshActive] = 0;
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
					unit_ext->values[ValueWyvernLandGrabOn] = 0;
				}
			} break;
			case UnitId::SharedTarasque:
			{
				if (unit_ext->timers[TimerTarasqueSecondaryWeaponCooldown] == 0)
				{
					if (fire_multiple(unit, unit_ext, WeaponId::TarasqueAcceleratedPlasmid, 5, Flags::Enemy, true))
					{
						unit_ext->timers[TimerTarasqueSecondaryWeaponCooldown] = weapons_dat::cooldown[WeaponId::TarasqueAcceleratedPlasmid];
					}
					else
					{
						unit_ext->timers[TimerTarasqueSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranAnticthon:
			{
				// Revive Status
				u32 flags = Flags::NotSelf | Flags::Ally | Flags::NoBuilding | Flags::NoTrap | Flags::NoHallucination | Flags::NoEgg | Flags::NoLarva | Flags::NoHangar;
				u32 radius = 4 * 32;
				if (!unit_ext->values[ValueReviveStarted])
				{
					auto match = cosmonarchy_finder_match(unit, flags, {});
					scbw::UnitFinder(
						unit->get_x() - radius,
						unit->get_y() - radius,
						unit->get_x() + radius,
						unit->get_y() + radius
					).for_each_in_match(match, [&](Unit* target) {
						UnitExt* targetExt = get_unit_ext(target);
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
					if (dist <= weapons_dat::outer_splash_radius[WeaponId::AnthelionEventHorizon])
					{
						UnitExt* targetExt = get_unit_ext(target);
						if (!(units_dat::base_property[target->id] & UnitProperty::Building)
							&& targetExt->timers[TimerAnthelionGravityWell])
						{
							targetExt->timers[TimerAnthelionGravityWell] = 2;
							targetExt->units[UnitAnthelionGravityWellSource] = unit;
						}

						if (unit_ext->timers[TimerAnthelionAttackCooldown] == 0)
						{
							attacked = true;

							u32 divider = 1;
							if (dist > weapons_dat::medium_splash_radius[WeaponId::AnthelionEventHorizon])
							{
								divider = 4;
							}
							else if (dist > weapons_dat::inner_splash_radius[WeaponId::AnthelionEventHorizon])
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
					unit_ext->timers[TimerAnthelionAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::AnthelionEventHorizon);
					unit->sprite->create_underlay(ImageId::OpticalFlareHit_Large);
					helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionWeaponFire01, SoundId::AnthelionWeaponFire04), unit, 1, 0);
				}

				if (unit->main_order_id == OrderId::Follow || unit->main_order_id == OrderId::AttackUnit)
				{
					unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
				}

				if (unit->main_order_id == OrderId::Move) {
					Point16 target = unit->move_target.pt;
					if (scbw::get_active_tile_at_coords(target.x, target.y)->visibility_flags & (1 << unit->player)) {
						unit_ext->anthelion_last_move_target = {};
					} else {
						if (unit_ext->anthelion_last_move_target != target) {
							if (unit_ext->timers[TimerAnthelionEndlessEclipse] > 1) {
								helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionMoveCharge01, SoundId::AnthelionMoveCharge02), unit, 1, 0);
							}
							unit_ext->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
						}
						unit_ext->anthelion_last_move_target = unit->move_target.pt;
						s16 oldX = unit->sprite->position.x;
						s16 oldY = unit->sprite->position.y;

						if (unit_ext->timers[TimerAnthelionEndlessEclipse] == 1) {
							bool teleport = scbw::teleport(unit, unit->move_target.pt, 0, true);
							if (teleport) {
								scbw::create_and_init_thingy(SpriteId::AnthelionTeleportDepart, oldX, oldY, unit->player, 11);
								helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionArrive01, SoundId::AnthelionArrive03), unit, 1, 0);
								unit->sprite->create_top_overlay(ImageId::AnthelionTeleportArrival, 0, 0, 0);
							}
						}
					}
				} else {
					unit_ext->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
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
						UnitExt* targetExt = get_unit_ext(target);
						if (!targetExt->timers[TimerObservanceDebuff])
						{
							unit_ext->timers[TimerObservanceOverlay] = 2;
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
						UnitExt* targetExt = get_unit_ext(target);
						targetExt->values[ValueHasSublimeShepherd] = 1;
					});
			} break;
			case UnitId::ZergAxitrilisk:
			{
				// Axitrilisk - play new_sound on shield loss, timer of 5 seconds per unit
				if (unit->shields < 1
				&& !unit_ext->timers[TimerAxitriliskShieldLossSFX])
				{
					unit_ext->timers[TimerAxitriliskShieldLossSFX] = AxitriliskShieldLossSFXTimer;
					helper::play_sound_from_unit(scbw::rand_between(SoundId::AxitriliskShieldLoss01, SoundId::AxitriliskShieldLoss03), unit, 1, 0);
				}
			} break;
			case UnitId::TerranPhalanxSiegeMode:
			{
				// Fix AI-controlled Phalanxes sometimes failing to acquire targets while in siege mode, originally from DarkenedFantasies.
				// TODO: Find a solution that works, the code below does not. -Pr0nogo
				/*
				if (g_player_table[unit->player].type == PlayerType::Computer && unit->main_order_id == OrderId::WatchTarget) {
					helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
				}
				*/
			} break;
			case UnitId::ZergIsthrathaleth:
			{
				if (unit->get_overlay(ImageId::PlagueChannelOverlay)) {
					if (unit->main_order_id != OrderId::Plague) {
						unit->remove_overlay(ImageId::PlagueChannelOverlay);
					}
				} else {
					if (unit->main_order_id == OrderId::Plague && unit->sprite->main_graphic->animation == IscriptAnimation::Unused2) {
						unit->sprite->create_top_overlay(ImageId::PlagueChannelOverlay); // TODO: Use the Isthrathaleth's attack overlay for origin coordinates.
					}
				}
			} break;
			case UnitId::TerranMissileSilo:
			{
				// Light up yellow when missile is loaded. -Pr0nogo
				if (unit->build_queue[unit->build_queue_slot] == UnitId::None) {
					if (unit_ext->values[ValueMissileSiloLoaded] == 0 && unit->building.silo.nuke) {
						unit_ext->values[ValueMissileSiloLoaded] = 1;
						unit->sprite->main_graphic->play_iscript_animation(IscriptAnimation::Unused2);
					}
					else if (unit_ext->values[ValueMissileSiloLoaded] == 1 && !unit->building.silo.nuke) {
						unit_ext->values[ValueMissileSiloLoaded] = 0;
						unit->sprite->main_graphic->play_iscript_animation(IscriptAnimation::Built);
					}
				}
			} break;
			case UnitId::ZergDroleth:
			{
				// Temporary? fix for order queue insanity. -Pr0nogo
				// DOES NOT FIX AI!
				if (unit->main_order_id == OrderId::Morph1
				&&	unit->build_queue[unit->build_queue_slot] != UnitId::None
				&&  unit->build_queue[unit->build_queue_slot] != UnitId::ZergGosvileth)
				{
					unit->build_queue[unit->build_queue_slot] = UnitId::ZergGosvileth;
				}
			} break;
			case UnitId::TerranAncile: {
				if (unit->energy >= 1 * 256) { // Providence
					if (!unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) {
						unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlayLarge);
					}
					EntryArray bullets = {};
					bullet_search_radial(&bullets, unit->position, 2 * 32);
					for (u32 i = 0; i < bullets.count; i++) {
						Bullet* bullet = bullets.bullets[i];

						Unit* target = bullet->attack_target.unit;
						if (!target || !scbw::is_allied_to(unit->player, target->player) || bullet->hit_flags & 1) { // We skip flagged bullets, so we don't have an infinite cycle of redirecting
							continue;
						}

						bullet->attack_target.unit = unit;
						bullet->attack_target.pt   = unit->position;
						bullet->move_target.unit   = unit;
						bullet->move_target.pt     = unit->position;
						bullet->targetPosition     = unit->position;
						bullet->targetPosition2    = unit->position;
						bullet->hit_flags |= 1; // For now we flag as missed, so it doesn't deal hp damage
						unit->spend_unit_energy(helper::get_unit_bullet_damage(bullet, unit)); // TODO: Instead of dealing damage immediately, we should flag the bullet to deal damage as energy
						helper::play_sound_from_unit(scbw::rand_between(SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03), unit, 1, 0);

						if (unit->energy < 1 * 256) {
							break;
						}
					}
				} else if (unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) {
					unit->remove_overlay(ImageId::SublimeShepherdOverlayLarge);
				}
			} break;
			case UnitId::TerranDurendal:
			{
				// Durendal - Forward March
				if (unit_ext->timers[TimerDurendalSecondaryWeaponCooldown] == 0)
				{
					if (fire_multiple(unit, unit_ext, WeaponId::DurendalPhilistineAutogun, 1, Flags::Enemy, true))
					{
						unit_ext->timers[TimerDurendalSecondaryWeaponCooldown] = weapons_dat::cooldown[WeaponId::DurendalPhilistineAutogun];
					}
					else
					{
						unit_ext->timers[TimerDurendalSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranAmbassador:
			{
				// Ambassador - Forward March
				if (unit_ext->timers[TimerAmbassadorSecondaryWeaponCooldown] == 0)
				{
					if (fire_multiple(unit, unit_ext, WeaponId::AmbassadorSignatorySalvo, 1, Flags::Enemy, true))
					{
						unit_ext->timers[TimerAmbassadorSecondaryWeaponCooldown] = weapons_dat::cooldown[WeaponId::DurendalPhilistineAutogun];
					}
					else
					{
						unit_ext->timers[TimerAmbassadorSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
		}

		// Tyranny
		if (unit_ext->values[ValueTyrannyPlayer]) {
			EntryArray mind_tyrants = {};
			unit_search_radial(&mind_tyrants, unit->position, 8 * 32);
			unit_search_filter(&mind_tyrants, unit, 0, {UnitId::ProtossMindTyrant});
			unit_search_custom_filter(&mind_tyrants, [=](Unit* mind_tyrant) {
				if (scbw::is_allied_to(mind_tyrant->player, unit_ext->values[ValueTyrannyPlayer] - 1)) {
					return true;
				}
				return false;
			});

			if (mind_tyrants.count) {
				if (unit_ext->timers[TimerTyrannyRecede] <= 4 * 24) {
					if (unit->status & UnitStatus::Frozen) {
						unit->unfreeze();
					}
					u32 offset = scbw::get_overlay_offset(unit->id, true);
					unit->remove_overlay(ImageId::TyrannyStunSmall + offset);
				}
				unit_ext->timers[TimerTyrannyRecede] = 5 * 24;
			}
			if (unit_ext->timers[TimerTyrannyRecede] <= 4 * 24) {
				if (!(unit->status & UnitStatus::Frozen)) {
					unit->freeze();
				}
				u32 offset = scbw::get_overlay_offset(unit->id, true);
				if (!unit->get_overlay(ImageId::TyrannyStunSmall + offset)) {
					unit->sprite->create_top_overlay(ImageId::TyrannyStunSmall + offset);
				}
			}
			if (!unit_ext->timers[TimerTyrannyRecede]) {
				if (unit->status & UnitStatus::Frozen) {
					unit->unfreeze();
				}
				u32 offset = scbw::get_overlay_offset(unit->id, true);
				unit->remove_overlay(ImageId::TyrannyStunSmall + offset);
				scbw::recede_mind_control(unit);
				unit_ext->values[ValueTyrannyPlayer] = 0;
			}
		}

		// Fix animation bug with acceleration movement flag
		if (!(units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding)
			&& unit->movement_flags & MovementFlags::Accelerating
			&& unit->sprite->main_graphic->animation != IscriptAnimation::Walking
			&& unit->sprite->main_graphic->animation != IscriptAnimation::WalkingToIdle)
		{
			unit->movement_flags &= ~MovementFlags::Accelerating;
		}

		// Fix AI melee units freezing - DarkenedFantasies
		if (unit->status & UnitStatus::Unmovable
		&&  unit->main_order_id & OrderId::AttackUnit
		&& !unit->movement_flags
		&&  unit->movement_state == 0x0A)
		{
			unit->status ^= UnitStatus::Unmovable;
		}

		// Sublime Shepherd - Check tagged units for nearby Azazels
		unit_ext->values[ValueSublimeShepherd] = 0;
		if (unit_ext->values[ValueHasSublimeShepherd])
		{
			unit_ext->values[ValueHasSublimeShepherd] = 0;
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
					unit_ext->values[ValueSublimeShepherd]++;
				});
		}

		// Zoryusthaleth - Swarming Omen marking
		if (unit_ext->values[ValueSwarmingOmenPlayer]) {
			u32 player = unit_ext->values[ValueSwarmingOmenPlayer] - 1;
			EntryArray targets = {};
			unit_search_radial(&targets, unit->position, 10 * 32);
			for (u32 i = 0; i < targets.count; ++i) {
				Unit* target = targets.units[i];
				if (!scbw::is_allied_to(player, target->player)) {
					continue;
				}
				UnitExt* target_ext = get_unit_ext(target);
				target_ext->timers[TimerSwarmingOmenSearch] = 24;
			}
		}

		// Azazel - Sublime Shepherd overlay
		if (unit_ext->values[ValueSublimeShepherd] && !unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id)))
		{
			unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		}
		else if (unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id)) && !unit_ext->values[ValueSublimeShepherd])
		{
			unit->sprite->remove_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		}

		// Fix target clearing bug
		/*
		if (unit->order_target.unit
			&& !(unit->order_target.unit->detection_status & (1 << unit->player)))
		{
			scbw::print("lard");
			unit->order_to_idle();
		}
		*/

		// Play harvesting animations when a worker harvests from a resource
		// Requires more involved anim/graphics work
		/*
		if (units_dat::base_property[unit->id] & UnitProperty::Worker)
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
		if (*g_elapsed_time_frames % 10 == 0
			&& unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].size())
		{
			unit->sprite->create_overlay(ImageId::PhaseLinkSmall + scbw::get_overlay_offset(unit->id), 0, 0, 0);
		}

		// Parasite
		if (unit_ext->timers[TimerParasiteArm] == 1) {
			unit_ext->parasite_spawn_count[unit_ext->values[ValueParasiteArmPlayer]] += 2;
		}

		// Incubation
		if (unit_ext->timers[TimerIncubationArm] == 1) {
			u32 unit_id = UnitId::ZergRilirokor;
			if (units_dat::base_property[unit->id] & UnitProperty::Flyer
			||  unit->status & UnitStatus::InAir)
			{
				unit_id = UnitId::ZergRililisk;
			}
			scbw::create_unit_from_ability(unit_id, 2, unit_ext->values[ValueIncubationPlayer], unit->position.x, unit->position.y, 720);
		}

		// Disruption Web Clear
		if (unit_ext->timers[TimerUnderDisruptionField] == 1)
		{
			unit->status &= ~UnitStatus::CanNotAttack;
		}

		// Madrigal - Dirge
		if (unit_ext->timers[TimerDirgeFuse] == 1)
		{
			scbw::madrigal_dirge_fuse(unit, unit_ext);
		}

		// Hierophant Signify Reset
		if (unit_ext->timers[TimerSignify] == 1)
		{
			unit_ext->values[ValueSignifyStacks] = 0;
			unit_ext->value_vectors[ValueVectorSignifySourcePlayer].clear();
			scbw::update_signify_status(unit, unit_ext);
		}

		// Decloak on stun
		if (unit->is_frozen() || unit_ext->timers[TimerStrigaRepulsion] || unit_ext->timers[TimerAnthelionGravityWell])
		{
			scbw::decloak_proc(unit);
		}

		// Play addon work animations when related units are being trained by parent structure
		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon && unit->building.addon->status & UnitStatus::Completed)
		{
			u16 unit_id = unit->build_queue[unit->build_queue_slot];
			if (unit_id != UnitId::None && !scbw::unit_id_is_available_without_an_addon(unit_id))
			{
				unit_ext->values[ValuePlayingAddonWorkingAnim] = 1;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			}
			else if (unit_ext->values[ValuePlayingAddonWorkingAnim])
			{
				unit_ext->values[ValuePlayingAddonWorkingAnim] = 0;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle);
			}
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Addon && !(unit->status & UnitStatus::Completed)) {
			if (unit->connected_unit && !unit->connected_unit->is_frozen() && unit_ext->timers[TimerAddonAnimation] == 0) {
				u32 width  = units_dat::building_dimensions[unit->id].x * 0.5;
				u32 height = units_dat::building_dimensions[unit->id].y * 0.5;

				s32 x_offset = scbw::random() % width - width / 2;
				s32 y_offset = scbw::random() % height - height / 2;

				x_offset = MathUtils::clamp(x_offset, -127, 128);
				y_offset = MathUtils::clamp(y_offset, -127, 128);

				unit->sprite->create_overlay(ImageId::FusionCutterHitDamageless, x_offset, y_offset); // TODO: Figure out why changing a construction frame freezes these overlays.
				unit_ext->timers[TimerAddonAnimation] = scbw::rand_between(16, 20);
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
		if (unit_ext->timers[TimerStrigaRepulsion])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unit_ext->repulsion_vector.x;
			finalPos.y += unit_ext->repulsion_vector.y;
			finalPos = scbw::clamp_point(finalPos);

			if (!scbw::move_unit_improved(unit, finalPos.x, finalPos.y))
			{
				unit_ext->timers[TimerStrigaRepulsion] = 0;
			}
		}

		// Magnetar - Law of Attraction
		// Todo - fix lag time between multiple pulls
		// Todo - fix delay before moved unit is able to do stuff
		if (unit_ext->timers[TimerMagnetarAttraction])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unit_ext->repulsion_vector.x;
			finalPos.y += unit_ext->repulsion_vector.y;
			finalPos = scbw::clamp_point(finalPos);

			if (!scbw::move_unit_improved(unit, finalPos.x, finalPos.y))
			{
				unit_ext->timers[TimerMagnetarAttraction] = 0;
			}
		}

		// Anthelion - Gravity Well
		if (unit_ext->timers[TimerAnthelionGravityWell] && unit_ext->units[UnitAnthelionGravityWellSource])
		{
			Point16 targetPos = unit_ext->units[UnitAnthelionGravityWellSource]->position;
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
		for (u32 fuse_timer : unit_ext->timer_vectors[TimerVectorCyclopsFuse]) {
			if (fuse_timer == 1) {
				helper::create_thingy(SpriteId::FragmentaryExplosion, unit->get_x(), unit->get_y(), 11);
				helper::play_sound_from_unit(scbw::rand_between(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04), unit, 1, 0);
				const u32 radius = 1.5 * 32;
				scbw::UnitFinder(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius
				).for_each_in_match(cosmonarchy_finder_match(unit, Flags::IncludingSelf, {}), [&](Unit* target) {
					target->damage_with_custom(5 * 256, damage_type::Explosive, nullptr, 0, 0, 1, false);
				});
			}
		}

		// Gorgon - Petrification
		if (unit_ext->timers[TimerPetrification] == 1) {
			unit_ext->values[ValuePetrificationStacks] = 0;
		}

		// Solarion - auto train
		if (unit->id == UnitId::ProtossSolarion && unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count < 12) {
			unit->build_queue[unit->build_queue_slot] = UnitId::ProtossSteward;
			unit->set_secondary_order(OrderId::TrainFighter);
		}

		// Blissful Embrace - Creep Spread
		if (unit_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
			EntryArray akistrokors = {};
			unit_search_radial(&akistrokors, unit->position, 6 * 32);
			unit_search_filter(&akistrokors, unit, FilterAlly, {UnitId::ZergAkistrokor});
			unit->hitpoints += akistrokors.count * 256.0 / 24.0;

			if (!units_dat::group_flags[unit->id].is_zerg && units_dat::group_flags[unit->id].is_factory) {
				helper::secondary_order_spawning_larva(unit);
			}

			if (*g_elapsed_time_frames % 15 == 0 && !(unit->status & UnitStatus::InAir) && !helper::spreads_creep(unit->id, true)) {
				scbw::spread_custom_creep(unit);
			}
		}

		// Akistrokor - Blissful Embrace ability
		if (unit->id == UnitId::ZergAkistrokor) {
			if (unit->main_order_id == OrderId::BlissfulEmbrace) {
				Unit* target = unit->order_target.unit;
				if (target && target->hitpoints <= target->get_max_hp_raw() / 2 && target->status & UnitStatus::Completed
					&& units_dat::base_property[target->id] & UnitProperty::Building && !(units_dat::base_property[target->id] & UnitProperty::Addon
					&& unit->player != target->player && !scbw::is_allied_to(unit->player, target->player))) {
					UnitExt* target_ext = get_unit_ext(target);

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
		if (((unit_ext->timers[TimerReviveFlagClearMin] == 0 && !(unit->status & UnitStatus::NoBrkCodeStart)) || unit_ext->timers[TimerReviveFlagClearMax] == 0)
			&& unit_ext->values[ValueReviveStarted])
		{
			unit->status &= ~(UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
			unit_ext->timers[TimerReviveFlagClearMin] = 0;
			unit_ext->timers[TimerReviveFlagClearMax] = 0;
			unit_ext->values[ValueReviveStarted] = 0;
			scbw::give_ai_ex(unit);
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
			unit_ext->timers[TimerAlaszilethRegenBoost] = 2;

			// Create overlay
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::get_overlay_offset(unit->id, true);
			if (!unit->get_overlay(overlay))
			{
				unit->sprite->create_top_overlay(overlay);
			}
		});

		// Seraph - Observance
		if (unit_ext->timers[TimerObservanceDebuff] > 1)
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
			}).get_all_units();

			if (!seraphs.empty())
			{
				// Seraph - Observance period
				unit_ext->timers[TimerObservanceDebuff] = 5 * 24 + 1;

				// Seraph - Observance ability
				if (unit_ext->timers[TimerObservancePeriod] == 1)
				{
					unit_ext->timers[TimerObservancePeriod] = 5 * 24 + 1;
					unit_ext->values[ValueObservanceRend] += 1;
				}

				// Seraph - Observance persistant overlay creation
				for (auto seraph : seraphs)
				{
					if (!seraph)
					{
						continue;
					}
					UnitExt* seraphExt = get_unit_ext(seraph);
					if (seraphExt->timers[TimerObservanceOverlay] == 0)
					{
						seraph->sprite->create_overlay(ImageId::SeraphRedOverlay);
						seraphExt->timers[TimerObservanceOverlay] = 18;
					}
				}
			}
		}
		else if (unit_ext->timers[TimerObservanceDebuff] == 1)
		{
			unit->sprite->remove_overlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
		}

		// Vithrilisk - Corrosive Touch clear
		if (unit_ext->timers[TimerCorrosiveTouch] == 1)
		{
			unit_ext->values[ValueCorrosiveTouchStacks] = 0;
			unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Zethrokor - Adrenal Glands
		if (unit_ext->timers[TimerAdrenal] == 1)
		{
			unit_ext->values[ValueAdrenalStacks] = 0;
		}

		// Vorvrokor - Captivating Claws
		if (unit_ext->timers[TimerCaptivatingClaws] == 1)
		{
			unit_ext->unit_vectors[UnitVectorCaptivatingClaws] = {};
		}

		// Game rule -  Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
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

		// Hallucinating visual effect
		u32 hallucinatingTimer = unit_ext->timers[TimerHallucinating];
		if (unit && unit->sprite) {
			if (hallucinatingTimer % 24 == 1) {
				Sprite* sprite;
				if (unit->subunit) {
					sprite = unit->subunit->sprite;
				} else {
					sprite = unit->sprite;
				}
				sprite->create_overlay(ImageId::HallucinationOverlay, 0, 0, 0);
			}
		}

		// Luminary - Aural Assault
		if (unit_ext->timers[TimerAuralAssaultBlind] > 1) {
			u32 overlay = ImageId::OpticalFlareHit_Small + scbw::get_overlay_offset(unit->id);
			Unit* overlay_unit = unit->subunit ? unit->subunit : unit;
			if (!overlay_unit->get_overlay(overlay)) {
				overlay_unit->sprite->create_top_overlay(overlay, 0, 0, 0);
			}
			if (unit->main_order_id != OrderId::AttackUnit) {
				const u32 radius = unit->get_seek_range() * 32 + 4 * 32;
				auto match = cosmonarchy_finder_match(unit, 0, {});
				Unit* best_target = scbw::UnitFinder::get_nearest_target(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius,
					unit, [&](Unit* target) {
					if (!match(target) || !unit->can_attack_target(target) || !helper::is_unit_in_weapon_range(unit, target)) {
						return false;
					}
					return true;
				});
				if (best_target) {
					unit->order_to(OrderId::AttackUnit, best_target);
					if (unit->subunit) {
						unit->subunit->order_to(OrderId::AttackUnit, best_target);
					}
				}
			}
			unit->status |= UnitStatus::CanNotReceiveOrders;
		}
		if (unit_ext->timers[TimerAuralAssaultBlind] == 1) {
			Unit* target = unit->order_target.unit;
			if (unit->main_order_id == OrderId::AttackUnit && target && scbw::is_allied_to(unit->player, target->player)) {
				unit->order_to_idle();
				if (unit->subunit) {
					unit->subunit->order_to_idle();
				}
			}
			unit->status &= ~UnitStatus::CanNotReceiveOrders;
		}

		// Talos - Salvage Stun
		if (unit_ext->timers[TimerSalvageStunTimer] > 1) {
			unit->freeze();

			u32 offset = scbw::get_overlay_offset(unit->id);
			if (!unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->create_top_overlay(ImageId::SalvageDisableSmall + offset);
			}
			if (unit_ext->timers[TimerTalosSoundLoopTimer] == 0) {
				helper::play_sound_from_unit(SoundId::TalosSalvageLoop, unit, 1, 0);
				unit_ext->timers[TimerTalosSoundLoopTimer] = 63;
			}
		}
		if (unit_ext->timers[TimerSalvageStunTimer] == 1) {
			unit->unfreeze();

			u32 offset = scbw::get_overlay_offset(unit->id);
			if (unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->remove_overlay(ImageId::SalvageDisableSmall + offset);
			}
		}

		// Cloaked units recloak after 2 seconds of being out of combat
		if (unit_ext->timers[TimerDecloak] == 1)
		{
			if (scbw::is_decloakable(unit) && units_dat::base_property[unit->id] & UnitProperty::PermanentCloak)
			{
				helper::anim_cloak(unit, SoundId::Terran_PHOENIX_TPhClo00_WAV);
			}
			else
			{
				unit->detection_status = 0xff;
				helper::update_visibility(unit); // didact cloak needs special handling
			}
		}

		// Autovitality - Simulacrum behavior
		if (unit_ext->timers[TimerSimulacrumClone] == 1)
		{
			scbw::create_unit_from_ability(UnitId::ProtossSimulacrum, 1, unit->player, unit->position.x, unit->position.y, 0, unit);
		}

		// Remove Ensnaring Brood overlay
		if (unit_ext->timers[TimerEnsnaringBrood] == 1)
		{
			unit->remove_overlay(ImageId::EnsnaringBroodOverlay);
		}

		// Remove Swarming Omen overlay
		if (unit_ext->timers[TimerSwarmingOmen] == 1)
		{
			// unit->remove_overlay();
		}

		// Crucible Stacks Clear
		{
			auto& crucibles = unit_ext->unit_vectors[UnitVectorCrucibles];
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
			EntryArray units = {};
			unit_search_radial(&units, unit->position, 8 * 32);
			unit_search_filter(&units, unit, FilterAlly | FilterBuilding);
			for (u32 i = 0; i < units.count; i++)
			{
				Unit* target = units.units[i];
				UnitExt* target_ext = get_unit_ext(target);
				if (scbw::unit_id_is_tech(target->id)) {
					if (units_dat::group_flags[target->id].is_terran && !target_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
						continue;
					}
				}
				switch (target->id) {
					case UnitId::TerranSupplyDepot:
					case UnitId::TerranBunker:
					case UnitId::TerranReservoir: {
						if (!target_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
							continue;
						}
					} break;
				}
				if (!(target->status & UnitStatus::Completed) && target->id != UnitId::ZergNydusCesiant) {
					continue;
				}

				UnitExt* targetExt = get_unit_ext(target);
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
			&& !unit_ext->timers[TimerEmergencyDetonation] && !unit_ext->timers[TimerControlledDemolition])
		{
			scbw::set_generic_rust_value(unit, RustId::CastControlledDemolition, 0);
			unit_ext->timers[TimerControlledDemolition] = ControlledDemolitionTimer;
			unit->sprite->create_overlay(ImageId::ControlledDemolition);
		}

		if (unit_ext->timers[TimerControlledDemolition] == 7) {
			Image* overlay = unit->get_overlay(ImageId::ControlledDemolition);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Controlled Demolition - destroy structure
		if (unit_ext->timers[TimerControlledDemolition] == 1)
		{
			unit->remove();
		}

		// Emergency Detonation - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation))
		{
			scbw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
			unit_ext->timers[TimerEmergencyDetonation] = (u32)(0.5 * unit_ext->timers[TimerControlledDemolition]);
			unit_ext->timers[TimerControlledDemolition] = 0;
			unit->sprite->remove_overlay(ImageId::ControlledDemolition);
			unit->sprite->create_top_overlay(ImageId::EmergencyDetonation);
		}

		if (unit_ext->timers[TimerEmergencyDetonation] == 7) {
			Image* overlay = unit->get_overlay(ImageId::EmergencyDetonation);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Emergency Detonation - destroy structure
		if (unit_ext->timers[TimerEmergencyDetonation] == 1)
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
					target->damage_with_custom(250 * 256, damage_type::Explosive, unit, unit->player, 0, 1, false);
				}
			});
			helper::create_thingy(SpriteId::ArcliteShockCannon_Damageless, unit->sprite->position.x, unit->sprite->position.y, unit->player);
			unit->remove();
		}

		// Demiurge - Gods Made Flesh - return units to original owner
		if (unit_ext->values[ValueWorshipStack]
			&& unit_ext->units[UnitMindController] == nullptr)
		{
			helper::play_sound_from_unit(SoundId::DemiurgeWorship01, unit, 1, 0);
			unit->sprite->create_top_overlay(ImageId::Worship_Small + scbw::get_overlay_offset(unit->id), 0, 0, 0);
			unit_ext->values[ValueWorshipStack] = 0;
			unit_ext->timers[TimerGodsMadeFlesh] = 0;
			scbw::recede_mind_control(unit);
		}

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
		if (unit_ext->timers[TimerGodsMadeFlesh] == 1 && unit_ext->units[UnitMindController])
		{
			unit->remove();
		}

		// Alaszileth - Dermal Accelerant - remove overlays
		if (unit_ext->timers[TimerAlaszilethRegenBoost] == 0)
		{
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::get_overlay_offset(unit->id, true);
			if (unit->get_overlay(overlay))
			{
				unit->remove_overlay(overlay);
			}
		}

		if (unit_ext->values[ValueAI_WorkerChosenForExpansion] && unit->resource_type == ResourceType::None) {
			unit->order_to(OrderId::Move, unit_ext->values[ValueAI_WorkerChosenX], unit_ext->values[ValueAI_WorkerChosenY]);
			unit_ext->values[ValueAI_WorkerChosenForExpansion] = 0;
			unit_ext->values[ValueAI_WorkerChosenX] = 0;
			unit_ext->values[ValueAI_WorkerChosenY] = 0;
		}

		// Anticthon - Counter-Proposal - add overlay
		if (unit_ext->timers[TimerAnticthonRevive] && !unit->get_overlay(ImageId::CreepMorphOverlay)) {
			unit->sprite->create_underlay(ImageId::CreepMorphOverlay, 0, 2);
		}

		// Legionnaire - Blade Vortex - Handle debuff overlay
		u32 blade_vortex = unit_ext->timers[TimerBladeVortex];
		if (blade_vortex > 1 && !unit->get_overlay(ImageId::BladeVortexDebuffSmall + scbw::get_overlay_offset(unit->id))) {
			unit->sprite->create_top_overlay(ImageId::BladeVortexDebuffSmall + scbw::get_overlay_offset(unit->id));
		}
		else if (unit->get_overlay(ImageId::BladeVortexDebuffSmall + scbw::get_overlay_offset(unit->id)) && blade_vortex < 1) {
			unit->sprite->remove_overlay(ImageId::BladeVortexDebuffSmall + scbw::get_overlay_offset(unit->id));
		}

		// Magister - Stellar Enforcement - Handle overlay looping
		if (unit_ext->values[ValueStellarEnforcement] && unit_ext->timers[TimerStellarEnforcementOverlay] == 1) {
			unit_ext->timers[TimerStellarEnforcementOverlay] = 30;
			unit->sprite->create_top_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true));
		}
		// Handle overlays after effect is cleansed
		if (unit->get_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true)) && unit_ext->values[ValueStellarEnforcement] == 0) {
			unit->sprite->remove_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true));
		}
	}

	bool game_on() {
		set_max_sight_range<20>();

		chk_cache_size = 0;
		memset(chk_cache, 0, sizeof(chk_cache));

		quickcasting_signal = false;
		quickcasting_command_card = 0;

		return true;
	}

	bool game_end()
	{
		end_scenario();
		return true;
	}
}
