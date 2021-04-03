#include "CMDRECV_Build.h"
#include <SCBW/api.h>

//Helper functions declaration

namespace {

bool32 OrderAllowed(CUnit* unit, uint16 order, uint32 nationID);														//6DC20
uint32 function_00473FB0(CUnit* unit, uint8 playerId, int x, int y, uint16 unitId, uint8 unk1, uint8 unk2, uint8 unk3, uint8 unk4 );	//73FB0
bool placeBuildingMsg(uint32 result_function_473FB0);																//8D930
bool CMDRECV_PlaceBuildingAllowedHelper(CUnit* unit, uint8 orderId, uint16 builtUnitId);								//8DBD0
void function_0048DE70(CUnit* unit, uint32 orderId, uint16 dimensionX, uint16 dimensionY, uint32 builtUnitId);				//8DE70
void cmdRECV_PlaceBuilding(int16 x, int16 y, uint8 orderId, uint16 builtUnitId);											//8E190

} //unnamed namespace

namespace hooks {

bool CMDRECV_PlaceBuildingAllowed(CUnit* builder, uint8 orderId, uint16 builtUnitId) {

	bool bReturnValue = false;

	if(orderId >= OrderId::DroneStartBuild && orderId <= OrderId::CTFCOP2) {

		if(
			orderId == OrderId::DroneStartBuild ||
			orderId == OrderId::BuildTerran ||
			orderId == OrderId::BuildProtoss1 ||
			orderId == OrderId::BuildProtoss2 ||
			orderId == OrderId::PlaceAddon
		)
		{
			if(builder->canMakeUnit(builtUnitId,*ACTIVE_NATION_ID))
				bReturnValue = (OrderAllowed(builder,orderId,*ACTIVE_NATION_ID) != 0);
		}
		else
		if(
			orderId == OrderId::Build5 ||		//Build Nydus Exit
			orderId == OrderId::BuildingLand
		)
		{
			if(builder->id == builtUnitId)
				bReturnValue = (OrderAllowed(builder,orderId,*ACTIVE_NATION_ID) != 0);
		}
		else
		if(orderId == OrderId::CTFCOP2) {	//Create flag beacon
			if(*elapsedTimeSeconds <= 600 && builder->id == builtUnitId)
				bReturnValue = (OrderAllowed(builder,orderId,*ACTIVE_NATION_ID) != 0);
		}

	}

	return bReturnValue;

}

;

void cmdRECV_PlaceBuildingNormal(int16 x, int16 y, uint32 orderId, uint32 builtUnitId) {

	CUnit* builder;
	uint32 result_function_00473FB0;

	*selectionIndexStart = 0;
	builder = getActivePlayerNextSelection();

	result_function_00473FB0 = 
		function_00473FB0(
			builder, 
			builder->playerId,
			x,
			y,
			builtUnitId,
			1,
			0,
			0,
			0
		);

	if(placeBuildingMsg(result_function_00473FB0)) {

		int32 DimensionX, DimensionY;

		DimensionX = (int16)units_dat::BuildingDimensions[builtUnitId].x;

		if(DimensionX < 0)
			DimensionX++;

		DimensionX /= 2;

		DimensionX += (x * 32);

		DimensionY = (int16)units_dat::BuildingDimensions[builtUnitId].y;

		if(DimensionY < 0)
			DimensionY++;

		DimensionY /= 2;

		DimensionY += (y * 32);

		function_0048DE70(builder,orderId,DimensionX,DimensionY,builtUnitId);

	}

}

;

void cmdRECV_PlaceBuildingAddon(int16 x, int16 y, uint32 orderId, uint32 builtUnitId) {

	CUnit* builder;
	uint32 result_function_00473FB0;

	uint16 addonPlaceX = 0, addonPlaceY = 0;

	*selectionIndexStart = 0;
	builder = getActivePlayerNextSelection();

	result_function_00473FB0 = 
		function_00473FB0(
			builder, 
			builder->playerId,
			x,
			y,
			builtUnitId,
			1,
			0,
			0,
			0
		);

	if(result_function_00473FB0 == 0) {

		addonPlaceX = (int16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].x;
		/*if (builtUnitId == UnitId::TerranFutureStation) {
			if (builder->id == UnitId::academy || builder->id == UnitId::armory) {
				addonPlaceX = 96;
			}
			else if (builder->id == UnitId::TerranDrydock) {
				addonPlaceX = 96;
			}
		}
		else if (builtUnitId == UnitId::TerranQuarry) {
			addonPlaceX = 128;
		}*/
		scbw::adjustAddonX(builtUnitId, builder->id, addonPlaceX);

		if(addonPlaceX < 0)
			addonPlaceX += 31;

		addonPlaceX /= 32;

		addonPlaceX = x - addonPlaceX;

		addonPlaceY = (int16)units_dat::AddonPlacement[builtUnitId - UnitId::TerranCommandCenter].y;
		/*if (builtUnitId == UnitId::TerranFutureStation) {
			if (builder->id == UnitId::academy || builder->id == UnitId::armory) {
				addonPlaceY = 0;
			}
			else if (builder->id == UnitId::TerranDrydock) {
				addonPlaceY = 48;
			}
		}
		else if (builtUnitId == UnitId::TerranQuarry) {
			addonPlaceY = 32;
		}*/
		scbw::adjustAddonY(builtUnitId, builder->id, addonPlaceY);
		if(addonPlaceY < 0)
			addonPlaceY += 31;

		addonPlaceY /= 32;

		addonPlaceY = y - addonPlaceY;

		result_function_00473FB0 = 
			function_00473FB0(
				builder, 
				builder->playerId,
				addonPlaceX,
				addonPlaceY,
				builder->id,
				0,
				0,
				0,
				0
			);

	}

	if(placeBuildingMsg(result_function_00473FB0)) {

		int32 DimensionX, DimensionY;

		DimensionX = (int16)units_dat::BuildingDimensions[builder->id].x;

		if(DimensionX < 0)
			DimensionX++;

		DimensionX /= 2;

		addonPlaceX *= 32;

		DimensionX += addonPlaceX;

		DimensionY = (int16)units_dat::BuildingDimensions[builder->id].y;

		if(DimensionY < 0)
			DimensionY++;

		DimensionY /= 2;

		addonPlaceY *= 32;

		DimensionY += addonPlaceY;

		function_0048DE70(builder,orderId,DimensionX,DimensionY,builtUnitId);

	}

}

;

void CMDRECV_Build(uint8 orderId, int16 x, int16 y, uint16 builtUnitId) {

	CUnit* builder;
	
	*selectionIndexStart = 0;
	builder = getActivePlayerNextSelection();

	if(builder != nullptr) {

		if(getActivePlayerNextSelection() == nullptr) { //multiple selection not allowed

			if(
				x < mapTileSize->width &&
				y < mapTileSize->height &&
				CMDRECV_PlaceBuildingAllowedHelper(builder, orderId, builtUnitId)
			) 
				cmdRECV_PlaceBuilding(x, y, orderId, builtUnitId);

		}

	}
	
}

;

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const uint32 Func_OrderAllowed = 0x0046DC20;
bool32 OrderAllowed(CUnit* unit, uint16 order, uint32 nationID) {

	static bool32 bResult;

	__asm {
		PUSHAD
		MOV BX, order
		MOV EAX, unit
		PUSH nationID
		CALL Func_OrderAllowed
		MOV bResult, EAX
		POPAD
	}

	return bResult;

}

;

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

const uint32 Func_placeBuildingMsg = 0x0048D930;
bool placeBuildingMsg(uint32 result_function_473FB0) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV EAX, result_function_473FB0
		CALL Func_placeBuildingMsg
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

const uint32 Func_CMDRECV_PlaceBuildingAllowed = 0x0048DBD0;
bool CMDRECV_PlaceBuildingAllowedHelper(CUnit* unit, uint8 orderId, uint16 builtUnitId) {

	static bool32 bPreResult;

	__asm {
		PUSHAD
		MOV AX, builtUnitId
		MOV DL, orderId
		MOV ECX, unit
		CALL Func_CMDRECV_PlaceBuildingAllowed
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

;

const uint32 Func_Sub48DE70 = 0x0048DE70;
void function_0048DE70(CUnit* unit, uint32 orderId, uint16 dimensionX, uint16 dimensionY, uint32 builtUnitId) {

	static Point16 dim;

	dim.x = dimensionX;dim.y = dimensionY;

	__asm {
		PUSHAD
		PUSH builtUnitId
		PUSH dim
		PUSH orderId
		MOV EAX, unit
		CALL Func_Sub48DE70
		POPAD
	}

}

;

const uint32 Func_cmdRECV_PlaceBuilding = 0x0048E190;
void cmdRECV_PlaceBuilding(int16 x, int16 y, uint8 orderId, uint16 builtUnitId) {

	static Point16 pos;

	pos.x = x;pos.y = y;

	__asm {
		PUSHAD
		MOVZX EAX, builtUnitId	//bugfix: should have been "MOV AX, builtUnitId" but for some reason it crashed
		MOV CL, orderId
		PUSH pos
		CALL Func_cmdRECV_PlaceBuilding
		POPAD
	}

}

;

} //unnamed namespace

//End of helper functions