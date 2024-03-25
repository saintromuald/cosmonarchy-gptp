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
		static Discord instance;
		return instance;
	}

private:
	Discord() = default;
	void init();

private:
	u32 m_init_timeout = 0;
	DiscordState m_state = DiscordState::None;
	IDiscordCore* m_core = 0;
};
