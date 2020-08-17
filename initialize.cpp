#include "definitions.h"
#include "Plugin.h"
#include "globals.h"
#include "hook_tools.h"

//Hook header files
#include "hooks/game_hooks.h"
#include "graphics/draw_hook.h"

#include "hooks/maxhp.h"
#include "hooks/apply_upgrade_flags.h"
#include "hooks/attack_priority.h"
#include "hooks/bunker_hooks.h"
#include "hooks/cloak_nearby_units.h"
#include "hooks/cloak_tech.h"
#include "hooks/detector.h"
#include "hooks/harvest.h"
#include "hooks/rally_point.h"
#include "hooks/recharge_shields.h"
#include "hooks/spider_mine.h"
#include "hooks/stim_packs.h"
#include "hooks/tech_target_check.h"
#include "hooks/transfer_tech_upgrades.h"
#include "hooks/unit_speed.h"
#include "hooks/update_status_effects.h"
#include "hooks/update_unit_state.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapons/weapon_fire.h"

#include "hooks/unit_destructor_special.h"
#include "hooks/psi_field.h"

#include "hooks/unit_stats/armor_bonus.h"
#include "hooks/unit_stats/sight_range.h"
#include "hooks/unit_stats/max_energy.h"
#include "hooks/unit_stats/weapon_range.h"
#include "hooks/interface/weapon_armor_tooltip.h"

//in alphabetical order
#include "hooks/orders/ai/ai_orders.h"
#include "hooks/orders/base_orders/attack_orders.h"
#include "hooks/interface/btns_cond.h"
#include "hooks/orders/building_making/building_morph.h"
#include "hooks/interface/buttonsets.h"
#include "hooks/orders/building_making/building_protoss.h"
#include "hooks/orders/building_making/building_terran.h"
#include "hooks/recv_commands/burrow_tech.h"
#include "hooks/orders/spells/cast_order.h"
#include "hooks/recv_commands/CMDRECV_Build.h"
#include "hooks/recv_commands/CMDRECV_Cancel.h"
#include "hooks/recv_commands/CMDRECV_MergeArchon.h"
#include "hooks/recv_commands/CMDRECV_Morph.h"
#include "hooks/recv_commands/CMDRECV_SiegeTank.h"
#include "hooks/recv_commands/CMDRECV_Stop.h"
#include "hooks/create_init_units.h"
#include "hooks/orders/base_orders/die_order.h"
#include "hooks/orders/enter_nydus.h"
#include "hooks/orders/spells/feedback_spell.h"
#include "hooks/give_unit.h"
#include "hooks/orders/spells/hallucination_spell.h"
#include "hooks/orders/infestation.h"
#include "hooks/orders/larva_creep_spawn.h"
#include "hooks/orders/liftland.h"
#include "hooks/orders/load_unload_orders.h"
#include "hooks/load_unload_proc.h"
#include "hooks/orders/building_making/make_nydus_exit.h"
#include "hooks/orders/medic_orders.h"
#include "hooks/orders/merge_units.h"
#include "hooks/orders/spells/mindcontrol_spell.h"
#include "hooks/miss_chance.h"
#include "hooks/orders/spells/nuke_orders.h"
#include "hooks/orders/spells/recall_spell.h"
#include "hooks/recv_commands/receive_command.h"
#include "hooks/orders/repair_order.h"
#include "hooks/orders/research_upgrade_orders.h"
#include "hooks/interface/select_larva.h"
#include "hooks/interface/selection.h"
#include "hooks/audiovisuals/sfxhooks.h"
#include "hooks/orders/siege_transform.h"
#include "hooks/interface/stats_panel_display.h"
#include "hooks/orders/base_orders/stopholdpos_orders.h"
#include "hooks/recv_commands/train_cmd_receive.h"
#include "hooks/orders/unit_making/unit_morph.h"
#include "hooks/orders/unit_making/unit_train.h"
#include "hooks/utils/utils.h"
#include "hooks/interface/wireframe.h"
#include "hooks/weapons/wpnspellhit.h"
#include "hooks/weapon_effect.h"
#include "hooks/use_tech.h"
#include "hooks/limits/ccmu.h"
#include "hooks/economic_units.h"
#include "hooks/limits/extender.h"
#include "hooks/build_time.h"
#include "hooks/misc/hydra_specific.h"
//#include "AI/spellcasting.h"

