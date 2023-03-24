#pragma once
#include "SCBW/api.h"
#include "trigger_parser.h"

struct Sound;

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

#define SpriteReplacementCap 800
struct SpriteReplacement {
	PlayerBitset players;
	UnitIdBitset unit_ids;
	u32 sprite_id;
};

static const char* race_names[] = {
	"Zerg",
	"Protoss",
	"Terran",
};

extern bool trigger_game_paused;
extern bool trigger_system_init;
extern bool trigger_system_in_game;

extern u32 sound_replacements_count;
extern SoundReplacement sound_replacements[SoundReplacementCap];

extern u32 sprite_replacements_count;
extern SpriteReplacement sprite_replacements[SpriteReplacementCap];

extern Token custom_unit_names[PlayerLimit][UnitId::Count];

struct PlayerVars {
	bool is_witness;
};

enum Celebration {
	Celebration_None,
	Celebration_Christmas,

	Celebration_Count,
};

struct GlobalVars {
	PlayerVars players[PlayerLimit];
	u32 celebration;
};

extern GlobalVars global_vars;

void init_trigger_system();
void update_trigger_system(bool in_game);

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
