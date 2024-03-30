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
	void update(bool in_game);

private:
	void init();
	void update_thread();

private:
	bool m_running = true;
	std::thread m_thread;
	std::mutex m_mutex;

	u32 m_init_timeout;
	IDiscordCore* m_core;

	DiscordState m_current_state;
	bool m_was_in_game;
	u32 m_timestamp;
};

inline Discord g_discord;
