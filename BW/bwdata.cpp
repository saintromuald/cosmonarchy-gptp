#include <BW/api.h>
#include <DatExtend/datext.h>
#include <hooks/grafting/grafting.h>

// Macros for actual instance declarations
#define BW_DATA_VAR(type, name, offset)  type name = (type)offset;
#define BW_DATA_DEF(type, name, var)     type& name = (type&)var;

// The header file rudely undefines this
#define BW_DATA(type, name, offset)  type const name = (type)offset;

// Pointers to default data
AI_Main* AIScriptController_dat[] = { (AI_Main*)(0x0068FEE8 + 1256 * 0), (AI_Main*)(0x0068FEE8 + 1256 * 1), (AI_Main*)(0x0068FEE8 + 1256 * 2), (AI_Main*)(0x0068FEE8 + 1256 * 3),
									  (AI_Main*)(0x0068FEE8 + 1256 * 4), (AI_Main*)(0x0068FEE8 + 1256 * 5), (AI_Main*)(0x0068FEE8 + 1256 * 6), (AI_Main*)(0x0068FEE8 + 1256 * 7)
};

_uavail UnitAvailability_dat = { (u8*)(0x0057F27C + 228 * 0), (u8*)(0x0057F27C + 228 * 1), (u8*)(0x0057F27C + 228 * 2), (u8*)(0x0057F27C + 228 * 3),
								 (u8*)(0x0057F27C + 228 * 4), (u8*)(0x0057F27C + 228 * 5), (u8*)(0x0057F27C + 228 * 6), (u8*)(0x0057F27C + 228 * 7),
								 (u8*)(0x0057F27C + 228 * 8), (u8*)(0x0057F27C + 228 * 9), (u8*)(0x0057F27C + 228 * 10), (u8*)(0x0057F27C + 228 * 11)
};

ImagesDatExtraOverlayLO_Files lo_files_dat = { (LO_Header**)0x0051F2A8, (LO_Header**)0x00520244, (LO_Header**)0x005211E0, (LO_Header**)0x0052217C, (LO_Header**)0x00523118 };

_bwTechs TechBw_dat = { { (u8*)(0x0058F050 + 20 * 0),(u8*)(0x0058F050 + 20 * 1),(u8*)(0x0058F050 + 20 * 2),(u8*)(0x0058F050 + 20 * 3), // is_enabled
						  (u8*)(0x0058F050 + 20 * 4),(u8*)(0x0058F050 + 20 * 5),(u8*)(0x0058F050 + 20 * 6),(u8*)(0x0058F050 + 20 * 7),
						  (u8*)(0x0058F050 + 20 * 8),(u8*)(0x0058F050 + 20 * 9),(u8*)(0x0058F050 + 20 * 10),(u8*)(0x0058F050 + 20 * 11) },
						{ (u8*)(0x0058F140 + 20 * 0),(u8*)(0x0058F140 + 20 * 1),(u8*)(0x0058F140 + 20 * 2),(u8*)(0x0058F140 + 20 * 3), // is_researched
						  (u8*)(0x0058F140 + 20 * 4),(u8*)(0x0058F140 + 20 * 5),(u8*)(0x0058F140 + 20 * 6),(u8*)(0x0058F140 + 20 * 7),
						  (u8*)(0x0058F140 + 20 * 8),(u8*)(0x0058F140 + 20 * 9),(u8*)(0x0058F140 + 20 * 10),(u8*)(0x0058F140 + 20 * 11) }
};

