#pragma once

namespace hooks {
	s32 __fastcall setUnderDisruptionWeb(s32 a1);
	s32 updateDisruptionWebStatus();

	extern void injectTargetRelatedHooks();
}