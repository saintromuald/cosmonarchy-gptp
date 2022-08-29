#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_reaver_attack(Unit* unit);		// 00465690
	void orders_carrier_attack(Unit* unit);		// 00465950
	void orders_turret_attack(Unit* unit);		// 00477980
	void orders_attack_fixed_range(Unit* unit);	// 00477D00
	void orders_sap_unit(Unit* unit);			// 004788E0
	void orders_sap_location(Unit* unit);		// 00478A40
	void orders_attack_move_ep(Unit* unit);		// 00478DE0
	void orders_attack_move(Unit* unit);		// 00479040
	void orders_attack_unit(Unit* unit);		// 00479BD0
	void orders_strafe_unit2(Unit* unit);		// 00465E00

	void inject_attack_orders_hooks();
}