_bwUpgrs UpgradesBw_dat = { { (u8*)(0x0058F278 + 20 * 0),(u8*)(0x0058F278 + 20 * 1),(u8*)(0x0058F278 + 20 * 2),(u8*)(0x0058F278 + 20 * 3), // max_level
							  (u8*)(0x0058F278 + 20 * 4),(u8*)(0x0058F278 + 20 * 5),(u8*)(0x0058F278 + 20 * 6),(u8*)(0x0058F278 + 20 * 7),
							  (u8*)(0x0058F278 + 20 * 8),(u8*)(0x0058F278 + 20 * 9),(u8*)(0x0058F278 + 20 * 10),(u8*)(0x0058F278 + 20 * 11) },
							{ (u8*)(0x0058F32C + 20 * 0),(u8*)(0x0058F32C + 20 * 1),(u8*)(0x0058F32C + 20 * 2),(u8*)(0x0058F32C + 20 * 3), // current_level
							  (u8*)(0x0058F32C + 20 * 4),(u8*)(0x0058F32C + 20 * 5),(u8*)(0x0058F32C + 20 * 6),(u8*)(0x0058F32C + 20 * 7),
							  (u8*)(0x0058F32C + 20 * 8),(u8*)(0x0058F32C + 20 * 9),(u8*)(0x0058F32C + 20 * 10),(u8*)(0x0058F32C + 20 * 11) }
};

// Referenced variables modified by DatExt
BW_DATA_VAR(UnitDeaths*,						deathTable_var, 0x0058A364);
BW_DATA_VAR(UnitDeaths*,						unitAllCountTable_var, 0x00582324);
BW_DATA_VAR(UnitDeaths*,						unitCompleteTable_var, 0x00584DE4);
BW_DATA_VAR(UnitDeaths*,						unitKilledTable_var, 0x005878A4);
BW_DATA_VAR(ButtonSet*,						buttonSetTable_var, 0x005187E8);
BW_DATA_VAR(UnitStatFuncArrayEntry*,			unitStatFuncs_var, 0x005193A0);
BW_DATA_VAR(u16*,								g_orders_requirement_tree_var, 0x00514CF8);
BW_DATA_VAR(const LO_Header* const*,			shieldOverlays_var, 0x0052E5C8);
BW_DATA_VAR(u8*,								g_building_overlay_state_max_var, 0x005240D0);
BW_DATA_VAR(GrpHead*							const*, imageGrpGraphics_var, 0x0051CED0);
BW_DATA_VAR(u32*,								ai_strength_air_var, 0x006BB210);
BW_DATA_VAR(u32*,								ai_strength_ground_var, 0x006BB5A0);
BW_DATA_VAR(u8*,								g_is_valid_sprite_var, 0x00629A88);

// bwdata expects pointer references, so make struct pointers to reference
BW_DATA_VAR(AI_Main*,							AIScriptController_var, &AIScriptController_dat);
BW_DATA_VAR(_uavail*,							unit_availability_var, &UnitAvailability_dat);
BW_DATA_VAR(ImagesDatExtraOverlayLO_Files*,	lo_files_var, &lo_files_dat);
BW_DATA_VAR(_bwTechs*,						TechBw_var, &TechBw_dat);
BW_DATA_VAR(_bwUpgrs*,						UpgradesBw_var, &UpgradesBw_dat);

// Define the extern variables
BW_DATA_DEF(UnitDeaths*,						g_unit_deaths_table, deathTable_var);
BW_DATA_DEF(UnitDeaths*,						g_all_unit_count_table, unitAllCountTable_var);
BW_DATA_DEF(UnitDeaths*,						g_completed_unit_count_table, unitCompleteTable_var);
BW_DATA_DEF(UnitDeaths*,						g_unit_killed_table, unitKilledTable_var);
BW_DATA_DEF(ButtonSet*,						g_buttonset_table, buttonSetTable_var);
BW_DATA_DEF(UnitStatFuncArrayEntry*,			unitStatFuncs, unitStatFuncs_var);
BW_DATA_DEF(u16*,								g_orders_requirement_tree, g_orders_requirement_tree_var);
BW_DATA_DEF(const LO_Header* const*,			g_shield_overlays, shieldOverlays_var);
BW_DATA_DEF(u8*,								g_building_overlay_state_max, g_building_overlay_state_max_var);
BW_DATA_DEF(GrpHead* const*,					g_image_grp_graphics, imageGrpGraphics_var);
BW_DATA_DEF(u32*,								aiStrength, ai_strength_air_var);
BW_DATA_DEF(u8*,								g_is_valid_sprite, g_is_valid_sprite_var);

