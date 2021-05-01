#include "larva_creep_spawn.h"
#include <SCBW/api.h>
#include "SCBW\UnitFinder.h"

namespace hooks {

	setCreepRelationalData_callback runCreepRandomizerPtr = &runCreepRandomizer;

	//probably create the single larva accompanying
	//a building at game beginning
	void function_0049D660(CUnit* main_building) {

		Point32 coords;

		//look for a location to spawn a larva at
		if (function_004E8E10(main_building, &coords)) {

			CUnit* unit_created = createUnit(UnitId::ZergLarva, coords.x, coords.y, main_building->playerId);

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

		} else
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
					} else if (unit->id == UnitId::ZergHive && unit->mainOrderId != OrderId::Morph2 ||
						unit->id == UnitId::ZergSire && unit->mainOrderId == OrderId::Morph2) {
						unit->building.larvaTimer = 18;
					} else if (unit->id == UnitId::ZergSire && unit->mainOrderId != OrderId::Morph2) {
						unit->building.larvaTimer = 12;
					}

					if (!function_004E8E10(unit, &coords_2))
						unit->building.larvaTimer = 3;
					else {

						//EA830:
						CUnit* new_larva = createUnit(UnitId::ZergLarva, coords_2.x, coords_2.y, unit->playerId);

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
			|| unit->id == UnitId::ZergLair
			|| unit->id == UnitId::ZergHive
			|| unit->id == UnitId::ZergSire
			|| (unit->id == UnitId::ZergInfestedCommandCenter && !(unit->status & UnitStatus::InAir))
			|| unit->id == UnitId::ZergLarvalColony) {
			secondaryOrd_SpawningLarva(unit);
		}
		if (unit->building.creepTimer != 0) {
			if (unit->building.creepTimer >= 2) {
				unit->building.creepTimer -= 2;
			} else {
				unit->building.creepTimer = 0;
			}
		} else {
			//EA8B6:
			uint8 result_00414680;

			unit->building.creepTimer = 15; /*0F*/
			if (unit->id == 180) {
				//scbw::print("spread basilisk creep");
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
				//scbw::print("Check the creep spread...");
				uint32 current_tile_x = unit->sprite->position.x / 32;
				uint32 current_tile_y = unit->sprite->position.y / 32;
				int width_x = 6;
				int width_y = 4;
				uint8 mustspread = 0;
				int x1 = current_tile_x - width_x;
				int y1 = current_tile_y - width_x;
				int x2 = current_tile_x + width_x;
				int y2 = current_tile_y + width_y;

				if (x1 < 0) {
					x1 = 0;
				}
				if (y1 < 0) {
					y1 = 0;
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
						int X = (x - (current_tile_x * 32));
						int Y = (y - (current_tile_y * 32));
						bool in_range = ((256 * X * X) + (100 * Y * Y)) < 1300000;
						//adjust value 1300000 to get slightly different radius

						if (in_range) {
							ActiveTile a = scbw::getActiveTileAt(i * 32, j * 32);

							if (a.hasCreep || a.temporaryCreep || a.creepReceeding) {
								//
							} else {
								//can spread
								if (!a.alwaysUnbuildable && a.isWalkable) {

									mustspread = 1;
								}
							}
						}

					}
				}
				char buf[64];
				sprintf(buf, "Limits: %d %d %d %d, status: %d", x1, y1, x2, y2, mustspread);
				//scbw::print(buf);	
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
					(unit->id == UnitId::ZergInfestedCommandCenter && !(unit->status & UnitStatus::InAir)) ||
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
			} else if (rect->right >= *mapTileBufWidth) {
				rect->right = *mapTileBufWidth - 1;
			}
			if (rect->top < 0) {
				rect->top = 0;
			} else if (rect->bottom >= *mapTileBufHeight) {
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
		} else if (rect->right >= *mapTileBufWidth) {
			rect->right = *mapTileBufWidth - 1;
		}
		if (rect->top < 0) {
			rect->top = 0;
		} else if (rect->bottom >= *mapTileBufHeight) {
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

		return (x * x * 100 + y * y * 256) <= 10240000;

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
		} else {
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
}