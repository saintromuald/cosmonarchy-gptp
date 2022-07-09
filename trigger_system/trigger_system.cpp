#include "trigger_system.h"
#include <limits.h>

#include "token_parser.h"
#include "base_layouts.h"

#include "trigger_music.h"
#include "trigger_recolor.h"
#include "trigger_objectives.h"
#include "trigger_spawn.h"

bool trigger_game_paused;
bool trigger_system_init;
bool trigger_system_in_game;

u32 sound_replacements_count;
SoundReplacement sound_replacements[SoundReplacementCap];

bool has_custom_player_name[PlayerLimit];
Token custom_unit_names[PlayerLimit][UnitId::Count];
u32 trigger_combat[ScoutingPlayerLimit][PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];
u32 trigger_kills[ScoutingPlayerLimit][PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];
u32 trigger_deaths[ScoutingPlayerLimit][PlayerLimit][UnitId::Count];

static void init_trigger_system() {
	trigger_system_init = true;
	g_action_table[47] = (ActionPointer)parse_comment_action;

	sound_replacements_count = 0;
	memset(sound_replacements, 0, sizeof(sound_replacements));

	memset(has_custom_player_name, 0, sizeof(has_custom_player_name));
	memset(custom_unit_names, 0, sizeof(custom_unit_names));
	memset(trigger_combat, 0, sizeof(trigger_combat));
	memset(trigger_kills, 0, sizeof(trigger_kills));
	memset(trigger_deaths, 0, sizeof(trigger_deaths));

	init_objectives();
	init_music();
	init_recolors();
	init_spawn();
}

void update_trigger_system(bool in_game) {
	if (!trigger_system_init || (in_game && *g_elapsed_time_frames == 0)) {
		init_trigger_system();
	}

	if (in_game) {
		trigger_system_in_game = true;
	} else if (trigger_system_in_game) {
		// Leaving the game should reset music
		trigger_system_in_game = false;
		init_trigger_system();
	}

	update_music();
	bool paused = scbw::is_game_paused();
	if (trigger_game_paused && !paused) {
		set_palette_recolor(recolor_system.current_recolor.set);
	}
	if (in_game && !paused) {
		update_recolors();
		update_spawn();
	}
	trigger_game_paused = paused;
}

