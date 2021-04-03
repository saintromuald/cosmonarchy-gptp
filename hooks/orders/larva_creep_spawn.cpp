#include "larva_creep_spawn.h"
#include <SCBW/api.h>

//Helper functions definitions

#include "SCBW\UnitFinder.h"



namespace {

	CUnit* getUnitPointer();

	uint8 spreadCreep(uint32 unitId, int x, int y);									//0x00414680
	void AI_TrainingOverlord(CUnit* main_building, CUnit* larva);				//0x00435770
	void updateUnitStrength(CUnit* unit);										//0x0049FA40
	void function_004A01F0(CUnit* unit);										//0x004A01F0
	CUnit* CreateUnit(uint32 unitId, int x, int y, uint32 playerId);					//0x004A09D0
	uint32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords);//0x004E8280
	void function_004E8DA0(CUnit* unit);										//0x004E8DA0
	bool function_004E8E10(CUnit* unit, Point32* coords);						//0x004E8E10

																				// SC Variables and Buffers used in this file
	uint32* const mapTileBufWidth = (uint32*)(0x006D0F08);
	uint32* const mapTileBufHeight = (uint32*)(0x006D0C6C);
	uint16** const mapTileBuf = (uint16**)(0x006D0E84);
	Point32* const someCreepPointBuf = (Point32*)(0x006D7578);

	typedef uint32(*isTileVisible_func)(uint32 x, uint32 y);
	typedef bool(*setCreepRelationalData_callback)(uint16* tileBackup, uint8* creepOverlayVal, uint32* retval, uint16* tile, uint32 arg);

	isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);


	uint32 getNumSurroundingCreepTiles(uint32 x, uint32 y); // 0x00414290
	void setCreepRelationalData(uint32 x, uint32 y, setCreepRelationalData_callback callback, uint32 arg); // 0x00413FA0
	uint32 iterateDirectionalCreepData(uint32 x, uint32 y, uint32 callback, uint32 arg); // 0x00413B30

	// for applyCreepAtLocationFromUnitType
	int32 modifyCreepState(int32 x_tile, int32 y_tile, int32 (__stdcall* creepChange)(int32, int32, int32, int32, int32), void* param);	// 00413FA0
	int32 runCreepRandomizer(int16* originalTile, int32 creepTileBorders, int32* outCreepStatus, int16* currentTile, int32 unused);	// 004140A0 // "note possible desync"
	int32 unkCreepBorders(int32 a1, int32 a2);																				// 006D0C78
	uint32 forEachAdjacentTile(int32 (__stdcall* callback)(int32, int32, int32, int32, void*), void* param, int32 x_tile, int32 y_tile);	// 00413B30
	int32 unkCreepTile(int32 pos, int32 originalTile, int32 x_tile, int32 y_tile, void* param);									// 00414440
	char unkSpreadCreep(int32 a1, int32 a2, int16 a3);																		// 00414680

} //unnamed namespace

;

namespace hooks {

	setCreepRelationalData_callback runCreepRandomizerPtr = &runCreepRandomizer;

	//probably create the single larva accompanying
	//a building at game beginning
	void function_0049D660(CUnit* main_building) {

		Point32 coords;

		//look for a location to spawn a larva at
		if (function_004E8E10(main_building, &coords)) {

			CUnit* unit_created = CreateUnit(UnitId::ZergLarva, coords.x, coords.y, main_building->playerId);

			if (unit_created != nullptr) {

				//update various stuff (set hp, set shield...) not finished on CreateUnit
				function_004A01F0(unit_created);

				updateUnitStrength(unit_created);

				unit_created->connectedUnit = main_building;

				//set mainOrderState depending on the collision
				//state with the connected building			
				function_004E8DA0(unit_created);

				if (main_building->pAI != nullptr)
					AI_TrainingOverlord(main_building, unit_created);

			} //if(unit_created != nullptr)

		} //if(function_004E8E10(main_building,&coords))

	} //void function_0049D660(CUnit* main_building) 

	;

	//filter for larva in internal game searches, use the limit of 3 larvas
	bool function_004E8C80(CUnit* unit, CUnit* main_building) {

		uint16* const LARVA_COUNTER = (uint16*)0x0067D3F8;

		//while bReturnValue == false, the calling function keep calling,
		//so it return true only when max amount of larvas was counted,
		//since the counting itself is made with LARVA_COUNTER here and
		//not in the calling function.
		bool bReturnValue;

		if (unit->id == UnitId::ZergLarva) {

			if (unit->sprite->flags & CSprite_Flags::Hidden)
				bReturnValue = false;
			else
				if (unit->connectedUnit != main_building)
					bReturnValue = false;
				else {
					(*LARVA_COUNTER)++;
					bReturnValue = (*LARVA_COUNTER > 3);
				}

		}
		else
			bReturnValue = false;

		return bReturnValue;

	}

