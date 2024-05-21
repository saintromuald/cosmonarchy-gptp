#include "game_hooks.h"
#include <graphics/graphics.h>
#include <BW/api.h>
#include <BW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "hooks/interface/uifeatures/hotkeys.h"
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
#include "BW\UnitFinder.h"
#include "hooks/interface/buttonsets.h"
#include "weapons/weapon_fire.h"
#include "harvest.h"
#include "hooks/weapons/weapon_cooldown.h"
#include <time.h>
#include "trigger_system/trigger_system.h"
#include "hooks/weapons/weapon_damage.h"
#include "selection_extender.h"
#include "hooks/interface/weapon_armor_tooltip.h"
#include "hooks/weapons/weapon_damage.h"
#include "hooks/update_unit_sight.h"
#include "hooks/orders/larva_creep_spawn.h"
#include "hooks/interface/uifeatures/hotkeys.h"
#include "Discord.h"
#include "hooks/rally_point.h"
#include "game_modes/IGameMode.h"
#include "game_modes/UprisingGameMode.h"

#include <random>
#include <format>

void shaman_nanite_field_proc(Unit* shaman, u32 heal, Unit* target) {
	if (target->get_current_hp_ingame() < target->get_max_hp_ingame()) {
		bw::add_unit_overlay(target, ImageId::ShamanNaniteField);
	}
	target->heal_hp(heal * 256, shaman);
}

namespace hooks {
	static bool g_game_was_init;
	static bool g_game_inited;

	void next_ui_frame() {
		if (!g_game_inited) {
			g_game_inited = true;
			/*
			Parser parser = new_parser(...);
			while (parser_get_word(&parser)) {
				if (parser_eq(&parser, "-map")) {
					Token token = parser_get_string(&parser);
					helper::load_scenario_singleplayer(token.data, 0, "Some Error Message", "Player Name", 6);
				}
			}
			*/
		}
		g_discord.update_status_if_needed(false);
		update_trigger_system(false);
	}

	void ui_button_click(BinDlg* dialog) {
		if (dialog->parent && dialog->parent->text && strcmp(dialog->parent->text, "StatBtn") == 0) {
			hooks::quickcasting_timeout  = 5;
			hooks::quickbuilding_timeout = 5;
		}
	}

	static bool process_quickcasting() {
		if (g_config.quick_casting && g_is_targeting && !quickcasting_timeout) {
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
			if (g_is_targeting) {
				helper::input_target_order_right_mouse_click();
			}
			return true;
		}
		return false;
	}

