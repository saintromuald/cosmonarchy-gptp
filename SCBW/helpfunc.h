
//Helpers functions for hooks
//Result of study for functions,
//but is partial.
//Maybe can be injected, but
//this is disabled by default
//and it would require more
//checks to make sure it would
//work in all cases.

#pragma once
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "Unitfinder.h"

namespace helpers {

	void disconnectAddOn_Sub464930(Unit* unit);										//00464930  sub_464930
	void removeUnitRefsFromTarget(Unit* unit, Unit* target);							//0049E590  sub_49E590
	void readUnitsArray_Sub49EFA0(Unit *unit, u8 playerId_param, uint32 unknown_param);	//0049EFA0  sub_49EFA0
	void ordersRescuePassive(Unit* unit);												//004A1EF0  orders_RescuePassive
	void changeUnitButtonSet(Unit *unit, u16 buttonSetId);								//004E5D60  sub_4E5D60 
	void hideAndDisableUnit(Unit *unit);												//004E6340  unitDeathSomething_0
	void showAndEnableUnit(Unit *unit);												//004E6490  initUnitTrapDoodad
	void setNextWaypoint_Sub4EB290(Unit *unit);										//004EB290  sub_4EB290

}

namespace {

//Originally 00464930  sub_464930
void __declspec(naked) disconnectAddOn_Sub464930Wrapper() {

	static Unit* unit;

	__asm {
		PUSHAD
		MOV unit, EAX
	}

	helpers::disconnectAddOn_Sub464930(unit);

	__asm POPAD

}

//Originally 0049E590  sub_49E590
void __declspec(naked) removeUnitRefsFromTargetWrapper() {

	static Unit* unit;
	static Unit* target;

	__asm {

		PUSHAD
		MOV EBP, ESP
		SUB ESP, 0x08

		MOV EAX, [EBP+0x08]

		MOV unit, EAX

		XOR EAX, EAX

		MOV target, ECX

	}

	helpers::removeUnitRefsFromTarget(unit, target);

	__asm {
		POPAD
		RET 0x0004
	}

}


//Originally 0049EFA0  sub_49EFA0
void __declspec(naked) readUnitsArray_Sub49EFA0Wrapper() {

	static Unit* unit;
	static u8 playerId;
	static uint32 unknown_param;

  __asm {

    PUSHAD

	MOV EBP, ESP
	SUB ESP, 0x08

	XOR EAX, EAX
	MOV AL, [EBP+0x08]
	MOV playerId, AL

	MOV EAX, [EBP+0x0C]
	MOV unknown_param, EAX

	MOV unit, ECX

  }

  helpers::readUnitsArray_Sub49EFA0(unit, playerId, unknown_param);

  __asm {
	  POPAD
	  RET 0x0008
  }

}


//Originally 004A1EF0 orders_RescuePassive
void __declspec(naked) ordersRescuePassive() {

	static Unit* unit;

	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unit, EAX
	}

	helpers::ordersRescuePassive(unit);

	__asm {
		POPAD
		RETN
	}


}
//Originally 004E5D60 sub_4E5D60
void __declspec(naked) changeUnitButtonSetWrapper() {

	static Unit* unit;
	static u16 buttonSetId;

	__asm {
		PUSHAD
		MOV unit, EAX
		MOV buttonSetId, CX
	}

	helpers::changeUnitButtonSet(unit, buttonSetId);

	__asm POPAD

}
//Originally 004E6340 unitDeathSomething_0
void __declspec(naked) unitDeathSomething0Wrapper() {

	static Unit* unit;

	__asm {
		PUSHAD
		MOV unit, EAX
	}

	helpers::hideAndDisableUnit(unit);

	__asm POPAD

}


//Originally 004E6490 initUnitTrapDoodad
void __declspec(naked) showAndEnableUnitWrapper() {

	static Unit *unit;

	__asm {
		PUSHAD
		MOV unit, EDI
	}

	helpers::showAndEnableUnit(unit);

    __asm POPAD

}




//Originally 004EB290  sub_4EB290
void __declspec(naked) setNextWaypoint_Sub4EB290Wrapper() {

	static Unit* unit;

	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unit, EAX
	}

	helpers::setNextWaypoint_Sub4EB290(unit);

	__asm {
		POPAD
		RETN
	}


}

} //end of unnamed namespace
