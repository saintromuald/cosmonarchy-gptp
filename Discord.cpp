#include "Discord.h"

#define NOMINMAX
#include <Windows.h>

#include <BW\api.h>
#include <trigger_system\trigger_system.h>

#define DiscordClientId 972072419013558326

DiscordState::DiscordState(bool in_game) : m_in_game(in_game) {
	if (in_game) {
		if (*g_is_in_replay) {
			m_state = EDiscordState::InReplay;
			m_activity_type = DiscordActivityType_Watching;
		} else if (g_globals->players[*g_local_nation_id].witness_mode) {
			m_state = EDiscordState::InGameWitness;
			m_activity_type = DiscordActivityType_Watching;
		} else {
			m_state = EDiscordState::InGamePlaying;
			m_activity_type = DiscordActivityType_Playing;
		}
	} else {
		m_state = EDiscordState::InMenu;
		m_activity_type = DiscordActivityType_Playing;
	}
	m_map_name = g_current_map_name;
	m_race = (RaceId::Enum)g_player_table[*g_local_nation_id].race;
	m_game_kind = bw::get_game_kind();
}

DiscordActivity DiscordState::make_discord_activity(u32 timestamp) const {
	// Robustness: Do not read any globals in this function, cache them inside of the constructor instead
	DiscordActivity activity = {};
	activity.type = m_activity_type;
	sprintf_s(activity.name, "Cosmonarchy");
	sprintf_s(activity.assets.large_text, "Cosmonarchy");
	sprintf_s(activity.assets.large_image, "cmbw");
	activity.timestamps.start = timestamp;
	if (m_in_game) {
		const char* game_kind = game_kind_to_string(m_game_kind);
		sprintf_s(activity.details, "%s (%s)", m_map_name.c_str(), game_kind);
		switch (m_state) {
			case EDiscordState::InGamePlaying: {
				sprintf_s(activity.state, "In Game (%s)", race_to_string(m_race));
			} break;
			case EDiscordState::InGameWitness: {
				sprintf_s(activity.state, "In Game (Witness)");
			} break;
			case EDiscordState::InReplay: {
				sprintf_s(activity.state, "In Replay");
			} break;
		}
		activity.instance = true;
	} else {
		sprintf_s(activity.details, "");
		sprintf_s(activity.state, "In Menu");
		activity.instance = false;
	}
	return activity;
}

Discord::Discord() : m_thread(&Discord::update_loop, this) {
	update_status_if_needed(false);
}

Discord::~Discord() {
	m_running = false;
	m_thread.join();
	if (m_core) {
		m_core->destroy(m_core);
		m_core = 0;
	}
}

bool Discord::init() {
	if (m_core) {
		return true;
	}

	static HMODULE discord_dll;
	if (!discord_dll) {
		discord_dll = LoadLibraryA("discord_game_sdk.dll");
		if (!discord_dll) {
			return false;
		}
	}

	typedef enum EDiscordResult discord_create_t(DiscordVersion version, struct DiscordCreateParams* params, struct IDiscordCore** result);
	static discord_create_t* discord_create;
	if (!discord_create) {
		discord_create = (discord_create_t*)GetProcAddress(discord_dll, "DiscordCreate");
		if (!discord_create) {
			return false;
		}
	}

	DiscordCreateParams params = {};
	params.client_id = DiscordClientId;
	params.flags = DiscordCreateFlags_NoRequireDiscord;
	EDiscordResult result = discord_create(DISCORD_VERSION, &params, &m_core);
	if (result != DiscordResult_Ok) {
		return false;
	}

	m_timestamp = time(0);
	return true;
}

void Discord::update_status_if_needed(bool in_game) {
	if (!m_core) {
		return;
	}

	if (m_was_in_game != in_game) {
		m_was_in_game = in_game;
		m_timestamp = time(0);
	}

	DiscordState state(in_game);
	if (m_current_state != state) {
		std::lock_guard lock{m_mutex};
		m_should_update_status = true;
		m_current_state = state;
	}
}

void Discord::update_loop() {
	while (m_running) {
		if (!m_core) {
			if (!init()) {
				std::this_thread::sleep_for(std::chrono::seconds(60));
				continue;
			}
		}

		if (m_should_update_status) {
			std::lock_guard lock{m_mutex};
			IDiscordActivityManager* activity_manager = m_core->get_activity_manager(m_core);
			if (activity_manager) {
				DiscordActivity activity = m_current_state.make_discord_activity(m_timestamp);
				activity_manager->update_activity(activity_manager, &activity, 0, 0);
			}
		}

		m_core->run_callbacks(m_core);
	}
}