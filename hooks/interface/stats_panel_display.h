#pragma once
#include "BW/structures.h"
#include "hooks/interface/hud/hud.h"
#include "hooks/interface/hud/hud_manager.h"

u16 __fastcall get_stat_icon_graphic(u8 value);

enum class tooltip_resource {
    Minerals,
    Gas,
    Energy,
    Unused,
    BuildTime,
    PSupply,
    TSupply,
    ZSupply,
};

struct tooltip_resource_cost {
    tooltip_resource resource;
    u16* value;
    GrpFile* grp;
    u16 base_index;
    u32 HudManager::Config::* hud_icon;
};

enum class StatPanel : s16 {
    ForceName = -21,
};

namespace hooks {
void stats_panel_display(BinDlg* dialog);																										// 00426C60
const char* get_unit_rank_string(Unit* unit);																									// 0042F370
void draw_unit_kills_string(BinDlg* dialog);																									// 00425DD0
void draw_building_info(BinDlg* dlg);																											// 00427890
void stats_armor_level(BinDlg* dialog, Unit* unit, u16 status_screen_index);																	// 00425600
void stats_weapon_level(BinDlg* dialog, u16 status_screen_index, WeaponId::Enum weapon_id);																	// 00425790
void stats_shield_level(BinDlg* dialog, u16 status_screen_index);																				// 00425510
b32 stats_spidermine_count(BinDlg* dialog, u16 index);																							// 00426300
const char* get_unit_name(Unit* unit);																											// 0047B5A0
void reset_tooltip_info();																														// 00459360 004593B0 004597D5
void set_tooltip_info(u32 mineral_cost, u32 vespene_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u32 z_supply);	// 00458800
void set_tooltip_info_unit(u16 unit_id);																										// 004594B0 0045952F
void draw_tooltip_with_icon(Box16* area);																										// 00458EF0
s32 stat_data_unit_wireframe_selection(BinDlg* dialog, DialogEvent* event_arg);	                                                                // 004583E0
u16 get_last_queue_slot_id(Unit* unit);																											// 0047B270
void unit_stat_action_transport(BinDlg* dialog);																								// 00427C90
void unit_stat_act_egg(BinDlg* dialog);																											// 00427260
void stats_addon_builder(BinDlg* dialog);																										// 00425F30
void stats_incomplete_unit(BinDlg* dialog);																										// 004260C0
void stats_queue_progress(BinDlg* dialog);																										// 004268D0
void stats_morph_progress(BinDlg* dialog, int index, u32 unk);																					// 00457250
void stats_nuke_silo(BinDlg* dialog);																											// 00426FF0
void unit_stat_act_archon(BinDlg* dialog);																										// 004273E0
void stats_text_energy_timer(BinDlg* dialog);																									// 00425A30
void unit_stat_act_default(BinDlg* dialog);																										// 00425EE0
void unit_stat_act_standard(BinDlg* dialog);																									// 00426F50
void __cdecl draw_status_screen();																												// 00458120
void set_unit_status_string_text(BinDlg* dialog);																								// 00425B50
void draw_resource_amount(BinDlg* dialog);																										// 00426190
void draw_upgrade_progress(BinDlg* dialog);																										// 00426500
void stats_hangar_count(BinDlg* dialog, u32 iterations);																						// 004253D0

void inject_stats_panel_display_hooks();
}