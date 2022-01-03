#include "scenario.h"
#include "unit_search.h"
#include "player_color.h"
#include "hooks/interface/uifeatures/ui_features.h"

#include <assert.h>

u32 get_transmission_playing() {
	return unit_deaths[UnitId::Special_PitDoor].player[0];
}

bool is_map(u32 campaign, u32 map) {
	return unit_deaths[UnitId::Spell_ScannerSweep].player[0] == campaign
		&& unit_deaths[UnitId::Special_RightPitDoor].player[0] == map;
}

enum {
	Fraud_None,

	/*
	-- The Legion of Frauds
	All members of this force are hostile to one another.
	A new member of this force arrives every five minutes. Each run has a unique order of arrival.
	*/
	// == TERRAN ==
	Fraud_AlexeiUkov,					// --	Orbital + Timing	--	<06> Autocrat
	Fraud_UllenBankOnMeBigmunney,		// --	Armor + Greed		--	<18> Blackjack
	Fraud_knightoftherealm,				// --	Raider + Rush		--	<1A> Maverick
	Fraud_KKKhris,						// --	General + Timing	--	<15> Cyprian
	Fraud_Dynamo,						// --	Raider + Timing		--	<11> Penumbra
	Fraud_TodsDadPat,					// --	Armor + Rush		--	<1B> Phalanx
	Fraud_Piglet,						// --	Shadow + Timing		--	<10> Cohort
	Fraud_DarkenedFantasies,			// --	Armor + Timing		--	<18> Goliath
	Fraud_CustomerSupporter,			// --	Orbital + Greed		--	<0F> Cyprian

	// == PROTOSS ==
	Fraud_BitcoinBoris,					// --	Profane + Rush		--	<15> Nexus
	Fraud_CryptoCarl,					// --	Skylord + Timing	--	<1B> Atreus
	Fraud_Veeq7,						// --	Ranger + Timing		--	<1A> Didact
	Fraud_jun3hong,						// --	Templar + Timing	--	<1B> Envoy
	Fraud_DigitalRightsManager,			// --	Templar + Rush		--	<05> Amaranth
	Fraud_Zeratool,                     // --   Profane + Timing    --  <10> Anthelion

	// == ZERG ==
	Fraud_CerebrateStalin,				// --	Miscreant + Timing	--	<10> Anticthon
	Fraud_AndyRoseUh,					// --	Assault + Timing	--	<10> Konvilisk
	Fraud_Winnie,						// --	Swarm + Greed		--	<03> Hatchery
	Fraud_A,							// --	Miscreant + Rush	--	<18> Zethrokor
	Fraud_SelloutSam,					// --	Swarm + Timing		--	<1B> Rilirokor
	Fraud_PlagiaristPaul,				// --	Assault + Greed		--	<15> Matraleth
	Fraud_RimTechCombot,				// --	Swarm + Timing		--	<10> Shaman

	// == RANDOM ==	
	Fraud_LightenedRealities,			// --	General + Rush							--	<03> Augur
	Fraud_AchromaticHalfdreams,			// --	General + Timing						--	<18> Wendigo
	Fraud_TonyTron,						// --	Orbital / Sunblot / Skylord + Greed		--	<0E> Olympian
	Fraud_Pr0nogo,						// --	Armor / Assault / Profane + Timing		--	<1A> Seraph
	Fraud_Ba3lethal,					// --	Raider / Assault / Templar + Rush		--	<11> Eidolon
	Fraud_theToweringPigprophet,	 	// --	Orbital / Sunblot / Skylord + Timing	--	<15> Ovileth
	Fraud_theAnalCrowsummoner,			// --	Shadow / Sunblot / Ranger + Greed		--	<05> Mutalisk
	Fraud_thePissingPorncultist,		// --	Shadow / Miscreant / Profane + Rush		--	<03> Cyprian
	Fraud_theBearborneArkanianpussy,	// --	Shadow / Swarm / Templar + Timing		--	<1D> Ultrokor
	Fraud_theDoublingWuckdevourer,		// --	Armor / Swarm / Templar + Greed			--	<0E> Droleth
	Fraud_theMongoloidPointcorruptor,	// --	Raider / Assault / Ranger + Timing		--	<1F> Cohort
	Fraud_theCalendaredRageboy,			// --	Orbital / Assault / Skylord + Rush		--	<06> Zealot
	Fraud_theWorthyPizzasniffer,		// --	Raider / Swarm / Templar + Rush			--	<01> Goliath
	Fraud_theSkeletalTodzombie,			// --	Armor / Miscreant / Profane + Timing	--	<1F> Sovroleth
	Fraud_HoopThrower,					// --	Raider / Miscreant / Templar + Timing	--	<07> Mason
	Fraud_Veeq8,						// --	Orbital / Swarm / Ranger + Timing		--	<1A> Didact

