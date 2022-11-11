#include "trigger_system.h"
#include <limits.h>

#include "trigger_parser.h"
#include "trigger_sound.h"
#include "trigger_recolor.h"
#include "trigger_objectives.h"
#include "trigger_spawn.h"
#include "trigger_vars.h"
#include "trigger_events.h"

#include "ai_layouts.h"
#include "player_color.h"
#include "threshold.h"

bool trigger_game_paused;
bool trigger_system_init;
bool trigger_system_in_game;

u32 sound_replacements_count;
SoundReplacement sound_replacements[SoundReplacementCap];

u32 sprite_replacements_count;
SpriteReplacement sprite_replacements[SpriteReplacementCap];

bool has_custom_player_name[PlayerLimit];
Token custom_unit_names[PlayerLimit][UnitId::Count];

void init_trigger_system() {
	trigger_system_init = true;
	trigger_game_paused = false;
	g_action_table[47] = (ActionPointer)parse_comment_action;

	sound_replacements_count = 0;
	memset(sound_replacements, 0, sizeof(sound_replacements));

	sprite_replacements_count = 0;
	memset(sprite_replacements, 0, sizeof(sprite_replacements));

	memset(has_custom_player_name, 0, sizeof(has_custom_player_name));
	memset(custom_unit_names, 0, sizeof(custom_unit_names));

	init_recolors();
	init_objectives();
	init_sound();
	init_spawn();
	init_trigger_vars();
	init_trigger_events();
}

static void replace_unit_sprite(Unit* unit, u32 sprite_id) {
	if (unit->sprite && unit->sprite->id != sprite_id) {
		Sprite* new_sprite = helper::create_sprite(sprite_id, unit->position.x, unit->position.y, unit->player);
		if (new_sprite) {
			UnitExt* unit_ext = get_unit_ext(unit);
			unit_ext->values[ValueHasCustomSprite] = true;

			unit->sprite->free();
			unit->sprite = new_sprite;
			for (Image* image = new_sprite->images.head; image; image = image->link.next) {
				helper::set_image_direction(image, unit->current_direction_1);
			}
		}
	}
}

void update_trigger_system(bool in_game) {
	if (!trigger_system_init) {
		init_trigger_system();
	}

	if (in_game) {
		trigger_system_in_game = true;
	} else if (trigger_system_in_game) {
		// Leaving the game should reset music
		trigger_system_in_game = false;
		init_trigger_system();
	}

	if (in_game) {
		update_sound();
	}
	bool paused = scbw::is_game_paused();
	if (trigger_game_paused && !paused) {
		set_palette_recolor(recolor_system.current_recolor.set);
	}
	if (in_game && !paused) {
		update_recolors();
		update_spawn();
		update_trigger_vars();

		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			UnitExt* unit_ext = get_unit_ext(unit);
			bool has_replacement = false;
			for (u32 i = 0; i < sprite_replacements_count; ++i) {
				SpriteReplacement* sprite_replacement = &sprite_replacements[i];
				if (unit->player > PlayerLimit || !sprite_replacement->players.set[unit->player]) {
					continue;
				}
				if (unit->id >= UnitId::Count || !sprite_replacement->unit_ids.set[unit->id]) {
					continue;
				}
				replace_unit_sprite(unit, sprite_replacement->sprite_id);
				has_replacement = true;
				break;
			}
			if (!has_replacement && unit_ext->values[ValueHasCustomSprite]) {
				unit_ext->values[ValueHasCustomSprite] = false;
				u32 flingy_id = units_dat::graphic[unit->id];
				u32 sprite_id = flingy_dat::sprite_id[flingy_id];
				replace_unit_sprite(unit, sprite_id);
			}
		}
	}
	trigger_game_paused = paused;
}

