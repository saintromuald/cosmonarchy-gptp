#include "weapon_extender.h"
#include "hook_tools.h"

/*
u16*,		Label,				weaponsDat[0].address);  // 006572E0
u32*,		FlingyId,			weaponsDat[1].address);  // 00656CA8
u8*,		SpecialAttack,		weaponsDat[2].address);  // 006573E8 //"Special Attack" according to EUDDB
TargetFlag*,TargetFlags,		weaponsDat[3].address);  // 00657998
u32*,		MinRange,			weaponsDat[4].address);  // 00656A18
u32*,		MaxRange,			weaponsDat[5].address);  // 00657470
u8*,		DamageUpgrade,		weaponsDat[6].address);  // 006571D0
u8*,		DamageType,			weaponsDat[7].address);  // 00657258 Normal, Explosive, etc.. Use with DamageType::Enum.
u8*,		Behavior,			weaponsDat[8].address);  // 00656670 Fly and follow target, appear on target unit, etc. Use with WeaponBehavior::Enum.
u8*,		RemoveAfter,		weaponsDat[9].address);  // 00657040
u8*,		ExplosionType,		weaponsDat[10].address); // 006566F8 NormalHit, RadialSplash, EnemySplash, etc. Use with WeaponEffect::Enum.
u16*,		InnerSplashRadius,	weaponsDat[11].address); // 00656888
u16*,		MediumSplashRadius, weaponsDat[12].address); // 006570C8
u16*,		OuterSplashRadius,	weaponsDat[13].address); // 00657780
u16*,		DamageAmount,		weaponsDat[14].address); // 00656EB0
u16*,		DamageBonus,		weaponsDat[15].address); // 00657678
u8*,		Cooldown,			weaponsDat[16].address); // 00656FB8
u8*,		DamageFactor,		weaponsDat[17].address); // 006564E0
u8*,		AttackAngle,		weaponsDat[18].address); // 00656990
u8*,		LaunchSpin,			weaponsDat[19].address); // 00657888
u8*,		ForwardOffset,		weaponsDat[20].address); // 00657910
u8*,		VerticalOffset,		weaponsDat[21].address); // 00656C20
u16*,		ErrorMessage,		weaponsDat[22].address); // 00656568 //Target Error Message according to EUDDB
u16*,		Icon,				weaponsDat[23].address); // 00656780 //Icon according to EUDDB

patchWeaponDat(0, 0x00513868, PatchLabel);						  DATStruct <offset weaponsdat_label, 2, 130>
patchWeaponDat(0, 0x00513874, PatchFlingyId);					  DATStruct <offset weaponsdat_graphics, 4, 130> ; 4
patchWeaponDat(0, 0x00513880, PatchSpecialAttack);				  DATStruct <offset weaponsdat_technology, 1, 130>
patchWeaponDat(0, 0x0051388C, PatchTargetFlags);				  DATStruct <offset weaponsdat_targetFlags, 2, 130> ; 2
patchWeaponDat(0, 0x00513898, PatchMinRange);					  DATStruct <offset weaponsdat_minRange, 4, 130> ; 16
patchWeaponDat(0, 0x005138A4, PatchMaxRange);					  DATStruct <offset weaponsdat_maxRange, 4, 130> ; 15
patchWeaponDat(0, 0x005138B0, PatchDamageUpgrade);				  DATStruct <offset weaponsdat_damageUpgrade, 1, 130> ; 4
patchWeaponDat(0, 0x005138BC, PatchDamageType);					  DATStruct <offset weaponsdat_wpnType, 1, 130> ; 5
patchWeaponDat(0, 0x005138C8, PatchBehavior);					  DATStruct <offset gubWeapFireType, 1, 130> ; 6
patchWeaponDat(0, 0x005138D4, PatchRemoveAfter);				  DATStruct <offset weaponsdat_removeAfter, 1, 130> ; 1
patchWeaponDat(0, 0x005138E0, PatchExplosionType);				  DATStruct <offset gubWeapDamageType, 1, 130> ; 3
patchWeaponDat(0, 0x005138EC, PatchInnerSplashRadius);			  DATStruct <offset weaponsdat_innerSplash, 2, 130> ; 4
patchWeaponDat(0, 0x005138F8, PatchMediumSplashRadius);			  DATStruct <offset weaponsdat_middleSplash, 2, 130> ; 1
patchWeaponDat(0, 0x00513904, PatchOuterSplashRadius);			  DATStruct <offset weaponsdat_outerSplash, 2, 130> ; 4
patchWeaponDat(0, 0x00513910, PatchDamageAmount);				  DATStruct <offset weaponsdat_dmgAmount, 2, 130> ; 12
patchWeaponDat(0, 0x0051391C, PatchDamageBonus);				  DATStruct <offset weaponsdat_dmgBonus, 2, 130> ; 12
patchWeaponDat(0, 0x00513928, PatchCooldown);					  DATStruct <offset gnWeapCooldown, 1, 130> ; 2
patchWeaponDat(0, 0x00513934, PatchDamageFactor);				  DATStruct <offset weaponsdat_dmgFactor, 1, 130> ; 7
patchWeaponDat(0, 0x00513940, PatchAttackAngle);				  DATStruct <offset weaponsdat_AttackAngle, 1, 130> ; 4
patchWeaponDat(0, 0x0051394C, PatchLaunchSpin);					  DATStruct <offset weaponsdat_LaunchSpin, 1, 130> ; 1
patchWeaponDat(0, 0x00513958, PatchForwardOffset);				  DATStruct <offset weaponsdat_ForwardOffset, 1, 130> ; 2
patchWeaponDat(0, 0x00513964, PatchVerticalOffset);				  DATStruct <offset weaponsdat_upwardOffset, 1, 130> ; 1
patchWeaponDat(0, 0x00513970, PatchErrorMessage);				  DATStruct <offset weaponsdat_targetErrorStr, 2, 130> ; 1
patchWeaponDat(0, 0x0051397C, PatchIcon);						  DATStruct <offset weaponsdat_icon, 2, 130> ; 1
*/

