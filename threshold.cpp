#include "threshold.h"
#include "unit_search.h"
#include "player_color.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/unit_destructor_special.h"

#include <assert.h>
#include <math.h>

u32 get_transmission_playing() {
	return g_unit_deaths_table[UnitId::Special_PitDoor].player[0];
}

bool is_map(u32 campaign, u32 map) {
	return g_unit_deaths_table[UnitId::Spell_ScannerSweep].player[0] == campaign
		&& g_unit_deaths_table[UnitId::Special_RightPitDoor].player[0] == map;
}

/*
	Bugs:
		V - Frauds occasionally initialize multiple towns (maybe not taking into account spawned town center?)
		V - Unit AI fails to initialize near the unit limit, for some reason
		V - Frauds fail to build while:
			- an old town is still active - needs to be fixed by waiting (for 1 second, probably less) when an incoming fraud uses the same player slot as a defeated fraud
		P/V - Fraud colors are not always accurate (needs more testing)
	Todo:
		V - Limit worker count to 30
			- A more optimal amount would be 2.5 * number of mineral fields + 3.5 * number of geysers, rounding up
			- Maybe Zerg gain +5 flat to account for their building consumption
		V - Add some kind of dialogue system
		V - Make mind controlled units (Mind Tyrant, Empress) run AI_TrainingBroodling or sendsuicide or w/e
		P - Add more AI build orders for more varied fraud behavior
		P - Audit fraud spawns for accuracy

	Proposed dialogue rules:
		1. God introduces the map
			i.		Use bank data to track number of attempts and number of completions
			ii.		Randomize through available lines (varies depending on the above calculation)
		2. The initial frauds are revealed
			i.		Initial fraud 1 plays their spawn line
			ii.		Initial fraud 2 plays their spawn line
			iii.	God has a chance to play a taunting line if the number of attempts is high enough
		3. If an incoming fraud will ally with the player...
			i.		God acknowledges the alliance first, before the fraud spawns
			ii.		The incoming fraud acknowledges the alliance last, right after their spawn line
		4. If an incoming fraud will ally with another fraud...
			i.		God acknowledges the alliance first, before the fraud spawns
			ii.		The established fraud acknowledges the alliance second, after the fraud spawns
			iii.	The incoming fraud acknowledges the alliance last, after the established fraud's line
		5. A fraud spawns,
		6. A fraud and the player do battle for the first time,
		7. A fraud destroys one of the player's bases,
		8. A fraud loses a base to the player,
		9. A fraud kills a large number of the player's units,
		10. A fraud loses a large number of units to the player,
		11. A fraud defeats another fraud,
		12. A fraud loses to another fraud,
		13. A fraud defeats the player,
		14. A fraud loses to the player
			i.		Randomize through available lines (average of 3 per event)
		15. If an incoming fraud is the first hero of the run,
		16. If an incoming fraud is the first super of the run
			i.		God acknowledges the arrival of a hero or super, before they spawn
		17. If an incoming fraud is a hero, and there is another hero on the map...
			i.		God acknowledges the pending alliance, before the hero spawns
		18. When the player destroys a hero for the first time,
		19. When the player destroys a super for the first time
			i.		Depending on how long the hero or super was active, the response from God will change
			ii.		God "congratulates" the player on their achievement (varies depending on the above calculation)
		20. When the player is 1/3rd of the way through,
		21. When the player is 2/3rds of the way through
			i.		Use bank data to track number of attempts and number of completions
			ii.		Track the average amount of uptime for frauds, heroes, and supers, as well as game time and unit deaths, to approximate the challenge the player has faced thus far
			iii.	God acknowledges the player's progress (varies depending on the above calculations)
		22. When the player completes the marathon
			i.		Use bank data to track number of attempts and number of completions
			ii.		Track the average amount of uptime for frauds, heroes, and supers, as well as game time and unit deaths, to approximate the challenge the player has faced thus far
			iii.	God acknowledges the player's progress (varies depending on the above calculations)
			iv.		God spawns, with a line for each hand
		23.	When a new hand spawns
			i.		Use bank data to track number of attempts and number of completions
			ii.		God reaffirms his greatness (varies depending on the above calculations)
		24.	If God defeats the player
			i.		Use bank data to track number of attempts and number of completions
			ii.		God acknowledges the player's failure (varies depending on the above calculations)
		25. If the player defeats God
			i.		Use bank data to track number of attempts and number of completions
			ii.		God acknowledges the player's success (varies depending on the above calculations)
*/

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
	Fraud_Desler,						// --	General + Timing						--	<18> Eidolon

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
	Fraud_Metathrom,					// --	Templar + Timing						--	<0E> Nexus

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
	Fraud_NThrower,						// --	Miscreant + Timing						--	<1F> Cohort

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
	A hero can first arrive after 2 standard spawns. Each time a hero is not spawned, the chance to spawn a hero increases by 33%.
	When a hero spawns, another one cannot arrive until after 1 spawn, after which the chance returns to the default 33%.
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
	A super first arrives after 5 spawns. Each time a super is not spawned, the chance to spawn a super increases by 20%.
	When a super spawns, another one cannot arrive until after 3 spawns, after which the chance returns to the default 20%.
	*/
	// == TERRAN ==
	FraudSuper_Ackmed,                  // --    Raider + Rush							-- <10> Harakan
	FraudSuper_TheDon,					// --	 Orbital + Greed						-- <06> Minotaur
	FraudSuper_Tolrim,					// --	 Armor + Timing							-- <10> Eidolon

	// == PROTOSS ==
	FraudSuper_Tetragram,               // --    Ranger + Greed							-- <0E> Demiurge
	FraudSuper_Amon,					// --	 Profane + Rush							-- <08> Mind Tyrant
	FraudSuper_TheGreatDevourer,		// --	 General + Timing						-- <0E> Accantor

	// == ZERG ==
	FraudSuper_Zerghere,                // --    Swarm + Greed							-- <10> Hatchery
	FraudSuper_Connor,					// --	 Assault + Rush							-- <16> Bactalisk
	FraudSuper_Kagg,					// --	 Timing + Rush							-- <15> Anticthon
	/*
	-- The Cosmonarch
	The final boss of the level awaits any player who can defeat 33 enemies in a single run.
	The Cosmonarch spawns controlling three distinct players at once, all of which are allied to each other.
	Every one minute, another hand of The Cosmonarch arrives, to a maximum of six.
	Frauds, heroes, and supers can no longer spawn after The Cosmonarch appears.
	*/
	// == RANDOM ==
	FraudGod_Hand1,                     // --    General + Timing						-- <06> Mind Tyrant
	FraudGod_Hand2,                     // --    General + Timing						-- <06> Mind Tyrant
	FraudGod_Hand3,                     // --    General + Timing						-- <06> Mind Tyrant
	FraudGod_Hand4,                     // --    General + Timing						-- <06> Mind Tyrant
	FraudGod_Hand5,                     // --    General + Timing						-- <06> Mind Tyrant
	FraudGod_Hand6,                     // --    General + Timing						-- <06> Mind Tyrant

	Fraud_Count,

	FraudGod_Count = 6,

	Fraud_TerranFirst = Fraud_AlexeiUkov,
	Fraud_TerranLast = Fraud_CustomerSupporter,
	Fraud_ProtossFirst = Fraud_BitcoinBoris,
	Fraud_ProtossLast = Fraud_iquare,
	Fraud_ZergFirst = Fraud_CerebrateStalin,
	Fraud_ZergLast = Fraud_Ultralisks,
	Fraud_RandomFirst = Fraud_LightenedRealities,
	Fraud_RandomLast = Fraud_IskatuMesk,

	Fraud_First = Fraud_TerranFirst,
	Fraud_Last = Fraud_RandomLast,

	FraudHero_TerranFirst = FraudHero_Francisco_Goska,
	FraudHero_TerranLast = FraudHero_SteveMiller,
	FraudHero_ProtossFirst = FraudHero_Torros,
	FraudHero_ProtossLast = FraudHero_EchoOfMixplay,
	FraudHero_ZergFirst = FraudHero_Stroluum,
	FraudHero_ZergLast = FraudHero_RektSon,

	FraudHero_First = FraudHero_TerranFirst,
	FraudHero_Last = FraudHero_ZergLast,

	FraudSuper_TerranFirst = FraudSuper_Ackmed,
	FraudSuper_TerranLast = FraudSuper_Tolrim,
	FraudSuper_ProtossFirst = FraudSuper_Tetragram,
	FraudSuper_ProtossLast = FraudSuper_TheGreatDevourer,
	FraudSuper_ZergFirst = FraudSuper_Zerghere,
	FraudSuper_ZergLast = FraudSuper_Kagg,

	FraudSuper_First = FraudSuper_TerranFirst,
	FraudSuper_Last = FraudSuper_ZergLast,

	FraudGod_First = FraudGod_Hand1,
	FraudGod_Last = FraudGod_Hand6,

	FraudClass_None = 0,
	FraudClass_Fraud,
	FraudClass_Hero,
	FraudClass_Super,
	FraudClass_God,
	FraudClass_Count,
};

