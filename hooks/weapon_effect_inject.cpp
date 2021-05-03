#include "weapon_effect.h"
#include "weapons\wpnspellhit.h"
#include "../hook_tools.h"
#include "SCBW/enumerations.h"

const u32 Addr_WeaponsDatEffectCount = 0x0048B785 + 2;
const u32 Addr_WeaponsDatEffectSwitch = 0x0048B790 + 3;

namespace
{

	void __declspec(naked) friendlySplashWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
			;//MOV effect, EAX -- or weapons_dat::ExplosionType[weaponId]
		}
		if (weaponId == WeaponId::SubterraneanSpines)
		{
			hooks::lurkerSplash(bullet, weaponId, SplashType::FriendlyNew);
		}
		else
		{
			hooks::splash(bullet, weaponId, SplashType::FriendlyNew);
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

	void __declspec(naked) enemySplashWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		if (weaponId == WeaponId::SubterraneanSpines)
		{
			hooks::lurkerSplash(bullet, weaponId, SplashType::Enemy);
		}
		else
		{
			hooks::splash(bullet, weaponId, SplashType::Enemy);
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

	void __declspec(naked) radialSplashWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		if (weaponId == WeaponId::SubterraneanSpines)
		{
			hooks::lurkerSplash(bullet, weaponId, SplashType::Radial);
		}
		else
		{
			hooks::splash(bullet, weaponId, SplashType::Radial);
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

	void __declspec(naked) normalWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::normalDamage(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) yamatoWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::yamatoDamage(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) parasiteWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::parasite(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	void __declspec(naked) broodlingsWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::broodlings(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	void __declspec(naked) lockdownWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::lockdown(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
	void __declspec(naked) airSplashWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::airSplash(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) irradiateWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::IrradiateHit(bullet->sourceUnit, bullet->attackTarget.pt, bullet->srcPlayer);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) nukeWrapper()
	{
		static CBullet* bullet;
		static u32 weaponId;
		__asm {
			MOV bullet, EBX
			MOV weaponId, ECX
		}
		hooks::nuke(bullet, weaponId);
		__asm {
			POP EDI
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	//	(void*)0x0048B797,	// 1 - Normal
	//	(void*)0x0048B80B,	// 15 - Yamato Gun
	//	(void*)0x0048B8B9,	// 2 - Splash (Radial)
	//	(void*)0x0048B8B9,	// 3 - Splash (Enemy)
	//	(void*)0x0048B910	// 4 - Lockdown
	//	(void*)0x0048B969,	// 6 - Parasite
	//	(void*)0x0048B9C8,	// 7 - Broodlings

	const void* WeaponEffects_SwitchTable[] = {
	  (void*)0x0048BBF6,		//  0 - None
	  normalWrapper,			//  1 - Normal
	  radialSplashWrapper,		//  2 - Splash (Radial)
	  enemySplashWrapper,		//  3 - Splash (Enemy)
	  lockdownWrapper,			//  4 - Lockdown
	  nukeWrapper,				//  5 - Nuclear Missile
	  parasiteWrapper,			//  6 - Parasite
	  broodlingsWrapper,		//  7 - Broodlings
	  (void*)0x0048BA0F,		//  8 - EMP Shockwave
	  irradiateWrapper,			//  9 - Irradiate
	  (void*)0x0048BAAB,		// 10 - Ensnare
	  (void*)0x0048BAC8,		// 11 - Plague
	  (void*)0x0048B93D,		// 12 - Stasis Field
	  (void*)0x0048BAE2,		// 13 - Dark Swarm
	  (void*)0x0048BB00,		// 14 - Consume
	  yamatoWrapper,			// 15 - Yamato Gun
	  (void*)0x0048BA2C,		// 16 - Restoration
	  (void*)0x0048BB35,		// 17 - Disruption Web
	  (void*)0x0048BB53,		// 18 - Corrosive Acid
	  (void*)0x0048BBE9,		// 19 - Mind Control -- unused
	  (void*)0x0048BBE9,		// 20 - Feedback -- unused
	  (void*)0x0048BBC2,		// 21 - Optical Flare
	  (void*)0x0048BB99,		// 22 - Maelstrom
	  friendlySplashWrapper,	// 23 - Unused
	  (void*)0x0048B828,		// 24 - Splash (Air)
   //   airSplashWrapper,		// 24 - Splash (Air)
	  // Add more !
	};

} //unnamed namespace

namespace hooks
{
	void injectWeaponEffectHook()
	{
		u8 maxID = ARRAY_SIZE(WeaponEffects_SwitchTable) - 1;
		u8 data[4];
		*(u32*)data = (u32)WeaponEffects_SwitchTable;
		memoryPatch(Addr_WeaponsDatEffectCount, maxID);
		memoryPatch(Addr_WeaponsDatEffectSwitch, data, 4); // load pointer to address table
	}
} //hooks
