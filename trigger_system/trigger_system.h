#pragma once
#include "SCBW/api.h"

extern bool trigger_game_paused;
extern bool trigger_system_init;
extern bool trigger_system_in_game;
extern bool has_custom_name[PlayerLimit];

void update_trigger_system(bool in_game);

b32 parse_comment_condition(TriggerAction* action);
b32 __fastcall parse_comment_action(TriggerAction* action);
