#include "trigger_system.h"
#include <limits.h>
#include <time.h>

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
#include "config.h"

#include "hooks/harvest.h"
#include "graphics/graphics.h"

bool trigger_game_paused;
bool trigger_system_init;
bool trigger_system_in_game;

u32 sound_replacements_count;
SoundReplacement sound_replacements[SoundReplacementCap];

u32 sprite_replacements_count;
SpriteReplacement sprite_replacements[SpriteReplacementCap];

Token custom_unit_names[PlayerLimit][UnitId::Count];

Globals globals;

static void init_auto_celebration() {
	if (!config.celebrations_enabled) {
		return;
	}

	time_t now = time(0);
	tm* ltm = localtime(&now);
	u32 month = ltm->tm_mon + 1;
	switch (month) {
		case 4: {
			globals.celebration = Celebration_AprilFools;
		} break;
		case 12: {
			globals.celebration = Celebration_Christmas;
		} break;
		default: {
			globals.celebration = Celebration_None;
		} break;
	}
}

AdvisorType get_advisor_type_from_config(u32 race) {
	std::vector<AdvisorType> advisors = config.common_advisors.types; // Note this is a copy
	switch (race) {
		case RaceId::Terran: {
			advisors.insert(advisors.end(), config.terran_advisors.types.begin(), config.terran_advisors.types.end());
		} break;
		case RaceId::Protoss: {
			advisors.insert(advisors.end(), config.protoss_advisors.types.begin(), config.protoss_advisors.types.end());
		} break;
		case RaceId::Zerg: {
			advisors.insert(advisors.end(), config.zerg_advisors.types.begin(), config.zerg_advisors.types.end());
		} break;
	}
	if (advisors.size()) {
		u32 index = scbw::rand_between_local(0, advisors.size() - 1);
		return advisors[index];
	}
	return AdvisorType_Default;
}

void init_trigger_system() {
	trigger_system_init = true;
	trigger_game_paused = false;
	g_action_table[47] = (ActionPointer)parse_comment_action;

	sound_replacements_count = 0;
	memset(sound_replacements, 0, sizeof(sound_replacements));

	sprite_replacements_count = 0;
	memset(sprite_replacements, 0, sizeof(sprite_replacements));

	memset(custom_unit_names, 0, sizeof(custom_unit_names));
	memset(&globals, 0, sizeof(globals));

	init_auto_celebration();
	init_recolors();
	init_objectives();
	init_sound();
	init_spawn();
	init_trigger_vars();
	init_trigger_events();
	
	globals.current_advisor = get_advisor_type_from_config(g_player_table[*g_local_nation_id].race);
}

static bool unit_can_change_sprite(Unit* unit) {
	if (!unit->sprite || !unit->sprite->main_graphic) {
		return false;
	}
	u32 flingy_id = units_dat::graphic[unit->id];
	u32 sprite_id = flingy_dat::sprite_id[flingy_id];
	if (unit->sprite->id != sprite_id) {
		return false;
	}
	u32 image_id = sprites_dat::image_id[sprite_id];
	UnitExt* unit_ext = get_unit_ext(unit);
	if (unit->sprite->main_graphic->id != image_id &&
		(!unit_ext->values[ValueCustomImage] || unit->sprite->main_graphic->id != unit_ext->values[ValueCustomImage] - 1)) {
		return false;
	}
	return true;
}

static u32 unit_get_desired_sprite_id(Unit* unit) {
	for (u32 i = 0; i < sprite_replacements_count; ++i) {
		SpriteReplacement* sprite_replacement = &sprite_replacements[i];
		if (unit->player > PlayerLimit || !sprite_replacement->players.set[unit->player]) {
			continue;
		}
		if (unit->id >= UnitId::Count || !sprite_replacement->unit_ids.set[unit->id]) {
			continue;
		}
		return sprite_replacement->sprite_id;
	}

	switch (globals.celebration) {
		case Celebration_AprilFools:
		{
			switch (unit->id) {
				case UnitId::ZergQuazilisk:					return SpriteId::AprilFools_Quazilisk;
			}

			// TODO: Sounds
			{
			//	SoundReplacement sound_replacement = {};
			//	sound_replacement.any_animation = true;
			//	sound_replacement.any_sound = true;
			//	sound_replacement.replace = false;
			}
		} break;
		case Celebration_Christmas:
		{
			switch (unit->id) {
				case UnitId::ResourceMineralField:		return SpriteId::Christmas_MineralField1;
				case UnitId::ResourceMineralFieldType2:	return SpriteId::Christmas_MineralField2;
				case UnitId::ResourceMineralFieldType3:	return SpriteId::Christmas_MineralField3;
				case UnitId::TerranMinistry:			return SpriteId::Christmas_CommandCenter;
				case UnitId::ProtossNexus:				return SpriteId::Christmas_Nexus;
				case UnitId::ZergHachirosk:				return SpriteId::Christmas_Hachirosk;
				case UnitId::ZergCaphrolosk:			return SpriteId::Christmas_Caphrolosk;
				case UnitId::ZergGathtelosk:			return SpriteId::Christmas_Gathtelosk;
			}
		} break;
	}

	u32 flingy_id = units_dat::graphic[unit->id];
	u32 sprite_id = flingy_dat::sprite_id[flingy_id];
	return sprite_id;
}

