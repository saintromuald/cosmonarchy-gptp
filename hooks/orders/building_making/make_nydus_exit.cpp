#include "make_nydus_exit.h"
#include <SCBW/api.h>

//helper functions def

namespace {

uint32 function_00473FB0(CUnit* unit, uint8 playerId, int x, int y, uint16 unitId, uint8 unk1, uint8 unk2, uint8 unk3, uint8 unk4 );	//73FB0
void function_0047D770(CUnit* unit);																			//7D770
void playWorkerErrorSound(CUnit* unit, uint32 errorTextIndex);														//8F6C0
void displayLastNetErrForPlayer(uint32 playerId);																	//9E530
CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId);														//A09D0
void function_004E65E0(CUnit* unit, bool32 flag);																//E65E0

} //unnamed namespace

namespace hooks {

	void orders_Build5(CUnit* nydus_canal) {

		int x, y;
		uint32 result_f00473FB0;

		x = units_dat::BuildingDimensions[UnitId::ZergNydusCanal].x;
		y = units_dat::BuildingDimensions[UnitId::ZergNydusCanal].y;

		if (x < 0)
			x++;
		if (y < 0)
			y++;

		x = nydus_canal->orderTarget.pt.x - (x / 2);
		y = nydus_canal->orderTarget.pt.y - (y / 2);

		if (x < 0)
			x += 31;
		if (y < 0)
			y += 31;

		x /= 32;
		y /= 32;

		//check if the nydus exit can be placed
		result_f00473FB0 = function_00473FB0(
			nydus_canal,
			nydus_canal->playerId,
			x,
			y,
			UnitId::ZergNydusCanal,
			0, 0, 1, 0
		);

		if (result_f00473FB0 != 0) {

			playWorkerErrorSound(nydus_canal, 0x358); //"You can't build there"

			if (nydus_canal->orderQueueHead != nullptr) {
				nydus_canal->userActionFlags |= 1;
				prepareForNextOrder(nydus_canal);
			}
			else
				if (nydus_canal->pAI != nullptr)
					nydus_canal->orderComputerCL(OrderId::ComputerAI);
				else
					nydus_canal->orderComputerCL(units_dat::ReturnToIdleOrder[nydus_canal->id]);

		}
		else { //5DCDB

			CUnit* nydusExit =
				createUnit(
					UnitId::ZergNydusCanal,
					nydus_canal->orderTarget.pt.x,
					nydus_canal->orderTarget.pt.y,
					nydus_canal->playerId
				);

			if (nydusExit == nullptr) {
				displayLastNetErrForPlayer(nydus_canal->playerId);
				nydus_canal->orderToIdle();
			}
			else {

				function_004E65E0(nydusExit, 1);

				//logically equivalent to a more complex original calculation
				nydusExit->hitPoints = (10 * units_dat::MaxHitPoints[nydusExit->id]) / 100;

				nydusExit->orderComputerCL(OrderId::ZergBuildSelf);
				nydusExit->currentButtonSet = UnitId::Buttons_Blank;
				function_0047D770(nydusExit);
				nydus_canal->building.nydusExit = nydusExit;
				nydusExit->building.nydusExit = nydus_canal;
				nydus_canal->orderToIdle();
			}
		}
	}

	;

	uint32 adjustFlags(uint32 flags, uint32 player) {
		return 0xffffffff;//don't check any creep tile flags for nydus placement
	}
}
;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const uint32 Func_Sub473FB0 = 0x00473FB0;
uint32 function_00473FB0(CUnit* unit, uint8 playerId, int x, int y, uint16 unitId, uint8 unk1, uint8 unk2, uint8 unk3, uint8 unk4 ) {

	uint32 return_value;

	__asm {
		PUSHAD
		MOVZX EAX, unk4		/*28*/
		PUSH EAX		
		MOVZX EAX, unk3		/*24*/
		PUSH EAX		
		MOVZX EAX, unk2		/*20*/
		PUSH EAX		
		MOVZX EAX, unk1		/*1C*/
		PUSH EAX		
		MOVZX EAX, unitId	/*18*/
		PUSH EAX		
		PUSH y				/*14*/
		PUSH x				/*10*/
		MOVZX EAX, playerId	/*0C*/
		PUSH EAX		
		PUSH unit			/*08*/
		CALL Func_Sub473FB0
		MOV return_value, EAX
		POPAD
	}

	return return_value;

}

;

const uint32 Func_Sub47D770 = 0x0047D770;
void function_0047D770(CUnit* unit) {

	__asm {
		PUSHAD
		PUSH unit
		CALL Func_Sub47D770
		POPAD
	}

}

;

const uint32 Func_PlayWorkerErrorSound = 0x0048F6C0;
void playWorkerErrorSound(CUnit* unit, uint32 errorTextIndex) {

	__asm {
		PUSHAD
		MOV EAX, unit
		PUSH errorTextIndex
		CALL Func_PlayWorkerErrorSound
		POPAD
	}

}

;

const uint32 Func_displayLastNetErrForPlayer = 0x0049E530;
void displayLastNetErrForPlayer(uint32 playerId) {
	__asm {
		PUSHAD
		PUSH playerId
		CALL Func_displayLastNetErrForPlayer
		POPAD
	}
}

;

const uint32 Func_CreateUnit = 0x004A09D0;
CUnit* createUnit(uint32 unitId, int x, int y, uint32 playerId) {

	static CUnit* unit_created;

	__asm {
		PUSHAD
		PUSH playerId
		PUSH y
		MOV ECX, unitId
		MOV EAX, x
		CALL Func_CreateUnit
		MOV unit_created, EAX
		POPAD
	}

	return unit_created;

}

;

const uint32 Func_Sub4E65E0 = 0x004E65E0;
void function_004E65E0(CUnit* unit, bool32 flag) {

	__asm {
		PUSHAD
		MOV EDI, unit
		PUSH flag
		CALL Func_Sub4E65E0
		POPAD
	}

}

;

} //Unnamed namespace

//End of helper functions
