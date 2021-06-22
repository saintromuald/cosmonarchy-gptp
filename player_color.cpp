#include "player_color.h"

u8 tminimapColors[MINIMAP_COLOR_COUNT];
u8 tunitColors[UNIT_COLOR_COUNT];
u8 playerColorList[PLAYABLE_PLAYER_COUNT];

static std::vector<u32> GetPlayerTintsFromColor(u32 color);
static bool CanUsePlayerColor(u32 color, u32 last_player);
static bool CanUsePlayerTint(u32 color_tint, u32 last_player);
static u32 GetRandomTint();
static u32 GetRandomColorFromTint(u32 tint);
static u32 GetColorForPersonality(u32 player_id);
static void SetPlayerColor(u32 player_id, u32 color);

static std::vector<u32> GetPlayerTintsFromColor(u32 color)
{
	std::vector<u32> tints = {};
	for (u32 tint = 0; tint < playerTints.size(); ++tint)
	{
		for (u32 color_index = 0; color_index < playerTints[tint].size(); color_index++)
		{
			if (playerTints[tint][color_index] == color)
			{
				tints.push_back(tint);
			}
		}
	}
	return tints;
}

static bool CanUsePlayerColor(u32 color, u32 last_player)
{
	for (u32 player_tint : GetPlayerTintsFromColor(color))
	{
		if (!CanUsePlayerTint(player_tint, last_player))
		{
			return false;
		}
	}
	return true;
}

static bool CanUsePlayerTint(u32 colorTint, u32 lastPlayer)
{
	if (playerTable[lastPlayer].race < bannedRaceTints.size())
	{
		for (u32 banned_tint : bannedRaceTints[playerTable[lastPlayer].race])
		{
			if (colorTint == banned_tint)
			{
				return false;
			}
		}
	}
	if (*CURRENT_TILESET < bannedTilesetTints.size())
	{
		for (u32 banned_tint : bannedTilesetTints[*CURRENT_TILESET])
		{
			if (colorTint == banned_tint)
			{
				return false;
			}
		}
	}
	for (u32 player_id = 0; player_id < lastPlayer; ++player_id)
	{
		for (u32 other_tint : GetPlayerTintsFromColor(playerColorList[player_id]))
		{
			if (colorTint == other_tint)
			{
				return false;
			}
		}
	}
	return true;
}

static u32 GetRandomTint()
{
	return helper::randFunc() % PlayerTintRollCount;
}

static u32 GetRandomColorFromTint(u32 tint)
{
	return playerTints[tint][helper::randFunc() % playerTints[tint].size()];
}

static u32 GetColorForPersonality(u32 player_id)
{
	const u32 race = playerTable[player_id].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[player_id] - 1;

	if (build >= 0
		&& personalityColors.size() > race
		&& personalityColors[race].size() > build)
	{
		u32 size = personalityColors[race][build].size();
		u32 start_index = helper::randFunc() % size;
		u32 index = start_index;

		do
		{
			if (CanUsePlayerColor(personalityColors[race][build][index], player_id))
			{
				return personalityColors[race][build][index];
			}

			index = (index + 1) % size;
		} while(index != start_index);
	}

	return PlayerColor::Invalid;
}

static void SetPlayerColor(u32 player_id, u32 color)
{
	playerColorList[player_id] = color;
	playerMinimapColors[player_id] = tminimapColors[color + COLOR_SET_COUNT * *CURRENT_TILESET];
	for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		playerInGameColors[player_id * PLAYABLE_PLAYER_COUNT + i] = tunitColors[color * PLAYABLE_PLAYER_COUNT + i];
	}
}

static void load_color_tables()
{
	helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimapColors, MINIMAP_COLOR_COUNT, 0, 0, 0);
	helper::SBmpLoadImage("game\\tunit.pcx", 0, tunitColors, UNIT_COLOR_COUNT, 0, 0, 0);
}

bool check_radomization_validity()
{
	u32 banned_race_tints_count = 0;
	for (auto race : bannedRaceTints)
	{
		if (race.size() > banned_race_tints_count)
		{
			banned_race_tints_count = race.size();
		}
	}

	if (playerTints.size() - bannedTilesetTints[*CURRENT_TILESET].size() - banned_race_tints_count < PLAYABLE_PLAYER_COUNT)
	{
		scbw::print("Color Randomization Error:", GameTextColor::Red);
		scbw::print("\tNot enough tints available for current tileset", GameTextColor::Red);
		LoadDefaultPlayerColors();
		return false;
	}
	return true;
}

void randomize_color_for(u32 player_id)
{
	if (playerTable[player_id].type == PlayerType::Computer)
	{
		u32 color = GetColorForPersonality(player_id);
		if (color != PlayerColor::Invalid)
		{
			SetPlayerColor(player_id, color);
			return;
		}
	}

	u32 tint = GetRandomTint();
	while (!CanUsePlayerTint(tint, player_id))
	{
		tint = GetRandomTint();
	}
	SetPlayerColor(player_id, GetRandomColorFromTint(tint));
}

void match_color_of(u32 player_id)
{
	for (u32 color = 0; color < COLOR_SET_COUNT; ++color)
	{
		bool match = true;
		for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; ++i)
		{
			if (playerInGameColors[player_id * PLAYABLE_PLAYER_COUNT + i] != tunitColors[color * PLAYABLE_PLAYER_COUNT + i])
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			SetPlayerColor(player_id, color);
			break;
		}
	}
}

void LoadMeleePlayerColors()
{
	if (!check_radomization_validity())
	{
		LoadDefaultPlayerColors();
		return;
	}

	load_color_tables();
	for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; player_id++)
	{
		randomize_color_for(player_id);
	}
}

void LoadDefaultPlayerColors()
{
	load_color_tables();
	bool can_random = check_radomization_validity();
	for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id)
	{
		if (can_random && deathTable[UnitId::ResourceVespeneGeyser].player[player_id] & VespeneDeaths_RandomizeColor)
		{
			randomize_color_for(player_id);
		}
		else
		{
			match_color_of(player_id);
		}
	}
}