BW_DATA_DEF(AI_Main*,							g_aiscript_controller, AIScriptController_var);
BW_DATA_DEF(_uavail*,							g_unit_availability, unit_availability_var);
BW_DATA_DEF(ImagesDatExtraOverlayLO_Files*,	lo_files, lo_files_var);
BW_DATA_DEF(_bwTechs*,						TechBw, TechBw_var);
BW_DATA_DEF(_bwUpgrs*,						UpgradesBw, UpgradesBw_var);

BW_DATA_DEF(u32*,								ai_strength_air, ai_strength_air_var);
BW_DATA_DEF(u32*,								ai_strength_ground, ai_strength_ground_var);

namespace units_dat {
	BW_DATA(const DatLoad*, unitsDat, 0x00513C30);
	
	BW_DATA_DEF(u16*,			graphic,				unitsDat[0].address);	// 006644F8 -- DatExt changes this to u16 from u8
	BW_DATA_DEF(u16*,			subunit,				unitsDat[1].address);	// 006607C0
	BW_DATA_DEF(u16*,			subunit_2,				unitsDat[2].address);	// 00660C38
	BW_DATA_DEF(u16*,			infested_unit_partial,	unitsDat[3].address);	// 00664980, (Id - UnitId::TerranMinistry) for it to work, last valid id is UnitId::Special_OvermindCocoon
	BW_DATA_DEF(u32*,			construction_graphic,	unitsDat[4].address);	// 006610B0
	BW_DATA_DEF(u8*,			start_direction,		unitsDat[5].address);	// 006605F0  Direction unit will face after it is created. From 0 (top) to 31 (before top turning clockwise).32 means random.
	BW_DATA_DEF(u8*,			shields_enabled,		unitsDat[6].address);	// 006647B0
	BW_DATA_DEF(u16*,			max_shield_points,		unitsDat[7].address);	// 00660E00
	BW_DATA_DEF(s32*,			max_hitpoints,			unitsDat[8].address);	// 00662350
	BW_DATA_DEF(u8*,			elevation,				unitsDat[9].address);	// 00663150
	BW_DATA_DEF(u8*,			movement_flags,			unitsDat[10].address);	// 00660FC8
	BW_DATA_DEF(u8*,			rank,					unitsDat[11].address);	// 00663DD0
	BW_DATA_DEF(u8*,			computer_idle_order,	unitsDat[12].address);	// 00662EA0
	BW_DATA_DEF(u8*,			human_idle_order,		unitsDat[13].address);	// 00662268
	BW_DATA_DEF(u8*,			return_to_idle_order,	unitsDat[14].address);	// 00664898
	BW_DATA_DEF(u8*,			attack_unit_order,		unitsDat[15].address);	// 00663320
	BW_DATA_DEF(u8*,			attack_move_order,		unitsDat[16].address);	// 00663A50
	BW_DATA_DEF(u8*,			ground_weapon,			unitsDat[17].address);	// 006636B8
	BW_DATA_DEF(u8*,			max_ground_hits,		unitsDat[18].address);	// 006645E0
	BW_DATA_DEF(u8*,			air_weapon,				unitsDat[19].address);	// 006616E0
	BW_DATA_DEF(u8*,			max_air_hits,			unitsDat[20].address);	// 0065FC18
	BW_DATA_DEF(u8*,			computer_ai_internal,	unitsDat[21].address);	// 00660178
	BW_DATA_DEF(u32*,			base_property,			unitsDat[22].address);  // 00664080 AKA SpecialAbilityFlags, UnitProperty, etc. Use with UnitProperty::Enum.
	BW_DATA_DEF(u8*,			seek_range,				unitsDat[23].address);  // 00662DB8 AKA Target Acquisition Range, Acquire Target Range, etc.
	BW_DATA_DEF(u8*,			sight_range,			unitsDat[24].address);	// 00663238
	BW_DATA_DEF(u8*,			armor_upgrade,			unitsDat[25].address);	// 006635D0
	BW_DATA_DEF(u8*,			size_type,				unitsDat[26].address);	// 00662180
	BW_DATA_DEF(u8*,			armor_amount,			unitsDat[27].address);	// 0065FEC8
	BW_DATA_DEF(u8*,			right_click_action,		unitsDat[28].address);	// 00662098
	BW_DATA_DEF(u16*,			sound_ready,			unitsDat[29].address);	// 00661FC0
	BW_DATA_DEF(u16*,			sound_what_first,		unitsDat[30].address);	// 0065FFB0
	BW_DATA_DEF(u16*,			sound_what_last,		unitsDat[31].address);	// 00662BF0
	BW_DATA_DEF(u16*,			sound_annoyed_first,	unitsDat[32].address);	// 00663B38
	BW_DATA_DEF(u16*,			sound_annoyed_last,		unitsDat[33].address);	// 00661EE8
	BW_DATA_DEF(u16*,			sound_yes_first,		unitsDat[34].address);	// 00663C10
	BW_DATA_DEF(u16*,			sound_yes_last,			unitsDat[35].address);	// 00661440
	BW_DATA_DEF(Point16*,		building_dimensions,	unitsDat[36].address);	// 00662860
	BW_DATA_DEF(Point16*,		addon_placement,		unitsDat[37].address);	// 006626E0
	BW_DATA_DEF(Box16*,		unit_bounds,			unitsDat[38].address);	// 006617C8
	BW_DATA_DEF(u16*,			portrait_id,			unitsDat[39].address);	// 00662F88
	BW_DATA_DEF(u16*,			mineral_cost,			unitsDat[40].address);	// 00663888
	BW_DATA_DEF(u16*,			vespene_cost,				unitsDat[41].address);	// 0065FD00
	BW_DATA_DEF(u16*,			time_cost,				unitsDat[42].address);	// 00660428
	BW_DATA_DEF(u16*,			requirement_offsets,	unitsDat[43].address);	// 00660A70 //"Restriction Address" according to EUDDB
	BW_DATA_DEF(GroupFlag*,	group_flags,			unitsDat[44].address);	// 006637A0
	BW_DATA_DEF(u8*,			supply_provided,		unitsDat[45].address);  // 006646C8 Supply shown in-game is (true supply) / 2; e.g. Zerglings have 1 true supply, Marines have 2 true supply, etc.
	BW_DATA_DEF(u8*,			supply_required,		unitsDat[46].address);  // 00663CE8 Supply shown in-game is (true supply) / 2; e.g. Zerglings have 1 true supply, Marines have 2 true supply, etc.
	BW_DATA_DEF(u8*,			space_required,			unitsDat[47].address);	// 00664410
	BW_DATA_DEF(u8*,			space_provided,			unitsDat[48].address);	// 00660988
	BW_DATA_DEF(u16*,			build_score,			unitsDat[49].address);	// 00663408
	BW_DATA_DEF(u16*,			destroy_score,			unitsDat[50].address);	// 00663EB8
	BW_DATA_DEF(u16*,			map_string_id,			unitsDat[51].address);	// 00660260
	BW_DATA_DEF(u8*,			broodwar_only,			unitsDat[52].address);	// 006606D8
	BW_DATA_DEF(u16*,			staredit_avail_flags,	unitsDat[53].address);	// 00661518 //"Staredit Availability sound_flags" according to EUDDB
} //units_dat (last entry at 0x00513EAC)