	;

	void secondaryOrd_SpawningLarva(CUnit* unit) {

		uint16* const LARVA_COUNTER = (uint16*)0x0067D3F8;
		
		if (unit->orderQueueTimer == 0 && unit->playerId != 11) {

			//EA79E

			Box16 coords;

			*LARVA_COUNTER = 0;

			coords.left = unit->sprite->position.x - 256;
			coords.top = unit->sprite->position.y - 256;
			coords.right = unit->sprite->position.x + 256;
			coords.bottom = unit->sprite->position.y + 256;

			//Count existing larvas in LARVA_COUNTER
			IterateUnitsAtLocationTargetProc_LarvaCount(unit, &coords);

			if (*LARVA_COUNTER < 3 && !(unit->status & UnitStatus::InAir)) {

				if (unit->building.larvaTimer != 0)
					unit->building.larvaTimer--;
				else {

					Point32 coords_2;
					//EA80E:
					unit->building.larvaTimer = 37; 
					/*0x25*/

					if (unit->id == UnitId::ZergLarvalColony) {
						unit->building.larvaTimer = 37 / LarvalColonyMultiplier;
					}

													//if failed to find a spawn point for larva
													//set the timer before retry to 3
					if (unit->id == UnitId::ZergLair && unit->mainOrderId != OrderId::Morph2 || 
						unit->id == UnitId::ZergHive && unit->mainOrderId == OrderId::Morph2) {
						unit->building.larvaTimer = 27;
					}
					else if (unit->id == UnitId::ZergHive && unit->mainOrderId != OrderId::Morph2 ||
							 unit->id == UnitId::ZergSire && unit->mainOrderId == OrderId::Morph2) {
						unit->building.larvaTimer = 18;
					}
					else if (unit->id == UnitId::ZergSire && unit->mainOrderId != OrderId::Morph2) {
						unit->building.larvaTimer = 12;
					}

					if (!function_004E8E10(unit, &coords_2))
						unit->building.larvaTimer = 3;
					else {

						//EA830:
						CUnit* new_larva = CreateUnit(UnitId::ZergLarva, coords_2.x, coords_2.y, unit->playerId);

						if (new_larva != nullptr) {

							//update various stuff (set hp, set shield...) not finished on CreateUnit
							function_004A01F0(new_larva);

							updateUnitStrength(new_larva);

							new_larva->connectedUnit = unit;

							//set mainOrderState depending on the collision
							//state with the connected building						
							function_004E8DA0(new_larva);

							if (unit->pAI != nullptr)
								AI_TrainingOverlord(unit, new_larva);

						} //if(new_larva != nullptr)

					} //if(!function_004E8E10(unit,&coords_2))

				} //if(unit->building.larvaTimer == 0)

			} //if(*LARVA_COUNTER < 3)

		} //if(unit->orderQueueTimer == 0 && unit->playerId != 11)

	} //void secondaryOrd_SpawningLarva(CUnit* unit)

	;

