#pragma once
#include <SCBW/api.h>
#include <DatExtend/datext.h>

namespace hooks
{
	void load_game_data(DatLoad* datTable, const char* filename);

	void inject_dat_load_hooks();
}