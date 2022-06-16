#pragma once
#include "common.h"

// Not like the advanced BWAPI struct
struct Player
{
	/*00*/  u32   id;
	/*04*/  u32   actions;    // Unused; FF FF FF FF if not a human player
	/*08*/	u8    type;
	/*09*/  u8    race;       // Use with scbw::RaceId
	/*0A*/  u8    force;
	/*0B*/  char  name[25];
	/*struct count = 36 (0x24)*/

	// return the possibly non-standard color of the current player
	u8 get_color();

	// return the possibly non-standard color of the player identified by PlayerId
	static u8 get_color(u8 player);

	// return the faction (tribe, brood or other) color  of current player
	u8 get_faction_color();

	// return the faction (tribe, brood or other) color of the player identified by PlayerId
	static u8 get_faction_color(u8 player);

	static u32 number_of_units_of_id(u8 player, u16 unit_id);
	static u32 number_of_completed_units_of_id(u8 player, u16 unit_id);
};