	/*
	-- The Hall of Heroes
	All members of this type are allied to one another, and deploy with additional starting units and resources.
	A new member of this type arrives once every eight minutes. Each run has a unique order of arrival.
	*/
	// == TERRAN ==
	FraudHero_Francisco_Goska,          // --    Raider + Timing    --    <11> Eidolon
	FraudHero_Ackmed_Clone,             // --    Raider + Rush      --    <11> Harakan
	FraudHero_Cullen,                   // --    Orbital + Rush     --    <0F> Blackjack

	// == PROTOSS ==
	FraudHero_Torros,                   // --    Templar + Rush     --    <0F> Amaranth
	FraudHero_Asalamir,                 // --    Ranger + Timing    --    <0F> Atreus
	FraudHero_Thalazim,					// --    Profane + Rush     --    <05> Cabalist

	// == ZERG ==
	FraudHero_Stroluum,                 // --    Swarm + Rush       --    <11> Isthrathaleth
	FraudHero_Tuzor,                    // --    Miscreant + Greed  --    <11> Anticthon
	FraudHero_Charlie,                  // --    Assaault + Timing  --    <16> Axitrilisk

	/*
	-- The Supreme Kings
	All members of this type are hostile to one another, deploy with additional starting units and resources, and have an increased resource multiplier.
	A new member of this type arrives every fifteen minutes. Each run has a unique order of arrival.
	*/
	// == TERRAN ==
	FraudSuper_Ackmed,                  // --    Raider + Rush      --    <10> Harakan

	// == PROTOSS ==
	FraudSuper_Drakkar,                 // --    Ranger + Timing    --    <0E> Atreus

	// == ZERG ==
	FraudSuper_Zerghere,                // --    Swarm + Greed      --    <10> Hatchery

	// == RANDOM ==
	FraudSuper_God1,                    // --    General + Timing   --    <06> Mind Tyrant
	FraudSuper_God2,                    // --    General + Timing   --    <06> Mind Tyrant
	FraudSuper_God3,                    // --    General + Timing   --    <06> Mind Tyrant
	FraudSuper_God4,                    // --    General + Timing   --    <06> Mind Tyrant
	FraudSuper_God5,                    // --    General + Timing   --    <06> Mind Tyrant
	FraudSuper_God6,                    // --    General + Timing   --    <06> Mind Tyrant

	Fraud_Count,

	Fraud_TerranFirst = Fraud_AlexeiUkov,
	Fraud_TerranLast = Fraud_CustomerSupporter,
	Fraud_ProtossFirst = Fraud_BitcoinBoris,
	Fraud_ProtossLast = Fraud_Zeratool,
	Fraud_ZergFirst = Fraud_CerebrateStalin,
	Fraud_ZergLast = Fraud_RimTechCombot,
	Fraud_RandomFirst = Fraud_LightenedRealities,
	Fraud_RandomLast = Fraud_Veeq8,

	FraudHero_TerranFirst = Fraud_AlexeiUkov,
	FraudHero_TerranLast = Fraud_CustomerSupporter,
	FraudHero_ProtossFirst = Fraud_BitcoinBoris,
	FraudHero_ProtossLast = Fraud_Zeratool,
	FraudHero_ZergFirst = Fraud_CerebrateStalin,
	FraudHero_ZergLast = Fraud_RimTechCombot,

