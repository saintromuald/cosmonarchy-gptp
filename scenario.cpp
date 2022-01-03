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
	Fraud_AlexeiUkov,					// --	Orbital + Timing						--	<06> Autocrat
	Fraud_UllenBankOnMeBigmunney,		// --	Armor + Greed							--	<18> Blackjack
	Fraud_knightoftherealm,				// --	Raider + Rush							--	<1A> Maverick
	Fraud_KKKhris,						// --	General + Timing						--	<15> Cyprian
	Fraud_Dynamo,						// --	Raider + Timing							--	<11> Penumbra
	Fraud_TodsDadPat,					// --	Armor + Rush							--	<1B> Phalanx
	Fraud_Piglet,						// --	Shadow + Timing							--	<10> Cohort
	Fraud_DarkenedFantasies,			// --	Armor + Timing							--	<18> Goliath
	Fraud_CustomerSupporter,			// --	Orbital + Greed							--	<0F> Cyprian

	// == PROTOSS ==
	Fraud_BitcoinBoris,					// --	Profane + Rush							--	<15> Nexus
	Fraud_CryptoCarl,					// --	Skylord + Timing						--	<1B> Atreus
	Fraud_Veeq7,						// --	Ranger + Timing							--	<1A> Didact
	Fraud_jun3hong,						// --	Templar + Timing						--	<1B> Envoy
	Fraud_DigitalRightsManager,			// --	Templar + Rush							--	<05> Amaranth
	Fraud_Zeratool,                     // --   Profane + Timing						--  <10> Anthelion
	Fraud_FRAUDGrandpaOfTony,			// --	General + Rush							--	<03> Cantavis
	Fraud_DK,							// --	Skylord + Greed							--	<1F> Panoptus
	Fraud_iquare,						// --	Ranger + Timing							--	<03> Accantor

	// == ZERG ==
	Fraud_CerebrateStalin,				// --	Miscreant + Timing						--	<10> Anticthon
	Fraud_AndyRoseUh,					// --	Assault + Timing						--	<10> Konvilisk
	Fraud_Winnie,						// --	Swarm + Greed							--	<03> Hatchery
	Fraud_A,							// --	Miscreant + Rush						--	<18> Zethrokor
	Fraud_SelloutSam,					// --	Swarm + Timing							--	<1B> Rilirokor
	Fraud_PlagiaristPaul,				// --	Assault + Greed							--	<15> Matraleth
	Fraud_RimTechCombot,				// --	Swarm + Timing							--	<10> Shaman
	Fraud_AfricanizedCrowkenyan,		// --	Sunblot + Rush							--	<05> Start Location
	Fraud_Ultralisks,					// --	Swarm + Timing							--	<1F> Ultrokor

	// == RANDOM ==
	Fraud_LightenedRealities,			// --	General + Rush							--	<03> Augur
	Fraud_AchromaticHalfdreams,			// --	General + Timing						--	<18> Wendigo
	Fraud_TonyTron,						// --	Orbital / Sunblot / Skylord + Greed		--	<0E> Olympian
	Fraud_Pr0nogo,						// --	Armor / Assault / Profane + Timing		--	<1A> Seraph
	Fraud_Ba3lethal,					// --	Raider / Assault / Templar + Rush		--	<11> Eidolon
	Fraud_ToweringPigprophet,	 		// --	Orbital / Sunblot / Skylord + Timing	--	<15> Ovileth
	Fraud_AnalCrowsummoner,				// --	Shadow / Sunblot / Ranger + Greed		--	<05> Mutalisk
	Fraud_PissingPorncultist,			// --	Shadow / Miscreant / Profane + Rush		--	<03> Cyprian
	Fraud_BearborneArkanianpussy,		// --	Shadow / Swarm / Templar + Timing		--	<1D> Ultrokor
	Fraud_DoublingWuckdevourer,			// --	Armor / Swarm / Templar + Greed			--	<0E> Droleth
	Fraud_MongoloidPointcorruptor,		// --	Raider / Assault / Ranger + Timing		--	<1F> Cohort
	Fraud_CalendaredRageboy,			// --	Orbital / Assault / Skylord + Rush		--	<06> Zealot
	Fraud_WorthyPizzasniffer,			// --	Raider / Swarm / Templar + Rush			--	<01> Goliath
	Fraud_SkeletalTodzombie,			// --	Armor / Miscreant / Profane + Timing	--	<1A> Sovroleth
	Fraud_HoopThrower,					// --	Raider / Miscreant / Templar + Timing	--	<07> Mason
	Fraud_Veeq8,						// --	Orbital / Swarm / Ranger + Timing		--	<1A> Didact
	Fraud_Bonzibuddy,					// --	General + Greed							--	<16> Command Center
	Fraud_IskatuMesk,					// --	Shadow / Assault / Profane + Rush		--	<15> Mutalisk

	/*
	-- The Hall of Heroes
	All members of this type are allied to one another, and deploy with additional starting units and resources.
	A hero can first arrive after 2 standard spawns. Each time a hero is not spawned, the chance to spawn a hero increases by 25%.
	When a hero spawns, another one cannot arrive until after 1 spawn, after which the chance returns to the default 25%.
	*/
	// == TERRAN ==
	FraudHero_Francisco_Goska,          // --    Raider + Timing						-- <11> Eidolon
	FraudHero_Ackmed_Clone,             // --    Raider + Rush							-- <11> Harakan
	FraudHero_Cullen,                   // --    Orbital + Rush							-- <0F> Blackjack
	FraudHero_SteveMiller,				// --	 Raider + Rush							-- <16> Penumbra

	// == PROTOSS ==
	FraudHero_Torros,                   // --    Templar + Rush							-- <0F> Amaranth
	FraudHero_Drakkar,					// --    Ranger + Timing						-- <0F> Atreus
	FraudHero_Thalazim,					// --    Profane + Rush							-- <05> Cabalist
	FraudHero_EchoOfMixplay,			// --	 General + Timing						-- <1F> TBD

	// == ZERG ==
	FraudHero_Stroluum,                 // --    Swarm + Rush							-- <11> Isthrathaleth
	FraudHero_Tuzor,                    // --    Miscreant + Greed						-- <11> Anticthon
	FraudHero_Charlie,                  // --    Assault + Timing						-- <16> Axitrilisk
	FraudHero_RektSon,                  // --    General + Rush							-- <11> Lakizilisk

	/*
	-- The Supreme Kings
	All members of this type are hostile to one another, deploy with additional starting units and resources, and have an increased resource multiplier.
	A super first arrives after 8 spawns. Each time a super is not spawned, the chance to spawn a super increases by 15%.
	When a super spawns, another one cannot arrive until after 3 spawns, after which the chance returns to the default 20%.
	*/
	// == TERRAN ==
	FraudSuper_Ackmed,                  // --    Raider + Rush							-- <10> Harakan

	// == PROTOSS ==
	FraudSuper_Tetragram,               // --    Ranger + Greed							-- <0E> Demiurge

	// == ZERG ==
	FraudSuper_Zerghere,                // --    Swarm + Greed							-- <10> Hatchery


	/*
	-- The Cosmonarch
	The final boss of the level awaits any player who can defeat 33 enemies in a single run.
	The Cosmonarch spawns controlling three distinct players at once, all of which are allied to each other.
	Every three minutes, another hand of The Cosmonarch arrives, to a maxmimum of six.
	Frauds, heroes, and supers can no longer spawn after The Cosmonarch appears.
	*/
	// == RANDOM ==
	FraudSuper_God1,                    // --    General + Timing						-- <06> Mind Tyrant
	FraudSuper_God2,                    // --    General + Timing						-- <06> Mind Tyrant
	FraudSuper_God3,                    // --    General + Timing						-- <06> Mind Tyrant
	FraudSuper_God4,                    // --    General + Timing						-- <06> Mind Tyrant
	FraudSuper_God5,                    // --    General + Timing						-- <06> Mind Tyrant
	FraudSuper_God6,                    // --    General + Timing						-- <06> Mind Tyrant

	Fraud_Count,

	Fraud_TerranFirst = Fraud_AlexeiUkov,
	Fraud_TerranLast = Fraud_CustomerSupporter,
	Fraud_ProtossFirst = Fraud_BitcoinBoris,
	Fraud_ProtossLast = Fraud_iquare,
	Fraud_ZergFirst = Fraud_CerebrateStalin,
	Fraud_ZergLast = Fraud_Ultralisks,
	Fraud_RandomFirst = Fraud_LightenedRealities,
	Fraud_RandomLast = Fraud_IskatuMesk,

	FraudHero_TerranFirst = FraudHero_Francisco_Goska,
	FraudHero_TerranLast = FraudHero_SteveMiller,
	FraudHero_ProtossFirst = FraudHero_Torros,
	FraudHero_ProtossLast = FraudHero_EchoOfMixplay,
	FraudHero_ZergFirst = FraudHero_Stroluum,
	FraudHero_ZergLast = FraudHero_RektSon,

	FraudSuper_TerranFirst = FraudSuper_Ackmed,
	FraudSuper_TerranLast = FraudSuper_Ackmed,
	FraudSuper_ProtossFirst = FraudSuper_Tetragram,
	FraudSuper_ProtossLast = FraudSuper_Tetragram,
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
	u32 fraud_class;
	u32 fraud;
	u32 race;
	u32 unit_timer;
	u32 source_index;
	u32 rally_index;
	u32 unit_index;
	void* ai_script;
	FraudSpawnLocation* location;
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
	// Todo: remove units properly
	Unit* next = 0;
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::TerranCommandCenter) {
			FraudSpawnLocation spawn = {};
			spawn.set = true;
			spawn.pos = unit->position;
			fraud_spawn.spawns.push_back(spawn);

			helper::hideAndDisableUnit(unit);
			//unit->userActionFlags |= 4;
			//unit->remove();
		}
	}
	for (Unit* unit = firstPlayerUnit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::ProtossPylon) {
			FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
			if (spawn) {
				spawn->sources.push_back({unit->position});
			}

			helper::hideAndDisableUnit(unit);
			//unit->userActionFlags |= 4;
			//unit->remove();
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

			helper::hideAndDisableUnit(unit);
			//unit->userActionFlags |= 4;
			//unit->remove();
		}
	}
}

