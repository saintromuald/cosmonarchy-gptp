#include "dat_load.h"
#include <SCBW/api.h>
#include <hook_tools.h>

namespace {
	void __declspec(naked) load_game_data_wrapper()
	{
		static DatLoad* datTable;
		static char* filename;
		__asm
		{
			MOV datTable, EAX
			MOV ECX, [ESP+4]
			MOV filename, ECX
			PUSHAD
		}
		hooks::load_game_data(datTable, filename);
		__asm
		{
			POPAD
			RETN 0x04
		}
	}

	void __declspec(naked) get_unit_name_wrapper()
	{
		static u16 unitId;
		static char* retval;
		__asm
		{
			MOV unitId, CX
			PUSHAD
		}
		retval = (char*)Unit::get_name(unitId);
		__asm
		{
			POPAD
			MOV EAX, retval
			RETN
		}
	}
}

namespace hooks {

	void inject_dat_load_hooks() {
		jump_patch(load_game_data_wrapper, 0x004D2E80); // Custom .dat file loading function
		jump_patch(get_unit_name_wrapper, 0x0047B090); // Reroute get unit name to CUnit's getUnitName to allow for extended unit names
	}
}