namespace flingy_dat {
	BW_DATA(const DatLoad*, flingyDat, 0x00515A38);
	
	BW_DATA_DEF(u16*, sprite_id,           flingyDat[0].address);	// 006CA318
	BW_DATA_DEF(u32*, top_speed,           flingyDat[1].address);	// 006C9EF8
	BW_DATA_DEF(u16*, acceleration,       flingyDat[2].address);	// 006C9C78
	BW_DATA_DEF(u32*, halt_distance,       flingyDat[3].address);	// 006C9930
	BW_DATA_DEF(u8*,  turn_speed,          flingyDat[4].address);	// 006C9E20 Incorrectly known as "Turn Radius"
//	BW_DATA_DEF(u8*, ???,				flingyDat[5].address);		// 006CA240
	BW_DATA_DEF(u8*,  movement_control,    flingyDat[6].address);	// 006C9858
} //flingy_dat (last entry at 0x00515A80)

// Farty sound extender
/*
namespace sfx_dat {
	BW_DATA(const	DatLoad*,	sfxDat,		0x00515498); // now in bwdata.cpp

	BW_DATA_DEF(u32*, sound_pointer, sfxDat[0].address);		// 0068DAA0
	BW_DATA_DEF(u8*,	priority, sfxDat[1].address);			// 0068D628
	BW_DATA_DEF(u8*,	sound_flags, sfxDat[2].address);		// 0068C8C0
	BW_DATA_DEF(u16*, portrait_duration, sfxDat[3].address);	// 0068CD38
	BW_DATA_DEF(u8*,	volume, sfxDat[4].address);				// 0068C448
}
*/