/// This function is called when the plugin is loaded into StarCraft.
/// You can enable/disable each group of hooks by commenting them.
/// You can also add custom modifications to StarCraft.exe by using:
///		memoryPatch(address_to_patch, value_to_patch_with);

namespace
{
	void runWMode()
	{
		//KYSXD W-Mode start
		int msgboxID = MessageBox(NULL, "Do you want to play in windowed mode?", PLUGIN_NAME, MB_YESNOCANCEL);
		switch (msgboxID) {
		case IDYES:
			HINSTANCE hDll;
			hDll = LoadLibrary("./WMODE.dll");
			hDll = LoadLibrary("./WMODE_FIX.dll");
			break;
		case IDCANCEL:
			exit(0);
		}//KYSXD W-Mode end
	}
}
BOOL WINAPI Plugin::InitializePlugin(IMPQDraftServer* lpMPQDraftServer) {
	runWMode();
	//StarCraft.exe version check
	char exePath[300];
	const DWORD pathLen = GetModuleFileName(NULL, exePath, sizeof(exePath));

	if (pathLen == sizeof(exePath)) {
		MessageBox(NULL, "Error: Cannot check version of StarCraft.exe. The file path is too long.", NULL, MB_OK);
		return FALSE;
	}

	if (!checkStarCraftExeVersion(exePath))
		return FALSE;


	//disable powerup hardcod
	// FLAG id only
	const u8 maxunsigned_2[5] = {0xff, 0xff };
	memoryPatch(0x0044095D+4, maxunsigned_2, 2);//flag
	memoryPatch(0x0045FE5C+1, maxunsigned_2, 2);//flag
	memoryPatch(0x00460902+ 1, maxunsigned_2, 2);//flag
	memoryPatch(0x0049F145 + 4, maxunsigned_2, 2);//flag
	memoryPatch(0x004E4153 + 4, maxunsigned_2, 2);//flag
	memoryPatch(0x004E416E + 4, maxunsigned_2, 2);//flag
//	memoryPatch(0x004E426B + 1, maxunsigned_2, 2);//ctf create flag, this code don't runs at all because CTF are spell orders now
	memoryPatch(0x004E5F68 + 3, maxunsigned_2, 2);
	memoryPatch(0x004E70E4 + 4, maxunsigned_2, 2);
	memoryPatch(0x004F3A9C + 4, maxunsigned_2, 2);
	memoryPatch(0x004F3C26 + 4, maxunsigned_2, 2);
	memoryPatch(0x004F3CD5 + 4, maxunsigned_2, 2);
	//iquare, start
	//medic hardcode patches

	const u8 none = 0xE4;
	const u8 moveaxpatch[5] = { 0xb8,  0x00, 0x00, 0x00, 0x00 };
	/*
	memoryPatch(0x0049ABEB + 2, &none, sizeof(u8));//disable turning of attack/move to heal-move on medics
	memoryPatch(0x00442358 + 4, &none, sizeof(u8));//disable medicheal/target acquire
	memoryPatch(0x00478263 + 4, &none, sizeof(u8));//disable turning of patrol to heal-move on medics
	memoryPatch(0x00455EFC + 3, &none, sizeof(u8));//changing right-click proc on medics
	memoryPatch(0x004784A8 + 4, &none, sizeof(u8));//disable computer return on medics
	memoryPatch(0x0043DCAD + 4, &none, sizeof(u8));//disable forced return to base on medics
	//
	memoryPatch(0x004C2124 + 2, &none, sizeof(u8));//disable medic-hold position
	memoryPatch(0x0043D70E + 4, &none, sizeof(u8));//disable medic hardcode in progress ai function
	memoryPatch(0x0043DAD5 + 4, &none, sizeof(u8));//disable medic hardcode in add military ai function
	memoryPatch(0x0043C215 + 4, &none, sizeof(u8));//disable unknown medic-related hardcode
	memoryPatch(0x004A290B + 2, &none, sizeof(u8));//disable computer_script medic hardcode
	memoryPatch(0x004477C0 + 4, &none, sizeof(u8));//disable medic_remove
	memoryPatch(0x0046305B + 4, &none, sizeof(u8));//disable give_ai medic hardcode
	memoryPatch(0x004A2854 + 4, &none, sizeof(u8));//disable inheritai2 medic hardcode
	memoryPatch(0x0043F92A + 4, &none, sizeof(u8));//disable ai react to hit hardcode
	*/
	memoryPatch(0x004357FB + 2, &none, sizeof(u8));//disable placing units near bunkers
	memoryPatch(0x0043583F + 1, &none, sizeof(u8));//disable placing units near missile turrets
	memoryPatch((void*)0x00487C93, moveaxpatch, sizeof(moveaxpatch));//disable unit selectable check in placement function

	//disable terran structure building transport condition
	const u8 fourth = 0x3;
	memoryPatch(0x004E6E7A + 1, &fourth, sizeof(u8));
	const u8 jnz = 0x75;
	memoryPatch(0x004E6E7C, &jnz, sizeof(u8));


	//hooks::injectEconomicUnitHooks();
	//test for globals
//	nops(0x004AAF23, 5);
//	const u8 retn = 0xc3;
//	memoryPatch(0x004AAF23 + 5, &retn, 1);
	injectGlobalsHooks();
	hooks::injectHydraSpecific();
	//hooks::injectLimitHooks();
	//


	//town hall code

	//worker code
	//00434C36, 0043378C, 0043214B
	//townhall code
	//00433f62, 0043A100, 004365f6, 00433EB5, 0044387B, 00446230, 

	//hatchery-lair-hive code, required later to patch that for sire/brood
	//0043A100,00433EB5,00401cf0,0043A146,004A1F8B (partial, id check is for zerg only)
	//00402C24, 0042850A, 0042974B,00444D51,004E8C04,004EA88C(hook exists, larva)
	//hatchery-only code 00434B2F,00436380,


	u8 patchLastUnit[] = { 0xE4 };
	//allow pissed/yes sounds for >= Command Center
//	memoryPatch(0x0048EA20 + 2, patchLastUnit, 1);
	//hook required
//	nops(0x0048EAB0, 5);


	//kakaru/exemplar hardcode
	memoryPatch(0x00431586 + 2, patchLastUnit, 1);
	memoryPatch(0x00431B49 + 2, patchLastUnit, 1);
	memoryPatch(0x00437E1D + 2, patchLastUnit, 1);
	memoryPatch(0x00437E4C + 2, patchLastUnit, 1);
	memoryPatch(0x0047B466 + 2, patchLastUnit, 1);
	memoryPatch(0x0047B7DB + 2, patchLastUnit, 1);
	memoryPatch(0x0047C425 + 4, patchLastUnit, 1);
	memoryPatch(0x0047C476 + 4, patchLastUnit, 1);
	memoryPatch(0x004CBE5D + 2, patchLastUnit, 1);
	//ursadon/kasan screamer hardcode
	memoryPatch(0x00431592 + 2, patchLastUnit, 1);
	memoryPatch(0x00431B55 + 2, patchLastUnit, 1);
	memoryPatch(0x00437E29 + 2, patchLastUnit, 1);
	memoryPatch(0x00437E58 + 2, patchLastUnit, 1);
	memoryPatch(0x0047B472 + 2, patchLastUnit, 1);
	memoryPatch(0x0047B7E7 + 2, patchLastUnit, 1);
	memoryPatch(0x004CBE62 + 2, patchLastUnit, 1);
	//
	//warbringer/clarion hardcode
	memoryPatch(0x004F6C1C + 2, patchLastUnit, 1);
	memoryPatch(0x004F6578 + 2, patchLastUnit, 1);
	memoryPatch(0x004F656B + 2, patchLastUnit, 1);
	memoryPatch(0x004E36EA + 2, patchLastUnit, 1);
	memoryPatch(0x004E3029 + 3, patchLastUnit, 1);
	memoryPatch(0x004CD28C + 2, patchLastUnit, 1);
	memoryPatch(0x004C8ADE + 3, patchLastUnit, 1);
	memoryPatch(0x004C8A94 + 2, patchLastUnit, 1);
	memoryPatch(0x004C8A5F + 2, patchLastUnit, 1);
	memoryPatch(0x004C20FA + 2, patchLastUnit, 1);
	memoryPatch(0x004C189C + 2, patchLastUnit, 1);
	memoryPatch(0x004C1859 + 2, patchLastUnit, 1);
	memoryPatch(0x004C1836 + 2, patchLastUnit, 1);
	memoryPatch(0x0049F4B9 + 2, patchLastUnit, 1);
	memoryPatch(0x0047CA8C + 2, patchLastUnit, 1);
	memoryPatch(0x004761D5 + 2, patchLastUnit, 1);
	memoryPatch(0x00475CAB + 2, patchLastUnit, 1);
	memoryPatch(0x00475B59 + 2, patchLastUnit, 1);
	memoryPatch(0x00468222 + 2, patchLastUnit, 1);
	memoryPatch(0x004668A5 + 2, patchLastUnit, 1);
	memoryPatch(0x004667A9 + 2, patchLastUnit, 1);
	memoryPatch(0x00466764 + 2, patchLastUnit, 1);
	memoryPatch(0x004659FD + 2, patchLastUnit, 1);
	memoryPatch(0x0046571A + 2, patchLastUnit, 1);
	memoryPatch(0x00465466 + 2, patchLastUnit, 1);
	memoryPatch(0x00465412 + 2, patchLastUnit, 1);
	memoryPatch(0x004653A7 + 3, patchLastUnit, 1);
	memoryPatch(0x0046536A + 2, patchLastUnit, 1);
	memoryPatch(0x0046534B + 2, patchLastUnit, 1);
	memoryPatch(0x004608D9 + 2, patchLastUnit, 1);
	memoryPatch(0x0045FEBD + 2, patchLastUnit, 1);
	memoryPatch(0x0045FE98 + 2, patchLastUnit, 1);
	memoryPatch(0x004407B5 + 2, patchLastUnit, 1);
	memoryPatch(0x0043CF4D + 2, patchLastUnit, 1);
	memoryPatch(0x004286B5 + 3, patchLastUnit, 1);
	memoryPatch(0x00426E4A + 2, patchLastUnit, 1);
	memoryPatch(0x004254D2 + 2, patchLastUnit, 1);
	memoryPatch(0x0042549C + 2, patchLastUnit, 1);
	memoryPatch(0x004253E6 + 2, patchLastUnit, 1);
	memoryPatch(0x0042491E + 3, patchLastUnit, 1);
	memoryPatch(0x004247C4 + 3, patchLastUnit, 1);
	memoryPatch(0x004014C6 + 2, patchLastUnit, 1);
	memoryPatch(0x0040149A + 2, patchLastUnit, 1);
	//dark archon - energy of mcontrolled dark archons no longer set to 0
//	memoryPatch(0x004F6938 + 4, patchLastUnit, 1);
	nops(0x0049EEBD, 0xa);//disable beacon collision hardcode
//	u8 p8a = { 0x8a };

	// TODO: Resolve this bug
	// This code has a bug, that makes workers not remove their resource chunks properly 
	//const u8 signff[2] = { 0xff,0x7f };
	//memoryPatch(0x004981C4 + 3, signff, 2);
	//memoryPatch(0x00497764 + 3, signff, 2);//lardcode
	//memoryPatch(0x004F3944 + 3, signff, 2);//lardcode
	//memoryPatch(0x00497784 + 3, signff, 2);//disable burrow hardcode for image ids: Flag, Young Chrysalis, Uraj, Khalis
	
	// This code is a replacement for the code above, that does not work properly
	u8 p8a = { 0x8a };
	u8 ff = { 0xff };
	memoryPatch(0x00497764 + 3, &p8a, sizeof(u8));//
	memoryPatch(0x00497784 + 3, &ff, sizeof(u8));//disable burrow hardcode for image ids: Flag, Young Chrysalis, Uraj, Khalis

	//ctf orders
	u8 patchCTF1[] = { 0xE9, 0xCC, 0x02, 0x00, 0x00 };
	memoryPatch(0x004EC80A, patchCTF1, sizeof patchCTF1);//CTFCOPInit //wyvern?
	u8 patchCTF2[] = { 0xE9, 0xC2, 0x02, 0x00, 0x00 };
	memoryPatch(0x004EC814, patchCTF2, sizeof patchCTF2);//CTFCOP1 //essence drain
	//

	//iquare, end

	//FaRTy1billion, start
	//crash fix for irregular minimap sizes
	const u8 nop3[3] = { 0x8d,0x76,0x00 };
	const u8 b74 = 0x74;
	u8 swtab[193];
	int i = 0;
	for (; i <= 64; i++) swtab[i] = 0;
	for (; i <= 128; i++) swtab[i] = 1;
	for (; i <= 192; i++) swtab[i] = 2;
	memoryPatch((void*)(0x004A4424 + 2), &b74, sizeof(u8));
	memoryPatch((void*)0x004A4584, swtab, sizeof(swtab));
	memoryPatch((void*)0x004A441B, nop3, sizeof(nop3));

	//allow display of extended tile groups
	const u8 nop = 0x90;
	const u8 nop6[6] = { 0x8d,0xb6,0x00,0x00,0x00,0x00 };
	const u8 bFF = 0xFF;
	const u8 b0F = 0x0F;
	const u8 b00 = 0x00; // easy workaround
	const u8 b01 = 0x01;

	memoryPatch((void*)(0x0047E347 + 3), &b0F, sizeof(u8));
	memoryPatch((void*)(0x0047E38F + 2), &b0F, sizeof(u8));
	memoryPatch((void*)(0x0049C563 + 3), &b0F, sizeof(u8));
	memoryPatch((void*)(0x0049C6AC + 3), &b0F, sizeof(u8));
	memoryPatch((void*)(0x004BCEE0 + 2), &b0F, sizeof(u8));
	memoryPatch((void*)(0x004BD13F + 3), &b0F, sizeof(u8));
	memoryPatch((void*)(0x0049C595 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0049C6DA + 3), &bFF, sizeof(u8));

	memoryPatch((void*)0x0047E3BA, nop6, sizeof(nop6));
	memoryPatch((void*)0x0049C580, nop6, sizeof(nop6));
	memoryPatch((void*)0x0049C6C9, nop6, sizeof(nop6));
	memoryPatch((void*)0x004BCF1E, nop6, sizeof(nop6));

	//allow building on extended tile groups
	memoryPatch((void*)(0x004BCF34 + 4), &b00, sizeof(u8));
	memoryPatch((void*)(0x00413F38 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00413F8D + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414210 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414252 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x004142C5 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x004142EF + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414326 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0041435A + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414390 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x004143C6 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x004143F2 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0041441A + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414467 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x004145D5 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414720 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x00414882 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0047D75A + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0047DC99 + 2), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0047DED5 + 3), &bFF, sizeof(u8));
	memoryPatch((void*)(0x0047E045 + 3), &bFF, sizeof(u8));
	//
	//FaRTy1billion, end
	//


	//h
	hooks::injectGameHooks();
	//h
	hooks::injectDrawHook();

	

	//in order of creation
	/*
	hooks::injectInfestationHooks();
	hooks::injectSiegeTransformHooks();*/


	hooks::injectButtonSetHooks();

	//h
	hooks::injectSelectMod();

	/*
	hooks::injectMergeUnitsHooks();*/


	//h
	hooks::injectLarvaCreepSpawnHooks();

	/*
	hooks::injectLiftLandHooks();*//*
	hooks::injectAttackOrdersHooks();
	hooks::injectStopHoldPosOrdersHooks();*/
	hooks::injectRecallSpellHooks();
	hooks::injectEnterNydusHooks();/*

	hooks::injectCastOrderHooks();*/
	hooks::injectWpnSpellHitHooks();

	hooks::injectBuildingMorphHooks();
	
	hooks::injectMakeNydusExitHook();
	hooks::injectUnitMorphHooks();/*
	hooks::injectWireframeHook();
	hooks::injectDieOrderHook();
	hooks::injectBuildingTerranHook();
	hooks::injectBuildingProtossHooks();*/
	hooks::injectUnitTrainHooks();


	hooks::injectLoadUnloadProcHooks();
	hooks::injectLoadUnloadOrdersHooks();/*
	hooks::injectNukeOrdersHooks();
	hooks::injectBurrowTechHooks();
	hooks::injectResearchUpgradeOrdersHooks();*/
	hooks::injectMedicOrdersHooks();
	hooks::injectHallucinationSpellHook();/*
	hooks::injectFeedbackSpellHook();
	hooks::injectBtnsCondHook();
	hooks::injectRecvCmdHook();*/
	hooks::injectCreateInitUnitsHooks();