void init_fraud_units() {
	for (u32 i = 0; i < Fraud_Count; ++i) {
		fraud_spawn.units[i][RaceId::Terran].push_back({8, UnitId::TerranMason});
		fraud_spawn.units[i][RaceId::Terran].push_back({3, UnitId::ZergOvileth});
		fraud_spawn.units[i][RaceId::Protoss].push_back({8, UnitId::ProtossScribe});
		fraud_spawn.units[i][RaceId::Protoss].push_back({3, UnitId::ZergOvileth});
		fraud_spawn.units[i][RaceId::Zerg].push_back({8, UnitId::ZergDroleth});
		fraud_spawn.units[i][RaceId::Zerg].push_back({3, UnitId::ZergOvileth});
	}

	// Todo: Finish adding units for all personalities

	// Alexei Ukov
	fraud_spawn.frauds[Fraud_AlexeiUkov][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VividRed};
	fraud_spawn.units[Fraud_AlexeiUkov][RaceId::Terran].push_back({8, UnitId::TerranMason});
	fraud_spawn.units[Fraud_AlexeiUkov][RaceId::Terran].push_back({5, UnitId::TerranMaverick});
	fraud_spawn.units[Fraud_AlexeiUkov][RaceId::Terran].push_back({2, UnitId::TerranGorgon});
	fraud_spawn.units[Fraud_AlexeiUkov][RaceId::Terran].push_back({1, UnitId::TerranWyvern});

	// Ullen "Bank on Me" Bigmunney
	fraud_spawn.frauds[Fraud_UllenBankOnMeBigmunney][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_None, AiBuildOrder_TerranArmor_IronCrusade, PlayerColor::VineGreen};
	fraud_spawn.units[Fraud_UllenBankOnMeBigmunney][RaceId::Terran].push_back({4, UnitId::TerranMason});
	fraud_spawn.units[Fraud_UllenBankOnMeBigmunney][RaceId::Terran].push_back({2, UnitId::TerranGoliath});
	fraud_spawn.units[Fraud_UllenBankOnMeBigmunney][RaceId::Terran].push_back({2, UnitId::TerranPhalanxTankMode});

	// knightoftherealm
	fraud_spawn.frauds[Fraud_knightoftherealm][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Aero};
	fraud_spawn.units[Fraud_knightoftherealm][RaceId::Terran].push_back({4, UnitId::TerranMaverick});
	fraud_spawn.units[Fraud_knightoftherealm][RaceId::Terran].push_back({4, UnitId::TerranCyprian});
	fraud_spawn.units[Fraud_knightoftherealm][RaceId::Terran].push_back({1, UnitId::TerranShaman});

	// KKKhris
	fraud_spawn.frauds[Fraud_KKKhris][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brown};
	fraud_spawn.units[Fraud_KKKhris][RaceId::Terran].push_back({4, UnitId::TerranMaverick});
	fraud_spawn.units[Fraud_KKKhris][RaceId::Terran].push_back({2, UnitId::TerranPhalanxTankMode});
	fraud_spawn.units[Fraud_KKKhris][RaceId::Terran].push_back({2, UnitId::TerranWraith});

	// Dynamo
	fraud_spawn.frauds[Fraud_Dynamo][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::ElectricOrange};
	fraud_spawn.units[Fraud_Dynamo][RaceId::Terran].push_back({4, UnitId::TerranMadcap});
	fraud_spawn.units[Fraud_Dynamo][RaceId::Terran].push_back({1, UnitId::TerranAutocrat});

	// TodsDadPat
	fraud_spawn.frauds[Fraud_TodsDadPat][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::SilverPink};
	fraud_spawn.units[Fraud_TodsDadPat][RaceId::Terran].push_back({2, UnitId::TerranPaladin});

	// Piglet
	fraud_spawn.frauds[Fraud_Piglet][RaceId::Terran] = {AiPersonality_TerranShadow, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	fraud_spawn.units[Fraud_Piglet][RaceId::Terran].push_back({2, UnitId::TerranBlackjack});
	fraud_spawn.units[Fraud_Piglet][RaceId::Terran].push_back({2, UnitId::SharedCohort});
	fraud_spawn.units[Fraud_Piglet][RaceId::Terran].push_back({2, UnitId::TerranSiren});

	// DarkenedFantasies
	fraud_spawn.frauds[Fraud_DarkenedFantasies][RaceId::Terran] = { AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSpringGreen};
	fraud_spawn.units[Fraud_DarkenedFantasies][RaceId::Terran].push_back({2, UnitId::TerranVulture});
	fraud_spawn.units[Fraud_DarkenedFantasies][RaceId::Terran].push_back({2, UnitId::TerranCyclops});
	fraud_spawn.units[Fraud_DarkenedFantasies][RaceId::Terran].push_back({2, UnitId::TerranGoliath});

	// Customer Supporter
	fraud_spawn.frauds[Fraud_CustomerSupporter][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Turquoise};
	fraud_spawn.units[Fraud_CustomerSupporter][RaceId::Terran].push_back({2, UnitId::TerranGorgon});
	fraud_spawn.units[Fraud_CustomerSupporter][RaceId::Terran].push_back({1, UnitId::TerranSalamander});

	// Bitcoin Boris
	fraud_spawn.frauds[Fraud_BitcoinBoris][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Brown};
	fraud_spawn.units[Fraud_BitcoinBoris][RaceId::Protoss].push_back({4, UnitId::ProtossBarghest});
	fraud_spawn.units[Fraud_BitcoinBoris][RaceId::Protoss].push_back({1, UnitId::SharedCharlatan});
	fraud_spawn.units[Fraud_BitcoinBoris][RaceId::Protoss].push_back({2, UnitId::SharedStriga});

	// Crypto Carl
	fraud_spawn.frauds[Fraud_CryptoCarl][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_None, AiBuildOrder_ProtossSkylord_SolarEscort, PlayerColor::BistreBrown};
	fraud_spawn.units[Fraud_CryptoCarl][RaceId::Protoss].push_back({2, UnitId::ProtossPanoptus});
	fraud_spawn.units[Fraud_CryptoCarl][RaceId::Protoss].push_back({1, UnitId::ProtossEmpyrean});

	// Veeq7
	fraud_spawn.frauds[Fraud_Veeq7][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_spawn.units[Fraud_Veeq7][RaceId::Protoss].push_back({4, UnitId::ProtossSimulacrum});
	fraud_spawn.units[Fraud_Veeq7][RaceId::Protoss].push_back({2, UnitId::ProtossDracadin});

	// jun3hong
	fraud_spawn.frauds[Fraud_jun3hong][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonCarrot};
	fraud_spawn.units[Fraud_jun3hong][RaceId::Protoss].push_back({4, UnitId::ProtossLegionnaire});
	fraud_spawn.units[Fraud_jun3hong][RaceId::Protoss].push_back({2, UnitId::ProtossHierophant});
	fraud_spawn.units[Fraud_jun3hong][RaceId::Protoss].push_back({1, UnitId::ProtossWitness});

	// Digital Rights Manager
	fraud_spawn.frauds[Fraud_DigitalRightsManager][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Glacial};
	fraud_spawn.units[Fraud_DigitalRightsManager][RaceId::Protoss].push_back({3, UnitId::ProtossAmaranth});
	fraud_spawn.units[Fraud_DigitalRightsManager][RaceId::Protoss].push_back({2, UnitId::ProtossAtreus});

	// Zeratool
	fraud_spawn.frauds[Fraud_Zeratool][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TyrianPurple};
	fraud_spawn.units[Fraud_Zeratool][RaceId::Protoss].push_back({4, UnitId::ProtossCabalist});
	fraud_spawn.units[Fraud_Zeratool][RaceId::Protoss].push_back({2, UnitId::ProtossMindTyrant});

	// F.R.A.U.D. Grandpa of Tony
	fraud_spawn.frauds[Fraud_FRAUDGrandpaOfTony][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Yellow};
	fraud_spawn.units[Fraud_FRAUDGrandpaOfTony][RaceId::Protoss].push_back({4, UnitId::ProtossZealot});
	fraud_spawn.units[Fraud_FRAUDGrandpaOfTony][RaceId::Protoss].push_back({2, UnitId::ProtossDracadin});

	// DK
	fraud_spawn.frauds[Fraud_DK][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::EggshellBlue};
	fraud_spawn.units[Fraud_DK][RaceId::Protoss].push_back({4, UnitId::ProtossAurora});
	fraud_spawn.units[Fraud_DK][RaceId::Protoss].push_back({2, UnitId::ProtossPanoptus});

	// iquare
	fraud_spawn.frauds[Fraud_iquare][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brass};
	fraud_spawn.units[Fraud_iquare][RaceId::Protoss].push_back({2, UnitId::ProtossIdol});
	fraud_spawn.units[Fraud_iquare][RaceId::Protoss].push_back({2, UnitId::ProtossAccantor});
	fraud_spawn.units[Fraud_iquare][RaceId::Protoss].push_back({1, UnitId::ProtossArchitect});

	// Cerebrate Stalin
	fraud_spawn.frauds[Fraud_CerebrateStalin][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Magma};
	fraud_spawn.units[Fraud_CerebrateStalin][RaceId::Zerg].push_back({4, UnitId::SharedCohort});
	fraud_spawn.units[Fraud_CerebrateStalin][RaceId::Zerg].push_back({2, UnitId::SharedAxitrilisk});

	// Andy Rose Uh
	fraud_spawn.frauds[Fraud_AndyRoseUh][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Orange};
	fraud_spawn.units[Fraud_AndyRoseUh][RaceId::Zerg].push_back({4, UnitId::ZergHydralisk});
	fraud_spawn.units[Fraud_AndyRoseUh][RaceId::Zerg].push_back({2, UnitId::ZergMutalisk});

	// Winnie
	fraud_spawn.frauds[Fraud_Winnie][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Yellow};
	fraud_spawn.units[Fraud_Winnie][RaceId::Zerg].push_back({8, UnitId::ZergZethrokor});
	fraud_spawn.units[Fraud_Winnie][RaceId::Zerg].push_back({4, UnitId::ZergQuazilisk});

	// A
	fraud_spawn.frauds[Fraud_A][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::ForestGreen};
	fraud_spawn.units[Fraud_A][RaceId::Zerg].push_back({8, UnitId::ZergZethrokor});
	fraud_spawn.units[Fraud_A][RaceId::Zerg].push_back({4, UnitId::SharedCohort});

	// Sellout Sam
	fraud_spawn.frauds[Fraud_SelloutSam][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Beaver};
	fraud_spawn.units[Fraud_SelloutSam][RaceId::Zerg].push_back({6, UnitId::ZergZethrokor});
	fraud_spawn.units[Fraud_SelloutSam][RaceId::Zerg].push_back({2, UnitId::ZergVorvrokor});
	fraud_spawn.units[Fraud_SelloutSam][RaceId::Zerg].push_back({3, UnitId::ZergLiiralisk});

	// Plagiarist Paul
	fraud_spawn.frauds[Fraud_PlagiaristPaul][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::ZinnwalditeBrown};
	fraud_spawn.units[Fraud_PlagiaristPaul][RaceId::Zerg].push_back({6, UnitId::ZergHydralisk});
	fraud_spawn.units[Fraud_PlagiaristPaul][RaceId::Zerg].push_back({2, UnitId::ZergKonvilisk});

	// RimTechCombot
	fraud_spawn.frauds[Fraud_RimTechCombot][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::DarkLavender};
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({6, UnitId::ZergZethrokor});
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({3, UnitId::ZergQuazilisk});
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({6, UnitId::ZergNathrokor});

	// Africanized Crowkenyan
	fraud_spawn.frauds[Fraud_AfricanizedCrowkenyan][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::BurntUmber};
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({8, UnitId::ZergNathrokor});
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({2, UnitId::ZergSkithrokor});
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({2, UnitId::ZergMutalisk});

	// Ultralisks
	fraud_spawn.frauds[Fraud_Ultralisks][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Glacial};
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({2, UnitId::ZergLakizilisk});
	fraud_spawn.units[Fraud_RimTechCombot][RaceId::Zerg].push_back({2, UnitId::ZergUltrokor});

	// LightenedRealities
	fraud_spawn.frauds[Fraud_LightenedRealities][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};


	fraud_spawn.frauds[Fraud_LightenedRealities][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};


	fraud_spawn.frauds[Fraud_LightenedRealities][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};


	// AchromaticHalfdreams
	fraud_spawn.frauds[Fraud_AchromaticHalfdreams][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};


	fraud_spawn.frauds[Fraud_AchromaticHalfdreams][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};


	fraud_spawn.frauds[Fraud_AchromaticHalfdreams][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};


	// TonyTron
	fraud_spawn.frauds[Fraud_TonyTron][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};


	fraud_spawn.frauds[Fraud_TonyTron][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};


	fraud_spawn.frauds[Fraud_TonyTron][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};


	// Pr0nogo
	fraud_spawn.frauds[Fraud_Pr0nogo][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Terran].push_back({2, UnitId::TerranGoliath});
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Terran].push_back({2, UnitId::TerranPhalanxTankMode});
	
	fraud_spawn.frauds[Fraud_Pr0nogo][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Protoss].push_back({2, UnitId::ProtossZealot});
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Protoss].push_back({2, UnitId::ProtossCabalist});

	fraud_spawn.frauds[Fraud_Pr0nogo][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Zerg].push_back({4, UnitId::ZergHydralisk});
	fraud_spawn.units[Fraud_Pr0nogo][RaceId::Zerg].push_back({2, UnitId::ZergBactalisk});

	// Ba3lethal
	fraud_spawn.frauds[Fraud_Ba3lethal][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};


	fraud_spawn.frauds[Fraud_Ba3lethal][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};


	fraud_spawn.frauds[Fraud_Ba3lethal][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};


	// Towering Pigprophet
	fraud_spawn.frauds[Fraud_ToweringPigprophet][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};


	fraud_spawn.frauds[Fraud_ToweringPigprophet][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};


	fraud_spawn.frauds[Fraud_ToweringPigprophet][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};


	// Anal Crowsummoner
	fraud_spawn.frauds[Fraud_AnalCrowsummoner][RaceId::Terran] = {AiPersonality_TerranShadow, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};


	fraud_spawn.frauds[Fraud_AnalCrowsummoner][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};


	fraud_spawn.frauds[Fraud_AnalCrowsummoner][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};


	// Pissing Porncultist
	fraud_spawn.frauds[Fraud_PissingPorncultist][RaceId::Terran] = {AiPersonality_TerranShadow, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};


	fraud_spawn.frauds[Fraud_PissingPorncultist][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};


	fraud_spawn.frauds[Fraud_PissingPorncultist][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};


	// Bearborne Arkanianpussy
	fraud_spawn.frauds[Fraud_BearborneArkanianpussy][RaceId::Terran] = {AiPersonality_TerranShadow, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};


	fraud_spawn.frauds[Fraud_BearborneArkanianpussy][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};


	fraud_spawn.frauds[Fraud_BearborneArkanianpussy][RaceId::Zerg] = { AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};


	// Doubling Wuckdevourer
	fraud_spawn.frauds[Fraud_DoublingWuckdevourer][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};


	fraud_spawn.frauds[Fraud_DoublingWuckdevourer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};


	fraud_spawn.frauds[Fraud_DoublingWuckdevourer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};


	// Mongoloid Pointcorruptor
	fraud_spawn.frauds[Fraud_MongoloidPointcorruptor][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};


	fraud_spawn.frauds[Fraud_MongoloidPointcorruptor][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};


	fraud_spawn.frauds[Fraud_MongoloidPointcorruptor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};


	// Calendared Rageboy
	fraud_spawn.frauds[Fraud_CalendaredRageboy][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};


	fraud_spawn.frauds[Fraud_CalendaredRageboy][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};


	fraud_spawn.frauds[Fraud_CalendaredRageboy][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};


	// Worthy Pizzasniffer
	fraud_spawn.frauds[Fraud_WorthyPizzasniffer][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};


	fraud_spawn.frauds[Fraud_WorthyPizzasniffer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};


	fraud_spawn.frauds[Fraud_WorthyPizzasniffer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};


	// Skeletal Todzombie
	fraud_spawn.frauds[Fraud_SkeletalTodzombie][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};


	fraud_spawn.frauds[Fraud_SkeletalTodzombie][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};


	fraud_spawn.frauds[Fraud_SkeletalTodzombie][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};


	// Hoop Thrower
	fraud_spawn.frauds[Fraud_HoopThrower][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VineGreen};


	fraud_spawn.frauds[Fraud_HoopThrower][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VineGreen};


	fraud_spawn.frauds[Fraud_HoopThrower][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VineGreen};

	// Veeq8
	fraud_spawn.frauds[Fraud_Veeq8][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_spawn.units[Fraud_Veeq8][RaceId::Terran].push_back({4, UnitId::TerranWraith});
	fraud_spawn.units[Fraud_Veeq8][RaceId::Terran].push_back({2, UnitId::TerranSalamander});

	fraud_spawn.frauds[Fraud_Veeq8][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_spawn.units[Fraud_Veeq8][RaceId::Protoss].push_back({6, UnitId::ProtossSimulacrum});
	fraud_spawn.units[Fraud_Veeq8][RaceId::Protoss].push_back({1, UnitId::ProtossClarion});

	fraud_spawn.frauds[Fraud_Veeq8][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_spawn.units[Fraud_Veeq8][RaceId::Zerg].push_back({8, UnitId::ZergZethrokor});
	fraud_spawn.units[Fraud_Veeq8][RaceId::Zerg].push_back({4, UnitId::SharedCohort});

	// Bonzibuddy
	fraud_spawn.frauds[Fraud_Bonzibuddy][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};


	fraud_spawn.frauds[Fraud_Bonzibuddy][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};


	fraud_spawn.frauds[Fraud_Bonzibuddy][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};


	// IskatuMesk
	fraud_spawn.frauds[Fraud_IskatuMesk][RaceId::Terran] = {AiPersonality_TerranShadow, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};


	fraud_spawn.frauds[Fraud_IskatuMesk][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};


	fraud_spawn.frauds[Fraud_IskatuMesk][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};


	// Francisco Goska
	fraud_spawn.frauds[FraudHero_Francisco_Goska][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::ElectricOrange};


	// Ackmed Clone
	fraud_spawn.frauds[FraudHero_Ackmed_Clone][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::BurntUmber};


	// Cullen
	fraud_spawn.frauds[FraudHero_Cullen][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::BdazzledBlue};


	// Steve Miller
	fraud_spawn.frauds[FraudHero_SteveMiller][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::White};


	// Torros
	fraud_spawn.frauds[FraudHero_Torros][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Saffron};
	

	// Drakkar
	fraud_spawn.frauds[FraudHero_Drakkar][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Emerald};


	// Thalazim
	fraud_spawn.frauds[FraudHero_Thalazim][RaceId::Protoss] = {AiPersonality_ProtossProfane, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Saffron};


	// Echo of Mixplay
	fraud_spawn.frauds[FraudHero_Thalazim][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};


	// Stroluum
	fraud_spawn.frauds[FraudHero_Stroluum][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::FernGreen};


	// Tuzor
	fraud_spawn.frauds[FraudHero_Tuzor][RaceId::Zerg] = {AiPersonality_ZergMiscreant, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::VividOrange};

	
	// Charlie
	fraud_spawn.frauds[FraudHero_Charlie][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};


	// Rekt'Son
	fraud_spawn.frauds[FraudHero_RektSon][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentViolet};


	// Ackmed
	fraud_spawn.frauds[FraudSuper_Ackmed][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::BurntUmber};


	// Tetragram
	fraud_spawn.frauds[FraudSuper_Tetragram][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};


	// Zerghere
	fraud_spawn.frauds[FraudSuper_Zerghere][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::DarkLavender};


	// God - TBD
}

void run_ai(u32 player, Point16 pos, s32 name) {
	Box32 area;
	area.left = pos.x - 32 * 2;
	area.right = pos.x + 32 * 2;
	area.top = pos.y - 32 * 1.5;
	area.bottom = pos.y + 32 * 1.5;
	s32 is_bw;
	s32 block = helper::parseAIScriptName(name, &is_bw);
	if (block) {
		fraud_spawn.sequence[player].ai_script = helper::AI_RunAIScript(&area, player, block, is_bw);
	}
}

bool place_fraud(u32 player, u32 fraud_class, bool spawn_townhall) {
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

				unit_deaths[UnitId::Spell_DisruptionWeb].player[player] = fraud_spawn.frauds[next_fraud][race].personality;
				unit_deaths[UnitId::ProtossPlasmaShell].player[player] = fraud_spawn.frauds[next_fraud][race].build_type;
				unit_deaths[UnitId::Spell_DarkSwarm].player[player] = fraud_spawn.frauds[next_fraud][race].build_order;
				SetPlayerColor(player, fraud_spawn.frauds[next_fraud][race].color);

				// Todo: reset kills using aise
				resources->cumulativeMinerals[player] += 400;
				resources->cumulativeGas[player] += 200;
				resources->minerals[player] = 400;
				resources->gas[player] = 200;

				if (spawn_townhall) {
					u32 townhall = UnitId::TerranCommandCenter;
					if (player == 2 || player == 5) townhall = UnitId::ProtossNexus;
					if (player == 3 || player == 6) townhall = UnitId::ZergHatchery;

					Point16 pos = fraud_spawn.sequence[player-1].location->pos;
					Unit* augur = scbw::createUnitInGame(UnitId::ProtossAugur, player, pos.x, pos.y);
					if (augur) {
						augur->remove();
					}
					Unit* unit = scbw::createUnitInGame(townhall, player, pos.x, pos.y);
					fraud_spawn.sequence[player-1].unit_timer = 24 * 0.8;
				}

				if (race == RaceId::Terran) {
					run_ai(player, spawn_location->pos, 'xCMT');
				} else if (race == RaceId::Protoss) {
					run_ai(player, spawn_location->pos, 'xCMP');
				} else if (race == RaceId::Zerg) {
					run_ai(player, spawn_location->pos, 'xCMZ');
				}

				for (u32 i = 1; i < 7; ++i) {
					if (player == i) {
						continue;
					}
					if (fraud_spawn.sequence[player-1].fraud_class == fraud_spawn.sequence[i-1].fraud_class &&
						(fraud_spawn.sequence[player-1].fraud_class == FraudClass_Hero || fraud_spawn.sequence[player-1].fraud_class == FraudClass_God)) {
						playerAlliance[player].flags[i] = true;
						playerAlliance[i].flags[player] = true;
						playerVision[player].flags[i] = true;
						playerVision[i].flags[player] = true;
					} else {
						playerAlliance[player].flags[i] = false;
						playerAlliance[i].flags[player] = false;
						playerVision[player].flags[i] = false;
						playerVision[i].flags[player] = false;
					}
				}

				scbw::minimapPing(spawn_location->pos.x, spawn_location->pos.y, 0, *LOCAL_NATION_ID);
				playerVision[0].flags[player] = true;

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
		place_fraud(first_fraud, FraudClass_Fraud, true);
		place_fraud(second_fraud, FraudClass_Fraud, true);
	}	
}

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
			if (fraud_spawn.sequence[i-1].location || all_factory_counts->unit[i] > 0) {
				++alive_count;
			}
		}
		
		if (!alive_count && fraud_spawn.used_count == Fraud_Count) {
			*currentTriggerPlayerId = 0;
			helper::Victory();
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
					if (all_factory_counts->unit[player] == 0) {
						if (place_fraud(player, i, false)) {
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
				place_fraud(i, FraudClass_God, true);
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
								if (!(units_dat::BaseProperty[unit_id] & UnitProperty::Worker)) {
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
						playerVision[0].flags[i] = false;
						sequence->location = 0;
					}
				}
			}
		}
	}
}