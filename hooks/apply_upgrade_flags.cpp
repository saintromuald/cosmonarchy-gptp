//Source file for the Apply Upgrade Flags hook module.
//This file is directly responsible for applying movement speed / attack speed
//upgrades to units.
#include "apply_upgrade_flags.h"
#include <SCBW/api.h>

//helper functions def
namespace {

	bool upgradeRestrictionProc(CUnit* unit, uint8 upgradeId, uint32 Func_UpgradeRestrictionProc);

} //unnamed namespace

namespace hooks {

	//This hook function is called when creating a new unit.
	//Equivalent to ApplySpeedUpgradeFromUnitType @ 00454370
	void applyUpgradeFlagsToNewUnitHook(CUnit* unit) {

		bool8 bSpeedUpgrade = 0, bCooldownUpgrade = 0;

		/*	if (unit->id == UnitId::vulture) {
				if(scbw::getUpgradeLevel(unit->playerId,UpgradeId::IonPropulsion) && unit->sprite!=nullptr){
					unit->sprite->elevationLevel = 12;
				}
			}*/

		if (unit->id == UnitId::ZergLakizilisk)
			bSpeedUpgrade = 1;

		if (bSpeedUpgrade == 1) {
			unit->status |= UnitStatus::SpeedUpgrade;
			unit->updateSpeed();
		}

		unit->updateSpeed();

	}

	//This function is called when an upgrade is finished, or when transferring a
	//unit's ownership from one player to another (via triggers or Mind Control).
	//Should be equivalent or identical to ApplySpeedUpgradeFromUpgradeType @ 00454540
	void applyUpgradeFlagsToExistingUnitsHook(CUnit* unit, uint8 upgradeId) {

		const uint32 UnitUpgradeRestrictionProc = 0x00453DC0;
		const uint32 UltraliskUpgradeRestrictionProc = 0x00454070;
		const uint32 VultureUpgradeRestrictionProc = 0x00454090;

		bool bCooldownUpgrade = false;	//[EBP-01]
		bool bSpeedUpgrade = true;		//[EBP-02]

		/*if (unit->id == UnitId::TerranVulture) {
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::IonPropulsion) && unit->sprite != nullptr) {
				unit->sprite->elevationLevel = 12;
			}
		}*/
	}

} //hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	bool upgradeRestrictionProc(CUnit* unit, uint8 upgradeId, uint32 Func_UpgradeRestrictionProc) {
		static uint32 bPreResult;
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
