#pragma once
#include "BW/bwdata.h"
#include "BW/api.h"

u32  captivating_claws_get_stacks(const Unit* target);
u32  captivating_claws_get_timer(const Unit* target);
void captivating_claws_apply(Unit* source, Unit* target);