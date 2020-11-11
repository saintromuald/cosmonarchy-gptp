#include "larva_creep_spawn.h"
#include <SCBW/api.h>

//Helper functions definitions

#include "SCBW\UnitFinder.h"



namespace {

	CUnit* getUnitPointer();

	u8 spreadCreep(u32 unitId, int x, int y);								//0x00414680
	void AI_TrainingOverlord(CUnit* main_building, CUnit* larva);				//0x00435770
	void updateUnitStrength(CUnit* unit);										//0x0049FA40
	void function_004A01F0(CUnit* unit);										//0x004A01F0
	CUnit* CreateUnit(u32 unitId, int x, int y, u32 playerId);					//0x004A09D0
	u32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords);//0x004E8280
	void function_004E8DA0(CUnit* unit);										//0x004E8DA0
	bool function_004E8E10(CUnit* unit, Point32* coords);						//0x004E8E10

																				// SC Variables and Buffers used in this file
	u32* const mapTileBufWidth = (u32*)(0x006D0F08);
	u32* const mapTileBufHeight = (u32*)(0x006D0C6C);
	u16** const mapTileBuf = (u16**)(0x006D0E84);
	Point32* const someCreepPointBuf = (Point32*)(0x006D7578);

	typedef u32(*isTileVisible_func)(u32 x, u32 y);
	typedef bool(*setCreepRelationalData_callback)(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);

	isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);


	u32 getNumSurroundingCreepTiles(u32 x, u32 y); // 0x00414290
	void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg); // 0x00413FA0
	u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg); // 0x00413B30

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

			if (unit_created != NULL) {

				//update various stuff (set hp, set shield...) not finished on CreateUnit
				function_004A01F0(unit_created);

				updateUnitStrength(unit_created);

				unit_created->connectedUnit = main_building;

				//set mainOrderState depending on the collision
				//state with the connected building			
				function_004E8DA0(unit_created);

				if (main_building->pAI != NULL)
					AI_TrainingOverlord(main_building, unit_created);

			} //if(unit_created != NULL)

		} //if(function_004E8E10(main_building,&coords))

	} //void function_0049D660(CUnit* main_building) 

	;

	//filter for larva in internal game searches, use the limit of 3 larvas
	bool function_004E8C80(CUnit* unit, CUnit* main_building) {

		u16* const LARVA_COUNTER = (u16*)0x0067D3F8;

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

		u16* const LARVA_COUNTER = (u16*)0x0067D3F8;
		
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

													//if failed to find a spawn point for larva
													//set the timer before retry to 3
					if ((unit->id == UnitId::ZergLair && unit->mainOrderId!=OrderId::Morph2) || 
						(unit->id == UnitId::ZergHive && unit->mainOrderId == OrderId::Morph2)) {
						unit->building.larvaTimer = 27;
					}
					else if (unit->id == UnitId::ZergHive && unit->mainOrderId!=OrderId::Morph2 ||
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

						if (new_larva != NULL) {

							//update various stuff (set hp, set shield...) not finished on CreateUnit
							function_004A01F0(new_larva);

							updateUnitStrength(new_larva);

							new_larva->connectedUnit = unit;

							//set mainOrderState depending on the collision
							//state with the connected building						
							function_004E8DA0(new_larva);

							if (unit->pAI != NULL)
								AI_TrainingOverlord(unit, new_larva);

						} //if(new_larva != NULL)

					} //if(!function_004E8E10(unit,&coords_2))

				} //if(unit->building.larvaTimer == 0)

			} //if(*LARVA_COUNTER < 3)

		} //if(unit->orderQueueTimer == 0 && unit->playerId != 11)

	} //void secondaryOrd_SpawningLarva(CUnit* unit)

	;

	void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit) {

		
		if (
			unit->id == UnitId::ZergHatchery ||
			unit->id == UnitId::ZergLair ||
			unit->id == UnitId::ZergHive ||
			unit->id == UnitId::ZergSire ||
			(unit->id == UnitId::ZergInfestedCommandCenter && !(unit->status & UnitStatus::InAir))
			)
			secondaryOrd_SpawningLarva(unit);
		if (unit->building.creepTimer != 0)
			unit->building.creepTimer--;
		else {

			//EA8B6:

			u8 result_00414680;

			unit->building.creepTimer = 15; /*0F*/
			if (unit->id == 180) {
				//scbw::printText("spread basilisk creep");
				unit->building.creepTimer = 10;
			}/*
			u32 lair_count = unitCountTable[UnitId::ZergLair].player[unit->playerId];
			u32 hive_count = unitCountTable[UnitId::ZergHive].player[unit->playerId];
			
			if (hive_count) {
				unit->building.creepTimer -= unit->building.creepTimer / 2;
			}
			else if (lair_count) {
				unit->building.creepTimer -= unit->building.creepTimer / 4;
			}*/
											//look for a place to spawn creep on
											//return value == 0 if all potential tiles are already creep, 2 == potential tiles are occupied by another building, 1 == potential tiles are otherwise still available
											// Replace UnitId::ZergHive with unit->id if giving specific buildings different creep bounding rectangles in getCreepRegion
			result_00414680 = spreadCreep(unit->id, unit->sprite->position.x, unit->sprite->position.y);
			if (unit->id == UnitId::infested_command_center) {
				result_00414680 = 0;
			}
			if (unit->id == 180 && result_00414680 != 0) {
				//scbw::printText("Check the creep spread...");
				u32 current_tile_x = unit->sprite->position.x / 32;
				u32 current_tile_y = unit->sprite->position.y / 32;
				int width_x = 6;
				int width_y = 4;
				u8 mustspread = 0;
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
				u8 orderId;

				bool isZergMainBuilding =
					unit->id == UnitId::ZergHatchery ||
					unit->id == UnitId::ZergLair ||
					unit->id == UnitId::ZergHive ||
					unit->id == UnitId::ZergSire ||
					(unit->id==UnitId::ZergInfestedCommandCenter && 
					!(unit->status & UnitStatus::InAir));

				if (isZergMainBuilding)
					orderId = OrderId::SpawningLarva;
				else
					orderId = OrderId::Nothing2;

				if (unit->secondaryOrderId != orderId) {
					unit->secondaryOrderId = orderId;
					unit->secondaryOrderPos.y = 0;
					unit->secondaryOrderPos.x = 0;
					unit->currentBuildUnit = NULL;
					unit->secondaryOrderState = 0;
				}

			}

		}

	} //void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit)

	;


	// Returns the bounding rectangle (in tiles) of the maximum extent of creep around a source or the building footprint of a non-source creep building.
	// Return value is if the building is a creep source (spreads creep) or not
	bool getCreepRegion(Box32* rect, u16 unitId, s32 x, s32 y, bool defRetVal) {

		// Default maximum bounding size
		const static s32 maxCreepAreaLeft = 320; // 10 tiles
		const static s32 maxCreepAreaRight = 320; // 10 tiles
		const static s32 maxCreepAreaUp = 200; // 6.25 tiles
		const static s32 maxCreepAreaDown = 200; // 6.25 tiles

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
		s32 w = units_dat::BuildingDimensions[unitId].x / 32;
		s32 h = units_dat::BuildingDimensions[unitId].y / 32;
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
	bool spreadsCreep(u16 unitId, bool isComplete) {
		

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
		case UnitId::ZergChrysalis:
			return true;

		default:
			return false;
		}
	}


	// x and y are relative positions where (0,0) is presumably the center of the building
	// !!! WARNING !!! Because of the super fun getUnitPointer call in this function, you probably never want to call this function directly
	bool inCreepArea(s32 x, s32 y) {
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
	bool runCreepRandomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg) {
		// Stores the existing tile to replace the creep tile when the creep is removed

		if (*tileBackup == 0) {
			*tileBackup = *tile;
		}

		u16 creepTile;

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


} //hooks

  //-------- Helper function definitions. Do NOT modify! --------//

