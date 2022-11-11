#pragma once
#include <SCBW/api.h>

enum Campaign {
	Campaign_OneForTheFuture = 2,
	Campaign_TheLostGods = 3,
	Campaign_Veeq7 = 7,
};

enum Mission {
	Mission_OFTF_WallStreetWar = 1,
	Mission_OFTF_TheLostHandOfGod,
	Mission_OFTF_AutisticYears,
	Mission_OFTF_PastaMasta,
	Mission_OFTF_ThreeHongKong,
	Mission_OFTF_SeventhCue,
	Mission_OFTF_FavoredFiction,
	Mission_OFTF_TheShitflingSinners,
	Mission_OFTF_TheCosmonarch,

	Mission_Veeq7_Fraudscendence1 = 1,
	Mission_Veeq7_Fraudscendence2,
};

struct ThresholdConfig {
	bool enabled;
	bool init; // Internal
	bool fraud_allowed[PlayerLimit];
	bool fraud_enabled[PlayerLimit]; // Internal
	bool witness_enabled[PlayerLimit];
	u32 initial_count;
	u32 min_frauds;
	u32 total_frauds;
	u32 timer;
	u32 god_timer;
};

extern ThresholdConfig threshold_config;

u32 get_transmission_playing();
bool is_map(u32 campaign, u32 map);

void init_scenario();
void end_threshold();
void tick_threshold();