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
#include "threshold.h"
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
#include "weapons/weapon_fire.h"
#include "harvest.h"
#include "hooks/weapons/weapon_cooldown.h"
#include <time.h>
#include "trigger_system/trigger_system.h"

void shaman_nanite_field_proc(Unit* shaman, u32 heal, Unit* target) {
	if (target->get_current_hp_ingame() < target->get_max_hp_ingame()) {
		u32 overlay = ImageId::ShamanNaniteField;
		Unit* target_context = target;
		if (target->subunit) {
			target_context = target->subunit;
		}
		target_context->sprite->create_top_overlay(overlay, 0, 0, 0);
	}
	target->heal_hp(heal * 256, shaman);
}

#pragma pack(push, 8)
#include <discord_game_sdk.h>
#pragma pack(pop)

#define DiscordClientId 972072419013558326
namespace hooks {
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
					sprintf_s(activity.details, "%s", g_current_map_name);
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

	int nop_click() {
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
			quickcasting_command_card = *g_buttonset_special_buttonset_id;
		}

		if (transport_ui_unit) {
			*left_click_proc  = (InputProcsFunc*)transport_ui_click;
			*right_click_proc = (InputProcsFunc*)nop_click;
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

	// Hooked to support quasi-fog on the minimap. -Veeq7
	void __cdecl update_minimap_colors() { // 004A3A00
		u32 local_flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[*g_local_nation_id];
		for (u32 i = 0; i < 256; ++i) {
			if (local_flags & VespeneDeaths_DisableQuasifog) {
				minimap_colors->index[MinimapColor_Unexplored][i] = palette_index_ref[15];
				minimap_colors->index[MinimapColor_Explored][i]   = dark_pcx_image_map->index[15][i];
				minimap_colors->index[MinimapColor_Visible][i]    = i;
				minimap_colors->index[MinimapColor_Visible2][i]   = i;
			} else {
				minimap_colors->index[MinimapColor_Unexplored][i] = dark_pcx_image_map->index[8][i];
				minimap_colors->index[MinimapColor_Explored][i]   = dark_pcx_image_map->index[15][i];
				minimap_colors->index[MinimapColor_Visible][i]    = i;
				minimap_colors->index[MinimapColor_Visible2][i]   = i;
			}
		}
	}

	// Hooked to show doodad sprites in quasi-fog. -Pr0nogo
	b32 update_doodad_visibility(Thingy* thingy) { // 00487690
		if (thingy->sprite->flags & SpriteFlags::Hidden) {
			return false;
		}

		Sprite* sprite = thingy->sprite;
		s32 tile_x = (sprite->position.x - sprite->width / 2) / 32;
		s32 tile_y = (sprite->position.y - sprite->height / 2) / 32;
		s32 tile_w = (sprite->width + 31) / 32;
		s32 tile_h = (sprite->height + 31) / 32;
		if (tile_x < 0) {
			tile_w = tile_x + tile_w;
			tile_x = 0;
		} else if (tile_x + tile_w > g_map_tile_size->width) {
			tile_w = g_map_tile_size->width - tile_x;
		}
		if (tile_y < 0) {
			tile_h = tile_y + tile_h;
			tile_y = 0;
		} else if (tile_y + tile_h > g_map_tile_size->height) {
			tile_h = g_map_tile_size->height - tile_y;
		}

		u8 visibility_mask = 0;
		if (*g_is_in_replay && *g_replay_reveal_all) {
			visibility_mask = 0xff;
		} else {
			visibility_mask = helper::get_visibility_mask_from_position_and_size(tile_x, tile_y, tile_w, tile_h);
		}

		for (u32 player = 0; player < PlayerLimit; ++player) {
			u32 flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player];
			if (!(flags & VespeneDeaths_DisableQuasifog)) {
				visibility_mask |= 1 << player;
			}
		}

		if (sprite->visibility_status != visibility_mask) {
			helper::refresh_all_visible_images_at_screen_pos(sprite, visibility_mask);
			u8 player_visions = *g_player_visions & sprite->visibility_status;
			if (player_visions) {
				if (!(*g_player_visions & thingy->sprite->visibility_status)) {
					return true;
				}
			}
		}
		return false;
	}

	static Point16 get_target_point(Target target) {
		return target.unit ? target.unit->position : target.pt;
	}

	static bool draw_simple_line(Point16 source, Point16 target, graphics::ColorId color) {
		if (target.x != 0 || target.y != 0) {
			graphics::draw_line(source.x, source.y, target.x, target.y, color, graphics::ON_MAP);
			return true;
		}
		return false;
	}

	static Point16 draw_order_line(Point16 source, Target target, graphics::ColorId color, Unit* ignore_draw_unit = 0) {
		Point16 target_pos = get_target_point(target);
		if (target_pos.x != 0 || target_pos.y != 0) {
			if (!ignore_draw_unit || target.unit != ignore_draw_unit) {
				graphics::draw_line(source.x, source.y, target_pos.x, target_pos.y, color, graphics::ON_MAP);
			}
			return target_pos;
		}
		return source;
	}

	static void draw_transport_ui() {
		Point16 start_pos = {179, 333};
		Point16 pos = start_pos;

		if (transport_ui_unit) {
			transport_ui_unit = 0;
			helper::set_ingame_input_procs();
		}

		for (u32 i = 0; i < *g_client_selection_count; ++i) {
			Unit* transport = g_client_selection_group->unit[i];
			bool is_replay_or_witness = *g_is_in_replay || global_vars.players[*g_local_human_id].is_witness;
			if (!scbw::unit_is_allied_or_detected(transport, *g_local_nation_id) && !is_replay_or_witness) {
				continue;
			}

			u32 space_used = 0;
			struct CargoSlot {
				Unit* unit;
				u16 count;
			};
			u32 cargo_slots_count = 0;
			CargoSlot cargo_slots[TransportCap];

			for (u32 j = 0; j < TransportCap; ++j) {
				Unit* cargo_unit = transport->get_loaded_unit(j);
				if (cargo_unit) {
					space_used += units_dat::space_required[cargo_unit->id];

					bool own_unit = cargo_unit->player == *g_local_nation_id;
					for (u32 k = 0; k < cargo_slots_count; ++k) {
						if (cargo_slots[k].unit->id == cargo_unit->id &&
							cargo_slots[k].unit->player == cargo_unit->player) {
							cargo_slots[k].count++;
							goto next_unit;
						}
					}

					if (cargo_slots_count >= TransportCap) {
					next_unit:
						continue;
					}

					CargoSlot* slot = &cargo_slots[cargo_slots_count++];
					slot->unit = cargo_unit;
					slot->count = 1;
				}
			}
			

			if (!cargo_slots_count) {
				continue;
			}
			pos.y -= (cargo_slots_count / 4) * 13;
			Point16 local_pos = pos;
			pos.y -= 2;

			Token short_unit_name = get_short_unit_name(transport->id);
			if (transport->id == UnitId::TerranPhalanxSiegeMode) {
				strcat_s(short_unit_name.data, " (Sieged)");
				short_unit_name.size = strlen(short_unit_name.data);
			}

			char unit_name[MaxToken + 1] = {};
			snprintf(unit_name, 128, "%s%s [%u/%u]:", scbw::get_player_color_code(transport->player).data, short_unit_name.data, space_used, units_dat::space_provided[transport->id]);
			graphics::draw_text(local_pos.x, local_pos.y, unit_name, graphics::FONT_SMALL, graphics::CoordType::ON_SCREEN);
			local_pos.y += 13;

			u32 x_count = 0;
			for (u32 j = 0; j < cargo_slots_count; ++j) {
				CargoSlot cargo_slot = cargo_slots[j];

				Token short_unit_name = get_short_unit_name(cargo_slot.unit->id);
				if (cargo_slot.unit->id == UnitId::TerranPhalanxSiegeMode) {
					strcat_s(short_unit_name.data, " (Sieged)");
					short_unit_name.size = strlen(short_unit_name.data);
				}
				char unit_name[MaxToken + 1] = {};
				if (cargo_slot.count > 1) {
					snprintf(unit_name, 128, "%s%s (%u)", scbw::get_player_color_code(cargo_slot.unit->player).data, short_unit_name.data, cargo_slot.count);
				} else {
					snprintf(unit_name, 128, "%s%s", scbw::get_player_color_code(cargo_slot.unit->player).data, short_unit_name.data);
				}
				u32 width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(unit_name);

				RECT rect = {};
				rect.left = local_pos.x;
				rect.top = local_pos.y;
				rect.right = rect.left + width;
				rect.bottom = rect.top + 12;
				bool hover = (*g_local_nation_id == transport->player || *g_local_nation_id == cargo_slot.unit->player) && PtInRect(&rect, *(POINT*) g_mouse);
				if (hover) {
					if (cargo_slot.count > 1) {
						snprintf(unit_name, 128, BWTC_WHITE "%s (%u)", short_unit_name.data, cargo_slot.count);
					} else {
						snprintf(unit_name, 128, BWTC_WHITE "%s", short_unit_name.data);
					}
					width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(unit_name);
				}

				graphics::draw_text(local_pos.x, local_pos.y, unit_name, graphics::FONT_SMALL, graphics::CoordType::ON_SCREEN);
				if (hover && !transport_ui_unit) {
					transport_ui_unit = cargo_slot.unit;
					helper::set_ingame_input_procs();
				}

				if (j < cargo_slots_count - 1) {
					local_pos.x += width + 5;
					x_count++;
					if (x_count > 4) {
						x_count = 0;
						local_pos.x = start_pos.x;
						local_pos.y += 13;
					}
				}
			}
		}
	}

