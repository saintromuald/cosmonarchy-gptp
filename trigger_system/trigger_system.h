#pragma once
#include "SCBW/api.h"

struct Sound;

extern bool trigger_game_paused;
extern bool trigger_system_init;
extern bool trigger_system_in_game;

#define SoundReplacementCap 1600
struct SoundReplacement {
	PlayerBitset players;
	UnitIdBitset unit_ids;
	bool any_animation;
	bool any_sound;
	bool replace;
	u32 animation;
	u32 old_sound_id;
	Sound* new_sound;
};

extern u32 sound_replacements_count;
extern SoundReplacement sound_replacements[SoundReplacementCap];

const char* race_names[] = {
	"Zerg",
	"Protoss",
	"Terran",
};

extern bool has_custom_player_name[PlayerLimit];
extern Token custom_unit_names[PlayerLimit][UnitId::Count];
extern u32 trigger_combat[ScoutingPlayerLimit][PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];
extern u32 trigger_kills[ScoutingPlayerLimit][PlayerLimit][UnitId::Count][PlayerLimit][UnitId::Count];
extern u32 trigger_deaths[ScoutingPlayerLimit][PlayerLimit][UnitId::Count];

void init_trigger_system();
void update_trigger_system(bool in_game);

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