	static bool process_quickbuilding() {
		if (g_config.quick_building && g_is_placing_building && !quickbuilding_timeout) {
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
				if (g_is_placing_building) {
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

	// Hooked to remove the 1-frame delay from quick-targeting. -Pr0nogo
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
		} else if (g_is_placing_building) {
			*left_click_proc  = (InputProcsFunc*)0x0048E5D0; // input_place_building_left_mouse_click;
			*right_click_proc = (InputProcsFunc*)0x00484600; // input_place_building_right_mouse_click;
		} else if (g_is_targeting) {
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
		if (g_is_in_replay && *g_replay_reveal_all) {
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
			g_graphics.draw_line(source.x, source.y, target.x, target.y, color, graphics::ON_MAP);
			return true;
		}
		return false;
	}

	static Point16 draw_order_line(Point16 source, Target target, graphics::ColorId color, Unit* ignore_draw_unit = 0) {
		Point16 target_pos = get_target_point(target);
		if (target_pos.x != 0 || target_pos.y != 0) {
			if (!ignore_draw_unit || target.unit != ignore_draw_unit) {
				g_graphics.draw_line(source.x, source.y, target_pos.x, target_pos.y, color, graphics::ON_MAP);
			}
			return target_pos;
		}
		return source;
	}

	static void draw_transport_ui() {
		Point16 start_pos = {179, 346};
		Point16 pos = start_pos;

		if (transport_ui_unit) {
			transport_ui_unit = 0;
			helper::set_ingame_input_procs();
		}

		for (u32 i = 0; i < *g_client_selection_count; ++i) {
			Unit* transport = g_client_selection_group->unit[i];
			bool is_replay_or_witness = g_is_in_replay || g_globals->players[*g_local_human_id].witness_mode;
			if (!bw::unit_is_allied_or_detected(transport, *g_local_nation_id) && !is_replay_or_witness) {
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

			pos.y -= 13 + ((cargo_slots_count + 3) / 4) * 13;
			Point16 local_pos = pos;
			pos.y -= 4;

			{
				Token short_unit_name = get_short_unit_name(transport->id);
				std::string unit_name = std::format("{}{} [{}/{}]:", bw::get_player_color_code(transport->player, true).data, short_unit_name.string().c_str(), space_used, units_dat::space_provided[transport->id]);
				g_graphics.draw_text(local_pos.x, local_pos.y, unit_name, graphics::FONT_SMALL, graphics::CoordType::ON_SCREEN);
				local_pos.y += 13;
			}

			u32 x_count = 0;
			for (u32 j = 0; j < cargo_slots_count; ++j) {
				CargoSlot cargo_slot = cargo_slots[j];

				std::string short_unit_name = get_short_unit_name(cargo_slot.unit->id).string();
				if (cargo_slot.unit->id == UnitId::TerranPhalanxSiegeMode) {
					short_unit_name += " (Sieged)";
				}
				std::string unit_name = "";
				if (cargo_slot.count > 1) {
					unit_name = std::format("{}{} ({})", bw::get_player_color_code(cargo_slot.unit->player, true).data, short_unit_name, cargo_slot.count);
				} else {
					unit_name = std::format("{}{}", bw::get_player_color_code(cargo_slot.unit->player, true).data, short_unit_name);
				}
				u32 width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(unit_name.c_str());

				RECT rect = {};
				rect.left = local_pos.x;
				rect.top = local_pos.y;
				rect.right = rect.left + width;
				rect.bottom = rect.top + 11;
				bool hover = (*g_local_nation_id == transport->player || *g_local_nation_id == cargo_slot.unit->player) && !g_is_in_replay && PtInRect(&rect, *(POINT*) g_mouse);
				if (hover) {
					if (cargo_slot.count > 1) {
						unit_name = std::format(BWTC_WHITE "{} ({})", short_unit_name, cargo_slot.count);
					} else {
						unit_name = std::format(BWTC_WHITE "{}", short_unit_name);
					}
					width = font_base[(u32)graphics::FONT_SMALL]->getTextWidth(unit_name.c_str());
				}

				g_graphics.draw_text(local_pos.x, local_pos.y, unit_name, graphics::FONT_SMALL, graphics::CoordType::ON_SCREEN);
				if (hover && !transport_ui_unit) {
					transport_ui_unit = cargo_slot.unit;
					helper::set_ingame_input_procs();
				}

				local_pos.x += width + 8;
				x_count++;
				if (x_count >= 4) {
					x_count = 0;
					local_pos.x = start_pos.x;
					local_pos.y += 13;
				}
			}
		}
	}

	static void draw_custom_ui() {
		// Draw selection info
		for (u32 i = 0; i < *g_client_selection_count; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			UnitExt* unit_ext = get_unit_ext(unit);
			Point16 source = unit->position;

			// For checking cargo-related elements.
			// This helped me solve an Anchor issue on 3 February 2023, please leave it here for the future. -Pr0nogo
			/*
			if (unit) {
				for (u32 i = 0; i < TransportCap; ++i) {
					Unit* cargo_unit = selected->get_loaded_unit(i);
					if (!cargo_unit) {
						continue;
					}
					u32 unit_order = cargo_unit->main_order_id;
					bw::printf("Order id %u", unit_order);
				}
			}
			*/

			// Show auras
			if (g_config.show_aura_circles && bw::can_show_custom_ui()
			&& (bw::are_mutual_allies(unit->player, *g_local_nation_id) || bw::in_replay_or_witness)) {
				switch (unit->id) {
					case UnitId::TerranShamanSupportMode:
					{
						g_graphics.draw_circle(source.x, source.y, 3 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::TerranShamanVisionaryMode:
					{
						g_graphics.draw_circle(source.x, source.y, 6 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::TerranAncile:
					{
						g_graphics.draw_circle(source.x, source.y, 2 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::TerranAzazel:
					{
						g_graphics.draw_circle(source.x, source.y, 5 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::ProtossClarion:
					{
						g_graphics.draw_circle(source.x, source.y, 5 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::ProtossCrucible:
					{
						g_graphics.draw_circle(source.x, source.y, 8 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::ProtossDidact:
					{
						g_graphics.draw_circle(source.x, source.y, 4 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::ZergMatraleth:
					{
						g_graphics.draw_circle(source.x, source.y, 4 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
					case UnitId::ZergVilgoleth:
					{
						g_graphics.draw_circle(source.x, source.y, 4 * 32, graphics::GREEN, graphics::ON_MAP);
					} break;
				}
			}

			// Show connected Nyduses
			if (unit->id == UnitId::ZergNydusCesiant && unit->building.nydus_exit) {
				if (bw::unit_is_allied_or_detected(unit, *g_local_nation_id) || bw::in_replay_or_witness()) {
					draw_simple_line(source, unit->building.nydus_exit->position, graphics::PURPLE);
				}
			}

			// Rally points
			if (g_config.show_rally_lines && bw::can_show_custom_ui()
			&& (bw::is_allied_to(*g_local_nation_id, unit->player) || bw::in_replay_or_witness()))
			{
				Point16 target = get_target_point(unit->rally);
				if (target.x == unit->position.x && target.y == unit->position.y) {
					target.x = 0;
					target.y = 0;
				}
				if (!unit_id_is_psi_provider(unit->id) && draw_simple_line(source, target, graphics::WHITE)) {
					source = target;
				} else if (unit->id == UnitId::ZergEgg && unit->connected_unit) {
					source = draw_order_line(source, unit->connected_unit->rally, graphics::GREY);
				}
			}
			
			// Ability ranges - DF
			if (g_config.show_ability_ranges && g_is_targeting && bw::can_show_custom_ui()) {
				u32 weapon = WeaponId::None;
				switch (*g_order_id_ground)
				{
					case OrderId::FireYamatoGun1:
					{
						weapon = WeaponId::YamatoGun;
					} break;
					case OrderId::CarapaceSwarm:
					{
						weapon = WeaponId::CarapaceSwarm;
					} break;
					case OrderId::CastParasite:
					{
						weapon = WeaponId::MatralethParasite;
					} break;
					case OrderId::PsiStorm:
					{
						weapon = WeaponId::PsiStorm;
					} break;
					case OrderId::Irradiate:
					{
						weapon = WeaponId::Irradiate;
					} break;
					case OrderId::Plague:
					{
						weapon = WeaponId::Plague;
					} break;
					case OrderId::StasisField:
					{
						weapon = WeaponId::StasisField; 
					} break;
					case OrderId::CastMaelstrom:
					{
						weapon = WeaponId::MindTyrantMaelstrom; 
					} break;
					case OrderId::CastOpticalFlare:
					{
						weapon = WeaponId::LuminaryAuralAssault; 
					} break;
					case OrderId::Hallucination1:
					case OrderId::NukePaint:
					case OrderId::CastTyranny:
					{
						bw::draw_aura_range(unit, units_dat::sight_range[unit->id], 164); //#88409C
					} break;
					case OrderId::DefensiveMatrix:
					{
						bw::draw_aura_range(unit, 8, 164); //#88409C
					} break;
					case OrderId::CastThroughTheLookingGlass:
					{
						bw::draw_aura_range(unit, ThroughTheLookingGlassRange / 32, 164);
					} break;
					default:
					{
					} break;
				}
				if (weapon != WeaponId::None) {
					bw::draw_weapon_range(*g_active_portrait_unit, weapon, 164); //#88409C
				}
			}

			if (g_config.show_order_lines && (bw::in_replay_or_witness() || bw::is_allied_to(*g_local_nation_id, unit->player)) && bw::can_show_custom_ui()) {
				Unit* ignore_resource_unit = 0;

				// Worker harvest links
				if (units_dat::base_property[unit->id] & UnitProperty::Worker
					&& order_id_is_harvest_related(unit->main_order_id)) {
					Unit* target_resource = unit->worker.target_resource.unit;
					if (target_resource) {
						graphics::ColorId color = graphics::CYAN;
						if (g_config.show_unique_gas_order_line_color && unit_id_is_gas_structure(target_resource->id)) {
							color = graphics::GREEN;
						}
						draw_simple_line(source, target_resource->position, color);
						ignore_resource_unit = target_resource;
					}
				}

				// Main orders
				if (unit_ext->values[ValueDiademTargeting]) {
					Target ion_target = {};
					ion_target.pt.x = unit_ext->values[ValueDiademTargetPosX];
					ion_target.pt.y = unit_ext->values[ValueDiademTargetPosY];
					source = draw_order_line(source, ion_target, graphics::RED, ignore_resource_unit);

					if (g_config.show_queued_order_lines) {
						ion_target.unit = unit_ext->units[UnitDiademTarget];
						source = draw_order_line(source, ion_target, graphics::GREY, ignore_resource_unit);
					}
				} else {
					graphics::ColorId color = graphics::GREEN;
					switch (unit->main_order_id) {
						case OrderId::Patrol:
						case OrderId::ReturnMinerals:
						case OrderId::ReturnGas:
							// TODO: Add more neutral orders
						{
							color = graphics::YELLOW;
						} break;
						case OrderId::Attack1:
						case OrderId::Attack2:
						case OrderId::AttackMove:
						case OrderId::AttackUnit:
						case OrderId::AttackFixedRange:
						case OrderId::AttackTile:
						case OrderId::TowerGuard:
						case OrderId::TowerAttack:
						case OrderId::TurretAttack:
						case OrderId::TurretGuard:
						case OrderId::PlayerGuard:
						case OrderId::StayInRange:
							// TODO: Add more aggressive orders
						{
							color = graphics::RED;
						} break;
					}

					source = draw_order_line(source, unit->order_target, color, ignore_resource_unit);
				}

				// Queued orders
				if (g_config.show_queued_order_lines && !ignore_resource_unit) { // Queued orders never work when harvesting, therefore we will not show them
					for (Order* order = unit->order_queue_head; order; order = order->next) {
						source = draw_order_line(source, order->target, graphics::GREY);
					}
				}
			}
		}

		draw_transport_ui();
	}

	void game_init_first_frame() {
#ifdef CM_DEBUG
		helper::play_sound_global(SoundId::Misc_Buzz_wav_1);
		bw::print("\tGPTP was compiled in debug!\n\tThis will make the game run slower.", GameTextColor::Yellow);
#endif
		bw::init_weapon_tile_heights();
		hooks::init_actions();
		iquare_globals.reset();

		for (s32 i = 0; i < PlayerLimit; i++) {
			// internal Zerg AI handling requires this to function properly
			// TODO: hook all the references of this in original bw
			bw::set_upgrade_level(i, UpgradeId::VentralSacs, 1);
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
		
		// Init extended features
		reset_search_index();
		update_search_index();
		init_player_data();
		init_trigger_system();
		g_ai_layouts.init();
		init_custom_ai();
		init_hotkeys();

		// Init unmovable data
		for (u32 i = 0; i < 16; i++) {
			g_unmovable_hits[i] = 0;
			g_unmovable_lastframe[i] = 0;
		}

		// Execute triggers before first frame update
		bw::set_ingame_loop_state(true);
		helper::execute_all_game_triggers(g_game_speed_replay_modifiers[g_options_list->speed]);
		bw::set_ingame_loop_state(false);

		g_active_game_mode.release();
		switch (bw::get_game_kind()) {
			case GameKind_Debug: {
				for (s32 i = 0; i < PlayerLimit; i++) {
					g_game_minerals[i] = 999999;
					g_total_minerals[i] = 999999;
					g_game_vespene[i] = 999999;
					g_total_vespene[i] = 999999;
				}
			} break;
			case GameKind_DebugFog: {
				ActiveTile* active_tile = *g_active_tile_array;
				for (u32 tile_size = g_map_tile_size->width * g_map_tile_size->height; tile_size; tile_size--) {
					active_tile->visibility_flags = 0;
					active_tile->explored_flags = 0;
					active_tile++;
				}
				if (!(g_game_layer->has_been_refreshed & 1)) {
					g_game_layer->has_been_refreshed |= 1;
					helper::bw_refresh((s16)g_game_layer->left, (s16)g_game_layer->height + (s16)g_game_layer->top  - 1,
									   (s16)g_game_layer->top,  (s16)g_game_layer->width  + (s16)g_game_layer->left - 1);
				}
			} break;
			case GameKind_Omnipresence: {
				init_omnipresence_state();
			} break;
			case GameKind_Uprising: {
				g_active_game_mode = std::make_unique<UprisingGameMode>();
			} break;
		}

		if (!bw::in_replay_or_witness()) {
			std::vector<PlayerColor> colors{};
			switch (g_player_table[*g_local_nation_id].race) {
				case RaceId::Terran: {
					colors = g_config.terran_preferred_colors;
				} break;
				case RaceId::Protoss: {
					colors = g_config.protoss_preferred_colors;
				} break;
				case RaceId::Zerg: {
					colors = g_config.zerg_preferred_colors;
				} break;
			}
			colors.insert_range(colors.end(), g_config.common_preferred_colors);

			if (colors.empty()) {
				SyncColorPrefCommand command{(u8)*g_local_nation_id};
				queue_game_command((u8*)&command, sizeof(command));
			} else {
				if (g_config.shuffle_preferred_colors) {
					std::random_device rd;
					std::mt19937 mt{rd()};
					std::ranges::shuffle(colors, mt);
				}

				for (size_t i = 0; i < colors.size(); i += 9) {
					SyncColorPrefCommand command{(u8)*g_local_nation_id};
					for (size_t j = 0; j < 9 && i + j < colors.size(); j++) {
						command.colors[j % 9] = colors[i + j];
					}
					queue_game_command((u8*)&command, sizeof(command));
				}
			}
		}
	}

	/// This hook is called every frame
	bool next_frame() {
		// Force disable cpu throttling, since on modern CPU it hurts more than helps
		*cpu_throttle = false;

		if (quickcasting_timeout && !g_is_targeting) {
			quickcasting_timeout--;
		}
		if (quickbuilding_timeout && !g_is_placing_building) {
			quickbuilding_timeout--;
		}

		s32* trigger_cooldown = (s32*)0x006509A0;
		*trigger_cooldown = 0; // Single frame hypertriggers
		if (!g_is_in_replay || g_elapsed_time_frames == 0) {
			g_options_list->speed = 6; // Lock game speed at fastest
		}

		g_discord.update_status_if_needed(true);

		if (!g_game_was_init) {
			g_game_was_init = true;
			game_init_first_frame();
		}
		update_hotkeys();

		if (bw::is_game_paused()) {
			return true;
		}

		g_graphics.reset_all_graphics();

		// Update extended features
		bw::set_ingame_loop_state(true);
		update_trigger_system(true);
		g_ai_layouts.update();
		update_custom_ai();
		hooks::update_actions();
		bw::set_ingame_loop_state(false);

		if (g_elapsed_time_frames % 2 == 0) {
			for (u32 i = 0; i < 16; ++i) {
				Ping* ping = ping_array + i;
				if (ping->timer < 8) {
					ping->timer++;
				}
			}
		}

		// Map Features
		if (g_elapsed_time_frames == 0 || g_elapsed_time_frames == 15) {
			bw::set_ingame_loop_state(true);
			load_player_names();
			bw::set_ingame_loop_state(false);
		}
		if (g_elapsed_time_frames == 0 || any_player_can_randomize_color()) {
			bw::set_ingame_loop_state(true);
			load_player_colors();
			bw::set_ingame_loop_state(false);
		}

		if (g_elapsed_time_frames == 1) {
			u32 local_flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[*g_local_nation_id];
			if (!(local_flags & VespeneDeaths_DisableQuasifog)) {
				// Default maps to "semi-explored", from DarkenedFantasies
				for (s32 i = 0; i < 4092; i++) {
					if (g_fog_of_war_mask->unexplored[i] < 8) {
						g_fog_of_war_mask->unexplored[i] = 8;
					}
				}
				bw::refresh_screen();
			}

			// Check deaths
			u8 reveal_flag = 0;
			for (u32 player_id = 0; player_id < PlayerLimit; ++player_id) {
				u32 flags = g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player_id];
				if (flags & VespeneDeaths_Reveal) {
					reveal_flag |= 1 << player_id;
				}
			}

			// Explores the entire map, requires a trigger that sets 1 death of Vespene Geyser for players in question
			if (reveal_flag) {
				// Explore map
				/*
				for (u32 x = 0; x < g_map_tile_size->width; x++) {
					for (u32 y = 0; y < g_map_tile_size->height; y++) {
						ActiveTile& tile = bw::get_active_tile_at(x * 32, y * 32);
						tile.explored_flags &= ~reveal_flag;
					}
				}
				*/

				// Reveal resources
				for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
					if (unit_id_is_resource_node(unit->id, false)) {
						unit->sprite->visibility_status |= reveal_flag;
					}
				}
			}

			if (local_flags & VespeneDeaths_Ping) {
				for (u32 target = 0; target < PLAYABLE_PLAYER_COUNT; ++target) {
					if (!g_globals->players[target].witness_mode) {
						u32 color = 0; // red
						if (g_globals->players[*g_local_nation_id].witness_mode) {
							color = 7; // yellow
						} else if (bw::is_allied_to(*g_local_nation_id, target)) {
							color = 8; // green
						}
						bw::minimap_ping(g_start_positions[target].x, g_start_positions[target].y, color, *g_local_nation_id);
					}
				}
			}
		}

		*g_ai_update_cooldown = 0; // We want the AI to update spending requests every tick, so we can have precise 
		*g_can_update_powered_status = true; // We want to have dynamic power updates, so we update power every frame
		for (u32 i = 0; i < 8; ++i) {
			g_race_supply[RaceId::Terran].provided[i] = 60000;
		}

		bw::set_ingame_loop_state(true); // Needed for bw::random() to work
		hooks::update_psi_field_providers();

		reset_search_index();
		update_search_index();
		iquare_globals.frame_hook();
		update_search_index_effects();
		update_ext_variables();
		update_threshold();
		if (g_active_game_mode) {
			g_active_game_mode->tick();
		}

		bw::refresh_screen();

		// Needed for extended tooltips
		static bool last_shift = 0;
		bool new_shift = *g_is_holding_shift;
		if (last_shift != new_shift) {
			bw::refresh_console();
		}
		last_shift = new_shift;

		// Implict ESC for closing submenus. -Pr0nogo
		// @Veeq7 - Should this be moved to hotkeys.cpp?
		if (g_is_keycode_used[VK_ESCAPE] && bw::buttonset_id_is_submenu(*g_active_portrait_buttonset_id) && !*g_is_game_paused) {
			hooks::update_buttonset();
		}
		
		draw_custom_ui();

		// Loop through bullets
		for (Bullet* bullet = *first_bullet; bullet; bullet = bullet->next) {
			if (weapon_id_is_incremental(bullet->weapon_id)) {
				if (bullet->order_signal & 0x80) {
					bullet->order_signal &= ~0x80;
					iquare_globals.remove_multihit_bullet(bullet);
				}
			}
		}

		process_quickbuilding();

		// Sync camera
		struct SyncCameraCommand {
			u8 type = GameCommand_Custom;
			u8 subtype = NewGameCommand::CameraSync;
			u8 player;
			Point16 pos;
			u8 padding[5];
		};
		assert(sizeof(SyncCameraCommand) == 0xc);

		Point16 camera = {};
		camera.x = *g_move_to_x;
		camera.y = *g_move_to_y;

		SyncCameraCommand command = {};
		command.player = *g_local_human_id;
		command.pos = camera;
		helper::queue_game_command((u8*)&command, 0xc);

		// Count workers
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit_id_is_resource_node(unit->id, true)) {
				UnitExt* unit_ext = get_unit_ext(unit);
				unit_ext->values[ValueAssignedHarvesters] = 0;
				unit_ext->values[ValueActiveHarvesters] = 0;
				unit_ext->values[ValueAssignedAiGasHarvesters] = 0;
			}
		}
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (!(units_dat::base_property[unit->id] & UnitProperty::Worker)) {
				continue;
			}
			UnitExt* unit_ext = get_unit_ext(unit);
			if (!order_id_is_harvest_related(unit->main_order_id)) {
				unit_ext->values[ValueHarvesterActivity] = 0;
				continue;
			}
			Unit* target = unit->worker.target_resource.unit;
			if (!target) {
				unit_ext->values[ValueHarvesterActivity] = 0;
				continue;
			}
			
			UnitExt* target_ext = get_unit_ext(target);
			target_ext->values[ValueAssignedHarvesters] += 1;
			if (unit->unit_ai && unit->unit_ai->type == AiType::Worker &&
					unit->unit_ai->worker_ai.target_resource == AiTargetResource_Gas) {
				target_ext->values[ValueAssignedAiGasHarvesters] += 1;
			}
			if (!helper::is_target_within_min_range(unit, 10, target) && !helper::is_unit_movable(unit)) {
				unit_ext->values[ValueHarvesterActivity] = 0;
				continue;
			}
			
			if (unit->worker.is_harvesting) {
				unit_ext->values[ValueHarvesterActivity] += 1;
			} else {
				unit_ext->values[ValueHarvesterActivity] = 1;
			}
			if (bw::harvester_is_active(unit) || bw::harvester_is_waiting(unit)) {
				// Always consider waiting workers active, since it's almost never optimal to triple harvest a node,
				// when you can double harvest another one, and that's what otherwise will often happen
				target_ext->values[ValueActiveHarvesters] += 1;
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
		bw::set_ingame_loop_state(false);

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
			case UnitId::Omnivale:
			{
				// Spread initial creep
				if (unit_spreads_custom_creep(unit)) {
					bw::spawn_custom_creep(unit);
				}
			} break;
			case UnitId::TerranGoliath: {
				helper::turn_unit(unit, 96);
				if (unit->subunit) {
					helper::turn_unit(unit->subunit, 96);
				}
			} break;
			case UnitId::TerranSentinel: {
				helper::turn_unit(unit, 96);
			} break;
			case UnitId::TerranWellbore:
			case UnitId::ZergEzigrisant:
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			case UnitId::ResourceVespeneGeyser:
			case UnitId::ResourceVespeneRidge: {
				unit->building.resource.resource_amount = get_harvest_amount(unit);
			} break;
			case UnitId::ResourceMineralField:
			case UnitId::ResourceMineralFieldType2:
			case UnitId::ResourceMineralFieldType3:
			{
				if (unit->building.resource.resource_amount < 4) {
					unit->building.resource.resource_amount = 2;
					unit->status |= UnitStatus::NoCollide;
				}
			} break;
			case UnitId::ZergNydusCesiant:
			{
				if (unit->player == 11) {
					unit_ext->values[ValueIsNeutralNydus] = 1;
				}
			} break;
			case UnitId::ProtossVassalOld:
			{
				helper::replace_unit_with_id(unit, UnitId::ProtossVassal);
			} break;
			case UnitId::ProtossAccantorOld:
			{
				helper::replace_unit_with_id(unit, UnitId::ProtossAccantor);
			} break;
			case UnitId::ProtossSolarion:
			{
				u32 missing_count = 12 - (unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count);
				if (missing_count) {
					bw::create_hangar_units(unit, missing_count, UnitId::ProtossSteward);
				}
			} break;
		}

		// Fixes first attack from Paladin (and maybe other units) aiming upwards
		unit->velocity_direction_2 = unit->velocity_direction_1;

		if (unit->player < PlayerLimit) {
			unit_ext->values[ValuePlayerColor] = (u32)g_player_data.player_color_list[unit->player];
		}

		if (bw::get_game_kind() == GameKind_Massacre) {
			if (unit_id_is_vespene_node(unit->id)) {
				unit->remove();
				return;
			}
		}
	}

	void tick_unit(Unit* unit, bool unit_is_hidden) {
		init_unit(unit);

		UnitExt* unit_ext = get_unit_ext(unit);
		/*
		if (unit_ext->values[ValueUnitIdName] != unit->id + 1) {
			unit_ext->values[ValueUnitIdName] = unit->id + 1;

			if (bw::rand_between(1, 256) == 1) {
				unit_ext->custom_name = get_random_easter_egg_unit_name(unit->id);
			}
		}
		*/

		Unit* overlay_unit = unit;
		if (unit->status & UnitStatus::InTransport && unit->connected_unit) {
			Unit* transport = unit->connected_unit;

			if (unit->position.x != transport->position.x || unit->position.y != transport->position.y) {
				bw::move_unit_force(unit, transport->position.x, transport->position.y);
			}

			if (unit_is_hidden) {
				overlay_unit = transport;
			}
		}

		if (unit->sprite) {
			SpriteExt* sprite_ext = get_sprite_ext(unit->sprite);
			sprite_ext->owner = unit;
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon) {
			UnitExt* addon_ext = get_unit_ext(unit->building.addon);
			addon_ext->units[UnitAddonParent] = unit;
		}

		if (!(unit->movement_flags & MovementFlags_StartingAttack)) {
			unit_ext->values[ValueProjectilePattern] = 0;
		}

		if (g_active_game_mode) {
			g_active_game_mode->tick_unit(unit);
		}

		/*
		if (bw::unit_id_is_resource_node(unit->id, true)) {
			bool show = keyboard.held(VK_MENU);
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

		// Overlay for units which do not leave behind corpses
		if (unit_ext->values[ValueNoCorpse] && !unit->sprite->get_overlay(ImageId::RevivedDecayingOverlay)) {
			unit->sprite->create_overlay(ImageId::RevivedDecayingOverlay);
		}

		// Handle behaviors related to Ocsal origins
		if (unit_ext->values[ValueOriginatedFromOcsalLarva]) {
			// Overlay for units which do not regress on death
			if (!unit->sprite->get_overlay(ImageId::OcsalRegressionOverlay)) {
				if (unit->id != UnitId::ZergOcsalLarva && unit->id != UnitId::ZergEgg && unit->id != UnitId::ZergCocoon) {
					bw::add_unit_overlay(unit, ImageId::OcsalRegressionOverlay);
				}
			}
		}
		
		// Overlay for units which are mind-controlled
		if ((unit_ext->values[ValueMindControlPlayer] || unit_ext->timers[TimerEmpressAnodyneAnnex])
			&& !unit_ext->values[ValueControlledByDemiurge] && !unit->sprite->get_overlay(ImageId::TyrannyOverlay))
		{
			bw::add_unit_overlay(unit, ImageId::TyrannyOverlay);
		}

		// Reclamation - Decaying
		if (unit_ext->values[ValueDecaying]) {
			if (!(unit->status & UnitStatus::Cloaked)) {
				if (!unit->sprite->get_overlay(ImageId::RevivedDecayingOverlay)) {
					unit->sprite->create_overlay(ImageId::RevivedDecayingOverlay);
				}
			}

			if (!unit_ext->timers[TimerReclaiming]) {
				unit->damage_hp((3 * 256) / 24, 0, unit->player, false);
			}
		}

		// Reclamation - Reclaiming
		if (unit_ext->timers[TimerReclaiming]) {
			unit->heal_hp((25 * 256) / (2 * 24));
			if (unit_ext->timers[TimerReclaiming] == 1) {
				unit->sprite->create_overlay(ImageId::LazarusAgentCast);
				// Play sound for the end of healing and stun
			} else if (unit_ext->timers[TimerReclaiming] % 12 == 1) {
				unit->sprite->create_overlay(ImageId::LazarusAgentCast);
				// Play sound for healing and stun
			}
		}

		// Revive
		if (((!unit_ext->timers[TimerReviveFlagClearMin] && !(unit->status & UnitStatus::NoBrkCodeStart)) || !unit_ext->timers[TimerReviveFlagClearMax])
			&& unit_ext->values[ValueReviveStarted]) {
			if (unit_ext->timers[TimerStun] > 1) {
				unit->status &= ~UnitStatus::Invincible;
			} else {
				++g_unmovable_hits[2];
				g_unmovable_lastframe[2] = g_elapsed_time_frames;
				unit->status &= ~(UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
			}
			unit_ext->timers[TimerReviveFlagClearMin] = 0;
			unit_ext->timers[TimerReviveFlagClearMax] = 0;
			unit_ext->values[ValueReviveStarted] = 0;
			bw::give_ai_ex(unit);
		}

		// Random movement for The Turning zombies
		if (bw::get_game_kind() == GameKind_TheTurning && unit->player == NeutralHostilePlayerId) {
			if (unit->main_order_id == OrderId::PlayerGuard) {
				unit->order_to_idle();
			}
			if (unit->main_order_id != OrderId::JunkYardDog && !unit_ext->timers[TimerTurningRoam]) {
				helper::add_order_targeting_nothing(unit, unit->order_queue_head, OrderId::JunkYardDog);
				unit_ext->timers[TimerTurningRoam] = 3;
			}
		}

		u32 creep_spreader_id = bw::unit_get_creep_spreader_id(unit);
		if (creep_spreader_id != UnitId::None && unit->player < PlayerLimit) {
			Box32 creep_area = {};
			helper::get_creep_spread_area(creep_spreader_id, unit->position.x, unit->position.y, unit->status & UnitStatus::Completed, &creep_area);
			for (u32 y = creep_area.top; y <= creep_area.bottom; ++y) {
				for (u32 x = creep_area.left; x <= creep_area.right; ++x) {
					s32 delta_x = unit->position.x - (x * 32 + 16);
					s32 delta_y = unit->position.y - (y * 32 + 16);
					if (delta_x * delta_x * 100 + delta_y * delta_y * 256 <= 10240000) {
						ActiveTile* tile = bw::get_active_tile_at_tile(x, y);
						if (tile->has_cagra || tile->temporary_cagra) {
							TileExt* tile_ext = get_tile_ext_at_tile(x, y);
							tile_ext->creep_owners |= 1 << unit->player;
						}
					}
				}
			}
		}

		// Clear stagger blacklist
		if (unit_ext->timers[TimerClearStaggerBlacklist]) {
			unit_ext->unit_vectors[UnitVectorStaggerBlacklist].clear();
			unit_ext->values[ValueStaggerCount] = 0;
			apply_weapon_cooldown(unit, unit_ext->values[ValueStaggerWeaponId]);
		}

		// Projection (line attack) handling
		if (unit_id_has_projection_attack(unit->id)) {
			bool remove = false;
			if (unit->order_signal & 0x20) {
				unit->order_signal &= ~0x20;
				remove = true;
			} else if (unit->subunit && unit->subunit->is_subunit() && unit->subunit->order_signal & 0x20) {
				unit->subunit->order_signal &= ~0x20;
				remove = true;
			}

			if (remove) {
				iquare_globals.remove_unit_from_line_attack_table(unit, true);
			}
		}

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

		if (unit_ext->timers[TimerWendigoShatteredWinds] == 1) {
			unit_ext->values[ValueWendigoShatteredWindsStacks] = 0;
		}

		if (unit_ext->timers[TimerStun] > 1) {
			++g_unmovable_hits[3];
			g_unmovable_lastframe[3] = g_elapsed_time_frames;
			unit->status |= UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
		} else if (unit_ext->timers[TimerStun] == 1) {
			++g_unmovable_hits[4];
			g_unmovable_lastframe[4] = g_elapsed_time_frames;
			unit->status &= ~(UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
		}

		// Unit behavior groups
		const static u32 broodspawn_max_time = 5 * 24; // TODO: move to unit consts?
		switch (unit->id) {
			// Sires
			case UnitId::ZergZoryusthaleth:
			case UnitId::ZergKonvilisk:
			case UnitId::ZergVilgoleth:
			case UnitId::ZergMatraleth:
			case UnitId::ZergTetcaerokor:
			{
				EntryArray broodspawns = {};
				unit_search_radial(&broodspawns, unit->position, unit->get_sight_range() * 32);
				unit_search_filter(&broodspawns, unit, FilterAlly, {UnitId::ZergRilirokor, UnitId::ZergRililisk});
				for (u32 i = 0; i < broodspawns.count; ++i) {
					Unit* broodspawn = broodspawns.units[i];
					broodspawn->remove_timer = broodspawn_max_time;
				}
			} break;
			// Broodspawn
			case UnitId::ZergRilirokor:
			case UnitId::ZergRililisk:
			{
				if (unit->remove_timer > broodspawn_max_time) {
					unit->remove_timer = broodspawn_max_time;
				}
			} break;
		}

		bool should_update_speed = false;

		// Unit-specific
		switch (unit->id) {
			case UnitId::ProtossHerald: {
				EntryArray allies = {};
				unit_search_radial(&allies, unit->position, 2 * 32);
				unit_search_filter(&allies, unit, FilterAlly);

				for (u32 i = 0; i < allies.count; ++i) {
					Unit* ally = allies.units[i];
					UnitExt* ally_ext = get_unit_ext(ally);
					ally_ext->timers[TimerHeraldAvenge] = 12;
				}
			} break;
			case UnitId::ZergVilgoleth: {
				if (!unit_ext->timers[TimerVilgolethSpawn]) {
					Thingy* corpse = get_closest_active_corpse(unit->position, 5 * 32);
					if (corpse && corpse->sprite->id != SpriteId::Corpse_Rilirokor) {
						unit_ext->timers[TimerVilgolethSpawn] = 24;
						Point16 pos = corpse->sprite->position;
						bw::create_unit_from_ability(UnitId::ZergRilirokor, 2, unit->player, pos);
						bw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, pos.x, pos.y - 12, unit->player, corpse->sprite->elevation + 1);
						consume_corpse(corpse);
						unit->sprite->create_overlay(ImageId::LazarusAgentCast);
					}
				}

				if (unit->player != 11 && !unit_ext->timers[TimerVilgolethTimedLife]) { // Ignores P12 Vilgoleths to make preplaced ones permanent
					unit_ext->timers[TimerVilgolethTimedLife] = 10 * 24;
				} else if (unit_ext->timers[TimerVilgolethTimedLife] == 1) {
					unit->remove();
				}
			} break;
			case UnitId::ZergLarva:
			case UnitId::ZergOcsalLarva:
			{
				Unit* building = unit->connected_unit;
				if (building && building->status & UnitStatus::Completed) {
					unit_ext->values[ValueSwarmhold] = unit_id_swarmhold_value(building->id);
					u32 unit_id = unit_id_get_larva_type(building->id);
					if (unit_id != UnitId::None && unit->id != unit_id) {
						helper::replace_unit_with_id(unit, unit_id);
					}
				}

				unit_ext->values[ValueOriginatedFromOcsalLarva] = unit->id == UnitId::ZergOcsalLarva;
			} break;
			case UnitId::ZergDroleth:
			case UnitId::ZergGosvileth:
			{
				unit_ext->values[ValueOriginatedFromGosvileth] = unit->id == UnitId::ZergGosvileth;

				if (unit->id == UnitId::ZergDroleth) {
					// Temporary? fix for order queue insanity. -Pr0nogo
					// DOES NOT FIX AI!
					if (unit->main_order_id == OrderId::MorphUnit
					&&	unit->build_queue[unit->build_queue_slot] != UnitId::None
					&&  unit->build_queue[unit->build_queue_slot] != UnitId::ZergGosvileth)
					{
						unit->build_queue[unit->build_queue_slot] = UnitId::ZergGosvileth;
					}
				}

				if (unit->main_order_id == OrderId::DroneLand) {
					// TODO: check if it collides with a building, and order it to decollide, without the conditionals being too expensive or naive. -Pr0nogo
				}
			} break;
			case UnitId::ZergEvigrilisk: {
				bool is_moving = unit->movement_flags & MovementFlags_Accelerating || unit->movement_flags & MovementFlags_Moving;
				bool can_fire = !unit->ground_weapon_cooldown;
				u32 weapon_id = WeaponId::EvigriliskBrutishTendrils;
				if (is_moving && can_fire) {
					FireMultipleOptions options = {};
					options.ignore_main = true;
					options.use_weapon_factor = false;
					if (fire_multiple(unit, weapon_id, 4, FilterEnemy, options)) {
						helper::play_sound_from_unit(bw::rand_between(SoundId::EvigriliskFire01, SoundId::EvigriliskFire03), unit, 1, 0);
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
							target_ext->timers[TimerSalvageStunTimer] = 2 * 24 + 1;
							
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

							if (!(unit->movement_flags & MovementFlags_Accelerating || unit->movement_flags & MovementFlags_Moving)
								&& !unit_ext->timers[TimerTalosAnimationSubtimer]) {

								u32 width  = units_dat::building_dimensions[target->id].x * 0.75;
								u32 height = units_dat::building_dimensions[target->id].y * 0.75;

								s32 x_offset = bw::random() % width  - width  / 2;
								s32 y_offset = bw::random() % height - height / 2;

								x_offset = MathUtils::clamp(x_offset, -127, 128);
								y_offset = MathUtils::clamp(y_offset, -127, 128);

								Unit* final_target = unit;
								if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit) {
									final_target = unit->subunit;
								}
								final_target->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
								unit_ext->timers[TimerTalosAnimationSubtimer] = bw::rand_between(3, 7);
							}

							if (!unit_ext->values[ValueTalosAnimationTimer]
							||   unit_ext->values[ValueTalosAnimationTimer] % 128 == 0)
							{
								Point16 pos = target->sprite->position;
								switch (bw::random() % 4) {
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
						should_update_speed = true;

						bw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
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
				should_update_speed = true;

				// Corpse Cocoon
				EntryArray corpses = {};
				corpse_search_rect(&corpses, {unit->get_left(), unit->get_top(), unit->get_right(), unit->get_bottom()});
				for (u32 i = 0; i < corpses.count; ++i) {
					Thingy* corpse = corpses.corpses[i];
					if (!(corpse->sprite->flags & SpriteFlags::RemoveAfterDeath)) {
						Unit* egg = bw::create_unit_in_game(UnitId::ZergEgg, unit->player, corpse->sprite->position.x, corpse->sprite->position.y, 0, false);
						if (egg) {
							set_custom_egg(egg, UnitId::ZergSovroleth, 4 * 24);
						}
						UnitExt* egg_ext = get_unit_ext(egg);
						egg_ext->values[ValueNoCorpse] = true;
						corpse->sprite->flags |= SpriteFlags::RemoveAfterDeath;
						corpse->sprite->play_iscript_animation(IscriptAnimation::Death);
					}
				}
			} break;
			case UnitId::ZergEgg:
			case UnitId::ZergCocoon:
			case UnitId::ZergQuazilisk:
			case UnitId::ZergNathrokor:
			{
				// Gimmick to allow morph cancellation animations to play correctly
				if (unit_ext->timers[TimerQuaziliskCancel]) {
					if (unit->sprite->main_graphic->animation != IscriptAnimation::Unused2) {
						unit->sprite->play_iscript_animation(IscriptAnimation::Unused2, true);
					}
				}

				if (unit_ext->timers[TimerCustomEggHatch]) {
					u32 morph_unit_id = unit->build_queue[unit->build_queue_slot];
					f32 time_factor = unit_ext->timers[TimerCustomEggHatch] / (f32)unit_ext->values[ValueCustomEggHatchMax];
					unit->remaining_build_time = units_dat::time_cost[morph_unit_id] * time_factor;

					if (unit_ext->timers[TimerCustomEggHatch] == 1 + 19 && morph_unit_id == UnitId::ZergSovroleth) {
						unit->play_iscript_animation(IscriptAnimation::GndAttkInit);
					}
					if (unit_ext->timers[TimerCustomEggHatch] == 1) {
						f32 hp_factor = (f32)unit->hitpoints / unit_ext->values[ValueCustomEggHpMax];
						helper::replace_unit_with_id(unit, morph_unit_id);
						unit->set_hitpoints(units_dat::max_hitpoints[morph_unit_id] * hp_factor);
						bw::give_ai_ex(unit);
						helper::play_unit_ready_sound(unit);
						unit->build_queue[unit->build_queue_slot] = UnitId::None;
						helper::unit_completion_init(unit, 1);
						unit_ext->values[ValueSpecialSovrolethBirth] = morph_unit_id == UnitId::ZergSovroleth;
						unit->order_computer(OrderId::ZergBirth);
					}
				}
				
				if ((unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon) && unit->order_signal & 0x8) {
					unit->order_signal &= ~0x8;
					helper::replace_sprite_images(unit->sprite, unit->id == UnitId::ZergEgg ? ImageId::Egg : ImageId::GuardianCocoon, 0);
					unit->sprite->play_iscript_animation(IscriptAnimation::GndAttkToIdle, true);
				}
			} break;
			case UnitId::ProtossStarSovereign:
			{
				// Star Sovereign - Intervention Missiles
				if (!unit_ext->timers[TimerStarSovereignInterventionMissiles]) {
					auto radius = 5 * 32;
					auto match_func = cosmonarchy_finder_match(unit, Flags::Enemy | Flags::FrontalArc, {});
					// TODO: unit search -Pr0nogo
					bw::UnitFinder unit_finder(
						unit->position.x - radius,
						unit->position.y - radius,
						unit->position.x + radius,
						unit->position.y + radius
					);
					bool enemies_detected = false;
					for (s32 i = 0; i < unit_finder.get_unit_count(); i++) {
						Unit* picked_unit = unit_finder.get_unit(i);
						if (match_func(picked_unit) && unit->can_attack_target(picked_unit)) {
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
							auto X = unit->position.x + offset.x;
							auto Y = unit->position.y + offset.y;
							auto target_unit = unit->order_target.unit;
							auto target_pos = unit->order_target.pt;
							auto offset_direction = -40 + arc * 16;
							auto max_range = weapons_dat::max_range[WeaponId::StarSovereignInterventionMissiles];
							auto polar_x = unit->position.x + bw::get_polar_x(max_range, bw::dir_saturate(unit->current_direction_1, offset_direction));
							auto polar_y = unit->position.y + bw::get_polar_y(max_range, bw::dir_saturate(unit->current_direction_1, offset_direction));
							unit->order_target.unit = 0;
							unit->order_target.pt.x = polar_x;
							unit->order_target.pt.y = polar_y;
							helper::create_bullet(WeaponId::StarSovereignInterventionMissiles, unit, X, Y, unit->player, bw::dir_saturate(unit->current_direction_1, offset_direction));
							helper::play_sound_from_unit(bw::rand_between(SoundId::StarSovereignMissileFire01, SoundId::StarSovereignMissileFire03), unit, 1, 0);
							unit->order_target.unit = target_unit;
							unit->order_target.pt = target_pos;
						}
					}
				}

				// Star Sovereign - Grief of All Gods
				if (unit_ext->timers[TimerStarSovereignGriefOfAllGodsChannel] == 1) {
					iquare_globals.create_effect(EffectId::StarSovereignTheGriefOfAllGods, 360, unit->position, 8 * 32, SpriteId::None, unit->sprite->elevation, unit->player, unit, { .origin = unit });
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

				// Manually handle the reset if stunned during channel
				bw::unfreeze_channeling_unit(unit);
			} break;
			case UnitId::TerranApostle:
			{
				// Apostle - Lazarus Agent cast
				if (bw::get_generic_rust_value(unit, RustId::ToggleLazarusAgent) == 1) {
					u32 energy = techdata_dat::energy_cost[TechId::ApostleLazarusAgent] * 256;
					if (unit->has_energy(energy) && !unit->is_dead()) {
						if (!unit_ext->timers[TimerReclaimInterval]) {
							unit_ext->timers[TimerReclaimInterval] = 24;
							bw::decloak_passive_proc(unit);
							unit->spend_unit_energy(energy);
							unit->sprite->create_top_overlay(ImageId::LazarusAgentCast);
							helper::play_sound_from_unit(SoundId::LazarusAgentCastTick, unit, 1, 0); // TODO: Use Start and End when applicable

							Thingy* corpse = get_closest_active_corpse(unit->position, 3 * 32);
							if (corpse) {
								reclaim_corpse(corpse, unit->player);
								break;
							}
						}
					} else {
						bw::set_generic_rust_value(unit, RustId::ToggleLazarusAgent, 0);
					}
				}
			} break;
			case UnitId::ProtossLanifect:
			{
				// Disruption Field - Lanifect ability
				if (bw::get_generic_rust_value(unit, RustId::ToggleDisruptionField)) {
					if (!unit_ext->values[ValueDisruptionFieldOn]) {
						unit_ext->values[ValueDisruptionFieldOn] = 1;
						iquare_globals.create_effect(EffectId::LanifectDisruptionField, -1, {unit->position.x, unit->position.y}, 0, SpriteId::Disruption_Web, 0, unit->player, unit, { .origin = unit });
					} else if (unit->is_frozen()) {
						bw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
					}
				} else {
					unit_ext->values[ValueDisruptionFieldOn] = 0;
				}
				if (unit_ext->values[ValueDisruptionFieldOn]) {
					bw::decloak_passive_proc(unit);
				}
			} break;
			case UnitId::TerranMadcap:
			{
				// Madcap Cabeirus Feeders - subtract stacks while moving
				if (unit->movement_flags & MovementFlags_Accelerating || unit->movement_flags & MovementFlags_Moving) {
					if (unit_ext->values[ValueMadcapStacks] > 0) {
						unit_ext->values[ValueMadcapStacks]--;
						unit_ext->values[ValueMadcapFullChargeRead] = 0;
					}
				}
			} break;
			case UnitId::TerranCuirassTankMode:
			{
				if (unit_ext->values[ValueCuirassIsSelfReviving]) {
					if (unit->hitpoints + CuirassSelfHeal >= unit->get_max_hp_raw()) {
						unit_ext->values[ValueCuirassIsSelfReviving] = 0;
						unit->hitpoints = unit->get_max_hp_raw();
						bw::restore_func(unit);
						unit->order_computer(OrderId::Stop);
						++g_unmovable_hits[5];
						g_unmovable_lastframe[5] = g_elapsed_time_frames;
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
			case UnitId::ProtossCabalist:
			{
				// Legionnaire - Blade Vortex
				// Cabalist - Silent Strides
				// Striga was here - removed 7 February 2023. -Pr0nogo
				Unit* target = unit->order_target.unit;
				if (target && (unit->main_order_id == OrderId::Attack1 || unit->main_order_id == OrderId::AttackUnit) && !unit->ground_weapon_cooldown) {
					u32 weapon_id = target->status & UnitStatus::InAir ? unit->get_air_weapon() : unit->get_ground_weapon();
					u32 min_radius = weapons_dat::max_range[weapon_id];
					u32 max_radius = 3 * 32;

					u32 distance = unit->get_distance_to_target(target);
					bool do_teleport = false;
					u32 image_id = 0;
					if (unit->id == UnitId::ProtossLegionnaire) {
						if (distance > min_radius && distance <= max_radius && bw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0) {
							do_teleport = true;
							image_id = ImageId::LegionnaireTeleportEffect;
						}
					} else if (unit->id == UnitId::ProtossCabalist) {
						if (distance > min_radius && distance <= max_radius && unit_ext->timers[TimerDecloak] < 2) {
							do_teleport = true;
							image_id = ImageId::LegionnaireTeleportEffect;
						}
					}

					if (do_teleport) {
						if (bw::teleport_to_unit(unit, target, min_radius, 0, true)) {
							if (!unit->get_overlay(image_id)) {
								bw::add_unit_overlay(unit, image_id);
							}
							bw::decloak_passive_proc(unit);
						}
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
				if (!unit->is_dead() && unit_ext->timers[TimerMissileReturnETA] == 1) {
					bw::launch_return_missile(unit, WeaponId::LakiziliskSubterraneanSpines);
				}
			} break;
			case UnitId::TerranShamanSupportMode:
			case UnitId::TerranShamanVisionaryMode:
			{
				// Shaman - Nanite Field
				if (unit->is_frozen() || unit_ext->values[ValueReviveStarted] || unit->status & UnitStatus::InTransport) {
					break;
				}
				u32 timer = unit_ext->timers[TimerNaniteField];
				if (!timer) {
					unit_ext->timers[TimerNaniteField] = 24;
					bw::add_unit_overlay(overlay_unit, ImageId::RestorationHit_Small + bw::get_overlay_offset(unit->id));

					EntryArray unit_array = {};
					u32 range = unit->id == UnitId::TerranShamanVisionaryMode ? 6 : 3;
					unit_search_radial(&unit_array, unit->position, range * 32);
					unit_search_add_units_from_connected_transport(&unit_array, unit);
					unit_search_filter(&unit_array, unit, FilterAlly | FilterNoBuilding);

					for (u32 i = 0; i < unit_array.count; ++i) {
						Unit* target = unit_array.units[i];
						shaman_nanite_field_proc(unit, 4, target);
					}
				}
			} break;
			case UnitId::TerranQilin:
			{
				if (unit->is_frozen() || unit_ext->values[ValueReviveStarted] || unit->status & UnitStatus::InTransport) {
					break;
				}
				u32 timer = unit_ext->timers[TimerNaniteField];
				if (!timer) {
					unit_ext->timers[TimerNaniteField] = 24;
					u32 counter = 0;
					bool has_cargo = false;
					for (; counter < units_dat::space_provided[unit->id]; ++counter) {
						Unit* cargo = helper::get_loaded_unit_from_index(unit, counter);
						if (!cargo) {
							break;
						}
						has_cargo = true;
						cargo->heal_hp(4 * 256, unit);
					}
					if (has_cargo) {
						bw::add_unit_overlay(overlay_unit, ImageId::RestorationHit_Small + bw::get_overlay_offset(unit->id));
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
						if (target && unit->get_distance_to_target(target) <= CentaurMissileDistance && !unit_ext->timers[TimerSecondaryWeaponCooldown])
						{
							unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::CentaurSunchaserMissiles);
							unit_ext->values[ValueCentaurSecondaryWeaponCount] = 4;
							unit_ext->units[UnitCentaurSecondaryWeaponTarget] = target;
							helper::play_sound_from_unit(bw::rand_between(SoundId::CentaurSunchaserMissilesFire01, SoundId::CentaurSunchaserMissilesFire04), unit);
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
					fire_bullet(unit, WeaponId::CentaurSunchaserMissiles, make_target(target), &offsets, false);
				}
			} break;
			case UnitId::TerranOlympian:
			{
				fire_secondary_melee_weapon_if_needed(unit, WeaponId::OlympianTitansFist);
			} break;
			case UnitId::ProtossPotentate:
			{
				fire_secondary_melee_weapon_if_needed(unit, WeaponId::PotentatePhaseSaber);
			} break;
			case UnitId::ZergMatraleth:
			case UnitId::InfestedKerriganExt:
			{
				// Matraleth - Ensnaring Brood
				EntryArray unit_array = {};
				unit_search_radial(&unit_array, unit->position, 4 * 32);
				unit_search_filter(&unit_array, unit, FilterAlly | FilterNoBuilding | FilterCompleted);

				for (u32 i = 0; i < unit_array.count; ++i) {
					Unit* target = unit_array.units[i];
					if (!unit_is_combatant(target)) {
						continue;
					}

					UnitExt* target_ext = get_unit_ext(target);
					target_ext->timers[TimerEnsnaringBrood] = 3;
					if (!(target->status & UnitStatus::Burrowed) && !target->get_overlay(ImageId::EnsnaringBroodOverlay)) {
						bw::add_unit_overlay(target, ImageId::EnsnaringBroodOverlay);
					}
				}
				if (unit->id == UnitId::ZergMatraleth && unit->order_signal & 32) {
					unit->order_signal &= ~32;
					bw::decloak_passive_proc(unit);
				}
			} break;
			case UnitId::ZergSasyrvoleth:
			{
				// Sasyrvoleth - Lucilleri Pheremones
				EntryArray unit_array = {};
				unit_search_radial(&unit_array, unit->position, 5 * 32);
				unit_search_filter(&unit_array, unit, FilterAlly | FilterNoBuilding | FilterCompleted);
				for (u32 i = 0; i < unit_array.count; ++i) {
					Unit* target = unit_array.units[i];
					if (!unit_is_combatant(target)) {
						continue;
					}

					UnitExt* target_ext = get_unit_ext(target);
					target_ext->timers[TimerLucilleriPheremones] = 3;
					u32 overlay_id = ImageId::DermalAccelerantSmall + bw::get_overlay_offset(target->id, true);
					if (!(target->status & UnitStatus::Burrowed) && !target->get_overlay(overlay_id)) {
						bw::add_unit_overlay(target, overlay_id);
					}
				}
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Phase Link
				if (bw::get_aise_value(unit, 0, AiseId::SendClarionValue, 0, 0) == 0) {
					bw::send_gptp_aise_command(unit, 0, GptpId::SetPhaseLinkStatus, 1, 0, 0);
				}
			} break;
			case UnitId::TerranMaverick:
			{
				// Decloak on Stimpack activation
				if (unit->stim_timer == StimpackTimer) {
					bw::decloak_passive_proc(unit);
				}
			} break;
			case UnitId::TerranWyvern:
			{
				if (bw::get_generic_rust_value(unit, RustId::CancelLandGrab)) {
					bw::set_generic_rust_value(unit, RustId::CancelLandGrab, 0);
					unit_ext->values[ValueWyvernLandGrabOn] = 0;
				}
			} break;
			case UnitId::TerranTarasque:
			{
				// Forward March
				if (!(unit->status & UnitStatus::InTransport) && !unit_ext->timers[TimerSecondaryWeaponCooldown]) {
					FireMultipleOptions options = {};
					options.apply_cooldown = false;
					if (fire_multiple(unit, WeaponId::TarasqueAcceleratedPlasmid, 5, FilterEnemy, options)) {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::TarasqueAcceleratedPlasmid);
					} else {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranAnticthon:
			{
				// Revive status
				if (!unit_ext->values[ValueReviveStarted] && !unit_ext->timers[TimerReclaimInterval]) {
					unit_ext->timers[TimerReclaimInterval] = 24;
					Thingy* corpse = get_closest_active_corpse(unit->position, 3 * 32);
					if (corpse) {
						reclaim_corpse(corpse, unit->player);
						break;
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
						u32 dist = bw::get_distance_fast(unit->position, target->position);
						if (dist <= weapons_dat::outer_splash_radius[WeaponId::AnthelionEventHorizon] && unit->can_attack_target(target)) {
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
						helper::play_sound_from_unit(bw::rand_between(SoundId::AnthelionWeaponFire01, SoundId::AnthelionWeaponFire03), unit, 1, 0);
					}
				}
				if (unit->main_order_id == OrderId::Follow || unit->main_order_id == OrderId::AttackUnit) {
					unit->order_to(OrderId::Move, unit->order_target.pt.x, unit->order_target.pt.y);
				}

				if (unit->main_order_id == OrderId::Move) {
					Point16 target = unit->move_target.pt;
					if (bw::get_active_tile_at_coords(target.x, target.y)->visibility_flags & (1 << unit->player)) {
						unit_ext->anthelion_last_move_target = {};
					} else {
						if (unit_ext->anthelion_last_move_target != target) {
							if (unit_ext->timers[TimerAnthelionEndlessEclipse] > 1) {
								helper::play_sound_from_unit(bw::rand_between(SoundId::AnthelionMoveCharge01, SoundId::AnthelionMoveCharge02), unit, 1, 0);
							}
							unit_ext->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
						}
						unit_ext->anthelion_last_move_target = unit->move_target.pt;

						if (unit_ext->timers[TimerAnthelionEndlessEclipse] == 1) {
							Point16 old_pos = unit->position;
							bool teleport = bw::teleport_to_point(unit, unit->move_target.pt, 0, true);
							if (teleport) {
								bw::create_and_init_thingy(SpriteId::AnthelionTeleportDepart, old_pos.x, old_pos.y, unit->player, 11);
								helper::play_sound_from_unit(bw::rand_between(SoundId::AnthelionArrive01, SoundId::AnthelionArrive03), unit, 1, 0);
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
				bw::UnitFinder(
					unit->get_x() - radius,
					unit->get_y() - radius,
					unit->get_x() + radius,
					unit->get_y() + radius
				).for_each_in_match(match, [&](Unit* target) {
					if ((!bw::is_allied_to(unit->player, target->player))) {
						UnitExt* target_ext = get_unit_ext(target);
						if (!target_ext->timers[TimerObservanceDebuff]) {
							unit_ext->timers[TimerObservanceOverlay] = 2;
							target_ext->timers[TimerObservanceDebuff] = 5 * 24 + 1;
							target_ext->timers[TimerObservancePeriod] = 5 * 24 + 1;
							if (!target_ext->values[ValueObservanceRend]) {
								bw::add_unit_overlay(target, ImageId::ObservanceOverlaySmall - bw::get_overlay_offset(target->id));
							}
							target_ext->values[ValueObservanceRend] += 1;
						}
					}
				});
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
				if (unit->order_signal & 32) {
					unit->order_signal &= ~32;
					bw::decloak_passive_proc(unit);
				}
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
			case UnitId::TerranAnvil:
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
			case UnitId::TerranAncile: {
				// Ancile - Providence - Projectile handling
				if (unit->energy >= 1 * 256) {
					if (!unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) { // TODO: Own overlays
						bw::add_unit_overlay(unit, ImageId::SublimeShepherdOverlayLarge);
					}

					const u32 radius = 2 * 32;
					EntryArray bullets = {};
					bullet_search_radial(&bullets, unit->position, radius);
					for (u32 i = 0; i < bullets.count; i++) {
						Bullet* bullet = bullets.bullets[i];

						// Check if the weapon should be interacted with
						if (!weapon_id_procs_onhits(bullet->weapon_id)) {
							continue;
						}

						// Check if the bullet is friendly
						if (bw::are_mutual_allies(unit->player, bullet->player)
						&&  weapons_dat::explosion_type[bullet->weapon_id] != WeaponEffect::SplashRadial
						&&  weapons_dat::explosion_type[bullet->weapon_id] != WeaponEffect::SplashAir)
						{
							continue;
						}

						// Check if the unit can be shielded
						Unit* target = bullet->attack_target.unit;
						if ((target && (!bw::is_allied_to(unit->player, target->player) || target->player == NeutralPlayer)) || bullet->hit_flags & 1) {
							// We skip flagged bullets, so we don't have an infinite cycle of redirecting
							continue;
						}

						// Check if the target is actually shielded (in area)
						Point16 target_position = bullet->attack_target.unit ? bullet->attack_target.unit->position : bullet->attack_target.pt;
						if (bw::get_distance_fast(unit->position, target_position) > radius) {
							continue;
						}

						// Redirect the hitscan bullet, hitscan attacks themselves are handled in weapon damage
						switch (weapons_dat::behavior[bullet->weapon_id]) {
							case WeaponBehavior_AppearOnAttacker:
							case WeaponBehavior_AppearOnTargetSite:
							case WeaponBehavior_AppearOnTargetUnit:
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
						ignore_alert = true;
						hooks::unit_attack_notify(unit);
						ignore_alert = false;
						if (bullet->source_unit) {
							bw::reveal_attacker(bullet->source_unit, unit);
						}

						if (target) {
							bw::create_shield_overlay(target, bullet->current_direction_1);
						}
						bw::set_bullet_target(bullet, unit);
						bw::set_bullet_direction(bullet, unit);
						bullet->hit_flags |= 1; // We set missed flag, so the bullet doesn't deal hp damage (this doesn't help with hitscan or melee)
						if (weapon_id_is_standard_weapon(bullet->weapon_id)) {
							unit->spend_unit_energy(helper::get_unit_bullet_damage(bullet, unit));
						}
						helper::play_sound_from_unit(bw::rand_between(SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03), unit, 1, 0);

						if (unit->energy < 1 * 256) {
							break;
						}
					}
				} else if (unit->get_overlay(ImageId::SublimeShepherdOverlayLarge)) { // TODO: Own overlays
					bw::remove_unit_overlay(unit, ImageId::SublimeShepherdOverlayLarge);
					helper::play_sound_from_unit(SoundId::AncileProvidenceDeactivate, unit, 1, 0);
				}
			} break;
			case UnitId::TerranDurendal:
			{
				// Durendal - Forward March
				if (!unit_ext->timers[TimerSecondaryWeaponCooldown] && bw::unit_can_fire_secondary_weapon(unit)) {
					FireMultipleOptions options = {};
					options.apply_cooldown = false;
					if (fire_multiple(unit, WeaponId::DurendalPhilistineAutogun, 1, FilterEnemy, options)) {
						unit->sprite->create_top_overlay(ImageId::DurendalWeaponfireOverlay);
						helper::play_sound_from_unit(bw::rand_between(SoundId::DurendalSecondaryFire01, SoundId::DurendalSecondaryFire03), unit, 1, 0);
						unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::DurendalPhilistineAutogun);
					} else {
						unit_ext->timers[TimerSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::TerranHypnagogue:
			{
				// Hypnagogue - Through the Looking Glass
				bool try_cast = unit_ext->values[ValueTryCastCustomSpell] != 0;
				if (try_cast && (unit->main_order_id != OrderId::CastThroughTheLookingGlass || unit->is_frozen())) {
					unit_ext->values[ValueTryCastCustomSpell] = 0;
				}
				if (try_cast && !unit_ext->timers[TimerTryCastCustomSpell] && !unit_ext->timers[TimerHypnagogueChannel]) {
					Point16 target_pos = {};
					if (unit->order_target.unit) {
						target_pos.x = unit->order_target.unit->get_x();
						target_pos.y = unit->order_target.unit->get_y();
					} else if (unit->order_target.pt.x && unit->order_target.pt.y) {
						target_pos = unit->order_target.pt;
					}
					u32 range = ThroughTheLookingGlassRange;
					if (bw::get_distance_fast(unit->position, target_pos) > range) {
						if (unit->order_target.unit) {
							helper::spell_order_move_towards_target(unit, range);
						} else {
							helper::set_move_target_xy(unit, target_pos.x, target_pos.y);
						}
						unit_ext->timers[TimerTryCastCustomSpell] = 3;
					} else {
						u16 energy_cost = 0;
						if (helper::spell_order(unit, TechId::HypnagogueThroughTheLookingGlass, &energy_cost, range, 1330)) {
							unit->spend_unit_energy(energy_cost);
							unit_ext->timers[TimerHypnagogueChannel] = 48 + 1;
							unit_ext->values[ValueThroughTheLookingGlassX] = target_pos.x;
							unit_ext->values[ValueThroughTheLookingGlassY] = target_pos.y;
							unit->freeze();
							unit_ext->values[ValueTryCastCustomSpell] = 0;
						}
					}
				}

				if (unit_ext->timers[TimerHypnagogueChannel]) {
					bw::decloak_passive_proc(unit);
					switch (unit_ext->timers[TimerHypnagogueChannel])
					{
						case 48:
						case 36:
						case 24:
						case 12:
						{
							Point16 target_pos = {};
							target_pos.x = unit_ext->values[ValueThroughTheLookingGlassX];
							target_pos.y = unit_ext->values[ValueThroughTheLookingGlassY];
							Point16 pos = unit->position;
							helper::create_thingy(SpriteId::ThroughTheLookingGlassCast, pos.x,		  pos.y, unit->player);
							helper::create_thingy(SpriteId::ThroughTheLookingGlassCast, target_pos.x, target_pos.y, unit->player);
						} break;
						case 1:
						{
							Point16 target_pos = {};
							target_pos.x = unit_ext->values[ValueThroughTheLookingGlassX];
							target_pos.y = unit_ext->values[ValueThroughTheLookingGlassY];
							Point16 pos = unit->position;

							EntryArray units = {};
							unit_search_radial(&units, pos, 2 * 32);
							unit_search_filter(&units, unit, FilterAlly | FilterNoBuilding | FilterCompleted);
							for (u32 index = 0; index < units.count; ++index) {
								Unit* target = units.units[index];
								helper::create_thingy(SpriteId::LegionnaireTeleportEffect, target->position.x, target->position.y, unit->player);
								bw::teleport_to_point(target, target_pos);
								bw::add_unit_overlay(target, ImageId::LegionnaireTeleportEffect);
							}
							helper::create_thingy(SpriteId::LegionnaireTeleportEffect, unit->position.x, unit->position.y, unit->player);
							bw::teleport_to_point(unit, target_pos);
							bw::add_unit_overlay(unit, ImageId::LegionnaireTeleportEffect);
							unit->unfreeze();
						} break;
					}
				}
			} break;
			case UnitId::ZergOkrimnilisk:
			{
				// Endless Limbs
				if (!(unit->status & UnitStatus::InTransport)) {
					bool is_moving = unit->movement_flags & MovementFlags_Accelerating || unit->movement_flags & MovementFlags_Moving;
					bool can_fire = !unit->ground_weapon_cooldown && !(unit->status & UnitStatus::Burrowed);
					u32 weapon_id = WeaponId::OkrimniliskRelentlessTendrils;
					if (is_moving && can_fire) {
						FireMultipleOptions options = {};
						options.ignore_main = false;
						options.use_weapon_factor = true;
						if (fire_multiple(unit, weapon_id, 1, FilterEnemy | FilterNoAir, options)) {
							helper::play_sound_from_unit(bw::rand_between(SoundId::EvigriliskFire01, SoundId::EvigriliskFire03), unit, 1, 0);
						}
					}
					if (!unit_ext->timers[TimerSecondaryWeaponCooldown]) {
						FireMultipleOptions options = {};
						options.use_weapon_factor = false;
						options.apply_cooldown = false;
						if (fire_multiple(unit, WeaponId::OkrimniliskOmnispores, 0, FilterEnemy, options)) {
							unit_ext->timers[TimerSecondaryWeaponCooldown] = hooks::get_modified_weapon_cooldown(unit, WeaponId::OkrimniliskOmnispores);
							unit->sprite->create_top_overlay(ImageId::OkrimniliskOmnisporesFire);
							helper::play_sound_from_unit(bw::rand_between(SoundId::SpirtithWeaponfire01, SoundId::SpirtithWeaponfire03), unit, 1, 0);
						} else {
							unit_ext->timers[TimerSecondaryWeaponCooldown] = 3;
						}
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
						UnitExt* target_ext = get_unit_ext(target);
						if (target_ext->shield_regen_timer > 2 * 24 || target->id == UnitId::ProtossSycophant) {
							continue;
						}

						if (target->shields < unit->shields && target->shields != target->get_max_shields_raw()) {
							shields -= target->heal_shields(shields, unit, false, false);
							unit->shields -= shields;
							unit->set_hitpoints(unit->hitpoints);
							bw::update_on_damage_taken(unit, 0);
							break;
						}
					}
				}
			} break;
			case UnitId::ZergMortothrokor:
			{
				if (unit->status & UnitStatus::Burrowed || unit_ext->timers[TimerMortothrokorPassive] == 0) {
					unit_ext->values[ValueMortothrokorStacks] = 0;
					if (unit->get_overlay(ImageId::MortothrokorPassiveOverlay)) {
						unit->remove_overlay(ImageId::MortothrokorPassiveOverlay);
					}
				} else if (unit_ext->timers[TimerMortothrokorPassive] > 0) {
					if (!(unit->status & UnitStatus::Burrowed) && !unit->get_overlay(ImageId::MortothrokorPassiveOverlay)) {
						unit->sprite->create_top_overlay(ImageId::MortothrokorPassiveOverlay);
					}
				}
				should_update_speed = true;
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
				if (unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count < 12
					&& unit->build_queue[unit->build_queue_slot] != UnitId::ProtossSteward)
				{
					unit->build_queue[unit->build_queue_slot] = UnitId::ProtossSteward;
					unit->set_secondary_order(OrderId::TrainFighter);
					unit->secondary_order_state = 0;
				}
			} break;
			case UnitId::TerranAnchor:
			{
				if (unit->status & UnitStatus::InAir) {
					for (u32 i = 0; i < TransportCap; ++i) {
						Unit* cargo_unit = unit->get_loaded_unit(i);
						if (!cargo_unit) {
							continue;
						}
						if (cargo_unit->main_order_id != OrderId::BunkerGuard) {
							helper::order(1, cargo_unit, OrderId::BunkerGuard, cargo_unit->position, 0, UnitId::None);
						}
					}
				}
			} break;
			case UnitId::TerranDiadem:
			{
				// Fire Hammerfall Beacon for weapon targeting
				if (unit->order_signal & 0x20) {
					unit->order_signal &= ~0x20;
					unit->order_target.pt.x = unit_ext->values[ValueDiademTargetPosX];
					unit->order_target.pt.y = unit_ext->values[ValueDiademTargetPosY];
					helper::create_bullet(WeaponId::DiademHammerfallBeacon, unit, unit->get_x(), unit->get_y(), unit->player, unit->current_direction_1);
				}
			} break;
			case UnitId::TerranSavant:
			{
				if (unit_ext->timers[TimerSavantPowerSiphonBuff] == 1) {
					unit_ext->values[ValueSavantPowerSiphonBuff] = 0;
				}
				if (unit_ext->timers[TimerSavantFormSwitchCooldown] == 1) {
					bw::remove_unit_overlay(unit, ImageId::SavantFormSwitchPower);
					bw::remove_unit_overlay(unit, ImageId::SavantFormSwitchEntropy);
				}
			} break;
			case UnitId::ZergGhitorokor:
			{
				// Clear Relentless Aggression
				if (unit_ext->timers[TimerGhitorokorRelentlessAggression] == 1) {
					unit_ext->values[ValueGhitorokorRelentlessAggressionStacks] = 0;
				}
			} break;
			case UnitId::ZergAxitriliskWeaponless:
			{
				// Perpetual Wind
				if (unit_ext->timers[TimerAxitriliskPerpetualWind] == 1) {
					unit->sprite->create_top_overlay(ImageId::ZergAirExplosionSmall_Reversed);
					helper::play_sound_from_unit(SoundId::Zerg_Hydra_ZHyDth00_WAV, unit, 1, 0);
					helper::replace_unit_with_id(unit, UnitId::ZergAxitrilisk);
				}
			} break;
			case UnitId::TerranVulture:
			{
				if (unit->mine_carrier.mine_count > 2) {
					unit->mine_carrier.mine_count = 2;
				}
			} break;
			case UnitId::ZergSkithrokor:
			{
				if (!unit->is_dead() && unit_ext->timers[TimerMissileReturnETA] == 1) {
					bw::launch_return_missile(unit, WeaponId::SkithrokorSkitteringSpores);
				}
			} break;
			case UnitId::ProtossArtisan:
			{
				switch (unit_ext->timers[TimerArtisanReturn])
				{
					case 48:
					case 36:
					case 24:
					case 12:
					{
						unit->sprite->create_top_overlay(ImageId::ArtisanResourceChannel);
					} break;
					case 1:
					{
						unit->unfreeze();
						unit->sprite->create_top_overlay(ImageId::ArtisanResourceProc);
						helper::play_sound_from_unit(SoundId::ArtisanRadiantProc, unit, 1, 0);
						helper::return_resources_from_unit(unit);
						Unit* target_resource = unit->worker.target_resource.unit;
						u32 order_id = unit->resource_type == ResourceType::Minerals ? OrderId::MoveToMinerals : OrderId::HarvestGas1;
						if (target_resource) {
							helper::prepend_order_targeting_unit(unit, target_resource, order_id);
						} else {
							helper::order_immediate(unit, order_id);
						}
						helper::to_idle(unit);
					} break;
				}
			} break;
			case UnitId::ZergVilgorokor:
			{
				if (unit_ext->timers[TimerReconstitutionDelay] == 1) {
					hooks::carapace_swarm_hit_internal(unit->position.x, unit->position.y, unit->player, UnitId::Spell_CarapaceSwarmSmall);
				}
			} break;
			case UnitId::ProtossCantavis:
			{
				if (unit->order_signal & 64) {
					unit->order_signal &= ~64;
					bw::decloak_passive_proc(unit);
				}
				if (unit_ext->values[ValueJustCastedHallucination] && unit->order_signal & 32) {
					unit_ext->values[ValueJustCastedHallucination] = 0;
				}
			} break;
			case UnitId::ProtossCrucible:
			{
				// Crucible - Khaydarin Charge stacks
				if (unit->status & UnitStatus::Completed) {
					EntryArray units = {};
					unit_search_radial(&units, unit->position, 8 * 32);
					unit_search_filter(&units, unit, FilterAlly | FilterBuilding);
					for (u32 i = 0; i < units.count; i++) {
						Unit* target = units.units[i];
						UnitExt* target_ext = get_unit_ext(target);
						if (unit_id_is_tech(target->id)) {
							if (units_dat::group_flags[target->id].is_terran && !target_ext->values[ValueInfested]) {
								continue;
							}
						}
						switch (target->id) {
							case UnitId::TerranSupplyDepot:
							case UnitId::TerranAnchor:
							case UnitId::TerranReservoir: { // @Veeq7: What are these edge cases? -Pr0nogo
								if (!target_ext->values[ValueInfested]) {
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
							target->update_speed();
						}
						if (!target->get_overlay(ImageId::CrucibleKhaydarinChargeOverlay)) {
							bw::add_unit_overlay(target, ImageId::CrucibleKhaydarinChargeOverlay, true);
						}
					}
				}
			} break;
			case UnitId::ProtossBarghest:
			{
				if (unit_ext->timers[TimerDecloak] >= 2) {
					if (unit->status & UnitStatus::Completed) {
						EntryArray units = {};
						unit_search_radial(&units, unit->position, 5 * 32);
						unit_search_filter(&units, unit, FilterAlly);
						for (u32 i = 0; i < units.count; i++) {
							Unit* target = units.units[i];
							UnitExt* target_ext = get_unit_ext(target);
							if (!(target->status & UnitStatus::Completed) || !target->has_weapon()) {
								continue;
							}
							auto& vec = target_ext->unit_vectors[UnitVectorBarghests];
							if (std::find(vec.begin(), vec.end(), unit) == vec.end()) {
								vec.push_back(unit);
							}
						}
					}
				}
			} break;
			case UnitId::ProtossDidact:
			{
				if (unit->order_signal & 32) {
					unit->order_signal &= ~32;
					bw::decloak_passive_proc(unit);
				}
			} break;
			case UnitId::TerranMagnetar:
			{
				if (unit->order_signal & 32) {
					unit->order_signal &= ~32;
					bw::decloak_passive_proc(unit);
				}
			} break;
			case UnitId::ZergExcisant:
			{
				// Bandage fix for Ezigrisant regression. Not sure why this field gets maxxed out. -Pr0nogo
				if (unit->building.resource.is_mining == 255) {
					unit->building.resource.is_mining = 0;
				}
			} break;
			case UnitId::ProtossGladius:
			{
				if (unit_ext->timers[TimerBindingPlasmaChargeOverlay]) {
					if (!unit->get_overlay(ImageId::GladiusCharge)) {
						bw::add_unit_overlay(unit, ImageId::GladiusCharge);
					}
				} else {
					Image* overlay = unit->get_overlay(ImageId::GladiusCharge);
					if (overlay) {
						overlay->play_iscript_animation(IscriptAnimation::Death);
					}
				}
			} break;
		}

		// Stimpack - Delayed buff handling
		if (unit_ext->timers[TimerStimpackDelay] == 1) {
			helper::play_sound_from_unit(bw::rand_between(SoundId::Terran_MARINE_TMaSti00_WAV, SoundId::Terran_MARINE_TMaSti01_WAV), unit, 1, 0);
			unit->stim_timer = StimpackTimer;
			unit_ext->values[ValueHasStimpack] = 1;
			unit->sprite->create_top_overlay(ImageId::MaverickStimpackProc);
			should_update_speed = true;
		}

		if (unit_ext->values[ValueHasStimpack] && !unit->stim_timer) {
			unit_ext->values[ValueHasStimpack] = 0;
			helper::play_sound_from_unit(SoundId::StimpackExpire01, unit, 1, 0);
			unit->sprite->create_top_overlay(ImageId::MaverickStimpackExpire);
		}

		// Protoss - Binding Plasma
		if (unit_ext->timers[TimerBindingPlasma]) {
			u32 source_player = unit_ext->values[ValueGladiusBindingPlasmaPlayer];

			bool has_source = false;
			EntryArray sources = {};
			unit_search_radial(&sources, unit->position, 8 * 32);
			unit_search_filter(&sources, unit, 0, {UnitId::ProtossGladius});
			for (u32 i = 0; i < sources.count; ++i) {
				Unit* source = sources.units[i];
				if (bw::is_allied_to(source->player, source_player)) {
					has_source = true;
					UnitExt* source_ext = get_unit_ext(source);
					source_ext->timers[TimerBindingPlasmaChargeOverlay] = 3;
				}
			}

			if (has_source) {
				if (unit_ext->timers[TimerBindingPlasma] % BindingPlasmaTimer == BindingPlasmaTimer - 1) {
					bw::add_unit_overlay(unit, ImageId::BindingPlasmaOverlay); // TODO: Audio
				}
				if (unit_ext->timers[TimerBindingPlasma] == 1) {
					if (bw::binding_plasma_proc(unit)) {
						helper::play_sound_from_unit(bw::rand_between(SoundId::GladiusBindingPlasmaProc01, SoundId::GladiusBindingPlasmaProc03), unit, 1, 0);
					}
				}
			} else {
				unit_ext->timers[TimerBindingPlasma] = 0;
				unit_ext->values[ValueGladiusBindingPlasmaStacks] = 0;
			}
		} else {
			unit_ext->values[ValueGladiusBindingPlasmaStacks] = 0;
		}

		if (unit_ext->timers[TimerAvenge] > 1) {
			u32 ascend_overlay_id = ImageId::Avenge_Small + bw::get_overlay_offset(unit->id, true);
			if (!unit->get_overlay(ascend_overlay_id)) {
				bw::add_unit_overlay(unit, ascend_overlay_id);
			}
		}

		if (unit_ext->timers[TimerAscend]) {
			u32 ascend_overlay_id = ImageId::Ascension_Small + bw::get_overlay_offset(unit->id, true);
			if (!unit->get_overlay(ascend_overlay_id)) {
				bw::add_unit_overlay(unit, ascend_overlay_id);
			}
		}

		{
			u32 shield_bleed_overlay = ImageId::AstralRule_Small + bw::get_overlay_offset(unit->id, true);
			if (unit_ext->timers[TimerTriumphantRiseDebuff] || unit_ext->timers[TimerPatriarchAstralRule] > 1) {
				if (!unit->get_overlay(shield_bleed_overlay)) {
					bw::add_unit_overlay(unit, shield_bleed_overlay);
				}
			} else {
				Image* overlay = unit->get_overlay(shield_bleed_overlay);
				if (overlay) {
					overlay->play_iscript_animation(IscriptAnimation::Death);
				}
			}
		}

		// Tyranny and Worship
		if (unit_ext->values[ValueMindControlPlayer] && !unit_ext->values[ValueControlledByDemiurge]) {
			EntryArray masters = {};
			unit_search_radial(&masters, unit->position, 8 * 32);
			unit_search_filter(&masters, unit, 0, {UnitId::ProtossMindTyrant});
			unit_search_custom_filter(&masters, [=](Unit* master) {
				if (bw::is_allied_to(master->player, unit_ext->values[ValueMindControlPlayer] - 1)) {
					return true;
				}
				return false;
			});

			if (masters.count) {
				if (unit_ext->timers[TimerMindControlRecede] <= 4 * 24) {
					unit->unfreeze();
					if (unit->subunit) {
						unit->subunit->unfreeze();
					}
					u32 offset = bw::get_overlay_offset(unit->id, true);
					bw::remove_unit_overlay(unit, ImageId::TyrannyStunSmall + offset);
				}
				unit_ext->timers[TimerMindControlRecede] = 5 * 24;
			}
			if (unit_ext->timers[TimerMindControlRecede] <= 4 * 24) {
				unit->freeze();
				u32 offset = bw::get_overlay_offset(unit->id, true);
				if (!unit->get_overlay(ImageId::TyrannyStunSmall + offset)) {
					bw::add_unit_overlay(unit, ImageId::TyrannyStunSmall + offset);
				}
			}
			if (!unit_ext->timers[TimerMindControlRecede]) {
				unit->unfreeze();
				u32 offset = bw::get_overlay_offset(unit->id, true);
				bw::remove_unit_overlay(unit, ImageId::TyrannyStunSmall + offset);
				recede_mind_control(unit);
				unit_ext->values[ValueMindControlPlayer] = 0;

				u32 worship_overlay_id = ImageId::DemiurgeWorshipOverlay_Small + offset;
				if (unit->get_overlay(worship_overlay_id)) {
					bw::remove_unit_overlay(unit, worship_overlay_id);
				}
			}
		}

		{
			EntryArray masters = {};
			unit_search_radial(&masters, unit->position, 6 * 32);
			unit_search_filter(&masters, unit, 0, { UnitId::ProtossDemiurge });
			unit_search_custom_filter(&masters, [=](Unit* master) {
				if (!bw::is_allied_to(unit->player, master->player)) {
					return true;
				}
				return false;
			});

			if (masters.count) {
				if (!(units_dat::base_property[unit->id] & UnitProperty::Building) && !unit_ext->timers[TimerDemiurgeWorshipOverlayDelay]) {
					unit_ext->timers[TimerDemiurgeWorshipOverlayDelay] = 1;
				}
			}

			if (unit_ext->timers[TimerDemiurgeWorship]) {
				if (unit_ext->timers[TimerDemiurgeWorship] == 1) {
					if (!masters.count) {
						unit->remove();
					} else {
						mind_control(masters.units[0]->player, UnitId::ProtossDemiurge, unit);
						unit_ext->timers[TimerDemiurgeWorshipOverlayDelay] = 0;
						unit->unfreeze();
						unit->set_hitpoints(100 * 256 + units_dat::max_hitpoints[unit->id] / 3);
						unit->status &= ~UnitStatus::Invincible; // Could cause collisions with stasis, should check.
					}
				}
			}
		}

		if (unit_ext->timers[TimerDemiurgeWorshipOverlayDelay] == 1) {
			u32 overlay_id = ImageId::DemiurgeWorshipOverlay_Small + bw::get_overlay_offset(unit->id, true);
			if (overlay_id == ImageId::ControlledDemolition) {
				ImageId::DemiurgeWorshipOverlay_XLarge; // Stupid workaround because XLarge was added out of sequence. -Pr0nogo
			}
			bw::add_unit_overlay(unit, overlay_id);
			unit_ext->timers[TimerDemiurgeWorshipOverlayDelay] = 49;
		}

		// Fix animation bug with acceleration movement flag
		if (!(units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding)
		&&    unit->movement_flags & MovementFlags_Accelerating
		&&    unit->sprite->main_graphic->animation != IscriptAnimation::Walking
		&&    unit->sprite->main_graphic->animation != IscriptAnimation::WalkingToIdle)
		{
			unit->movement_flags &= ~MovementFlags_Accelerating;
		}

		// Fix AI melee units freezing - DarkenedFantasies
		if (unit->status & UnitStatus::Unmovable
		&&  unit->main_order_id == OrderId::AttackUnit
		&& !unit->movement_flags
		&&  unit->movement_state == UnitMovement::UnitMovement_Dormant)
		{
			++g_unmovable_hits[6];
			g_unmovable_lastframe[6] = g_elapsed_time_frames;
			unit->status ^= UnitStatus::Unmovable;
		}

		// Temp fix for iscript-based workers getting stuck - DarkenedFantasies
		if ((unit->id == UnitId::TerranMason || unit->id == UnitId::ProtossScribe || unit->id == UnitId::ZergDroleth)
		&&	unit->current_speed_1 == 0
		&&	unit->current_speed_2 == 0
		&&	unit->movement_flags & MovementFlags_Accelerating)
		{
			unit->movement_flags &= ~MovementFlags_Accelerating;
		}

		// Zoryusthaleth - Swarming Omen marking
		{
			u32 overlay_id = ImageId::SwarmingOmenSmall + bw::get_overlay_offset(unit->id, true);
			if (unit_ext->timers[TimerSwarmingOmenDebuff] > 1) {
				if (!unit->get_overlay(overlay_id)) {
					bw::add_unit_overlay(unit, overlay_id);
				}

				EntryArray targets = {};
				unit_search_radial(&targets, unit->position, 10 * 32);
				for (u32 i = 0; i < targets.count; ++i) {
					Unit* target = targets.units[i];
					if (bw::is_allied_to(unit->player, target->player)) {
						continue;
					}
					UnitExt* target_ext = get_unit_ext(target);
					target_ext->timers[TimerSwarmingOmenSearch] = 24 + 1;
				}
			} else if (unit_ext->timers[TimerSwarmingOmenDebuff == 1]) {
				bw::remove_unit_overlay(unit, overlay_id);
			}
		}

		if (hooks::unit_can_have_rally(unit) && unit->rally.unit && !helper::is_target_visible(unit, unit->rally.unit)) {
			unit->rally.unit = 0;
			unit->rally.pt.x = 0;
			unit->rally.pt.y = 0;
		}

		// Azazel - Sublime Shepherd
		if (!unit_id_is_resource_node(unit->id, false)) {
			EntryArray azazels = {};
			unit_search_radial(&azazels, unit->position, 4 * 32);
			unit_search_filter(&azazels, unit, FilterAlly, {UnitId::TerranAzazel});
			if (!azazels.count) {
				if (unit_ext->values[ValueHasSublimeShepherd]) {
					unit_ext->values[ValueHasSublimeShepherd] = 0;
					if (unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - bw::get_overlay_offset(unit->id))) {
						unit->sprite->remove_overlay(ImageId::SublimeShepherdOverlaySmall - bw::get_overlay_offset(unit->id));
					}
				}
			} else {
				unit_ext->values[ValueHasSublimeShepherd] = 1;
				if (!unit->get_overlay(ImageId::SublimeShepherdOverlaySmall - bw::get_overlay_offset(unit->id))) {
					unit->sprite->create_top_overlay(ImageId::SublimeShepherdOverlaySmall - bw::get_overlay_offset(unit->id));
				}
			}
		}

		// Fix target clearing bug - Doesn't work yet...
		/*
		if (unit->order_target.unit
			&& !(unit->order_target.unit->detection_status & (1 << unit->player))) {
			bw::print("lard");
			unit->order_to_idle();
		}
		*/

		// Play harvesting animations when a worker harvests from a resource
		// Requires more involved anim/graphics work
		/*
		if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
			Unit* harvest_target = unit->worker.harvest_target;
			// Eventually make it happen for vespene geysers and mineral fields, not just refineries
			if (harvest_target && bw::unit_id_is_gas_structure(harvest_target->id, true) && harvest_target->sprite->main_graphic->animation != IscriptAnimation::IsWorking) {
				harvest_target->sprite->play_iscript_animation(IscriptAnimation::IsWorking);
			}
		}
		*/

		// Periodic overlays
		if (g_elapsed_time_frames % 10 == 0) {
			if (unit_ext->unit_vectors[UnitVectorBarghests].size()) {
				bw::add_unit_overlay(unit, ImageId::RepentantChorusSmall + bw::get_overlay_offset(unit->id));
			}
			if (unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].size()) {
				bw::add_unit_overlay(unit, ImageId::PhaseLinkSmall + bw::get_overlay_offset(unit->id));
			}
		}
		

		// Incubation
		if (unit_ext->timers[TimerIncubationArm] == 1) {
			for (u32 i = 0; i < unit_ext->values[ValueIncubationCount]; ++i) {
				bw::create_unit_from_ability(unit_ext->values[ValueIncubationUnitId], 1, unit_ext->values[ValueIncubationPlayer], unit->position);
			}
		}

		// Disruption Field - clear status
		if (unit_ext->timers[TimerUnderDisruptionField] == 1) {
			unit->status &= ~UnitStatus::CanNotAttack;
			if (unit->subunit) {
				unit->subunit->status &= ~UnitStatus::CanNotAttack;
			}
			if (unit_id_is_garrison(unit->id)) {
				for (s32 i = 0; i < TransportCap; i++)
				{
					Unit* cargo_unit = unit->get_loaded_unit(i);
					if (!cargo_unit) continue;
					if (cargo_unit) {
						cargo_unit->status &= ~UnitStatus::CanNotAttack;
						if (cargo_unit->subunit) {
							cargo_unit->subunit->status &= ~UnitStatus::CanNotAttack;
						}
					}
				}
			}
		}

		// Hierophant Signify Reset
		if (unit_ext->timers[TimerSignify] == 1) {
			unit_ext->values[ValueSignifyStacks] = 0;
			unit_ext->values[ValueSignifyPlayer] = 0;
			bw::update_signify_status(unit, unit_ext);
			should_update_speed = true;
		}

		// Decloak on stun
		if (unit->is_frozen() || unit_ext->timers[TimerStrigaRepulsion] || unit_ext->timers[TimerAnthelionGravityWell]) {
			bw::decloak_passive_proc(unit);
		}

		// Play addon work animations when related units are being trained by parent structure
		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->building.addon && unit->building.addon->status & UnitStatus::Completed)
		{
			// Wait for addon connector extension sequence to finish before animating
			// Have to find a working method... -Pr0nogo
		/*
			Image* connector = bw::get_addon_connector_image(unit->building.addon);
			bool connector_is_extended = (connector && connector->animation != IscriptAnimation::Landing);
			if (!connector) // For addons without connectors, don't wait for anything
			{
				connector_is_extended = true;
			}
		*/
			bool connector_is_extended = true;
			u16 unit_id = unit->build_queue[unit->build_queue_slot];
			if (unit_id != UnitId::None && !bw::unit_id_is_available_without_an_addon(unit_id) && connector_is_extended) {
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

				s32 x_offset = bw::random() % width - width / 2;
				s32 y_offset = bw::random() % height - height / 2;

				x_offset = MathUtils::clamp(x_offset, -127, 128);
				y_offset = MathUtils::clamp(y_offset, -127, 128);

				unit->sprite->create_overlay(ImageId::AddonWeldOverlay, x_offset, y_offset);
				helper::play_sound_from_unit(bw::rand_between(SoundId::AddonConstruction01, SoundId::AddonConstruction04), unit, 1, 0);
				unit_ext->timers[TimerAddonAnimation] = bw::rand_between(16, 20);
			}
		}

		// Correct vanilla timers -Pr0nogo
		if (unit->defensive_matrix_timer > AzazelDefensiveMatrixTimer) {
			unit->defensive_matrix_timer = AzazelDefensiveMatrixTimer;
		}
		if (unit->stasis_timer > DidactStasisFieldTimer) {
			unit->stasis_timer = DidactStasisFieldTimer;
		}
		if (unit->plague_timer > IsthrathalethPlagueTimer) {
			unit->plague_timer = IsthrathalethPlagueTimer;
		}
		if (unit->defensive_matrix_hp > 100 * 256) {
			unit->defensive_matrix_hp = 100 * 256;
		}

		// Striga - Fleeting Focus
		if (unit_ext->timers[TimerStrigaRepulsion]) {
			Point16 final_pos = unit->position;
			final_pos.x += unit_ext->repulsion_vector.x;
			final_pos.y += unit_ext->repulsion_vector.y;
			final_pos = bw::clamp_point(final_pos);

			if (!bw::move_unit_improved(unit, final_pos.x, final_pos.y)) {
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
			final_pos = bw::clamp_point(final_pos);

			if (!bw::move_unit_improved(unit, final_pos.x, final_pos.y)) {
				unit_ext->timers[TimerMagnetarAttraction] = 0;
			}
		}

		// Anthelion - Gravity Well
		if (unit_ext->timers[TimerAnthelionGravityWell] && unit_ext->units[UnitAnthelionGravityWellSource]) {
			Point16 target_pos = unit_ext->units[UnitAnthelionGravityWellSource]->position;
			if (unit->position != target_pos && !(unit->status & UnitStatus::Burrowed)) {
				Vector32 diff = bw::get_vector_between(unit->position, target_pos);
				if (bw::get_vector_distance(diff) < 2) {
					bw::move_unit_improved(unit, target_pos.x, target_pos.y);
				} else {
					Vector32f displacement = bw::get_normalized_vector(diff);
					displacement.x *= 2;
					displacement.y *= 2;

					Point16 final_pos = unit->position;
					final_pos.x += displacement.x;
					final_pos.y += displacement.y;
					final_pos = bw::clamp_point(final_pos);

					bw::move_unit_improved(unit, final_pos.x, final_pos.y);
					helper::turn_unit(unit, bw::get_angle(unit->position, target_pos));
				}
			}
		}

		// Gorgon - Petrification - clear stacks
		if (unit_ext->timers[TimerPetrification] == 1) {
			unit_ext->values[ValuePetrificationStacks] = 0;
			should_update_speed = true;
			if (unit->get_overlay(ImageId::GorgonPetrification)) {
				bw::remove_unit_overlay(unit, ImageId::GorgonPetrification);
			}
		} else if (unit_ext->timers[TimerPetrification] > 1) {
			if (!unit->get_overlay(ImageId::GorgonPetrification)) {
				bw::add_unit_overlay(unit, ImageId::GorgonPetrification);
			}
			should_update_speed = true;
		}

		// Custom Larva Spawn
		if (unit_spawns_custom_larva(unit)) {
			if (unit->secondary_order_id != OrderId::SpawningLarva) { // let's not duplicate the order
				hooks::secondary_order_spawning_larva(unit);
			}
		}

		// Custom Creep Spread
		if (unit_spreads_custom_creep(unit)) {
			if (g_elapsed_time_frames % 15 == 0) {
				bw::spread_custom_creep(unit);
			}
		}

		// Blissful Embrace - Init & Visuals
		if (unit_ext->values[ValueInfested] && unit->status & UnitStatus::Completed) {
			blissful_embrace_tick(unit);
			should_update_speed = true;
		}

		// Matraleth - Blissful Embrace ability
		if (unit->id == UnitId::ZergMatraleth) {
			if (unit->main_order_id == OrderId::BlissfulEmbrace) {
				Unit* target = unit->order_target.unit;
				if (target && can_infest(unit->player, target)) {
					if (helper::get_distance_to_unit(unit, target) > 16) {
						helper::spell_order_move_towards_target(unit, 16);
					}
					if (unit->sprite->main_graphic->animation == IscriptAnimation::CastSpell) {
						mark_infesting(unit->player, target);
						bw::decloak_passive_proc(unit);
					}
				} else {
					helper::force_order_done(unit);
				}
			}
		}

		// Clear Barghest vector
		{
			auto& barghests = unit_ext->unit_vectors[UnitVectorBarghests];
			for (auto it = barghests.begin(); it != barghests.end(); ++it) {
				Unit* barghest = *it;
				UnitExt* barghest_ext = get_unit_ext(barghest);
				if (barghest_ext->timers[TimerDecloak] < 2 || bw::get_distance_fast(unit->position, barghest->position) > 5 * 32) {
					barghests.erase(it);
					--it;
				}
			}
		}

		// Seraph - Observance
		if (unit_ext->timers[TimerObservanceDebuff] > 1) {
			// TODO: unit search
			const auto range = 6 * 32;
			auto seraphs = bw::UnitFinder(
				unit->get_x() - range,
				unit->get_y() - range,
				unit->get_x() + range,
				unit->get_y() + range
			).filter([&](Unit* target) {
				if (unit->player == target->player || bw::is_allied_to(unit->player, target->player) || target->id != UnitId::TerranSeraph) {
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
			bw::remove_unit_overlay(unit, ImageId::ObservanceOverlaySmall - bw::get_overlay_offset(unit->id));
			unit_ext->timers[TimerObservancePeriod] = 0;
			unit_ext->values[ValueObservanceRend] = 0;
		}

		// Vithrilisk - Corrosive Touch clear
		if (unit_ext->timers[TimerCorrosiveTouch] == 1) {
			unit_ext->values[ValueCorrosiveTouchStacks] = 0;
			unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Vorvrokor - Captivating Claws
		if (unit_ext->timers[TimerCaptivatingClaws] == 1) {
			unit_ext->unit_vectors[UnitVectorCaptivatingClaws] = {};
			should_update_speed = true;
		}

		// Terran traps - No collision
		if (unit_id_is_trap(unit->id)) {
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
		u32 halluc_timer = unit_ext->timers[TimerHallucinating];
		if (unit && unit->sprite) {
			if (halluc_timer % 24 == 1) {
				bw::add_unit_overlay(unit, ImageId::HallucinationOverlay);
			}
		}

		// Luminary - Aural Assault - overlays
		if (unit_ext->timers[TimerAuralAssaultBlind] > 1) {
			u32 overlay = ImageId::OpticalFlareHit_Small + bw::get_overlay_offset(unit->id);
			Unit* overlay_unit = unit->subunit ? unit->subunit : unit;
			if (!overlay_unit->get_overlay(overlay)) {
				overlay_unit->sprite->create_top_overlay(overlay, 0, 0, 0);
			}
		}

		// Talos - Salvage stun
		if (unit_ext->timers[TimerSalvageStunTimer] > 1) {
			unit->freeze();

			u32 offset = bw::get_overlay_offset(unit->id);
			if (!unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				bw::add_unit_overlay(unit, ImageId::SalvageDisableSmall + offset);
			}
			if (!unit_ext->timers[TimerTalosSoundLoopTimer]) {
				helper::play_sound_from_unit(SoundId::TalosSalvageLoop, unit, 1, 0);
				unit_ext->timers[TimerTalosSoundLoopTimer] = 63;
			}
		}
		if (unit_ext->timers[TimerSalvageStunTimer] == 1) {
			unit->unfreeze();

			u32 offset = bw::get_overlay_offset(unit->id);
			if (unit->get_overlay(ImageId::SalvageDisableSmall + offset)) {
				bw::remove_unit_overlay(unit, ImageId::SalvageDisableSmall + offset);
			}
		}

		// Proximity cloaking - recloak after 2 seconds of no nearby enemies
		if (bw::is_proximity_decloakable(unit)) {
			EntryArray units = {};
			u32 search_margin = 4 * 32;
			Box32 search_area = {};
			search_area.left = unit->position.x - search_margin;
			search_area.top = unit->position.y - search_margin;
			search_area.right = unit->position.x + search_margin;
			search_area.bottom = unit->position.y + search_margin;
			unit_search_rect(&units, search_area);
			unit_search_filter(&units, unit, FilterEnemy);

			if (units.count) {
				if (unit_ext->timers[TimerDecloak] < 2) {
					bw::decloak_proximity_proc(unit);
				} else {
					unit_ext->timers[TimerDecloak] = 48 + 1;
				}
			} else {
				if (unit_ext->timers[TimerDecloak] == 1) {
					helper::begin_cloak(unit, bw::get_cloak_sound_id(unit));
				}
			}
		}

		// Passive cloaking - recloak after 2 seconds of no combat
		if (bw::is_passive_decloakable(unit)) {
			if (unit_ext->timers[TimerDecloak] == 1) {
				helper::begin_cloak(unit, bw::get_cloak_sound_id(unit));
			}
		}

		// Autovitality - Simulacrum behavior
		if (unit_ext->timers[TimerSimulacrumClone] == 1) {
			bw::create_unit_from_ability(UnitId::ProtossSimulacrum, 1, unit->player, unit->position, unit);
		}

		// Remove Ensnaring Brood overlay
		if (unit_ext->timers[TimerEnsnaringBrood] == 1 || (unit_ext->timers[TimerEnsnaringBrood] > 1 && unit->status & UnitStatus::Burrowed)) {
			bw::remove_unit_overlay(unit, ImageId::EnsnaringBroodOverlay);
		}

		// Remove Lucilerri Pheremones overlay
		if (unit_ext->timers[TimerLucilleriPheremones] == 1 || (unit_ext->timers[TimerLucilleriPheremones] > 1 && unit->status & UnitStatus::Burrowed)) {
			bw::remove_unit_overlay(unit, ImageId::DermalAccelerantSmall + bw::get_overlay_offset(unit->id, true));
		}

		// Crucible stacks clear
		{
			auto& crucibles = unit_ext->unit_vectors[UnitVectorCrucibles];
			for (auto it = crucibles.begin(); it != crucibles.end(); ++it) {
				Unit* target = *it;
				if (bw::get_distance_fast(unit->position, target->position) > 8 * 32) {
					bw::remove_unit_overlay(target, ImageId::CrucibleKhaydarinChargeOverlay);
					crucibles.erase(it);
					--it;
					should_update_speed = true;
				}
			}
		}

		// Controlled Demolition - initial cast
		if (bw::get_generic_rust_value(unit, RustId::CastControlledDemolition)
		&& !unit_ext->timers[TimerEmergencyDetonation] && !unit_ext->timers[TimerControlledDemolition])
		{
			if (unit->id == UnitId::TerranLobotomyMine) {
				unit->remove();
			} else {
				bw::set_generic_rust_value(unit, RustId::CastControlledDemolition, 0);
				unit_ext->timers[TimerControlledDemolition] = ControlledDemolitionTimer;
				unit->sprite->create_overlay(ImageId::ControlledDemolition);
			}
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

		// Controlled Demolition - cancel
		if (unit_id_can_do_controlled_demolition(unit->id)) {
			if (bw::get_generic_rust_value(unit, RustId::CancelControlledDemolition)) {
				bw::set_generic_rust_value(unit, RustId::CancelControlledDemolition, 0);
				if (unit_ext->timers[TimerControlledDemolition])
				{
					unit_ext->timers[TimerControlledDemolition] = 0;
					Image* overlay = unit->get_overlay(ImageId::ControlledDemolition);
					if (overlay)
					{
						overlay->play_iscript_animation(IscriptAnimation::Death);
					}
				}
			}
		}

		// Emergency Detonation - initial cast
		if (bw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation)) {
			bw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
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
			// TODO: search
			auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
			const u32 radius = 3 * 32;
			bw::UnitFinder(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius
			).for_each_in_match(match, [&](Unit* target) {
				if (bw::get_distance_fast(unit->position, target->position) <= radius) {
					target->damage_with_custom(250 * 256, 0, unit, unit->player);
				}
			});
			helper::create_thingy(SpriteId::ArcliteShockCannon_Damageless, unit->sprite->position.x, unit->sprite->position.y, unit->player);
			unit->remove();
		}

		// Silly way of pathing workers to expansion site
		if (unit_ext->values[ValueAI_WorkerChosenForExpansion] && unit->resource_type == ResourceType::None) {
			unit_ext->values[ValueAI_WorkerChosenForExpansion] = false;
			unit->order_to(OrderId::Move, unit_ext->values[ValueAI_WorkerChosenX], unit_ext->values[ValueAI_WorkerChosenY]);
		}

		if (unit_ext->timers[TimerHidebreaker] == 1) {
			unit_ext->values[ValueHidebreaker] = 0;
			// TODO: Overlay
		}

		// Legionnaire - Blade Vortex - Handle debuff overlay
		{
			u32 blade_vortex = unit_ext->timers[TimerBladeVortex];
			u32 image_id = ImageId::BladeVortexDebuffSmall + bw::get_overlay_offset(unit->id);
			if (blade_vortex > 1 && !unit->get_overlay(image_id)) {
				bw::add_unit_overlay(unit, image_id);
			} else if (blade_vortex <= 1 && unit->get_overlay(image_id)) {
				bw::remove_unit_overlay(unit, image_id);
			}
		}

		// Magister - Stellar Enforcement - Handle overlay looping
		{
			u32 overlay_id = ImageId::StellarEnforcementDebuffSmall + bw::get_overlay_offset(unit->id, true);
			if (unit_ext->timers[TimerStellarEnforcement] > 1 && unit_ext->timers[TimerStellarEnforcementOverlay] <= 1) {
				unit_ext->timers[TimerStellarEnforcementOverlay] = 30;
				bw::add_unit_overlay(unit, overlay_id);
			}

			// Handle overlays after effect expires
			if (unit->get_overlay(overlay_id) && unit_ext->timers[TimerStellarEnforcement] <= 1) {
				bw::remove_unit_overlay(unit, overlay_id);
			}
		}

		// Clear Deconstruction on timer end
		if (unit_ext->values[ValueDeconstruction] && unit_ext->timers[TimerDeconstruction] <= 1) {
			unit_ext->values[ValueDeconstruction] = 0;
			bw::remove_unit_overlay(unit, ImageId::DeconstructionSmall + bw::get_overlay_offset(unit->id));
		}

		// Harakan - Apollyoid Adhesive
		{
			u32 image_id = ImageId::HarakanApollyoidAdhesiveOverlay;
			if (unit_ext->timers[TimerApollyoidAdhesive] >= 1) {
				if (!unit->sprite->get_overlay(image_id)) {
					bw::add_unit_overlay(unit, image_id);
				}
				if (!unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod]) {
					unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod] = 24;
				}
				if (unit_ext->timers[TimerApollyoidAdhesiveDamagePeriod] == 1) {
					unit->damage_with_custom(256);
				}

			}
			if (unit_ext->timers[TimerApollyoidAdhesive] == 1) {
				bw::remove_unit_overlay(unit, image_id);
				unit_ext->values[ValueApollyoidAdhesiveStacks] = 0;
			}
		}

		// Auriel - Incineration
		{
			u32 image_id = ImageId::HarakanApollyoidAdhesiveOverlay;
			if (unit_ext->timers[TimerAurielIncineration] >= 1) {
				if (!unit->sprite->get_overlay(image_id)) {
					bw::add_unit_overlay(unit, image_id);
				}
				if (!unit_ext->timers[TimerAurielIncinerationDamagePeriod]) {
					unit_ext->timers[TimerAurielIncinerationDamagePeriod] = 6;
				}
				if (unit_ext->timers[TimerAurielIncinerationDamagePeriod] == 1) {
					unit->damage_with_custom(256);
				}

			}
			if (unit_ext->timers[TimerAurielIncineration] == 1) {
				bw::remove_unit_overlay(unit, image_id);
			}
		}

		// Clear speed buffs for currently-moving flingy units. -Pr0nogo
		if (unit_ext->timers[TimerTerminalSurge] == 1) {
			should_update_speed = true;
		}

		if (unit_ext->timers[TimerCausticDischargeSpeed] >= 1) {
			should_update_speed = true;
		}

		// Clear Power Siphon debuff
		if (unit_ext->timers[TimerSavantPowerSiphonDebuff] == 1) {
			unit_ext->values[ValueSavantPowerSiphonDebuff] = 0;
			if (unit->subunit && unit->subunit->is_subunit()) {
				UnitExt* subunit_ext = get_unit_ext(unit->subunit);
				subunit_ext->values[ValueSavantPowerSiphonDebuff] = 0;
			}
		}

		// Clear Autocrat - Controlled Opposition
		if (unit_ext->timers[TimerAutocratControlledOpposition] == 1) {
			unit_ext->values[ValueAutocratControlledOppositionStacks] = 0;
			unit_ext->values[ValueAutocratControlledOppositionPlayer] = 0;
			if (unit->get_overlay(ImageId::AutocratControlledOpposition)) {
				bw::remove_unit_overlay(unit, ImageId::AutocratControlledOpposition);
			}
		}
		else if (unit_ext->values[ValueAutocratControlledOppositionStacks] && !unit->get_overlay(ImageId::AutocratControlledOpposition)) {
			bw::add_unit_overlay(unit, ImageId::AutocratControlledOpposition);
		}

		// Clear Empress - Anodyne Annex
		if (unit_ext->timers[TimerEmpressAnodyneAnnex] == 1) {
			unit_ext->units[UnitEmpressAnodyneAnnexSource] = 0;
			unit_ext->values[ValueEmpressAnodyneAnnexStacks] = 0;
			unit_ext->values[ValueEmpressAnodyneAnnexSourcePlayer] = 0;
		}

		// Claymore - Kelvin Munitions - Create overlays
		if ((unit_ext->timers[TimerKelvinMunitions] > 1 || unit_ext->timers[TimerLobotomySlow] > 1)) { // Piggybacking overlay from Claymore, for now. -Pr0nogo
			should_update_speed = true;
			// Overlay
			if (!unit_ext->timers[TimerClaymoreKelvinMunitionsZap]) {
				u32 width = units_dat::unit_bounds[unit->id].x * 0.75;
				u32 height = units_dat::unit_bounds[unit->id].y * 0.75;

				s32 x_offset = bw::random() % width - width / 2;
				s32 y_offset = bw::random() % height - height / 2;

				x_offset = MathUtils::clamp(x_offset, -127, 128);
				y_offset = MathUtils::clamp(y_offset, -127, 128);

				Unit* final_unit = unit;
				if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit) {
					final_unit = unit->subunit;
				}
				final_unit->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
				unit_ext->timers[TimerClaymoreKelvinMunitionsZap] = 7;
			}
		}

		if (unit_ext->timers[TimerKelvinMunitions] == 1) {
			unit_ext->timers[TimerKelvinMunitions] = 0;
			should_update_speed = true;
		}

		if (unit_ext->timers[TimerLobotomySlow] == 1) {
			unit_ext->timers[TimerLobotomySlow] = 0;
			should_update_speed = true;
		}

		if (unit_ext->timers[TimerMindTyrantMaelstromSlow] > 1) {
			should_update_speed = true;
		}

		// Remove Maelstrom overlays
		if (unit_ext->timers[TimerMindTyrantMaelstromSlow] == 1) {
			unit->sprite->remove_overlay(ImageId::MaelstromOverlay_Small, ImageId::MaelstromOverlay_Large);
			if (unit->subunit) {
				unit->subunit->sprite->remove_overlay(ImageId::MaelstromOverlay_Small, ImageId::MaelstromOverlay_Large);
			}
			unit_ext->timers[TimerMindTyrantMaelstromSlow] = 0;
			should_update_speed = true;
		}

		if (unit_ext->timers[TimerAddonDisconnectDelay] == 1) {
			for (Image* i = unit->sprite->images.head; i; i = i->link.next) {
				play_image_iscript(i, IscriptAnimation::LiftOff);
			}
		}

		// Generic speed timers
		if (unit_ext->timers[TimerAvenge] == 1 || unit_ext->timers[TimerZergAdrenalFrenzy] == 1) {
			should_update_speed = true;
		}

		if (unit_is_tower(unit) && unit->status & UnitStatus::Completed && unit->main_order_id == OrderId::Nothing2) {
			unit->main_order_id = OrderId::TowerGuard;
		}

		// Meme to make lifted buildings use their flingy speed. -Pr0nogo
		if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding && unit->status & UnitStatus::InAir && unit_ext->timers[TimerLiftLandDelay] == 1) {
			should_update_speed = false;
			unit->update_speed(); // Have to do this here because of the Diadem bandage. -Pr0nogo
			if (unit->id == UnitId::TerranDiadem) {
				unit->flingy_turn_rate = 27;
			}
		}

		if (should_update_speed) {
			unit->update_speed();
		}

		// CELEBRATIONS
		if (g_globals->celebration == Celebration_Christmas) {
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
					case UnitId::ZergLarvoskCesiant:
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
						image_id = ImageId::SantaHatSmall17 + bw::get_overlay_offset(unit->id, true);
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

	LPCSTR string_convert(LPCSTR aString) {
		return aString;
	}

	bool game_init() {
		set_max_sight_range();

		// Update harvest time for mineral fields
		memory_patch(0x004698BC + 3, (u8)HarvestTimer);

		// Fix order timer
		memory_patch(0x004EBF50 + 6, (u8)OrderQueueTimer);
	//	memory_patch(0x004EC70B + 6, (u8)OrderQueueTimer); // hooked within unused_order_patch_wrapper in cosmonarchy_specific_inject.cpp 

		g_chk_cache.clear();

		quickcasting_signal = false;
		quickcasting_command_card = 0;

		weapon_damage_current_bullet = 0;

		g_graphics.reset_all_graphics();
		g_graphics.reset_stat_res_graphics();

		buttonset_context.reset();

		init_threshold();
		if (!grp_wire_Font) {
			init_grp_wire_font("font_wire");
		}

		memset(vision_timers, 0, 8*256*256);

		init_ext_variables();
		init_omnipresence_state();

		gptp::g_logger << "Started a match!\n";
		g_game_was_init = false;
		hotkeys.game_started = true;

		return true;
	}

	bool game_end() {
		end_threshold();
		g_graphics.reset_stat_res_graphics();
		for (u32 player = 0; player < PlayerLimit; player++)
		{
			g_factions_colors_ordering[player] = g_player_color_ids[player];
			g_player_color_ids[player] = 0;
		}
		hotkeys.game_started = false;
		return true;
	}

	// Hooked to explore custom game commands. -Pr0nogo
	void game_recv(char* buffer, s32 size, bool in_replay) { // 004865D0
		// Size needs to be s32 to support less than 0 comparison...
		if (size) {
			bool is_in_replay = g_is_in_replay;
			u32 command_id = 0;
			u32 command_size = 0;
			while (true) {
				command_id = *buffer;
				if (!g_is_in_replay || in_replay) {
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
						{
							u32 command_size = g_turn_command_size[command_id];
							if (command_id >= GameCommand_Select && command_id <= GameCommand_ShiftDeselect) {
								command_size = 2 * (u8)buffer[1] + 2;
							}
							size -= command_size;
						}
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
				case GameCommand_Custom:
				{
					command_size = 12;
					if (size >= command_size) {
						u32 type = buffer[1];
						switch (type) {
							case NewGameCommand::GenericSwitchAbility: {
								u32 value = buffer[2];
								u32 player = buffer[3];
								u32 rust_id = buffer[4];
								for (u32 i = 0; i < SelectionCap; ++i) {
									Unit* unit = players_selections->unit[player][i];
									if (!unit) {
										continue;
									}

									UnitExt* unit_ext = get_unit_ext(unit);
									bool set_value = true;
									switch (unit->id) // Synced reads and conditions
									{
										case UnitId::TerranSavant:
										{
											if (unit_ext->timers[TimerSavantFormSwitchCooldown] > 1) {
												set_value = false;
												break;
											}
											u32 sound = value ? SoundId::SavantSwitchA01 : SoundId::SavantSwitchB01;
											helper::play_sound_from_unit(sound, unit, 1, 0);

											u32 image_id = value ? ImageId::SavantFormSwitchPower : ImageId::SavantFormSwitchEntropy;
											bw::add_unit_overlay(unit, image_id);

											unit_ext->timers[TimerSavantFormSwitchCooldown] = 49;
										} break;
									}
									if (set_value) {
										bw::set_generic_rust_value(unit, rust_id, value);
									}
								}
							} break;
							case NewGameCommand::IssueCTFCop2: {
								u32 player = buffer[2];
								for (u32 i = 0; i < SelectionCap; ++i) {
									Unit* unit = players_selections->unit[player][i];
									if (!unit || unit->is_dead() || unit->id != UnitId::ProtossStarSovereign || unit->main_order_id == OrderId::CTFCOP2) {
										continue;
									}
									unit->order_to(OrderId::CTFCOP2, unit->position.x, unit->position.y);
								}
							} break;
							case NewGameCommand::CameraSync: {
								u32 player = buffer[2];
								Point16 pos = *(Point16*)(buffer + 3);
								g_globals->players[player].camera = pos;
							} break;
							case NewGameCommand::SelectFaction: {
								u32 player = buffer[2];
								Faction faction = (Faction)buffer[3];
								g_globals->players[player].faction = faction;
								g_globals->players[player].faction_was_set = true;
								switch (g_player_table[player].race) {
									case RaceId::Terran: {
										switch (faction) {
											case Faction::TerranOrionImperium: {
												bw::printf("\tPlayer %u has chosen the Orion Imperium.", player + 1, GameTextColor::White);
											} break;
											case Faction::TerranHecatonCoalition: {
												bw::printf("\tPlayer %u has chosen the Hecaton Coalition.", player + 1, GameTextColor::White);
											} break;
											case Faction::TerranFuturistFederation: {
												bw::printf("\tPlayer %u has chosen the Futurist Federation.", player + 1, GameTextColor::White);
											} break;
											case Faction::TerranGoldenRepublic: {
												bw::printf("\tPlayer %u has chosen the Golden Republic.", player + 1, GameTextColor::White);
											} break;
											case Faction::TerranNydusTrinity: {
												bw::printf("\tPlayer %u has chosen the Nydus Trinity.", player + 1, GameTextColor::White);
											} break;
											case Faction::TerranUpstart: {
												bw::printf("\tPlayer %u has chosen the Terran Upstarts.", player + 1, GameTextColor::White);
											} break;
										}
									} break;
									case RaceId::Protoss: {
										switch (faction) {
											case Faction::ProtossDivinePrimacy: {
												bw::printf("\tPlayer %u has chosen the Divine Primacy.", player + 1, GameTextColor::White);
											} break;
											case Faction::ProtossOrderOfTheSacredSaber: {
												bw::printf("\tPlayer %u has chosen the Order of the Sacred Saber.", player + 1, GameTextColor::White);
											} break;
										}
									} break;
									case RaceId::Zerg: {
										switch (faction) {
											case Faction::ZergAtaltrethTendril: {
												bw::printf("\tPlayer %u has chosen the Ataltreth Tendril.", player + 1, GameTextColor::White);
											} break;
											case Faction::ZergZorthosTendril: {
												bw::printf("\tPlayer %u has chosen the Zorthos Tendril.", player + 1, GameTextColor::White);
											} break;
										}
									} break;
								}
								helper::play_sound_global(SoundId::Misc_button_wav);
							} break;
							case NewGameCommand::SyncColorPref: {
								auto command = (SyncColorPrefCommand*)buffer;
								g_globals->preferred_colors_set = true;
								for (PlayerColor color : command->colors) {
									if (color < PlayerColor::Count) {
										g_globals->players[command->player].preferred_colors.push_back(color);
									}
								}
							} break;
						}
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
							hooks::register_action(&player_apm_counter[*g_active_nation_id], buffer);
						}
						helper::save_replay_action(*g_replay_data, buffer, command_size, *g_active_storm_id);
						size -= command_size;
						buffer += command_size;
						is_in_replay = g_is_in_replay;
						goto maybe_process_command;
					}
				} break;
				default:
					return;
			}
		}
	}

	// Hooked to handle versioning. -Veeq7
	void initialize_provider_version(GameVersion* version) { // 00472770
		*version = {};
		version->size = sizeof(*version);
		version->game_id = 'CMBW';
		version->game_title = "Cosmonarchy";

		version->game_description = "internal version unknown";
		version->version_id = VersionHash;
		version->max_players = 8;
		version->category_bits = 255;
		helper::get_cd_key_info(&version->key_owner, &version->key);
		version->is_demo = *g_is_demo;
		version->lang = helper::local_get_lang();
	}
}