#pragma once
#include "common.h"

//Not like the advanced BWAPI class
struct PLAYER {

	/*00*/  uint32   id;
	/*04*/  uint32   actions;    //Unused; FF FF FF FF if not a human player
	/*08*/	uint8    type;
	/*09*/  uint8    race;       //Use with scbw::RaceId
	/*0A*/  uint8    force;
	/*0B*/  char  name[25];

	/*struct size = 36 (0x24)*/

	  /// return the possibly non-standard color of the 
	  /// current player
	uint8 getColor();

	/// return the possibly non-standard color of the 
	/// player identified by PlayerId
	static uint8 getColor(uint8 playerId);

	/// return the faction (tribe, brood or other) color 
	///of current player
	uint8 getFactionColor();

	/// return the faction (tribe, brood or other) color 
	/// of the player identified by PlayerId
	static uint8 getFactionColor(uint8 playerId);

};
