#include "threshold.h"
#include "unit_search.h"
#include "player_color.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/unit_destructor_special.h"
#include "ai_layouts.h"

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
#define FraudSourceCap 10
#define FraudRallyCap 100
#define FraudUnitCap 20
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

struct FraudSpawnUnit {
	u32 count;
	u32 unit_id;
};

struct FraudSpawnArray {
	u32 count;
	FraudSpawnUnit units[FraudUnitCap];
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
	FraudSpawnArray spawn_array;
};

struct ThresholdState {
	bool init;
	bool fraud_enabled[PlayerLimit];
	u32 fraud_slots;
	f32 scalar;
	f32 multiplier_scalar;

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
	FraudSpawnArray fraud_presets[Fraud_Count][RaceId::Count];
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
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold.fraud_enabled[player]) {
				continue;
			}
			if (threshold.sequence[player].location == &threshold.spawns[location]) {
				skip = true;
				break;
			}
		}
		if (skip) {
			continue;
		}

		EntryArray units = {};
		unit_search_radial(&units, threshold.spawns[location].pos, 12 * 32);
		u32 cost = units.count;
		unit_search_filter(&units, 0, FilterBuilding);
		cost += units.count;

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

InitPositions sources_positions;
InitPositions rally_positions;

bool init_threshold_spawn(CHKUnit* unit, u32 flags) {
	if (unit->player != 11) {
		return false;
	}

	Point16 position = { unit->x, unit->y };
	switch (unit->unit_id) {
		case UnitId::TownCenterMarker:
		{
			if (!(flags & ChkUnitFlags_Cloaked)) {
				FraudSpawnLocation spawn = {};
				spawn.pos = position;
				if (threshold.spawn_count < FraudSpawnCap) {
					threshold.spawns[threshold.spawn_count++] = spawn;
				} else {
					scbw::printf("Spawn cap reached (" STR(FraudSpawnCap) ")");
				}
			}
			return false; // We don't want to delete Town Center Markers, since they are supposed to be also processed by AI layouts
		} break;
		case UnitId::TerranSupplyDepot:
		{
			if (sources_positions.count < InitPositionsCap) {
				sources_positions.positions[sources_positions.count++] = position;
			}
			return true;
		} break;
		case UnitId::TerranMaverick:
		{
			if (rally_positions.count < InitPositionsCap) {
				rally_positions.positions[rally_positions.count++] = position;
			}
			return true;
		} break;
	}
	return false;
}

static void init_threshold_spawns() {
	for (u32 i = 0; i < sources_positions.count; ++i) {
		Point16 position = sources_positions.positions[i];
		FraudSpawnLocation* spawn = get_closest_spawn(position);
		if (spawn) {
			if (spawn->source_count < FraudSourceCap) {
				spawn->sources[spawn->source_count++] = { position };
			} else {
				scbw::printf("Source cap per spawn reached (" STR(FraudSourceCap) ")");
			}
		}
	}
	for (u32 i = 0; i < rally_positions.count; ++i) {
		Point16 position = rally_positions.positions[i];
		FraudSpawnLocation* spawn = get_closest_spawn(position);
		if (spawn) {
			FraudSpawnSource* source = get_closest_source(spawn, position);
			if (source) {
				if (source->rally_count < FraudRallyCap) {
					source->rallies[source->rally_count++] = position;
				} else {
					scbw::printf("Rally cap per source reached (" STR(FraudRallyCap) ")");
				}
			}
		}
	}
}

static void spawn_array_add(FraudSpawnArray* spawn_array, FraudSpawnUnit unit, bool merge = true) {
	if (merge) {
		for (u32 i = 0; i < spawn_array->count; ++i) {
			if (spawn_array->units[i].unit_id == unit.unit_id) {
				spawn_array->units[i].count += unit.count;
				return;
			}
		}
	}
	if (spawn_array->count < FraudUnitCap) {
		spawn_array->units[spawn_array->count++] = unit;
	} else {
		scbw::printf("Unit entry cap per fraud/race reached (" STR(FraudUnitCap) ")");
	}
}

static void fraud_add_unit(u32 fraud, u32 race, FraudSpawnUnit unit, bool merge = true) {
	FraudSpawnArray* fraud_preset = &threshold.fraud_presets[fraud][race];
	spawn_array_add(fraud_preset, unit, merge);
}

static s32 find_alliance(u32 fraud) {
	for (u32 i = 0; i < ARRAY_SIZE(fraud_alliances); ++i) {
		for (u32 j = 0; j < AlliancePlayerCap; ++j) {
			if (fraud_alliances[i][j] == fraud) {
				return i;
			}
		}
	}
	return -1;
}

static bool fraud_share_alliance(u32 a, u32 b) {
	if (threshold.sequence[a].fraud_class == threshold.sequence[b].fraud_class &&
		(threshold.sequence[a].fraud_class == FraudClass_Hero || threshold.sequence[a].fraud_class == FraudClass_God)) {
		return true;
	}

	s32 a_alliance = find_alliance(a);
	s32 b_alliance = find_alliance(b);
	return a_alliance != -1 && b_alliance != -1 && a_alliance == b_alliance;
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
	if (a == *g_local_nation_id || b == *g_local_nation_id) {
		helper::update_alliances();
	}
}

static void messagef(char* format, ...) {
	static char buffer[1024];

	va_list va;
	va_start(va, format);
	vsprintf_s(buffer, sizeof(buffer), format, va);
	va_end(va);
	
	helper::display_text_message(buffer, 2, GetTickCount() + helper::get_text_display_time(buffer), 1);
}

static u32 get_next_fraud(u32 fraud_class) {
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
	return Fraud_None;
}

static u32 get_fraud_class(u32 fraud) {
	if (fraud >= Fraud_First      && fraud <= Fraud_Last)      return FraudClass_Fraud;
	if (fraud >= FraudHero_First  && fraud <= FraudHero_Last)  return FraudClass_Hero;
	if (fraud >= FraudSuper_First && fraud <= FraudSuper_Last) return FraudClass_Super;
	if (fraud >= FraudGod_First   && fraud <= FraudGod_Last)   return FraudClass_God;
	return FraudClass_None;
}

static u32 get_fraud_race(u32 fraud) {
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

static void clear_player_slot(u32 player, bool map_start) {
	// Clear units
	Unit* unit_next = 0;
	for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit_next) {
		unit_next = unit->player_link.next;
		if (map_start) {
			helper::remove_creep_at_unit(unit->position.x, unit->position.y, unit->id);
			unit->user_action_flags |= 4; // silent remove
		}
		unit->remove();
	}

	// Clear fow sprites
	Thingy* thingy_next = 0;
	for (Thingy* thingy = *g_first_fow_sprite; thingy; thingy = thingy_next) {
		thingy_next = thingy->next;
		if (thingy->sprite->player == player) {
			scbw::delete_fow_sprite(thingy);
		}
	}

	// Clear AI towns
	AiTown* ai_town_prev = 0;
	for (AiTown* ai_town = g_ai_town_list[player].first; ai_town; ai_town = ai_town_prev) {
		ai_town_prev = ai_town->prev;
		scbw::delete_town(ai_town);
	}

	// Clear AI scripts
	AiScript* ai_script_next = 0;
	for (AiScript* ai_script = *g_first_active_aiscript; ai_script; ai_script = ai_script_next) {
		ai_script_next = ai_script->link.next;
		if (ai_script->player == player) {
			scbw::delete_ai_script(ai_script);
		}
	}
}

static u32 player_has_a_factory(u32 player) { // TODO: maybe cache this
	for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
		if (units_dat::group_flags[unit->id].is_factory) {
			return true;
		}
	}
	return false;
}

static bool fraud_is_alive(u32 player, bool active_only = false) {
	FraudSpawnSequence* sequence = &threshold.sequence[player];
	if (!sequence->fraud) {
		return active_only;
	}
	if (sequence->location || sequence->safety_timer || player_has_a_factory(player)) {
		return true;
	}
	return false;
}