	static void draw_custom_ui() {
		// Draw selection info
		Unit* selected = *g_active_portrait_unit;
		if (selected) {
			for (u32 i = 0; i < *g_client_selection_count; ++i) {
				Unit* unit = g_client_selection_group->unit[i];
				Point16 source = unit->position;

				// For checking cargo-related elements.
				// This helped me solve a Bunker issue on 3 February 2023, please leave it here for the future. -Pr0nogo
				/*
				if (unit) {
					for (u32 i = 0; i < TransportCap; ++i) {
						Unit* cargo_unit = selected->get_loaded_unit(i);
						if (!cargo_unit) {
							continue;
						}
						u32 unit_order = cargo_unit->main_order_id;
						scbw::printf("Order id %u", unit_order);
					}
				}
				*/

				// Show connected Nyduses
				if (unit->id == UnitId::ZergNydusCesiant && unit->building.nydus_exit) {
					if (scbw::is_allied_to(unit->player, *g_local_nation_id) || helper::get_detecting_players(unit) & (1 << *g_local_nation_id)) {
						draw_simple_line(source, unit->building.nydus_exit->position, graphics::PURPLE);
					}
				}
				bool in_replay_or_witness = global_vars.players[*g_local_nation_id].is_witness || *g_is_in_replay;
				if ((!in_replay_or_witness && config.show_order_lines && scbw::is_allied_to(*g_local_nation_id, unit->player))
				||   (in_replay_or_witness && config.witness_show_order_lines)) {
					Unit* ignore_resource_unit = 0;

					// Worker harvest links
					if (units_dat::base_property[unit->id] & UnitProperty::Worker
						&& scbw::order_id_is_harvest_related(unit->main_order_id)) {
						Unit* target_resource = unit->worker.target_resource.unit;
						if (target_resource) {
							graphics::ColorId color = graphics::CYAN;
							if (config.show_unique_gas_order_line_color && scbw::unit_id_is_gas_structure(target_resource->id)) {
								color = graphics::GREEN;
							}
							draw_simple_line(source, target_resource->position, color);
							ignore_resource_unit = target_resource;
						}
					}

					// Rally points
					{
						Point16 target = get_target_point(unit->rally);
						if (target.x == unit->position.x &&
							target.y == unit->position.y) {
							target.x = 0;
							target.y = 0;
						}
						if (!hooks::can_make_psi_field(unit->id) && draw_simple_line(source, target, graphics::WHITE)) {
							source = target;
						}
						else if (unit->id == UnitId::ZergEgg && unit->connected_unit) {
							source = draw_order_line(source, unit->connected_unit->rally, graphics::GREY);
						}
					}

					// Main orders
					{
						graphics::ColorId color = graphics::GREEN;
						switch (unit->main_order_id) {
						case OrderId::Patrol:
						case OrderId::ReturnMinerals:
						case OrderId::ReturnGas:
							// TODO: Add more neutral orders
						{
							color = graphics::YELLOW;
						} break;
						case OrderId::AttackMove:
						case OrderId::AttackUnit:
							// TODO: Add more aggressive orders
						{
							color = graphics::RED;
						} break;
						}

						source = draw_order_line(source, unit->order_target, color, ignore_resource_unit);
					}

					// Queued orders
					if ((config.show_queued_order_lines || in_replay_or_witness && config.witness_show_order_lines)
						&& !ignore_resource_unit) { // Queued orders never work when harvesting, therefore we will not show them
						for (Order* order = unit->order_queue_head; order; order = order->next) {
							source = draw_order_line(source, order->target, graphics::GREY);
						}
					}
				}
			}
		}

		draw_transport_ui();
	}

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
		if (!scbw::is_in_replay() || *g_elapsed_time_frames == 0) {
			g_options_list->speed = 6; // Lock game speed at fastest
		}

		update_discord(true);

		handle_screen_hotkeys();
		if (scbw::is_game_paused()) {
			return true;
		}

		// This block is executed once every game
		if (*g_elapsed_time_frames == 0) {
#ifdef CM_DEBUG
			helper::play_sound_global(SoundId::Misc_Buzz_wav_1);
			scbw::print("\tGPTP was compiled in debug!\n\tThis will make the game run slower.", GameTextColor::Yellow);
#endif
			if (game_data->teams_enabled) {
				scbw::print("Teams are enabled.");
			}
			init_screen_locations();
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
			update_search_index();
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

		if (*g_elapsed_time_frames % 2 == 0) {
			for (u32 i = 0; i < 16; ++i) {
				Ping* ping = ping_array + i;
				if (ping->timer < 8) {
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
			for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id) {
				u32 flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player_id];
				if (flags & VespeneDeaths_Reveal) {
					reveal_flag |= 1 << player_id;
				}
				if (flags & VespeneDeaths_Ping) {
					do_ping[player_id] = true;
				}
			}

			// Explores the entire map, requires a trigger that sets 1 death of Vespene Geyser for players in question
			if (reveal_flag) {
				// Explore map
				/*
				for (u32 x = 0; x < g_map_tile_size->width; x++) {
					for (u32 y = 0; y < g_map_tile_size->height; y++) {
						ActiveTile& tile = scbw::get_active_tile_at(x * 32, y * 32);
						tile.exploredFlags &= ~reveal_flag;
					}
				}
				*/

				// Reveal resources
				for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
					if (scbw::unit_id_is_resource_node(unit->id, false)) {
						unit->sprite->visibility_status |= reveal_flag;
					}
				}
			}

			for (u32 target = 0; target < PLAYABLE_PLAYER_COUNT; ++target) {
				if (do_ping[target]) {
					u32 color = 0; // red
					if (scbw::is_allied_to(*g_local_nation_id, target)) {
						color = 8; // green
					}

					scbw::minimap_ping(g_start_positions[target].x, g_start_positions[target].y, color, *g_local_nation_id);
				}
			}
		}

		*g_ai_update_cooldown = 0; // We want the AI to update spending requests every tick, so we can have precise 
		*g_can_update_powered_status = true; // We want to have dynamic power updates, so we update power every frame
		for (u32 i = 0; i < 8; ++i) {
			g_race_supply[RaceId::Terran].provided[i] = 60000;
		}

		scbw::set_ingame_loop_state(true); // Needed for scbw::random() to work
		hooks::update_psi_field_providers();

		reset_search_index();
		update_search_index();
		globals.frame_hook();
		update_search_index_effects();
		update_ext_variables();
		update_threshold();

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
		if (g_is_keycode_used[VK_ESCAPE] && scbw::buttonset_id_is_submenu(*g_active_portrait_buttonset_id) && !*g_is_game_paused) {
			hooks::update_buttonset();
		}
		
		draw_custom_ui();

		// Loop through bullets
		for (Bullet* bullet = *first_bullet; bullet; bullet = bullet->next) {
			if (bullet->weapon_id == WeaponId::KalkaliskViscousAcid) {
				if (bullet->order_signal & 0x80) {
					bullet->order_signal &= ~0x80;
					globals.remove_multihit_bullet(bullet);
				}
			}
		}

		process_quickbuilding();

