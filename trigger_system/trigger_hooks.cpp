#include "trigger_hooks.h"
#include "trigger_system.h"
#include "trigger_sound.h"
#include "hook_tools.h"
#include "hooks/limits/sfx_extender.h"

// HOOKS \\

static bool execute_trigger_condition(TriggerCondition* condition) {
	static b32 result;
	const void* address = g_trigger_condition_list[condition->condition_type];
	__asm {
		PUSHAD
		MOV ECX, condition
		CALL address
		MOV result, EAX
		POPAD
	}
	return result;
}

// Hooked to check custom conditions. -Pr0nogo
s32 execute_trigger_conditions(TriggerLink* trigger_link) { // 00489200
	for (u32 i = 0; i < 64; ++i) {
		TriggerAction* action = &trigger_link->trigger.actions[i];
		if (action->flags & 2) {
			continue;
		}

		if (action->action_type == 47 && !parse_comment_condition(action)) {
			return false;
		}
	}

	for (u32 i = 0; i < 16; ++i) {
		TriggerCondition* condition = &trigger_link->trigger.conditions[i];
		if (condition->flags & 2) {
			continue;
		}

		if (!condition->condition_type) {
			return true;
		}

		if (!execute_trigger_condition(condition)) {
			return false;
		}
	}
	return true;
}

static bool sound_replacement_match(SoundReplacement* sound_replacement, Image* image, u32 sound_id, Unit* unit) {
	return (sound_replacement->any_sound || sound_replacement->old_sound_id == sound_id)
		&& (sound_replacement->any_animation || sound_replacement->animation == image->animation)
		&& unit->id < UnitId::Count && sound_replacement->unit_ids[unit->id]
		&& unit->player < PlayerLimit && sound_replacement->players[unit->player];
}

static u32 valid_sound_replacement_count(Image* image, u32 sound_id, Unit* unit) {
	u32 count = 0;
	for (u32 i = 0; i < g_globals->sound_replacements.size(); ++i) {
		SoundReplacement* sound_replacement = &g_globals->sound_replacements[i];
		if (sound_replacement_match(sound_replacement, image, sound_id, unit)) {
			count += 1;
		}
	}
	return count;
}

void iscript_play_sound(Image* image, u32 sound_id) { // 004D6610
	Sprite* sprite = image->parent_sprite;
	SpriteExt* sprite_ext = get_sprite_ext(image->parent_sprite);
	Unit* unit = sprite_ext->owner;

	bool replace = false;
	if (unit && sprite->main_graphic == image) {
		u32 valid_count = valid_sound_replacement_count(image, sound_id, unit);
		if (valid_count) {
			u32 target_index = bw::rand_between_local(0, valid_count - 1);
			u32 index = 0;
			for (SoundReplacement& sound_replacement : g_globals->sound_replacements) {
				if (sound_replacement_match(&sound_replacement, image, sound_id, unit)) {
					if (index == target_index) {
						play_sound(sound_replacement.new_sound, get_sound_volume_at_pos(sprite->position), helper::get_sfx_pan_from_screen_distance(sprite->position.x));
						if (sound_replacement.replace) {
							replace = true;
						}
					} else {
						index++;
					}
				}
			}
		}
	}

	if (!replace) {
		helper::play_sound_at_pos(sound_id, sprite->position, 1, 0);
	}
}