static f32 get_multiplier() {
	s32 fraud_count = threshold.used_count - threshold_config.initial_count; // Don't count initial spawns towards the multiplier
	if (fraud_count < 0) {
		fraud_count = 0;
	}
	return (1 + fraud_count * 0.05) * threshold.multiplier_scalar;
}

static bool place_fraud(u32 player, u32 fraud_class, bool initial) {
	if (fraud_class == FraudClass_None) {
		return false;
	}
	if (fraud_is_alive(player)) {
		return false;
	}
	u32 fraud = get_next_fraud(fraud_class);
	if (fraud) {
		FraudSpawnLocation* spawn_location = get_random_spawn();
		if (spawn_location) {
			threshold.used_count += fraud_class;
			threshold.used[fraud] = true;
			threshold.timer = threshold_config.timer / threshold.scalar;
			if (fraud_class == FraudClass_God) {
				threshold.timer = 1 * 60 * 24;
			}

			messagef("Spawn %s\x02 (Class: %s\x01)!", fraud_names[fraud], fraud_class_names[fraud_class]);

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

			u32 race = get_fraud_race(fraud);
			g_player_table[player].race = race;

			FraudSpawnSequence* sequence = &threshold.sequence[player];
			sequence->fraud_class = fraud_class;
			sequence->fraud = fraud;
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
				g_unit_deaths_table[i].player[player] = threshold.frauds[fraud][race].deaths[i];
			}

			g_unit_deaths_table[UnitId::Spell_DisruptionWeb].player[player] = threshold.frauds[fraud][race].personality;
			g_unit_deaths_table[UnitId::ProtossPlasmaShell].player[player] = threshold.frauds[fraud][race].build_type;
			g_unit_deaths_table[UnitId::Spell_DarkSwarm].player[player] = threshold.frauds[fraud][race].build_order;
			set_player_color(player, threshold.frauds[fraud][race].color);
			set_player_name(player, (char*)fraud_names[fraud]);
			set_player_force_name(player, (char*)fraud_class_names[fraud_class]);

			s32 alliance = find_alliance(fraud);
			if (alliance != -1) {
				set_player_force_name(player, (char*)fraud_alliance_names[alliance]);
			}

			f32 multiplier = get_multiplier();
			sequence->spawn_array = threshold.fraud_presets[fraud][race];
			for (u32 i = 0; i < sequence->spawn_array.count; ++i) {
				if (initial) {
					if (units_dat::base_property[sequence->spawn_array.units[i].unit_id] & UnitProperty::Worker) {
						sequence->spawn_array.units[i].count = 6 * threshold.multiplier_scalar;
					} else {
						sequence->spawn_array.units[i].count = ceil(sequence->spawn_array.units[i].count * (threshold.multiplier_scalar - 1));
					}
				} else {
					sequence->spawn_array.units[i].count = ceil(sequence->spawn_array.units[i].count * multiplier);
					if (units_dat::base_property[sequence->spawn_array.units[i].unit_id] & UnitProperty::Worker) {
						if (sequence->spawn_array.units[i].count > 40 * threshold.multiplier_scalar) {
							sequence->spawn_array.units[i].count = 40 * threshold.multiplier_scalar;
						}
					}
				}
			}
			multiplier *= 1 + 0.5 * (fraud_class - 1);
			u32 minerals = 600 * multiplier;
			u32 gas      = 200 * multiplier;
			if (initial) {
				minerals = 100;
				gas      = 0;
			}
			resources->minerals[player] = minerals;
			resources->cumulative_minerals[player] += minerals;
			resources->gas[player] = gas;
			resources->cumulative_gas[player] += gas;

			g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player] = 1; // Disable AI upgrade overwrite
			u32 resource_upgrade = fraud_class;
			scbw::set_upgrade_level(player, UpgradeId::MiningEfficiency, resource_upgrade);

			g_start_positions[player] = spawn_location->pos;
			
			clear_player_slot(player, false);

			{
				Point16 pos = threshold.sequence[player].location->pos;
			//	Unit* augur = scbw::create_unit_in_game(UnitId::ProtossAugur, player, pos.x, pos.y);
			//	if (augur) {
			//		augur->user_action_flags |= 4;
			//		augur_sacrifice(augur, 10 * 32);
			//	}

				u32 townhall = UnitId::None;
				switch (race) {
					case RaceId::Terran:	townhall = UnitId::TerranCommandCenter; break;
					case RaceId::Protoss:	townhall = UnitId::ProtossNexus;		break;
					case RaceId::Zerg:		townhall = UnitId::ZergHachirosk;		break;
				}
				if (townhall != UnitId::None) {
					Unit* unit = helper::create_unit_at_pos(townhall, player, pos.x, pos.y);
					sequence->temp_townhall = unit;
					if (unit) {
						scbw::create_and_init_thingy(SpriteId::AugurDeathExplosion, pos.x, pos.y, player, unit->sprite->elevation + 1);
						helper::play_sound_at_pos(helper::get_random_sfx_between(SoundId::AugurDeath01, SoundId::AugurDeath04), pos);
					}
				}
			}

			for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
				if (player == other_player) {
					continue;
				}
				if (threshold.fraud_enabled[other_player] && threshold.sequence[other_player].fraud &&
					fraud_share_alliance(threshold.sequence[player].fraud, threshold.sequence[other_player].fraud)) {
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
				if (!threshold.fraud_enabled[other_player] || threshold_config.witness_enabled[other_player]) {
					player_vision->flags[player] |= 1 << other_player;
				}
			}

			scbw::run_default_ai(player, spawn_location->pos);
			return true;
		}
	}
	return false;
}

static void init_threshold_units();

void init_threshold() {
	memset(&threshold_config, 0, sizeof(threshold_config));
	memset(&threshold, 0, sizeof(threshold));
	memset(&sources_positions, 0, sizeof(sources_positions));
	memset(&rally_positions, 0, sizeof(rally_positions));

	threshold_config.total_count = 33;
	threshold_config.auto_scale = 0.5;
	threshold_config.timer = 4 * 60 * 24;
	threshold_config.initial_count = 2;
	threshold_config.min_count = 2;
}

void end_threshold() {
	memset(&threshold_config, 0, sizeof(threshold_config));
	memset(&threshold, 0, sizeof(threshold));
}

static void update_config() {
	bool first = true;
	threshold.scalar = 1;
	threshold.fraud_slots = 0;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (threshold_config.fraud_allowed[player] && g_player_table[player].type == PlayerType::Computer) {
			threshold.fraud_slots += 1;
			threshold.fraud_enabled[player] = true;
		} else {
			threshold.fraud_enabled[player] = false;
		}
		if (!threshold.fraud_enabled[player] && !threshold_config.witness_enabled[player]) {
			set_player_force_name(player, BWTC_WHITE "Contenders" BWTC_RESET);
			if (first) {
				first = false;
			} else {
				threshold.scalar += threshold_config.auto_scale;
			}
		}
		if (threshold_config.witness_enabled[player]) {
			set_player_force_name(player, BWTC_YELLOW "Witness" BWTC_RESET);
		}
	}
	if (threshold_config.total_count > Fraud_Count - FraudGod_Count) {
		threshold_config.total_count = Fraud_Count - FraudGod_Count;
	}
}

static bool try_prepare_threshold() {
	// Scenario Defaults
	if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch) ||
		is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence1) ||
		is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence2)) {
		threshold_config.enabled = true;
		for (u32 player = 1; player <= 6; ++player) {
			threshold_config.fraud_allowed[player] = true;
		}
		if (is_map(Campaign_OneForTheFuture, Mission_OFTF_TheCosmonarch) ||
			is_map(Campaign_Veeq7, Mission_Veeq7_Fraudscendence1)) {
			threshold_config.witness_enabled[7] = true;
		}
	}

	if (!threshold_config.enabled) {
		return false;
	}
	if (!threshold.init) {
		threshold.init = true;

		init_threshold_spawns();
		init_threshold_units();
		update_config();

		threshold.multiplier_scalar = 1;
		threshold_config.initial_count *= threshold.scalar;
		if (threshold_config.initial_count > threshold.fraud_slots) {
			threshold.multiplier_scalar += (threshold_config.initial_count - threshold.fraud_slots) * threshold_config.auto_scale / 2;
			threshold_config.initial_count = threshold.fraud_slots;
		}

		threshold.hero_spawn_counter = -1 - threshold_config.initial_count;
		threshold.super_spawn_counter = -4 - threshold_config.initial_count;
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (threshold.fraud_enabled[player]) {
				clear_player_slot(player, true);
			}
			if (threshold_config.witness_enabled[player]) {
				for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
					if (threshold.fraud_enabled[other_player]) {
						set_ally(player, other_player, false);
					} else {
						set_ally(player, other_player, true);
					}
					player_vision->flags[other_player] |= 1 << player;
				}
			}
		}

		threshold.timer = threshold_config.timer / threshold.scalar;
	}
	return threshold.init;
}

