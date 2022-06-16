#include <cassert>
#include "Player.h"
#include "../api.h"
#include "../enumerations.h"
#include "../../graphics/graphics.h"

// ----------------------------------------------------------------------------//
// Return the proper ColorId to use a player color with graphics function
// ----------------------------------------------------------------------------//

graphics::ColorId Player::get_color()
{
	assert(this);
	return playerMinimapColors[(u8)this->id];
}

graphics::ColorId Player::get_color(u8 player)
{
	if (player < PLAYER_COUNT)
	{
		return playerMinimapColors[player];
	}
	else
	{
		return graphics::BLACK; // return 0/Black if error
	}
}

graphics::ColorId Player::get_faction_color()
{
	assert(this);
	return get_faction_color(this->id);
}

graphics::ColorId Player::get_faction_color(u8 player)
{
	// store a player until the end where it will properly store a ColorId
	graphics::ColorId return_value;

	// for normal players, apply colors switching
	if (player < PLAYABLE_PLAYER_COUNT)
	{
		// get the proper player in multiplayer game (don't break campaigns/UMS)
		return_value = factionsColorsOrdering[player];

		// fix BROWN to GREEN color swap in Desert map
		if (*CURRENT_TILESET == TilesetType::Desert && return_value == 5)
		{
			return_value = 8;
		}

		// fix WHITE to GREEN color swap in Ice map
		if (*CURRENT_TILESET == TilesetType::Ice && return_value == 6)
		{
			return_value = 8;
		}
	}

	else
	{
		return_value = player;
	}

	if (player < PLAYER_COUNT)
	{
		return_value = graphics::standardColors[return_value];
	}

	else
	{
		return_value = graphics::BLACK; // return 0/Black if error
	}
	return return_value;
}

u32 Player::number_of_units_of_id(u8 player, u16 unit_id)
{
	if (unit_id > UNIT_TYPE_COUNT - 1)
	{
		return 0;
	}
	return all_unit_count_table[unit_id].player[player];
}

u32 Player::number_of_completed_units_of_id(u8 player, u16 unit_id)
{
	if (unit_id > UNIT_TYPE_COUNT - 1)
	{
		return 0;
	}
	return unit_count_table[unit_id].player[player];
}