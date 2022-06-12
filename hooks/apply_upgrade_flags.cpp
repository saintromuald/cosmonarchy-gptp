//Source file for the Apply Upgrade Flags hook module.
//This file is directly responsible for applying movement speed / attack speed
//upgrades to units.
#include "apply_upgrade_flags.h"
#include <SCBW/api.h>

//helper functions def
namespace
{

	bool upgradeRestrictionProc(Unit* unit, u8 upgradeId, u32 Func_UpgradeRestrictionProc);

} //unnamed namespace

namespace hooks
{

	//This hook function is called when creating a new unit.
	//Equivalent to apply_speed_upgrade_from_unit_type @ 00454370
	void applyUpgradeFlagsToNewUnitHook(Unit* unit)
	{

		b8 bSpeedUpgrade = 0, bCooldownUpgrade = 0;

		/*	if (unit->id == UnitId::mine_carrier) {
				if(scbw::get_upgrade_level(unit->player,UpgradeId::IonPropulsion) && unit->sprite!=nullptr){
					unit->sprite->elevation_level = 12;
				}
			}*/

		if (unit->id == UnitId::ZergLakizilisk)
			bSpeedUpgrade = 1;

		if (bSpeedUpgrade == 1)
		{
			unit->status |= UnitStatus::SpeedUpgrade;
			unit->update_speed();
		}

		unit->update_speed();

	}

	//This function is called when an upgrade is finished, or when transferring a
	//unit's ownership from one player to another (via triggers or Mind Control).
	//Should be equivalent or identical to apply_speed_upgrade_from_upgrade_id @ 00454540
	void applyUpgradeFlagsToExistingUnitsHook(Unit* unit, u8 upgradeId)
	{

		const u32 UnitUpgradeRestrictionProc = 0x00453DC0;
		const u32 UltraliskUpgradeRestrictionProc = 0x00454070;
		const u32 VultureUpgradeRestrictionProc = 0x00454090;

		bool bCooldownUpgrade = false;	//[EBP-01]
		bool bSpeedUpgrade = true;		//[EBP-02]

		/*if (unit->id == UnitId::TerranVulture) {
			if (scbw::get_upgrade_level(unit->player, UpgradeId::IonPropulsion) && unit->sprite != nullptr) {
				unit->sprite->elevation_level = 12;
			}
		}*/
	}

} //hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace
{

	bool upgradeRestrictionProc(Unit* unit, u8 upgradeId, u32 Func_UpgradeRestrictionProc)
	{
		static u32 bPreResult;
		__asm {
			PUSHAD
			MOVZX EDX, upgradeId
			MOV ECX, unit
			CALL Func_UpgradeRestrictionProc
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

} //Unnamed namespace

//End of helper functions