	FraudSuper_TerranFirst = FraudSuper_Ackmed,
	FraudSuper_TerranLast = FraudSuper_Ackmed,
	FraudSuper_ProtossFirst = FraudSuper_Drakkar,
	FraudSuper_ProtossLast = FraudSuper_Drakkar,
	FraudSuper_ZergFirst = FraudSuper_Zerghere,
	FraudSuper_ZergLast = FraudSuper_Zerghere,

	FraudGod_FinalFirst = FraudSuper_God1,
	FraudGod_FinalLast = FraudSuper_God6,

	FraudClass_Fraud = 0,
	FraudClass_Hero,
	FraudClass_Super,
	FraudClass_Count,

	FraudClass_God,
};

struct Fraud {
	u32 personality;
	u32 build_type;
	u32 build_order;
	u32 color;
};

struct FraudSpawnSource {
	Point16 pos;
	std::vector<Point16> rallies;
};

struct FraudSpawnLocation {
	bool set;
	Point16 pos;
	std::vector<FraudSpawnSource> sources;
};

struct FraudSpawnSequence {
	bool townhall_placed;
	u32 fraud_class;
	u32 fraud;
	u32 race;
	u32 unit_timer;
	u32 source_index;
	u32 rally_index;
	u32 unit_index;
	FraudSpawnLocation* location;
	Unit* revealer;
};

struct FraudSpawnUnit {
	u32 count;
	u32 unit_id;
};

struct FraudSpawn {
	Fraud frauds[Fraud_Count][RaceId::Count];
	u32 used_count;
	bool used[Fraud_Count];
	u32 timer[FraudClass_Count];

	FraudSpawnSequence sequence[6];
	std::vector<FraudSpawnLocation> spawns;
	std::vector<FraudSpawnUnit> units[Fraud_Count][RaceId::Count];
};

FraudSpawn fraud_spawn;

FraudSpawnLocation* get_random_spawn() {
	u32 count = fraud_spawn.spawns.size();
	if (!count) {
		return 0;
	}
	u32 offset = scbw::randBetween(0, count - 1);
	for (u32 i = 0; i < count; ++i) {
		u32 location = (i + offset) % count;

		u32 skip = false;
		for (u32 j = 0; j < 6; ++j) {
			if (fraud_spawn.sequence[j].location == &fraud_spawn.spawns[location]) {
				skip = true;
				break;
			}
		}
		if (skip) {
			continue;
		}

		UnitArray unit_array = UnitSearchRadial(fraud_spawn.spawns[location].pos, 12 * 32);
		UnitArrayFilter(&unit_array, 0, FilterBuilding);
		if (!unit_array.size) {
			return &fraud_spawn.spawns[location];
		}
	}
	return &fraud_spawn.spawns[offset];
}

FraudSpawnLocation* get_closest_spawn(Point16 pos) {
	FraudSpawnLocation* closest_loc = 0;
	f32 lowest_distance = 999999999;

	for (FraudSpawnLocation& loc : fraud_spawn.spawns) {
		f32 distance = scbw::getDistanceFast(loc.pos, pos);
		if (distance < lowest_distance) {
			closest_loc = &loc;
			lowest_distance = distance;
		}
	}

	return closest_loc;
}

FraudSpawnSource* get_closest_source(FraudSpawnLocation* location, Point16 pos) {
	FraudSpawnSource* closest_loc = 0;
	f32 lowest_distance = 999999999;

	for (FraudSpawnSource& loc : location->sources) {
		f32 distance = scbw::getDistanceFast(loc.pos, pos);
		if (distance < lowest_distance) {
			closest_loc = &loc;
			lowest_distance = distance;
		}
	}

	return closest_loc;
}