// Hooked to extend sound channels. -Pr0nogo
s32 play_transmission_from_location(u32 sound_id, s32 volume, s32 pan, Unit* unit) { // 004BBFD0
	if (!g_direct_sound_interface->lpVtbl || !g_options_list->sfx) {
		return false;
	}
	
	if (sfx_dat::sound_file[sound_id]) {
		char buffer[MAX_PATH] = {};
		snprintf(buffer, MAX_PATH, "sound\\%s", sound_paths[sound_id]); // sfx_dat::sound_file[sound_id]);
		Sound* sound = get_sound(buffer);
		if (sound) {
			u32 flags = sfx_dat::sound_flags[sound_id];
			if (flags & DatSoundFlags_OneAtATime) {
				DWORD status = 0;
				if (sound->ds_buffer) {
					sound->ds_buffer->GetStatus(&status);
					if (status == DSBSTATUS_PLAYING) {
						return false;
					}
				}
			}

			if (flags & DatSoundFlags_UnitResponse && unit) {
				for (u32 i = 0; i < MaxDupChannels; ++i) {
					DupChannel* dup_channel = &dup_channels[i];
					if (!dup_channels[i].ds_buffer) {
						continue;
					}
					if (dup_channels[i].unit_id != unit->id) {
						continue;
					}
					DWORD status = 0;
					dup_channels[i].ds_buffer->GetStatus(&status);
					if (status == DSBSTATUS_PLAYING) {
						return false;
					}
				}
			}

			s32 unit_id = -1;
			if (flags & DatSoundFlags_UnitResponse && unit) {
				unit_id = unit->id;
			}
			if (play_sound(sound, volume, pan, unit_id)) {
				return true;
			}
		}
	}
	return false;
}

// Hooked to debug sound related crashes (caused by invalid sound data init in sfx data)
void free_sound_buffer(u32 sound_id) {
	SoundData* sound_data = &sfx_dat::sound_data[sound_id];
	IDirectSoundBuffer* ds_buffer = sound_data->ds_buffer;
	if (!ds_buffer) {
		return;
	}
	for (u32 i = 0; i < 8; ++i) {
		if (g_sound_channels[i].ds_buffer == ds_buffer) {
			g_sound_channels[i].ds_buffer = 0;
		}
	}
	ds_buffer->Release();
	sound_data->ds_buffer = 0;
	if (sfx_dat::sound_flags[sound_id] & DatSoundFlags_PermanentlyLoaded) {
		return;
	}
	*g_current_sound_memory -= sound_data->wav_data_size;
}

// Hooked to stop endless tick sound in scoreboard and other issues
void stop_sounds_partial() {
	for (u32 i = 0; i < loaded_sounds_count; ++i) {
		Sound* sound = &loaded_sounds[i];
		if (sound->ds_buffer) {
			sound->ds_buffer->Stop();
		}
	}
	for (u32 i = 0; i < MaxDupChannels; ++i) {
		DupChannel* dup_channel = &dup_channels[i];
		if (dup_channel->ds_buffer) {
			dup_channel->ds_buffer->Stop();
		}
	}
}

u32 unit_is_at_height(Unit* unit, u32 height) { // 0045F8D0
	if (!unit) {
		return 0;
	}
	
	s32 ground_height = helper::get_ground_height_at_px(unit->position.y, unit->position.x);
	if (ground_height < 0) {
		ground_height = 0;
	} else if (ground_height >= 2) {
		ground_height = 3; // Incrementing here is intentional; matches with high elevation flag bit. -DF
	}

	++ground_height;
	if (unit->status & UnitStatus::InAir) {
		ground_height <<= 3;
	}
	return height & ground_height;
}

// Hooked to add support for factional and crig-based advisors. -Pr0nogo
u16 get_unit_portrait_override(u16 unit_id) { // 0045E320
	u32 portrait_id = units_dat::portrait_id[unit_id];
	u32 player = *g_local_nation_id;
	Unit* active_unit = *g_active_portrait_unit;
	if (active_unit) {
		player = active_unit->player;
	}
	portrait_id = get_player_portrait_id_override(portrait_id, player);

	if (strcmp(g_current_map_name, "Anad's Greatsword") == 0) {
		switch (portrait_id) {
			case PortraitId::Maverick:
			{
				portrait_id = PortraitId::AnadThyron;
			} break;
			case PortraitId::Legionnaire:
			{
				portrait_id = PortraitId::Patriarch;
			} break;
			case PortraitId::Apostle:
			{
				portrait_id = PortraitId::AdvisorZorthos;
			} break;
		}
	} else if (strcmp(g_current_map_name, "The Initiate") == 0) {
		switch (portrait_id) {
			case PortraitId::ProtossAdvisor:
			{
				portrait_id = PortraitId::Solarion;
			} break;
		}
	} else if (strcmp(g_current_map_name, "Autophagy") == 0) {
		switch (portrait_id) {
			case PortraitId::Scribe:
			{
				portrait_id = PortraitId::AdvisorZorthos;
			} break;
		}
	}

	return portrait_id;
}

