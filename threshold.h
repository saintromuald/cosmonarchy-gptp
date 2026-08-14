#pragma once
#include <BW/api.h>

enum class ThresholdCampaign {
	OneForTheFuture = 2,
	TheLostGods = 3,
	Veeq7 = 7,
};

enum class ThresholdMission {
	OFTF_WallStreetWar = 1,
	OFTF_TheLostHandOfGod,
	OFTF_AutisticYears,
	OFTF_PastaMasta,
	OFTF_ThreeHongKong,
	OFTF_SeventhCue,
	OFTF_FavoredFiction,
	OFTF_TheShitflingSinners,
	OFTF_TheCosmonarch,

	Veeq7_Fraudscendence1 = 1,
	Veeq7_Fraudscendence2,
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
	v2u16 positions[InitPositionsCap];
};

extern InitPositions sources_positions;
extern InitPositions rally_positions;

u32 get_transmission_playing();
bool is_map(u32 campaign, u32 map);

bool init_threshold_spawn(CHKUnit* unit, u32 flags);
void init_threshold();
void end_threshold();
void update_threshold();