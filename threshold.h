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
	bool fraud_allowed[PlayerLimit];
	bool witness_enabled[PlayerLimit];
	f32 auto_scale;
	u32 initial_count;
	u32 min_count;
	u32 total_count;
	u32 timer;
};

extern ThresholdConfig threshold_config;

#define InitPositionsCap UnitLimit
struct InitPositions {
	u32 count;
	Point16 positions[InitPositionsCap];
};

extern InitPositions sources_positions;
extern InitPositions rally_positions;

u32 get_transmission_playing();
bool is_map(u32 campaign, u32 map);

bool init_threshold_spawn(CHKUnit* unit, u32 flags);
void init_threshold();
void end_threshold();
void tick_threshold();