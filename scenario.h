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

u32 get_transmission_playing();
bool is_map(u32 campaign, u32 map);

void init_scenario();
void end_scenario();
void tick_scenario();