static void replace_sprite(Unit* unit, u32 sprite_id) {
	u32 image_id = sprites_dat::image_id[sprite_id];
	if (!unit->sprite || !unit->sprite->main_graphic || unit->sprite->main_graphic->id == image_id) {
		return;
	}

	// This shouldn't be needed, since `unit_can_change_sprite` should protect us already and avoid visual flicker
	// Unfortunately without this shitty hardcode nexus never warps
	if (!(unit->status & UnitStatus::Completed)) {
		return;
	}

	// Remove old shadow, since the new image will create it's own
	// Would be better to do it only after we know new_image is created, but then the other shadow also gets removed
	// As of right now shadow would disappear if the main image replacement fails
	Image* next_image = 0;
	for (Image* image = unit->sprite->images.head; image; image = next_image) {
		next_image = image->link.next;
		if (image->palette_type == PaletteType::RLE_SHADOW) {
			image->free();
		}
	}

	// Replace main image
	Image* new_image = unit->sprite->create_overlay(image_id, 0, 0, unit->sprite->main_graphic->direction);
	if (new_image) {
		new_image->play_iscript_animation((IscriptAnimation::Enum)unit->sprite->main_graphic->animation);
		unit->sprite->main_graphic->free();
		unit->sprite->main_graphic = new_image;

		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->values[ValueCustomImage] = image_id + 1;
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
		tick_objectives();

		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (!unit_can_change_sprite(unit)) {
				continue;
			}
			u32 sprite_id = unit_get_desired_sprite_id(unit);
			replace_sprite(unit, sprite_id);
		}

		if (globals.alert.own_cooldown) {
			globals.alert.own_cooldown--;
		}
		if (globals.alert.ally_cooldown) {
			globals.alert.ally_cooldown--;
		}
		if (globals.alert.play_timer) {
			globals.alert.play_timer--;
			if (!globals.alert.play_timer) {
				helper::play_sound_from_unit(globals.alert.sound_id, 0);
			}
		}
	}
	trigger_game_paused = paused;
}

