#pragma once
#include "../BW/structures/Unit.h"

u32 get_gas_tier(Unit* unit);
u32 get_harvest_amount(Unit* unit);

namespace hooks {
void remove_powerup_image_overlay(Unit* worker);											// 004F3900
void transfer_resource_to_worker(Unit* worker, Unit* resource);								// 004696D0
bool unit_is_geyser_ex(Unit* resource_node, Unit* worker, bool ignore_completed = false);	// 00468930
void order_enter_exit_gas(Unit* unit);														// 00469980
void can_harvest_gas(Unit* unit);															// 00469000
u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker);				// 004557C0
void orders_harvest_finish(Unit* worker);													// 00468ED0
s32 begin_harvest(Unit* worker, Unit* node);												// 00468CAA
Unit* let_next_unit_mine(Unit* worker);														// 00468970
void orders_entries_move_to_harvest_obscured(Unit* worker);									// 00469500
Unit* remove_references_node_worker(Unit* node, Unit* worker);								// 00468EF0
Unit* remove_from_current_harvesters(Unit* worker, Unit* node);								// 004686D0
void remove_harvesters(Unit* node);															// 00468770
void update_mineral_field_image(Unit* unit);												// 00468830
void worker_wandering(Unit* worker, Unit* current_node);									// 00469393 partial
b32 return_cargo_to_best_depot_proc(Unit* depot, Unit* worker);								// 00440FA0
Unit* find_best_return_cargo_target(Unit* worker);											// 00441310
u8 mine_resources(Unit* node, bool is_minerals);											// 00469650
void remove_stunned_worker_from_node(Unit* unit);											// 00468DB0
void orders_harvesting_minerals(Unit* unit);												// 004697C0
void return_resources_from_unit(Unit* unit);												// 004F39A0

void inject_harvest_resource();
}