namespace weapons_dat {
	BW_DATA(const DatLoad*, weaponsDat, 0x00513868);
	
	BW_DATA_DEF(u16*,			label,					weaponsDat[0].address);		// 006572E0
	BW_DATA_DEF(u32*,			flingy_id,				weaponsDat[1].address);		// 00656CA8
	BW_DATA_DEF(u8*,			special_attack,			weaponsDat[2].address);		// 006573E8 //"Special Attack" according to EUDDB
	BW_DATA_DEF(TargetFlag*,	target_flags,			weaponsDat[3].address);		// 00657998
	BW_DATA_DEF(u32*,			min_range,				weaponsDat[4].address);		// 00656A18
	BW_DATA_DEF(u32*,			max_range,				weaponsDat[5].address);		// 00657470
	BW_DATA_DEF(u8*,			damage_upgrade,			weaponsDat[6].address);		// 006571D0
	BW_DATA_DEF(u8*,			damage_type,			weaponsDat[7].address);		// 00657258 Normal, Explosive, etc.. Use with damage_type::Enum.
	BW_DATA_DEF(u8*,			behavior,				weaponsDat[8].address);		// 00656670 Fly and follow target, appear on target unit, etc. Use with WeaponBehavior enum.
	BW_DATA_DEF(u8*,			remove_after,			weaponsDat[9].address);		// 00657040
	BW_DATA_DEF(u8*,			explosion_type,			weaponsDat[10].address);	// 006566F8 NormalHit, RadialSplash, EnemySplash, etc. Use with WeaponEffect::Enum.
	BW_DATA_DEF(u16*,			inner_splash_radius,	weaponsDat[11].address);	// 00656888
	BW_DATA_DEF(u16*,			medium_splash_radius,	weaponsDat[12].address);	// 006570C8
	BW_DATA_DEF(u16*,			outer_splash_radius,	weaponsDat[13].address);	// 00657780
	BW_DATA_DEF(u16*,			damage_amount,			weaponsDat[14].address);	// 00656EB0
	BW_DATA_DEF(u16*,			armor_pen,				weaponsDat[15].address);	// 00657678
	BW_DATA_DEF(u8*,			cooldown,				weaponsDat[16].address);	// 00656FB8
	BW_DATA_DEF(u8*,			damage_factor,			weaponsDat[17].address);	// 006564E0
	BW_DATA_DEF(u8*,			attack_angle,			weaponsDat[18].address);	// 00656990
	BW_DATA_DEF(u8*,			launch_spin,			weaponsDat[19].address);	// 00657888
	BW_DATA_DEF(u8*,			forward_offset,			weaponsDat[20].address);	// 00657910
	BW_DATA_DEF(u8*,			vertical_offset,		weaponsDat[21].address);	// 00656C20
	BW_DATA_DEF(u16*,			error_message,			weaponsDat[22].address);	// 00656568 //Target Error Message according to EUDDB
	BW_DATA_DEF(u16*,			weapon_icon,			weaponsDat[23].address);	// 00656780
} //weapons_dat (last entry at 0x0051397C)

namespace upgrades_dat {
	BW_DATA(const DatLoad*, upgradesDat, 0x005136E0);
	
