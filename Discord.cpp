#include "Discord.h"

#define NOMINMAX
#include <Windows.h>

#include <BW\api.h>
#include <trigger_system\trigger_system.h>

#define DiscordClientId 972072419013558326

void Discord::init() {
	if (m_core) {
		return;
	}

	if (time(0) < m_init_timeout) {
		return;
	}

	static HMODULE discord_dll;
	if (!discord_dll) {
		discord_dll = LoadLibraryA("discord_game_sdk.dll");
		if (!discord_dll) {
			m_init_timeout = time(0) + 60; // Let's forget about discord for a while, if this fails
			return;
		}
	}

	typedef enum EDiscordResult discord_create_t(DiscordVersion version, struct DiscordCreateParams* params, struct IDiscordCore** result);
	static discord_create_t* discord_create;
	if (!discord_create) {
		discord_create = (discord_create_t*)GetProcAddress(discord_dll, "DiscordCreate");
		if (!discord_create) {
			m_init_timeout = time(0) + 60; // Let's forget about discord for a while, if this fails
			return;
		}
	}

	DiscordCreateParams params = {};
	params.client_id = DiscordClientId;
	params.flags = DiscordCreateFlags_NoRequireDiscord;
	EDiscordResult result = discord_create(DISCORD_VERSION, &params, &m_core);
	if (result != DiscordResult_Ok) {
		m_init_timeout = time(0) + 60; // Let's forget about discord for a while, if this fails
		return;
	}

	m_timestamp = time(0);
}

void Discord::update(bool in_game) {
	init();
	if (!m_core) {
		return;
	}

	EDiscordActivityType activity_type = DiscordActivityType_Playing;
	DiscordState discord_state = DiscordState::InMenu;
	if (in_game) {
		if (*g_is_in_replay) {
			discord_state = DiscordState::InReplay;
			activity_type = DiscordActivityType_Watching;
		} else if (globals->players[*g_local_nation_id].witness_mode) {
			discord_state = DiscordState::InGameWitness;
			activity_type = DiscordActivityType_Watching;
		} else {
			discord_state = DiscordState::InGamePlaying;
		}
	}

	if (m_was_in_game != in_game) {
		m_was_in_game = in_game;
		m_timestamp = time(0);
	}

	if (m_state != discord_state) {
		m_state = discord_state;

		IDiscordActivityManager* activity_manager = m_core->get_activity_manager(m_core);
		if (activity_manager) {
			DiscordActivity activity = {};
			activity.type = activity_type;
			sprintf_s(activity.name, "Cosmonarchy");
			sprintf_s(activity.assets.large_text, "Cosmonarchy");
			sprintf_s(activity.assets.large_image, "cmbw");
			activity.timestamps.start = m_timestamp;
			if (in_game) {
			//	const char* game_kind = game_kind_to_string(bw::get_game_kind());
			//	sprintf_s(activity.details, "%s (%s)", g_current_map_name, game_kind);
				sprintf_s(activity.details, "%s", g_current_map_name);
				switch (discord_state) {
					case DiscordState::InGamePlaying: {
					//	const char* race = race_to_string(g_player_table[*g_local_nation_id].race);
					//	sprintf_s(activity.state, "In Game (%s)", race);
						sprintf_s(activity.state, "In Game");
					} break;
					case DiscordState::InGameWitness: {
						sprintf_s(activity.state, "In Game (Witness)");
					} break;
					case DiscordState::InReplay: {
						sprintf_s(activity.state, "In Replay");
					} break;
				}
				activity.instance = true;
			} else {
				sprintf_s(activity.details, "");
				sprintf_s(activity.state, "In Menu");
				activity.instance = false;
			}
			activity_manager->update_activity(activity_manager, &activity, 0, 0);
		}
	}

	m_core->run_callbacks(m_core);
}
