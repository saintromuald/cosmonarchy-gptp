#include "receive_command.h"
#include <SCBW/api.h>

//Helper functions declaration

namespace {

bool32 OrderAllowed(CUnit* unit, uint16 orderId, uint32 nationID);							//0x0046DC20
void function_004756B0(CUnit* unit, uint8 orderId, uint32 unk1, uint32 unk2, uint32 bCommandType);	//0x004756B0
void function_004756E0(CUnit* unit, CUnit* target, uint32 orderId, uint32 bCommandType);		//0x004756E0
uint32 function_0049A410(CUnit* unit, uint8 orderId);											//0x0049A410
bool CanTargetSelf(CUnit* unit, uint8 orderId);											//0x0049A480
void function_0049A500(CUnit* unit,	uint8* array_of_data);									//0x0049A500
void function_0049A8C0(uint8* array_of_data,bool32 bCanBeObstructed);						//0x0049A8C0

} //unnamed namespace

namespace hooks {

	//this function receives the Attack/Attack Move commands and
	//the spells commands, maybe more
	void receive_command(int x, int y, CUnit* unitParam, uint32 wUnitType, uint32 bActionOrder, uint32 bCommandType) {

		uint8 array_of_data[0x30];
		uint16* saved_x = (uint16*)&array_of_data[0];					//[EBP-0x30]
		uint16* saved_y = (uint16*)&array_of_data[2];					//[EBP-0x2E]
		uint32* saved_unknown_1 = (uint32*)&array_of_data[0];			//[EBP-0x30] 
		uint32* saved_unknown_2 = (uint32*)&array_of_data[4];			//[EBP-0x2C] 
		uint32* saved_bCommandType = (uint32*)&array_of_data[0x1C];	//[EBP-0x14]
		uint32* saved_order = (uint32*)&array_of_data[0x20];			//[EBP-0x10]
		uint32* saved_order_2 = (uint32*)&array_of_data[0x24];		//[EBP-0x0C]
		uint32* saved_order_3 = (uint32*)&array_of_data[0x28];		//[EBP-0x08]
		CUnit** saved_unit = (CUnit**)&array_of_data[0x2C];		//[EBP-0x04]

		uint8* switch_value = (uint8*)0x0049ADA0;

		*saved_x = x;
		*saved_y = y;
		*saved_order = bActionOrder;

		function_0049A8C0((uint8*)&array_of_data[0],orders_dat::CanBeObstructed[bActionOrder]);

		if(bActionOrder > OrderId::CTFCOP2 || switch_value[bActionOrder] == 1) {

			CUnit* current_unit;
			uint32 current_order = bActionOrder;	//EBX

			*selectionIndexStart = 0;
			current_unit = getActivePlayerNextSelection();

			*saved_unit = current_unit;

			while(current_unit != nullptr) {

				bool jump_to_9AD7E = false;
				bool bOrderAllowed;

				if(orders_dat::TechUsed[current_order] != TechId::None)
					bOrderAllowed = (current_unit->canUseTech(orders_dat::TechUsed[current_order],*ACTIVE_NATION_ID) == 1);
				else
					bOrderAllowed = (OrderAllowed(current_unit,current_order,*ACTIVE_NATION_ID) == 1);

				if(bOrderAllowed) {

					if(current_unit == unitParam) { //target is the caster/ordered unit
						if(!CanTargetSelf(current_unit,bActionOrder))
							jump_to_9AD7E = true;
					}

				}
				else
					jump_to_9AD7E = true;

				if(!jump_to_9AD7E) { //9ABC4:

					if(function_0049A410(current_unit,bActionOrder) == 0)
						jump_to_9AD7E = true;
					else
					if(	current_unit->mainOrderId != OrderId::NukeLaunch ||
						bActionOrder == OrderId::Die
					) { //9ABE7

						bool jump_to_9ac4e = false;
						bool jump_to_9aca9 = false;
						bool jump_to_9ad09 = false;
						bool jump_to_9ad74 = false;
						bool jump_to_9ad77 = false;
						bool loop_to_9ac0d = true;

						*saved_order_2 = OrderId::Nothing2;
						*saved_order_3 = bActionOrder;

						if(current_unit->id == UnitId::TerranCleric) {

							if(	bActionOrder == OrderId::AttackMove ||
								bActionOrder == OrderId::Attack1
							) { //9AC09

								*saved_order_3 = OrderId::HealMove;

							}
							else
								jump_to_9aca9 = true;

						}
						else
							jump_to_9ac4e = true;

						while(loop_to_9ac0d) {

							if(!jump_to_9ac4e && !jump_to_9aca9) {

								if(unitParam == nullptr) {
									jump_to_9ad09 = true;
									loop_to_9ac0d = false;
								}
								else {

									function_004756E0(current_unit,unitParam,*saved_order_3,bCommandType);

									if(*saved_order_2 == OrderId::Nothing2) {
										jump_to_9ad74 = true;
										loop_to_9ac0d = false;
									}
									else {
										function_004756E0(current_unit->subunit,unitParam,*saved_order_2,bCommandType);
										jump_to_9ad74 = true;
										loop_to_9ac0d = false;
									}

								}

							}

							if(jump_to_9ac4e) {

								jump_to_9ac4e = false;

								if(bActionOrder != OrderId::Attack1)
									jump_to_9aca9 = true;
								else {

									//Attack command conversion

									if(unitParam != nullptr)
										*saved_order_3 = units_dat::AttackUnitOrder[current_unit->id];
									else
										*saved_order_3 = units_dat::AttackMoveOrder[current_unit->id];
									
									if(*saved_order_3 == OrderId::Nothing2) {
										jump_to_9AD7E = true;
										loop_to_9ac0d = false;
									}
									else
									if(current_unit->subunit->isSubunit()) {

										if(unitParam != nullptr) {
											*saved_order_2 = units_dat::AttackUnitOrder[current_unit->subunit->id];
											jump_to_9aca9 = true;
										}
										else {
											*saved_order_2 = units_dat::AttackMoveOrder[current_unit->subunit->id];
											jump_to_9aca9 = true;
										}

									}
									else
										jump_to_9aca9 = true;

								}

							}

							//9aca9:
							if(jump_to_9aca9) {

								jump_to_9aca9 = false;

								if(*saved_order_3 == OrderId::RallyPoint1) {

									CUnit* target = unitParam;

									if(target == nullptr)
										target = current_unit;

									current_unit->rally.unit = target;
									current_unit->rally.pt.x = target->sprite->position.x;
									current_unit->rally.pt.y = target->sprite->position.y;

									jump_to_9ad77 = true;
									loop_to_9ac0d = false;

								}

								//9acdf
								if(*saved_order_2 == OrderId::RallyPoint2) {

									loop_to_9ac0d = false;

									current_unit->rally.unit = nullptr;
									current_unit->rally.pt.x = x;
									current_unit->rally.pt.y = y;

									jump_to_9ad77 = true;

								}

							}

						}

						//9ad09
						if(jump_to_9ad09) {

							jump_to_9ad09 = false;

							*saved_x = x;
							*saved_y = y;

							if(wUnitType == UnitId::None)
								function_0049A500(current_unit,(uint8*)&array_of_data[0]);
							else
								*saved_unknown_2 = *saved_unknown_1;

							function_004756B0(
								current_unit,
								*saved_order_3,
								*saved_unknown_2 / 65536,
								*saved_unknown_2,
								bCommandType
							);

							if(*saved_order_2 == OrderId::Nothing2)
								jump_to_9ad74 = true;
							else {
								function_004756B0(
									current_unit->subunit,
									*saved_order_2,
									*saved_unknown_2 / 65536,
									*saved_unknown_2,
									bCommandType
								);
								jump_to_9ad74 = true;
							}

						}

						//9ad74
						if(jump_to_9ad74 == true) {
							jump_to_9ad74 = false;
							jump_to_9ad77 = true;
						}

						//9ad77
						if(jump_to_9ad77 == true) {
							jump_to_9ad77 = false;
							current_unit->userActionFlags |= 2;
							jump_to_9AD7E = true;
						}
					}
					else
						jump_to_9AD7E = true;

				}

				//9AD7E (use next unit in selection):
				if(jump_to_9AD7E) {
					jump_to_9AD7E = false;
					*saved_unit = getActivePlayerNextSelection();
				}

				//9AB62:
				current_unit = *saved_unit;
				current_order = *saved_order;

			}

		}

	}

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const uint32 Func_OrderAllowed = 0x0046DC20;
bool32 OrderAllowed(CUnit* unit, uint16 orderId, uint32 nationID) {

	static bool32 bResult;

	__asm {
		PUSHAD
		MOV BX, orderId
		MOV EAX, unit
		PUSH nationID
		CALL Func_OrderAllowed
		MOV bResult, EAX
		POPAD
	}

	return bResult;

}

;

const uint32 Func_Sub4756B0 = 0x004756B0;
void function_004756B0(CUnit* unit, uint8 orderId, uint32 unk1, uint32 unk2, uint32 bCommandType) {

	__asm {
		PUSHAD
		MOV DL, orderId
		MOV ESI, unit
		PUSH unk1
		PUSH unk2
		PUSH bCommandType
		CALL Func_Sub4756B0
		POPAD
	}

}

;

const uint32 Func_Sub4756E0 = 0x004756E0;
void function_004756E0(CUnit* unit, CUnit* target, uint32 orderId, uint32 bCommandType) {

	__asm {
		PUSHAD
		MOV EDX, target
		MOV ESI, unit
		PUSH orderId
		PUSH bCommandType
		CALL Func_Sub4756E0
		POPAD
	}

}

;

const uint32 Func_Sub49A410 = 0x0049A410;
uint32 function_0049A410(CUnit* unit, uint8 orderId) {

	static uint32 result; 

	__asm {
		PUSHAD
		MOV EAX, unit
		MOV DL, orderId
		CALL Func_Sub49A410
		MOV result, EAX
		POPAD
	}

	return result;

}

;

//Logically identical to 0049A480 sub_49A480
bool CanTargetSelf(CUnit* unit, uint8 orderId) {

	bool return_value;

	if(
		unit->status & UnitStatus::GroundedBuilding &&
		(orderId == OrderId::RallyPoint1 || orderId == OrderId::RallyPoint2)
	)
		return_value = true;
	else
	if(
		(unit->id == UnitId::ZergIsthrathaleth || unit->id == UnitId::Hero_UncleanOne) &&
		orderId == OrderId::DarkSwarm
	)
		return_value = true;
	else
	if(unit->status & UnitStatus::IsHallucination)
		return_value = false;
	else
	if(
		unit->id == UnitId::ZergOvileth &&
		UpgradesSc->currentLevel[unit->playerId][UpgradeId::VentralSacs] == 0
	)
		return_value = false;
	else
	if(units_dat::SpaceProvided[unit->id] == 0)
		return_value = false;
	else
	if(orderId == OrderId::MoveUnload)
		return_value = true;
	else
		return_value = false;

	return return_value;

}

;

const uint32 Func_Sub49A500 = 0x0049A500;
void function_0049A500(CUnit* unit,	uint8* array_of_data) {

	__asm {
		PUSHAD
		PUSH array_of_data
		PUSH unit
		CALL Func_Sub49A500
		POPAD
	}

}

;

const uint32 Func_Sub49A8C0 = 0x0049A8C0;
void function_0049A8C0(uint8* array_of_data, bool32 bCanBeObstructed) {

	__asm {
		PUSHAD
		PUSH bCanBeObstructed
		MOV EDI, array_of_data
		CALL Func_Sub49A8C0
		POPAD
	}	

}

;

} //unnamed namespace

//End of helper functions
