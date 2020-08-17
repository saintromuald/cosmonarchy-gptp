#pragma once

namespace hooks {
	void injectMaxHpHooks();
	u32 maxhp(int id, int playerId);
	u32 maxshields(int id, int playerId);
}