	BW_DATA_DEF(u16*, mineral_cost_base,		upgradesDat[0].address);	// 00656740
	BW_DATA_DEF(u16*, mineral_cost_factor,	upgradesDat[1].address);	// 006559C0
	BW_DATA_DEF(u16*, vespene_cost_base,			upgradesDat[2].address);	// 00655840
	BW_DATA_DEF(u16*, vespene_cost_factor,		upgradesDat[3].address);	// 006557C0
	BW_DATA_DEF(u16*, time_cost_base,			upgradesDat[4].address);	// 00655B80
	BW_DATA_DEF(u16*, time_cost_factor,		upgradesDat[5].address);	// 00655940
	BW_DATA_DEF(u16*, requirement_offsets,	upgradesDat[6].address);	// 006558C0	//Restriction sound_flags according to EUDDB
	BW_DATA_DEF(u16*, upgrade_icon,			upgradesDat[7].address);	// 00655AC0	//"weapon_icon, cmdicons.grp frame" according to EUDDB
	BW_DATA_DEF(u16*, label,					upgradesDat[8].address);	// 00655A40
	BW_DATA_DEF(u8*,  race,					upgradesDat[9].address);	// 00655BFC
	BW_DATA_DEF(u8*,  max_repeats,			upgradesDat[10].address);	// 00655700
//	BW_DATA_DEF(u8*, ???,						upgradesDat[11].address);	// 00655B3C	//Brood War Flag according to EUDDB
} //upgrades_dat (last entry at 0x00513764)

namespace techdata_dat {
	BW_DATA(const DatLoad*, techdataDat, 0x005137D8);
	
	BW_DATA_DEF(u16*, mineral_cost,			techdataDat[0].address);	// 00656248
	BW_DATA_DEF(u16*, vespene_cost,				techdataDat[1].address);	// 006561F0
	BW_DATA_DEF(u16*, time_cost,				techdataDat[2].address);	// 006563D8
	BW_DATA_DEF(u16*, energy_cost,			techdataDat[3].address);	// 00656380
//	BW_DATA_DEF(u16*, ???,					techdataDat[4].address);	// 00656198
	BW_DATA_DEF(u16*, requirement_offsets,	techdataDat[5].address);	// 006562F8	//Restriction sound_flags according to EUDDB
	BW_DATA_DEF(u16*, tech_icon,				techdataDat[6].address);	// 00656430	//"Icon, cmdicons.grp frame" according to EUDDB
	BW_DATA_DEF(u16*, label,					techdataDat[7].address);	// 006562A0
//	BW_DATA_DEF(u8*,	???,					techdataDat[8].address);	// 00656488	//race according to EUDDB
//	BW_DATA_DEF(u8*,	???,					techdataDat[9].address);	// 00656350	//"Researched" according to EUDDB
//	BW_DATA_DEF(u8*,	???,					techdataDat[10].address);	// 006564B4	//Brood War Flag according to EUDDB
} //techdata_dat (last entry at 0x00513850)

namespace orders_dat {
	BW_DATA(const DatLoad*, ordersDat, 0x00513EC8);
	