b32 parse_comment_condition(TriggerAction* action) {
	u32 trigger_player = *g_current_trigger_player_id;
	bool is_local_player = trigger_player == *g_local_nation_id;
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
						if (event->source_player >= FullPlayerLimit || event->target_player >= FullPlayerLimit) {
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
						if (event->source_player >= FullPlayerLimit || !source_players.set[event->source_player]) {
							continue;
						}
						if (!source_unit_ids.set[event->source_unit_id]) {
							continue;
						}
						if (event->target_player >= FullPlayerLimit || !target_players.set[event->target_player]) {
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
						if (event->target_player >= FullPlayerLimit || !players.set[event->target_player]) {
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

					EntryArray units = {};
					if (scouting_player) {
						seen_unit_search_rect(&units, scouting_player - 1, area.box);
					} else {
						unit_search_rect(&units, area.box);
					}

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
						if (unit->player >= FullPlayerLimit || !players.set[unit->player]) {
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

					if (center.x >= area.box.left && center.x <= area.box.right &&
						center.y >= area.box.top  && center.y <= area.box.bottom) {
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
					QuotaType quota_type = parser_get_quota_type(&parser, QuotaType_Equal);
					TriggerVar* var = parser_get_char(&parser, '=') ? find_or_create_trigger_var(var_type, name.data) : find_trigger_var(var_type, name.data);
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
	u32 trigger_player = *g_current_trigger_player_id;
	bool is_local_player = trigger_player == *g_local_nation_id;
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
				if (!ignore_human_name && (g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::HumanDefeated)) {
					FullName buffer = {};
					snprintf(buffer.data, FullNameCap, "%s (%s)", name.data, g_player_table[player].name);
					set_player_name(player, buffer.data, true);
				} else {
					set_player_name(player, name.data, true);
				}
			}
		} else if (parser_eq(&parser, "set_force_name") && is_local_player) {
			u32 player = parser_get_player(&parser);
			Token name = parser_get_string(&parser);
			if (player) {
				player -= 1;
				set_player_force_name(player, name.data);
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
		} else if (parser_eq(&parser, "objective_set") && is_local_player) {
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
		} else if (parser_eq(&parser, "objective_timer") && is_local_player) {
			s32 key = parser_get_s32(&parser);
			u32 time = parser_get_time_ticks(&parser);

			Objective* objective = find_or_create_objective(key);
			if (objective) {
				objective->timer = time;
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

			parser_get_word(&parser);
			if (parser_eq(&parser, "clear")) {
				for (u32 i = 0; i < sound_replacements_count; ++i) {
					if (memcmp(&sound_replacement, &sound_replacements[i], sizeof(SoundReplacement)) == 0) {
						sound_replacements[i--] = sound_replacements[--sound_replacements_count];
					}
				}
			} else {
				if (sound_replacements_count < SoundReplacementCap) {
					sound_replacements[sound_replacements_count++] = sound_replacement;
				}
			}
		// Bitset Local Only Triggers
		} else if (parser_eq(&parser, "play_sound") && is_local_player) {
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
					if (placeholder->unit_id == UnitId::TownCenterMarker) {
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

			if (priority) {
				for (u32 i = 0; i < PlayerLimit; ++i) {
					if (!players.set[i]) {
						continue;
					}
					AiLayouts* layouts = &player_layouts[i];
					for (u32 j = 0; j < layouts->placeholders_count; ++j) {
						AiPlaceholder* placeholder = &layouts->placeholders[j];
						if (placeholder->unit_id == UnitId::TownCenterMarker) {
							continue;
						}
						if (!unit_ids.set[placeholder->unit_id]) {
							continue;
						}
						if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
							continue;
						}
						placeholder->priority = CLAMP(placeholder->priority + priority, 0, 255);
						layouts->should_sort = true;
					}
				}
			}
		} else if (parser_eq(&parser, "layouts_priority_set")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 priority = parser_get_s32(&parser);

			u8 new_priority = CLAMP(priority, 0, 255);
			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				AiLayouts* layouts = &player_layouts[i];
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (placeholder->unit_id == UnitId::TownCenterMarker) {
						continue;
					}
					if (placeholder->unit_id >= UnitId::Count || !unit_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
						continue;
					}
					if (placeholder->priority != new_priority) {
						placeholder->priority = new_priority;
						layouts->should_sort = true;
					}
				}
			}
		} else if (parser_eq(&parser, "layouts_assign")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			Point16 resarea_position = parser_get_position(&parser, box_center(area.box));

			u32 resarea = scbw::get_closest_resarea(resarea_position);
			for (u32 i = 0; i < PlayerLimit; ++i) {
				if (!players.set[i]) {
					continue;
				}
				AiLayouts* layouts = &player_layouts[i];
				for (u32 j = 0; j < layouts->placeholders_count; ++j) {
					AiPlaceholder* placeholder = &layouts->placeholders[j];
					if (placeholder->unit_id == UnitId::TownCenterMarker) {
						continue;
					}
					if (placeholder->unit_id >= UnitId::Count || !unit_ids.set[placeholder->unit_id]) {
						continue;
					}
					if (!check_area(area, placeholder->position, units_dat::base_property[placeholder->unit_id] & UnitProperty::Flyer)) {
						continue;
					}
					if (placeholder->resarea != resarea) {
						placeholder->resarea = resarea;
						layouts->should_sort = true;
					}
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
				if (player == *g_local_nation_id) {
					globals.current_advisor = get_advisor_type_from_config(race);
				}
			}
		} else if (parser_eq(&parser, "set_player_color")) {
			u32 player = parser_get_player(&parser);
			u32 player_color = parser_get_enum(&parser, player_color_names, ARRAY_SIZE(player_color_names));

			if (player) {
				player -= 1;
				set_player_color(player, player_color);
			}
		} else if (parser_eq(&parser, "set_ai_auto_burrow")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 enable = parser_get_s32(&parser);

			EntryArray units = {};
			unit_search_rect(&units, area.box);
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
		} else if (parser_eq(&parser, "set_celebration")) {
			parser_get_word(&parser);
			if (parser_eqi(&parser, "auto")) {
				init_auto_celebration();
			} else if (parser_eqi(&parser, "none")) {
				globals.celebration = Celebration_None;
			} else if (parser_eqi(&parser, "april_fools")) {
				globals.celebration = Celebration_AprilFools;
			} else if (parser_eqi(&parser, "christmas")) {
				globals.celebration = Celebration_Christmas;
			}
		} else if (parser_eq(&parser, "spawn_cargo")) {
			Area area = parser_get_area(&parser);
			PlayerBitset players = parser_get_player_bitset(&parser);
			UnitIdBitset unit_ids = parser_get_unit_id_bitset(&parser);
			s32 count = parser_get_s32(&parser, 0);
			u32 unit_id = parser_get_unit_id(&parser);
			s32 player = parser_get_s32(&parser, 0);
			bool force = parser_get_s32(&parser, 0);

			EntryArray units = {};
			unit_search_rect(&units, area.box);
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
				for (u32 j = 0; j < count; ++j) {
					Unit* cargo_unit = scbw::create_unit_in_game(unit_id, player ? player : unit->player, 0, 0);
					if (cargo_unit) {
						if (force ? !scbw::transport_is_full(unit) : scbw::can_be_entered_by(unit, cargo_unit)) {
							helper::load_unit_proc(unit, cargo_unit);
						} else {
							cargo_unit->user_action_flags |= 0x4;
							cargo_unit->remove();
							break;
						}
					}
				}
			}
		} else if (parser_eq(&parser, "witness_mode")) {
			WitnessMode new_value = (WitnessMode)parser_get_s32(&parser);

			PlayerVars* player_vars = &globals.players[trigger_player];
			WitnessMode old_value = player_vars->witness_mode;
			if (old_value != new_value) {
				globals.players[trigger_player].witness_mode = new_value;
				graphics::reset_stat_res_graphics(); // Must call this to reset custom graphics data for statres display. -Veeq7
				if (new_value == WitnessMode_Pure) {
					// Hide Pure Witnesses from scoreboard
					g_player_name_list->player[trigger_player].type = 0;
					*g_score_screen_player_count -= 1;
				} else if (old_value == WitnessMode_Pure) {
					// Unhide from scoreboard
					g_player_name_list->player[trigger_player].type = g_player_table[trigger_player].type; // This stores the old player type, resets to CPU or human, etc. -Pr0nogo
					*g_score_screen_player_count += 1;
				}

				if (new_value && *g_elapsed_time_frames == 0 && trigger_player == *g_local_nation_id) {
					u32 first_player = 0;
					for (u32 i = 0; i < PlayerLimit; ++i) {
						if (globals.players[i].witness_mode) continue;
						if (g_player_table[i].type == PlayerType::Human) {
							first_player = i + 1;
							break;
						}
					}
					if (!first_player) {
						for (u32 i = 0; i < PlayerLimit; ++i) {
							if (globals.players[i].witness_mode) continue;
							if (g_player_table[i].type == PlayerType::Computer) {
								first_player = i + 1;
								break;
							}
						}
					}
					if (first_player) {
						first_player -= 1;
						Point16 pos = g_start_positions[first_player];
						pos.x = pos.x > 320 ? pos.x - 320 : 0;
						pos.y = pos.y > 200 ? pos.y - 200 : 0;
						helper::move_screen(pos);
					}
				}
			}
		} else if (parser_eq(&parser, "set_faction")) {
			u32 player = parser_get_player(&parser);
			Faction faction = (Faction)parser_get_enum(&parser, faction_symbol_names, ARRAY_SIZE(faction_symbol_names));

			if (player) {
				player -= 1;
				globals.players[player].faction = faction;
				globals.players[player].faction_was_set = true;
			}
		} else if (parser_eq(&parser, "set_advisor")) {
			u32 player = parser_get_player(&parser);
			AdvisorType advisor = (AdvisorType)parser_get_enum(&parser, advisor_symbol_names, ARRAY_SIZE(advisor_symbol_names));

			if (player) {
				player -= 1;
				if (player == *g_local_nation_id) {
					globals.current_advisor = advisor;
				}
			}
		} else if (parser_eq(&parser, "neutral_hostile")) {
			bool neutral_hostile = (bool)parser_get_s32(&parser);
			PlayerVars* player_vars = &globals.players[trigger_player];
			if (player_vars->neutral_hostile != neutral_hostile) {
				player_vars->neutral_hostile = (bool)neutral_hostile;
			}
		}
	}
	return true;
}