//	hooks::injectGiveUnitHook();
	hooks::injectTrainCmdRecvHooks();/*
	hooks::injectCMDRECV_SiegeTankHooks();
	hooks::injectCMDRECV_MergeArchonHooks();*/


	hooks::injectCMDRECV_MorphHooks();
	/*


	hooks::injectCMDRECV_StopHooks();*/
	hooks::injectCMDRECV_CancelHooks();/*
	hooks::injectSelectLarvaHooks();*/
	hooks::injectRepairOrderHook();/*
	hooks::injectStatsPanelDisplayHook();
	hooks::injectUtilsHooks();*/
	hooks::injectMindControlSpellHook();/*
	hooks::injectCMDRECV_BuildHooks();
	*/

	hooks::injectApplyUpgradeFlags();
	hooks::injectAttackPriorityHooks();
	
	hooks::injectBunkerHooks();
	hooks::injectCloakNearbyUnits();
	/*
	hooks::injectCloakingTechHooks();
	hooks::injectDetectorHooks();*/
	hooks::injectHarvestResource();
	/*
	hooks::injectRallyHooks();*/

//	hooks::injectRechargeShieldsHooks();

	/*
	hooks::injectSpiderMineHooks();
	hooks::injectStimPacksHooks();*/
	hooks::injectTechTargetCheckHooks();/*
	hooks::injectTransferTechAndUpgradesHooks();*/
	hooks::injectUnitSpeedHooks();
	hooks::injectUpdateStatusEffects();
	hooks::injectUpdateUnitState();
	hooks::injectWeaponCooldownHook();
	hooks::injectWeaponDamageHook();

	hooks::injectWeaponFireHooks();


	hooks::injectUnitDestructorSpecial();//hydra

	hooks::injectWeaponEffectHook();
	hooks::injectUseTechHooks();
	
	//hooks::injectPsiFieldHooks();

	hooks::injectArmorBonusHook();
	hooks::injectSightRangeHook();
	hooks::injectUnitMaxEnergyHook();
	hooks::injectWeaponRangeHooks();

	hooks::injectUnitTooltipHook();
	hooks::injectMaxHpHooks();

//	hooks::injectSpellcasterAI();
//	hooks::injectAIOrdersHooks();
	hooks::injectMissChanceHooks();
	hooks::injectBuildTimeHooks();

	return TRUE;
}
