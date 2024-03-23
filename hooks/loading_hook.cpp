#include "loading_hook.h"
#include <BW/api.h>
#include <BW/Storm.h>
#include <cstdio>
#include "hooks/DatExtend/dat_load.h"
#include "hooks/grafting/grafting.h"

namespace hooks {
	// Called the end of the data loading function, after GRPs and such are loaded
	void loading_data() {
		// Pre-load Units.dat to get UNIT_TYPE_COUNT
		load_game_data((DatLoad*)DatExt::LoadTable::Units_Dat, "arr\\units.dat");

		// Load graft patch files -- can be called in gameOn to re-load data if it gets modified in-game
		load_req_grafts();
		load_button_grafts();
	}
}