// Fraud alliances:
// If the following frauds are on the field at the same time, they should ally immediately, with a special dialogue event that plays instead of the newcomer's normal spawn quote.
// Line breaks denote new alliances.
#define AlliancePlayerCap 16
u32 fraud_alliances[][AlliancePlayerCap] = {
	// New Ruska
	{
		Fraud_AlexeiUkov,
		Fraud_CerebrateStalin,
	},

	// Nah, Fuck That!
	{
		Fraud_BitcoinBoris,
		Fraud_CryptoCarl,
		Fraud_Ba3lethal,
	},

	// East Aero Progamers
	{
		Fraud_Veeq7,
		Fraud_knightoftherealm,
	},

	// StarEdit Crusaders
	{
		Fraud_AndyRoseUh,
		Fraud_KKKhris,
	},

	// ColoredVaguery
	{
		Fraud_LightenedRealities,
		Fraud_AchromaticHalfdreams,
	},

	// Of the Utalian Peninsula...
	{
		Fraud_TodsDadPat,
		Fraud_Dynamo,
	},

	// Chinese Translators
	{
		Fraud_Winnie,
		Fraud_Piglet,
	},

	// The Wireframers
	{
		Fraud_jun3hong,
		Fraud_DarkenedFantasies,
	},

	// The 3%
	{
		Fraud_UllenBankOnMeBigmunney,
		Fraud_A,
	},

	// The Vile Voidspeakers
	{
		Fraud_ToweringPigprophet,
		Fraud_AnalCrowsummoner,
		Fraud_PissingPorncultist,
		Fraud_BearborneArkanianpussy,
		Fraud_DoublingWuckdevourer,
		Fraud_MongoloidPointcorruptor,
		Fraud_CalendaredRageboy,
		Fraud_WorthyPizzasniffer,
		Fraud_SkeletalTodzombie,
	},

	// Gatesoft Microbills
	{
		Fraud_CustomerSupporter,
		Fraud_DigitalRightsManager,
	},

	// The Shitfling Sinners
	{
		Fraud_SelloutSam,
		Fraud_PlagiaristPaul,
	},

	// Fraudulent Task Force
	{
		Fraud_TonyTron,
		Fraud_Veeq8,
	},

	// Ego Death
	{
		Fraud_Pr0nogo,
		Fraud_iquare,
	},
};

const char* fraud_names[] = {
	"None",
	//// -- The Legion of Frauds
	// == TERRAN ==
	"\x06\Alexei Ukov",
	"\x18Ullen \"Bank On Me\" Bigmunney",
	"\x1A\knightoftherealm",
	"\x15\KKKhris",
	"\x11\Dynamo",
	"\x1B\TodsDadPat",
	"\x10\Piglet",
	"\x18\DarkenedFantasies",
	"\x0F\Customer Supporter",
	"\x18\Desler",

	// == PROTOSS ==
	"\x15\Bitcoin Boris",
	"\x1B\Crypto Carl",
	"\x1A\Veeq7",
	"\x1B\jun3hong",
	"\x1E\Digital Rights Manager",
	"\x10\Zeratool",
	"\x19\F.R.A.U.D. Grandpa of Tony",
	"\x1F\DK",
	"\x03\iquare",
	"\x0E\Metathrom",

	// == ZERG ==
	"\x11\Cerebrate Stalin",
	"\x11\AndyRoseUh",
	"\x03\Winnie",
	"\x18\A",
	"\x1B\Sellout Sam",
	"\x15\Plagiarist Paul",
	"\x10\RimTechCombot",
	"\x1E\Africanized Crowkenyan",
	"\x16Ultralisks",
	"\x1F\N Thrower",

	// == RANDOM ==
	"\x07\LightenedRealities",
	"\x16\AchromaticHalfdreams",
	"\x0E\TonyTron",
	"\x1A\Pr0nogo",
	"\x11\Ba3lethal",
	"\x1C\Towering Pigprophet",
	"\x1E\Anal Crowsummoner",
	"\x03\Pissing Porncultist",
	"\x1D\Bearborne Arkanianpussy",
	"\x10\Doubling Wuckdevourer",
	"\x1F\Mongoloid Pointcorruptor",
	"\x06\Calendared Rageboy",
	"\x03\Worthy Pizzasniffer",
	"\x16\Skeletal Todzombie",
	"\x18\Hoop Thrower",
	"\x1A\Veeq8",
	"\x10\Bonzibuddy",
	"\x15\IskatuMesk",

	//// -- The Hall of Heroes
	// == TERRAN ==
	"\x11\Francisco Goska",
	"\x15\An Ackmed Clone",
	"\x1E\Cullen",
	"\x16\Steve Miller",

	// == PROTOSS ==
	"\x19\Torros",
	"\x18\Drakkar",
	"\x15\Thalazim",
	"\x0F\Echo of Mixplay",

	// == ZERG ==
	"\x1D\Stroluum",
	"\x10\Tuzor",
	"\x18\Charlie",
	"\x1E\Rekt'Son",

	//// -- The Supreme Kings
	// == TERRAN ==
	"\x15\Ackmed",
	"\x06\The Don",
	"\x1B\Mathias Tolrim",

	// == PROTOSS ==
	"\x1C\Tetragram",
	"\x08\Amon",
	"\x1C\The Great Devourer",

	// == ZERG ==
	"\x15\Zerghere",
	"\x16\Connor",
	"\x1E\Kagg",

	// == RANDOM ==
	"\x06\God's Hand",
	"\x06\God's Hand",
	"\x06\God's Hand",
	"\x06\God's Hand",
	"\x06\God's Hand",
	"\x06\God's Hand",
};
C_ASSERT(ARRAY_SIZE(fraud_names) == Fraud_Count);

const char* fraud_alliance_names[] = {
	"New Ruska",
	"Nah, Fuck That!",
	"East Aero Progamers",
	"StarEdit Crusaders",
	"ColoredVaguery",
	"Of the Utalian Peninsula...",
	"Chinese Translators",
	"The Wireframers",
	"The 3%",
	"The Vile Voidspeakers",
	"Gatesoft Microbills",
	"The Shitfling Sinners",
	"Fraudulent Task Force",
	"Ego Death",
};
C_ASSERT(ARRAY_SIZE(fraud_alliances) == ARRAY_SIZE(fraud_alliances));

const char* fraud_class_names[] = {
	"None",
	"\x10\Fraud",
	"\x11\Hero",
	"\x0F\Super",
	"\x06\God",
};
C_ASSERT(ARRAY_SIZE(fraud_class_names) == FraudClass_Count);

struct FraudInfo {
	u32 personality;
	u32 build_type;
	u32 build_order;
	u32 color;

	u8 deaths[UnitId::Count];
};

#define FraudSpawnCap 100
#define FraudUnitCap 20
#define FraudSourceCap 10
#define FraudRallyCap 100
struct FraudSpawnSource {
	Point16 pos;
	u32 rally_count;
	Point16 rallies[FraudRallyCap];
};

struct FraudSpawnLocation {
	Point16 pos;
	u32 source_count;
	FraudSpawnSource sources[FraudSourceCap];
};

struct FraudSpawnSequence {
	u32 fraud_class;
	u32 fraud;
	u32 race;
	u32 unit_timer;
	u32 safety_timer;
	u32 source_index;
	u32 rally_index;
	u32 unit_index;
	Unit* temp_townhall;
	FraudSpawnLocation* location;
};

struct FraudSpawnUnit {
	u32 count;
	u32 unit_id;
};

struct ThresholdState {
	FraudInfo frauds[Fraud_Count][RaceId::Count];
	u32 used_count;
	u32 used_type_count[FraudClass_Count];
	s32 hero_spawn_counter;
	s32 super_spawn_counter;
	bool used[Fraud_Count];
	u32 timer;

	FraudSpawnSequence sequence[PlayerLimit];
	u32 spawn_count;
	FraudSpawnLocation spawns[FraudSpawnCap];
	u32 unit_count[Fraud_Count][RaceId::Count];
	FraudSpawnUnit units[Fraud_Count][RaceId::Count][FraudUnitCap];
};

ThresholdConfig threshold_config;
static ThresholdState threshold;

static FraudSpawnLocation* get_random_spawn() {
	u32 count = threshold.spawn_count;
	if (!count) {
		return 0;
	}

	u32 offset = scbw::rand_between(0, count - 1);
	u32 best_cost = UINT_MAX;
	u32 best_location = offset;
	for (u32 i = 0; i < count; ++i) {
		u32 location = (i + offset) % count;

		u32 skip = false;
		for (u32 j = 0; j < PlayerLimit; ++j) {
			if (!threshold_config.fraud_enabled[j]) {
				continue;
			}
			if (threshold.sequence[j].location == &threshold.spawns[location]) {
				skip = true;
				break;
			}
		}
		if (skip) {
			continue;
		}

		u32 cost = 0;
		EntryArray unit_array = {};
		unit_search_radial(&unit_array, threshold.spawns[location].pos, 12 * 32);
		if (unit_array.count) {
			cost += 1;
		}
		unit_search_filter(&unit_array, 0, FilterBuilding);
		if (unit_array.count) {
			cost += 1;
		}

		if (cost < best_cost) {
			best_cost = cost;
			best_location = location;
		}
	}
	return &threshold.spawns[best_location];
}

static FraudSpawnLocation* get_closest_spawn(Point16 pos) {
	FraudSpawnLocation* closest_loc = 0;
	f32 lowest_distance = 999999999;

	for (u32 i = 0; i < threshold.spawn_count; ++i) {
		FraudSpawnLocation* loc = &threshold.spawns[i];
		f32 distance = scbw::get_distance_fast(loc->pos, pos);
		if (distance < lowest_distance) {
			closest_loc = loc;
			lowest_distance = distance;
		}
	}

	return closest_loc;
}

static FraudSpawnSource* get_closest_source(FraudSpawnLocation* location, Point16 pos) {
	FraudSpawnSource* closest_loc = 0;
	f32 lowest_distance = 999999999;

	for (u32 i = 0; i < location->source_count; ++i) {
		FraudSpawnSource* loc = &location->sources[i];
		f32 distance = scbw::get_distance_fast(loc->pos, pos);
		if (distance < lowest_distance) {
			closest_loc = loc;
			lowest_distance = distance;
		}
	}

	return closest_loc;
}