void init_fraud_spawns() {
	Unit* next = 0;
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::TerranCommandCenter) {
			FraudSpawnLocation spawn = {};
			spawn.set = true;
			spawn.pos = unit->position;
			fraud_spawn.spawns.push_back(spawn);
			unit->userActionFlags |= 4;
			unit->remove();
		}
	}
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::ProtossPylon) {
			FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
			if (spawn) {
				spawn->sources.push_back({unit->position});
			}
			unit->userActionFlags |= 4;
			unit->remove();
		}
	}
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::TerranMaverick) {
			FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
			if (spawn) {
				FraudSpawnSource* source = get_closest_source(spawn, unit->position);
				if (source) {
					source->rallies.push_back(unit->position);
				}
			}
			unit->userActionFlags |= 4;
			unit->remove();
		}
	}
}

void init_fraud_units() {
	// Todo: Only for testing, remove this
	for (u32 i = 0; i < Fraud_Count; ++i) {
		fraud_spawn.frauds[i][RaceId::Terran] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Aero};
		fraud_spawn.units[i][RaceId::Terran].push_back({8, UnitId::TerranMason});
		fraud_spawn.units[i][RaceId::Terran].push_back({2, UnitId::ZergOvileth});
		fraud_spawn.units[i][RaceId::Terran].push_back({8, UnitId::TerranMaverick});
		fraud_spawn.units[i][RaceId::Terran].push_back({4, UnitId::TerranCyprian});

		fraud_spawn.frauds[i][RaceId::Protoss] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Magma};
		fraud_spawn.units[i][RaceId::Protoss].push_back({8, UnitId::ProtossScribe});
		fraud_spawn.units[i][RaceId::Protoss].push_back({2, UnitId::ZergOvileth});
		fraud_spawn.units[i][RaceId::Protoss].push_back({4, UnitId::ProtossZealot});
		fraud_spawn.units[i][RaceId::Protoss].push_back({2, UnitId::ProtossDracadin});

		fraud_spawn.frauds[i][RaceId::Zerg] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Magenta};
		fraud_spawn.units[i][RaceId::Zerg].push_back({8, UnitId::ZergDroleth});
		fraud_spawn.units[i][RaceId::Zerg].push_back({2, UnitId::ZergOvileth});
		fraud_spawn.units[i][RaceId::Zerg].push_back({16, UnitId::ZergZethrokor});
		fraud_spawn.units[i][RaceId::Zerg].push_back({8, UnitId::ZergHydralisk});
	}

	// Todo: Add units for all personalities, using the format below

	// Veeq7
	fraud_spawn.frauds[Fraud_Veeq7][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aero};
	fraud_spawn.units[Fraud_Veeq7][RaceId::Protoss].push_back({4, UnitId::ProtossScribe});
	fraud_spawn.units[Fraud_Veeq7][RaceId::Protoss].push_back({2, UnitId::ProtossZealot});

	// Pr0nogo
	fraud_spawn.frauds[Fraud_Pr0nogo][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Aero};
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Protoss].push_back({4, UnitId::ProtossScribe});
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Protoss].push_back({2, UnitId::ProtossZealot});
}