namespace weapons_dat
{
	u16			Label[MAX_WEAPONS];			
	u32			FlingyId[MAX_WEAPONS];			
	u8			SpecialAttack[MAX_WEAPONS];	
	TargetFlag	TargetFlags[MAX_WEAPONS];		
	u32			MinRange[MAX_WEAPONS];			
	u32			MaxRange[MAX_WEAPONS];			
	u8			DamageUpgrade[MAX_WEAPONS];		
	u8			DamageType[MAX_WEAPONS];			
	u8			Behavior[MAX_WEAPONS];			
	u8			RemoveAfter[MAX_WEAPONS];		
	u8			ExplosionType[MAX_WEAPONS];		
	u16			InnerSplashRadius[MAX_WEAPONS];	
	u16			MediumSplashRadius[MAX_WEAPONS]; 
	u16			OuterSplashRadius[MAX_WEAPONS];	
	u16			DamageAmount[MAX_WEAPONS];		
	u16			DamageBonus[MAX_WEAPONS];		
	u8			Cooldown[MAX_WEAPONS];			
	u8			DamageFactor[MAX_WEAPONS];		
	u8			AttackAngle[MAX_WEAPONS];		
	u8			LaunchSpin[MAX_WEAPONS];			
	u8			ForwardOffset[MAX_WEAPONS];		
	u8			VerticalOffset[MAX_WEAPONS];		
	u16			ErrorMessage[MAX_WEAPONS];		
	u16			Icon[MAX_WEAPONS];
}

enum PatchType
{
	PatchLabel,				
	PatchFlingyId,			
	PatchSpecialAttack,		
	PatchTargetFlags,		
	PatchMinRange,			
	PatchMaxRange,			
	PatchDamageUpgrade,		
	PatchDamageType,			
	PatchBehavior,			
	PatchRemoveAfter,		
	PatchExplosionType,		
	PatchInnerSplashRadius,	
	PatchMediumSplashRadius, 
	PatchOuterSplashRadius,	
	PatchDamageAmount,		
	PatchDamageBonus,		
	PatchCooldown,			
	PatchDamageFactor,		
	PatchAttackAngle,		
	PatchLaunchSpin,			
	PatchForwardOffset,		
	PatchVerticalOffset,		
	PatchErrorMessage,		
	PatchIcon,

	PatchMaxWeaponCompare,
	PatchMaxWeaponLoad,
};

static inline void patchWeaponDat(u32 opcodeSize, u32 address, u32 patchType)
{
	const void* patches[] =
	{
		weapons_dat::Label,				
		weapons_dat::FlingyId,			
		weapons_dat::SpecialAttack,		
		weapons_dat::TargetFlags,		
		weapons_dat::MinRange,			
		weapons_dat::MaxRange,			
		weapons_dat::DamageUpgrade,		
		weapons_dat::DamageType,			
		weapons_dat::Behavior,			
		weapons_dat::RemoveAfter,		
		weapons_dat::ExplosionType,		
		weapons_dat::InnerSplashRadius,	
		weapons_dat::MediumSplashRadius, 
		weapons_dat::OuterSplashRadius,	
		weapons_dat::DamageAmount,		
		weapons_dat::DamageBonus,		
		weapons_dat::Cooldown,			
		weapons_dat::DamageFactor,		
		weapons_dat::AttackAngle,		
		weapons_dat::LaunchSpin,			
		weapons_dat::ForwardOffset,		
		weapons_dat::VerticalOffset,		
		weapons_dat::ErrorMessage,		
		weapons_dat::Icon,
	};

	if (patchType == PatchMaxWeaponCompare)
	{
		const u8 noneWeaponId = MAX_WEAPONS - 1;
		memoryPatch(address + opcodeSize, noneWeaponId);
	}
	else if (patchType == PatchMaxWeaponLoad)
	{
		const u32 maxWeapons = MAX_WEAPONS;
		memoryPatch(address + opcodeSize, maxWeapons);
	}
	else
	{
		memoryPatch(address + opcodeSize, patches[patchType]);
	}
}

