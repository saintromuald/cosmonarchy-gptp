#include "player_color.h"

u32 get_player_tint(u32 color)
{
	for (u32 tint = 0; tint < tints.size(); ++tint)
	{
		for (u32 color_index = 0; color_index < tints[tint].size(); color_index++)
		{
			if (tints[tint][color_index] == color)
			{
				return tint;
			}
		}
	}
	return -1;
}

bool can_use_player_tint(u32 color_tint, u32 last_player)
{
	for (u32 player_id = 0; player_id < last_player; ++player_id)
	{
		if (get_player_tint(player_id) == color_tint)
		{
			return false;
		}
	}
	return true;
}

u32 get_random_tint()
{
	return scbw::randBetween(0, tints.size() - 1);
}

u32 get_random_color_from_tint(u32 tint)
{
	return tints[tint][scbw::randBetween(0, tints[tint].size() - 1)];
}

u32 get_color_for_personality(u32 player_id)
{
	enum
	{
		TerranBuildMech = 1,
		TerranBuildRaider,
		TerranBuildAir,
		TerranBuildShadow,
		TerranBuildGeneral,

		ProtossBuildRanger = 1,
		ProtossBuildTemplar,
		ProtossBuildSkylord,
		ProtossBuildGeneral,

		ZergBuildAssault = 1,
		ZergBuildSwarm,
		ZergBuildAir,
		ZergBuildGeneral,
	};

	const u32 race = playerTable[player_id].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[player_id];

	switch (race)
	{
		case RaceId::Terran:
		{
			switch (build)
			{
				case TerranBuildMech:
				{
					if (can_use_player_tint(PlayerTintDarkRed, player_id))
					{
						return PlayerColor::Red;
					}
				} break;
				case TerranBuildRaider:
				{

				} break;
				case TerranBuildAir:
				{

				} break;
				case TerranBuildShadow:
				{

				} break;
				case TerranBuildGeneral:
				{

				} break;
			}
		} break;
		case RaceId::Protoss:
		{
			switch (build)
			{
				case ProtossBuildRanger:
				{

				} break;
				case ProtossBuildTemplar:
				{

				} break;
				case ProtossBuildSkylord:
				{

				} break;
				case ProtossBuildGeneral:
				{

				} break;
			}
		} break;
		case RaceId::Zerg:
		{
			switch (build)
			{
				case ZergBuildAssault:
				{

				} break;
				case ZergBuildSwarm:
				{

				} break;
				case ZergBuildAir:
				{

				} break;
				case ZergBuildGeneral:
				{

				} break;
			}
		} break;
	}
	return PlayerColor::Invalid;
}

static u8 tminimap_colors[256];
static u8 tunit_colors[2048];

inline void set_player_color(u32 player_id, u32 color)
{
	playerMinimapColors[player_id] = tminimap_colors[color];
	for (u32 i = 0; i < 8; i++)
	{
		playerInGameColors[player_id * 8 + i] = tunit_colors[8 * color + i];
	}
}

void set_melee_player_colors()
{
	helper::SBmpLoadImage("game\\tunit.pcx", 0, tunit_colors, 2048, 0, 0, 0);
	helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimap_colors, 256, 0, 0, 0);

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