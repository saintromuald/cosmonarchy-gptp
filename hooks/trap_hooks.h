#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks
{
	bool unit_is_floor_trap(Unit* unit); // 0047B070
	bool unit_is_installation_asset(u16 id);
	s16 disable_trap(Unit* pUnit); // 0047B4B0
	void inject_trap_hooks();
}