	void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit) {

		
		if (unit->id == UnitId::ZergHatchery
		||  unit->id == UnitId::ZergLair
		||  unit->id == UnitId::ZergHive
		||  unit->id == UnitId::ZergSire
		|| (unit->id == UnitId::ZergInfestedCommandCenter && !(unit->status & UnitStatus::InAir))
		||	unit->id == UnitId::ZergLarvalColony) {
			secondaryOrd_SpawningLarva(unit);
		}
		if (unit->building.creepTimer != 0) {
			if (unit->building.creepTimer >= 2) {
				unit->building.creepTimer -= 2;
			}
			else {
				unit->building.creepTimer = 0;
			}
		}
		else {
			//EA8B6:
			uint8 result_00414680;

			unit->building.creepTimer = 15; /*0F*/
			if (unit->id == 180) {
				//scbw::printText("spread basilisk creep");
				unit->building.creepTimer = 10;
			}

			//look for a place to spawn creep on
			//return value == 0 if all potential tiles are already creep, 2 == potential tiles are occupied by another building, 1 == potential tiles are otherwise still available
			// Replace UnitId::ZergHive with unit->id if giving specific buildings different creep bounding rectangles in getCreepRegion
			result_00414680 = spreadCreep(unit->id, unit->sprite->position.x, unit->sprite->position.y);
			if (unit->id == UnitId::ZergInfestedCommandCenter) {
				result_00414680 = 0;
			}
			if (unit->id == 180 && result_00414680 != 0) {
				//scbw::printText("Check the creep spread...");
				uint32 current_tile_x = unit->sprite->position.x / 32;
				uint32 current_tile_y = unit->sprite->position.y / 32;
				int width_x = 6;
				int width_y = 4;
				uint8 mustspread = 0;
				int x1 = current_tile_x-width_x;
				int y1 = current_tile_y-width_x;
				int x2 = current_tile_x+width_x;
				int y2 = current_tile_y+width_y;

				if(x1<0){
					x1=0;
				}
				if(y1<0){
					y1=0;
				}
				if (x2 >= mapTileSize->width) {
					x2 = mapTileSize->width - 1;
				}
				if (y2 >= mapTileSize->height) {
					y2 = mapTileSize->height - 1;
				}
				for (int i = x1; i <= x2; i++) {
					for (int j = y1; j <= y2; j++) {						
						int x = (i * 32) + 16;
						int y = (j * 32) + 16;
						int X = (x-(current_tile_x*32));
						int Y = (y-(current_tile_y*32));
						bool in_range = ((256 * X*X) + (100 * Y*Y)) < 1300000;
						//adjust value 1300000 to get slightly different radius

						if (in_range) {
							ActiveTile a = scbw::getActiveTileAt(i*32, j*32);
							
							if (a.hasCreep || a.temporaryCreep || a.creepReceeding) {
								//
							}
							else{
								//can spread
								if (!a.alwaysUnbuildable && a.isWalkable ) {	
														
									mustspread = 1;
								}
							}
						}
						
					}
				}
				char buf[64];
				sprintf(buf,"Limits: %d %d %d %d, status: %d",x1,y1,x2,y2,mustspread);
				//scbw::printText(buf);	
				if (!mustspread) {
					result_00414680 = 0;
				}
			}
			if (result_00414680 == 0) {

				//EA8DC:
				uint8 orderId;

				bool isZergMainBuilding =
					unit->id == UnitId::ZergHatchery ||
					unit->id == UnitId::ZergLair ||
					unit->id == UnitId::ZergHive ||
					unit->id == UnitId::ZergSire ||
					(unit->id==UnitId::ZergInfestedCommandCenter && !(unit->status & UnitStatus::InAir)) ||
					unit->id == UnitId::ZergLarvalColony;

				if (isZergMainBuilding)
					orderId = OrderId::SpawningLarva;
				else
					orderId = OrderId::Nothing2;

				if (unit->secondaryOrderId != orderId) {
					unit->secondaryOrderId = orderId;
					unit->secondaryOrderPos.y = 0;
					unit->secondaryOrderPos.x = 0;
					unit->currentBuildUnit = nullptr;
					unit->secondaryOrderState = 0;
				}

			}

		}

	} //void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit)

	;


	// Returns the bounding rectangle (in tiles) of the maximum extent of creep around a source or the building footprint of a non-source creep building.
	// Return value is if the building is a creep source (spreads creep) or not
	bool getCreepRegion(Box32* rect, uint16 unitId, int32 x, int32 y, bool defRetVal) {

		// Default maximum bounding size
		const static int32 maxCreepAreaLeft = 320; // 10 tiles
		const static int32 maxCreepAreaRight = 320; // 10 tiles
		const static int32 maxCreepAreaUp = 200; // 6.25 tiles
		const static int32 maxCreepAreaDown = 200; // 6.25 tiles

		if (spreadsCreep(unitId, defRetVal)) {
			rect->left = (x - maxCreepAreaLeft) / 32;
			rect->right = (x + maxCreepAreaRight) / 32;
			rect->top = (y - maxCreepAreaUp) / 32;
			rect->bottom = (y + maxCreepAreaDown) / 32;
			if (rect->left < 0) {
				rect->left = 0;
			}
			else if (rect->right >= *mapTileBufWidth) {
				rect->right = *mapTileBufWidth - 1;
			}
			if (rect->top < 0) {
				rect->top = 0;
			}
			else if (rect->bottom >= *mapTileBufHeight) {
				rect->bottom = *mapTileBufHeight - 1;
			}
			return true;
		}
		int32 w = units_dat::BuildingDimensions[unitId].x / 32;
		int32 h = units_dat::BuildingDimensions[unitId].y / 32;
		rect->left = x / 32 - w / 2;
		rect->top = y / 32 - h / 2;
		rect->right = rect->left + w - 1;
		rect->bottom = rect->top + h - 1;
		if (rect->left < 0) {
			rect->left = 0;
		}
		else if (rect->right >= *mapTileBufWidth) {
			rect->right = *mapTileBufWidth - 1;
		}
		if (rect->top < 0) {
			rect->top = 0;
		}
		else if (rect->bottom >= *mapTileBufHeight) {
			rect->bottom = *mapTileBufHeight - 1;
		}
		return false;
	}


	// Determines which unit IDs are a creep source (i.e. spreads creap, not just requires it)
	// Base buildings built by a drone return the value of "isComplete", which is an argument that is true if the unit status flag has "Completed" set -- the building only spawns creep once it has been built
	bool spreadsCreep(uint16 unitId, bool isComplete) {
		

		switch (unitId) {
		case UnitId::ZergHatchery:
		case UnitId::ZergCreepColony:
		case UnitId::ZergInfestedCommandCenter:
			return isComplete;

		case UnitId::ZergSire:
		case UnitId::ZergHive:
		case UnitId::ZergLair:
		case UnitId::ZergSporeColony:
		case UnitId::ZergSunkenColony:

		case UnitId::BasiliskCreeper:
			return true;

		default:
			return false;
		}
	}


	// x and y are relative positions where (0,0) is presumably the center of the building
	// !!! WARNING !!! Because of the super fun getUnitPointer call in this function, you probably never want to call this function directly
	bool inCreepArea(int32 x, int32 y) {
		// Default function:
		CUnit* unit = getUnitPointer();

		return (x*x * 100 + y*y * 256) <= 10240000;

		// Per-unit function example
/*		CUnit* unit = getUnitPointer();
		*/
		/*
		switch (deathTable[214].player[11]) {
		case 180:
			return (x*x * 100 + y*y * 256) <= 2560000;
		case UnitId::ZergCreepColony:
		case UnitId::ZergSporeColony:
		case UnitId::ZergSunkenColony:
			return (x*x * 100 + y*y * 256) <= 10240000;

		default: // hatchery/hive/lair
			return (x*x*100 + y*y*256) <= 10240000;
		}
		*/
	}


	// Callback to determine the creep tile that will replace the existing tile.
	bool runCreepRandomizer(uint16* tileBackup, uint8* creepOverlayVal, uint32* retval, uint16* tile, uint32 arg) {
		// Stores the existing tile to replace the creep tile when the creep is removed

		if (*tileBackup == 0) {
			*tileBackup = *tile;
		}

		uint16 creepTile;

		// use of std::rand() can cause desyncs
		if ((std::rand() % 100) >= 4) {
			creepTile = std::rand() % 6; // normal tiles
		}
		else {
			creepTile = (std::rand() % 7) + 6; // clutter tiles
		}
		*tile = creepTile + 0x10; // tile in creep group
		*retval = 0;
		return true;
	}

	/*
	char applyCreepAtLocationFromUnitType(int16 a1, int32 a2, int32 a3) { // 004148F0
		uint32 v3; // ecx
		uint32 v4; // ebx
		int32 v5; // edi
		uint32 v6; // esi
		int32 v7; // eax
		uint32 v8; // eax
		int32 v9; // edi
		char result; // al
		uint32 v11; // [esp+Ch] [ebp-10h]
		int32 v12; // [esp+10h] [ebp-Ch]
		uint32 v13; // [esp+14h] [ebp-8h]
		int32 v14; // [esp+18h] [ebp-4h]

		v7 = units_dat::BuildingDimensions[a1];
		v5 = (int16)v7 / 32;
		v4 = a2 / 32 - v5 / 2;
		v6 = a3 / 32 - (v7 >> 16) / 32 / 2;
		v3 = v6 + (v7 >> 16) / 32;
		v12 = a2 / 32 - v5 / 2;
		v11 = v3;
		if (v6 < v3) {
			v8 = v5 + v4;
			v13 = v5 + v4;
			do {
				v9 = v4;
				v14 = v4;
				if (v4 < v8) {
					do {
						if (!(*activeTileArray[(int16)v9 + mapTileSize->width * (int16)v6] & 0x20800000)
							&& ((int16)v6 + 1 == mapTileSize->height
								|| !(*activeTileArray[(int16)v9 + mapTileSize->width * (int16)(v6 + 1)] & 0x800000)))
						{
							modifyCreepState(
								v9,
								v6,
								(int32(__stdcall*)(int32, int32, int32, int32, int32))runCreepRandomizer,
								0);
							if (!(*(uint32*)0x006D0C78) || !unkCreepBorders(v9, v6))
							{
								forEachAdjacentTile(unkCreepTile, 0, v9, v6);
								v9 = v14;
							}
						}
						v8 = v13;
						v14 = ++v9;
					} while (v9 < v13);
					v4 = v12;
					v3 = v11;
				}
				++v6;
			} while (v6 < v3);
		}
		do
			result = unkSpreadCreep(a3, a2, a1);
		while (result == 1);
		return result;
	}*/

} //hooks

  //-------- Helper function definitions. Do NOT modify! --------//