namespace hooks
{
	void injectWeaponExtender()
	{
		// Load code

		// .data:00513868 E0 72 65 00 02 00 00 00 82 00 00 00 weapons_DATLoad DATStruct <offset weaponsdat_label, 2, 130>
		// .data:00513874 A8 6C 65 00 04 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_graphics, 4, 130> ; 4
		// .data:00513880 E8 73 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_technology, 1, 130>
		// .data:0051388C 98 79 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_targetFlags, 2, 130> ; 2
		// .data:00513898 18 6A 65 00 04 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_minRange, 4, 130> ; 16
		// .data:005138A4 70 74 65 00 04 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_maxRange, 4, 130> ; 15
		// .data:005138B0 D0 71 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_damageUpgrade, 1, 130> ; 4
		// .data:005138BC 58 72 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_wpnType, 1, 130> ; 5
		// .data:005138C8 70 66 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset gubWeapFireType, 1, 130> ; 6
		// .data:005138D4 40 70 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_removeAfter, 1, 130> ; 1
		// .data:005138E0 F8 66 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset gubWeapDamageType, 1, 130> ; 3
		// .data:005138EC 88 68 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_innerSplash, 2, 130> ; 4
		// .data:005138F8 C8 70 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_middleSplash, 2, 130> ; 1
		// .data:00513904 80 77 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_outerSplash, 2, 130> ; 4
		// .data:00513910 B0 6E 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_dmgAmount, 2, 130> ; 12
		// .data:0051391C 78 76 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_dmgBonus, 2, 130> ; 12
		// .data:00513928 B8 6F 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset gnWeapCooldown, 1, 130> ; 2
		// .data:00513934 E0 64 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_dmgFactor, 1, 130> ; 7
		// .data:00513940 90 69 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_AttackAngle, 1, 130> ; 4
		// .data:0051394C 88 78 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_LaunchSpin, 1, 130> ; 1
		// .data:00513958 10 79 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_ForwardOffset, 1, 130> ; 2
		// .data:00513964 20 6C 65 00 01 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_upwardOffset, 1, 130> ; 1
		// .data:00513970 68 65 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_targetErrorStr, 2, 130> ; 1
		// .data:0051397C 80 67 65 00 02 00 00 00 82 00 00 00                 DATStruct <offset weaponsdat_icon, 2, 130> ; 1
		patchWeaponDat(0, 0x00513868, PatchLabel);
		patchWeaponDat(0, 0x00513874, PatchFlingyId);
		patchWeaponDat(0, 0x00513880, PatchSpecialAttack);
		patchWeaponDat(0, 0x0051388C, PatchTargetFlags);
		patchWeaponDat(0, 0x00513898, PatchMinRange);
		patchWeaponDat(0, 0x005138A4, PatchMaxRange);
		patchWeaponDat(0, 0x005138B0, PatchDamageUpgrade);
		patchWeaponDat(0, 0x005138BC, PatchDamageType);
		patchWeaponDat(0, 0x005138C8, PatchBehavior);
		patchWeaponDat(0, 0x005138D4, PatchRemoveAfter);
		patchWeaponDat(0, 0x005138E0, PatchExplosionType);
		patchWeaponDat(0, 0x005138EC, PatchInnerSplashRadius);
		patchWeaponDat(0, 0x005138F8, PatchMediumSplashRadius);
		patchWeaponDat(0, 0x00513904, PatchOuterSplashRadius);
		patchWeaponDat(0, 0x00513910, PatchDamageAmount);
		patchWeaponDat(0, 0x0051391C, PatchDamageBonus);
		patchWeaponDat(0, 0x00513928, PatchCooldown);
		patchWeaponDat(0, 0x00513934, PatchDamageFactor);
		patchWeaponDat(0, 0x00513940, PatchAttackAngle);
		patchWeaponDat(0, 0x0051394C, PatchLaunchSpin);
		patchWeaponDat(0, 0x00513958, PatchForwardOffset);
		patchWeaponDat(0, 0x00513964, PatchVerticalOffset);
		patchWeaponDat(0, 0x00513970, PatchErrorMessage);
		patchWeaponDat(0, 0x0051397C, PatchIcon);

		patchWeaponDat(8, 0x00513868, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513874, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513880, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x0051388C, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513898, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138A4, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138B0, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138BC, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138C8, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138D4, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138E0, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138EC, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x005138F8, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513904, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513910, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x0051391C, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513928, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513934, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513940, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x0051394C, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513958, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513964, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x00513970, PatchMaxWeaponLoad);
		patchWeaponDat(8, 0x0051397C, PatchMaxWeaponLoad);

		// dword weapons_dat_flingy
		// 00475A33  |.  8B0C95 A86C65>MOV ECX,DWORD PTR DS:[EDX*4+656CA8]
		// 00479C38  |.  8B04BD A86C65>MOV EAX,DWORD PTR DS:[EDI*4+656CA8]
		// 00479D14  |>  8B14BD A86C65>MOV EDX,DWORD PTR DS:[EDI*4+656CA8]
		// 0048BED7  |.  8B0485 A86C65>MOV EAX,DWORD PTR DS:[EAX*4+656CA8]
		patchWeaponDat(3, 0x00475A33, PatchFlingyId);
		patchWeaponDat(3, 0x00479C38, PatchFlingyId);
		patchWeaponDat(3, 0x00479D14, PatchFlingyId);
		patchWeaponDat(3, 0x0048BED7, PatchFlingyId);

		// byte weapons_dat_death_time
		// 0048BF1B  |.  8A90 40706500 MOV DL,BYTE PTR DS:[EAX+657040]
		patchWeaponDat(2, 0x0048BF1B, PatchRemoveAfter);

		// byte weapons_dat_launch_spin
		// 0048BF52  |.  8A90 88786500 MOV DL,BYTE PTR DS:[EAX+657888]
		patchWeaponDat(2, 0x0048BF52, PatchLaunchSpin);
		
		// byte weapons_dat_behavior
		// 004418E5   .  80BA 70666500>CMP BYTE PTR DS:[EDX+656670],5
		// 0044190C   .  0FB680 706665>MOVZX EAX,BYTE PTR DS:[EAX+656670]
		// 00479C99  |.  8A87 70666500 MOV AL,BYTE PTR DS:[EDI+656670]
		// 0048B7AE  |.  80B9 70666500>CMP BYTE PTR DS:[ECX+656670],7
		// 0048BD67   .  8A80 70666500 MOV AL,BYTE PTR DS:[EAX+656670]
		// 0048C02C  |.  0FB689 706665>MOVZX ECX,BYTE PTR DS:[ECX+656670]
		patchWeaponDat(2, 0x004418E5, PatchBehavior);
		patchWeaponDat(3, 0x0044190C, PatchBehavior);
		patchWeaponDat(2, 0x00479C99, PatchBehavior);
		patchWeaponDat(2, 0x0048B7AE, PatchBehavior);
		patchWeaponDat(2, 0x0048BD67, PatchBehavior);
		patchWeaponDat(3, 0x0048C02C, PatchBehavior);

		// byte weapons_dat_effect
		// 0048ACD8  |.  80BF F8666500>CMP BYTE PTR DS:[EDI+6566F8],5
		// 0048B58C  |.  8A80 F8666500 MOV AL,BYTE PTR DS:[EAX+6566F8]
		// 0048B77E  |.  0FB681 F86665>MOVZX EAX,BYTE PTR DS:[ECX+6566F8]
		patchWeaponDat(2, 0x0048ACD8, PatchExplosionType);
		patchWeaponDat(2, 0x0048B58C, PatchExplosionType);
		patchWeaponDat(3, 0x0048B77E, PatchExplosionType);

		// word weapons_dat_outer_splash
		// 0048B181   .  0FB78B 807765>MOVZX ECX,WORD PTR DS:[EBX+657780]
		// 0048B5C0  |.  0FB7144D 8077>MOVZX EDX,WORD PTR DS:[ECX*2+657780]
		// 0048B828  |>  0FB7044D 8077>MOVZX EAX,WORD PTR DS:[ECX*2+657780]     ;  Case 18 of switch 0048B785
		// 0048B8CC  |.  0FB70445 8077>MOVZX EAX,WORD PTR DS:[EAX*2+657780]
		patchWeaponDat(3, 0x0048B181, PatchOuterSplashRadius);
		patchWeaponDat(4, 0x0048B5C0, PatchOuterSplashRadius);
		patchWeaponDat(4, 0x0048B828, PatchOuterSplashRadius);
		patchWeaponDat(4, 0x0048B8CC, PatchOuterSplashRadius);

		// word weapons_dat_middle_splash
		// 0048B647  |.  0FB70455 C870>MOVZX EAX,WORD PTR DS:[EDX*2+6570C8]
		patchWeaponDat(4, 0x0048B647, PatchMediumSplashRadius);

		// word weapons_dat_inner_splash
		// 00465EC6   .  0FB7044D 8868>MOVZX EAX,WORD PTR DS:[ECX*2+656888]
		// 0048B195   .  0FB793 886865>MOVZX EDX,WORD PTR DS:[EBX+656888]
		// 0048B5FF  |.  0FB70C45 8868>MOVZX ECX,WORD PTR DS:[EAX*2+656888]
		// 0048B6B3   .  0FB7144D 8868>MOVZX EDX,WORD PTR DS:[ECX*2+656888]
		patchWeaponDat(4, 0x00465EC6, PatchInnerSplashRadius);
		patchWeaponDat(3, 0x0048B195, PatchInnerSplashRadius);
		patchWeaponDat(4, 0x0048B5FF, PatchInnerSplashRadius);
		patchWeaponDat(4, 0x0048B6B3, PatchInnerSplashRadius);

		// word weapons_dat_flags
		// 00475CEE  |.  0FB60445 9879>MOVZX EAX,BYTE PTR DS:[EAX*2+657998]
		// 00475D19  |.  66:8B0445 987>MOV AX,WORD PTR DS:[EAX*2+657998]
		patchWeaponDat(4, 0x00475CEE, PatchTargetFlags);
		patchWeaponDat(4, 0x00475D19, PatchTargetFlags);

		// byte weapons_dat_upgrade
		// 0042579D  |.  66:0FB688 D07>MOVZX CX,BYTE PTR DS:[EAX+6571D0]
		// 00475E47  |.  66:0FB68A D07>MOVZX CX,BYTE PTR DS:[EDX+6571D0]
		// 00475EC7  |.  66:0FB68A D07>MOVZX CX,BYTE PTR DS:[EDX+6571D0]
		// 0048ACE5  |.  66:0FB688 D07>MOVZX CX,BYTE PTR DS:[EAX+6571D0]
		patchWeaponDat(4, 0x0042579D, PatchDamageUpgrade);
		patchWeaponDat(4, 0x00475E47, PatchDamageUpgrade);
		patchWeaponDat(4, 0x00475EC7, PatchDamageUpgrade);
		patchWeaponDat(4, 0x0048ACE5, PatchDamageUpgrade);

		// word weapons_dat_upgrade_bonus
		// 00475E9D  |.  66:8B0455 787>MOV AX,WORD PTR DS:[EDX*2+657678]
		// 00475EE4  |.  66:8B0455 787>MOV AX,WORD PTR DS:[EDX*2+657678]
		// 00475EFF  |.  66:8B0455 787>MOV AX,WORD PTR DS:[EDX*2+657678]
		// 0048AD1B  |>  0FB7047D 7876>MOVZX EAX,WORD PTR DS:[EDI*2+657678]
		// 004BF0F4  |.  BF 78766500   MOV EDI,00657678
		// 004BF0F9  |.  68 78766500   PUSH 00657678
		// 004BF28B  |.  68 78766500   PUSH 00657678
		// 004BF43D  |.  B8 78766500   MOV EAX,00657678
		// 004CAC6A  |.  66:899F 78766>|MOV WORD PTR DS:[EDI+657678],BX
		// 004CAC9B  |.  66:89BA 78766>|MOV WORD PTR DS:[EDX+657678],DI
		// 004CAE2A  |.  66:899F 78766>|MOV WORD PTR DS:[EDI+657678],BX
		// 004CAE5B  |.  66:89BA 78766>|MOV WORD PTR DS:[EDX+657678],DI
		patchWeaponDat(4, 0x00475E9D, PatchDamageBonus);
		patchWeaponDat(4, 0x00475EE4, PatchDamageBonus);
		patchWeaponDat(4, 0x00475EFF, PatchDamageBonus);
		patchWeaponDat(4, 0x0048AD1B, PatchDamageBonus);
		patchWeaponDat(1, 0x004BF0F4, PatchDamageBonus);
		patchWeaponDat(1, 0x004BF0F9, PatchDamageBonus);
		patchWeaponDat(1, 0x004BF28B, PatchDamageBonus);
		patchWeaponDat(1, 0x004BF43D, PatchDamageBonus);
		patchWeaponDat(3, 0x004CAC6A, PatchDamageBonus);
		patchWeaponDat(3, 0x004CAC9B, PatchDamageBonus);
		patchWeaponDat(3, 0x004CAE2A, PatchDamageBonus);
		patchWeaponDat(3, 0x004CAE5B, PatchDamageBonus);

		// byte weapons_dat_damage_type
		// 0047978E  |.  8A88 58726500 MOV CL,BYTE PTR DS:[EAX+657258]
		// 004799D2  |.  80B8 58726500>CMP BYTE PTR DS:[EAX+657258],4
		// 00479A05  |.  80B9 58726500>CMP BYTE PTR DS:[ECX+657258],4
		// 00479A43  |.  0FB681 587265>MOVZX EAX,BYTE PTR DS:[ECX+657258]
		// 00479A99  |.  8A8A 58726500 MOV CL,BYTE PTR DS:[EDX+657258]
		patchWeaponDat(2, 0x0047978E, PatchDamageType);
		patchWeaponDat(2, 0x004799D2, PatchDamageType);
		patchWeaponDat(2, 0x00479A05, PatchDamageType);
		patchWeaponDat(3, 0x00479A43, PatchDamageType);
		patchWeaponDat(2, 0x00479A99, PatchDamageType);

		// word weapons_dat_error_msg
		// 0046F02B  |.  66:8B044D 686>MOV AX,WORD PTR DS:[ECX*2+656568]
		patchWeaponDat(4, 0x0046F02B, PatchErrorMessage);

		// byte weapons_dat_factor
		// 0043120A   .  0FB691 E06465>MOVZX EDX,BYTE PTR DS:[ECX+6564E0]
		// 004312A8  |.  0FB698 E06465>MOVZX EBX,BYTE PTR DS:[EAX+6564E0]
		// 00475E50  |.  8A9A E0646500 MOV BL,BYTE PTR DS:[EDX+6564E0]
		// 00475F1B  |.  8A91 E0646500 MOV DL,BYTE PTR DS:[ECX+6564E0]
		// 00479C65  |.  80BF E0646500>CMP BYTE PTR DS:[EDI+6564E0],2
		// 00479D4C  |.  80B8 E0646500>CMP BYTE PTR DS:[EAX+6564E0],2
		// 00479DB0  |.  80BA E0646500>CMP BYTE PTR DS:[EDX+6564E0],2
		// 004D80ED  |.  80B8 E0646500>|CMP BYTE PTR DS:[EAX+6564E0],2
		patchWeaponDat(3, 0x0043120A, PatchDamageFactor);
		patchWeaponDat(3, 0x004312A8, PatchDamageFactor);
		patchWeaponDat(2, 0x00475E50, PatchDamageFactor);
		patchWeaponDat(2, 0x00475F1B, PatchDamageFactor);
		patchWeaponDat(2, 0x00479C65, PatchDamageFactor);
		patchWeaponDat(2, 0x00479D4C, PatchDamageFactor);
		patchWeaponDat(2, 0x00479DB0, PatchDamageFactor);
		patchWeaponDat(2, 0x004D80ED, PatchDamageFactor);

		// byte weapons_dat_cooldown
		// 00431219   .  0FB689 B86F65>MOVZX ECX,BYTE PTR DS:[ECX+656FB8]
		// 004312B0  |.  0FB6B0 B86F65>MOVZX ESI,BYTE PTR DS:[EAX+656FB8]
		// 00475DCC  |.  0FB680 B86F65>MOVZX EAX,BYTE PTR DS:[EAX+656FB8]
		patchWeaponDat(3, 0x00431219, PatchCooldown);
		patchWeaponDat(3, 0x004312B0, PatchCooldown);
		patchWeaponDat(3, 0x00475DCC, PatchCooldown);

		// word weapons_dat_damage
		// 00431211   .  0FB7044D B06E>MOVZX EAX,WORD PTR DS:[ECX*2+656EB0]
		// 004312B8  |.  0FB73C45 B06E>MOVZX EDI,WORD PTR DS:[EAX*2+656EB0]
		// 00475F41  |.  66:8B044D B06>MOV AX,WORD PTR DS:[ECX*2+656EB0]
		// 00479BB3  |.  0FB7144D B06E>MOVZX EDX,WORD PTR DS:[ECX*2+656EB0]
		// 0048ACF1  |.  0FB73445 B06E>MOVZX ESI,WORD PTR DS:[EAX*2+656EB0]
		// 004BF0CE  |.  BF B06E6500   MOV EDI,00656EB0
		// 004BF0D3  |.  68 B06E6500   PUSH 00656EB0
		// 004BF277  |.  68 B06E6500   PUSH 00656EB0
		// 004BF425  |.  B8 B06E6500   MOV EAX,00656EB0
		// 004CAC5B  |.  66:899F B06E6>|MOV WORD PTR DS:[EDI+656EB0],BX
		// 004CAC8C  |.  66:89BA B06E6>|MOV WORD PTR DS:[EDX+656EB0],DI
		// 004CAE1B  |.  66:899F B06E6>|MOV WORD PTR DS:[EDI+656EB0],BX
		// 004CAE4C  |.  66:89BA B06E6>|MOV WORD PTR DS:[EDX+656EB0],DI
		patchWeaponDat(4, 0x00431211, PatchDamageAmount);
		patchWeaponDat(4, 0x004312B8, PatchDamageAmount);
		patchWeaponDat(4, 0x00475F41, PatchDamageAmount);
		patchWeaponDat(4, 0x00479BB3, PatchDamageAmount);
		patchWeaponDat(4, 0x0048ACF1, PatchDamageAmount);
		patchWeaponDat(1, 0x004BF0CE, PatchDamageAmount);
		patchWeaponDat(1, 0x004BF0D3, PatchDamageAmount);
		patchWeaponDat(1, 0x004BF277, PatchDamageAmount);
		patchWeaponDat(1, 0x004BF425, PatchDamageAmount);
		patchWeaponDat(3, 0x004CAC5B, PatchDamageAmount);
		patchWeaponDat(3, 0x004CAC8C, PatchDamageAmount);
		patchWeaponDat(3, 0x004CAE1B, PatchDamageAmount);
		patchWeaponDat(3, 0x004CAE4C, PatchDamageAmount);

		// byte weapons_dat_attack_angle
		// 004675E0  |.  0FB681 906965>MOVZX EAX,BYTE PTR DS:[ECX+656990]
		// 00467E3F  |.  0FB688 906965>MOVZX ECX,BYTE PTR DS:[EAX+656990]
		// 00475C67  |.  0FB68A 906965>MOVZX ECX,BYTE PTR DS:[EDX+656990]
		// 00478C73  |.  0FB682 906965>MOVZX EAX,BYTE PTR DS:[EDX+656990]
		patchWeaponDat(3, 0x004675E0, PatchAttackAngle);
		patchWeaponDat(3, 0x00467E3F, PatchAttackAngle);
		patchWeaponDat(3, 0x00475C67, PatchAttackAngle);
		patchWeaponDat(3, 0x00478C73, PatchAttackAngle);

		// byte weapons_dat_x_offset
		// 00479CDD  |.  0FB697 107965>MOVZX EDX,BYTE PTR DS:[EDI+657910]
		// 004D819A  |.  8890 10796500 |MOV BYTE PTR DS:[EAX+657910],DL
		patchWeaponDat(3, 0x00479CDD, PatchForwardOffset);
		patchWeaponDat(2, 0x004D819A, PatchForwardOffset);

		// byte weapons_dat_y_offset
		// 00479CF1  |.  0FB697 206C65>MOVZX EDX,BYTE PTR DS:[EDI+656C20]
		patchWeaponDat(3, 0x00479CF1, PatchVerticalOffset);

		// dword weapons_dat_max_range
		// 004312C0  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 004758B5  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 004758D6  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 004758F7  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 00475910  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 00475945  |.  8B0485 707465>MOV EAX,DWORD PTR DS:[EAX*4+657470]
		// 0048C16B  |.  8B0C85 707465>MOV ECX,DWORD PTR DS:[EAX*4+657470]
		// 0049E059   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E078   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E0B6   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E0D5   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E115   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E134   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E172   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		// 0049E191   .  8B0C8D 707465>MOV ECX,DWORD PTR DS:[ECX*4+657470]
		patchWeaponDat(3, 0x004312C0, PatchMaxRange);
		patchWeaponDat(3, 0x004758B5, PatchMaxRange);
		patchWeaponDat(3, 0x004758D6, PatchMaxRange);
		patchWeaponDat(3, 0x004758F7, PatchMaxRange);
		patchWeaponDat(3, 0x00475910, PatchMaxRange);
		patchWeaponDat(3, 0x00475945, PatchMaxRange);
		patchWeaponDat(3, 0x0048C16B, PatchMaxRange);
		patchWeaponDat(3, 0x0049E059, PatchMaxRange);
		patchWeaponDat(3, 0x0049E078, PatchMaxRange);
		patchWeaponDat(3, 0x0049E0B6, PatchMaxRange);
		patchWeaponDat(3, 0x0049E0D5, PatchMaxRange);
		patchWeaponDat(3, 0x0049E115, PatchMaxRange);
		patchWeaponDat(3, 0x0049E134, PatchMaxRange);
		patchWeaponDat(3, 0x0049E172, PatchMaxRange);
		patchWeaponDat(3, 0x0049E191, PatchMaxRange);

		// dword weapons_dat_min_range
		// 004400F0  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 00440586  |.  8B0C8D 186A65>MOV ECX,DWORD PTR DS:[ECX*4+656A18]
		// 00440590  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 004405A5  |.  8B0C85 186A65>MOV ECX,DWORD PTR DS:[EAX*4+656A18]
		// 004405BA  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 00462FCB  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 00466077  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 00476525  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 00476682  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 004768FB  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 00477C48  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 004787A4  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 00478C15  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 0047BB44  |.  8B0485 186A65>MOV EAX,DWORD PTR DS:[EAX*4+656A18]
		// 0047C5D4  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		// 0047C688  |.  8B0495 186A65>MOV EAX,DWORD PTR DS:[EDX*4+656A18]
		patchWeaponDat(3, 0x004400F0, PatchMinRange);
		patchWeaponDat(3, 0x00440586, PatchMinRange);
		patchWeaponDat(3, 0x00440590, PatchMinRange);
		patchWeaponDat(3, 0x004405A5, PatchMinRange);
		patchWeaponDat(3, 0x004405BA, PatchMinRange);
		patchWeaponDat(3, 0x00462FCB, PatchMinRange);
		patchWeaponDat(3, 0x00466077, PatchMinRange);
		patchWeaponDat(3, 0x00476525, PatchMinRange);
		patchWeaponDat(3, 0x00476682, PatchMinRange);
		patchWeaponDat(3, 0x004768FB, PatchMinRange);
		patchWeaponDat(3, 0x00477C48, PatchMinRange);
		patchWeaponDat(3, 0x004787A4, PatchMinRange);
		patchWeaponDat(3, 0x00478C15, PatchMinRange);
		patchWeaponDat(3, 0x0047BB44, PatchMinRange);
		patchWeaponDat(3, 0x0047C5D4, PatchMinRange);
		patchWeaponDat(3, 0x0047C688, PatchMinRange);

		// word weapons_dat_label
		// 00457720  |.  66:8B0C55 E07>MOV CX,WORD PTR DS:[EDX*2+6572E0]
		// 00457788  |.  66:8B0C55 E07>MOV CX,WORD PTR DS:[EDX*2+6572E0]
		// 004577FB  |.  66:8B0C4D E07>MOV CX,WORD PTR DS:[ECX*2+6572E0]
		// 004DA0BA   >  8B0D 68385100 MOV ECX,DWORD PTR DS:[513868]            ;  StarCraf.006572E0
		patchWeaponDat(4, 0x00457720, PatchLabel);
		patchWeaponDat(4, 0x00457788, PatchLabel);
		patchWeaponDat(4, 0x004577FB, PatchLabel);
		patchWeaponDat(2, 0x004DA0BA, PatchLabel);	

		// weapons_dat icon
		// 0x00425840  |.  66:8B0C55 806>MOV CX,WORD PTR DS:[EDX*2+656780]
		patchWeaponDat(4, 0x00425840, PatchIcon);

		// // Weapon checks
		// void __stdcall drawDisplayButtons(int a2) // 00426C60
		// 00426CFC  |.  80FB 82       CMP BL,82
		// 00426D16  |>  80F9 82       CMP CL,82
		// 00426D30  |>  80FB 82       CMP BL,82
		// 00426D74  |.  80FB 82       CMP BL,82
		// 00426D92  |>  3C 82         CMP AL,82
		// 00426D9A  |.  80FB 82       CMP BL,82
		// 00426DD7  |.  3C 82         CMP AL,82
		// 00426DF5  |.  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x00426CFC, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00426D16, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00426D30, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00426D74, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00426D92, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00426D9A, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00426DD7, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00426DF5, PatchMaxWeaponCompare);

		// void __thiscall UnitStatAct_Building(void *dlg) // 00427890
		// 004279EF   .  3C 82         CMP AL,82
		patchWeaponDat(1, 0x004279EF, PatchMaxWeaponCompare);

		// int __userpurge calcUnitStrength@<eax>(int a1@<eax>, unsigned __int8 weaponType) // 00431270
		// 004312D6  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x004312D6, PatchMaxWeaponCompare);

		// unsigned int __cdecl sub_4313C0() // 004313C0
		// 00431416  |.  3D 82000000   |CMP EAX,82
		// 0043148D  |.  3D 82000000   |CMP EAX,82
		patchWeaponDat(1, 0x00431416, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x0043148D, PatchMaxWeaponCompare);
	
		// CUNIT *__userpurge sub_431990@<eax>(int *a1@<eax>, int arg0, CUNIT *a2, int a4, int a5) // 00431990
		// 00431B13  |.  80F9 82       |CMP CL,82
		// 00431B24  |.  3C 82         |CMP AL,82
		patchWeaponDat(2, 0x00431B13, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00431B24, PatchMaxWeaponCompare);
		
		// BOOL __usercall sub_437E70@<eax>(unsigned int playerID@<eax>, int rgn1@<ebx>, int rgn2@<esi>) // 00437E70
		// 00437ECB  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x00437ECB, PatchMaxWeaponCompare);

		// void __usercall getAITownCapabilities(aiCaptain *pCapn@<eax>, DWORD *a2@<ebx>, DWORD *a3@<edi>, DWORD *a4@<esi>) // 00438FC0
		// 00439012  |.  80F9 82       |CMP CL,82
		// 00439028  |.  80F9 82       |CMP CL,82
		patchWeaponDat(2, 0x00439012, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00439028, PatchMaxWeaponCompare);

		// int __stdcall sub_43ADA0(CUNIT *arg0, CUNIT *a2, int a3) // 0043ADA0
		// 0043AFD4  |>  3C 82         |CMP AL,82
		patchWeaponDat(1, 0x0043AFD4, PatchMaxWeaponCompare);

		// void __usercall prepare_suicide(CUNIT *eax0@<eax>) // 0043CD40
		// 0043CFAA   .  80FA 82       CMP DL,82
		// 0043CFB9   .  3C 82         CMP AL,82
		patchWeaponDat(2, 0x0043CFAA, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x0043CFB9, PatchMaxWeaponCompare);

		// void __stdcall AI_AttackUnit(CUNIT *unitMem) // 0043FFD0
		// 004400E8  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x004400E8, PatchMaxWeaponCompare);

		// char __usercall sub_440520@<al>(CUNIT *a1@<esi>) // 00440520
		// 00440540  |.  80F9 82       CMP CL,82
		// 00440561  |.  3C 82         CMP AL,82
		// 0044057A  |>  80F9 82       CMP CL,82
		// 0044057F  |.  3C 82         CMP AL,82
		// 004405B3  |>  3C 82         CMP AL,82
		patchWeaponDat(2, 0x00440540, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00440561, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0044057A, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x0044057F, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x004405B3, PatchMaxWeaponCompare);
		
		// int __fastcall canHitUnit(CUNIT *pTarget, CUNIT *pUnit) // 00441870
		// 004418DE   .  3C 82         CMP AL,82
		// 00441905   .  3C 82         CMP AL,82
		patchWeaponDat(1, 0x004418DE, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00441905, PatchMaxWeaponCompare);

		// signed int __userpurge sub_462EA0@<eax>(CUNIT *a1@<eax>, int a2) // 00462EA0
		// 00462FC3  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x00462FC3, PatchMaxWeaponCompare);

		// void __usercall orders_StrafeUnit(CUNIT *this@<eax>) // 00465F60
		// 0046606F  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x0046606F, PatchMaxWeaponCompare);

		// __int16 __usercall getTargettingErrorString@<ax>(DWORD a1@<eax>, int a2@<edx>, CUNIT *a3@<ecx>) // 0046EFE0
		// 0046F023  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x0046F023, PatchMaxWeaponCompare);

		// char __thiscall Unit::getAirWeapon(CUNIT *this) // 00475840
		// 0047584A  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x0047584A, PatchMaxWeaponCompare);

		// BOOL __usercall hasGrndWeaponGraphic@<eax>(int a1@<eax>) // 00475A10
		// 00475A26  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x00475A26, PatchMaxWeaponCompare);

		// char __thiscall getGroundWeaponType(CUNIT *this) // 00475A90
		// 00475AAC  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x00475AAC, PatchMaxWeaponCompare);

		// BOOL __usercall unitCanAttackGroundType@<eax>(unsigned __int16 a1@<ax>, CUNIT *pUnit@<ecx>) // 00475C80
		// 00475CC2  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x00475CC2, PatchMaxWeaponCompare);

		// int __userpurge canWeaponTargetUnit@<eax>(unsigned __int8 wpnType@<al>, CUNIT *target@<edx>, CUNIT *a3) // 00475CE0
		// 00475CE3  |.  3C 82         CMP AL,82
		patchWeaponDat(1, 0x00475CE3, PatchMaxWeaponCompare);

		// bool __userpurge isUnitTargetOutOfMaxRange@<al>(CUNIT *unit@<eax>, CUNIT *target) // 00476430
		// 004764A0  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x004764A0, PatchMaxWeaponCompare);

		// bool __usercall isUnitTargetInsideMinRange@<al>(CUNIT *unit@<eax>, CUNIT *target@<esi>) // 004764D0
		// 0047651E  |>  3C 82         CMP AL,82
		patchWeaponDat(1, 0x0047651E, PatchMaxWeaponCompare);

		// bool __userpurge unit_can_attack_target@<al>(CUNIT *target@<ebx>, CUNIT *unit@<esi>, int a3) // 00476730
		// 0047683E  |.  80FA 82       CMP DL,82
		// 00476854  |.  3C 82         CMP AL,82
		patchWeaponDat(2, 0x0047683E, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00476854, PatchMaxWeaponCompare);

		// bool __userpurge isUnitInWeaponRange@<al>(CUNIT *a1@<eax>, CUNIT *a2) // 00476870
		// 004768F3  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x004768F3, PatchMaxWeaponCompare);

		// signed int __userpurge doWeaponIscript@<eax>(CUNIT *a1@<eax>, unsigned __int8 a2@<bl>, int a3@<edi>, char iscriptAnim) // 00476C90
		// 00476C93  |.  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x00476C93, PatchMaxWeaponCompare);

		// void __usercall orders_TurretAttack(CUNIT *a1@<eax>, int a2@<ecx>) // 00477980
		// 00477ADE  |.  80F9 82       CMP CL,82
		// 00477AEF  |.  3C 82         CMP AL,82
		// 00477C40  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x00477ADE, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00477AEF, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x00477C40, PatchMaxWeaponCompare);

		// void __usercall orders_AttackFixedRange(CUNIT *this@<eax>, int a2@<ecx>) // 00477D00
		// 00477E2C  |.  80F9 82       CMP CL,82
		// 00477E3D  |.  3C 82         CMP AL,82
		patchWeaponDat(2, 0x00477E2C, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00477E3D, PatchMaxWeaponCompare);

		// signed int __usercall OrderAttackBehaviour@<eax>(CUNIT *a1@<eax>) // 00478540
		// 00478636  |.  80FA 82       CMP DL,82
		// 00478647  |.  3C 82         CMP AL,82
		// 0047879C  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x00478636, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x00478647, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0047879C, PatchMaxWeaponCompare);
		
		// void __usercall orders_WatchTarget(CUNIT *this@<eax>) // 0047BAB0
		// 0047BB3C  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x0047BB3C, PatchMaxWeaponCompare);

		// void __usercall orders_StayInRange(CUNIT *this@<eax>) // 0047C4F0
		// 0047C5CC  |>  80FB 82       CMP BL,82
		// 0047C680  |>  80FB 82       CMP BL,82
		patchWeaponDat(2, 0x0047C5CC, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0047C680, PatchMaxWeaponCompare);

		// BOOL __thiscall FindNextBounceTargetProc(CUNIT *pIterator) // 0048AFD0
		// 0048B0C7   .  80FA 82       CMP DL,82
		// 0048B0D8   .  3C 82         CMP AL,82
		patchWeaponDat(2, 0x0048B0C7, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x0048B0D8, PatchMaxWeaponCompare);

		// BOOL __usercall CMDRECV_PlaceBuildingAllowed@<eax>(unsigned __int16 a1@<ax>, unsigned __int8 a2@<dl>, CUNIT *a3@<ecx>) // 0048DBD0
		// 0048DBDC  |.  81F9 82000000 CMP ECX,82
		patchWeaponDat(2, 0x0048DBDC, PatchMaxWeaponCompare);

		// void __cdecl ReassignTargetAcquisitionRange() // 0049E010
		// 0049E04A   .  81F9 82000000 CMP ECX,82
		// 0049E070   .  81F9 82000000 CMP ECX,82
		// 0049E0A7   .  81F9 82000000 CMP ECX,82
		// 0049E0CD   .  81F9 82000000 CMP ECX,82
		// 0049E106   .  81F9 82000000 CMP ECX,82
		// 0049E12C   .  81F9 82000000 CMP ECX,82
		// 0049E163   .  81F9 82000000 CMP ECX,82
		// 0049E189   .  81F9 82000000 CMP ECX,82
		patchWeaponDat(2, 0x0049E04A, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E070, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E0A7, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E0CD, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E106, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E12C, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E163, PatchMaxWeaponCompare);
		patchWeaponDat(2, 0x0049E189, PatchMaxWeaponCompare);
		
		// bool __usercall Unload@<al>(CUNIT *a1@<eax>) // 004E7F70
		// 004E807E  |.  3D 82000000   CMP EAX,82
		// 004E8094  |.  3D 82000000   CMP EAX,82
		patchWeaponDat(1, 0x004E807E, PatchMaxWeaponCompare);
		patchWeaponDat(1, 0x004E8094, PatchMaxWeaponCompare);

	}
}