static void init_threshold_spawns() {
	Unit* next = 0;
	for (Unit* unit = first_player_unit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::TerranCommandCenter) {
			FraudSpawnLocation spawn = {};
			spawn.pos = unit->position;
			if (threshold.spawn_count < FraudSpawnCap) {
				threshold.spawns[threshold.spawn_count++] = spawn;
			} else {
				scbw::printf("Spawn cap reached (" STR(FraudSpawnCap) ")");
			}

			unit->user_action_flags |= 4; // silent remove
			unit->remove();
		}
	}
	for (Unit* unit = first_player_unit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::ProtossPylon) {
			FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
			if (spawn) {
				if (spawn->source_count < FraudSourceCap) {
					spawn->sources[spawn->source_count++] = {unit->position};
				} else {
					scbw::printf("Source cap per spawn reached (" STR(FraudSourceCap) ")");
				}
			}

			unit->user_action_flags |= 4; // silent remove
			unit->remove();
		}
	}
	for (Unit* unit = first_player_unit->unit[11]; unit; unit = next) {
		next = unit->player_link.next;
		if (unit->id == UnitId::TerranMaverick) {
			FraudSpawnLocation* spawn = get_closest_spawn(unit->position);
			if (spawn) {
				FraudSpawnSource* source = get_closest_source(spawn, unit->position);
				if (source) {
					if (source->rally_count < FraudRallyCap) {
						source->rallies[source->rally_count++] = unit->position;
					} else {
						scbw::printf("Rally cap per source reached (" STR(FraudRallyCap) ")");
					}
				}
			}

			unit->user_action_flags |= 4; // silent remove
			unit->remove();
		}
	}
}

static void add_fraud_unit(u32 fraud, u32 race, FraudSpawnUnit unit, u32 min_index = 0) {
	for (u32 i = min_index; i < threshold.unit_count[fraud][race]; ++i) {
		if (threshold.units[fraud][race][i].unit_id == unit.unit_id) {
			threshold.units[fraud][race][i].count += unit.count;
			return;
		}
	}
	if (threshold.unit_count[fraud][race] < FraudUnitCap) {
		threshold.units[fraud][race][threshold.unit_count[fraud][race]++] = unit;
	} else {
		scbw::printf("Unit entry cap per fraud/race reached (" STR(FraudUnitCap) ")");
	}
}

static bool fraud_share_alliance(u32 a, u32 b) {
	u32* alliance = 0;
	for (u32 i = 0; i < ARRAY_SIZE(fraud_alliances); ++i) {
		for (u32 j = 0; j < AlliancePlayerCap; ++j) {
			if (fraud_alliances[i][j] == a) {
				alliance = fraud_alliances[i];
				break;
			}
		}
	}

	if (alliance) {
		for (u32 i = 0; i < AlliancePlayerCap; ++i) {
			if (alliance[i] == b) {
				return true;
			}
		}
	}
	return false;
}

static void set_ally(u32 a, u32 b, bool ally) {
	player_alliance[a].flags[b] = ally;
	player_alliance[b].flags[a] = ally;
	if (ally) {
		player_vision->flags[a] |= 1 << b;
		player_vision->flags[b] |= 1 << a;
	} else {
		player_vision->flags[a] &= ~(1 << b);
		player_vision->flags[b] &= ~(1 << a);
	}
}

void messagef(char* format, ...) {
	static char buffer[1024];

	va_list va;
	va_start(va, format);
	vsprintf_s(buffer, sizeof(buffer), format, va);
	va_end(va);
	
	helper::display_text_message(buffer, 2, GetTickCount() + helper::get_text_display_time(buffer), 1);
}

u32 get_next_fraud(u32 player, u32 fraud_class) {
	if (!threshold.sequence[player].location) {
		u32 first = 0;
		u32 last  = 0;
		if (fraud_class == FraudClass_Fraud) {
			first = Fraud_First;
			last  = Fraud_Last;
		} else if (fraud_class == FraudClass_Hero) {
			first = FraudHero_First;
			last  = FraudHero_Last;
		} else if (fraud_class == FraudClass_Super) {
			first = FraudSuper_First;
			last  = FraudSuper_Last;
		} else if (fraud_class == FraudClass_God) {
			first = FraudGod_First;
			last  = FraudGod_Last;
		} else {
			return Fraud_None;
		}

		u32 count = 0;
		for (u32 i = first; i <= last; ++i) {
			if (!threshold.used[i]) {
				++count;
			}
		}
		if (count) {
			u32 fraud = first;
			u32 index = scbw::rand_between(1, count);
			while (index) {
				if (!threshold.used[fraud]) {
					if (!--index) {
						return fraud;
					}
				}
				if (++fraud > last) { // This should never happen if the count is correct, but I will leave it as a fail safe -Veeq7
					fraud = first;
				}
			}
		}
	}
	return Fraud_None;
}

u32 get_fraud_class(u32 fraud) {
	if (fraud >= Fraud_First      && fraud <= Fraud_Last)      return FraudClass_Fraud;
	if (fraud >= FraudHero_First  && fraud <= FraudHero_Last)  return FraudClass_Hero;
	if (fraud >= FraudSuper_First && fraud <= FraudSuper_Last) return FraudClass_Super;
	if (fraud >= FraudGod_First   && fraud <= FraudGod_Last)   return FraudClass_God;
	return FraudClass_None;
}

u32 get_fraud_race(u32 fraud) {
	if (fraud >= Fraud_TerranFirst && fraud <= Fraud_TerranLast
		|| fraud >= FraudHero_TerranFirst && fraud <= FraudHero_TerranLast
		|| fraud >= FraudSuper_TerranFirst && fraud <= FraudSuper_TerranLast) {
		return RaceId::Terran;
	}
	if (fraud >= Fraud_ProtossFirst && fraud <= Fraud_ProtossLast
		|| fraud >= FraudHero_ProtossFirst && fraud <= FraudHero_ProtossLast
		|| fraud >= FraudSuper_ProtossFirst && fraud <= FraudSuper_ProtossLast) {
		return RaceId::Protoss;
	}
	if (fraud >= Fraud_ZergFirst && fraud <= Fraud_ZergLast
		|| fraud >= FraudHero_ZergFirst && fraud <= FraudHero_ZergLast
		|| fraud >= FraudSuper_ZergFirst && fraud <= FraudSuper_ZergLast) {
		return RaceId::Zerg;
	}
	return scbw::rand_between(0, 2);
}

bool place_fraud(u32 player, u32 fraud_class, bool spawn_townhall) {
	if (fraud_class == FraudClass_None) {
		return false;
	}
	if (threshold.sequence[player].location) { // Do not spawn frauds if they are already on the map
		return false;
	}
	u32 next_fraud = get_next_fraud(player, fraud_class);
	if (next_fraud) {
		FraudSpawnLocation* spawn_location = get_random_spawn();
		if (spawn_location) {
			threshold.used_count += fraud_class;
			threshold.used[next_fraud] = true;
			threshold.timer = threshold_config.timer;
			if (fraud_class == FraudClass_God) {
				threshold.timer = threshold_config.god_timer;
			}

			messagef("Spawn %s\x02 (Class: %s\x01)!", fraud_names[next_fraud], fraud_class_names[fraud_class]);

			++threshold.used_type_count[fraud_class];
			if (fraud_class == FraudClass_Hero) {
				threshold.hero_spawn_counter = 0;
			} else {
				++threshold.hero_spawn_counter;
			}
			if (fraud_class == FraudClass_Super) {
				threshold.super_spawn_counter = -2;
			} else {
				++threshold.super_spawn_counter;
			}

			u32 race = get_fraud_race(next_fraud);
			g_player_table[player].race = race;

			FraudSpawnSequence* sequence = &threshold.sequence[player];
			sequence->fraud_class = fraud_class;
			sequence->fraud = next_fraud;
			sequence->race = race;
			sequence->unit_timer = 0;
			sequence->safety_timer = 30 * 24;
			sequence->source_index = 0;
			sequence->rally_index = 0;
			sequence->unit_index = 0;
			sequence->location = spawn_location;

			for (u32 i = 0; i < UnitId::Count; ++i) {
				if (i == UnitId::Spell_ScannerSweep)   continue;
				if (i == UnitId::Special_RightPitDoor) continue;
				if (i == UnitId::Spell_DarkSwarm)      continue;
				g_unit_deaths_table[i].player[player] = threshold.frauds[next_fraud][race].deaths[i];
			}

			g_unit_deaths_table[UnitId::Spell_DisruptionWeb].player[player] = threshold.frauds[next_fraud][race].personality;
			g_unit_deaths_table[UnitId::ProtossPlasmaShell].player[player] = threshold.frauds[next_fraud][race].build_type;
			g_unit_deaths_table[UnitId::Spell_DarkSwarm].player[player] = threshold.frauds[next_fraud][race].build_order;
			set_player_color(player, threshold.frauds[next_fraud][race].color);
			set_player_name(player, (char*)fraud_names[next_fraud]);
			set_player_force_name(player, (char*)fraud_class_names[fraud_class]);
			for (u32 i = 0; i < ARRAY_SIZE(fraud_alliances); ++i) {
				bool exit = false;
				for (u32 j = 0; j < AlliancePlayerCap; ++j) {
					if (fraud_alliances[i][j] == next_fraud) {
						set_player_force_name(player, (char*)fraud_alliance_names[i]);
						exit = true;
						break;
					}
				}
				if (exit) {
					break;
				}
			}

			// Todo: reset kills using aise
			s32 fraud_count = threshold.used_count - threshold_config.initial_count; // Don't count initial spawns towards the multiplier
			if (fraud_count < 0) {
				fraud_count = 0;
			}

			f32 multiplier = 0.5 + fraud_count * 0.2;
			for (u32 i = 0; i < threshold.unit_count[next_fraud][race]; ++i) {
				threshold.units[next_fraud][race][i].count *= multiplier;
				if (units_dat::base_property[threshold.units[next_fraud][race][i].unit_id] & UnitProperty::Worker) {
					if (threshold.units[next_fraud][race][i].count > 40) {
						threshold.units[next_fraud][race][i].count = 40;
					}
				}
			}
			multiplier += 0.5 * (fraud_class - 1);
			resources->cumulative_minerals[player] += 1200 * multiplier;
			resources->cumulative_gas[player] += 300 * multiplier;
			resources->minerals[player] = 1200 * multiplier;
			resources->gas[player] = 300 * multiplier;

			g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player] = 1; // Disable AI upgrade overwrite
			u32 resource_upgrade = fraud_class;
			scbw::set_upgrade_level(player, UpgradeId::MiningEfficiency, resource_upgrade);

			g_start_positions[player] = spawn_location->pos;
			
			// Mark current AI scripts to die
			for (AiScript* ai_script = *g_first_active_aiscript; ai_script; ai_script = ai_script->link.next) {
				if (ai_script->player == player) {
					ai_script->flags |= 0x4;
				}
			}
			// Mark current AI towns to die
			AiTown* ai_town_prev = 0;
			for (AiTown* ai_town = g_ai_active_town_array[player].first; ai_town; ai_town = ai_town_prev) {
				ai_town_prev = ai_town->prev;
				helper::delete_ai_town_if_needed(ai_town);
			}

			if (spawn_townhall) {
				Point16 pos = threshold.sequence[player].location->pos;
				Unit* augur = scbw::create_unit_in_game(UnitId::ProtossAugur, player, pos.x, pos.y);
				if (augur) {
					augur->user_action_flags |= 4;
					augur_sacrifice(augur, 10 * 32);
				}

				u32 townhall = UnitId::None;
				switch (race) {
					case RaceId::Terran:	townhall = UnitId::TerranCommandCenter; break;
					case RaceId::Protoss:	townhall = UnitId::ProtossNexus;		break;
					case RaceId::Zerg:		townhall = UnitId::ZergHachirosk;		break;
				}
				if (townhall != UnitId::None) {
					sequence->temp_townhall = helper::create_unit_at_pos(townhall, player, pos.x, pos.y);
				}
			}

			set_ally(player, 0, false);
			for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
				if (player == other_player) {
					continue;
				}
				if (threshold_config.fraud_enabled[other_player] && 
					fraud_share_alliance(threshold.sequence[player].fraud, threshold.sequence[other_player].fraud) ||
					(threshold.sequence[player].fraud_class == threshold.sequence[other_player].fraud_class &&
						(threshold.sequence[player].fraud_class == FraudClass_Hero || threshold.sequence[player].fraud_class == FraudClass_God))) {
					set_ally(player, other_player, true);

					messagef("%s\x01 allies to %s\x01!", fraud_names[threshold.sequence[player].fraud], fraud_names[threshold.sequence[other_player].fraud]);
				} else if (threshold_config.witness_enabled[other_player]) {
					set_ally(player, other_player, true);
				} else {
					set_ally(player, other_player, false);
				}
			}

			scbw::minimap_ping(spawn_location->pos.x, spawn_location->pos.y, 0, *g_local_nation_id);
			
			for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
				if (!threshold_config.fraud_enabled[other_player] || threshold_config.witness_enabled[other_player]) {
					player_vision->flags[player] |= 1 << other_player;
				}
			}

			scbw::run_default_ai(player, spawn_location->pos);

			return true;
		}
	}
	return false;
}