// Hooked to add support for factional and crig-based advisors. -Pr0nogo
void game_talking_portrait(s32 x_pos, u32 time, u32 unit_id, s32 y_pos) { // 0045EDD0
	g_trigger_portrait_active = 1;
	helper::set_callback_timer(*g_portrait, 4, time, 0x0045E610);
	GameKind game_kind = bw::get_game_kind();
	u32 portrait_id = units_dat::portrait_id[unit_id];
	if (game_kind == GameKind_Scenario || game_kind == GameKind_FactionWars) {
		portrait_id = helper::get_unit_portrait_override(unit_id);
	}
	helper::display_update_portrait(portrait_id, 0, 2);
	helper::set_callback_timer(*g_portrait, 3, time, 0x0045EC40);
	if (x_pos != -1) {
		g_last_event_position->x = (x_pos - 320) & ((x_pos - 320 < 0) - 1);
		g_last_event_position->y = (y_pos - 200) & ((y_pos - 200 < 0) - 1);
	}
}

// Hooked to add support for factional and crig-based advisors. -Pr0nogo
void __fastcall show_portrait(u8 portrait_slot, u32 player, u16 unit_id) // 0046CF30
{
	BinDlg* ready_room_dlg = *g_ready_room_dialog;
	if (ready_room_dlg->control_type) {
		ready_room_dlg = ready_room_dlg->parent;
	}

	BinDlg* child = ready_room_dlg->child_dialog;
	BinDlg* final_dlg = 0;
	if (child) {
		u16 slot = portrait_slot + 15;
		while (child->index != slot) {
			child = child->next;
			if (!child) {
				goto exit_loop;
			}
		}
		final_dlg = child;
	} else {
	exit_loop:
		final_dlg = 0;
	}

	u16 portrait_id = helper::get_unit_portrait_override(unit_id);
	if (strcmp(g_current_map_name, "Anad's Greatsword") == 0) {
		switch (portrait_id) {
			case PortraitId::AdvisorOrionImperium:
			{
				portrait_id = PortraitId::HADRAL;
			} break;
			case PortraitId::Maverick:
			{
				portrait_id = PortraitId::AnadThyron;
			} break;
			case PortraitId::Zealot:
			{
				portrait_id = PortraitId::Patriarch;
			} break;
			case PortraitId::Ovileth:
			{
				portrait_id = PortraitId::AdvisorZorthos;
			} break;
		}
	} else if (strcmp(g_current_map_name, "Further Notice") == 0) {
		switch (portrait_id) {
			case PortraitId::Cyprian:
			{
				portrait_id = PortraitId::Hotshot;
			} break;
		}
	} else if (strcmp(g_current_map_name, "Autophagy") == 0) {
		switch (portrait_id) {
			case PortraitId::Scribe:
			{
				portrait_id = PortraitId::AdvisorZorthos;
			} break;
		}
	}

	PortraitData* portrait_data = final_dlg->portrait_data; // Ignoring nullcheck warning as it is accessed without nullchecks all over the codebase. -Pr0nogo
	if (portrait_data->smk) {
		helper::svid_play_end((u8)portrait_data->smk);
		portrait_data->smk = 0;
	}
	portrait_data->flags = 2;
	portrait_data->portrait_id = portrait_id;
	helper::start_unit_portrait(final_dlg);
}

