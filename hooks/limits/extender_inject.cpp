#include "hooks/limits/extender.h"
#include "hooks/limits/ccmu.h"
#include "hooks/limits/sfx_extender.h"
#include "hooks/limits/tbl_extender.h"
#include "hooks/limits/misc_extenders.h"
#include "logger.h"
#include "globals.h"
#include <cassert>
namespace hooks
{
	void loadAiseSamaseData()
	{
		hooks::sfxExtenderGetCount();
	}

	u32 buildingVisionSyncHook()
	{
		s32 result = 0;
		u8* visHash = (u8*)0x0063fd30;
		u32* bWantVisibilityHashUpdate = (u32*)0x00629d90;
		u8* regionVisionHash = (u8*)0x0063fe38;
		s32* nCurrentHashOffset = (s32*)0x0063fd28;
		//
		memset(visHash, 0, 64);
		*bWantVisibilityHashUpdate = 0;
		*regionVisionHash = 0;
		*nCurrentHashOffset = 0;
		s32 i = 0;
		do
		{
			if (!(units_dat::base_property[i] & UnitProperty::Building))
			{
				if (units_dat::staredit_avail_flags[i] & 0x1)
				{
					auto flingy_id = fakeExtFlingyIds[i];
					auto sprite_id = flingy_dat::sprite_id[flingy_id];
					scbw::set_sprite_data(SpritesDatEntries::IncludeInVision, sprite_id, 1);
				}
			}
			i++;
		} while (i < 228);
		return i;
	}

	void __declspec(naked) buildingVisionSyncHook_Wrapper()
	{
		static u32 result;
		__asm {
			PUSHAD
		}
		result = buildingVisionSyncHook();
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void inject_limit_hooks()
	{
//		hooks::sfxExtender();
		newSfxExtender();
//		jump_patch(preInitDataWrapper, 0x004DAF31);
	}
}
