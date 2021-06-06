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
	return scbw::random() % tints.size();
}

u32 get_random_color_from_tint(u32 tint)
{
	return scbw::random() % tints[tint].size();
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

void set_melee_player_colors()
{
	for (u32 player_id = 0; player_id < 8; player_id++)
	{
		if (playerTable[player_id].type == PlayerType::Computer)
		{
			u32 color = get_color_for_personality(player_id);
			if (color != PlayerColor::Invalid)
			{
				playersColors[player_id] = color;
				return;
			}
		}

		u32 tint = get_random_tint();
		while (!can_use_player_tint(tint, player_id))
		{
			tint = get_random_tint();
		}
		playersColors[player_id] = get_random_color_from_tint(tint);
	}
}