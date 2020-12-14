#pragma once
#pragma warning(disable: 4455)

// This file is meant to gather constants used for unit behaviors.
// It's primary role is to reduce amounts of bugs which come from partial code editing,
// by removing the need to hard code same "magic numbers" in multiple places of the codebase.
// And also to make it easier to edit and manage game variables in general.
//
// Obviously for now not many things are using it, but I would recommend moving things here as time passes.
// ~ Veeq7

// Notes:
// - It's pretty annoying that project has to recompile most files after chaning this one. that was the case with CUnit already,
//		but we might want to limit the include scope for this one
// - Maybe we should also move aise enums in here, I don't think it ever made sense for it to be in CUnit, since it's included everywhere.

// Shorthand for in-game seconds (1 second - 24 frames). only works in this file to avoid potential conflicts.
static constexpr int operator "" s(unsigned long long amount) {
	return (int)(amount * 24);
}
static constexpr int operator "" s(long double amount) {
	return (int)(amount * 24);
}

// Constants
// Note: I would recommend having unit name as first segment of any related variable name.

// Madcap
constexpr int MADCAP_PASSIVE_DECAY_TIME = 0.5s;
constexpr int MADCAP_MAX_STACKS = 10;
constexpr double MADCAP_ATTACK_SPEED_PER_STACK = 0.05;

//Corsair - Disruption Web - AI
constexpr int AI_WEB_SEEK_RANGE = 96;
constexpr int AI_CORSAIR_ALLY_SEEK_RANGE = 128;
constexpr int AI_CORSAIR_ENEMY_SEEK_RANGE = 128;
constexpr double AI_CORSAIR_LIFE_PERCENTAGE = 0.5;
constexpr int AI_WEB_MIN_ENERGY = 30;
constexpr int AI_WEB_ALLY_MIN_COUNT = 3;
constexpr int AI_WEB_ENEMY_MIN_COUNT = 2;
constexpr int AI_WEB_TIMER = 3 * 24;

//Star Sovereign - Grief of All Gods - AI
constexpr int AI_STARSOVEREIGN_ABIL_SEEKRANGE = 32 * 10;
constexpr int AI_STARSOVEREIGN_ABIL_MIN_ENEMY_HEALTH = 2000;
constexpr int AI_STARSOVEREIGN_ABIL_MAX_ALLY_HEALTH = 2000;
constexpr double AI_STARSOVEREIGN_ABIL_LIFE_PERCENTAGE = 0.5;
constexpr int AI_STARSOVEREIGN_ABIL_TIMER = 612;