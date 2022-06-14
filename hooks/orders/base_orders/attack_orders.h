#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_ReaverAttack1(Unit* unit);		//00465690
	void orders_CarrierAttack1(Unit* unit);	//00465950
	void orders_TurretAttack(Unit* unit);		//00477980
	void orders_AttackFixedRange(Unit* unit);	//00477D00
	void orders_SapUnit(Unit* unit);			//004788E0
	void orders_SapLocation(Unit* unit);		//00478A40
	void orders_AttackMoveEP(Unit* unit);		//00478DE0
	void orders_AttackMove(Unit* unit);		//00479040
	void orders_AttackUnit(Unit* unit);		//00479BD0

	void inject_attack_orders_hooks();

} //hooks
