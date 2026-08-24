#include "weapon_effect.h"
#include "weapons\weapon_spellhit.h"
#include "../hook_tools.h"
#include "BW/enumerations.h"

const u32 Addr_WeaponsDatEffectCount = 0x0048B785 + 2;
const u32 Addr_WeaponsDatEffectSwitch = 0x0048B790 + 3;

namespace {
    void __declspec(naked) friendly_splash_wrapper() {
        static Bullet* bullet;
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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
        static WeaponId::Enum weapon_id;
        __asm {
            MOV bullet, EBX
            MOV weapon_id, CX
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

    void __declspec(naked) air_splash_proc_wrapper() { // 0048B150
        static Unit* target;
        static Bullet* bullet;
        static b32 result;
        __asm {
            MOV target, ECX
            MOV bullet, EDX
            PUSHAD
        }
        result = hooks::air_splash_proc(target, bullet);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) lakiz_splash_proc_wrapper() { // 0048B680
        static Unit* target;
        static Bullet* bullet;
        static b32 result;
        __asm {
            MOV target, ECX
            MOV bullet, EDX
            PUSHAD
        }
        result = hooks::lakiz_splash_proc(target, bullet);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    //	(void*)0x0048B797,	// 1 - Normal
    //	(void*)0x0048B80B,	// 15 - Yamato Gun
    //	(void*)0x0048B8B9,	// 2 - Splash (Radial)
    //	(void*)0x0048B8B9,	// 3 - Splash (Enemy)
    //	(void*)0x0048B910	// 4 - LobotomyGrenade
    //	(void*)0x0048B969,	// 6 - MatralethParasite
    //	(void*)0x0048B9C8,	// 7 - Broodlings

    const void* WeaponEffects_SwitchTable[] = {
        (void*)0x0048BBF6,			//  0 - None
        normal_damage_wrapper,		//  1 - Normal
        radial_splash_wrapper,		//  2 - Splash (Radial)
        enemy_splash_wrapper,		//  3 - Splash (Enemy)
        lockdown_wrapper,			//  4 - LobotomyGrenade
        nuke_wrapper,				//  5 - Nuclear Missile
        parasite_wrapper,			//  6 - MatralethParasite
        broodspawn_wrapper,			//  7 - Broodlings
        (void*)0x0048BA0F,			//  8 - EMP Shockwave       | emp_shockwave_hit in weapon_spellhit
        irradiate_wrapper,			//  9 - Irradiate
        (void*)0x0048BAAB,			// 10 - Ensnare             | ensnare_hit in weapon_spellhit
        (void*)0x0048BAC8,			// 11 - Plague              | plague_hit in weapon_spellhit
        (void*)0x0048B93D,			// 12 - Stasis Field        | stasis_field_hit in weapon_spellhit
        (void*)0x0048BAE2,			// 13 - Dark Swarm          | carapace_swarm_hit in weapon_spellhit
        (void*)0x0048BB00,			// 14 - Consume             | consume_hit in weapon_spellhit
        yamato_damage_wrapper,		// 15 - Yamato Gun
        (void*)0x0048BA2C,			// 16 - Restoration         | restore_hit in weapon_spellhit
        (void*)0x0048BB35,			// 17 - Disruption Web      | disruption_field_hit in weapon_spellhit
        (void*)0x0048BB53,			// 18 - Corrosive Acid      | corrosive_acid_hit in weapon_spellhit
        (void*)0x0048BBE9,			// 19 - Mind Control -- unused
        (void*)0x0048BBE9,			// 20 - Feedback -- unused
        (void*)0x0048BBC2,			// 21 - Optical Flare       | optical_flare_hit in weapon_spellhit
        (void*)0x0048BB99,			// 22 - Maelstrom           | maelstrom_hit in weapon_spellhit
        friendly_splash_wrapper,	// 23 - Unused
		air_splash_wrapper,			// 24 - Splash (Air)
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

        jump_patch(splash_target_check_wrapper,		0x0048B570, 0);
        jump_patch(air_splash_proc_wrapper,	0x0048B150, 0);
        jump_patch(lakiz_splash_proc_wrapper,	0x0048B680, 0);
    }
}