// Hooked to add support for factional and crig-based advisors. -Pr0nogo
void do_unit_event_notification(Unit* unit, u8 event, u32 timer, bool ping, u32 message_id) { // 0045EFE0
	if (!g_trigger_portrait_active) {
		u32 portrait_id = helper::get_unit_portrait_override(helper::get_last_queue_slot_id(unit));
		helper::display_update_portrait(portrait_id, unit, 2);
		if (timer || message_id >= SoundIdLimit) {
			helper::set_callback_timer(*g_portrait, 3, timer, 0x0045EC40);
		} else {
			*g_notification_message_id = message_id;
			helper::set_callback_timer(*g_portrait, 3, 1000, 0x0045EF50);
		}
		if (unit && ping) {
			helper::set_last_event_position(unit->position.x, unit->position.y);
			unit->last_event_timer = 8;
			unit->last_event_color = event;
			helper::minimap_ping(unit->position.x, unit->position.y, event);
		}
	}
}

// Hooked to add support for crig-based portrait overrides. -Pr0nogo
void update_selected_unit_portrait() { // 0045EBC0
	if (g_portrait_redraw) {
		if (!*g_unk_portrait) {
			BinDlg* portrait_dlg = *g_portrait;
			Unit* portrait_unit = portrait_dlg->unit_user;
			Unit* active_unit = *g_active_portrait_unit;

			if (!active_unit) {
				portrait_dlg->unit_user = active_unit;
				portrait_dlg->graphic = -1;
				helper::hide_dialog(portrait_dlg);
				g_portrait_redraw = false;
				return;
			}

			u32 active_unit_id = helper::get_last_queue_slot_id(active_unit);
			if (portrait_unit != active_unit || portrait_unit && helper::get_last_queue_slot_id(portrait_unit) != active_unit_id) {
				helper::display_update_portrait(helper::get_unit_portrait_override(active_unit_id), active_unit, 1);
			}
		}
		g_portrait_redraw = false;
	}
}

// Hooked to enable crig-based briefing music. -Pr0nogo
void dialog_music_fade(u32 music_id) { // 004A6310
	if (!g_direct_sound_handle || !g_music_playing) {
		switch (music_id) {
			case MusicId_TRdyRoom: {
				if (!strcmp(g_current_map_name, "Anad's Greatsword")) {
					// Temporary, for mapcrap
					music_id = MusicId_None;
				}
			} break;
		}

		if (g_music_bank[music_id].flags) {
			u32 volume = g_music_volume;
			g_music_volume = -10000;
			helper::play_music(music_id);
			g_music_volume = volume;
			if (g_options_list->music) {
				g_big_music_volume = 4294963900;
				KillTimer(*g_main_window, 3);
				SetTimer(*g_main_window, 3, 0x14, (TIMERPROC)0x004A6030);
			}
		} else {
			helper::play_music(music_id);
		}
	}
}

// Hooked to investigate adding a rescue unit crig action. -Pr0nogo
void orders_rescue_passive(Unit* unit) // 004A1EF0
{
	if (!unit->main_order_timer) {
		Sprite* sprite = unit->sprite;
		unit->main_order_timer = 1;
		u32 pos_x = sprite->position.x;
		u32 pos_y = sprite->position.y;
		
		EntryArray units = {};
		unit_search_radial(&units, unit->sprite->position, 64);
		for (u32 i = 0; i < units.count; ++i) {
			Unit* unit = units.units[i];
			if (g_player_table[unit->player].type != PlayerType::Human) {
				entry_array_remove(&units, i--);
				continue;
			}
		}
		if (units.count) {
			bool is_valid_town_center = false;
			if (unit_id_is_town_center(unit->id)) {
				if (unit->id == UnitId::ZergHachirosk && unit->unit_is_training_or_morphing()) {
					is_valid_town_center = true;
				} else if (unit->status & (UnitStatus::Completed | UnitStatus::GroundedBuilding)) {
					is_valid_town_center = true;
				}
			}

			u32 idle_order = OrderId::Die;
			if (is_valid_town_center) {
				Unit* first_unit = first_player_unit->unit[unit->player];
				if (first_unit) {
					Unit* next_unit = unit;
					do
					{
						Sprite* first_sprite = first_unit->sprite;
						next_unit = first_unit->player_link.next;
						if (first_sprite && first_unit->main_order_id) {
							rescue_unit(first_unit, units.units[0]->player);
							next_unit = first_unit->player_link.next;
						}
						first_unit = next_unit;
					} while (next_unit);
				}
			} else {
				rescue_unit(unit, units.units[0]->player);
			}
			// Audiovisuals were here -- now handled in rescue_unit
			bw::refresh_console();
		}
		u32 temp_array = g_temp_units_list_arrays_counts_list_last_index[g_temp_units_list_arrays_counts_list_last_index[0]];
		--g_temp_units_list_arrays_counts_list_last_index[0];
		*g_temp_units_list_current_array_count = temp_array;
	}
}

