#pragma once
#include "types.h"

#include <thread>
#include <mutex>

#pragma pack(push, 8)
#include <discord_game_sdk.h>
#pragma pack(pop)

enum class EDiscordState {
	None,
	InMenu,
	InGamePlaying,
	InGameWitness,
	InReplay,
};

class DiscordState {
public:
	DiscordState() {}
	explicit DiscordState(bool in_game);
	bool operator==(const DiscordState&) const = default;

	DiscordActivity make_discord_activity(u32 timestamp) const;

private:
	bool m_in_game;
	EDiscordState m_state;
	RaceId::Enum m_race;
	GameKind m_game_kind;
	std::string m_map_name; // Note: This has to be std::string, since we want to cache a copy, if we just cache a pointer to char*, that can and will be modified
	EDiscordActivityType m_activity_type;
};

class Discord {
public:
	Discord();
	~Discord();
	void update_status_if_needed(bool in_game);

private:
	bool init();
	void update_loop();

private:
	bool m_running = true;
	std::atomic<bool> m_should_update_status;
	std::mutex m_mutex;
	std::thread m_thread;

	IDiscordCore* m_core;

	DiscordState m_current_state;
	bool m_was_in_game;
	u32 m_timestamp;
};

inline Discord g_discord;
