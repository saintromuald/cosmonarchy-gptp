#include <SCBW/structures/Unit.h>

namespace hooks {
	u16 getTechUseErrorMessageHook(Unit* target, u8 castingPlayer, u16 techId); // 00491E80
	
	void injectTechTargetCheckHooks();
}