void init_threshold_units();

void init_scenario() {
	memset(&threshold_config, 0, sizeof(threshold_config));
	memset(&threshold, 0, sizeof(threshold));

	// Defaults
	if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch) ||
		is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence1) ||
		is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence2)) {
		threshold_config.enabled = true;
		for (u32 player = 0; player < PlayerLimit; ++player) {
			threshold_config.fraud_allowed[player] = true;
		}
	}
	if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch) ||
		is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence1)) {
		threshold_config.witness_enabled[7] = true;
	}
	if (is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence2)) {
		threshold_config.timer = 1.5 * 60 * 24;
		threshold_config.god_timer = 1 * 60 * 24;
		threshold_config.initial_count = 4;
		threshold_config.min_frauds = 4;
	} else {
		threshold_config.timer = 3 * 60 * 24;
		threshold_config.god_timer = 1 * 60 * 24;
		threshold_config.timer = 4 * 60 * 24;
		threshold_config.initial_count = 2;
		threshold_config.min_frauds = 2;
	}
}

void end_threshold() {
	memset(&threshold_config, 0, sizeof(threshold_config));
	memset(&threshold, 0, sizeof(threshold));
}

void tick_threshold() {
	if (!threshold_config.enabled) {
		return;
	}
	if (!threshold_config.init) {
		threshold_config.init = true;
		init_threshold_spawns();
		init_threshold_units();

		threshold.hero_spawn_counter = -1 - threshold_config.initial_count;
		threshold.super_spawn_counter = -4 - threshold_config.initial_count;
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold_config.fraud_enabled[player]) {
				continue;
			}
			if (threshold_config.initial_count) {
				if (place_fraud(player, FraudClass_Fraud, true)) {
					threshold_config.initial_count -= 1;
				}
			}
		}
	}

	// Update Config
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (threshold_config.fraud_allowed[player] && g_player_table[player].type == PlayerType::Computer) {
			threshold_config.fraud_enabled[player] = true;
		} else {
			threshold_config.fraud_enabled[player] = false;
		}
	}
	if (threshold_config.total_frauds > Fraud_Count - FraudGod_Count - 1) {
		threshold_config.total_frauds = Fraud_Count - FraudGod_Count - 1;
	}
	const u32 god_spawn_breakpoint = threshold_config.total_frauds;

	static char buffer[64];
	if (threshold.used_count < god_spawn_breakpoint) {
		sprintf_s(buffer, sizeof(buffer), "Frauds remaining: %u", god_spawn_breakpoint - threshold.used_count);
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;

		format_time(buffer, sizeof(buffer), "Next Fraud: ", threshold.timer / 24);
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;
	} else if (threshold.used_type_count[FraudClass_God]) {
		sprintf_s(buffer, sizeof(buffer), "Hands remaining: %u", 6 - threshold.used_type_count[FraudClass_God]);
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;

		if (threshold.used_type_count[FraudClass_God] < 6) {
			format_time(buffer, sizeof(buffer), "Next Hand: ", threshold.timer / 24);
			graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		}
		ui_text_pos.y += ui_text_height;
	} else {
		sprintf_s(buffer, sizeof(buffer), "Frauds remaining: 0");
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;
	}

	sprintf_s(buffer, sizeof(buffer), "Active Frauds:");
	graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
	ui_text_pos.y += ui_text_height;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold_config.fraud_enabled[player]) {
			continue;
		}
		FraudSpawnSequence* sequence = &threshold.sequence[player];
		if (sequence->fraud) {
			sprintf_s(buffer, sizeof(buffer), "%s\x02 [%s\x02]", fraud_names[sequence->fraud], fraud_class_names[sequence->fraud_class]);
			graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
			ui_text_pos.y += ui_text_height;
		}
	}

	u32 alive_count = 0;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold_config.fraud_enabled[player]) {
			continue;
		}
		if (threshold.sequence[player].location || g_all_factory_counts->unit[player] > 0) {
			++alive_count;
		}
	}

	if (!alive_count && threshold.used_type_count[FraudClass_God] == 6) {
		g_unit_deaths_table[UnitId::Special_PitDoor].player[0] = 69;
	}

	// Fraud *Boom*
	for (u32 player = 0; player < PlayerLimit; ++player) {
		FraudSpawnSequence* sequence = &threshold.sequence[player];
		if (sequence->safety_timer) {
			--sequence->safety_timer;
		}
		if (g_all_factory_counts->unit[player] == 0 && sequence->fraud && !sequence->location && !sequence->safety_timer) {
			if (sequence->fraud_class == FraudClass_God) {
				if (first_player_unit->unit[player]) {
					sequence->fraud = Fraud_None;
					sequence->fraud_class = FraudClass_None;
				}
			} else {
				if (first_player_unit->unit[player]) {
					messagef("%s\x01 (Class: %s\x01) died!", fraud_names[sequence->fraud], fraud_class_names[threshold.sequence[player].fraud_class]);
					sequence->safety_timer = 24;
				}
				Unit* next = 0;
				for (Unit* unit = first_player_unit->unit[player]; unit; unit = next) {
					next = unit->player_link.next;
					unit->remove();
				}
				sequence->fraud = Fraud_None;
				sequence->fraud_class = FraudClass_None;
			}
		}
	}

	// Fraud Spawning
	if (alive_count >= threshold_config.min_frauds && threshold.timer) {
		--threshold.timer;
	} else {
		threshold.timer = 0;
	}
	if (!threshold.timer) {
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold_config.fraud_enabled[player]) {
				continue;
			}
			FraudSpawnSequence* sequence = &threshold.sequence[player];
			if (g_all_factory_counts->unit[player] == 0 && !sequence->safety_timer && *g_unit_count < 1600) {
				u32 fraud_class = FraudClass_None;
				if (threshold.used_count < god_spawn_breakpoint) {
					fraud_class = FraudClass_Fraud;
					if (get_next_fraud(player, FraudClass_Hero) && (s32)scbw::rand_between(0, 99) < threshold.hero_spawn_counter * 33) {
						fraud_class = FraudClass_Hero;
					} else if (get_next_fraud(player, FraudClass_Super) && (s32)scbw::rand_between(0, 99) < threshold.super_spawn_counter * 20) {
						fraud_class = FraudClass_Super;
					}
				} else if (threshold.used_type_count[FraudClass_God]) {
					fraud_class = FraudClass_God;
				}

				if (place_fraud(player, fraud_class, true)) {
					++alive_count;
					break;
				}
			}
		}
	}

	u32 non_god_alive_count = 0;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold_config.fraud_enabled[player]) {
			continue;
		}
		if (threshold.sequence[player].location || g_all_factory_counts->unit[player] > 0) {
			if (threshold.sequence[player].fraud_class != FraudClass_God) {
				++non_god_alive_count;
			}
		}
	}

	if (!non_god_alive_count && threshold.used_count >= god_spawn_breakpoint && threshold.used_type_count[FraudClass_God] < 3) {
		if (threshold.used_type_count[FraudClass_God] < 2) {
			threshold.timer = 0;
		}
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold_config.fraud_enabled[player]) {
				continue;
			}
			if (place_fraud(player, FraudClass_God, true)) {
				break;
			}
		}
	}

	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold_config.fraud_enabled[player]) {
			continue;
		}
		FraudSpawnSequence* sequence = &threshold.sequence[player];
		if (sequence->temp_townhall && *g_elapsed_time_frames > 1) {
			Unit* unit = sequence->temp_townhall;
			helper::set_building_tile_flag(unit, unit->get_x(), unit->get_y());
			sequence->temp_townhall = 0;
		}

		// Spawn Units
		if (sequence->location) {
			if (sequence->unit_timer) {
				--sequence->unit_timer;
			}
			if (!sequence->unit_timer) {
				if (sequence->unit_index < threshold.unit_count[sequence->fraud][sequence->race] && sequence->location->source_count && *g_unit_count < 1650) {
					FraudSpawnSource* source = &sequence->location->sources[sequence->source_index % sequence->location->source_count];
					u32 unit_id = threshold.units[sequence->fraud][sequence->race][sequence->unit_index].unit_id;
					if (source->rally_count) {
						Point16 source_pos = source->pos;
						Point16 rally = source->rallies[sequence->rally_index % source->rally_count];

						Unit* augur = scbw::create_unit_in_game(UnitId::ProtossAugur, player, source_pos.x, source_pos.y);
						if (augur) {
							augur->user_action_flags |= 4;
							augur_sacrifice(augur, 10 * 32);
						}
						Unit* unit = scbw::create_unit_in_game(unit_id, player, source_pos.x, source_pos.y);
						if (unit) {
							if (!(units_dat::base_property[unit_id] & UnitProperty::Worker)) {
								unit->order_to(OrderId::AttackMove, rally.x, rally.y);
							}

							sequence->unit_timer = 24 * 0.3;
							if (threshold.units[sequence->fraud][sequence->race][sequence->unit_index].count) {
								--threshold.units[sequence->fraud][sequence->race][sequence->unit_index].count;
							}
							if (!threshold.units[sequence->fraud][sequence->race][sequence->unit_index].count) {
								++sequence->unit_index;
							}
						}
					}
					++sequence->source_index;
					if (!(units_dat::base_property[unit_id] & UnitProperty::Worker)) {
						if (sequence->source_index % sequence->location->source_count == 0) {
							++sequence->rally_index;
						}
					}
				} else {
					for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
						if (!threshold_config.fraud_enabled[other_player] && !threshold_config.witness_enabled[other_player]) {
							player_vision->flags[player] &= ~(1 << other_player);
						}
					}
					sequence->location = 0;
				}
			}
		}
	}
}

