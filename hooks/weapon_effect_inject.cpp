#include "weapon_effect.h"
#include "weapons\weapon_spellhit.h"
#include "../hook_tools.h"
#include "BW/enumerations.h"

const u32 Addr_WeaponsDatEffectCount = 0x0048B785 + 2;
const u32 Addr_WeaponsDatEffectSwitch = 0x0048B790 + 3;

namespace {
	void __declspec(naked) friendly_splash_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		if (weapon_id == WeaponId::LakiziliskSubterraneanSpines) {
			hooks::lakizilisk_splash(bullet, weapon_id, SplashType::FriendlyNew);
		} else {
			hooks::splash(bullet, weapon_id, SplashType::FriendlyNew);
		}
		// Everything is conveniently already pushed!
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) enemy_splash_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		if (weapon_id == WeaponId::LakiziliskSubterraneanSpines) {
			hooks::lakizilisk_splash(bullet, weapon_id, SplashType::Enemy);
		} else {
			hooks::splash(bullet, weapon_id, SplashType::Enemy);
		}
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) radial_splash_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		if (weapon_id == WeaponId::LakiziliskSubterraneanSpines) {
			hooks::lakizilisk_splash(bullet, weapon_id, SplashType::Radial);
		} else {
			hooks::splash(bullet, weapon_id, SplashType::Radial);
		}
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) normal_damage_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::normal_damage(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) yamato_damage_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::yamato_damage(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) parasite_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::parasite(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) broodspawn_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::broodlings(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) lockdown_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::lockdown(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) air_splash_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::air_splash(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) irradiate_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::irradiate_hit(bullet->source_unit, bullet->attack_target.pt, bullet->player);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) nuke_wrapper() {
		static Bullet* bullet;
		static u32 weapon_id;
		__asm {
			MOV bullet, EBX
			MOV weapon_id, ECX
		}
		hooks::nuke(bullet, weapon_id);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) splash_target_check_wrapper() { // 0048B570
		static Bullet* bullet;
		static Unit* target;
		static b32 result;
		__asm {
			MOV target, ESI
			MOV bullet, EDI
			PUSHAD
		}
		result = hooks::splash_target_check(bullet, target);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	//	(void*)0x0048B797,	// 1 - Normal
	//	(void*)0x0048B80B,	// 15 - Yamato Gun
	//	(void*)0x0048B8B9,	// 2 - Splash (Radial)
	//	(void*)0x0048B8B9,	// 3 - Splash (Enemy)
	//	(void*)0x0048B910	// 4 - Lockdown
	//	(void*)0x0048B969,	// 6 - MatralethParasite
	//	(void*)0x0048B9C8,	// 7 - Broodlings

	const void* WeaponEffects_SwitchTable[] = {
		(void*)0x0048BBF6,			//  0 - None
		normal_damage_wrapper,		//  1 - Normal
		radial_splash_wrapper,		//  2 - Splash (Radial)
		enemy_splash_wrapper,		//  3 - Splash (Enemy)
		lockdown_wrapper,			//  4 - Lockdown
		nuke_wrapper,				//  5 - Nuclear Missile
		parasite_wrapper,			//  6 - MatralethParasite
		broodspawn_wrapper,			//  7 - Broodlings
		(void*)0x0048BA0F,			//  8 - EMP Shockwave
		irradiate_wrapper,			//  9 - Irradiate
		(void*)0x0048BAAB,			// 10 - Ensnare
		(void*)0x0048BAC8,			// 11 - Plague
		(void*)0x0048B93D,			// 12 - Stasis Field
		(void*)0x0048BAE2,			// 13 - Dark Swarm
		(void*)0x0048BB00,			// 14 - Consume
		yamato_damage_wrapper,		// 15 - Yamato Gun
		(void*)0x0048BA2C,			// 16 - Restoration
		(void*)0x0048BB35,			// 17 - Disruption Web
		(void*)0x0048BB53,			// 18 - Corrosive Acid
		(void*)0x0048BBE9,			// 19 - Mind Control -- unused
		(void*)0x0048BBE9,			// 20 - Feedback -- unused
		(void*)0x0048BBC2,			// 21 - Optical Flare
		(void*)0x0048BB99,			// 22 - Maelstrom
		friendly_splash_wrapper,	// 23 - Unused
		(void*)0x0048B828,			// 24 - Splash (Air)
//		air_splash_wrapper,			// 24 - Splash (Air)
		// Add more !
	};
}

namespace hooks {
	void inject_weapon_effect_hooks() {
		u8 max_id = ARRAY_SIZE(WeaponEffects_SwitchTable) - 1;
		u8 data[4];
		*(u32*)data = (u32)WeaponEffects_SwitchTable;
		memory_patch(Addr_WeaponsDatEffectCount, max_id);
		memory_patch(Addr_WeaponsDatEffectSwitch, data, 4); // load pointer to address table
		
		// Remove nuclear missile damage rule (use weapon damage instead)
		nop_patch(0x0048ACDF, 2);

		jump_patch(splash_target_check_wrapper, 0x0048B570, 0);
	}
}