static void draw_fraud_ui() {
	static char buffer[64] = {};
	if (threshold.used_type_count[FraudClass_God]) {
		sprintf_s(buffer, sizeof(buffer), "Hands remaining: %u", 6 - threshold.used_type_count[FraudClass_God]);
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;

		if (threshold.used_type_count[FraudClass_God] < 6) {
			format_time(buffer, sizeof(buffer), "Next Hand: ", threshold.timer / 24);
			graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		}
		ui_text_pos.y += ui_text_height;
	} else if (threshold.used_count < threshold_config.total_count) {
		sprintf_s(buffer, sizeof(buffer), "Frauds remaining: %u", threshold_config.total_count - threshold.used_count);
		graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
		ui_text_pos.y += ui_text_height;

		if (threshold.used_count < threshold_config.total_count) {
			format_time(buffer, sizeof(buffer), "Next Fraud: ", threshold.timer / 24);
			graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
			ui_text_pos.y += ui_text_height;
		}
	}

	sprintf_s(buffer, sizeof(buffer), "Active Frauds:");
	graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
	ui_text_pos.y += ui_text_height;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold.fraud_enabled[player]) {
			continue;
		}
		FraudSpawnSequence* sequence = &threshold.sequence[player];
		if (sequence->fraud) {
			sprintf_s(buffer, sizeof(buffer), "%s\x02 [%s\x02]", fraud_names[sequence->fraud], fraud_class_names[sequence->fraud_class]);
			graphics::draw_text(ui_text_pos.x, ui_text_pos.y, buffer);
			ui_text_pos.y += ui_text_height;
		}
	}

}

static void cleanup_dead_frauds() {
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold.fraud_enabled[player]) {
			continue;
		}

		FraudSpawnSequence* sequence = &threshold.sequence[player];
		if (sequence->safety_timer) {
			--sequence->safety_timer;
		}
		if (!fraud_is_alive(player, true)) {
			if (sequence->fraud_class == FraudClass_God) {
				if (first_player_unit->unit[player]) {
					sequence->fraud = Fraud_None;
					sequence->fraud_class = FraudClass_None;
				}
			} else {
				messagef("%s\x01 (Class: %s\x01) died!", fraud_names[sequence->fraud], fraud_class_names[threshold.sequence[player].fraud_class]);
				sequence->safety_timer = 24;

				Unit* next = 0;
				for (Unit* unit = first_player_unit->unit[player]; unit; unit = next) {
					next = unit->player_link.next;
					unit->remove();
				}
				clear_player_slot(player, false);
				sequence->fraud = Fraud_None;
				sequence->fraud_class = FraudClass_None;
			}
		}
	}
}

void spawn_new_frauds() {
	if (threshold_config.initial_count) {
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold.fraud_enabled[player]) {
				continue;
			}
			if (threshold_config.initial_count) {
				if (*g_unit_count < 1600 && place_fraud(player, FraudClass_Fraud, true)) {
					threshold_config.initial_count -= 1;
					break;
				}
			} else {
				break;
			}
		}
		return;
	}

	u32 alive_count = 0;
	u32 non_god_alive_count = 0;
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold.fraud_enabled[player]) {
			continue;
		}
		if (fraud_is_alive(player)) {
			++alive_count;

			if (threshold.sequence[player].fraud_class != FraudClass_God) {
				++non_god_alive_count;
			}
		}
	}

	if (!alive_count && threshold.used_type_count[FraudClass_God] == 6) {
		g_unit_deaths_table[UnitId::Special_PitDoor].player[0] = 69;
	}

	u32 min_count = threshold_config.min_count * threshold.scalar;
	if (alive_count >= min_count && threshold.timer) {
		--threshold.timer;
	} else {
		threshold.timer = 0;
	}
	if (!threshold.timer) {
		u32 fraud_class = FraudClass_None;
		if (threshold.used_count < threshold_config.total_count) {
			fraud_class = FraudClass_Fraud;
			if (get_next_fraud(FraudClass_Hero) && (s32)scbw::rand_between(0, 99) < threshold.hero_spawn_counter * 33) {
				fraud_class = FraudClass_Hero;
			} else if (get_next_fraud(FraudClass_Super) && (s32)scbw::rand_between(0, 99) < threshold.super_spawn_counter * 20) {
				fraud_class = FraudClass_Super;
			}
		} else if (threshold.used_type_count[FraudClass_God]) {
			if (threshold.used_type_count[FraudClass_God] < 3) {
				fraud_class = FraudClass_None;
			} else {
				fraud_class = FraudClass_God;
			}
		}

		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold.fraud_enabled[player]) {
				continue;
			}
			FraudSpawnSequence* sequence = &threshold.sequence[player];
			if (!fraud_is_alive(player) && *g_unit_count < 1600) {
				if (place_fraud(player, fraud_class, false)) {
					++alive_count;
					break;
				}
			}
		}
	}

	if (!non_god_alive_count && threshold.used_count >= threshold_config.total_count && threshold.used_type_count[FraudClass_God] < 3) {
		if (threshold.used_type_count[FraudClass_God] < 3) {
			threshold.timer = 0;
		}
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (!threshold.fraud_enabled[player]) {
				continue;
			}
			if (place_fraud(player, FraudClass_God, false)) {
				break;
			}
		}
	}
}

static void spawn_sequence_units() {
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (!threshold.fraud_enabled[player]) {
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
				u32 unit_count = sequence->spawn_array.count;
				// Skip units without count
				while (sequence->unit_index < unit_count
					&& !sequence->spawn_array.units[sequence->unit_index].count) {
					++sequence->unit_index;
				}

				// Spawn a unit
				if (sequence->unit_index < unit_count && *g_unit_count < UnitLimit) {
					FraudSpawnSource* source = &sequence->location->sources[sequence->source_index % sequence->location->source_count];
					FraudSpawnUnit* spawn_unit = &sequence->spawn_array.units[sequence->unit_index];
					u32 unit_id = spawn_unit->unit_id;
					if (source->rally_count) {
						Point16 source_pos = source->pos;
						Point16 rally = source->rallies[sequence->rally_index % source->rally_count];

					//	Unit* augur = scbw::create_unit_in_game(UnitId::ProtossAugur, player, source_pos.x, source_pos.y);
					//	if (augur) {
					//		augur->user_action_flags |= 4;
					//		augur_sacrifice(augur, 10 * 32);
					//	}
						Unit* unit = scbw::create_unit_in_game(unit_id, player, source_pos.x, source_pos.y);
						if (unit) {
							scbw::create_and_init_thingy(SpriteId::AugurDeathExplosion, source_pos.x, source_pos.y, unit->player, unit->sprite->elevation + 1);
							helper::play_sound_at_pos(helper::get_random_sfx_between(SoundId::AugurDeath01, SoundId::AugurDeath04), source_pos);
							if (!(units_dat::base_property[unit_id] & UnitProperty::Worker)) {
								unit->order_to(OrderId::AttackMove, rally.x, rally.y);
							}

							sequence->unit_timer = 24 * 0.3 / get_multiplier();
							assert(spawn_unit->count); // We should implictly have set count, since we first skipped all the units without count
							spawn_unit->count -= 1;
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
						if (!threshold.fraud_enabled[other_player] && !threshold_config.witness_enabled[other_player]) {
							player_vision->flags[player] &= ~(1 << other_player);
						}
					}
					sequence->location = 0;
				}
			}
		}
	}
}