namespace {

	/* !!! WARNING !!! Use EXTREME caution when calling this. 
	There is absolutely no guarantee that it will find the intended unit, 
	or that it will find a unit at all */
	CUnit* getUnitPointer() {
		static CUnit* unit = nullptr;
		__asm {
			XOR EAX, EAX
			SuperFunUnitFinder :
			MOV EDX, DWORD PTR[EBP + EAX * 4]
				INC EAX
				CMP EDX, 0x0059CCA8
				JB SuperFunUnitFinder
				CMP EDX, 0x006283E8
				JNB SuperFunUnitFinder
				MOV unit, EDX
		}
		return unit;
	}


	const uint32 Func_Sub414680 = 0x00414680;
	uint8 spreadCreep(uint32 unitId, int x, int y) {

		static uint8 return_value;

		__asm {
			PUSHAD
			MOV ECX, x
			MOV EAX, y
			MOV EDX, unitId
			CALL Func_Sub414680
			MOV return_value, AL
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_AI_TrainingOverlord = 0x00435770;
	void AI_TrainingOverlord(CUnit* main_building, CUnit* larva) {

		__asm {
			PUSHAD
			MOV EAX, larva
			MOV ECX, main_building
			CALL Func_AI_TrainingOverlord
			POPAD
		}

	}

	;

	const uint32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}

	}

