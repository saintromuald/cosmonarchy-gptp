#include "player_color.h"

static u32 get_random_tint();
static u32 get_random_color_from_tint(u32 tint);
static u32 get_player_tint(u32 color);
static bool can_use_player_tint(u32 color_tint, u32 last_player);
static u32 get_color_for_personality(u32 player_id);
static void set_player_color(u32 player_id, u32 color);

static u32 get_player_tint(u32 color)
{
	for (u32 tint = 0; tint < player_tints.size(); ++tint)
	{
		for (u32 color_index = 0; color_index < player_tints[tint].size(); color_index++)
		{
			if (player_tints[tint][color_index] == color)
			{
				return tint;
			}
		}
	}
	return -1;
}

static bool can_use_player_tint(u32 color_tint, u32 last_player)
{
	if (playerTable[last_player].race < banned_race_tints.size())
	{
		for (u32 banned_tint : banned_race_tints[playerTable[last_player].race])
		{
			if (banned_tint == color_tint)
			{
				return false;
			}
		}
	}
	if (*CURRENT_TILESET < banned_tileset_tints.size())
	{
		for (u32 banned_tint : banned_tileset_tints[*CURRENT_TILESET])
		{
			if (banned_tint == color_tint)
			{
				return false;
			}
		}
	}
	for (u32 player_id = 0; player_id < last_player; ++player_id)
	{
		if (get_player_tint(player_id) == color_tint)
		{
			return false;
		}
	}
	return true;
}

static u32 get_random_tint()
{
	return helper::randFunc() % PlayerTintRollCount;
}

static u32 get_random_color_from_tint(u32 tint)
{
	return player_tints[tint][helper::randFunc() % player_tints[tint].size()];
}

static u32 get_color_for_personality(u32 player_id)
{
	const u32 race = playerTable[player_id].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[player_id] - 1;

	if (build >= 0
		&& personality_colors.size() > race
		&& personality_colors[race].size() > build)
	{
		for (u32 color : personality_colors[race][build])
		{
			if (can_use_player_tint(get_player_tint(color), player_id))
			{
				return color;
			}
		}
	}

	return PlayerColor::Invalid;
}

static u8 tminimap_colors[256];
static u8 tunit_colors[2048];

static void set_player_color(u32 player_id, u32 color)
{
	factionsColorsOrdering[player_id] = color;
	playerMinimapColors[player_id] = tminimap_colors[color];
	for (u32 i = 0; i < 8; i++)
	{
		playerInGameColors[player_id * 8 + i] = tunit_colors[8 * color + i];
	}
}

static void set_melee_player_colors()
{
	u32 banned_race_tints_count = 0;
	for (auto race : banned_race_tints)
	{
		if (race.size() > banned_race_tints_count)
		{
			banned_race_tints_count = race.size();
		}
	}

	if (player_tints.size() - banned_tileset_tints[*CURRENT_TILESET].size() - banned_race_tints_count < 8)
	{
		scbw::print("Color Randomization Error:", GameTextColor::Red);
		scbw::print("\tNot enough tints available for current tileset", GameTextColor::Red);
		return;
	}

	helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimap_colors, 256, 0, 0, 0);
	helper::SBmpLoadImage("game\\tunit.pcx", 0, tunit_colors, 2048, 0, 0, 0);

	for (u32 player_id = 0; player_id < 8; player_id++)
	{
		if (playerTable[player_id].type == PlayerType::Computer)
		{
			u32 color = get_color_for_personality(player_id);
			if (color != PlayerColor::Invalid)
			{
				set_player_color(player_id, color);
				continue;
			}
		}

		u32 tint = get_random_tint();
		while (!can_use_player_tint(tint, player_id))
		{
			tint = get_random_tint();
		}
		set_player_color(player_id, get_random_color_from_tint(tint));
	}
}