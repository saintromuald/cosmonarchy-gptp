#include "apply_upgrade_flags.h"
#include <BW/api.h>

namespace hooks
{
	void apply_upgrade_flags_to_new_unit(Unit* unit) // 00454370
	{
		b8 bSpeedUpgrade = 0, bCooldownUpgrade = 0;
		if (unit->id == UnitId::ZergLakizilisk) {
			bSpeedUpgrade = 1;
		}
		if (bSpeedUpgrade == 1) {
			unit->status |= UnitStatus::SpeedUpgrade;
			unit->update_speed();
		}
		unit->update_speed();
	}

	void apply_upgrade_flags_to_existing_units(Unit* unit, u8 upgrade_id) // 00454540
	{
		const u32 UnitUpgradeRestrictionProc = 0x00453DC0;
		const u32 UltraliskUpgradeRestrictionProc = 0x00454070;
		const u32 VultureUpgradeRestrictionProc = 0x00454090;
		bool bCooldownUpgrade = false;	//[EBP-01]
		bool bSpeedUpgrade = true;		//[EBP-02]		
	}
}