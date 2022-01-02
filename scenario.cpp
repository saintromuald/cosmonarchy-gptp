#include "scenario.h"
#include "unit_search.h"
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
	A new member of this force arrives every five minutes.Each run has a unique order of arrival.
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
	Fraud_Zeratool,                     // --   Profane + Timing    --    <10> Anthelion
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
	FraudSuper_God,                     // --    General + Timing   --    <06> Mind Tyrant

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

	FraudGod_Final = FraudSuper_God,

	FraudClass_Fraud = 0,
	FraudClass_Hero,
	FraudClass_Super,
	FraudClass_God,
	FraudClass_Count,
};

struct Fraud {
	u32 personality;
	u32 build_type;
	u32 build_order;
	u32 color;
};

struct FraudSpawnLocation {
	bool set;
	Point16 pos;
	std::vector<Point16> sources;
	std::vector<Point16> rallies;
};

struct FraudSpawn {
	// Fraud Spawning
	Fraud frauds[Fraud_Count][RaceId::Count];
	u32 used_count;
	bool used[Fraud_Count];
	u32 timer[FraudClass_Count];
	u32 player;
	u32 fraud;
	u32 race;

	// Unit Spawning
	u32 unit_timer;
	u32 unit_index;
	FraudSpawnLocation* location;
	std::vector<FraudSpawnLocation> spawns;
	std::vector<u8> units[Fraud_Count][RaceId::Count];
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

Point16 get_closest_source(FraudSpawnLocation* location, Point16 pos) {
	Point16 closest_loc = pos;
	f32 lowest_distance = 999999999;

	for (Point16 loc : location->sources) {
		f32 distance = scbw::getDistanceFast(loc, pos);
		if (distance < lowest_distance) {
			closest_loc = loc;
			lowest_distance = distance;
		}
	}

	return closest_loc;
}

void init_fraud_spawns() {
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = unit->player_link.next) {
		if (unit->id == UnitId::TerranCommandCenter) {
			FraudSpawnLocation spawn = {};
			spawn.set = true;
			spawn.pos = unit->position;
			fraud_spawn.spawns.push_back(spawn);
			unit->remove();
		}
	}
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = unit->player_link.next) {
		FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
		if (spawn) {
			if (unit->id == UnitId::ProtossPylon) {
				spawn->sources.push_back(unit->position);
				unit->remove();
			} else if (unit->id == UnitId::TerranMaverick) {
				spawn->rallies.push_back(unit->position);
				unit->remove();
			}
		}
	}
}

void add_fraud_units(u32 fraud, u32 race, u8 unit, u32 count = 1) {
	assert(race < RaceId::Count);
	assert(fraud < Fraud_Count);
	assert(unit < UnitId::None);
	for (u32 i = 0; i < count; ++i) {
		fraud_spawn.units[fraud][race].push_back(unit);
	}
}

void init_fraud_units() {
	// Todo: Add units for all personalities

	// Veeq7
	fraud_spawn.frauds[Fraud_Veeq7][RaceId::Protoss] = {0, 0, 0, PlayerColor::Aero};
	add_fraud_units(Fraud_Veeq7, RaceId::Protoss, UnitId::ProtossScribe, 4);
	add_fraud_units(Fraud_Veeq7, RaceId::Protoss, UnitId::ProtossZealot, 10);

	// Pr0nogo
	fraud_spawn.frauds[Fraud_Pr0nogo][RaceId::Protoss] = {0, 0, 0, PlayerColor::Aero};
	add_fraud_units(Fraud_Pr0nogo, RaceId::Protoss, UnitId::ProtossScribe, 4);
	add_fraud_units(Fraud_Pr0nogo, RaceId::Protoss, UnitId::ProtossZealot, 10);
}

bool place_fraud(u32 player, u32 fraud_class) {
	u32 race = 0;
	u32 race_first = 0;
	u32 race_last = 0;
	u32 random_first = 0;
	u32 random_last = 0;
	if (fraud_class == FraudClass_Fraud) {
		random_first = Fraud_RandomFirst;
		random_last = Fraud_RandomLast;
	} else if (fraud_class == FraudClass_God) {
		random_first = FraudGod_Final;
		random_last = FraudGod_Final;
	}
	if (player == 2 || player == 5) { // Terran
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
	} else if (player == 3 || player == 6) { // Protoss
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
	} else if (player == 4 || player == 7) { // Zerg
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
		if (fraud_spawn.used[i]) {
			++race_count;
		}
	}
	for (u32 i = random_first; i <= random_last; ++i) {
		if (fraud_spawn.used[i]) {
			++random_count;
		}
	}

	u32 next_fraud = 0;
	if (race_count && scbw::random() % (race_count + random_count) < race_count) {
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
			fraud_spawn.player = player;
			fraud_spawn.fraud = next_fraud;
			fraud_spawn.race = race;

			fraud_spawn.unit_timer = 0;
			fraud_spawn.unit_index = 0;
			fraud_spawn.location = spawn_location;
			unit_deaths[UnitId::Special_MapRevealer].player[player] = next_fraud;
			return true;
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

		u32 first_fraud = scbw::randBetween(0, 5);
		u32 second_fraud = scbw::randBetween(0, 5);
		while (second_fraud == first_fraud) {
			u32 second_fraud = scbw::randBetween(0, 5);
		}

		init_fraud_spawns();
		init_fraud_units();

		place_fraud(first_fraud, FraudClass_Fraud);
		place_fraud(second_fraud, FraudClass_Fraud);
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
		for (u32 i = 0; i < FraudClass_Count; ++i) {
			char buffer[64];
			format_time(buffer, 64, fraud_spawn.timer[i]);
			hooks::drawText(buffer, 10, 3, 100 + 100 * i, 15, 7);
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
			if (alive_count && fraud_spawn.timer) {
				--fraud_spawn.timer[i];
			} else {
				fraud_spawn.timer[i] = 0;
			}
			if (!fraud_spawn.timer[i] && !fraud_spawn.location) {
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

		// Unit Spawning
		if (fraud_spawn.location) {
			if (fraud_spawn.unit_timer) {
				--fraud_spawn.unit_timer;
			}
			if (!fraud_spawn.unit_timer) {
				if (fraud_spawn.unit_index < fraud_spawn.units[fraud_spawn.fraud][fraud_spawn.race].size() && fraud_spawn.location->rallies.size()) {
					Point16 rally = fraud_spawn.location->rallies[fraud_spawn.unit_index % fraud_spawn.location->rallies.size()];
					Point16 source = get_closest_source(fraud_spawn.location, rally);

					Unit* augur = scbw::createUnitInGame(UnitId::ProtossAugur, fraud_spawn.player, source.x, source.y);
					if (augur) {
						augur->remove();
					}
					Unit* unit = scbw::createUnitInGame(fraud_spawn.units[fraud_spawn.fraud][fraud_spawn.race][fraud_spawn.unit_index], fraud_spawn.player, source.x, source.y);
					if (unit) {
						unit->order(OrderId::AttackMove, rally.x, rally.y, 0, 0, true);

						fraud_spawn.unit_timer = 24 * 0.8;
						++fraud_spawn.unit_index;
					}
				} else {
					fraud_spawn.location = 0;
				}
			}
		}
	}
}