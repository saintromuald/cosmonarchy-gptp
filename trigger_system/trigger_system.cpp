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

bool has_custom_name[PlayerLimit];
u32 trigger_kills[PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];
u32 trigger_combat[PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];

static void init_trigger_system() {
	trigger_system_init = true;
	g_action_table[47] = (ActionPointer)parse_comment_action;

	memset(has_custom_name, 0, sizeof(has_custom_name));
	for (u32 i = 0; i < PlayerLimit; ++i) {
		for (u32 j = 0; j < UnitId::Count; ++j) {
			for (u32 k = 0; k < PlayerLimit; ++k) {
				for (u32 l = 0; l < UnitId::Count; ++l) {
					trigger_kills [i][j][k][l] = 0;
					trigger_combat[i][j][k][l] = UINT_MAX;
				}
			}
		}
	}

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

		for (u32 i = 0; i < PlayerLimit; ++i) {
			for (u32 j = 0; j < UnitId::Count; ++j) {
				for (u32 k = 0; k < PlayerLimit; ++k) {
					for (u32 l = 0; l < UnitId::Count; ++l) {
						if (trigger_combat[i][j][k][l] != UINT_MAX) {
							trigger_combat[i][j][k][l]++;
						}
					}
				}
			}
		}
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
				if (parser_eq(&parser, "player_combat")) {
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					f32 time = parser_get_f32(&parser);

					for (u32 i = 0; i < PlayerLimit; ++i) {
						if (!source_players.set[i]) {
							continue;
						}
						for (u32 j = 0; j < UnitId::Count; ++j) {
							if (!source_unit_ids.set[j]) {
								continue;
							}
							for (u32 k = 0; k < PlayerLimit; ++k) {
								if (!target_players.set[k]) {
									continue;
								}
								for (u32 l = 0; l < UnitId::Count; ++l) {
									if (!target_unit_ids.set[l]) {
										continue;
									}
									if (trigger_combat[i][j][k][l] <= time * 24) {
										return true;
									}
								}
							}
						}
					}
					return false;
				} else if (parser_eq(&parser, "player_kills")) {
					PlayerBitset source_players = parser_get_player_bitset(&parser);
					UnitIdBitset source_unit_ids = parser_get_unit_id_bitset(&parser);
					PlayerBitset target_players = parser_get_player_bitset(&parser);
					UnitIdBitset target_unit_ids = parser_get_unit_id_bitset(&parser);
					u32 kills = parser_get_s32(&parser);

					u32 kill_count = 0;
					for (u32 i = 0; i < PlayerLimit; ++i) {
						if (!source_players.set[i]) {
							continue;
						}
						for (u32 j = 0; j < UnitId::Count; ++j) {
							if (!source_unit_ids.set[j]) {
								continue;
							}
							for (u32 k = 0; k < PlayerLimit; ++k) {
								if (!target_players.set[k]) {
									continue;
								}
								for (u32 l = 0; l < UnitId::Count; ++l) {
									if (!target_unit_ids.set[l]) {
										continue;
									}
									kill_count += trigger_kills[i][j][k][l];
								}
							}
						}
					}

					if (kill_count >= kills) {
						return true;
					}
					return false;
				} else if (parser_eq(&parser, "local_camera")) {
					Box32 area = parser_get_location(&parser);
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
			f32 fade_in = parser_get_f32(&parser);
			playback.fade_in = fade_in;
		} else if (parser_eq(&parser, "fade_out") && is_local_player) {
			f32 fade_out = parser_get_f32(&parser);
			playback.fade_out = fade_out;
		} else if (parser_eq(&parser, "volume") && is_local_player) {
			f32 volume = parser_get_f32(&parser);
			f32 time = parser_get_f32(&parser);
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
			s32 player = parser_get_s32(&parser);
			Token name = parser_get_string(&parser);
			if (player >= 0 && player < 8) {
				has_custom_name[player] = true;
				if (g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::HumanDefeated) {
					snprintf(g_player_name_list->player[player].player_name, sizeof(g_player_name_list->player[player].player_name),
						"%s (%s)", name.data, g_player_table[player].name);
				} else {
					snprintf(g_player_name_list->player[player].player_name, sizeof(g_player_name_list->player[player].player_name),
						"%s", name.data);
				}
			}
		} else if (parser_eq(&parser, "set_force_name") && is_local_player) {
			s32 player = parser_get_s32(&parser);
			Token name = parser_get_string(&parser);
			if (player >= 0 && player < 8) {
				snprintf(g_player_name_list->player[player].force_name, sizeof(g_player_name_list->player[player].force_name),
					"%s", name.data);
			}
		} else if (parser_eq(&parser, "recolor") && is_local_player) {
			TimedRecolor recolor = {};
			recolor.set  = parser_get_recolor_set(&parser);
			recolor.time = parser_get_f32(&parser);

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
			recolor.time = parser_get_f32(&parser);

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
			s32 player = parser_get_s32(&parser);
			s32 value = parser_get_s32(&parser);
			log_placeholder_requests[player] = value;
		// Gameplay Actions
		} else if (parser_eq(&parser, "clear_creep")) {
			Box32 location = parser_get_location(&parser);
			scbw::clear_creep(location);
		} else if (parser_eq(&parser, "spawn")) {
			SpawnSequence sequence = {};
			sequence.count = parser_get_s32(&parser);
			sequence.unit_id = parser_get_unit_id(&parser);
			sequence.player = parser_get_player(&parser);
			sequence.type = parser_get_enum(&parser, spawn_type_names, SpawnType_Count);
			sequence.delay = parser_get_f32(&parser);
			sequence.stagger = parser_get_f32(&parser);
			sequence.source_pos = parser_get_position(&parser);
			sequence.move_pos = parser_get_position(&parser);
			
			if (spawn_sequence_count < SpawnSequenceCap) {
				spawn_sequences[spawn_sequence_count++] = sequence;
			}
		}
	}

	return true;
}