b32 parse_comment_condition(TriggerAction* action) {
	bool is_local_player = *g_current_trigger_player_id == *g_local_nation_id;
	u32 trigger_player = *g_current_trigger_player_id;
	Parser parser = {};
	parser.at = helper::get_chk_string(action->text_string);
	while (parser_get_command(&parser)) {
		if (parser_eq(&parser, "condition")) {
			if (parser_get_word(&parser).size) {
				if (parser_eq(&parser, "combat")) {
					Area area = parser_get_area(&parser);
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					QuotaType time_quota_type = parser_get_quota_type(&parser, QuotaType_LessOrEqual);
					u32 time_quota = parser_get_time_ticks(&parser);
					u32 scouting_player = parser_get_player(&parser);

					for (u32 i = 0; i < combat_events.count; ++i) {
						TriggerEvent* event = &kill_events.events[i];
						if (!check_area(area, event->position, units_dat::base_property[event->target_unit_id] & UnitProperty::Flyer)) {
							continue;
						}
						if ((!source_players.set[event->source_player] || !source_unit_ids.set[event->source_unit_id] || !target_players.set[event->target_player] || !target_unit_ids.set[event->target_unit_id])
						&&  (!source_players.set[event->target_player] || !source_unit_ids.set[event->target_unit_id] || !target_players.set[event->source_player] || !target_unit_ids.set[event->source_unit_id])) {
							continue;
						}
						if (!check_quota(time_quota_type, time_quota, *g_elapsed_time_frames - event->timestamp)) {
							continue;
						}
						if (scouting_player) {
							if (!(event->seen_by & 1 << (scouting_player - 1))) {
								continue;
							}
						}
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "kills")) {
					Area area = parser_get_area(&parser);
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					QuotaType quota_type = parser_get_quota_type(&parser, QuotaType_GreaterOrEqual);
					u32 quota = parser_get_s32(&parser);
					QuotaType time_quota_type = parser_get_quota_type(&parser, QuotaType_LessOrEqual);
					u32 time_quota = parser_get_time_ticks(&parser);
					u32 scouting_player = parser_get_player(&parser);

					u32 count = 0;
					for (u32 i = 0; i < kill_events.count; ++i) {
						TriggerEvent* event = &kill_events.events[i];
						if (!check_area(area, event->position, units_dat::base_property[event->target_unit_id] & UnitProperty::Flyer)) {
							continue;
						}
						if (!source_players.set[event->source_player]) {
							continue;
						}
						if (!source_unit_ids.set[event->source_unit_id]) {
							continue;
						}
						if (!target_players.set[event->target_player]) {
							continue;
						}
						if (!target_unit_ids.set[event->target_unit_id]) {
							continue;
						}
						if (!check_quota(time_quota_type, time_quota, *g_elapsed_time_frames - event->timestamp)) {
							continue;
						}
						if (scouting_player) {
							if (!(event->seen_by & 1 << (scouting_player - 1))) {
								continue;
							}
						}
						count += 1;
					}

					if (check_quota(quota_type, quota, count)) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "deaths")) {
					Area area = parser_get_area(&parser);
					PlayerBitset players = parser_get_player_bitset(&parser);
					UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
					QuotaType quota_type = parser_get_quota_type(&parser, QuotaType_GreaterOrEqual);
					u32 quota = parser_get_s32(&parser);
					QuotaType time_quota_type = parser_get_quota_type(&parser, QuotaType_LessOrEqual);
					u32 time_quota = parser_get_time_ticks(&parser);
					u32 scouting_player = parser_get_player(&parser);

					u32 count = 0;
					for (u32 i = 0; i < kill_events.count; ++i) {
						TriggerEvent* event = &kill_events.events[i];
						if (!check_area(area, event->position, units_dat::base_property[event->target_unit_id] & UnitProperty::Flyer)) {
							continue;
						}
						if (!players.set[event->target_player]) {
							continue;
						}
						if (!unit_ids.set[event->target_unit_id]) {
							continue;
						}
						if (!check_quota(time_quota_type, time_quota, *g_elapsed_time_frames - event->timestamp)) {
							continue;
						}
						if (scouting_player) {
							if (!(event->seen_by & 1 << (scouting_player - 1))) {
								continue;
							}
						}
						count += 1;
					}

					if (check_quota(quota_type, quota, count)) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "unit_count")) {
					Area area = parser_get_area(&parser);
					PlayerBitset players = parser_get_player_bitset(&parser);
					UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
					QuotaType quota_type = parser_get_quota_type(&parser, QuotaType_GreaterOrEqual);
					u32 quota = parser_get_s32(&parser);
					u32 scouting_player = parser_get_player(&parser);
					QuotaType scouting_time_quota_type = parser_get_quota_type(&parser, QuotaType_LessOrEqual);
					u32 scouting_time_quota = parser_get_time_ticks(&parser);

					area.box.w += area.box.x;
					area.box.h += area.box.y;
					EntryArray units = {};
					if (scouting_player) {
						seen_unit_search_rect(&units, scouting_player - 1, area.box);
					} else {
						unit_search_rect(&units, area.box);
					}
					area.box.w -= area.box.x;
					area.box.h -= area.box.y;

					u32 count = 0;
					for (u32 i = 0; i < units.count; ++i) {
						Unit* unit = units.units[i];
						UnitExt* unit_ext = get_unit_ext(unit);
						Point16 position = unit->position;
						if (scouting_player) {
							position = unit_ext->last_seen_position[scouting_player - 1];
						}
						if (!check_area(area, position, unit->status & UnitStatus::InAir)) {
							continue;
						}
						if (unit->player >= PlayerLimit || !players.set[unit->player]) {
							continue;
						}
						if (unit->id >= UnitId::Count || !unit_ids.set[unit->id]) {
							continue;
						}
						if (!scouting_player || !scouting_time_quota || check_quota(scouting_time_quota_type, scouting_time_quota, unit_ext->last_seen_timer[scouting_player - 1])) {
							count++;
						}
					}

					if (check_quota(quota_type, quota, count)) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "local_camera")) {
					Area area = parser_get_area(&parser);
					Token min_visibility = parser_get_word(&parser);

					Point16 center = {};
					center.x = *g_move_to_x + 320;
					center.y = *g_move_to_y + 200;

					if (center.x >= area.box.x && center.y <= area.box.x + area.box.w &&
						center.x >= area.box.y && center.y <= area.box.y + area.box.h) {
						ActiveTile* tile = scbw::get_active_tile_at_coords(center.x, center.y);
						// TODO: should we check area flags?
						if (token_eqi(&min_visibility, "unexplored")) {
							return true;
						} else if (token_eqi(&min_visibility, "explored")) {
							if (!(tile->explored_flags & (1 << *g_local_nation_id))) {
								return true;
							}
						} else if (token_eqi(&min_visibility, "visible") || true) {
							if (!(tile->visibility_flags & (1 << *g_local_nation_id))) {
								return true;
							}
						}
					}
					return false;
				} else if (parser_eq(&parser, "var") || parser_eq(&parser, "timer")) {
					bool is_timer = parser_eq(&parser, "timer");
					TriggerVarType var_type = is_timer ? TriggerVarType_Timer : TriggerVarType_Var;

					Token name = parser_get_string(&parser);
					TriggerVar* var = find_trigger_var(var_type, name.data);
					QuotaType quota_type = parser_get_quota_type(&parser, QuotaType_Equal);
					s32 quota = is_timer ? parser_get_time_ticks(&parser) : parser_get_s32(&parser);
					if (var) {
						return check_quota(quota_type, quota, var->value);
					}
					return false;
				}
			}
		}
	}

	return true;
}