b32 parse_comment_condition(TriggerAction* action) {
	bool is_local_player = *g_current_trigger_player_id == *g_local_nation_id;
	Parser parser = {};
	parser.at = helper::get_chk_string(action->text_string);
	while (parser_get_command(&parser)) {
		if (parser_eq(&parser, "condition")) {
			if (parser_get_word(&parser)) {
				if (parser_eq(&parser, "combat")) {
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					f32 time_quota = parser_get_time(&parser);
					u32 scouting_player = parser_get_player(&parser);

					for (u32 source_player = 0; source_player < PlayerLimit; ++source_player) {
						if (!source_players.set[source_player]) {
							continue;
						}
						for (u32 source_unit = 0; source_unit < UnitId::Count; ++source_unit) {
							if (!source_unit_ids.set[source_unit]) {
								continue;
							}
							for (u32 target_player = 0; target_player < PlayerLimit; ++target_player) {
								if (!target_players.set[target_player]) {
									continue;
								}
								for (u32 target_unit = 0; target_unit < UnitId::Count; ++target_unit) {
									if (!target_unit_ids.set[target_unit]) {
										continue;
									}
									u32 combat = trigger_combat[scouting_player][source_player][source_unit][target_player][target_unit];
									if (combat && *g_elapsed_time_frames - combat <= time_quota * 24) {
										return true;
									}
								}
							}
						}
					}
					return false;
				} else if (parser_eq(&parser, "kills")) {
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					u32 quota = parser_get_s32(&parser);
					u32 scouting_player = parser_get_player(&parser);

					u32 count = 0;
					for (u32 source_player = 0; source_player < PlayerLimit; ++source_player) {
						if (!source_players.set[source_player]) {
							continue;
						}
						for (u32 source_unit = 0; source_unit < UnitId::Count; ++source_unit) {
							if (!source_unit_ids.set[source_unit]) {
								continue;
							}
							for (u32 target_player = 0; target_player < PlayerLimit; ++target_player) {
								if (!target_players.set[target_player]) {
									continue;
								}
								for (u32 target_unit = 0; target_unit < UnitId::Count; ++target_unit) {
									if (!target_unit_ids.set[target_unit]) {
										continue;
									}
									count += trigger_kills[scouting_player][source_player][source_unit][target_player][target_unit];
								}
							}
						}
					}

					if (count >= quota) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "deaths")) {
					PlayerBitset players = parser_get_player_bitset(&parser);
					UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
					u32 quota = parser_get_s32(&parser);
					u32 scouting_player = parser_get_player(&parser);

					u32 count = 0;
					for (u32 player = 0; player < PlayerLimit; ++player) {
						if (!players.set[player]) {
							continue;
						}
						for (u32 unit = 0; unit < UnitId::Count; ++unit) {
							if (!unit_ids.set[unit]) {
								continue;
							}
							count += trigger_deaths[scouting_player][player][unit];
						}
					}

					if (count >= quota) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "unit_count")) {
					Box32 area = parser_get_area(&parser);
					PlayerBitset players = parser_get_player_bitset(&parser);
					UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
					u32 quota = parser_get_s32(&parser);
					u32 scouting_player = parser_get_player(&parser);
					f32 scouting_time_quota = parser_get_time(&parser);

					area.w += area.x;
					area.h += area.y;
					EntryArray units = {};
					if (scouting_player) {
						units = seen_unit_search_rect(scouting_player - 1, area);
					} else {
						units = unit_search_rect(area);
					}

					u32 count = 0;
					for (u32 i = 0; i < units.count; ++i) {
						Unit* unit = units.units[i];
						if (unit->player < PlayerLimit && unit->id < UnitId::Count) {
							if (!players.set[unit->player]) {
								continue;
							}
							if (!unit_ids.set[unit->id]) {
								continue;
							}
							UnitExt* unit_ext = globals.get_unit_ext(unit);
							if (!scouting_player || !scouting_time_quota || unit_ext->last_seen_timer[scouting_player - 1] < scouting_time_quota / 24) {
								count++;
							}
						}
					}

					if (count >= quota) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "local_camera")) {
					Box32 area = parser_get_area(&parser);
					if (*g_move_to_x + 320 >= area.x && *g_move_to_x + 320 <= area.x + area.w &&
						*g_move_to_y + 200 >= area.y && *g_move_to_y + 200 <= area.y + area.h) {
						return true;
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
	Parser parser = {};
	parser.at = helper::get_chk_string(action->text_string);
	while (parser_get_command(&parser)) {
		// Local Only Triggers
		if (parser_eq(&parser, "play_music") && is_local_player) {
			Token filename = parser_get_string(&parser);
			playback.next_music = get_music(filename.data);
			playback.target_fade = 0;
		} else if (parser_eq(&parser, "stop_music") && is_local_player) {
			playback.target_fade = 0;
		} else if (parser_eq(&parser, "pause_music") && is_local_player) {
			playback.target_fade = 0;
			playback.paused = true;
		} else if (parser_eq(&parser, "resume_music") && is_local_player) {
			playback.target_fade = 1;
			playback.paused = false;
			resume_music();
		} else if (parser_eq(&parser, "fade_in") && is_local_player) {
			f32 fade_in = parser_get_time(&parser);
			playback.fade_in = fade_in;
		} else if (parser_eq(&parser, "fade_out") && is_local_player) {
			f32 fade_out = parser_get_time(&parser);
			playback.fade_out = fade_out;
		} else if (parser_eq(&parser, "volume") && is_local_player) {
			f32 volume = parser_get_f32(&parser);
			f32 time = parser_get_time(&parser);
			if (time == 0) {
				playback.volume = 0;
			}
			playback.target_volume = volume;
			playback.volume_time = time;
		} else if (parser_eq(&parser, "playlist_clear") && is_local_player) {
			current_playlist.size = 0;
		} else if (parser_eq(&parser, "playlist_add") && is_local_player) {
			Token filename = parser_get_string(&parser);
			if (current_playlist.size < MaxMusic) {
				Music* music = get_music(filename.data);
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
			if (player) {
				player -= 1;
				has_custom_player_name[player] = true;
				if (g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::HumanDefeated) {
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
			recolor.time = parser_get_time(&parser);

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
			recolor.time = parser_get_time(&parser);

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
		} else if (parser_eq(&parser, "log_placeholder_requests") && is_local_player) {
			u32 player = parser_get_player(&parser);
			s32 value = parser_get_s32(&parser);
			if (player) {
				player -= 1;
				log_placeholder_requests[player] = value;
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
		} else if (parser_eq(&parser, "replace_sound") && is_local_player) {
			SoundReplacement sound_replacement = {};
			sound_replacement.player = parser_get_player(&parser);
			sound_replacement.image_id = parser_get_s32(&parser);
			sound_replacement.animation = parser_get_s32(&parser);
			sound_replacement.old_sound_id = parser_get_s32(&parser);
			if (parser_eq(&parser, "*")) {
				sound_replacement.any_sound = true;
			}
			sound_replacement.new_sound_id = parser_get_s32(&parser);
			sound_replacement.replace = parser_get_s32(&parser);
			if (sound_replacement.player) {
				sound_replacement.player -= 1;
				if (sound_replacements_count < SoundReplacementCap) {
					sound_replacements[sound_replacements_count++] = sound_replacement;
				}
			}
		// Gameplay Actions
		} else if (parser_eq(&parser, "clear_creep")) {
			Box32 location = parser_get_area(&parser);
			scbw::clear_creep(location);
		} else if (parser_eq(&parser, "spawn")) {
			SpawnSequence sequence = {};
			sequence.count = parser_get_s32(&parser);
			sequence.unit_id = parser_get_unit_id(&parser);
			sequence.player = parser_get_player(&parser, true);
			sequence.type = parser_get_enum(&parser, spawn_type_names, SpawnType_Count);
			sequence.delay = parser_get_time(&parser) * 24;
			sequence.stagger = parser_get_time(&parser) * 24;
			sequence.source_pos = parser_get_position(&parser);
			sequence.move_pos = parser_get_position(&parser);
			
			if (sequence.player) {
				sequence.player -= 1;
				if (spawn_sequence_count < SpawnSequenceCap) {
					spawn_sequences[spawn_sequence_count++] = sequence;
				}
			}
		} else if (parser_eq(&parser, "placeholders_enable")) {
			Box32 area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset units_ids = parser_get_unit_id_bitset(&parser);
			s32 enable = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				Layouts* layouts = &player_layouts[i];
				for (u32 j = 0; j < PlaceholderLimit; ++j) {
					Placeholder* placeholder = &layouts->placeholders[j];
					if (!units_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (placeholder->position.x < area.x || placeholder->position.x > area.x + area.w &&
						placeholder->position.y < area.y || placeholder->position.y > area.y + area.h) {
						continue;
					}
					placeholder->disabled = !enable;
				}
			}
		} else if (parser_eq(&parser, "placeholders_priority_add")) {
			Box32 area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset units_ids = parser_get_unit_id_bitset(&parser);
			s32 priority = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				Layouts* layouts = &player_layouts[i];
				bool changed = false;
				for (u32 j = 0; j < PlaceholderLimit; ++j) {
					Placeholder* placeholder = &layouts->placeholders[j];
					if (!units_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (placeholder->position.x < area.x || placeholder->position.x > area.x + area.w &&
						placeholder->position.y < area.y || placeholder->position.y > area.y + area.h) {
						continue;
					}
					placeholder->priority = CLAMP(placeholder->priority + priority, 0, 255);
					changed = true;
				}
				if (changed) {
					sort_layouts(layouts);
				}
			}
		} else if (parser_eq(&parser, "placeholders_priority_set")) {
			Box32 area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset units_ids = parser_get_unit_id_bitset(&parser);
			s32 priority = parser_get_s32(&parser);

			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				Layouts* layouts = &player_layouts[i];
				bool changed = false;
				for (u32 j = 0; j < PlaceholderLimit; ++j) {
					Placeholder* placeholder = &layouts->placeholders[j];
					if (!units_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (placeholder->position.x < area.x || placeholder->position.x > area.x + area.w &&
						placeholder->position.y < area.y || placeholder->position.y > area.y + area.h) {
						continue;
					}
					placeholder->priority = CLAMP(priority, 0, 255);
					changed = true;
				}
				if (changed) {
					sort_layouts(layouts);
				}
			}

		}
	}

	return true;
}