bool place_fraud(u32 player, u32 fraud_class) {
	if (!fraud_spawn.sequence[player - 1].location) {
		u32 race = 0;
		u32 race_first = 0;
		u32 race_last = 0;
		u32 random_first = 0;
		u32 random_last = 0;
		if (fraud_class == FraudClass_Fraud) {
			random_first = Fraud_RandomFirst;
			random_last = Fraud_RandomLast;
		} else if (fraud_class == FraudClass_God) {
			random_first = FraudGod_FinalFirst;
			random_last = FraudGod_FinalLast;
		}
		if (player == 1 || player == 4) { // Terran
			race = RaceId::Terran;
			if (fraud_class == FraudClass_Fraud) {
				race_first = Fraud_TerranFirst;
				race_last  = Fraud_TerranLast;
			} else if (fraud_class == FraudClass_Hero) {
				race_first = FraudHero_TerranFirst;
				race_last  = FraudHero_TerranLast;
			} else if (fraud_class == FraudClass_Super) {
				race_first = FraudSuper_TerranFirst;
				race_last  = FraudSuper_TerranLast;
			}
		} else if (player == 2 || player == 5) { // Protoss
			race = RaceId::Protoss;
			if (fraud_class == FraudClass_Fraud) {
				race_first = Fraud_ProtossFirst;
				race_last  = Fraud_ProtossLast;
			} else if (fraud_class == FraudClass_Hero) {
				race_first = FraudHero_ProtossFirst;
				race_last  = FraudHero_ProtossLast;
			} else if (fraud_class == FraudClass_Super) {
				race_first = FraudSuper_ProtossFirst;
				race_last  = FraudSuper_ProtossLast;
			}
		} else if (player == 3 || player == 6) { // Zerg
			race = RaceId::Zerg;
			if (fraud_class == FraudClass_Fraud) {
				race_first = Fraud_ZergFirst;
				race_last  = Fraud_ZergLast;
			} else if (fraud_class == FraudClass_Hero) {
				race_first = FraudHero_ZergFirst;
				race_last  = FraudHero_ZergLast;
			} else if (fraud_class == FraudClass_Super) {
				race_first = FraudSuper_ZergFirst;
				race_last  = FraudSuper_ZergLast;
			}
		}

		u32 race_count = 0;
		u32 random_count = 0;
		for (u32 i = race_first; i <= race_last; ++i) {
			if (!fraud_spawn.used[i]) {
				++race_count;
			}
		}
		for (u32 i = random_first; i <= random_last; ++i) {
			if (!fraud_spawn.used[i]) {
				++random_count;
			}
		}

		u32 next_fraud = 0;
		if (race_count && scbw::randBetween(0, race_count + random_count - 1) < race_count) {
			next_fraud = scbw::randBetween(race_first, race_last);
			while (fraud_spawn.used[next_fraud]) {
				next_fraud = scbw::randBetween(race_first, race_last);
			}
		} else if (random_count) {
			next_fraud = scbw::randBetween(random_first, random_last);
			while (fraud_spawn.used[next_fraud]) {
				next_fraud = scbw::randBetween(random_first, random_last);
			}
		}

		if (next_fraud) {
			FraudSpawnLocation* spawn_location = get_random_spawn();
			if (spawn_location) {
				++fraud_spawn.used_count;
				fraud_spawn.used[next_fraud] = true;
				if (fraud_class == FraudClass_Fraud) {
					fraud_spawn.timer[fraud_class] = 5 * 60 * 24;
				} else if (fraud_class == FraudClass_Hero) {
					fraud_spawn.timer[fraud_class] = 8 * 60 * 24;
				} else if (fraud_class == FraudClass_Super) {
					fraud_spawn.timer[fraud_class] = 15 * 60 * 24;
				}
				fraud_spawn.sequence[player-1].fraud_class = fraud_class;
				fraud_spawn.sequence[player-1].fraud = next_fraud;
				fraud_spawn.sequence[player-1].race = race;
				fraud_spawn.sequence[player-1].unit_timer = 0;
				fraud_spawn.sequence[player-1].source_index = 0;
				fraud_spawn.sequence[player-1].rally_index = 0;
				fraud_spawn.sequence[player-1].unit_index = 0;
				fraud_spawn.sequence[player-1].location = spawn_location;
				fraud_spawn.sequence[player-1].revealer = scbw::createUnitInGame(UnitId::Special_MapRevealer, 0, spawn_location->pos.x, spawn_location->pos.y);

				scbw::minimapPing(spawn_location->pos.x, spawn_location->pos.y, 0, *LOCAL_NATION_ID);

				unit_deaths[UnitId::Spell_DisruptionWeb].player[player] = fraud_spawn.frauds[next_fraud][race].personality;
				unit_deaths[UnitId::ProtossPlasmaShell].player[player] = fraud_spawn.frauds[next_fraud][race].build_type;
				unit_deaths[UnitId::Spell_DarkSwarm].player[player] = fraud_spawn.frauds[next_fraud][race].build_order;
				SetPlayerColor(player, fraud_spawn.frauds[next_fraud][race].color);

				for (u32 i = 1; i < 7; ++i) {
					for (u32 j = 1; j < 7; ++j) {
						if (i == j) {
							continue;
						}
						if (fraud_spawn.sequence[i-1].fraud_class == fraud_spawn.sequence[j-1].fraud_class &&
							(fraud_spawn.sequence[i-1].fraud_class == FraudClass_Hero || fraud_spawn.sequence[i-1].fraud_class == FraudClass_God)) {
							playerAlliance[i].flags[j] = true;
							playerAlliance[j].flags[i] = true;
						} else {
							playerAlliance[i].flags[j] = false;
							playerAlliance[j].flags[i] = false;
						}
					}
				}
				return true;
			}
		}
	}
	return false;
}