	BW_DATA_DEF(u16*,		label,					ordersDat[0].address);	// 00665280
	BW_DATA_DEF(u8*,		use_weapon_targeting,	ordersDat[1].address);	// 00664B00
//	BW_DATA_DEF(u8*,		???,					ordersDat[2].address);	// 00665940 //May be "secondary order" used for debug/assert only / "Order goes in bSecondaryOrderID instead of bMainOrderID." according to EUDDB
	BW_DATA_DEF(u8*,		Unused3,				ordersDat[3].address);	// 00665A00 //May be "Non-subunit" used for debug only
	BW_DATA_DEF(u8*,		Unknown4,				ordersDat[4].address);	// 00664A40
//	BW_DATA_DEF(u8*,		???,					ordersDat[5].address);	// 006657C0 //May be "Subunit can use" used for debug only
	BW_DATA_DEF(b8*,		can_be_interrupted,		ordersDat[6].address);	// 00665040
	BW_DATA_DEF(u8*,		Unknown7,				ordersDat[7].address);	// 00665100
	BW_DATA_DEF(b8*,		can_be_queued,			ordersDat[8].address);	// 00665700
	BW_DATA_DEF(u8*,		disabling_keeps_target,	ordersDat[9].address);	// 006651C0
	BW_DATA_DEF(u8*,		can_be_obstructed,		ordersDat[10].address);	// 006654C0 //"Can Be Obstructed?" according to EUDDB
	BW_DATA_DEF(u8*,		Unknown11,				ordersDat[11].address);	// 00664C80
//	BW_DATA_DEF(u8*,		???,					ordersDat[12].address);	// 00664BC0 //May be "Order requires moving"
	BW_DATA_DEF(u8*,		order_weapon_id,		ordersDat[13].address);	// 00665880
	BW_DATA_DEF(u8*,		tech_used,				ordersDat[14].address);	// 00664E00
	BW_DATA_DEF(u8*,		order_animation,		ordersDat[15].address);	// 00664D40 //Animation (IscriptAnimation) according to EUDDB
	BW_DATA_DEF(s16*,		button_icon,			ordersDat[16].address);	// 00664EC0
	BW_DATA_DEF(u16*,		req_offset,				ordersDat[17].address);	// 00665580 //based on context
	BW_DATA_DEF(u8*,		obscured_order,			ordersDat[18].address);	// 00665400
} //orders_dat (last entry at 0x00513FA0)

namespace sprites_dat {
	BW_DATA(const DatLoad*, spritesDat, 0x00513FB8);
	
	BW_DATA_DEF(u16*,		image_id,					spritesDat[0].address);	// 00666160
	BW_DATA_DEF(s8*,		hp_bar_size,				spritesDat[1].address);	// 00665E50
//	BW_DATA_DEF(s8*,		???,						spritesDat[2].address);	// 00666570
	BW_DATA_DEF(u8*,		is_visible,					spritesDat[3].address);	// 00665C48
	BW_DATA_DEF(s8*,		selection_circle_image,		spritesDat[4].address);	// 00665AC0
	BW_DATA_DEF(s8*,		selection_circle_offset,	spritesDat[5].address);	// 00665FD8
} //sprites_dat (last entry at 0x00513FF4)

namespace images_dat {
	BW_DATA(const DatLoad*, imagesDat, 0x00514010);
	
	BW_DATA_DEF(const u32*, grp_file,			imagesDat[0].address);	// 00668AA0
	BW_DATA_DEF(const u8*,  is_turnable,		imagesDat[1].address);	// 0066E860
	BW_DATA_DEF(const u8*,  is_clickable,		imagesDat[2].address);	// 0066C150
	BW_DATA_DEF(const u8*,  use_full_iscript,	imagesDat[3].address);	// 0066D4D8
	BW_DATA_DEF(const u8*,  draw_if_cloaked,	imagesDat[4].address);	// 00667718
	BW_DATA_DEF(const u8*,  rle_function,		imagesDat[5].address);	// 00669E28
	BW_DATA_DEF(const u8*,  remapping,		imagesDat[6].address);	// 00669A40
	BW_DATA_DEF(const u32*, iscript_id,		imagesDat[7].address);	// 0066EC48
	BW_DATA_DEF(const u32*, lo_shields,		imagesDat[8].address);	// 0066C538
	BW_DATA_DEF(const u32*, lo_attack,		imagesDat[9].address);	// 0066B1B0
	BW_DATA_DEF(const u32*, lo_damage,		imagesDat[10].address);	// 0066A210
	BW_DATA_DEF(const u32*, lo_special,		imagesDat[11].address);	// 00667B00
	BW_DATA_DEF(const u32*, lo_landing,		imagesDat[12].address);	// 00666778
	BW_DATA_DEF(const u32*, lo_liftoff,		imagesDat[13].address);	// 0066D8C0
} //images_dat (last entry at 0x005140AC)

namespace requirements {
	BW_DATA_DEF(u16*, units,		units_req);
	BW_DATA_DEF(u16*, orders,		upgrades_req);
	BW_DATA_DEF(u16*, research,	research_req);
	BW_DATA_DEF(u16*, upgrade,	tech_req);
	BW_DATA_DEF(u16*, tech,		orders_req);
}