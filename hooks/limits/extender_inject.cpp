#include "hooks/limits/extender.h"
#include "hooks/limits/ccmu.h"
#include "hooks/limits/sfx_extender.h"
namespace hooks {
	void injectLimitHooks() {
		//hooks::injectCCMUHooks();
		hooks::sfxExtender();

	}
}