void init_scenario() {
	fraud_spawn = {};
	if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch)) {
		fraud_spawn.timer[FraudClass_Fraud] = 5 * 60 * 24;
		fraud_spawn.timer[FraudClass_Hero] = 8 * 60 * 24;
		fraud_spawn.timer[FraudClass_Super] = 15 * 60 * 24;

		init_fraud_spawns();
		init_fraud_units();

		u32 first_fraud = scbw::randBetween(1, 6);
		u32 second_fraud = scbw::randBetween(1, 6);
		while (second_fraud == first_fraud) {
			second_fraud = scbw::randBetween(1, 6);
		}
		
		// Todo: spawn creep
		place_fraud(first_fraud, FraudClass_Fraud);
		if (fraud_spawn.sequence[first_fraud-1].location) {
			fraud_spawn.sequence[first_fraud - 1].townhall_placed = true;
			u32 townhall = UnitId::TerranCommandCenter;
			if (first_fraud == 2 || first_fraud == 5) townhall = UnitId::ProtossNexus;
			if (first_fraud == 3 || first_fraud == 6) townhall = UnitId::ZergHatchery;
		
			scbw::createUnitInGame(townhall, first_fraud, fraud_spawn.sequence[first_fraud-1].location->pos.x, fraud_spawn.sequence[first_fraud-1].location->pos.y);
		}
		place_fraud(second_fraud, FraudClass_Fraud);
		if (fraud_spawn.sequence[second_fraud-1].location) {
			fraud_spawn.sequence[second_fraud - 1].townhall_placed = true;
			u32 townhall = UnitId::TerranCommandCenter;
			if (second_fraud == 2 || second_fraud == 5) townhall = UnitId::ProtossNexus;
			if (second_fraud == 3 || second_fraud == 6) townhall = UnitId::ZergHatchery;
		
			scbw::createUnitInGame(townhall, second_fraud, fraud_spawn.sequence[second_fraud-1].location->pos.x, fraud_spawn.sequence[second_fraud-1].location->pos.y);
		}
	}	
}

typedef s32 __cdecl VictoryFunc();
VictoryFunc* Victory = (VictoryFunc*)0x4C5310;