void init_threshold_units() {
	for (u32 i = 0; i < Fraud_Count; ++i) {
		switch (get_fraud_class(i)) {
			case FraudClass_Fraud:
			{
				add_fraud_unit(i, RaceId::Terran,  {12, UnitId::TerranMason});
				add_fraud_unit(i, RaceId::Terran,  {1, UnitId::TerranAncile});

				add_fraud_unit(i, RaceId::Protoss, {12, UnitId::ProtossScribe});
				add_fraud_unit(i, RaceId::Protoss, {1, UnitId::ProtossWitness});

				add_fraud_unit(i, RaceId::Zerg,    {12, UnitId::ZergDroleth});
				add_fraud_unit(i, RaceId::Zerg,    {1, UnitId::ZergOvileth});
			} break;
			case FraudClass_Hero:
			{
				add_fraud_unit(i, RaceId::Terran,  {10, UnitId::TerranMason});
				add_fraud_unit(i, RaceId::Terran,  {2, UnitId::TerranTalos});
				add_fraud_unit(i, RaceId::Terran,  {2, UnitId::TerranAncile});

				add_fraud_unit(i, RaceId::Protoss, {10, UnitId::ProtossScribe});
				add_fraud_unit(i, RaceId::Protoss, {2, UnitId::ProtossArtisan});
				add_fraud_unit(i, RaceId::Protoss, {2, UnitId::ProtossWitness});

				add_fraud_unit(i, RaceId::Zerg,    {10, UnitId::ZergDroleth});
				add_fraud_unit(i, RaceId::Zerg,    {2, UnitId::ZergGosvileth});
				add_fraud_unit(i, RaceId::Zerg,    {1, UnitId::ZergIroleth});
			} break;
			case FraudClass_Super:
			{
				add_fraud_unit(i, RaceId::Terran,  {8, UnitId::TerranMason});
				add_fraud_unit(i, RaceId::Terran,  {4, UnitId::TerranTalos});
				add_fraud_unit(i, RaceId::Terran,  {1, UnitId::TerranSeraph});

				add_fraud_unit(i, RaceId::Protoss, {8, UnitId::ProtossScribe});
				add_fraud_unit(i, RaceId::Protoss, {4, UnitId::ProtossArtisan});
				add_fraud_unit(i, RaceId::Protoss, {3, UnitId::ProtossWitness});

				add_fraud_unit(i, RaceId::Zerg,    {8, UnitId::ZergDroleth});
				add_fraud_unit(i, RaceId::Zerg,    {4, UnitId::ZergGosvileth});
				add_fraud_unit(i, RaceId::Zerg,    {1, UnitId::ZergAlaszileth});
			} break;
			case FraudClass_God:
			{
				add_fraud_unit(i, RaceId::Terran,  {6, UnitId::TerranMason});
				add_fraud_unit(i, RaceId::Terran,  {6, UnitId::TerranTalos});
				add_fraud_unit(i, RaceId::Terran,  {2, UnitId::TerranSeraph});

				add_fraud_unit(i, RaceId::Protoss, {6, UnitId::ProtossScribe});
				add_fraud_unit(i, RaceId::Protoss, {6, UnitId::ProtossArtisan});
				add_fraud_unit(i, RaceId::Protoss, {4, UnitId::ProtossWitness});

				add_fraud_unit(i, RaceId::Zerg,    {6, UnitId::ZergDroleth});
				add_fraud_unit(i, RaceId::Zerg,    {6, UnitId::ZergGosvileth});
				add_fraud_unit(i, RaceId::Zerg,    {2, UnitId::ZergAlaszileth});
			} break;
		}
	}

	//// Frauds

	// Alexei Ukov
	threshold.frauds[Fraud_AlexeiUkov][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VividRed};
	add_fraud_unit(Fraud_AlexeiUkov, RaceId::Terran, {5, UnitId::TerranMaverick});
	add_fraud_unit(Fraud_AlexeiUkov, RaceId::Terran, {2, UnitId::TerranGorgon});
	add_fraud_unit(Fraud_AlexeiUkov, RaceId::Terran, {1, UnitId::TerranWyvern});

	// Ullen "Bank on Me" Bigmunney
	threshold.frauds[Fraud_UllenBankOnMeBigmunney][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_None, AiBuildOrder_TerranArmor_IronCrusade, PlayerColor::VineGreen};
	add_fraud_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {4, UnitId::TerranMason});
	add_fraud_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {2, UnitId::TerranGoliath});
	add_fraud_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {2, UnitId::TerranPhalanxTankMode});

	// knightoftherealm
	threshold.frauds[Fraud_knightoftherealm][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Aero};
	add_fraud_unit(Fraud_knightoftherealm, RaceId::Terran, {4, UnitId::TerranMaverick});
	add_fraud_unit(Fraud_knightoftherealm, RaceId::Terran, {4, UnitId::TerranCyprian});
	add_fraud_unit(Fraud_knightoftherealm, RaceId::Terran, {1, UnitId::TerranShaman});

	// KKKhris
	threshold.frauds[Fraud_KKKhris][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brown};
	add_fraud_unit(Fraud_KKKhris, RaceId::Terran, {4, UnitId::TerranMaverick});
	add_fraud_unit(Fraud_KKKhris, RaceId::Terran, {2, UnitId::TerranPhalanxTankMode});
	add_fraud_unit(Fraud_KKKhris, RaceId::Terran, {2, UnitId::TerranWraith});

	// Dynamo
	threshold.frauds[Fraud_Dynamo][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_3RaxMarineAndCleric, PlayerColor::VividOrange};
	add_fraud_unit(Fraud_Dynamo, RaceId::Terran, {4, UnitId::TerranMadcap});
	add_fraud_unit(Fraud_Dynamo, RaceId::Terran, {1, UnitId::TerranAutocrat});

	// TodsDadPat
	threshold.frauds[Fraud_TodsDadPat][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Platinum};
	add_fraud_unit(Fraud_TodsDadPat, RaceId::Terran, {2, UnitId::TerranPaladin});

	// Piglet
	threshold.frauds[Fraud_Piglet][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cordovan};
	add_fraud_unit(Fraud_Piglet, RaceId::Terran, {2, UnitId::TerranBlackjack});
	add_fraud_unit(Fraud_Piglet, RaceId::Terran, {2, UnitId::TerranCohort});
	add_fraud_unit(Fraud_Piglet, RaceId::Terran, {2, UnitId::TerranSiren});

	// DarkenedFantasies
	threshold.frauds[Fraud_DarkenedFantasies][RaceId::Terran] = { AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSpringGreen};
	add_fraud_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranVulture});
	add_fraud_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranCyclops});
	add_fraud_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranGoliath});

	// Customer Supporter
	threshold.frauds[Fraud_CustomerSupporter][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Turquoise};
	add_fraud_unit(Fraud_CustomerSupporter, RaceId::Terran, {2, UnitId::TerranGorgon});
	add_fraud_unit(Fraud_CustomerSupporter, RaceId::Terran, {1, UnitId::TerranSalamander});

	// Customer Supporter
	threshold.frauds[Fraud_Desler][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Mint};
	add_fraud_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranEidolon});
	add_fraud_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranGoliath});
	add_fraud_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranBlackjack});

	// Bitcoin Boris
	threshold.frauds[Fraud_BitcoinBoris][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::CoyoteBrown};
	add_fraud_unit(Fraud_BitcoinBoris, RaceId::Protoss, {4, UnitId::ProtossBarghest});
	add_fraud_unit(Fraud_BitcoinBoris, RaceId::Protoss, {1, UnitId::ProtossCharlatan});
	add_fraud_unit(Fraud_BitcoinBoris, RaceId::Protoss, {2, UnitId::TerranStriga});

	// Crypto Carl
	threshold.frauds[Fraud_CryptoCarl][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_None, AiBuildOrder_ProtossSkylord_SolarEscort, PlayerColor::BistreBrown};
	add_fraud_unit(Fraud_CryptoCarl, RaceId::Protoss, {2, UnitId::ProtossPanoptus});
	add_fraud_unit(Fraud_CryptoCarl, RaceId::Protoss, {1, UnitId::ProtossEmpyrean});

	// Veeq7
	threshold.frauds[Fraud_Veeq7][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RoyalBlue};
	add_fraud_unit(Fraud_Veeq7, RaceId::Protoss, {4, UnitId::ProtossSimulacrum});
	add_fraud_unit(Fraud_Veeq7, RaceId::Protoss, {2, UnitId::ProtossDracadin});

	// jun3hong
	threshold.frauds[Fraud_jun3hong][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::PersianOrange};
	add_fraud_unit(Fraud_jun3hong, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	add_fraud_unit(Fraud_jun3hong, RaceId::Protoss, {2, UnitId::ProtossHierophant});
	add_fraud_unit(Fraud_jun3hong, RaceId::Protoss, {1, UnitId::ProtossWitness});

	// Digital Rights Manager
	threshold.frauds[Fraud_DigitalRightsManager][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Glacial};
	add_fraud_unit(Fraud_DigitalRightsManager, RaceId::Protoss, {3, UnitId::ProtossAmaranth});
	add_fraud_unit(Fraud_DigitalRightsManager, RaceId::Protoss, {2, UnitId::ProtossAtreus});

	// Zeratool
	threshold.frauds[Fraud_Zeratool][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TyrianPurple};
	add_fraud_unit(Fraud_Zeratool, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	add_fraud_unit(Fraud_Zeratool, RaceId::Protoss, {2, UnitId::ProtossMindTyrant});

	// F.R.A.U.D. Grandpa of Tony
	threshold.frauds[Fraud_FRAUDGrandpaOfTony][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Peach};
	add_fraud_unit(Fraud_FRAUDGrandpaOfTony, RaceId::Protoss, {4, UnitId::ProtossZealot});
	add_fraud_unit(Fraud_FRAUDGrandpaOfTony, RaceId::Protoss, {2, UnitId::ProtossDracadin});

	// DK
	threshold.frauds[Fraud_DK][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::EggshellBlue};
	add_fraud_unit(Fraud_DK, RaceId::Protoss, {4, UnitId::ProtossAurora});
	add_fraud_unit(Fraud_DK, RaceId::Protoss, {2, UnitId::ProtossPanoptus});

	// iquare
	threshold.frauds[Fraud_iquare][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brass};
	add_fraud_unit(Fraud_iquare, RaceId::Protoss, {2, UnitId::ProtossIdol});
	add_fraud_unit(Fraud_iquare, RaceId::Protoss, {2, UnitId::ProtossAccantor});
	add_fraud_unit(Fraud_iquare, RaceId::Protoss, {1, UnitId::ProtossArchitect});

	// Metathrom
	threshold.frauds[Fraud_Metathrom][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::BdazzledBlue};
	add_fraud_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::ProtossAmaranth});
	add_fraud_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::TerranAspirant});
	add_fraud_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::ProtossCantavis});
	add_fraud_unit(Fraud_Metathrom, RaceId::Protoss, {1, UnitId::ProtossArchon});

	// Cerebrate Stalin
	threshold.frauds[Fraud_CerebrateStalin][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Mango};
	add_fraud_unit(Fraud_CerebrateStalin, RaceId::Zerg, {4, UnitId::TerranCohort});
	add_fraud_unit(Fraud_CerebrateStalin, RaceId::Zerg, {2, UnitId::ZergAxitrilisk});

	// Andy Rose Uh
	threshold.frauds[Fraud_AndyRoseUh][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Orange};
	add_fraud_unit(Fraud_AndyRoseUh, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_AndyRoseUh, RaceId::Zerg, {2, UnitId::ZergMutalisk});

	// Winnie
	threshold.frauds[Fraud_Winnie][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Yellow};
	add_fraud_unit(Fraud_Winnie, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_Winnie, RaceId::Zerg, {4, UnitId::ZergQuazilisk});

	// A
	threshold.frauds[Fraud_A][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::ForestGreen};
	add_fraud_unit(Fraud_A, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_A, RaceId::Zerg, {4, UnitId::TerranCohort});

	// Sellout Sam
	threshold.frauds[Fraud_SelloutSam][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Beaver};
	add_fraud_unit(Fraud_SelloutSam, RaceId::Zerg, {6, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_SelloutSam, RaceId::Zerg, {2, UnitId::ZergVorvrokor});
	add_fraud_unit(Fraud_SelloutSam, RaceId::Zerg, {3, UnitId::ZergLiiralisk});

	// Plagiarist Paul
	threshold.frauds[Fraud_PlagiaristPaul][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::ZinnwalditeBrown};
	add_fraud_unit(Fraud_PlagiaristPaul, RaceId::Zerg, {6, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_PlagiaristPaul, RaceId::Zerg, {2, UnitId::ZergKonvilisk});

	// RimTechCombot
	threshold.frauds[Fraud_RimTechCombot][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::DarkLavender};
	add_fraud_unit(Fraud_RimTechCombot, RaceId::Zerg, {6, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_RimTechCombot, RaceId::Zerg, {3, UnitId::ZergQuazilisk});
	add_fraud_unit(Fraud_RimTechCombot, RaceId::Zerg, {6, UnitId::ZergNathrokor});

	// Africanized Crowkenyan
	threshold.frauds[Fraud_AfricanizedCrowkenyan][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::DarkOlive};
	add_fraud_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {8, UnitId::ZergNathrokor});
	add_fraud_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {2, UnitId::ZergSkithrokor});
	add_fraud_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {2, UnitId::ZergMutalisk});

	// Ultralisks
	threshold.frauds[Fraud_Ultralisks][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Silver};
	add_fraud_unit(Fraud_Ultralisks, RaceId::Zerg, {2, UnitId::ZergLakizilisk});
	add_fraud_unit(Fraud_Ultralisks, RaceId::Zerg, {2, UnitId::ZergUltrokor});

	// N Thrower
	threshold.frauds[Fraud_NThrower][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};
	add_fraud_unit(Fraud_NThrower, RaceId::Zerg, {4, UnitId::TerranCohort});
	add_fraud_unit(Fraud_NThrower, RaceId::Zerg, {2, UnitId::ZergKonvilisk});
	add_fraud_unit(Fraud_NThrower, RaceId::Zerg, {2, UnitId::ZergMatraleth});

	// LightenedRealities
	threshold.frauds[Fraud_LightenedRealities][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Terran, {2, UnitId::TerranHeracles});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Terran, {2, UnitId::TerranCuirass});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Terran, {2, UnitId::TerranSiren});

	threshold.frauds[Fraud_LightenedRealities][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Protoss, {2, UnitId::ProtossZealot});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Protoss, {1, UnitId::ProtossArchitect});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Protoss, {2, UnitId::ProtossGladius});

	threshold.frauds[Fraud_LightenedRealities][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Zerg, {2, UnitId::ZergZoryusthaleth});
	add_fraud_unit(Fraud_LightenedRealities, RaceId::Zerg, {1, UnitId::ZergGeszithalor});

	// AchromaticHalfdreams
	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranApostle});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranOlympian});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {2, UnitId::TerranSiren});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranSalamander});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranSeraph});

	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {1, UnitId::ProtossArchon});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {2, UnitId::ProtossGolem});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {2, UnitId::ProtossExemplar});

	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {2, UnitId::ZergLiiralisk});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {2, UnitId::ZergKalkalisk});
	add_fraud_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {1, UnitId::ZergUltrokor});

	// TonyTron
	threshold.frauds[Fraud_TonyTron][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	add_fraud_unit(Fraud_TonyTron, RaceId::Terran, {2, UnitId::TerranWraith});
	add_fraud_unit(Fraud_TonyTron, RaceId::Terran, {2, UnitId::TerranGorgon});
	add_fraud_unit(Fraud_TonyTron, RaceId::Terran, {1, UnitId::TerranMagnetar});

	threshold.frauds[Fraud_TonyTron][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	add_fraud_unit(Fraud_TonyTron, RaceId::Protoss, {2, UnitId::ProtossPanoptus});
	add_fraud_unit(Fraud_TonyTron, RaceId::Protoss, {2, UnitId::ProtossGladius});
	add_fraud_unit(Fraud_TonyTron, RaceId::Protoss, {1, UnitId::ProtossExemplar});

	threshold.frauds[Fraud_TonyTron][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	add_fraud_unit(Fraud_TonyTron, RaceId::Zerg, {2, UnitId::ZergMutalisk});
	add_fraud_unit(Fraud_TonyTron, RaceId::Zerg, {2, UnitId::ZergKalkalisk});
	add_fraud_unit(Fraud_TonyTron, RaceId::Zerg, {1, UnitId::ZergGeszithalor});

	// Pr0nogo
	threshold.frauds[Fraud_Pr0nogo][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Terran, {2, UnitId::TerranGoliath});
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Terran, {2, UnitId::TerranPhalanxTankMode});

	threshold.frauds[Fraud_Pr0nogo][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Protoss, {2, UnitId::ProtossZealot});
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Protoss, {2, UnitId::ProtossCabalist});

	threshold.frauds[Fraud_Pr0nogo][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_Pr0nogo, RaceId::Zerg, {2, UnitId::ZergBactalisk});

	// Ba3lethal
	threshold.frauds[Fraud_Ba3lethal][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Terran, {4, UnitId::TerranMadcap});
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Terran, {2, UnitId::TerranCleric});
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Terran, {1, UnitId::TerranApostle});

	threshold.frauds[Fraud_Ba3lethal][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Protoss, {2, UnitId::ProtossArchon});

	threshold.frauds[Fraud_Ba3lethal][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_Ba3lethal, RaceId::Zerg, {1, UnitId::ZergKeskathalor});

	// Towering Pigprophet
	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Terran, {4, UnitId::TerranWraith});
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Terran, {1, UnitId::TerranCentaur});

	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Protoss, {4, UnitId::ProtossAurora});
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Protoss, {1, UnitId::ProtossEmpyrean});

	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Zerg, {6, UnitId::ZergNathrokor});
	add_fraud_unit(Fraud_ToweringPigprophet, RaceId::Zerg, {1, UnitId::ZergAlkajelisk});

	// Anal Crowsummoner
	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Terran, {2, UnitId::TerranBlackjack});
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Terran, {6, UnitId::TerranCohort});

	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Protoss, {5, UnitId::ProtossManifold});
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Protoss, {3, UnitId::ProtossIdol});

	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Zerg, {6, UnitId::ZergNathrokor});
	add_fraud_unit(Fraud_AnalCrowsummoner, RaceId::Zerg, {4, UnitId::ZergZethrokor});

	// Pissing Porncultist
	threshold.frauds[Fraud_PissingPorncultist][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Terran, {4, UnitId::TerranMaverick});
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Terran, {4, UnitId::TerranCohort});

	threshold.frauds[Fraud_PissingPorncultist][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Protoss, {2, UnitId::ProtossLegionnaire});
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Protoss, {4, UnitId::TerranAspirant});

	threshold.frauds[Fraud_PissingPorncultist][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_PissingPorncultist, RaceId::Zerg, {4, UnitId::TerranCohort});

	// Bearborne Arkanianpussy
	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Terran, {4, UnitId::TerranBlackjack});
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Terran, {1, UnitId::TerranAspirant});

	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {2, UnitId::ProtossLegionnaire});
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {2, UnitId::ProtossEcclesiast});
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {1, UnitId::ProtossHierophant});
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {1, UnitId::ProtossCharlatan});

	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Zerg] = { AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_BearborneArkanianpussy, RaceId::Zerg, {4, UnitId::TerranCohort});

	// Doubling Wuckdevourer
	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Terran, {4, UnitId::TerranCyclops});
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Terran, {1, UnitId::TerranPhalanxTankMode});

	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {3, UnitId::ProtossZealot});
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {1, UnitId::ProtossDracadin});
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {1, UnitId::ProtossEcclesiast});

	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_DoublingWuckdevourer, RaceId::Zerg, {2, UnitId::ZergLakizilisk});

	// Mongoloid Pointcorruptor
	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {4, UnitId::TerranMaverick});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {2, UnitId::TerranCleric});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {1, UnitId::TerranApostle});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {2, UnitId::TerranCohort});

	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {4, UnitId::ProtossVassal});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {4, UnitId::ProtossSimulacrum});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {1, UnitId::ProtossIdol});

	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Zerg, {6, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_MongoloidPointcorruptor, RaceId::Zerg, {2, UnitId::TerranCohort});

	// Calendared Rageboy
	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	add_fraud_unit(Fraud_CalendaredRageboy, RaceId::Terran, {4, UnitId::TerranHeracles});

	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	add_fraud_unit(Fraud_CalendaredRageboy, RaceId::Protoss, {4, UnitId::ProtossAmaranth});

	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	add_fraud_unit(Fraud_CalendaredRageboy, RaceId::Zerg, {3, UnitId::ZergUltrokor});

	// Worthy Pizzasniffer
	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Terran, {6, UnitId::TerranHarakan});
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Terran, {1, UnitId::TerranApostle});

	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Protoss, {2, UnitId::ProtossAtreus});

	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {4, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {2, UnitId::ZergVorvrokor});
	add_fraud_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {4, UnitId::ZergNathrokor});

	// Skeletal Todzombie
	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Terran, {4, UnitId::TerranPhalanxTankMode});

	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Protoss, {1, UnitId::ProtossPariah});

	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {2, UnitId::ZergBactalisk});
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {4, UnitId::TerranCohort});
	add_fraud_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {1, UnitId::ZergZoryusthaleth});

	// Hoop Thrower
	threshold.frauds[Fraud_HoopThrower][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	add_fraud_unit(Fraud_HoopThrower, RaceId::Terran, {6, UnitId::TerranCyprian});
	add_fraud_unit(Fraud_HoopThrower, RaceId::Terran, {2, UnitId::TerranOlympian});

	threshold.frauds[Fraud_HoopThrower][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	add_fraud_unit(Fraud_HoopThrower, RaceId::Protoss, {3, UnitId::ProtossZealot});
	add_fraud_unit(Fraud_HoopThrower, RaceId::Protoss, {3, UnitId::ProtossDracadin});

	threshold.frauds[Fraud_HoopThrower][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	add_fraud_unit(Fraud_HoopThrower, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	add_fraud_unit(Fraud_HoopThrower, RaceId::Zerg, {2, UnitId::ZergAxitrilisk});

	// Veeq8
	threshold.frauds[Fraud_Veeq8][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	add_fraud_unit(Fraud_Veeq8, RaceId::Terran, {4, UnitId::TerranWraith});
	add_fraud_unit(Fraud_Veeq8, RaceId::Terran, {2, UnitId::TerranSalamander});

	threshold.frauds[Fraud_Veeq8][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	add_fraud_unit(Fraud_Veeq8, RaceId::Protoss, {6, UnitId::ProtossSimulacrum});
	add_fraud_unit(Fraud_Veeq8, RaceId::Protoss, {1, UnitId::ProtossClarion});

	threshold.frauds[Fraud_Veeq8][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	add_fraud_unit(Fraud_Veeq8, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(Fraud_Veeq8, RaceId::Zerg, {4, UnitId::TerranCohort});

	// Bonzibuddy
	threshold.frauds[Fraud_Bonzibuddy][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	add_fraud_unit(Fraud_Bonzibuddy, RaceId::Terran, {8, UnitId::TerranMadcap});

	threshold.frauds[Fraud_Bonzibuddy][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	add_fraud_unit(Fraud_Bonzibuddy, RaceId::Protoss, {2, UnitId::ProtossAmaranth});
	add_fraud_unit(Fraud_Bonzibuddy, RaceId::Protoss, {4, UnitId::ProtossEcclesiast});

	threshold.frauds[Fraud_Bonzibuddy][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	add_fraud_unit(Fraud_Bonzibuddy, RaceId::Zerg, {4, UnitId::TerranCohort});
	add_fraud_unit(Fraud_Bonzibuddy, RaceId::Zerg, {4, UnitId::ZergLiiralisk});

	// IskatuMesk
	threshold.frauds[Fraud_IskatuMesk][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Terran, {4, UnitId::TerranBlackjack});
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Terran, {2, UnitId::TerranCleric});

	threshold.frauds[Fraud_IskatuMesk][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Protoss, {2, UnitId::TerranAspirant});

	threshold.frauds[Fraud_IskatuMesk][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	add_fraud_unit(Fraud_IskatuMesk, RaceId::Zerg, {1, UnitId::ZergKeskathalor});

	//// Heroes

	// Francisco Goska
	threshold.frauds[FraudHero_Francisco_Goska][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_8Rax, PlayerColor::ElectricOrange};
	add_fraud_unit(FraudHero_Francisco_Goska, RaceId::Terran, {4, UnitId::TerranMaverick});
	add_fraud_unit(FraudHero_Francisco_Goska, RaceId::Terran, {2, UnitId::TerranCleric});
	add_fraud_unit(FraudHero_Francisco_Goska, RaceId::Terran, {4, UnitId::TerranMadcap});
	add_fraud_unit(FraudHero_Francisco_Goska, RaceId::Terran, {3, UnitId::TerranApostle});
	add_fraud_unit(FraudHero_Francisco_Goska, RaceId::Terran, {2, UnitId::TerranAutocrat});

	// Ackmed Clone
	threshold.frauds[FraudHero_Ackmed_Clone][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_HolyAckmeds, PlayerColor::DarkBronze};
	add_fraud_unit(FraudHero_Ackmed_Clone, RaceId::Terran, {16, UnitId::TerranHarakan});
	add_fraud_unit(FraudHero_Ackmed_Clone, RaceId::Terran, {4, UnitId::TerranHeracles});

	// Cullen
	threshold.frauds[FraudHero_Cullen][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_None, AiBuildOrder_TerranOrbital_2PortWraith, PlayerColor::MayaBlue};
	add_fraud_unit(FraudHero_Cullen, RaceId::Terran, {8, UnitId::TerranWraith});
	add_fraud_unit(FraudHero_Cullen, RaceId::Terran, {2, UnitId::TerranCentaur});
	add_fraud_unit(FraudHero_Cullen, RaceId::Terran, {1, UnitId::TerranMagnetar});

	// Steve Miller
	threshold.frauds[FraudHero_SteveMiller][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_None, AiBuildOrder_TerranArmor_2FactVult, PlayerColor::White};
	add_fraud_unit(FraudHero_SteveMiller, RaceId::Terran, {6, UnitId::TerranPhalanxTankMode});
	add_fraud_unit(FraudHero_SteveMiller, RaceId::Terran, {2, UnitId::TerranMadrigal});
	add_fraud_unit(FraudHero_SteveMiller, RaceId::Terran, {2, UnitId::TerranPaladin});

	// Torros
	threshold.frauds[FraudHero_Torros][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_None, AiBuildOrder_ProtossTemplar_Initiates, PlayerColor::Saffron};
	add_fraud_unit(FraudHero_Torros, RaceId::Protoss, {6, UnitId::ProtossAmaranth});
	add_fraud_unit(FraudHero_Torros, RaceId::Protoss, {4, UnitId::ProtossAtreus});

	// Drakkar
	threshold.frauds[FraudHero_Drakkar][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_None, AiBuildOrder_ProtossRanger_2GateStrider, PlayerColor::Emerald};
	add_fraud_unit(FraudHero_Drakkar, RaceId::Protoss, {4, UnitId::ProtossHierophant});
	add_fraud_unit(FraudHero_Drakkar, RaceId::Protoss, {4, UnitId::ProtossAccantor});
	add_fraud_unit(FraudHero_Drakkar, RaceId::Protoss, {1, UnitId::ProtossArchitect});

	// Thalazim
	threshold.frauds[FraudHero_Thalazim][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_None, AiBuildOrder_ProtossProfane_CloakAndDagger, PlayerColor::Earthtone};
	add_fraud_unit(FraudHero_Thalazim, RaceId::Protoss, {8, UnitId::ProtossCabalist});
	add_fraud_unit(FraudHero_Thalazim, RaceId::Protoss, {2, UnitId::ProtossPariah});

	// Echo of Mixplay
	threshold.frauds[FraudHero_EchoOfMixplay][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_None, AiBuildOrder_ProtossGeneral_LunaticLegion, PlayerColor::ElectricBlue};
	add_fraud_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {4, UnitId::ProtossDracadin});
	add_fraud_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {2, UnitId::ProtossIdol});
	add_fraud_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {2, UnitId::ProtossGolem});
	add_fraud_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {1, UnitId::ProtossSolarion});

	// Stroluum
	threshold.frauds[FraudHero_Stroluum][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_None, AiBuildOrder_ZergSwarm_InstantQuazrok, PlayerColor::FernGreen};
	add_fraud_unit(FraudHero_Stroluum, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(FraudHero_Stroluum, RaceId::Zerg, {4, UnitId::ZergUltrokor});
	add_fraud_unit(FraudHero_Stroluum, RaceId::Zerg, {1, UnitId::ZergIsthrathaleth});
	add_fraud_unit(FraudHero_Stroluum, RaceId::Zerg, {3, UnitId::ZergLakizilisk});

	// Tuzor
	threshold.frauds[FraudHero_Tuzor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_None, AiBuildOrder_ZergMiscreant_RapidAnnex, PlayerColor::Wine};
	add_fraud_unit(FraudHero_Tuzor, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	add_fraud_unit(FraudHero_Tuzor, RaceId::Zerg, {12, UnitId::TerranCohort});
	add_fraud_unit(FraudHero_Tuzor, RaceId::Zerg, {2, UnitId::ZergGeszithalor});

	// Charlie
	threshold.frauds[FraudHero_Charlie][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_None, AiBuildOrder_ZergAssault_3HatchHydrok, PlayerColor::MidnightGreen};
	add_fraud_unit(FraudHero_Charlie, RaceId::Zerg, {12, UnitId::ZergHydralisk});
	add_fraud_unit(FraudHero_Charlie, RaceId::Zerg, {4, UnitId::ZergBactalisk});
	add_fraud_unit(FraudHero_Charlie, RaceId::Zerg, {2, UnitId::ZergAxitrilisk});

	// Rekt'Son
	threshold.frauds[FraudHero_RektSon][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_None, AiBuildOrder_ZergGeneral_ZorkizFlood, PlayerColor::IridescentViolet};
	add_fraud_unit(FraudHero_RektSon, RaceId::Zerg, {10, UnitId::ZergHydralisk});
	add_fraud_unit(FraudHero_RektSon, RaceId::Zerg, {6, UnitId::ZergLakizilisk});
	add_fraud_unit(FraudHero_RektSon, RaceId::Zerg, {1, UnitId::ZergUltrokor});

	//// Supers

	// Ackmed
	threshold.frauds[FraudSuper_Ackmed][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_HolyAckmeds, PlayerColor::BurntUmber};
	add_fraud_unit(FraudSuper_Ackmed, RaceId::Terran, {24, UnitId::TerranHarakan});
	add_fraud_unit(FraudSuper_Ackmed, RaceId::Terran, {8, UnitId::TerranHeracles});
	add_fraud_unit(FraudSuper_Ackmed, RaceId::Terran, {6, UnitId::TerranSalamander});
	add_fraud_unit(FraudSuper_Ackmed, RaceId::Terran, {2, UnitId::TerranSeraph});

	// The Don
	threshold.frauds[FraudSuper_TheDon][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_None, AiBuildOrder_TerranOrbital_TheDon, PlayerColor::Vermillion};
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranSeraph});
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {9, UnitId::TerranCentaur});
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {6, UnitId::TerranMinotaur});
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranMagnetar});
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {3, UnitId::TerranPhobos});
	add_fraud_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranAzazel});

	// Tolrim
	threshold.frauds[FraudSuper_Tolrim][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Redwood};
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranEidolon});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {2, UnitId::TerranAutocrat});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {6, UnitId::TerranMadrigal});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranPaladin});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {6, UnitId::TerranPazuzu});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranClaymore});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {3, UnitId::TerranPenumbra});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {2, UnitId::TerranAzazel});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {1, UnitId::TerranSeraph});
	add_fraud_unit(FraudSuper_Tolrim, RaceId::Terran, {1, UnitId::TerranMagnetar});

	// Tetragram
	threshold.frauds[FraudSuper_Tetragram][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_None, AiBuildOrder_ProtossRanger_1GateExpo, PlayerColor::YinmnBlue};
	add_fraud_unit(FraudSuper_Tetragram, RaceId::Protoss, {16, UnitId::ProtossSimulacrum});
	add_fraud_unit(FraudSuper_Tetragram, RaceId::Protoss, {2, UnitId::ProtossClarion});
	add_fraud_unit(FraudSuper_Tetragram, RaceId::Protoss, {2, UnitId::ProtossDemiurge});

	// Amon
	threshold.frauds[FraudSuper_Amon][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_ProtossProfane_Starbreach, PlayerColor::Scarlet};
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {8, UnitId::ProtossCabalist});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {20, UnitId::ProtossBarghest});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {6, UnitId::ProtossArchon});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {3, UnitId::ProtossMindTyrant});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {3, UnitId::ProtossPariah});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {6, UnitId::ProtossEmpress});
	add_fraud_unit(FraudSuper_Amon, RaceId::Protoss, {1, UnitId::ProtossAnthelion});

	// The Great Devourer
	threshold.frauds[FraudSuper_TheGreatDevourer][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DelftBlue};
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossIdol});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossGolem});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {8, UnitId::ProtossAtreus});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {6, UnitId::ProtossAccantor});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossArchitect});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {3, UnitId::ProtossClarion});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {3, UnitId::ProtossMagister});
	add_fraud_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {1, UnitId::ProtossDidact});

	// Zerghere
	threshold.frauds[FraudSuper_Zerghere][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_None, AiBuildOrder_ZergSwarm_GatesOfHell, PlayerColor::Auburn};
	add_fraud_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergZethrokor});
	add_fraud_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergQuazilisk});
	add_fraud_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergNathrokor});

	// Connor
	threshold.frauds[FraudSuper_Connor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Beige};
	add_fraud_unit(FraudSuper_Connor, RaceId::Zerg, {20, UnitId::ZergHydralisk});
	add_fraud_unit(FraudSuper_Connor, RaceId::Zerg, {15, UnitId::ZergBactalisk});
	add_fraud_unit(FraudSuper_Connor, RaceId::Zerg, {12, UnitId::ZergAlmaksalisk});
	add_fraud_unit(FraudSuper_Connor, RaceId::Zerg, {10, UnitId::ZergAxitrilisk});

	// Kagg
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::SmokyBlack};
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergVorvrokor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergBactalisk] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergGeszithalor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergUltrokor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergIsthrathaleth] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergAlkajelisk] = 1;
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {20, UnitId::ZergZethrokor});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergVorvrokor});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergHydralisk});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {8, UnitId::ZergBactalisk});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {5, UnitId::ZergZoryusthaleth});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergUltrokor});
	add_fraud_unit(FraudSuper_Kagg, RaceId::Zerg, {5, UnitId::ZergGeszithalor});

	//// God - TBD

	// Testing
	for (u32 i = 0; i < 6; ++i) {
		threshold.frauds[FraudGod_Hand1 + i][RaceId::Terran] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {8, UnitId::TerranOlympian});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {4, UnitId::TerranApostle});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {2, UnitId::TerranAutocrat});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {3, UnitId::TerranPaladin});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {3, UnitId::TerranCentaur});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {2, UnitId::TerranMinotaur});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {1, UnitId::TerranPhobos});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Terran, {1, UnitId::TerranMagnetar});

		threshold.frauds[FraudGod_Hand1 + i][RaceId::Protoss] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Protoss, {6, UnitId::ProtossArchon});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Protoss, {4, UnitId::ProtossAtreus});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Protoss, {2, UnitId::ProtossAugur});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Protoss, {1, UnitId::ProtossDemiurge});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Protoss, {1, UnitId::ProtossStarSovereign});

		threshold.frauds[FraudGod_Hand1 + i][RaceId::Zerg] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {10, UnitId::ZergHydralisk});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {5, UnitId::ZergAlmaksalisk});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {4, UnitId::ZergKonvilisk});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {4, UnitId::ZergUltrokor});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergIsthrathaleth});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergAlkajelisk});
		add_fraud_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergKeskathalor});
	}
}