void update_threshold() {
	if (!try_prepare_threshold()) {
		return;
	}
	update_config();
	draw_fraud_ui();
	cleanup_dead_frauds();
	spawn_new_frauds();
	spawn_sequence_units();
}

static void init_threshold_units() {
	for (u32 i = 0; i < Fraud_Count; ++i) {
		switch (get_fraud_class(i)) {
			case FraudClass_Fraud:
			{
				fraud_add_unit(i, RaceId::Terran,  {12, UnitId::TerranMason});
				fraud_add_unit(i, RaceId::Terran,  {1, UnitId::TerranAncile});

				fraud_add_unit(i, RaceId::Protoss, {12, UnitId::ProtossScribe});
				fraud_add_unit(i, RaceId::Protoss, {1, UnitId::ProtossWitness});

				fraud_add_unit(i, RaceId::Zerg,    {12, UnitId::ZergDroleth});
				fraud_add_unit(i, RaceId::Zerg,    {1, UnitId::ZergOvileth});
			} break;
			case FraudClass_Hero:
			{
				fraud_add_unit(i, RaceId::Terran,  {10, UnitId::TerranMason});
				fraud_add_unit(i, RaceId::Terran,  {2, UnitId::TerranTalos});
				fraud_add_unit(i, RaceId::Terran,  {2, UnitId::TerranAncile});

				fraud_add_unit(i, RaceId::Protoss, {10, UnitId::ProtossScribe});
				fraud_add_unit(i, RaceId::Protoss, {2, UnitId::ProtossArtisan});
				fraud_add_unit(i, RaceId::Protoss, {2, UnitId::ProtossWitness});

				fraud_add_unit(i, RaceId::Zerg,    {14, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {10, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {2, UnitId::ZergGosvileth});
				fraud_add_unit(i, RaceId::Zerg,    {1, UnitId::ZergIroleth});
			} break;
			case FraudClass_Super:
			{
				fraud_add_unit(i, RaceId::Terran,  {8, UnitId::TerranMason});
				fraud_add_unit(i, RaceId::Terran,  {4, UnitId::TerranTalos});
				fraud_add_unit(i, RaceId::Terran,  {1, UnitId::TerranSeraph});

				fraud_add_unit(i, RaceId::Protoss, {8, UnitId::ProtossScribe});
				fraud_add_unit(i, RaceId::Protoss, {4, UnitId::ProtossArtisan});
				fraud_add_unit(i, RaceId::Protoss, {3, UnitId::ProtossWitness});

				fraud_add_unit(i, RaceId::Zerg,    {16, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {8, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {4, UnitId::ZergGosvileth});
				fraud_add_unit(i, RaceId::Zerg,    {1, UnitId::ZergOthstoleth});
			} break;
			case FraudClass_God:
			{
				fraud_add_unit(i, RaceId::Terran,  {6, UnitId::TerranMason});
				fraud_add_unit(i, RaceId::Terran,  {6, UnitId::TerranTalos});
				fraud_add_unit(i, RaceId::Terran,  {2, UnitId::TerranSeraph});

				fraud_add_unit(i, RaceId::Protoss, {6, UnitId::ProtossScribe});
				fraud_add_unit(i, RaceId::Protoss, {6, UnitId::ProtossArtisan});
				fraud_add_unit(i, RaceId::Protoss, {4, UnitId::ProtossWitness});

				fraud_add_unit(i, RaceId::Zerg,    {18, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {6, UnitId::ZergDroleth});
			//	fraud_add_unit(i, RaceId::Zerg,    {6, UnitId::ZergGosvileth});
				fraud_add_unit(i, RaceId::Zerg,    {2, UnitId::ZergOthstoleth});
			} break;
		}
	}

	//// Frauds

	// Alexei Ukov
	threshold.frauds[Fraud_AlexeiUkov][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::VividRed};
	fraud_add_unit(Fraud_AlexeiUkov, RaceId::Terran, {5, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_AlexeiUkov, RaceId::Terran, {2, UnitId::TerranWraith});
	fraud_add_unit(Fraud_AlexeiUkov, RaceId::Terran, {1, UnitId::TerranGorgon});

	// Ullen "Bank on Me" Bigmunney
	threshold.frauds[Fraud_UllenBankOnMeBigmunney][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_None, AiBuildOrder_TerranArmor_IronCrusade, PlayerColor::VineGreen};
	fraud_add_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {4, UnitId::TerranMason});
	fraud_add_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {2, UnitId::TerranGoliath});
	fraud_add_unit(Fraud_UllenBankOnMeBigmunney, RaceId::Terran, {4, UnitId::TerranBlackjack});

	// knightoftherealm
	threshold.frauds[Fraud_knightoftherealm][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Aero};
	fraud_add_unit(Fraud_knightoftherealm, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_knightoftherealm, RaceId::Terran, {4, UnitId::TerranCyprian});
	fraud_add_unit(Fraud_knightoftherealm, RaceId::Terran, {1, UnitId::TerranShaman});

	// KKKhris
	threshold.frauds[Fraud_KKKhris][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brown};
	fraud_add_unit(Fraud_KKKhris, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_KKKhris, RaceId::Terran, {3, UnitId::TerranGoliath});
	fraud_add_unit(Fraud_KKKhris, RaceId::Terran, {2, UnitId::TerranWraith});

	// Dynamo
	threshold.frauds[Fraud_Dynamo][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_3RaxMarineAndCleric, PlayerColor::VividOrange};
	fraud_add_unit(Fraud_Dynamo, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_Dynamo, RaceId::Terran, {1, UnitId::TerranOlympian});

	// TodsDadPat
	threshold.frauds[Fraud_TodsDadPat][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Platinum};
	fraud_add_unit(Fraud_TodsDadPat, RaceId::Terran, {3, UnitId::TerranVulture});
	fraud_add_unit(Fraud_TodsDadPat, RaceId::Terran, {5, UnitId::TerranCyclops});

	// Piglet
	threshold.frauds[Fraud_Piglet][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cordovan};
	fraud_add_unit(Fraud_Piglet, RaceId::Terran, {3, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_Piglet, RaceId::Terran, {2, UnitId::TerranCyprian});
	fraud_add_unit(Fraud_Piglet, RaceId::Terran, {3, UnitId::TerranHarakan});
	fraud_add_unit(Fraud_Piglet, RaceId::Terran, {2, UnitId::TerranCleric});

	// DarkenedFantasies
	threshold.frauds[Fraud_DarkenedFantasies][RaceId::Terran] = { AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSpringGreen};
	fraud_add_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranVulture});
	fraud_add_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranCyclops});
	fraud_add_unit(Fraud_DarkenedFantasies, RaceId::Terran, {2, UnitId::TerranGoliath});

	// Customer Supporter
	threshold.frauds[Fraud_CustomerSupporter][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Turquoise};
	fraud_add_unit(Fraud_CustomerSupporter, RaceId::Terran, {2, UnitId::TerranWraith});
	fraud_add_unit(Fraud_CustomerSupporter, RaceId::Terran, {1, UnitId::TerranGorgon});

	// Customer Supporter
	threshold.frauds[Fraud_Desler][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Mint};
	fraud_add_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranEidolon});
	fraud_add_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranCyclops});
	fraud_add_unit(Fraud_Desler, RaceId::Terran, {2, UnitId::TerranBlackjack});

	// Bitcoin Boris
	threshold.frauds[Fraud_BitcoinBoris][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::CoyoteBrown};
	fraud_add_unit(Fraud_BitcoinBoris, RaceId::Protoss, {1, UnitId::ProtossAmaranth});
	fraud_add_unit(Fraud_BitcoinBoris, RaceId::Protoss, {2, UnitId::ProtossVagrant});
	fraud_add_unit(Fraud_BitcoinBoris, RaceId::Protoss, {2, UnitId::ProtossBarghest});

	// Crypto Carl
	threshold.frauds[Fraud_CryptoCarl][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_None, AiBuildOrder_ProtossSkylord_SolarEscort, PlayerColor::BistreBrown};
	fraud_add_unit(Fraud_CryptoCarl, RaceId::Protoss, {2, UnitId::ProtossPanoptus});
	fraud_add_unit(Fraud_CryptoCarl, RaceId::Protoss, {1, UnitId::ProtossMagister});
	fraud_add_unit(Fraud_CryptoCarl, RaceId::Protoss, {1, UnitId::ProtossEpigraph});

	// Veeq7
	threshold.frauds[Fraud_Veeq7][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RoyalBlue};
	fraud_add_unit(Fraud_Veeq7, RaceId::Protoss, {6, UnitId::ProtossSimulacrum});
	fraud_add_unit(Fraud_Veeq7, RaceId::Protoss, {2, UnitId::ProtossDracadin});

	// jun3hong
	threshold.frauds[Fraud_jun3hong][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::PersianOrange};
	fraud_add_unit(Fraud_jun3hong, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	fraud_add_unit(Fraud_jun3hong, RaceId::Protoss, {2, UnitId::ProtossHierophant});
	fraud_add_unit(Fraud_jun3hong, RaceId::Protoss, {1, UnitId::ProtossClarion});

	// Digital Rights Manager
	threshold.frauds[Fraud_DigitalRightsManager][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Glacial};
	fraud_add_unit(Fraud_DigitalRightsManager, RaceId::Protoss, {4, UnitId::ProtossAmaranth});
	fraud_add_unit(Fraud_DigitalRightsManager, RaceId::Protoss, {2, UnitId::ProtossHierophant});

	// Zeratool
	threshold.frauds[Fraud_Zeratool][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TyrianPurple};
	fraud_add_unit(Fraud_Zeratool, RaceId::Protoss, {2, UnitId::ProtossVagrant});
	fraud_add_unit(Fraud_Zeratool, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	fraud_add_unit(Fraud_Zeratool, RaceId::Protoss, {2, UnitId::ProtossBarghest});

	// F.R.A.U.D. Grandpa of Tony
	threshold.frauds[Fraud_FRAUDGrandpaOfTony][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Peach};
	fraud_add_unit(Fraud_FRAUDGrandpaOfTony, RaceId::Protoss, {4, UnitId::ProtossZealot});
	fraud_add_unit(Fraud_FRAUDGrandpaOfTony, RaceId::Protoss, {2, UnitId::ProtossDracadin});

	// DK
	threshold.frauds[Fraud_DK][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::EggshellBlue};
	fraud_add_unit(Fraud_DK, RaceId::Protoss, {4, UnitId::ProtossAurora});
	fraud_add_unit(Fraud_DK, RaceId::Protoss, {2, UnitId::ProtossPanoptus});

	// iquare
	threshold.frauds[Fraud_iquare][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Brass};
	fraud_add_unit(Fraud_iquare, RaceId::Protoss, {3, UnitId::ProtossManifold});
	fraud_add_unit(Fraud_iquare, RaceId::Protoss, {2, UnitId::ProtossIdol});
	fraud_add_unit(Fraud_iquare, RaceId::Protoss, {2, UnitId::ProtossAccantor});

	// Metathrom
	threshold.frauds[Fraud_Metathrom][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::BdazzledBlue};
	fraud_add_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::ProtossHierophant});
	fraud_add_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::ProtossAmaranth});
	fraud_add_unit(Fraud_Metathrom, RaceId::Protoss, {2, UnitId::ProtossCantavis});

	// Cerebrate Stalin
	threshold.frauds[Fraud_CerebrateStalin][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Mango};
	fraud_add_unit(Fraud_CerebrateStalin, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_CerebrateStalin, RaceId::Zerg, {2, UnitId::ZergLiiralisk});

	// Andy Rose Uh
	threshold.frauds[Fraud_AndyRoseUh][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Orange};
	fraud_add_unit(Fraud_AndyRoseUh, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_AndyRoseUh, RaceId::Zerg, {2, UnitId::ZergMutalisk});

	// Winnie
	threshold.frauds[Fraud_Winnie][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Yellow};
	fraud_add_unit(Fraud_Winnie, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_Winnie, RaceId::Zerg, {4, UnitId::ZergQuazilisk});

	// A
	threshold.frauds[Fraud_A][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::ForestGreen};
	fraud_add_unit(Fraud_A, RaceId::Zerg, {6, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_A, RaceId::Zerg, {4, UnitId::ZergCikralisk});

	// Sellout Sam
	threshold.frauds[Fraud_SelloutSam][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Beaver};
	fraud_add_unit(Fraud_SelloutSam, RaceId::Zerg, {6, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_SelloutSam, RaceId::Zerg, {2, UnitId::ZergVorvrokor});
	fraud_add_unit(Fraud_SelloutSam, RaceId::Zerg, {3, UnitId::ZergLiiralisk});

	// Plagiarist Paul
	threshold.frauds[Fraud_PlagiaristPaul][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::ZinnwalditeBrown};
	fraud_add_unit(Fraud_PlagiaristPaul, RaceId::Zerg, {6, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_PlagiaristPaul, RaceId::Zerg, {2, UnitId::ZergKonvilisk});

	// RimTechCombot
	threshold.frauds[Fraud_RimTechCombot][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::DarkLavender};
	fraud_add_unit(Fraud_RimTechCombot, RaceId::Zerg, {6, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_RimTechCombot, RaceId::Zerg, {3, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_RimTechCombot, RaceId::Zerg, {6, UnitId::ZergNathrokor});

	// Africanized Crowkenyan
	threshold.frauds[Fraud_AfricanizedCrowkenyan][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::DarkOlive};
	fraud_add_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {8, UnitId::ZergNathrokor});
	fraud_add_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {2, UnitId::ZergSkithrokor});
	fraud_add_unit(Fraud_AfricanizedCrowkenyan, RaceId::Zerg, {2, UnitId::ZergMutalisk});

	// Ultralisks
	threshold.frauds[Fraud_Ultralisks][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Silver};
	fraud_add_unit(Fraud_Ultralisks, RaceId::Zerg, {2, UnitId::ZergZoryusthaleth});
	fraud_add_unit(Fraud_Ultralisks, RaceId::Zerg, {2, UnitId::ZergGorgrokor});
	fraud_add_unit(Fraud_Ultralisks, RaceId::Zerg, {1, UnitId::ZergUltrokor});

	// N Thrower
	threshold.frauds[Fraud_NThrower][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Cyan};
	fraud_add_unit(Fraud_NThrower, RaceId::Zerg, {12, UnitId::ZergRililisk});
	fraud_add_unit(Fraud_NThrower, RaceId::Zerg, {2, UnitId::ZergKonvilisk});
	fraud_add_unit(Fraud_NThrower, RaceId::Zerg, {2, UnitId::ZergMatraleth});

	// LightenedRealities
	threshold.frauds[Fraud_LightenedRealities][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Terran, {4, UnitId::TerranHeracles});
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Terran, {2, UnitId::TerranSiren});

	threshold.frauds[Fraud_LightenedRealities][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Protoss, {2, UnitId::ProtossZealot});
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Protoss, {1, UnitId::ProtossAccantor});
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Protoss, {2, UnitId::ProtossGladius});

	threshold.frauds[Fraud_LightenedRealities][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Harlequin};
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Zerg, {2, UnitId::ZergZoryusthaleth});
	fraud_add_unit(Fraud_LightenedRealities, RaceId::Zerg, {1, UnitId::ZergGeszithalor});

	// AchromaticHalfdreams
	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {2, UnitId::TerranMadcap});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranApostle});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Terran, {1, UnitId::TerranSiren});

	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {1, UnitId::ProtossArchon});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {2, UnitId::ProtossGolem});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Protoss, {2, UnitId::ProtossExemplar});

	threshold.frauds[Fraud_AchromaticHalfdreams][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::TeaGreen};
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {2, UnitId::ZergLiiralisk});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {2, UnitId::ZergKalkalisk});
	fraud_add_unit(Fraud_AchromaticHalfdreams, RaceId::Zerg, {2, UnitId::ZergZoryusthaleth});

	// TonyTron
	threshold.frauds[Fraud_TonyTron][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	fraud_add_unit(Fraud_TonyTron, RaceId::Terran, {3, UnitId::TerranWraith});
	fraud_add_unit(Fraud_TonyTron, RaceId::Terran, {2, UnitId::TerranGorgon});
	fraud_add_unit(Fraud_TonyTron, RaceId::Terran, {1, UnitId::TerranClaymore});

	threshold.frauds[Fraud_TonyTron][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	fraud_add_unit(Fraud_TonyTron, RaceId::Protoss, {2, UnitId::ProtossPanoptus});
	fraud_add_unit(Fraud_TonyTron, RaceId::Protoss, {2, UnitId::ProtossGladius});
	fraud_add_unit(Fraud_TonyTron, RaceId::Protoss, {1, UnitId::ProtossExemplar});

	threshold.frauds[Fraud_TonyTron][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinateBlue};
	fraud_add_unit(Fraud_TonyTron, RaceId::Zerg, {2, UnitId::ZergMutalisk});
	fraud_add_unit(Fraud_TonyTron, RaceId::Zerg, {2, UnitId::ZergKalkalisk});
	fraud_add_unit(Fraud_TonyTron, RaceId::Zerg, {1, UnitId::ZergGeszithalor});

	// Pr0nogo
	threshold.frauds[Fraud_Pr0nogo][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Terran, {2, UnitId::TerranCyclops});
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Terran, {2, UnitId::TerranGoliath});
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Terran, {2, UnitId::TerranBlackjack});

	threshold.frauds[Fraud_Pr0nogo][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Protoss, {2, UnitId::ProtossZealot});
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Protoss, {2, UnitId::ProtossEcclesiast});
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Protoss, {2, UnitId::ProtossCabalist});

	threshold.frauds[Fraud_Pr0nogo][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Aquamarine};
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_Pr0nogo, RaceId::Zerg, {2, UnitId::ZergBactalisk});

	// Ba3lethal
	threshold.frauds[Fraud_Ba3lethal][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Terran, {2, UnitId::TerranCleric});
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Terran, {2, UnitId::TerranMadcap});

	threshold.frauds[Fraud_Ba3lethal][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Protoss, {2, UnitId::ProtossAtreus});

	threshold.frauds[Fraud_Ba3lethal][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::NeonCarrot};
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_Ba3lethal, RaceId::Zerg, {2, UnitId::ZergEvigrilisk});

	// Towering Pigprophet
	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Terran, {4, UnitId::TerranWraith});
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Terran, {1, UnitId::TerranGorgon});
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Terran, {2, UnitId::TerranSundog});

	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Protoss, {4, UnitId::ProtossAurora});
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Protoss, {1, UnitId::ProtossEpigraph});

	threshold.frauds[Fraud_ToweringPigprophet][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DarkSapphire};
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Zerg, {6, UnitId::ZergNathrokor});
	fraud_add_unit(Fraud_ToweringPigprophet, RaceId::Zerg, {2, UnitId::ZergTosgrilisk});

	// Anal Crowsummoner
	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Terran, {2, UnitId::TerranCyprian});
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Terran, {2, UnitId::TerranBlackjack});

	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Protoss, {5, UnitId::ProtossManifold});
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Protoss, {3, UnitId::ProtossIdol});

	threshold.frauds[Fraud_AnalCrowsummoner][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::Grey};
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Zerg, {6, UnitId::ZergNathrokor});
	fraud_add_unit(Fraud_AnalCrowsummoner, RaceId::Zerg, {4, UnitId::ZergZethrokor});

	// Pissing Porncultist
	threshold.frauds[Fraud_PissingPorncultist][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Terran, {2, UnitId::TerranCleric});
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Terran, {2, UnitId::TerranVulture});

	threshold.frauds[Fraud_PissingPorncultist][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Protoss, {2, UnitId::ProtossLegionnaire});
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Protoss, {4, UnitId::ProtossDracadin});

	threshold.frauds[Fraud_PissingPorncultist][RaceId::Zerg] = {AiPersonality_ZergSunblot, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::IridescentYellow};
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_PissingPorncultist, RaceId::Zerg, {4, UnitId::ZergHydralisk});

	// Bearborne Arkanianpussy
	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Terran, {2, UnitId::TerranGoliath});
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Terran, {4, UnitId::TerranBlackjack});

	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {2, UnitId::ProtossLegionnaire});
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {2, UnitId::ProtossEcclesiast});
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {1, UnitId::ProtossHierophant});
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Protoss, {1, UnitId::ProtossCharlatan});

	threshold.frauds[Fraud_BearborneArkanianpussy][RaceId::Zerg] = { AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::IridescentGreen};
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_BearborneArkanianpussy, RaceId::Zerg, {2, UnitId::ZergLiiralisk});

	// Doubling Wuckdevourer
	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Terran, {4, UnitId::TerranCyclops});
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Terran, {1, UnitId::TerranPhalanxTankMode});

	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {3, UnitId::ProtossZealot});
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {1, UnitId::ProtossDracadin});
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Protoss, {1, UnitId::ProtossEcclesiast});

	threshold.frauds[Fraud_DoublingWuckdevourer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::PalatinatePurple};
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_DoublingWuckdevourer, RaceId::Zerg, {2, UnitId::ZergLakizilisk});

	// Mongoloid Pointcorruptor
	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {2, UnitId::TerranCleric});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {1, UnitId::TerranApostle});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Terran, {1, UnitId::TerranSiren});

	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {4, UnitId::ProtossVassal});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {4, UnitId::ProtossSimulacrum});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Protoss, {1, UnitId::ProtossIdol});

	threshold.frauds[Fraud_MongoloidPointcorruptor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::AliceBlue};
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Zerg, {6, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_MongoloidPointcorruptor, RaceId::Zerg, {2, UnitId::ZergCikralisk});

	// Calendared Rageboy
	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	fraud_add_unit(Fraud_CalendaredRageboy, RaceId::Terran, {4, UnitId::TerranHeracles});

	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	fraud_add_unit(Fraud_CalendaredRageboy, RaceId::Protoss, {4, UnitId::ProtossAmaranth});

	threshold.frauds[Fraud_CalendaredRageboy][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Red};
	fraud_add_unit(Fraud_CalendaredRageboy, RaceId::Zerg, {3, UnitId::ZergUltrokor});

	// Worthy Pizzasniffer
	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Terran, {6, UnitId::TerranHarakan});
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Terran, {1, UnitId::TerranApostle});

	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Protoss, {4, UnitId::ProtossLegionnaire});
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Protoss, {2, UnitId::ProtossAtreus});

	threshold.frauds[Fraud_WorthyPizzasniffer][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Magma};
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {4, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {2, UnitId::ZergVorvrokor});
	fraud_add_unit(Fraud_WorthyPizzasniffer, RaceId::Zerg, {4, UnitId::ZergNathrokor});

	// Skeletal Todzombie
	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Terran, {4, UnitId::TerranPhalanxTankMode});

	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Protoss, {2, UnitId::ProtossHerald});

	threshold.frauds[Fraud_SkeletalTodzombie][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::SilverPink};
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {2, UnitId::ZergBactalisk});
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_SkeletalTodzombie, RaceId::Zerg, {1, UnitId::ZergZoryusthaleth});

	// Hoop Thrower
	threshold.frauds[Fraud_HoopThrower][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	fraud_add_unit(Fraud_HoopThrower, RaceId::Terran, {6, UnitId::TerranCyprian});
	fraud_add_unit(Fraud_HoopThrower, RaceId::Terran, {2, UnitId::TerranMadcap});

	threshold.frauds[Fraud_HoopThrower][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	fraud_add_unit(Fraud_HoopThrower, RaceId::Protoss, {3, UnitId::ProtossZealot});
	fraud_add_unit(Fraud_HoopThrower, RaceId::Protoss, {3, UnitId::ProtossDracadin});

	threshold.frauds[Fraud_HoopThrower][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::RussianGreen};
	fraud_add_unit(Fraud_HoopThrower, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_HoopThrower, RaceId::Zerg, {2, UnitId::ZergCikralisk});

	// Veeq8
	threshold.frauds[Fraud_Veeq8][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_add_unit(Fraud_Veeq8, RaceId::Terran, {4, UnitId::TerranWraith});
	fraud_add_unit(Fraud_Veeq8, RaceId::Terran, {2, UnitId::TerranSalamander});

	threshold.frauds[Fraud_Veeq8][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_add_unit(Fraud_Veeq8, RaceId::Protoss, {2, UnitId::ProtossGolem});
	fraud_add_unit(Fraud_Veeq8, RaceId::Protoss, {6, UnitId::ProtossSimulacrum});
	fraud_add_unit(Fraud_Veeq8, RaceId::Protoss, {1, UnitId::ProtossClarion});

	threshold.frauds[Fraud_Veeq8][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::NeonBlue};
	fraud_add_unit(Fraud_Veeq8, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(Fraud_Veeq8, RaceId::Zerg, {4, UnitId::ZergLiiralisk});

	// Bonzibuddy
	threshold.frauds[Fraud_Bonzibuddy][RaceId::Terran] = {AiPersonality_TerranGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Terran, {8, UnitId::TerranMadcap});

	threshold.frauds[Fraud_Bonzibuddy][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Protoss, {2, UnitId::ProtossAmaranth});
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Protoss, {3, UnitId::ProtossEcclesiast});
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Protoss, {1, UnitId::ProtossVagrant});

	threshold.frauds[Fraud_Bonzibuddy][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Greed, AiBuildOrder_None, PlayerColor::JapaneseViolet};
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Zerg, {4, UnitId::ZergQuazilisk});
	fraud_add_unit(Fraud_Bonzibuddy, RaceId::Zerg, {4, UnitId::ZergLiiralisk});

	// IskatuMesk
	threshold.frauds[Fraud_IskatuMesk][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Terran, {4, UnitId::TerranBlackjack});
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Terran, {2, UnitId::TerranCleric});

	threshold.frauds[Fraud_IskatuMesk][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Protoss, {4, UnitId::ProtossCabalist});
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Protoss, {2, UnitId::ProtossPanoptus});

	threshold.frauds[Fraud_IskatuMesk][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Chestnut};
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Zerg, {4, UnitId::ZergHydralisk});
	fraud_add_unit(Fraud_IskatuMesk, RaceId::Zerg, {2, UnitId::ZergKagralisk});

	//// Heroes

	// Francisco Goska
	threshold.frauds[FraudHero_Francisco_Goska][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_8Rax, PlayerColor::ElectricOrange};
	fraud_add_unit(FraudHero_Francisco_Goska, RaceId::Terran, {4, UnitId::TerranMaverick});
	fraud_add_unit(FraudHero_Francisco_Goska, RaceId::Terran, {2, UnitId::TerranCleric});
	fraud_add_unit(FraudHero_Francisco_Goska, RaceId::Terran, {4, UnitId::TerranMadcap});
	fraud_add_unit(FraudHero_Francisco_Goska, RaceId::Terran, {3, UnitId::TerranApostle});
	fraud_add_unit(FraudHero_Francisco_Goska, RaceId::Terran, {2, UnitId::TerranAutocrat});

	// Ackmed Clone
	threshold.frauds[FraudHero_Ackmed_Clone][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_HolyAckmeds, PlayerColor::DarkBronze};
	fraud_add_unit(FraudHero_Ackmed_Clone, RaceId::Terran, {16, UnitId::TerranHarakan});
	fraud_add_unit(FraudHero_Ackmed_Clone, RaceId::Terran, {4, UnitId::TerranHeracles});

	// Cullen
	threshold.frauds[FraudHero_Cullen][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_None, AiBuildOrder_TerranOrbital_2PortWraith, PlayerColor::MayaBlue};
	fraud_add_unit(FraudHero_Cullen, RaceId::Terran, {8, UnitId::TerranWraith});
	fraud_add_unit(FraudHero_Cullen, RaceId::Terran, {2, UnitId::TerranCentaur});
	fraud_add_unit(FraudHero_Cullen, RaceId::Terran, {1, UnitId::TerranMagnetar});

	// Steve Miller
	threshold.frauds[FraudHero_SteveMiller][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_None, AiBuildOrder_TerranArmor_2FulcVult, PlayerColor::White};
	fraud_add_unit(FraudHero_SteveMiller, RaceId::Terran, {6, UnitId::TerranPhalanxTankMode});
	fraud_add_unit(FraudHero_SteveMiller, RaceId::Terran, {2, UnitId::TerranMadrigal});
	fraud_add_unit(FraudHero_SteveMiller, RaceId::Terran, {2, UnitId::TerranPaladin});

	// Torros
	threshold.frauds[FraudHero_Torros][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_None, AiBuildOrder_ProtossTemplar_Initiates, PlayerColor::Saffron};
	fraud_add_unit(FraudHero_Torros, RaceId::Protoss, {6, UnitId::ProtossAmaranth});
	fraud_add_unit(FraudHero_Torros, RaceId::Protoss, {4, UnitId::ProtossAtreus});

	// Drakkar
	threshold.frauds[FraudHero_Drakkar][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_None, AiBuildOrder_ProtossRanger_2GateStrider, PlayerColor::Emerald};
	fraud_add_unit(FraudHero_Drakkar, RaceId::Protoss, {4, UnitId::ProtossHierophant});
	fraud_add_unit(FraudHero_Drakkar, RaceId::Protoss, {4, UnitId::ProtossAccantor});
	fraud_add_unit(FraudHero_Drakkar, RaceId::Protoss, {1, UnitId::ProtossArchitect});

	// Thalazim
	threshold.frauds[FraudHero_Thalazim][RaceId::Protoss] = {AiPersonality_ProtossTemplar, AiBuildType_None, AiBuildOrder_ProtossTemplar_SuffocatingShadow, PlayerColor::Earthtone};
	fraud_add_unit(FraudHero_Thalazim, RaceId::Protoss, {8, UnitId::ProtossCabalist});
	fraud_add_unit(FraudHero_Thalazim, RaceId::Protoss, {2, UnitId::ProtossPariah});

	// Echo of Mixplay
	threshold.frauds[FraudHero_EchoOfMixplay][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_None, AiBuildOrder_ProtossGeneral_LunaticLegion, PlayerColor::ElectricBlue};
	fraud_add_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {4, UnitId::ProtossDracadin});
	fraud_add_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {2, UnitId::ProtossIdol});
	fraud_add_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {2, UnitId::ProtossGolem});
	fraud_add_unit(FraudHero_EchoOfMixplay, RaceId::Protoss, {1, UnitId::ProtossSolarion});

	// Stroluum
	threshold.frauds[FraudHero_Stroluum][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_None, AiBuildOrder_ZergSwarm_InstantQuazrok, PlayerColor::FernGreen};
	fraud_add_unit(FraudHero_Stroluum, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(FraudHero_Stroluum, RaceId::Zerg, {4, UnitId::ZergUltrokor});
	fraud_add_unit(FraudHero_Stroluum, RaceId::Zerg, {1, UnitId::ZergIsthrathaleth});
	fraud_add_unit(FraudHero_Stroluum, RaceId::Zerg, {3, UnitId::ZergLakizilisk});

	// Tuzor
	threshold.frauds[FraudHero_Tuzor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_None, AiBuildOrder_ZergAssault_3HatchHydrok, PlayerColor::Wine};
	fraud_add_unit(FraudHero_Tuzor, RaceId::Zerg, {8, UnitId::ZergZethrokor});
	fraud_add_unit(FraudHero_Tuzor, RaceId::Zerg, {12, UnitId::ZergQuazilisk});
	fraud_add_unit(FraudHero_Tuzor, RaceId::Zerg, {2, UnitId::ZergGeszithalor});

	// Charlie
	threshold.frauds[FraudHero_Charlie][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_None, AiBuildOrder_ZergAssault_3HatchHydrok, PlayerColor::MidnightGreen};
	fraud_add_unit(FraudHero_Charlie, RaceId::Zerg, {12, UnitId::ZergHydralisk});
	fraud_add_unit(FraudHero_Charlie, RaceId::Zerg, {4, UnitId::ZergBactalisk});
	fraud_add_unit(FraudHero_Charlie, RaceId::Zerg, {2, UnitId::ZergProtathalor});

	// Rekt'Son
	threshold.frauds[FraudHero_RektSon][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_None, AiBuildOrder_ZergGeneral_BeforeTheStorm, PlayerColor::IridescentViolet};
	fraud_add_unit(FraudHero_RektSon, RaceId::Zerg, {10, UnitId::ZergHydralisk});
	fraud_add_unit(FraudHero_RektSon, RaceId::Zerg, {6, UnitId::ZergLakizilisk});
	fraud_add_unit(FraudHero_RektSon, RaceId::Zerg, {1, UnitId::ZergUltrokor});

	//// Supers

	// Ackmed
	threshold.frauds[FraudSuper_Ackmed][RaceId::Terran] = {AiPersonality_TerranRaider, AiBuildType_None, AiBuildOrder_TerranRaider_HolyAckmeds, PlayerColor::BurntUmber};
	fraud_add_unit(FraudSuper_Ackmed, RaceId::Terran, {24, UnitId::TerranHarakan});
	fraud_add_unit(FraudSuper_Ackmed, RaceId::Terran, {8, UnitId::TerranHeracles});
	fraud_add_unit(FraudSuper_Ackmed, RaceId::Terran, {6, UnitId::TerranSalamander});
	fraud_add_unit(FraudSuper_Ackmed, RaceId::Terran, {2, UnitId::TerranSeraph});

	// The Don
	threshold.frauds[FraudSuper_TheDon][RaceId::Terran] = {AiPersonality_TerranOrbital, AiBuildType_None, AiBuildOrder_TerranOrbital_TheDon, PlayerColor::Vermillion};
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranSeraph});
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {9, UnitId::TerranCentaur});
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {6, UnitId::TerranMinotaur});
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranMagnetar});
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {3, UnitId::TerranPhobos});
	fraud_add_unit(FraudSuper_TheDon, RaceId::Terran, {2, UnitId::TerranAzazel});

	// Tolrim
	threshold.frauds[FraudSuper_Tolrim][RaceId::Terran] = {AiPersonality_TerranArmor, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::Redwood};
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranEidolon});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {2, UnitId::TerranAutocrat});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {6, UnitId::TerranMadrigal});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranPaladin});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {6, UnitId::TerranPazuzu});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {4, UnitId::TerranClaymore});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {3, UnitId::TerranPenumbra});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {2, UnitId::TerranAzazel});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {1, UnitId::TerranSeraph});
	fraud_add_unit(FraudSuper_Tolrim, RaceId::Terran, {1, UnitId::TerranMagnetar});

	// Tetragram
	threshold.frauds[FraudSuper_Tetragram][RaceId::Protoss] = {AiPersonality_ProtossRanger, AiBuildType_None, AiBuildOrder_ProtossRanger_LatticeExpo, PlayerColor::YinmnBlue};
	fraud_add_unit(FraudSuper_Tetragram, RaceId::Protoss, {16, UnitId::ProtossSimulacrum});
	fraud_add_unit(FraudSuper_Tetragram, RaceId::Protoss, {2, UnitId::ProtossClarion});
	fraud_add_unit(FraudSuper_Tetragram, RaceId::Protoss, {2, UnitId::ProtossDemiurge});

	// Amon
	threshold.frauds[FraudSuper_Amon][RaceId::Protoss] = {AiPersonality_ProtossSkylord, AiBuildType_Rush, AiBuildOrder_ProtossSkylord_SolarEscort, PlayerColor::Scarlet};
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {8, UnitId::ProtossCabalist});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {20, UnitId::ProtossBarghest});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {6, UnitId::ProtossArchon});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {3, UnitId::ProtossMindTyrant});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {3, UnitId::ProtossPariah});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {6, UnitId::ProtossEmpress});
	fraud_add_unit(FraudSuper_Amon, RaceId::Protoss, {1, UnitId::ProtossAnthelion});

	// The Great Devourer
	threshold.frauds[FraudSuper_TheGreatDevourer][RaceId::Protoss] = {AiPersonality_ProtossGeneral, AiBuildType_Timing, AiBuildOrder_None, PlayerColor::DelftBlue};
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossIdol});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossGolem});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {8, UnitId::ProtossAtreus});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {6, UnitId::ProtossAccantor});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {4, UnitId::ProtossArchitect});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {3, UnitId::ProtossClarion});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {3, UnitId::ProtossMagister});
	fraud_add_unit(FraudSuper_TheGreatDevourer, RaceId::Protoss, {1, UnitId::ProtossDidact});

	// Zerghere
	threshold.frauds[FraudSuper_Zerghere][RaceId::Zerg] = {AiPersonality_ZergSwarm, AiBuildType_None, AiBuildOrder_ZergSwarm_GatesOfHell, PlayerColor::Auburn};
	fraud_add_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergZethrokor});
	fraud_add_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergQuazilisk});
	fraud_add_unit(FraudSuper_Zerghere, RaceId::Zerg, {20, UnitId::ZergNathrokor});

	// Connor
	threshold.frauds[FraudSuper_Connor][RaceId::Zerg] = {AiPersonality_ZergAssault, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::Beige};
	fraud_add_unit(FraudSuper_Connor, RaceId::Zerg, {20, UnitId::ZergHydralisk});
	fraud_add_unit(FraudSuper_Connor, RaceId::Zerg, {15, UnitId::ZergBactalisk});
	fraud_add_unit(FraudSuper_Connor, RaceId::Zerg, {12, UnitId::ZergAlmaksalisk});
	fraud_add_unit(FraudSuper_Connor, RaceId::Zerg, {10, UnitId::ZergProtathalor});

	// Kagg
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg] = {AiPersonality_ZergGeneral, AiBuildType_Rush, AiBuildOrder_None, PlayerColor::SmokyBlack};
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergVorvrokor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergBactalisk] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergGeszithalor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergUltrokor] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergIsthrathaleth] = 1;
	threshold.frauds[FraudSuper_Kagg][RaceId::Zerg].deaths[UnitId::ZergAlkajelisk] = 1;
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {20, UnitId::ZergZethrokor});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergVorvrokor});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergHydralisk});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {8, UnitId::ZergBactalisk});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {5, UnitId::ZergZoryusthaleth});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {10, UnitId::ZergUltrokor});
	fraud_add_unit(FraudSuper_Kagg, RaceId::Zerg, {5, UnitId::ZergGeszithalor});

	//// God - TBD

	// Testing
	for (u32 i = 0; i < 6; ++i) {
		threshold.frauds[FraudGod_Hand1 + i][RaceId::Terran] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {8, UnitId::TerranOlympian});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {4, UnitId::TerranApostle});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {2, UnitId::TerranAutocrat});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {3, UnitId::TerranPaladin});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {3, UnitId::TerranCentaur});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {2, UnitId::TerranMinotaur});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {1, UnitId::TerranPhobos});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Terran, {1, UnitId::TerranMagnetar});

		threshold.frauds[FraudGod_Hand1 + i][RaceId::Protoss] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Protoss, {6, UnitId::ProtossArchon});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Protoss, {4, UnitId::ProtossAtreus});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Protoss, {1, UnitId::ProtossDemiurge});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Protoss, {1, UnitId::ProtossStarSovereign});

		threshold.frauds[FraudGod_Hand1 + i][RaceId::Zerg] = {AiPersonality_None, AiBuildType_None, AiBuildOrder_None, PlayerColor::Blood};
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {10, UnitId::ZergHydralisk});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {5, UnitId::ZergAlmaksalisk});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {4, UnitId::ZergKonvilisk});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {4, UnitId::ZergUltrokor});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergIsthrathaleth});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergAlkajelisk});
		fraud_add_unit(FraudGod_Hand1 + i, RaceId::Zerg, {2, UnitId::ZergKeskathalor});
	}
}