		// Count workers
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (!scbw::unit_id_is_resource_node(unit->id, true)) {
				continue;
			}
			UnitExt* unit_ext = get_unit_ext(unit);
			unit_ext->values[ValueAssignedHarvesters] = 0;
		}
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (!(units_dat::base_property[unit->id] & UnitProperty::Worker)) {
				continue;
			}
			if (unit->worker.target_resource.unit) {
				UnitExt* target_ext = get_unit_ext(unit->worker.target_resource.unit);
				target_ext->values[ValueAssignedHarvesters] += 1;
			}
		}

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

		return true;
	}

	// Init the unit if not already
	void init_unit(Unit* unit) {
		if (!(unit->status & UnitStatus::Completed)) {
			return;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueUnitInit] == unit->id + 1) {
			return;
		}
		unit_ext->values[ValueUnitInit] = unit->id + 1;
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
			case UnitId::ResourceMineralField:
			case UnitId::ResourceMineralFieldType2:
			case UnitId::ResourceMineralFieldType3:
			{
				u16* yield = &unit->building.resource.resource_yield;
				if (*yield >= 750) {
					*yield = 8;
				} else if (*yield >= 500) {
					*yield = 6;
				} else if (*yield >= 250) {
					*yield = 4;
				} else if (*yield > 8) {
					*yield = 2;
				} else if (*yield == 8) {
					*yield = 8;
				} else if (*yield >= 6) {
					*yield = 6;
				} else if (*yield >= 4) {
					*yield = 4;
				} else if (*yield >= 2) {
					*yield = 2;
				} else {
					*yield = 1;
				}
				update_mineral_field_image(unit);
			} break;
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			case UnitId::PoliceDispensary:
			{
				unit->building.resource.resource_yield = 6;
			} break;
			case UnitId::ResourceVespeneGeyser:
			{
				unit->building.resource.resource_yield = 4;
			} break;
			case UnitId::ResourceVespeneRidge:
			{
				unit->building.resource.resource_yield = 3;
			} break;
		}
	}

	void tick_unit(Unit* unit, bool unit_is_hidden) {
		init_unit(unit);

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
		}

		UnitExt* unit_ext = get_unit_ext(unit);
		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon) {
			UnitExt* addon_ext = get_unit_ext(unit->building.addon);
			addon_ext->units[UnitAddonParent] = unit;
		}

		/*
		if (scbw::unit_id_is_resource_node(unit->id, true)) {
			bool show = GetAsyncKeyState(VK_MENU);
			if (!show) {
				for (u32 i = 0; i < *g_client_selection_count; ++i) {
					if (unit == g_client_selection_group->unit[i]) {
						show = true;
						break;
					}
				}
			}
			if (show) {
				u32 count = unit_ext->values[ValueAssignedHarvesters];
				const u32 radius = 2;
				const u32 gap = 3;
				u32 w = count * radius * 2 + (count - 1) * gap;
				Point16 pos = { unit->position.x - w / 2, unit->get_bottom() };
				for (u32 i = 0; i < count; ++i) {
					graphics::draw_filled_circle(pos.x, pos.y, radius, graphics::CYAN, graphics::ON_MAP);
					pos.x += radius * 2 + gap;
				}
			}
		}
		*/

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

		// No Corpse Overlay
		if (unit_ext->values[ValueNoCorpse] && !unit->sprite->get_overlay(ImageId::CreepMorphOverlay)) {
			unit->sprite->create_overlay(ImageId::CreepMorphOverlay);
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

		if ((unit->id == UnitId::ProtossPariah || unit->id == UnitId::TerranStriga) && unit->order_signal & 0x20) {
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

		switch (unit->id) {
			case UnitId::ZergLarva:
			case UnitId::ZergOcsalLarva:
			{
				Unit* building = unit->connected_unit;
				if (building && building->status & UnitStatus::Completed) {
					unit_ext->values[ValueSwarmhold] = scbw::unit_id_swarmhold_value(building->id);
					u32 unit_id = scbw::unit_id_get_larva_type(building->id);
					if (unit_id != UnitId::None && unit->id != unit_id) {
						helper::replace_unit_with_id(unit, unit_id);
					}
				}
			} break;
			case UnitId::ZergEvigrilisk: {
				bool is_moving = unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving;
				bool can_fire = !unit->ground_weapon_cooldown;
				u32 weapon_id = WeaponId::EvigriliskBrutishTendrils;
				if (is_moving && can_fire) {
					if (fire_multiple(unit, weapon_id, 4, FilterEnemy, true, false, true)) {
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
							UnitExt* target_ext = get_unit_ext(target);
							target_ext->timers[TimerSalvageStunTimer] = 2 * 24;
							
							u32 time = units_dat::time_cost[target->id] / 2;
							target_ext->salvage_time += 1;
							
							target_ext->salvage_minerals += (f32)units_dat::mineral_cost[target->id] / time;
							while (target_ext->salvage_minerals > 1) {
								target_ext->salvage_minerals -= 1;
								resources->minerals[unit->player] += 1;
							}
							
							target_ext->salvage_gas += (f32)units_dat::vespene_cost[target->id] / time;
							while (target_ext->salvage_gas > 1) {
								target_ext->salvage_gas -= 1;
								resources->gas[unit->player] += 1;
							}
							
							if (target_ext->salvage_time >= time) {
								resources->minerals[unit->player] += ceilf(target_ext->salvage_minerals);
								resources->gas[unit->player] += ceilf(target_ext->salvage_gas);
								target->remove();
							}

							if (!(unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving)
								&& !unit_ext->timers[TimerTalosAnimationSubtimer]) {

								u32 width  = units_dat::building_dimensions[target->id].x * 0.75;
								u32 height = units_dat::building_dimensions[target->id].y * 0.75;

								s32 x_offset = scbw::random() % width  - width  / 2;
								s32 y_offset = scbw::random() % height - height / 2;

								x_offset = MathUtils::clamp(x_offset, -127, 128);
								y_offset = MathUtils::clamp(y_offset, -127, 128);

								target->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
								unit_ext->timers[TimerTalosAnimationSubtimer] = scbw::rand_between(3, 7);
							}

							if (!unit_ext->values[ValueTalosAnimationTimer]
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
								helper::set_move_target_xy(unit, pos.x, pos.y);
							}
							unit_ext->values[ValueTalosAnimationTimer] += 1;
						} else {
							unit_ext->values[ValueTalosAnimationTimer] = 0;
						}
					}
				}
			} break;
			case UnitId::ZergVilgorokor: {
				// Vilgorokor - Reconstitution
				if (!unit_ext->timers[TimerReconstitutionCooldown]) {
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
						//	scbw::create_and_init_thingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
						}
						scbw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
						unit_ext->timers[TimerReconstitutionCooldown] = 1 * 24;
						scbw::create_unit_from_ability(UnitId::ZergRilirokor, 2, unit->player, corpse->sprite->position);
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
						corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
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
					if (!(corpse->sprite->flags & SpriteFlags::RemoveAfterDeath)) {
						Unit* egg = scbw::create_unit_in_game(UnitId::ZergEgg, unit->player, corpse->sprite->position.x, corpse->sprite->position.y, 0, false);
						if (egg) {
							egg->sprite->play_iscript_animation(IscriptAnimation::Walking, true);
							egg->build_queue[egg->build_queue_slot] = UnitId::ZergSovroleth;

							UnitExt* eggExt = get_unit_ext(egg);
							eggExt->timers[TimerCorpseCocoonHatch] = 2 * 24;
						}
						corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
					}
				}
			} break;
			case UnitId::ZergEgg:
			case UnitId::ZergCocoon:
			case UnitId::ZergQuazilisk:
			{
				// Gimmick to allow morph cancellation animations to play correctly
				if (unit_ext->timers[TimerQuaziliskCancel]) {
					if (unit->sprite->main_graphic->animation != IscriptAnimation::CastSpell) {
						unit->play_iscript_animation(IscriptAnimation::CastSpell);
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
				if (!unit_ext->timers[TimerStarSovereignInterventionMissiles]) {
					auto radius = 5 * 32;
					auto match_func = cosmonarchy_finder_match(unit, Flags::Enemy | Flags::FrontalArc, {});
					// TODO: unit search, also wtf is the above aids? -Pr0nogo
					scbw::UnitFinder unit_finder(
						unit->position.x - radius,
						unit->position.y - radius,
						unit->position.x + radius,
						unit->position.y + radius
					);
					bool enemies_detected = false;
					for (s32 i = 0; i < unit_finder.get_unit_count(); i++) {
						Unit* picked_unit = unit_finder.get_unit(i);
						if (match_func(picked_unit)) {
							enemies_detected = true;
							break;
						}
					}
					if (enemies_detected) {
						unit_ext->timers[TimerStarSovereignInterventionMissiles] = weapons_dat::cooldown[WeaponId::StarSovereignInterventionMissiles];
						for (s32 arc = 0; arc < 6; arc++) {
							const LO_Header* loFile = lo_files->special_overlays[unit->sprite->main_graphic->id];
							u8 frame_direction = unit->sprite->main_graphic->direction;
							Point8 offset = loFile->get_offset(frame_direction, 5 - arc);
							if (unit->sprite->main_graphic->flags & ImageFlags::Mirrored) {
								offset.x = -offset.x;
							}
							auto X = unit->get_x() + offset.x;
							auto Y = unit->get_y() + offset.y;
							auto target_unit = unit->order_target.unit;
							auto target_pos = unit->order_target.pt;
							auto offset_direction = -40 + arc * 16;
							auto max_range = weapons_dat::max_range[WeaponId::StarSovereignInterventionMissiles];
							auto polar_x = unit->get_x() + scbw::get_polar_x(max_range, scbw::dir_saturate(unit->current_direction_1, offset_direction));
							auto polar_y = unit->get_y() + scbw::get_polar_y(max_range, scbw::dir_saturate(unit->current_direction_1, offset_direction));
							unit->order_target.unit = 0;
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
				if (unit_ext->timers[TimerPlanetCrackerChannel] == 1) {
					globals.create_effect(EffectId_StarSovereignTheGriefOfAllGods, 360, {unit->position.x, unit->position.y}, 8 * 32, -1, unit->sprite->elevation, unit->player, unit, unit);
				}

				// blast effects
				u32 player = unit->player;
				u16 X = unit->position.x;
				u16 Y = unit->position.y;
				if (unit->order_signal & 8) {
					unit->order_signal &= ~8;
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y + 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y + 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y - 32, player);
					helper::create_thingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y - 32, player);
				} else if (unit->order_signal & 16) {
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
				} else if (unit->order_signal & 32) {
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
				} else if (unit->order_signal & 64) {
					unit->order_signal &= ~64;
				}
			} break;
			case UnitId::TerranApostle:
			{
				// Apostle - Lazarus Agent cast
				if (scbw::get_generic_rust_value(unit, RustId::CastLazarusAgent) == 1) {
					scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 0);

					if (!unit_ext->timers[TimerLazarusCasting]) {
						u32 energy = techdata_dat::energy_cost[TechId::ApostleLazarusAgent] * 256;
						if (unit->has_energy(energy)) {
							unit->spend_unit_energy(energy);
							unit_ext->timers[TimerLazarusCasting] = 2 * 24;
							unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
							helper::play_sound_from_unit(SoundId::LazarusAgentCastStart, unit, 1, 0);
						}
					}
				}
				if (unit_ext->timers[TimerLazarusCasting]) {
					scbw::decloak_proc(unit);	
				}
				if (unit_ext->timers[TimerLazarusCasting] % 12 == 1) {
					unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
					helper::play_sound_from_unit(SoundId::LazarusAgentCastTick, unit, 1, 0);
				}
				// Apply Lazarus Agent
				if (unit_ext->timers[TimerLazarusCasting] == 1) {
					helper::play_sound_from_unit(SoundId::LazarusAgentCastEnd, unit, 1, 0);

					EntryArray corpses = {};
					corpse_search_radial(&corpses, unit->position, 2.5 * 32);
					for (u32 i = 0; i < corpses.count; i++) {
						Thingy* corpse = corpses.corpses[i];
						scbw::reclaim_corpse(corpse, unit->player);
					}
				}
			} break;
			case UnitId::ProtossLanifect:
			{
				// Disruption Field - Lanifect ability
				if (scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField)) {
					if (!unit_ext->values[ValueDisruptionFieldOn]) {
						unit_ext->values[ValueDisruptionFieldOn] = 1;
						globals.create_effect(EffectId_LanifectDisruptionField, -1, {unit->position.x, unit->position.y}, 0, SpriteId::Disruption_Web, 0, unit->player, unit, unit);
					} else if (unit->is_frozen()) {
						scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
					}
				} else {
					unit_ext->values[ValueDisruptionFieldOn] = 0;
				}
				if (unit_ext->values[ValueDisruptionFieldOn]) {
					scbw::decloak_proc(unit);
				}
			} break;
			case UnitId::TerranMadcap:
			{
				// Madcap Cabeirus Feeders - subtract stacks while moving
				if (unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving) {
					if (unit_ext->values[ValueMadcapStacks] > 0) {
						unit_ext->values[ValueMadcapStacks]--;
					}
				}
			} break;
			case UnitId::TerranHeracles:
			{
				// Heracles Nemean Reactors - remove stacks when out of combat
				if (unit_ext->values[ValueNemeanReactorsStack] > 0
				&&  unit_ext->timers[TimerNemeanReactors] <= 1) {
					unit_ext->values[ValueNemeanReactorsStack] = 0;
				}
			} break;
			case UnitId::TerranCuirass:
			{
				if (unit_ext->values[ValueCuirassIsSelfReviving]) {
					if (unit->hitpoints + CuirassSelfHeal >= unit->get_max_hp_raw()) {
						unit_ext->values[ValueCuirassIsSelfReviving] = 0;
						unit->hitpoints = unit->get_max_hp_raw();
						scbw::restore_func(unit);
						unit->order_computer(OrderId::Stop);
						unit->status &= ~(UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
						helper::play_sound_from_unit(SoundId::Terran_GOLIATH_TGoWht03_WAV, unit, 1, 0);
					} else {
						unit->heal_hp(CuirassSelfHeal);
						if (!unit->sprite->get_overlay(ImageId::StonewallProtocolOverlay)) {
							unit->sprite->create_top_overlay(ImageId::StonewallProtocolOverlay, -3, -8);
						}
					}
				}
			} break;
			case UnitId::ProtossLegionnaire:
			{
				// Legionnaire - Blade Vortex
				// Striga was here - removed 7 February 2023. -Pr0nogo
				if (unit->order_target.unit && (unit->main_order_id == OrderId::Attack1 || unit->main_order_id == OrderId::AttackUnit) && !unit->ground_weapon_cooldown) {
					Unit* target = unit->order_target.unit;
					u32 distance = unit->get_distance_to_target(target);
					bool do_teleport = false;
					u32 image_id = 0;

					const u32 min_radius = 0.75 * 32;
					const u32 max_radius = 3 * 32;

					if (unit->id == UnitId::ProtossLegionnaire) {
						if (distance > min_radius && distance <= max_radius && scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0) {
							do_teleport = true;
							image_id = ImageId::LegionnaireTeleportEffect;
						}
					}

					if (do_teleport) {
						bool teleport = scbw::teleport_to_unit(unit, target, min_radius, 0, true);
						if (!unit->get_overlay(image_id) && teleport) {
							unit->sprite->create_top_overlay(image_id);
						}
					}
				}
			} break;
			case UnitId::TerranPaladin:
			{
				// Paladin - Skyfall
				if (unit_ext->timers[TimerSkyfall] == 1 && !unit->is_frozen()) {
					u8 missile = unit_ext->values[ValueSkyfallStacks];
					unit_ext->values[ValueSkyfallStacks]++;
					if (missile == 20) {
						unit_ext->values[ValueSkyfallStacks] = 0;
					} else {
						unit_ext->timers[TimerSkyfall] = 2 + 1;
						u32 target_x = unit_ext->values[ValueSkyfallX];
						u32 target_y = unit_ext->values[ValueSkyfallY];

						u32 column = missile / 4;
						u32 row = missile % 4;
						u32 distance = scbw::get_distance_fast(unit->get_x(), unit->get_y(), target_x, target_y);
						f64 angle = scbw::bw_angle_to_normal(scbw::get_angle(unit->get_x(), unit->get_y(), target_x, target_y));
						Vector32 rotation_pt = {0, 0};
						rotation_pt.x += 16 + (row - 2) * 32;
						rotation_pt.y -= (column - 2) * 32;
						auto product = scbw::rotate_point({0, 0}, rotation_pt, angle);
						Point16 missile_target;
						missile_target.x = target_x + product.x;
						missile_target.y = target_y + product.y;

						s32 forwardOffset = weapons_dat::forward_offset[10];
						auto source_x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
						auto source_y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1) - weapons_dat::vertical_offset[10];
						unit->order_target.unit = 0;
						unit->order_target.pt = missile_target;
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
				if (unit->sprite && unit->main_order_id != OrderId::Die
					&& unit_ext->timers[TimerLakiziliskETA] == 1) {
					auto position = unit->position; // new target
					auto spr_position = unit->sprite->position;
					Point16 pos;
					pos.x = unit_ext->values[ValueSpineX];
					pos.y = unit_ext->values[ValueSpineY];
					unit->position = pos;
					unit->sprite->position = pos;
					scbw::artificial_bullet_from(WeaponId::LakiziliskSubterraneanSpines, unit->player, unit, 0, spr_position);
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
				u32 timer = unit_ext->timers[TimerNaniteField];
				if (!timer) {
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
				// Start the sequence
				Unit* target = unit->order_target.unit;
				switch (unit->sprite->main_graphic->animation) {
					case IscriptAnimation::GndAttkInit:
					case IscriptAnimation::GndAttkRpt:
					case IscriptAnimation::AirAttkInit:
					case IscriptAnimation::AirAttkRpt: {
						if (target && unit->get_distance_to_target(target) <= CentaurMissileDistance
							&& !unit_ext->timers[TimerSecondaryWeaponCooldown]) {
							unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::CentaurSunchaserMissiles);
							unit_ext->values[ValueCentaurSecondaryWeaponCount] = 4;
							unit_ext->units[UnitCentaurSecondaryWeaponTarget] = target;
							helper::play_sound_from_unit(scbw::rand_between(SoundId::CentaurSunchaserMissilesFire01, SoundId::CentaurSunchaserMissilesFire04), unit);
						}
					} break;
				}
				// Progress the sequence
				target = unit_ext->units[UnitCentaurSecondaryWeaponTarget];
				if (target && unit_ext->values[ValueCentaurSecondaryWeaponCount] && !unit_ext->timers[TimerCentaurSecondaryWeaponStagger]) {
					unit_ext->values[ValueCentaurSecondaryWeaponCount]--;
					unit_ext->timers[TimerCentaurSecondaryWeaponStagger] = hooks::get_modified_cooldown(unit, 4, false);
					WeaponOffsets offsets = {};
					offsets.value_id = ValueCentaurLaunchPos;
					offsets.count = 4;
					fire_bullet(unit, WeaponId::CentaurSunchaserMissiles, target, &offsets, false);
				}
			} break;
			case UnitId::TerranOlympian:
			{
				Unit* target = unit->order_target.unit;
				if (unit->order_signal & 0x20 && unit->ground_weapon_cooldown <= 1) {
					unit->order_signal &= ~0x20;
					if (target && !(units_dat::base_property[target->id] & UnitProperty::Flyer) && unit->get_distance_to_target(target) <= OlympianMeleeDistance) {
						unit->play_iscript_animation(IscriptAnimation::SpecialState1);
					}
				}
			} break;
			case UnitId::ZergMatraleth:
			case UnitId::InfestedKerriganExt:
			{
				// Matraleth - Ensnaring Brood
				Point16 pos = unit->position;
				s32 radius = 5 * 32;
				// TODO: unit search
				scbw::UnitFinder(
					pos.x - radius,
					pos.y - radius,
					pos.x + radius,
					pos.y + radius
				).for_each([&](Unit* target) {
					if (!(scbw::is_allied_to(target->player, unit->player))) {
						return;
					}
					if  (units_dat::base_property[target->id] & UnitProperty::Building
					|| !(target->status & UnitStatus::Completed)) {
						return;
					}

					// Overlays
					UnitExt* target_ext = get_unit_ext(target);
					target_ext->timers[TimerEnsnaringBrood] = 3;
					if (!target->get_overlay(ImageId::EnsnaringBroodOverlay)) {
						target->sprite->create_top_overlay(ImageId::EnsnaringBroodOverlay);
					}
					});
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Phase Link
				if (scbw::get_aise_value(unit, 0, AiseId::SendClarionValue, 0, 0) == 0) {
					scbw::send_gptp_aise_command(unit, 0, GptpId::SetPhaseLinkStatus, 1, 0, 0);
				}
			} break;
			case UnitId::ProtossDemiurge:
			{
				// Demiurge - Gods Made Flesh - initial cast
			//	if (scbw::get_generic_rust_value(unit, RustId::CastGodsMadeFlesh) && !unit_ext->values[ValueGodsMadeFleshActive]) {
			//		scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 0);
			//		scbw::decloak_proc(unit);
			//		unit_ext->values[ValueGodsMadeFleshActive] = 1;
			//		helper::play_sound_from_unit(scbw::rand_between(SoundId::DemiurgeGodsMadeFleshCast01, SoundId::DemiurgeGodsMadeFleshCast02), unit, 1, 0);
			//		unit->sprite->create_top_overlay(ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(unit->id));
			//	}
				// Demiurge - Gods Made Flesh - apply link
			//	if (unit_ext->values[ValueGodsMadeFleshActive]) {
			//		for (Unit* target : unit_ext->unit_vectors[UnitVectorWorshipersDemiurge]) {
			//			UnitExt* target_ext = get_unit_ext(target);
			//		//	target_ext->timers[TimerGodsMadeFlesh] = 3;
			//			u32 overlay = ImageId::GodsMadeFlesh_Small + scbw::get_overlay_offset(target->id);
			//			if (!target->get_overlay(overlay)) {
			//				target->sprite->create_top_overlay(overlay);
			//			}
			//		}
			//		if (!unit->energy) {
			//			unit_ext->values[ValueGodsMadeFleshActive] = 0;
			//		}
			//	}
			} break;
			case UnitId::TerranMaverick:
			{
				// Decloak on Stimpack activation
				if (unit->stim_timer == StimpackTimer) {
					scbw::decloak_proc(unit);
				}
			} break;
			case UnitId::TerranWyvern:
			{
				if (scbw::get_generic_rust_value(unit, RustId::CancelLandGrab)) {
					scbw::set_generic_rust_value(unit, RustId::CancelLandGrab, 0);
					unit_ext->values[ValueWyvernLandGrabOn] = 0;
				}
			} break;
			case UnitId::TerranTarasque:
			{
				// Forward March
				if (!(unit->status & UnitStatus::InTransport) && !unit_ext->timers[TimerSecondaryWeaponCooldown]) {
					if (fire_multiple(unit, WeaponId::TarasqueAcceleratedPlasmid, 5, FilterEnemy, false, true, false)) {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::TarasqueAcceleratedPlasmid);
					} else {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranAnticthon:
			{
				// Revive status
				if (!unit_ext->values[ValueReviveStarted]) {
					EntryArray corpses = {};
					corpse_search_radial(&corpses, unit->position, 3 * 32);
					for (u32 i = 0; i < corpses.count; ++i) {
						Thingy* corpse = corpses.corpses[i];
						scbw::reclaim_corpse(corpse, unit->player);
					}
				}
			} break;
			case UnitId::ProtossAnthelion:
			{
				if (!unit_ext->timers[TimerAnthelionAttackCooldown]) {
					bool attacked = false;
					EntryArray targets = {};
					unit_search_radial(&targets, unit->position, weapons_dat::outer_splash_radius[WeaponId::AnthelionEventHorizon]);
					unit_search_filter(&targets, unit, FilterEnemy);
					for (u32 i = 0; i < targets.count; ++i) {
						Unit* target = targets.units[i];
						u32 dist = scbw::get_distance_fast(unit->position, target->position);
						if (dist <= weapons_dat::outer_splash_radius[WeaponId::AnthelionEventHorizon]) {
							UnitExt* target_ext = get_unit_ext(target);
							if (!(units_dat::base_property[target->id] & UnitProperty::Building)) {
								target_ext->timers[TimerAnthelionGravityWell] = 24;
								target_ext->units[UnitAnthelionGravityWellSource] = unit;
							}

							u32 divisor = 4;
							if (dist <= weapons_dat::inner_splash_radius[WeaponId::AnthelionEventHorizon]) {
								divisor = 1;
							} else if (dist <= weapons_dat::medium_splash_radius[WeaponId::AnthelionEventHorizon]) {
								divisor = 2;
							}
							target->damage_with(WeaponId::AnthelionEventHorizon, -1, unit, unit->player, 0, divisor);
							helper::create_thingy(SpriteId::NeutronFlareOrange_Damageless, target->position.x, target->position.y, unit->player);
							attacked = true;
						}
					}
					if (attacked) {
						unit_ext->timers[TimerAnthelionAttackCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::AnthelionEventHorizon);
						unit->sprite->create_underlay(ImageId::OpticalFlareHit_Large);
						helper::play_sound_from_unit(scbw::rand_between(SoundId::AnthelionWeaponFire01, SoundId::AnthelionWeaponFire04), unit, 1, 0);
					}
				}

				if (unit->main_order_id == OrderId::Follow || unit->main_order_id == OrderId::AttackUnit) {
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

						if (unit_ext->timers[TimerAnthelionEndlessEclipse] == 1) {
							Point16 old_pos = unit->position;
							bool teleport = scbw::teleport_to_point(unit, unit->move_target.pt, 0, true);
							if (teleport) {
								scbw::create_and_init_thingy(SpriteId::AnthelionTeleportDepart, old_pos.x, old_pos.y, unit->player, 11);
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
				// TODO: unit search
				u32 radius = 6 * 32;
				auto match = cosmonarchy_finder_match(unit, Flags::Enemy, {});
				bool attacked = false;
				scbw::UnitFinder(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius
				).for_each_in_match(match, [&](Unit* target) {
					if ((!scbw::is_allied_to(unit->player, target->player))) {
						UnitExt* targetExt = get_unit_ext(target);
						if (!targetExt->timers[TimerObservanceDebuff]) {
							unit_ext->timers[TimerObservanceOverlay] = 2;
							targetExt->timers[TimerObservanceDebuff] = 5 * 24 + 1;
							targetExt->timers[TimerObservancePeriod] = 5 * 24 + 1;
							if (!targetExt->values[ValueObservanceRend]) {
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
				// TODO: unit search
				scbw::UnitFinder(
					unit->get_x() - 128,
					unit->get_y() - 128,
					unit->get_x() + 128,
					unit->get_y() + 128
				).for_each([&](Unit* target) {
					if (target == unit) {
						return;
					}
					if (scbw::unit_id_is_resource_node(target->id, false)) {
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
				&& !unit_ext->timers[TimerAxitriliskShieldLossSFX]) {
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
					if (!unit_ext->values[ValueMissileSiloLoaded] && unit->building.silo.nuke) {
						unit_ext->values[ValueMissileSiloLoaded] = 1;
						unit->sprite->main_graphic->play_iscript_animation(IscriptAnimation::Unused2);
					} else if (unit_ext->values[ValueMissileSiloLoaded] == 1 && !unit->building.silo.nuke) {
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
				&&  unit->build_queue[unit->build_queue_slot] != UnitId::ZergGosvileth) {
					unit->build_queue[unit->build_queue_slot] = UnitId::ZergGosvileth;
				}
			} break;
			case UnitId::TerranAncile: {
				// Ancile - Providence
				if (unit->energy >= 1 * 256) {
					if (!unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) {
						unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlayLarge);
					}

					const u32 radius = 2 * 32;
					EntryArray bullets = {};
					bullet_search_radial(&bullets, unit->position, radius);
					for (u32 i = 0; i < bullets.count; i++) {
						Bullet* bullet = bullets.bullets[i];

						// Check if the unit can be shielded
						Unit* target = bullet->attack_target.unit;
						if ((target && (!scbw::is_allied_to(unit->player, target->player) || target->player == NeutralPlayer)) || bullet->hit_flags & 1) {
							// We skip flagged bullets, so we don't have an infinite cycle of redirecting
							continue;
						}

						// Check if the target is actually shielded (in area)
						Point16 target_position = bullet->attack_target.unit ? bullet->attack_target.unit->position : bullet->attack_target.pt;
						if (scbw::get_distance_fast(unit->position, target_position) > radius) {
							continue;
						}

						// Redirect the hitscan bullet, hitscan attacks themselves are handled in weapon damage
						switch (weapons_dat::behavior[bullet->weapon_id]) {
							case WeaponBehavior::AppearOnAttacker:
							case WeaponBehavior::AppearOnTargetSite:
							case WeaponBehavior::AppearOnTargetUnit:
							{
								bullet->position = unit->position;
								if (bullet->sprite) {
									bullet->sprite->set_position(unit->position.x, unit->position.y);
									bullet->sprite->elevation = unit->sprite->elevation + 1;
								}
								continue;
							} break;
						}

						// Redirect
						if (target) {
							scbw::create_shield_overlay(target, bullet->direction_1);
						}
						scbw::set_bullet_target(bullet, unit);
						scbw::set_bullet_direction(bullet, unit);
						bullet->hit_flags |= 1; // We set missed flag, so the bullet doesn't deal hp damage (this doesn't help with hitscan or melee)
						if (scbw::weapon_id_is_standard_weapon(bullet->weapon_id)) {
							unit->spend_unit_energy(helper::get_unit_bullet_damage(bullet, unit));
						}
						helper::play_sound_from_unit(scbw::rand_between(SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03), unit, 1, 0);

						if (unit->energy < 1 * 256) {
							break;
						}
					}
				} else if (unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) {
					unit->remove_overlay(ImageId::SublimeShepherdOverlayLarge);
					helper::play_sound_from_unit(SoundId::AncileProvidenceDeactivate, unit, 1, 0);
				}
			} break;
			case UnitId::TerranDurendal:
			{
				// Durendal - Forward March
				if (!(unit->status & UnitStatus::InTransport) && !unit_ext->timers[TimerSecondaryWeaponCooldown]) {
					if (fire_multiple(unit, WeaponId::DurendalPhilistineAutogun, 1, FilterEnemy, false, true, false)) {
						unit->sprite->create_top_overlay(ImageId::DurendalWeaponfireOverlay);
						helper::play_sound_from_unit(scbw::rand_between(SoundId::DurendalSecondaryFire01, SoundId::DurendalSecondaryFire03), unit, 1, 0);
						unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::DurendalPhilistineAutogun);
					} else {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranHypnagogue:
			{
				// Hypnagogue - Through the Looking Glass
				if (unit->main_order_id == OrderId::CastThroughTheLookingGlass) {
					scbw::decloak_proc(unit);
					Point16 target_pos = {};
					if (unit->order_target.pt.x && unit->order_target.pt.y) {
						target_pos = unit->order_target.pt;
					} else if (unit->order_target.unit) {
						target_pos.x = unit->order_target.unit->get_x();
						target_pos.y = unit->order_target.unit->get_y();
					}
					unit_ext->timers[TimerHypnagogueChannel] = 48 + 1;
					unit_ext->values[ValueThroughTheLookingGlassX] = target_pos.x;
					unit_ext->values[ValueThroughTheLookingGlassY] = target_pos.y;
				//	unit->spend_unit_energy(techdata_dat::energy_cost[TechId::HypnagogueThroughTheLookingGlass] * 256); // Please add unit->has_energy(...) check if this is reenabled -Veeq7
				}
				if (unit_ext->timers[TimerHypnagogueChannel] == 1) {
					Point16 target_pos = {};
					target_pos.x = unit_ext->values[ValueThroughTheLookingGlassX];
					target_pos.y = unit_ext->values[ValueThroughTheLookingGlassY];
					Point16 pos = unit->position;
					s32 radius = 2 * 32;
					scbw::UnitFinder(
						pos.x - radius,
						pos.y - radius,
						pos.x + radius,
						pos.y + radius
					).for_each([&](Unit* target) {
						if (units_dat::base_property[target->id] & UnitProperty::Building || !(target->status & UnitStatus::Completed)) {
							return;
						}
						scbw::teleport_to_point(target, target_pos);
					});
					scbw::teleport_to_point(unit, target_pos);
				}
			} break;
			case UnitId::ProtossOptecton:
			{
				// Optecton - Forward March
				if (!(unit->status & UnitStatus::InTransport) && !unit_ext->timers[TimerSecondaryWeaponCooldown]) {
					if (fire_multiple(unit, WeaponId::OptectonSkybondRay, 4, FilterEnemy | FilterNoGround, false, false, false)) {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::OptectonSkybondRay);
						helper::play_sound_from_unit(scbw::rand_between(SoundId::OptectonWeaponfireSecondary01, SoundId::OptectonWeaponfireSecondary03), unit, 1, 0);
					} else {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::ProtossSycophant:
			{
				if (unit->shields) {
					EntryArray unit_array = {};
					unit_search_radial(&unit_array, unit->position, 3 * 32);
					unit_search_add_units_from_connected_transport(&unit_array, unit);
					unit_search_filter(&unit_array, unit, FilterAlly | FilterNotSelf | FilterCompleted);
					unit_search_sort_by_distance(&unit_array, unit->position);

					u32 shields = 6 * 256 / 24;
					if (shields > unit->shields) {
						shields = unit->shields;
					} 
					for (u32 i = 0; i < unit_array.count; ++i) {
						Unit* target = unit_array.units[i];
						if (target->shields < unit->shields) {
							shields -= target->heal_shields(shields, unit, false, false);
							unit->shields -= shields;
							unit->set_hitpoints(unit->hitpoints);
							break;
						}
					}
				}
			} break;
			case UnitId::TerranMatador:
			{
				if (unit->movement_flags & MovementFlags::Accelerating || unit->movement_flags & MovementFlags::Moving)
				{
					unit_ext->timers[TimerMatadorHeal] = 1 * 24;
				}

				if (unit->hitpoints != unit->get_max_hp_raw() && unit_ext->timers[TimerMatadorHeal] == 0 && !unit->sprite->get_overlay(ImageId::RestorationHit_Small)) {
					unit->sprite->create_top_overlay(ImageId::RestorationHit_Small);
				}
			} break;
			case UnitId::ZergMortothrokor:
			{
				if (unit_ext->timers[TimerMortothrokorPassive] == 0) {
					unit_ext->values[ValueMortothrokorStacks] = 0;
				}
			} break;
			case UnitId::ProtossSteward:
			{
				// Evasive Maneuvers - Return to Solarion
				if (unit->hangar_unit.parent && unit->hitpoints < 30 * 256)
				{
					helper::issue_order_targeting_nothing(unit, OrderId::Return);
				}
			} break;
			case UnitId::ProtossSolarion:
			{
				// Auto-train Stewards
				if (unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count < 12 && unit->build_queue[unit->build_queue_slot] != UnitId::ProtossSteward)
				{
					unit->build_queue[unit->build_queue_slot] = UnitId::ProtossSteward;
					unit->set_secondary_order(OrderId::TrainFighter);
					unit->secondary_order_state = 0;
				}
			} break;
			case UnitId::TerranBunker:
			{
				if (unit->status & UnitStatus::InAir) {
					for (u32 i = 0; i < TransportCap; ++i) {
						Unit* cargo_unit = unit->get_loaded_unit(i);
						if (!cargo_unit) {
							continue;
						}
						if (cargo_unit->main_order_id != OrderId::BunkerGuard) {
							scbw::print("lol");
							helper::order(1, cargo_unit, OrderId::BunkerGuard, cargo_unit->position, 0, UnitId::None);
						}
					}
				}
			} break;
			case UnitId::TerranMadrigal:
			{
				if (scbw::get_generic_rust_value(unit, RustId::CastToggleTempoChange)) {
					scbw::set_generic_rust_value(unit, RustId::CastToggleTempoChange, 0);
					if (!unit_ext->timers[TimerMadrigalTempoChange]) {
						unit_ext->timers[TimerMadrigalTempoChange] = 18;
						unit_ext->values[ValueMadrigalTempoDown] = !unit_ext->values[ValueMadrigalTempoDown];
						unit->status |= UnitStatus::CanNotAttack;
						unit->sprite->create_overlay(ImageId::StonewallProtocolOverlay);
						if (unit_ext->values[ValueMadrigalTempoDown]) {
							helper::play_sound_from_unit(SoundId::MadrigalTempoChangeDown, unit);
						} else {
							helper::play_sound_from_unit(SoundId::MadrigalTempoChangeUp, unit);
						}
					}
				}
				if (unit_ext->timers[TimerMadrigalTempoChange]) {
					scbw::decloak_proc(unit);
				}
				if (unit_ext->timers[TimerMadrigalTempoChange] == 1) {
					unit->status &= ~UnitStatus::CanNotAttack;
					unit->remove_overlay(ImageId::StonewallProtocolOverlay);
				}
			} break;
		}

		// Gladius Binding Plasma
		if (unit_ext->timers[TimerGladiusBindingPlasma]) {
			u32 source_player = unit_ext->values[ValueGladiusBindingPlasmaPlayer];

			bool has_source = false;
			EntryArray sources = {};
			unit_search_radial(&sources, unit->position, 8 * 32);
			for (u32 i = 0; i < sources.count; ++i) {
				Unit* source = sources.units[i];
				if (source->id == UnitId::ProtossGladius && scbw::is_allied_to(source->player, source_player)) {
					has_source = true;
					break;
				}
			}

			if (has_source) {
				if (unit_ext->timers[TimerGladiusBindingPlasma] % 12 == 11) {
					unit->sprite->create_overlay(ImageId::BindingPlasmaOverlay);
				//	helper::play_sound_from_unit(SoundId::LazarusAgentCastStart, unit, 1, 0); // TODO: Audio
				}
				if (unit_ext->timers[TimerGladiusBindingPlasma] == 1) {
					scbw::binding_plasma_proc(unit);
				}
			} else {
				unit_ext->timers[TimerGladiusBindingPlasma] = 0;
				unit_ext->values[ValueGladiusBindingPlasmaStacks] = 0;
			}
		} else {
			unit_ext->values[ValueGladiusBindingPlasmaStacks] = 0;
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
					unit->unfreeze();
					u32 offset = scbw::get_overlay_offset(unit->id, true);
					unit->remove_overlay(ImageId::TyrannyStunSmall + offset);
				}
				unit_ext->timers[TimerTyrannyRecede] = 5 * 24;
			}
			if (unit_ext->timers[TimerTyrannyRecede] <= 4 * 24) {
				unit->freeze();
				u32 offset = scbw::get_overlay_offset(unit->id, true);
				if (!unit->get_overlay(ImageId::TyrannyStunSmall + offset)) {
					unit->sprite->create_top_overlay(ImageId::TyrannyStunSmall + offset);
				}
			}
			if (!unit_ext->timers[TimerTyrannyRecede]) {
				unit->unfreeze();
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
			&& unit->sprite->main_graphic->animation != IscriptAnimation::WalkingToIdle) {
			unit->movement_flags &= ~MovementFlags::Accelerating;
		}

		// Fix AI melee units freezing - DarkenedFantasies
		if (unit->status & UnitStatus::Unmovable
		&&  unit->main_order_id & OrderId::AttackUnit
		&& !unit->movement_flags
		&&  unit->movement_state == 0x0A) {
			unit->status ^= UnitStatus::Unmovable;
		}

		// Sublime Shepherd - Check tagged units for nearby Azazels
		unit_ext->values[ValueSublimeShepherd] = 0;
		if (unit_ext->values[ValueHasSublimeShepherd]) {
			unit_ext->values[ValueHasSublimeShepherd] = 0;
			scbw::UnitFinder(
				unit->get_x() - 128,
				unit->get_y() - 128,
				unit->get_x() + 128,
				unit->get_y() + 128
			).for_each([&](Unit* target) {
				if (target == unit) {
					return;
				}
				if (target->id != UnitId::TerranAzazel) {
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
		if (unit_ext->values[ValueSublimeShepherd] && !unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id))) {
			unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		} else if (unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id)) && !unit_ext->values[ValueSublimeShepherd]) {
			unit->sprite->remove_overlay(ImageId::SublimeShepherdOverlaySmall - scbw::get_overlay_offset(unit->id));
		}

		// Fix target clearing bug
		/*
		if (unit->order_target.unit
			&& !(unit->order_target.unit->detection_status & (1 << unit->player))) {
			scbw::print("lard");
			unit->order_to_idle();
		}
		*/

		// Play harvesting animations when a worker harvests from a resource
		// Requires more involved anim/graphics work
		/*
		if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
			Unit* harvest_target = unit->worker.harvest_target;
			// Eventually make it happen for vespene geysers and mineral fields, not just refineries
			if (harvest_target && scbw::unit_id_is_gas_structure(harvest_target->id, true) && harvest_target->sprite->main_graphic->animation != IscriptAnimation::IsWorking) {
				harvest_target->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			}
		}
		*/

		// Phase Link Overlay
		if (*g_elapsed_time_frames % 10 == 0
			&& unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].size()) {
			unit->sprite->create_overlay(ImageId::PhaseLinkSmall + scbw::get_overlay_offset(unit->id), 0, 0, 0);
		}

		// Incubation
		if (unit_ext->timers[TimerIncubationArm] == 1) {
			u32 unit_id = UnitId::ZergRilirokor;
			if (units_dat::base_property[unit->id] & UnitProperty::Flyer
			||  unit->status & UnitStatus::InAir) {
				unit_id = UnitId::ZergRililisk;
			}
			scbw::create_unit_from_ability(unit_id, 2, unit_ext->values[ValueIncubationPlayer], unit->position);
		}

		// Disruption Web Clear
		if (unit_ext->timers[TimerUnderDisruptionField] == 1) {
			unit->status &= ~UnitStatus::CanNotAttack;
			if (unit->subunit) {
				unit->subunit->status &= ~UnitStatus::CanNotAttack;
			}
		}

		// Madrigal - Dirge
		if (unit_ext->timers[TimerDirgeFuse] == 1) {
			scbw::madrigal_dirge_fuse(unit, unit_ext);
		}

		// Hierophant Signify Reset
		if (unit_ext->timers[TimerSignify] == 1) {
			unit_ext->values[ValueSignifyStacks] = 0;
			unit_ext->value_vectors[ValueVectorSignifySourcePlayer].clear();
			scbw::update_signify_status(unit, unit_ext);
		}

		// Decloak on stun
		if (unit->is_frozen() || unit_ext->timers[TimerStrigaRepulsion] || unit_ext->timers[TimerAnthelionGravityWell]) {
			scbw::decloak_proc(unit);
		}

		// Play addon work animations when related units are being trained by parent structure
		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon && unit->building.addon->status & UnitStatus::Completed)
		{
			// Wait for addon connector extension sequence to finish before animating
			// Have to find a working method... -Pr0nogo
		/*
			Image* connector = scbw::get_addon_connector_image(unit->building.addon);
			bool connector_is_extended = (connector && connector->animation != IscriptAnimation::Landing);
			if (!connector) // For addons without connectors, don't wait for anything
			{
				connector_is_extended = true;
			}
		*/
			bool connector_is_extended = true;
			u16 unit_id = unit->build_queue[unit->build_queue_slot];
			if (unit_id != UnitId::None && !scbw::unit_id_is_available_without_an_addon(unit_id) && connector_is_extended) {
				unit_ext->values[ValuePlayingAddonWorkingAnim] = 1;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			} else if (unit_ext->values[ValuePlayingAddonWorkingAnim] && connector_is_extended) {
				unit_ext->values[ValuePlayingAddonWorkingAnim] = 0;
				unit->building.addon->sprite->play_iscript_animation(IscriptAnimation::WorkingToIdle);
			}
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Addon && !(unit->status & UnitStatus::Completed)) {
			if (unit->connected_unit && !unit->connected_unit->is_frozen() && !unit_ext->timers[TimerAddonAnimation]) {
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

		// Correct vanilla timers. -Pr0nogo
		if (unit->defensive_matrix_timer > AzazelDefensiveMatrixTimer) {
			unit->defensive_matrix_timer = AzazelDefensiveMatrixTimer;
		}
		if (unit->stasis_timer > DidactStasisFieldTimer) {
			unit->stasis_timer = DidactStasisFieldTimer;
		}
		if (unit->maelstrom_timer > MindTyrantMaelstromTimer) {
			unit->maelstrom_timer = MindTyrantMaelstromTimer;
		}
		if (unit->plague_timer > IsthrathalethPlagueTimer) {
			unit->plague_timer = IsthrathalethPlagueTimer;
		}

		// Striga - Fleeting Focus
		if (unit_ext->timers[TimerStrigaRepulsion]) {
			Point16 final_pos = unit->position;
			final_pos.x += unit_ext->repulsion_vector.x;
			final_pos.y += unit_ext->repulsion_vector.y;
			final_pos = scbw::clamp_point(final_pos);

			if (!scbw::move_unit_improved(unit, final_pos.x, final_pos.y)) {
				unit_ext->timers[TimerStrigaRepulsion] = 0;
			}
		}

		// Magnetar - Law of Attraction
		// Todo - fix lag time between multiple pulls
		// Todo - fix delay before moved unit is able to do stuff
		if (unit_ext->timers[TimerMagnetarAttraction]) {
			Point16 final_pos = unit->position;
			final_pos.x += unit_ext->repulsion_vector.x;
			final_pos.y += unit_ext->repulsion_vector.y;
			final_pos = scbw::clamp_point(final_pos);

			if (!scbw::move_unit_improved(unit, final_pos.x, final_pos.y)) {
				unit_ext->timers[TimerMagnetarAttraction] = 0;
			}
		}

		// Anthelion - Gravity Well
		if (unit_ext->timers[TimerAnthelionGravityWell] && unit_ext->units[UnitAnthelionGravityWellSource]) {
			Point16 target_pos = unit_ext->units[UnitAnthelionGravityWellSource]->position;
			if (unit->position != target_pos) {
				Vector32 diff = scbw::get_vector_between_points(unit->position, target_pos);
				if (scbw::get_vector_distance(diff) < 2) {
					scbw::move_unit_improved(unit, target_pos.x, target_pos.y);
				} else {
					Vector32f displacement = scbw::get_normalized_vector(diff);
					displacement.x *= 2;
					displacement.y *= 2;

					Point16 final_pos = unit->position;
					final_pos.x += displacement.x;
					final_pos.y += displacement.y;
					final_pos = scbw::clamp_point(final_pos);

					scbw::move_unit_improved(unit, final_pos.x, final_pos.y);
					helper::turn_unit(unit, scbw::get_angle(unit->position, target_pos));
				}
			}
		}

		// Cyclops - Fragmentary Shells fuse
		for (u32 fuse_timer : unit_ext->timer_vectors[TimerVectorCyclopsFuse]) {
			if (fuse_timer == 1) {
				helper::create_thingy(SpriteId::FragmentaryExplosion, unit->get_x(), unit->get_y(), 11);
				helper::play_sound_from_unit(scbw::rand_between(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04), unit, 1, 0);

				EntryArray units = {};
				unit_search_radial(&units, unit->position, 2 * 32);
				for (u32 i = 0; i < units.count; ++i) {
					Unit* target = units.units[i];
					target->damage_with_custom(5 * 256, 2);
					target->sprite->create_top_overlay(ImageId::CyclopsFragmentaryExplosion);
				}
			}
		}

		// Gorgon - Petrification
		if (unit_ext->timers[TimerPetrification] == 1) {
			unit_ext->values[ValuePetrificationStacks] = 0;
		}

		// Blissful Embrace - Creep spread and Larva spawn
		if (unit_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
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
		if (((!unit_ext->timers[TimerReviveFlagClearMin] && !(unit->status & UnitStatus::NoBrkCodeStart)) || !unit_ext->timers[TimerReviveFlagClearMax])
			&& unit_ext->values[ValueReviveStarted]) {
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
			if (!(scbw::is_allied_to(foundUnit->player, unit->player))) {
				return;
			}
			if (foundUnit->id != UnitId::ZergAlaszileth) {
				return;
			}
			if (scbw::unit_id_is_resource_node(unit->id, false)) {
				return;
			}
			unit_ext->timers[TimerAlaszilethRegenBoost] = 2;

			// Create overlay
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::get_overlay_offset(unit->id, true);
			if (!unit->get_overlay(overlay)) {
				unit->sprite->create_top_overlay(overlay);
			}
		});

		// Seraph - Observance
		if (unit_ext->timers[TimerObservanceDebuff] > 1) {
			const auto range = 6 * 32;
			auto seraphs = scbw::UnitFinder(
				unit->get_x() - range,
				unit->get_y() - range,
				unit->get_x() + range,
				unit->get_y() + range
			).filter([&](Unit* target) {
				if (unit->player == target->player
					|| scbw::is_allied_to(unit->player, target->player)
					|| target->id != UnitId::TerranSeraph) {
					return false;
				}
				return true;
			}).get_all_units();

			if (!seraphs.empty()) {
				// Seraph - Observance period
				unit_ext->timers[TimerObservanceDebuff] = 5 * 24 + 1;

				// Seraph - Observance ability
				if (unit_ext->timers[TimerObservancePeriod] == 1) {
					unit_ext->timers[TimerObservancePeriod] = 5 * 24 + 1;
					unit_ext->values[ValueObservanceRend] += 1;
				}

				// Seraph - Observance persistant overlay creation
				for (auto seraph : seraphs) {
					if (!seraph) {
						continue;
					}
					UnitExt* seraph_ext = get_unit_ext(seraph);
					if (!seraph_ext->timers[TimerObservanceOverlay]) {
						seraph->sprite->create_overlay(ImageId::SeraphRedOverlay);
						seraph_ext->timers[TimerObservanceOverlay] = 18;
					}
				}
			}
		} else if (unit_ext->timers[TimerObservanceDebuff] == 1) {
			unit->sprite->remove_overlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
		}

		// Vithrilisk - Corrosive Touch clear
		if (unit_ext->timers[TimerCorrosiveTouch] == 1) {
			unit_ext->values[ValueCorrosiveTouchStacks] = 0;
			unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Zethrokor - Adrenal Glands
		if (unit_ext->timers[TimerAdrenal] == 1) {
			unit_ext->values[ValueAdrenalStacks] = 0;
		}

		// Vorvrokor - Captivating Claws
		if (unit_ext->timers[TimerCaptivatingClaws] == 1) {
			unit_ext->unit_vectors[UnitVectorCaptivatingClaws] = {};
		}

		// Low-yield Mineral Fields - No collision
		if (unit->id >= UnitId::ResourceMineralField
		&&  unit->id <= UnitId::ResourceMineralFieldType3
		&&  unit->building.resource.resource_yield <= 2) {
			unit->status |= UnitStatus::NoCollide;
			unit->sprite->elevation = 0;
		}

		// Spider Mine - No collision
		if (unit->id == UnitId::TerranSpiderMine) {
			unit->status |= UnitStatus::NoCollide;
		}

		// Game rule - Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
		if (unit->main_order_id == OrderId::Harvest1
		||  unit->main_order_id == OrderId::HarvestGas1
		||  unit->main_order_id == OrderId::ReturnMinerals
		||  unit->main_order_id == OrderId::ReturnGas) {
			if (unit->status & UnitStatus::IsGathering) {
				unit->status |= UnitStatus::NoCollide;
			} else {
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

		// Talos - Salvage stun
		if (unit_ext->timers[TimerSalvageStunTimer] > 1) {
			unit->freeze();

			u32 offset = scbw::get_overlay_offset(unit->id);
			if (!unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->create_top_overlay(ImageId::SalvageDisableSmall + offset);
			}
			if (!unit_ext->timers[TimerTalosSoundLoopTimer]) {
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
		if (unit_ext->timers[TimerDecloak] == 1) {
			if (scbw::is_decloakable(unit)) {
				helper::begin_cloak(unit, SoundId::Terran_PHOENIX_TPhClo00_WAV);
			}
		}

		// Autovitality - Simulacrum behavior
		if (unit_ext->timers[TimerSimulacrumClone] == 1) {
			scbw::create_unit_from_ability(UnitId::ProtossSimulacrum, 1, unit->player, unit->position, unit);
		}

		// Remove Ensnaring Brood overlay
		if (unit_ext->timers[TimerEnsnaringBrood] == 1) {
			unit->remove_overlay(ImageId::EnsnaringBroodOverlay);
		}

		// Crucible stacks clear
		{
			auto& crucibles = unit_ext->unit_vectors[UnitVectorCrucibles];
			for (auto it = crucibles.begin(); it != crucibles.end(); ++it) {
				Unit* target = *it;
				if (scbw::get_distance_fast(unit->position, target->position) > 8 * 32) {
					target->remove_overlay(ImageId::CrucibleKhaydarinChargeOverlay);
					crucibles.erase(it);
					--it;
				}
			}
		}

		// Crucible - Khaydarin Charge stacks
		if (unit->id == UnitId::ProtossCrucible && unit->status & UnitStatus::Completed) {
			EntryArray units = {};
			unit_search_radial(&units, unit->position, 8 * 32);
			unit_search_filter(&units, unit, FilterAlly | FilterBuilding);
			for (u32 i = 0; i < units.count; i++) {
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
					case UnitId::TerranReservoir: { // @Veeq7: What are these edge cases? -Pr0nogo
						if (!target_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
							continue;
						}
					} break;
				}
				if (!(target->status & UnitStatus::Completed) && target->id != UnitId::ZergNydusCesiant) {
					continue;
				}

				auto& vec = target_ext->unit_vectors[UnitVectorCrucibles];
				if (std::find(vec.begin(), vec.end(), unit) == vec.end()) {
					vec.push_back(unit);
				}
				if (!target->get_overlay(ImageId::CrucibleKhaydarinChargeOverlay)) {
					target->sprite->create_underlay(ImageId::CrucibleKhaydarinChargeOverlay, 0, 0, 0);
				}
			}
		}

		// Controlled Demolition - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastControlledDemolition)
			&& !unit_ext->timers[TimerEmergencyDetonation] && !unit_ext->timers[TimerControlledDemolition]) {
			scbw::set_generic_rust_value(unit, RustId::CastControlledDemolition, 0);
			unit_ext->timers[TimerControlledDemolition] = ControlledDemolitionTimer;
			unit->sprite->create_overlay(ImageId::ControlledDemolition);
		}

		// Start overlay fade
		if (unit_ext->timers[TimerControlledDemolition] == 7) {
			Image* overlay = unit->get_overlay(ImageId::ControlledDemolition);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Controlled Demolition - destroy structure
		if (unit_ext->timers[TimerControlledDemolition] == 1) {
			unit->remove();
		}

		// Emergency Detonation - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation)) {
			scbw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
			unit_ext->timers[TimerEmergencyDetonation] = 72 + 1;
			unit_ext->timers[TimerControlledDemolition] = 0;
			unit->sprite->remove_overlay(ImageId::ControlledDemolition);
			unit->sprite->create_top_overlay(ImageId::EmergencyDetonation);
		}

		// Start overlay fade
		if (unit_ext->timers[TimerEmergencyDetonation] == 7) {
			Image* overlay = unit->get_overlay(ImageId::EmergencyDetonation);
			overlay->play_iscript_animation(IscriptAnimation::Death);
		}

		// Emergency Detonation - destroy structure
		if (unit_ext->timers[TimerEmergencyDetonation] == 1) {
			auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
			const u32 radius = 3 * 32;
			scbw::UnitFinder(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius
			).for_each_in_match(match, [&](Unit* target) {
				if (scbw::get_distance_fast(unit->position, target->position) <= radius) {
					target->damage_with_custom(250 * 256, 0, unit, unit->player);
				}
			});
			helper::create_thingy(SpriteId::ArcliteShockCannon_Damageless, unit->sprite->position.x, unit->sprite->position.y, unit->player);
			unit->remove();
		}

		// Demiurge - Gods Made Flesh - return units to original owner
	//	if (unit_ext->values[ValueWorshipStack] && !unit_ext->units[UnitMindController]) {
	//		helper::play_sound_from_unit(SoundId::DemiurgeWorship01, unit, 1, 0);
	//		unit->sprite->create_top_overlay(ImageId::Worship_Small + scbw::get_overlay_offset(unit->id), 0, 0, 0);
	//		unit_ext->values[ValueWorshipStack] = 0;
	//		unit_ext->timers[TimerGodsMadeFlesh] = 0;
	//		scbw::recede_mind_control(unit);
	//	}

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
	//	if (unit_ext->timers[TimerGodsMadeFlesh] == 1 && unit_ext->units[UnitMindController]) {
	//		unit->remove();
	//	}

		// Silly way of pathing workers to expansion site
		if (unit_ext->values[ValueAI_WorkerChosenForExpansion] && unit->resource_type == ResourceType::None) {
			unit_ext->values[ValueAI_WorkerChosenForExpansion] = false;
			unit->order_to(OrderId::Move, unit_ext->values[ValueAI_WorkerChosenX], unit_ext->values[ValueAI_WorkerChosenY]);
		}

		// Legionnaire - Blade Vortex - Handle debuff overlay
		{
			u32 blade_vortex = unit_ext->timers[TimerBladeVortex];
			u32 image_id = ImageId::BladeVortexDebuffSmall + scbw::get_overlay_offset(unit->id);
			if (blade_vortex > 1 && !unit->get_overlay(image_id)) {
				unit->sprite->create_top_overlay(image_id);
			} else if (blade_vortex <= 1 && unit->get_overlay(image_id)) {
				unit->sprite->remove_overlay(image_id);
			}
		}

		// Magister - Stellar Enforcement - Handle overlay looping
		if (unit_ext->timers[TimerStellarEnforcement] > 1 && unit_ext->timers[TimerStellarEnforcementOverlay] <= 1) {
			unit_ext->timers[TimerStellarEnforcementOverlay] = 30;
			unit->sprite->create_top_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true));
		}
		// Handle overlays after effect is cleansed
		if (unit->get_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true)) && unit_ext->timers[TimerStellarEnforcement] >= 1) {
			unit->sprite->remove_overlay(ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(unit->id, true));
		}

		// Clear Deconstruction on timer end
		if (unit_ext->values[ValueDeconstruction] && unit_ext->timers[TimerDeconstruction] <= 1) {
			unit_ext->values[ValueDeconstruction] = 0;
			unit->sprite->remove_overlay(ImageId::DeconstructionSmall + scbw::get_overlay_offset(unit->id));
		}

		// Harakan - Apollyoid Adhesive
		{
			u32 image_id = ImageId::HarakanApollyoidAdhesiveOverlay;
			if (unit_ext->timers[TimerApollyoidAdhesive] >= 1) {
				if (!unit->sprite->get_overlay(image_id)) {
					unit->sprite->create_top_overlay(image_id);
				}
				if (!unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod]) {
					unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod] = 24;
				}
				if (unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod] == 1) {
					unit->damage_with_custom(unit_ext->values[ValueApollyoidAdhesiveStacks] * 256, 2);
				}
			}
			if (unit_ext->timers[TimerApollyoidAdhesive] == 1) {
				unit->remove_overlay(image_id);
				unit_ext->values[ValueApollyoidAdhesiveStacks] = 0;
			}
		}

		// Clear speed buffs for currently-moving flingy units. -Pr0nogo
		if (unit_ext->timers[TimerTerminalSurge] == 1) {
			unit->update_speed();
		}

		// Meme to make lifted buildings use their flingy speed. -Pr0nogo
		if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding
		&&	unit->status & UnitStatus::InAir && unit_ext->timers[TimerLiftLandDelay] == 1) {
			unit->update_speed();
		}

		if (global_vars.celebration == Celebration_Christmas) {
			if (unit->status & UnitStatus::Burrowed) {
				unit->sprite->remove_overlay(ImageId::SantaHatSmall, ImageId::SantaHatXLarge17);
			} else if (unit->id == UnitId::ZergEgg || unit->status & UnitStatus::Completed) {
				u32 image_id = ImageId::None;
				u32 offset_x = 0;
				u32 offset_y = 0;
				switch (unit->id) {
					// Use special offsets
					case UnitId::TerranCohort:
					case UnitId::TerranMason:
					case UnitId::TerranMaverick:
					case UnitId::TerranHarakan:
					case UnitId::ProtossArtisan:
					case UnitId::ProtossSimulacrum:
					case UnitId::ZergEgg:
					{
						image_id = ImageId::SantaHatSmall17;
						if (unit->sprite->main_graphic->flags & ImageFlags::Omnidirectional) {
							image_id -= 4;
						}
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 6;
					} break;
					case UnitId::ZergQuazilisk:
					case UnitId::ZergHydralisk:
					case UnitId::ZergKagralisk:
					{
						image_id = ImageId::SantaHatSmall17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 9;
					} break;
					case UnitId::ProtossPylon:
					{
						image_id = ImageId::SantaHatMedium17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 12;
					} break;
					case UnitId::ProtossCenotaph:
					{
						image_id = ImageId::SantaHatXLarge17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 17;
					} break;
					case UnitId::ProtossGladius:
					{
						image_id = ImageId::SantaHatLarge;
						offset_y = units_dat::unit_bounds[unit->id].top / 2;
					} break;
					case UnitId::ProtossAnthelion:
					{
						image_id = ImageId::SantaHatXLarge;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 46;
					} break;
					case UnitId::ProtossAccantor:
					case UnitId::ZergOvileth:
					{
						image_id = ImageId::SantaHatLarge17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 8;
					} break;
					case UnitId::ZergLarvoskColony:
					{
						image_id = ImageId::SantaHatLarge17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 6;
					} break;
					case UnitId::ZergMatravilNest:
					{
						image_id = ImageId::SantaHatXLarge17;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 14;
					} break;
					case UnitId::ZergGosvileth:
					{
						image_id = ImageId::SantaHatSmall;
						offset_y = units_dat::unit_bounds[unit->id].top / 2 + 4;
					} break;
					// Use unit bounds
					case UnitId::TerranTalos:
					case UnitId::TerranWraith:
					case UnitId::ProtossScribe:
					case UnitId::ProtossSteward:
					case UnitId::ZergLarva:
					case UnitId::ZergDroleth:
					case UnitId::ZergZethrokor:
					case UnitId::ZergVorvrokor:
					case UnitId::ZergLiiralisk:
					case UnitId::ZergGeszithalor:
					case UnitId::ZergRilirokor:
					case UnitId::ZergRililisk:
					case UnitId::ZergIsthrathaleth:
					{
						image_id = ImageId::SantaHatSmall17 + scbw::get_overlay_offset(unit->id, true);
						offset_y = units_dat::unit_bounds[unit->id].top / 2;
						if (unit->sprite->main_graphic->flags & ImageFlags::Omnidirectional) {
							image_id -= 4;
						}
					} break;
				}

				if (image_id != ImageId::None) {
					if (!unit->get_overlay(image_id)) {
						unit->sprite->create_top_overlay(image_id, offset_x, -offset_y, 0, unit);
					}
				}
			}
		}
	}

	bool game_on() {
		set_max_sight_range<20>();

		chk_cache_size = 0;
		memset(chk_cache, 0, sizeof(chk_cache));

		quickcasting_signal = false;
		quickcasting_command_card = 0;

		init_threshold();
		return true;
	}

	bool game_end() {
		end_threshold();
		graphics::reset_stat_res_graphics();
		return true;
	}

	// Hooked to explore custom game commands. -Pr0nogo
	void game_recv(char* buffer, s32 size, bool in_replay) { // 004865D0
		// Size needs to be s32 to support less than 0 comparison...
		if (size) {
			b32 is_in_replay = *g_is_in_replay;
			u32 command_id = 0;
			u32 command_size = 0;
			while (true) {
				command_id = *buffer;
				if (!*g_is_in_replay || in_replay) {
					break;
				}
				switch (command_id) {
					case GameCommand_Sequence:
					case GameCommand_RestartGame:
					case GameCommand_PauseGame:
					case GameCommand_ResumeGame:
					case GameCommand_Hash:
					case GameCommand_VoiceEnable1:
					case GameCommand_VoiceEnable2:
					case GameCommand_Unused1:
					case GameCommand_Unused2:
					case GameCommand_SetLatency:
					case GameCommand_SetReplaySpeed: {
						goto process_command;
					}
					default: {
						u32 command_size = g_turn_command_size[command_id];
						if (command_id >= GameCommand_Select && command_id <= GameCommand_ShiftDeselect) {
							command_size = 2 * (u8)buffer[1] + 2;
						}
						size -= command_size;
					maybe_process_command:
						if (size <= 0) {
							return;
						}
					} break;
				}
			}
		process_command:
			switch (command_id) {
				case GameCommand_Sequence:
				{
					command_size = 1;
					if (size >= command_size) {
						++*g_sequence;
						goto next_command;
					}
				} break;
				case GameCommand_SaveGame:
				{
					command_size = 5;
					if (size >= command_size) {
						s32 string_size = helper::get_recv_string(28u, size - command_size, 0, buffer + 5, 0);
						if (string_size) {
							command_size += string_size;
							helper::CMDRECV_save_game(buffer);
							goto next_command;
						}
					}
				} break;
				case GameCommand_LoadGame:
				{
					command_size = 5;
					if (size >= command_size) {
						s32 string_size = helper::get_recv_string(28u, size - command_size, 0, buffer + 5, 0);
						if (string_size) {
							command_size += string_size;
							if (!*g_multiplayer_mode) {
								helper::CMDRECV_load_game(buffer + 5);
							}
							goto next_command;
						}
					}
				} break;
				case GameCommand_RestartGame:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_restart_game();
						goto next_command;
					}
				} break;
				case GameCommand_Select:
				{
					command_size = 2;
					if (size >= command_size) {
						command_size += 2 * (u8)buffer[1];
						if (size >= command_size) {
							helper::CMDRECV_select(buffer);
							goto next_command;
						}
					}
				} break;
				case GameCommand_ShiftSelect:
				{
					command_size = 2;
					if (size >= command_size) {
						command_size += 2 * (u8)buffer[1];
						if (size >= command_size) {
							helper::CMDRECV_shift_select(buffer);
							goto next_command;
						}
					}
				} break;
				case GameCommand_ShiftDeselect:
				{
					command_size = 2;
					if (size >= command_size) {
						command_size += 2 * (u8)buffer[1];
						if (size >= command_size) {
							helper::CMDRECV_shift_deselect(buffer);
							goto next_command;
						}
					}
				} break;
				case GameCommand_Build:
				{
					command_size = 8;
					if (size >= command_size) {
						helper::CMDRECV_build(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Vision:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_vision(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Ally:
				{
					command_size = 5;
					if (size >= command_size) {
						helper::CMDRECV_ally(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_GameSpeed:
				{
					command_size = 2;
					if (size >= command_size) {
						u32 speed = buffer[1];
						if (speed < 7 && !*g_multiplayer_mode) {
							(*g_options_list).speed = speed;
						}
						goto next_command;
					}
				} break;
				case GameCommand_PauseGame:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_pause_game();
						goto next_command;
					}
				} break;
				case GameCommand_ResumeGame:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_resume_game();
						goto next_command;
					}
				} break;
				case GameCommand_UseCheat:
				{
					command_size = 5;
					if (size >= command_size) {
						helper::CMDRECV_use_cheat(buffer, (char*)g_is_in_replay);
						goto next_command;
					}
				} break;
				case GameCommand_Hotkey:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_hotkey(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_RightClick:
				{
					command_size = 10;
					if (size >= command_size) {
						helper::CMDRECV_right_click((Unit*)buffer);
						goto next_command;
					}
				} break;
				case GameCommand_TargetCommand:
				{
					command_size = 11;
					if (size >= command_size) {
						helper::CMDRECV_target_command(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_CancelConstruction:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_construction();
						goto next_command;
					}
				} break;
				case GameCommand_CancelUnitMorph:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_unit_morph();
						goto next_command;
					}
				} break;
				case GameCommand_Stop:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_stop(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_CarrierStop:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_carrier_stop();
						goto next_command;
					}
				} break;
				case GameCommand_ReaverStop:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_reaver_stop();
						goto next_command;
					}
				} break;
				case GameCommand_Nothing:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_nothing();
						goto next_command;
					}
				} break;
				case GameCommand_ReturnCargo:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_return_cargo(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Train:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_train(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_CancelTrain:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_cancel_train(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Cloak:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_cloak();
						goto next_command;
					}
				} break;
				case GameCommand_Decloak:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_decloak();
						goto next_command;
					}
				} break;
				case GameCommand_UnitMorph:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_unit_morph(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Unsiege:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_unsiege(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Siege:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_siege(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_TrainFighter:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_train_fighter();
						goto next_command;
					}
				} break;
				case GameCommand_UnloadAll:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_unload_all(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Unload:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_unload((Unit*)buffer);
						goto next_command;
					}
				} break;
				case GameCommand_MergeArchon:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_merge_archon();
						goto next_command;
					}
				} break;
				case GameCommand_HoldPosition:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_hold_position(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Burrow:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_burrow(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Unburrow:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_unburrow();
						goto next_command;
					}
				} break;
				case GameCommand_CancelNuke:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_nuke();
						goto next_command;
					}
				} break;
				case GameCommand_Liftoff:
				{
					command_size = 5;
					if (size >= command_size) {
						helper::CMDRECV_liftoff(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Research:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_research(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_CancelResearch:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_research();
						goto next_command;
					}
				} break;
				case GameCommand_Upgrade:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_upgrade(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_CancelUpgrade:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_upgrade();
						goto next_command;
					}
				} break;
				case GameCommand_CancelAddon:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_cancel_addon();
						goto next_command;
					}
				} break;
				case GameCommand_BuildingMorph:
				{
					command_size = 3;
					if (size >= command_size) {
						helper::CMDRECV_building_morph(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Stimpack:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_stim_pack();
						goto next_command;
					}
				} break;
				case GameCommand_Hash:
				{
					command_size = 7;
					if (size >= command_size) {
						if (!is_in_replay && !helper::CMD_game_hash((HashPacket*)buffer)) {
							*g_is_bad_hash = 1;
							g_array_player_status[*g_active_storm_id] = 0x10000;
						}
						++(*g_sequence);
						goto next_command;
					}
				} break;
				case GameCommand_VoiceEnable1:
				case GameCommand_VoiceEnable2:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_voice_enable(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_Unused1:
				case GameCommand_Unused2:
				{
					command_size = 2;
					if (size >= command_size) {
						goto next_command;
					}
				} break;
				case GameCommand_SetLatency:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_set_latency(buffer, 0);
						goto next_command;
					}
				} break;
				case GameCommand_SetReplaySpeed:
				{
					command_size = 10;
					if (size >= command_size) {
						if (is_in_replay) {
							helper::CMDRECV_set_replay_speed(*(s32*)(buffer + 2), buffer[1], *(s32*)(buffer + 6));
						}
						goto next_command;
					}
				} break;
				case GameCommand_LeaveGame:
				{
					command_size = 2;
					if (size >= command_size) {
						helper::CMDRECV_leave_game(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_MinimapPing:
				{
					command_size = 5;
					if (size >= command_size) {
						helper::CMDRECV_minimap_ping(buffer);
						goto next_command;
					}
				} break;
				case GameCommand_MergeDarkArchon:
				{
					command_size = 1;
					if (size >= command_size) {
						helper::CMDRECV_merge_dark_archon();
						goto next_command;
					}
				} break;
				case GameCommand_DisplaySomeText:
				{
					command_size = 82;
					if (size >= command_size) {
						helper::bwfxn_display_text(buffer[1], (s32)(buffer + 2), in_replay);
					next_command:
						if (*g_active_nation_id < PlayerLimit) {
							hooks::register_action(&player_apm_counter[*g_active_nation_id], command_id);
						}
						helper::save_replay_action(*g_replay_data, buffer, command_size, *g_active_storm_id);
						size -= command_size;
						buffer += command_size;
						is_in_replay = *g_is_in_replay;
						goto maybe_process_command;
					}
				} break;
				default:
					return;
			}
		}
	}
}
