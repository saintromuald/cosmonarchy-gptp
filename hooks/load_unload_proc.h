#include <SCBW/structures.h>

namespace hooks
{
	void load_unit_proc(Unit* unit, Unit* unitToLoad);					// 004E78E0
	b32 unload_unit_proc(Unit* unit);									// 004E7F70

	// Transport extender
	char update_unit_transport_data(Unit* cargo, Unit* unused);			// 00401EA0
	void kill_all_loaded_units(Unit* transport);						// 0049FDD0
	bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args);	// 004C7E20
	Unit* get_loaded_unit_from_index(Unit* transport, u32 index);		// 004E6C40
	Unit* get_first_loaded_unit(Unit* transport);						// 004E6C90
	Unit* get_loaded_unit(Unit* transport, u32 index);					// Moved from Unit.cpp
	Unit* get_unit_from_index(u16 index);								// Moved from Unit.cpp
	u32 iterate_loaded_units(Unit* transport, void* func, Unit* cargo);	// 004E6D00
	s32 get_first_loaded_worker(Unit* transport);						// 004E6D90
	s32 is_carrying_flag(Unit* unit);									// 004F3A80
	s32 is_unit_loaded(Unit* transport);								// 004E7110
	u32 get_loaded_space_amount(Unit* transport);						// 004E7170
	void unload_unit_ext(Unit* cargo, u32 transport_died);				// 004E7E10
	u32 set_under_disruption_field(Unit* unit);							// 004EB170

	void inject_load_unload_proc_hooks();
}