	;

	const uint32 Func_Sub4A01F0 = 0x004A01F0;
	void function_004A01F0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const uint32 Func_CreateUnit = 0x004A09D0;
	CUnit* CreateUnit(uint32 unitId, int x, int y, uint32 playerId) {

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

	const uint32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
	//hardcoding the larva count function, keeping the return value
	//though it's unused here
	uint32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords) {

		static uint32 return_value;

		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, coords
			MOV EBX, 0x004E8C80
			CALL Func_IterateUnitsAtLocationTargetProc
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_Sub4E8DA0 = 0x004E8DA0;
	void function_004E8DA0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub4E8DA0
			POPAD
		}

	}

	;

	const uint32 Func_Sub4E8E10 = 0x004E8E10;
	bool function_004E8E10(CUnit* unit, Point32* coords) {

		static bool32 return_pre_value;

		__asm {
			PUSHAD
			MOV EDI, coords
			MOV EAX, unit
			CALL Func_Sub4E8E10
			MOV return_pre_value, EAX
			POPAD
		}

		return (return_pre_value != 0);

	}

	;

	const uint32 Func_getNumSurroundingCreepTiles = 0x00414290;
	uint32 getNumSurroundingCreepTiles(uint32 x, uint32 y) {
		static uint32 return_val;

		__asm {
			PUSHAD
			PUSH y
			PUSH x
			CALL Func_getNumSurroundingCreepTiles
			MOV return_val, EAX
			POPAD
		}

		return return_val;
	}

	const uint32 Func_setCreepRelationalData = 0x00413FA0;
	void setCreepRelationalData(uint32 x, uint32 y, setCreepRelationalData_callback callback, uint32 arg) {
		__asm {
			PUSHAD
			PUSH arg
			PUSH callback
			PUSH y
			PUSH x
			CALL Func_setCreepRelationalData
			POPAD
		}
	}

	const uint32 Func_iterateDirectionalCreepData = 0x00413B30;
	uint32 iterateDirectionalCreepData(uint32 x, uint32 y, uint32 callback, uint32 arg) {
		static uint32 return_val;

		__asm {
			PUSHAD
			PUSH y
			PUSH x
			MOV EBX, callback
			MOV EDI, arg
			CALL Func_iterateDirectionalCreepData
			MOV return_val, EAX
			POPAD
		}

		return return_val;
	}

} //Unnamed namespace

  //End of helper functions