// HOOK INJECTION \\

void __declspec(naked) execute_trigger_conditions_wrapper() { // 00489200
	static b32 result;
	static TriggerLink* trigger_vector;
	__asm {
		MOV trigger_vector, EAX
		PUSHAD
	}
	result = execute_trigger_conditions(trigger_vector);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

void __declspec(naked) iscript_play_sound_wrapper() { // 004D6610
	static Image* image;
	static u32 sound_id;
	__asm {
		MOV sound_id, EBX
		MOV image, EAX
		PUSHAD
	}
	iscript_play_sound(image, sound_id);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) play_transmission_from_location_wrapper() { // 004BBFD0
	static s32 result;
	static u32 sound_id;
	static s32 volume;
	static s32 pan;
	static Unit* unit;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x08]
		MOV volume, EAX
		MOV EAX, [EBP + 0x0C]
		MOV pan, EAX
		MOV EAX, [EBP + 0x10]
		MOV unit, EAX
		POP EAX
		MOV sound_id, EAX
		PUSHAD
	}
	result = play_transmission_from_location(sound_id, volume, pan, unit);
	__asm {
		POPAD
		MOV EAX, result
		MOV ESP, EBP
		POP EBP
		RETN 0xC
	}
}

void __declspec(naked) free_sound_buffer_wrapper() { // 004BB8E0
	static u32 sound_id;
	__asm {
		MOV sound_id, EBX
		PUSHAD
	}
	free_sound_buffer(sound_id);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) stop_sounds_partial_wrapper() { // 004BBF5C
	static const u32 func_address = 0x004015F0;
	__asm {
		PUSHAD
	}
	stop_sounds_partial();
	__asm {
		POPAD
		JMP func_address
	}
}

void __declspec(naked) pylon_placement_wrapper() { // 00446775
	static const u32 jz_address = 0x0044677B;
	static const u32 jmp_address = 0x00446800;
	static Unit* disabled_unit;
	static u32 player;
	static s32* region_id;
	__asm {
		MOV disabled_unit, EAX
		MOV player, ECX
		PUSH EAX
		LEA EAX, [EBP-0x14]
		MOV region_id, EAX
		POP EAX
		PUSHAD
	}
	if (disabled_unit) {
		__asm {
			POPAD
			JMP jz_address
		}
	}
	if (g_aiscript_controller[player].ai_flags.spread_creep) {
		helper::get_bunker_placement_position(player, region_id);
	}
	__asm {
		POPAD
		JMP jmp_address
	}
}

const u32 init_players_4A9DCB = 0x004A9DCB;
const u32 init_players_4A9DEE = 0x004A9DEE;
void __declspec(naked) team_game_check_wrapper() {
	__asm {
		PUSHAD
	}
	if (bw::is_team_game_map(g_current_map_name)) {
		game_data->teams_enabled = true;
	}

	// Team game
	if (game_data->teams_enabled) {
		static const u32 init_players_team_game = 0x004A9DCB;
		__asm {
			POPAD
			JMP init_players_team_game // this then calls "init_team_game_players"
		}
	}
	// or else...
	static const u32 init_players_normal = 0x004A9DEE;
	__asm {
		POPAD
		JMP init_players_normal
	}
}