void tick_scenario() {
	/*
		Terran players: 2, 5
		Protoss players: 3, 6
		Zerg players: 4, 7
	*/
	if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch)) {
		char* timer_prefix[FraudClass_Count] = {
			"Next Fraud: ",
			"Next Hero: ",
			"Next Super: ",
		};
		for (u32 i = 0; i < FraudClass_Count; ++i) {
			static char buffer[64];
			format_time(buffer, 64, timer_prefix[i], fraud_spawn.timer[i] / 24);
			graphics::drawText(10, 3 + 15 * (i + 1), std::string(buffer));
		}

		u32 alive_count = 0;
		for (u32 i = 1; i < 7; ++i) {
			if (unit_deaths[UnitId::Factories].player[i] > 0) {
				++alive_count;
			}
		}
		
		if (!alive_count && fraud_spawn.used_count == Fraud_Count) {
			*currentTriggerPlayerId = 0;
			Victory();
		}

		// Fraud Spawning
		for (u32 i = 0; i < FraudClass_Count; ++i) {
			if (alive_count && fraud_spawn.timer[i]) {
				--fraud_spawn.timer[i];
			} else {
				fraud_spawn.timer[i] = 0;
			}
			if (!fraud_spawn.timer[i]) {
				u32 offset = scbw::randBetween(0, 5);
				for (u32 i = 0; i < 6; ++i) {
					u32 player = (i + offset) % 6 + 1;
					if (unit_deaths[UnitId::Factories].player[player] == 0) {
						if (place_fraud(player, i)) {
							++alive_count;
							break;
						}
					}
				}
			}
		}

		u32 final_counter = 0;
		for (u32 i = 0; i < FraudSuper_God1; ++i) {
			if (fraud_spawn.used[i]) {
				++final_counter;
			}
		}
		if (!alive_count && !final_counter) {
			for (u32 i = 1; i < 7; ++i) {
				place_fraud(i, FraudClass_God);
			}
		}

		// Unit Spawning
		for (u32 i = 1; i < 7; ++i) {
			FraudSpawnSequence* sequence = &fraud_spawn.sequence[i - 1];
			if (sequence->location) {
				if (sequence->unit_timer) {
					--sequence->unit_timer;
				}
				if (!sequence->unit_timer) {
					if (sequence->unit_index < fraud_spawn.units[sequence->fraud][sequence->race].size() && sequence->location->sources.size()) {
						FraudSpawnSource* source = &sequence->location->sources[sequence->source_index % sequence->location->sources.size()];
						u32 unit_id = fraud_spawn.units[sequence->fraud][sequence->race][sequence->unit_index].unit_id;
						if (source->rallies.size()) {
							Point16 source_pos = source->pos;
							Point16 rally = source->rallies[sequence->rally_index % source->rallies.size()];

							Unit* augur = scbw::createUnitInGame(UnitId::ProtossAugur, i, source_pos.x, source_pos.y);
							if (augur) {
								augur->remove();
							}
							Unit* unit = helper::AI_CreateUnit(source_pos.x, source_pos.y, unit_id, i);
							if (unit) {
								if (units_dat::BaseProperty[unit_id] & UnitProperty::Worker) {
									if (!sequence->townhall_placed) {
										sequence->townhall_placed = true;

										// Todo: make sure this counts properly
										resources->cumulativeMinerals[i] += 400;
										resources->minerals[i] += 400;

										// Todo: make building work
										unit->userActionFlags |= 0x1;
										if (unit_id == UnitId::TerranMason) {
											unit->order(OrderId::BuildTerran, sequence->location->pos.x, sequence->location->pos.y, 0, UnitId::TerranCommandCenter, true);
										} if (unit_id == UnitId::ProtossScribe) {
											unit->order(OrderId::BuildProtoss1, sequence->location->pos.x, sequence->location->pos.y, 0, UnitId::ProtossNexus, true);
										} if (unit_id == UnitId::ZergDroleth) {
											unit->order(OrderId::DroneLand, sequence->location->pos.x, sequence->location->pos.y, 0, UnitId::ZergHatchery, true);
										}
										prepareForNextOrder(unit);
									}
								} else {
									unit->orderTo(OrderId::AttackMove, rally.x, rally.y);
								}

								sequence->unit_timer = 24 * 0.8;
								if (fraud_spawn.units[sequence->fraud][sequence->race][sequence->unit_index].count) {
									--fraud_spawn.units[sequence->fraud][sequence->race][sequence->unit_index].count;
								}
								if (!fraud_spawn.units[sequence->fraud][sequence->race][sequence->unit_index].count) {
									++sequence->unit_index;
								}
							}
						}
						++sequence->source_index;
						if (!(units_dat::BaseProperty[unit_id] & UnitProperty::Worker)) {
							if (sequence->source_index % sequence->location->sources.size() == 0) {
								++sequence->rally_index;
							}
						}
					} else {
						sequence->location = 0;
						if (sequence->revealer) {
							sequence->revealer->remove();
							sequence->revealer = 0;
						}
					}
				}
			}
		}
	}
}