namespace {

	/* !!! WARNING !!! Use EXTREME caution when calling this. 
	There is absolutely no guarantee that it will find the intended unit, 
	or that it will find a unit at all */
	CUnit* getUnitPointer() {
		static CUnit* unit = NULL;
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


	const u32 Func_Sub414680 = 0x00414680;
	u8 spreadCreep(u32 unitId, int x, int y) {

		static u8 return_value;

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

	const u32 Func_AI_TrainingOverlord = 0x00435770;
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

	const u32 Func_UpdateUnitStrength = 0x0049FA40;
	void updateUnitStrength(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_UpdateUnitStrength
			POPAD
		}

	}

	;

	const u32 Func_Sub4A01F0 = 0x004A01F0;
	void function_004A01F0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub4A01F0
			POPAD
		}

	}

	;

	const u32 Func_CreateUnit = 0x004A09D0;
	CUnit* CreateUnit(u32 unitId, int x, int y, u32 playerId) {

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

	const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
	//hardcoding the larva count function, keeping the return value
	//though it's unused here
	u32 IterateUnitsAtLocationTargetProc_LarvaCount(CUnit* unit, Box16* coords) {

		static u32 return_value;

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

	const u32 Func_Sub4E8DA0 = 0x004E8DA0;
	void function_004E8DA0(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EDI, unit
			CALL Func_Sub4E8DA0
			POPAD
		}

	}

	;

	const u32 Func_Sub4E8E10 = 0x004E8E10;
	bool function_004E8E10(CUnit* unit, Point32* coords) {

		static Bool32 return_pre_value;

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

	const u32 Func_getNumSurroundingCreepTiles = 0x00414290;
	u32 getNumSurroundingCreepTiles(u32 x, u32 y) {
		static u32 return_val;

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

	const u32 Func_setCreepRelationalData = 0x00413FA0;
	void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg) {
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

	const u32 Func_iterateDirectionalCreepData = 0x00413B30;
	u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg) {
		static u32 return_val;

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
