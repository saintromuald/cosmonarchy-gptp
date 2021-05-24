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

	PatchNoneWeaponCMP,
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

	if (patchType == PatchNoneWeaponCMP)
	{
		const u8 noneWeaponId = 256;
		memoryPatch(address + opcodeSize, noneWeaponId);
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
		//patchWeaponDat(2, 0x0048BF1B, PatchDeathTime);

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
		//patchWeaponDat(2, 0x0048ACD8, PatchEffect);
		//patchWeaponDat(2, 0x0048B58C, PatchEffect);
		//patchWeaponDat(3, 0x0048B77E, PatchEffect);

		// word weapons_dat_outer_splash
		// 0048B181   .  0FB78B 807765>MOVZX ECX,WORD PTR DS:[EBX+657780]
		// 0048B5C0  |.  0FB7144D 8077>MOVZX EDX,WORD PTR DS:[ECX*2+657780]
		// 0048B828  |>  0FB7044D 8077>MOVZX EAX,WORD PTR DS:[ECX*2+657780]     ;  Case 18 of switch 0048B785
		// 0048B8CC  |.  0FB70445 8077>MOVZX EAX,WORD PTR DS:[EAX*2+657780]
		patchWeaponDat(3, 0x0048B828, PatchOuterSplashRadius);
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
		patchWeaponDat(3, 0x00475D19, PatchTargetFlags);

		// byte weapons_dat_upgrade
		// 0042579D  |.  66:0FB688 D07>MOVZX CX,BYTE PTR DS:[EAX+6571D0]
		// 00475E47  |.  66:0FB68A D07>MOVZX CX,BYTE PTR DS:[EDX+6571D0]
		// 00475EC7  |.  66:0FB68A D07>MOVZX CX,BYTE PTR DS:[EDX+6571D0]
		// 0048ACE5  |.  66:0FB688 D07>MOVZX CX,BYTE PTR DS:[EAX+6571D0]
		//patchWeaponDat(3, 0x0042579D, PatchUpgrade);
		//patchWeaponDat(3, 0x00475E47, PatchUpgrade);
		//patchWeaponDat(3, 0x00475EC7, PatchUpgrade);
		//patchWeaponDat(3, 0x0048ACE5, PatchUpgrade);

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
		patchWeaponDat(3, 0x00475E9D, PatchDamageUpgrade);
		patchWeaponDat(3, 0x00475EE4, PatchDamageUpgrade);
		patchWeaponDat(3, 0x00475EFF, PatchDamageUpgrade);
		patchWeaponDat(4, 0x0048AD1B, PatchDamageUpgrade);
		patchWeaponDat(1, 0x004BF0F4, PatchDamageUpgrade);
		patchWeaponDat(1, 0x004BF0F9, PatchDamageUpgrade);
		patchWeaponDat(1, 0x004BF28B, PatchDamageUpgrade);
		patchWeaponDat(1, 0x004BF43D, PatchDamageUpgrade);
		patchWeaponDat(2, 0x004CAC6A, PatchDamageUpgrade);
		patchWeaponDat(2, 0x004CAC9B, PatchDamageUpgrade);
		patchWeaponDat(2, 0x004CAE2A, PatchDamageUpgrade);
		patchWeaponDat(2, 0x004CAE5B, PatchDamageUpgrade);

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
		patchWeaponDat(3, 0x0046F02B, PatchErrorMessage);

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
		

		// byte weapons_dat_attack_angle
		// 004675E0  |.  0FB681 906965>MOVZX EAX,BYTE PTR DS:[ECX+656990]
		// 00467E3F  |.  0FB688 906965>MOVZX ECX,BYTE PTR DS:[EAX+656990]
		// 00475C67  |.  0FB68A 906965>MOVZX ECX,BYTE PTR DS:[EDX+656990]
		// 00478C73  |.  0FB682 906965>MOVZX EAX,BYTE PTR DS:[EDX+656990]
		

		// byte weapons_dat_x_offset
		// 00479CDD  |.  0FB697 107965>MOVZX EDX,BYTE PTR DS:[EDI+657910]
		// 004D819A  |.  8890 10796500 |MOV BYTE PTR DS:[EAX+657910],DL
		

		// byte weapons_dat_y_offset
		// 00479CF1  |.  0FB697 206C65>MOVZX EDX,BYTE PTR DS:[EDI+656C20]
		

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
		

		// word weapons_dat_label
		// 00457720  |.  66:8B0C55 E07>MOV CX,WORD PTR DS:[EDX*2+6572E0]
		// 00457788  |.  66:8B0C55 E07>MOV CX,WORD PTR DS:[EDX*2+6572E0]
		// 004577FB  |.  66:8B0C4D E07>MOV CX,WORD PTR DS:[ECX*2+6572E0]
		// 004DA0BA   >  8B0D 68385100 MOV ECX,DWORD PTR DS:[513868]            ;  StarCraf.006572E0
	}
}