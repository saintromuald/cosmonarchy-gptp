#pragma once
#include "types.h"

#pragma pack(push, 8)
#include <discord_game_sdk.h>
#pragma pack(pop)

enum class DiscordState {
	None,
	InMenu,
	InGamePlaying,
	InGameWitness,
	InReplay,
};

class Discord {
public:
	void update(bool in_game);

	inline static Discord& instance() {
		static Discord instance = {};
		return instance;
	}

private:
	Discord() = default;
	void init();

private:
	u32 m_init_timeout;
	IDiscordCore* m_core;

	DiscordState m_state;
	bool m_was_in_game;
	u32 m_timestamp;
};
