#include "player_color.h"

u8 tminimapColors[MINIMAP_COLOR_COUNT];
u8 tunitColors[UNIT_COLOR_COUNT];
u8 playerColorList[PLAYABLE_PLAYER_COUNT];

static std::vector<u32> GetPlayerTintsFromColor(u32 color);
static bool CanUsePlayerColor(u32 color, u32 lastPlayer);
static bool CanUsePlayerTint(u32 colorTint, u32 lastPlayer);
static u32 GetRandomTint();
static u32 GetRandomColorFromTint(u32 tint);
static u32 GetColorForPersonality(u32 playerId);
static void SetPlayerColor(u32 playerId, u32 color);

static std::vector<u32> GetPlayerTintsFromColor(u32 color)
{
	std::vector<u32> tints = {};
	for (u32 tint = 0; tint < playerTints.size(); ++tint)
	{
		for (u32 colorIndex = 0; colorIndex < playerTints[tint].size(); colorIndex++)
		{
			if (playerTints[tint][colorIndex] == color)
			{
				tints.push_back(tint);
			}
		}
	}
	return tints;
}

static bool CanUsePlayerColor(u32 color, u32 lastPlayer)
{
	for (u32 playerTint : GetPlayerTintsFromColor(color))
	{
		if (!CanUsePlayerTint(playerTint, lastPlayer))
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
		for (u32 bannedTint : bannedRaceTints[playerTable[lastPlayer].race])
		{
			if (colorTint == bannedTint)
			{
				return false;
			}
		}
	}
	if (*CURRENT_TILESET < bannedTilesetTints.size())
	{
		for (u32 bannedTint : bannedTilesetTints[*CURRENT_TILESET])
		{
			if (colorTint == bannedTint)
			{
				return false;
			}
		}
	}
	for (u32 playerId = 0; playerId < lastPlayer; ++playerId)
	{
		for (u32 otherTint : GetPlayerTintsFromColor(playerColorList[playerId]))
		{
			if (colorTint == otherTint)
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

static u32 GetColorForPersonality(u32 playerId)
{
	const u32 race = playerTable[playerId].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[playerId] - 1;

	if (build >= 0
		&& personalityColors.size() > race
		&& personalityColors[race].size() > build)
	{
		u32 size = personalityColors[race][build].size();
		u32 startIndex = helper::randFunc() % size;
		u32 index = startIndex;

		do
		{
			if (CanUsePlayerColor(personalityColors[race][build][index], playerId))
			{
				return personalityColors[race][build][index];
			}

			index = (index + 1) % size;
		} while(index != startIndex);
	}

	return PlayerColor::Invalid;
}

static void SetPlayerColor(u32 playerId, u32 color)
{
	playerColorList[playerId] = color;
	playerMinimapColors[playerId] = tminimapColors[color + COLOR_SET_COUNT * *CURRENT_TILESET];
	for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
	{
		playerInGameColors[playerId * PLAYABLE_PLAYER_COUNT + i] = tunitColors[color * PLAYABLE_PLAYER_COUNT + i];
	}
}

static void LoadColorTables()
{
	helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimapColors, MINIMAP_COLOR_COUNT, 0, 0, 0);
	helper::SBmpLoadImage("game\\tunit.pcx", 0, tunitColors, UNIT_COLOR_COUNT, 0, 0, 0);
}

bool CheckRandomizationValidity()
{
	u32 bannedRaceTintsCount = 0;
	for (auto race : bannedRaceTints)
	{
		if (race.size() > bannedRaceTintsCount)
		{
			bannedRaceTintsCount = race.size();
		}
	}

	if (playerTints.size() - bannedTilesetTints[*CURRENT_TILESET].size() - bannedRaceTintsCount < PLAYABLE_PLAYER_COUNT)
	{
		scbw::print("Color Randomization Error:", GameTextColor::Red);
		scbw::print("\tNot enough tints available for current tileset", GameTextColor::Red);
		LoadDefaultPlayerColors();
		return false;
	}
	return true;
}

void RandomizeColorFor(u32 playerId)
{
	if (playerTable[playerId].type == PlayerType::Computer)
	{
		u32 color = GetColorForPersonality(playerId);
		if (color != PlayerColor::Invalid)
		{
			SetPlayerColor(playerId, color);
			return;
		}
	}

	u32 tint = GetRandomTint();
	while (!CanUsePlayerTint(tint, playerId))
	{
		tint = GetRandomTint();
	}
	SetPlayerColor(playerId, GetRandomColorFromTint(tint));
}

void MatchColorOf(u32 player_id)
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
	if (!CheckRandomizationValidity())
	{
		LoadDefaultPlayerColors();
		return;
	}

	LoadColorTables();
	for (u32 playerId = 0; playerId < PLAYABLE_PLAYER_COUNT; playerId++)
	{
		RandomizeColorFor(playerId);
	}
}

void LoadDefaultPlayerColors()
{
	LoadColorTables();
	bool canRandomize = CheckRandomizationValidity();
	for (u32 playerId = 0; playerId < PLAYABLE_PLAYER_COUNT; ++playerId)
	{
		if (canRandomize && deathTable[UnitId::ResourceVespeneGeyser].player[playerId] & VespeneDeaths_RandomizeColor)
		{
			RandomizeColorFor(playerId);
		}
		else
		{
			MatchColorOf(playerId);
		}
	}
}