void __declspec(naked) unit_is_at_height_wrapper() { // 0045F8D0
	static Unit* unit;
	static u32 height;
	static u32 result;
	__asm {
		MOV unit, EAX
		MOV height, ESI
		PUSHAD
	}
	result = unit_is_at_height(unit, height);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

void __declspec(naked) get_unit_portrait_override_wrapper() { // 0045E320
	static u16 unit_id;
	static u16 result;
	__asm {
		MOV unit_id, AX
		PUSHAD
	}
	result = get_unit_portrait_override(unit_id);
	__asm {
		POPAD
		MOV AX, result
		RETN
	}
}

void __declspec(naked) game_talking_portrait_wrapper() { // 0045EDD0
	static s32 x_pos;
	static u32 time;
	static u32 unit_id;
	static s32 y_pos;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x08]
		MOV unit_id, EAX
		MOV EAX, [EBP + 0x0C]
		MOV y_pos, EAX
		POP EAX
		MOV x_pos, EDI
		MOV time, ESI
		PUSHAD
	}
	game_talking_portrait(x_pos, time, unit_id, y_pos);
	__asm {
		POPAD
		MOV EAX, 1 // return value unused
		MOV ESP, EBP
		POP EBP
		RETN 0x08
	}
}

void __declspec(naked) do_unit_event_notification_wrapper() { // 0045EFE0
	static Unit* unit;
	static u8 event_arg;
	static u32 timer;
	static bool ping;
	static u32 message_id;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x08]
		MOV ping, AL
		MOV EAX, [EBP + 0x0C]
		MOV message_id, EAX
		POP EAX
		MOV timer, EDI
		MOV event_arg, BL
		MOV unit, EAX
		PUSHAD
	}
	do_unit_event_notification(unit, event_arg, timer, ping, message_id);
	__asm {
		POPAD
		MOV ESP, EBP
		POP EBP
		RETN 0x08
	}
}

void __declspec(naked) update_selected_unit_portrait_wrapper() { // 0045EBC0
	__asm {
		PUSHAD
	}
	update_selected_unit_portrait();
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) dialog_music_fade_wrapper() { // 004A6310
	static u32 music_id;
	__asm {
		MOV music_id, EAX
		PUSHAD
	}
	dialog_music_fade(music_id);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) orders_rescue_passive_wrapper() { // 004A6310
	static Unit* unit;
	__asm {
		MOV unit, EAX
		PUSHAD
	}
	orders_rescue_passive(unit);
	__asm {
		POPAD
		RETN
	}
}

const char* create_next_campaign_game_4DC1F7() {
	return game_kind_to_filename_string(bw::get_game_kind());
}

void inject_trigger_system_hooks() {
	jump_patch(execute_trigger_conditions_wrapper,			0x00489200, 1);
	jump_patch(iscript_play_sound_wrapper,					0x004D6610, 2);
	jump_patch(play_transmission_from_location_wrapper,		0x004BBFD0, 0);
	call_patch(stop_sounds_partial_wrapper,					0x004BBF5C, 0);
//	jump_patch(pylon_placement_wrapper,						0x00446775, 1);
//	jump_patch(free_sound_buffer_wrapper,					0x004BB8E0, 1);
	jump_patch(unit_is_at_height_wrapper,					0x0045F8D0, 2);
	jump_patch(get_unit_portrait_override_wrapper,			0x0045E320, 6);
	jump_patch(game_talking_portrait_wrapper,				0x0045EDD0, 4);
	jump_patch(show_portrait,								0x0046CF30, 3);
	jump_patch(do_unit_event_notification_wrapper,			0x0045EFE0, 1);
	jump_patch(update_selected_unit_portrait_wrapper,		0x0045EBC0, 0);
	jump_patch(dialog_music_fade_wrapper,					0x004A6310, 3);
	jump_patch(orders_rescue_passive_wrapper,				0x004A1EF0, 1);
}