b32 __fastcall parse_comment_action(TriggerAction* action) {
	bool is_local_player = *g_current_trigger_player_id == *g_local_nation_id;
	u32 trigger_player = *g_current_trigger_player_id;
	Parser parser = {};
	parser.at = helper::get_chk_string(action->text_string);
	while (parser_get_command(&parser)) {
		// Local Only Triggers
		if (parser_eq(&parser, "play_music") && is_local_player) {
			Token filename = parser_get_string(&parser);
			playback.next_music = get_sound(filename.data);
			playback.target_fade = 0;
		} else if (parser_eq(&parser, "stop_music") && is_local_player) {
			playback.target_fade = 0;
		} else if (parser_eq(&parser, "pause_music") && is_local_player) {
			playback.target_fade = 0;
			playback.paused = true;
		} else if (parser_eq(&parser, "resume_music") && is_local_player) {
			playback.target_fade = 1;
			playback.paused = false;
			play_music(true);
		} else if (parser_eq(&parser, "music_fade_in") && is_local_player) {
			f32 fade_in = parser_get_time_seconds(&parser);
			playback.fade_in = fade_in;
		} else if (parser_eq(&parser, "music_fade_out") && is_local_player) {
			f32 fade_out = parser_get_time_seconds(&parser);
			playback.fade_out = fade_out;
		} else if (parser_eq(&parser, "music_volume") && is_local_player) {
			f32 volume = parser_get_f32(&parser);
			f32 time = parser_get_time_seconds(&parser);
			if (time == 0) {
				playback.volume = 0;
			}
			playback.target_volume = volume;
			playback.volume_time = time;
		} else if (parser_eq(&parser, "playlist_clear") && is_local_player) {
			memset(&current_playlist, 0, sizeof(current_playlist));
		} else if (parser_eq(&parser, "playlist_add") && is_local_player) {
			Token filename = parser_get_string(&parser);
			if (current_playlist.size < MaxSound) {
				Sound* music = get_sound(filename.data);
				if (music) {
					current_playlist.tracks[current_playlist.size++] = music;
				}
			} else {
				scbw::printf("Error: Playlist already full! (%s)", filename.data);
			}
		} else if (parser_eq(&parser, "playlist_shuffle") && is_local_player) {
			s32 do_shuffle = parser_get_s32(&parser); // 0 - false, 1 - true
			current_playlist.shuffle = do_shuffle;
		} else if (parser_eq(&parser, "set_player_name") && is_local_player) {
			u32 player = parser_get_player(&parser);
			Token name = parser_get_string(&parser);
			s32 ignore_human_name = parser_get_s32(&parser);
			if (player) {
				player -= 1;
				has_custom_player_name[player] = true;
				if (!ignore_human_name && (g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::HumanDefeated)) {
					snprintf(g_player_name_list->player[player].player_name, sizeof(g_player_name_list->player[player].player_name),
						"%s (%s)", name.data, g_player_table[player].name);
				} else {
					snprintf(g_player_name_list->player[player].player_name, sizeof(g_player_name_list->player[player].player_name),
						"%s", name.data);
				}
			}
		} else if (parser_eq(&parser, "set_force_name") && is_local_player) {
			u32 player = parser_get_player(&parser);
			Token name = parser_get_string(&parser);
			if (player) {
				player -= 1;
				snprintf(g_player_name_list->player[player].force_name, sizeof(g_player_name_list->player[player].force_name),
					"%s", name.data);
			}
		} else if (parser_eq(&parser, "recolor") && is_local_player) {
			TimedRecolor recolor = {};
			recolor.set  = parser_get_recolor_set(&parser);
			recolor.time = parser_get_time_seconds(&parser);

			recolor_system.recolor_time = 0;
			recolor_system.source_recolor = recolor_system.current_recolor;
			recolor_system.target_recolor = recolor;
			if (!recolor.time) {
				recolor_system.current_recolor = recolor;
				set_palette_recolor(recolor.set);
			}
		} else if (parser_eq(&parser, "recolor_loop_add") && is_local_player) {
			TimedRecolor recolor = {};
			recolor.set  = parser_get_recolor_set(&parser);
			recolor.time = parser_get_time_seconds(&parser);

			if (recolor_system.recolor_list.count < MaxRecolors) {
				recolor_system.recolor_list.recolors[recolor_system.recolor_list.count++] = recolor;
			}
		} else if (parser_eq(&parser, "recolor_loop_clear") && is_local_player) {
			memset(&recolor_system.recolor_list, 0, sizeof(recolor_system.recolor_list));
		} else if (parser_eq(&parser, "objective_set")) {
			s32 key = parser_get_s32(&parser);
			Token text = parser_get_string(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				sprintf_s(objective->text, "%s", text.data);
				objective->show = true;
			}
		} else if (parser_eq(&parser, "objective_text") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			Token text = parser_get_string(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				sprintf_s(objective->text, "%s", text.data);
			}
		} else if (parser_eq(&parser, "objective_desc") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			Token text = parser_get_string(&parser);

			// TODO: set objective description
		} else if (parser_eq(&parser, "objective_show") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			s32 show = parser_get_s32(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				objective->show = show;
			}
		} else if (parser_eq(&parser, "objective_score") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			s32 score = parser_get_s32(&parser);
			s32 max_score = parser_get_s32(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				objective->score = score;
				objective->max_score = max_score;
				if (objective->score < 0) {
					objective->score = 0;
				}
				if (objective->score > objective->max_score) {
					objective->score = objective->max_score;
				}
			}
		} else if (parser_eq(&parser, "objective_score_add") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			s32 score = parser_get_s32(&parser);
			s32 max_score = parser_get_s32(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				objective->score += score;
				objective->max_score += max_score;
				if (objective->score < 0) {
					objective->score = 0;
				}
				if (objective->score > objective->max_score) {
					objective->score = objective->max_score;
				}
			}
		} else if (parser_eq(&parser, "objective_remove") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			Objective* objective = find_objective(key);
			if (objective) {
				remove_objective(objective);
			}
		} else if (parser_eq(&parser, "objectives_sync") && is_local_player) {
			// TODO: sync objectives to mission objectives menu
		} else if (parser_eq(&parser, "objectives_clear") && is_local_player) {
			memset(&objectives, 0, sizeof(objectives));
		} else if (parser_eq(&parser, "show_player_layouts") && is_local_player) {
			PlayerBitset players = parser_get_player_bitset(&parser);
			s32 value = parser_get_s32(&parser);
			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				show_player_layouts[i] = value;
			}
		} else if (parser_eq(&parser, "set_player_unit_name") && is_local_player) {
			u32 player = parser_get_player(&parser);
			u32 unit_id = parser_get_unit_id(&parser);
			Token name = parser_get_string(&parser);
			if (player && unit_id != UnitId::None) {
				player -= 1;
				custom_unit_names[player][unit_id] = name;
			}
		} else if (parser_eq(&parser, "set_player_race_name") && is_local_player) {
			u32 player = parser_get_player(&parser);
			Token old_race = parser_get_string(&parser);
			Token new_race = parser_get_string(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);

			if (player) {
				player -= 1;
				for (u32 i = 0; i < UnitId::Count; ++i) {
					if (!unit_ids.set[i]) {
						continue;
					}
					if (!custom_unit_names[player][i].size) {
						const char* default_name = Unit::get_name(i);
						u32 len = strlen(default_name);
						if (len > MaxToken) {
							len = MaxToken;
						}
						custom_unit_names[player][i].size = len;
						memset(custom_unit_names[player][i].data, 0, MaxToken);
						strncpy(custom_unit_names[player][i].data, default_name, len);
					}

					if (old_race.size <= custom_unit_names[player][i].size && strncmp(custom_unit_names[player][i].data, old_race.data, old_race.size) == 0) {
						Token copy = custom_unit_names[player][i];
						snprintf(custom_unit_names[player][i].data, MaxToken, "%s%s", new_race.data, copy.data + old_race.size);
						custom_unit_names[player][i].size = strlen(custom_unit_names[player][i].data);
					}
				}
			}
		} else if (parser_eq(&parser, "reset_player_unit_names") && is_local_player) {
			u32 player = parser_get_player(&parser);
			if (player) {
				player -= 1;
				for (u32 i = 0; i < UnitId::Count; ++i) {
					custom_unit_names[player][i] = {};
				}
			}
		} else if (parser_eq(&parser, "unit_sound") && is_local_player) {
			SoundReplacement sound_replacement = {};
			sound_replacement.players = parser_get_player_bitset(&parser);
			sound_replacement.unit_ids = parser_get_unit_id_bitset(&parser);
			sound_replacement.animation = parser_get_enum(&parser, animation_type_names, ARRAY_SIZE(animation_type_names));
			if (parser_eq(&parser, "*")) {
				sound_replacement.any_animation = true;
			}
			sound_replacement.old_sound_id = parser_get_s32(&parser);
			if (parser_eq(&parser, "*")) {
				sound_replacement.any_sound = true;
			}
			Token path = parser_get_string(&parser);
			sound_replacement.new_sound = get_sound(path.data);
			sound_replacement.replace = parser_get_s32(&parser);
			if (sound_replacements_count < SoundReplacementCap) {
				sound_replacements[sound_replacements_count++] = sound_replacement;
			}
		// Bitset Local Only Triggers
		} else if (parser_eq(&parser, "play_sound")) {
			Token path = parser_get_string(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			Point16 position = parser_get_position(&parser);
			Token min_visibility = parser_get_word(&parser);

			if (trigger_player < PlayerLimit && players.set[trigger_player]) {
				if (position.x == 0 && position.y == 0) {
					play_sound(get_sound(path.data), get_sound_volume());
				} else {
					bool play = false;
					ActiveTile* tile = scbw::get_active_tile_at_coords(position.x, position.y);

					if (token_eqi(&min_visibility, "unexplored")) {
						play = true;
					} else if (token_eqi(&min_visibility, "explored")) {
						if (!(tile->explored_flags & (1 << *g_local_nation_id))) {
							play = true;
						}
					} else if (token_eqi(&min_visibility, "visible") || true) {
						if (!(tile->visibility_flags & (1 << *g_local_nation_id))) {
							play = true;
						}
					}

					if (play) {
						play_sound(get_sound(path.data), get_sound_volume_at_pos(position), helper::get_sfx_pan_from_screen_distance(position.x));
					}
				}
			}
		// Gameplay Actions
		} else if (parser_eq(&parser, "unit_sprite")) {
			SpriteReplacement sprite_replacement = {};
			sprite_replacement.players = parser_get_player_bitset(&parser);
			sprite_replacement.unit_ids = parser_get_unit_id_bitset(&parser);
			parser_get_word(&parser);
			if (parser_eq(&parser, "sprite_id")) {
				sprite_replacement.sprite_id = parser_get_s32(&parser);
			} else if (parser_eq(&parser, "unit_id")) {
				u32 unit_id = parser_get_unit_id(&parser);
				u32 flingy_id = units_dat::graphic[unit_id];
				sprite_replacement.sprite_id = flingy_dat::sprite_id[flingy_id];
			} else {
				continue;
			}

			if (sprite_replacements_count < SpriteReplacementCap) {
				sprite_replacements[sprite_replacements_count++] = sprite_replacement;
			}
		} else if (parser_eq(&parser, "clear_unit_sprite")) {
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			u32 sprite_id = 0;
			parser_get_word(&parser);
			if (parser_eq(&parser, "sprite_id")) {
				sprite_id = parser_get_s32(&parser);
			} else if (parser_eq(&parser, "unit_id")) {
				u32 unit_id = parser_get_unit_id(&parser);
				u32 flingy_id = units_dat::graphic[unit_id];
				sprite_id = flingy_dat::sprite_id[flingy_id];
			} else {
				continue;
			}

			for (u32 i = 0; i < sprite_replacements_count; ++i) {
				SpriteReplacement* sprite_replacement = &sprite_replacements[i];
				if (memcmp(&sprite_replacement->players, &players, sizeof(PlayerBitset)) != 0) {
					continue;
				}
				if (memcmp(&sprite_replacement->unit_ids, &unit_ids, sizeof(UnitIdBitset)) != 0) {
					continue;
				}
				if (sprite_replacement->sprite_id != sprite_id) {
					continue;
				}
				sprite_replacements[i--] = sprite_replacements[--sprite_replacements_count];
			}
		} else if (parser_eq(&parser, "clear_creep")) {
			Area location = parser_get_area(&parser);
			scbw::clear_creep(location.box);
		} else if (parser_eq(&parser, "spawn")) {
			SpawnSequence sequence = {};
			sequence.count = parser_get_s32(&parser);
			sequence.unit_id = parser_get_unit_id(&parser);
			sequence.player = parser_get_player(&parser, true);
			sequence.type = parser_get_enum(&parser, spawn_type_names, SpawnType_Count);
			sequence.delay = parser_get_time_ticks(&parser);
			sequence.stagger = parser_get_time_ticks(&parser);
			sequence.source_pos = parser_get_position(&parser);
			sequence.move_pos = parser_get_position(&parser);
			
			if (sequence.player) {
				sequence.player -= 1;
				if (spawn_sequence_count < SpawnSequenceCap) {
					spawn_sequences[spawn_sequence_count++] = sequence;
				}
			}
		} else if (parser_eq(&parser, "layouts_enable")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 enable = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				AiLayouts* layouts = &player_layouts[i];
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (placeholder->is_townhall_marker) {
						continue;
					}
					if (placeholder->unit_id == UnitId::None || !unit_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
						continue;
					}
					placeholder->disabled = !enable;
				}
			}
		} else if (parser_eq(&parser, "layouts_priority_add")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 priority = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				AiLayouts* layouts = &player_layouts[i];
				bool changed = false;
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (placeholder->is_townhall_marker) {
						continue;
					}
					if (!unit_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
						continue;
					}
					placeholder->priority = CLAMP(placeholder->priority + priority, 0, 255);
					changed = true;
				}
				if (changed) {
					sort_layouts(layouts);
				}
			}
		} else if (parser_eq(&parser, "layouts_priority_set")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 priority = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				AiLayouts* layouts = &player_layouts[i];
				bool changed = false;
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (placeholder->is_townhall_marker) {
						continue;
					}
					if (placeholder->unit_id >= UnitId::Count || !unit_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
						continue;
					}
					placeholder->priority = CLAMP(priority, 0, 255);
					changed = true;
				}
				if (changed) {
					sort_layouts(layouts);
				}
			}
		} else if (parser_eq(&parser, "create_town")) {
			u32 player = parser_get_player(&parser);
			Point16 pos = parser_get_position(&parser);
			s32 worker_count = parser_get_s32(&parser);
			s32 attach_to_nearest_resarea = parser_get_s32(&parser);
			Area claim_placeholders_area = parser_get_area(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);

			if (player) {
				player -= 1;
				AiTown* town = helper::create_ai_town(player, pos.x, pos.y);
				if (town) {
					town->inited = true;
					AiLayoutsTown* layouts_town = get_layouts_town(town);
					layouts_town->worker_limit = worker_count;
					layouts_town->worker_maynard = worker_count;
					if (worker_count) {
						helper::ai_get_old_peons(town, worker_count);
					}
					if (attach_to_nearest_resarea) {
						town->resarea = scbw::get_closest_resarea(pos);
						layouts_town->wants_no_resarea = false;
					} else {
						town->resarea = 0;
						layouts_town->wants_no_resarea = true;
					}
					town->resource_units_not_set = town->resarea != 0;
					if (claim_placeholders_area.box.w && claim_placeholders_area.box.h) {
						AiLayouts* layouts = &player_layouts[player];
						for (u32 i = 0; i < layouts->placeholders_count; ++i) {
							AiPlaceholder* placeholder = &layouts->placeholders[i];
							if (placeholder->unit_id >= UnitId::Count || !unit_ids.set[placeholder->unit_id]) {
								continue;
							}
							if (!check_area(claim_placeholders_area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
								continue;
							}
							if (!placeholder->town) {
								placeholder->town = town;
								if (show_player_layouts[player]) {
									scbw::printf("Player %u claimed %s #%u from resarea %u", player + 1, Unit::get_name(placeholder->unit_id), i, placeholder->resarea);
								}
							}
						}
					}
				}
			}
		} else if (parser_eq(&parser, "set_start_location")) {
			u32 player = parser_get_player(&parser);
			Point16 position = parser_get_position(&parser);

			if (player) {
				player -= 1;
				g_start_positions[player] = position;
			}
		} else if (parser_eq(&parser, "set_race")) {
			u32 player = parser_get_player(&parser);
			u32 race = parser_get_enum(&parser, race_names, ARRAY_SIZE(race_names));

			if (player) {
				player -= 1;
				g_player_table[player].race = race;
			}
		} else if (parser_eq(&parser, "set_player_color")) {
			u32 player = parser_get_player(&parser);
			u32 player_color = parser_get_enum(&parser, playerColorNames, ARRAY_SIZE(playerColorNames));

			if (player) {
				player -= 1;
				set_player_color(player, player_color);
			}
		} else if (parser_eq(&parser, "set_ai_auto_burrow")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 enable = parser_get_s32(&parser);

			area.box.w += area.box.x;
			area.box.h += area.box.y;
			EntryArray units = {};
			unit_search_rect(&units, area.box);
			area.box.w -= area.box.x;
			area.box.h -= area.box.y;

			for (u32 i = 0; i < units.count; ++i) {
				Unit* unit = units.units[i];
				if (unit->player >= PlayerLimit || !players.set[unit->player]) {
					continue;
				}
				if (unit->id >= UnitId::Count || !unit_ids.set[unit->id]) {
					continue;
				}
				if (!check_area(area, unit->position, unit->status & UnitStatus::InAir)) {
					continue;
				}
				UnitExt* unit_ext = get_unit_ext(unit);
				unit_ext->values[ValueDisableAutoBurrow] = !enable;
			}
		} else if (parser_eq(&parser, "var") || parser_eq(&parser, "timer")) {
			bool is_timer = parser_eq(&parser, "timer");
			TriggerVarType var_type = is_timer ? TriggerVarType_Timer : TriggerVarType_Var;

			Token name = parser_get_string(&parser);
			Token var_operator = parser_get_string(&parser);

			if (token_eq(&var_operator, "=")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = find_or_create_trigger_var(var_type, name.data);
				if (var) {
					var->value = value;
				}
			} else if (token_eq(&var_operator, "+=") || token_eq(&var_operator, "+==")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = parser.token.size == 3 ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
				if (var) {
					var->value += value;
				}
			} else if (token_eq(&var_operator, "-=") || token_eq(&var_operator, "-==")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = parser.token.size == 3 ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
				if (var) {
					var->value -= value;
				}
			} else if (token_eq(&var_operator, "*=") || token_eq(&var_operator, "*==")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = parser.token.size == 3 ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
				if (var) {
					var->value *= value;
				}
			} else if (token_eq(&var_operator, "/=") || token_eq(&var_operator, "/==")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = parser.token.size == 3 ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
				if (var) {
					var->value /= value;
				}
			} else if (token_eq(&var_operator, "%=") || token_eq(&var_operator, "%==")) {
				s32 value = parser_get_var_value(&parser, is_timer);
				TriggerVar* var = parser.token.size == 3 ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
				if (var) {
					var->value %= value;
				}
			} else if (token_eq(&var_operator, "toggle")) {
				TriggerVar* var = find_or_create_trigger_var(var_type, name.data);
				if (var) {
					var->value = !var->value;
				}
			} else if (token_eq(&var_operator, "unset")) {
				unset_trigger_var(var_type, name.data);
			}
		} else if (parser_eq(&parser, "convert_to_ai")) {
			u32 player = parser_get_player(&parser);

			if (player) {
				player -= 1;
				g_player_table[player].type = PlayerType::Computer;
				scbw::run_default_ai(player, g_start_positions[player]);
			};
		} else if (parser_eq(&parser, "threshold_set_fraud_slots")) {
			PlayerBitset players = parser_get_player_bitset(&parser);

			threshold_config.enabled = true;
			for (u32 player = 0; player < PlayerLimit; ++player) {
				threshold_config.fraud_allowed[player] = players.set[player];
				if (players.set[player]) {
					threshold_config.witness_enabled[player] = false;
				}
			}
		} else if (parser_eq(&parser, "threshold_set_witness_slots")) {
			PlayerBitset players = parser_get_player_bitset(&parser);

			for (u32 player = 0; player < PlayerLimit; ++player) {
				threshold_config.witness_enabled[player] = players.set[player];
				if (players.set[player]) {
					threshold_config.fraud_allowed[player] = false;
				}
			}
		} else if (parser_eq(&parser, "threshold_autoscale")) {
			f32 value = parser_get_f32(&parser);

			threshold_config.auto_scale = value;
		} else if (parser_eq(&parser, "threshold_initial_count")) {
			u32 count = parser_get_s32(&parser);

			threshold_config.initial_count = count;
		} else if (parser_eq(&parser, "threshold_min_count")) {
			u32 count = parser_get_s32(&parser);

			threshold_config.min_count = count;
		} else if (parser_eq(&parser, "threshold_total_count")) {
			u32 count = parser_get_s32(&parser);

			threshold_config.total_count = count;
		} else if (parser_eq(&parser, "threshold_timer")) {
			u32 timer = parser_get_time_ticks(&parser);

			threshold_config.timer = timer;
		}
	}

	return true;
}