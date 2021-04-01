#include "load_unload_proc.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"
//helper functions def

namespace {

	u32 getUpgradedWpnCooldown(CUnit* unit, u8 weaponId);							//75DC0
	void IgnoreAllScriptAndGotoIdle(CUnit* unit);									//76550
	void hideAndDisableUnit(CUnit* unit);											//E6340
	u32 function_004E76C0(CUnit* transport, CUnit* loaded_unit, Point16* pos);		//E76C0
	void function_004E7E10(CUnit* unit, u32 unk);									//E7E10

} //unnamed namespace

namespace hooks {

	//initial name was sub_4E78E0
	//the transport is "unit", the target is "unitToLoad"
	void loadUnitProc(CUnit* unit, CUnit* unitToLoad) {

		u8 unitRaceId;
		u32 loaded_index;
		int counter = 0;
		//before CCMU extender change
//		static const CUnit* unitTable_0059CB58 = (CUnit*)0x0059CB58;
//		static const CUnit* unitTable_0059CB64 = (CUnit*)0x0059CB64;
		static CUnit* unitTable_0059CB58 = (CUnit*)0x0059CB58;
		static CUnit* unitTable_0059CB64 = (CUnit*)0x0059CB64;
		if (Globals.extended_unit_table == 1) {
			const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0])-336;
			unitTable_0059CB58 = (CUnit*)index_start;
			unitTable_0059CB64 = (CUnit*)(index_start+12);
		}
		static u32* u32_0x006BEE84 = (u32*)(0x006BEE84);
		static u32* u32_0x006BEE8C = (u32*)(0x006BEE8C);

		if (units_dat::SpaceProvided[unit->id] != 0) {

			bool bEndLoop = false;
			u32* loadedUnitOffset;

			while (!bEndLoop) {

				loadedUnitOffset = (u32*)&unit->loadedUnit[counter];

				if (*loadedUnitOffset == 0)
					bEndLoop = true;
				else {

					if (unitTable_0059CB64[unit->loadedUnit[counter].index].link.prev == nullptr)
						bEndLoop = true;
					else {

						CUnit* current_loaded_unit = (CUnit*) & (unitTable_0059CB58[unit->loadedUnit[counter].index]);

						if (
							current_loaded_unit->mainOrderId == OrderId::Die &&
							current_loaded_unit->mainOrderState == 1
							)
							bEndLoop = true;
						else {

							if (current_loaded_unit->targetOrderSpecial != unit->loadedUnit[counter].unitId)
								bEndLoop = true;
							else {

								counter++;

								if (counter >= units_dat::SpaceProvided[unit->id])
									bEndLoop = true;

							}

						}

					}

				}

			}

		}

		//E7952
		if (units_dat::GroupFlags[unit->id].isZerg)
			unitRaceId = RaceId::Zerg;
		else
			if (units_dat::GroupFlags[unit->id].isProtoss)
				unitRaceId = RaceId::Protoss;
			else
				if (units_dat::GroupFlags[unit->id].isTerran)
					unitRaceId = RaceId::Terran;
				else
					unitRaceId = RaceId::Neutral;

		scbw::playSound(SoundId::Misc_ZOvTra00_wav + unitRaceId, unit);

		if (unitToLoad->id == UnitId::TerranMason) {

			if (unitToLoad->pAI != nullptr) {

				u32* pAI_0x14 = (u32*)((u32)unitToLoad->pAI + 0x14);

				if (*pAI_0x14 == (u32)unitToLoad)
					*pAI_0x14 = NULL;

			}

		}
		//new - CCMU extender
		auto new_table = unitTable;
		if (Globals.extended_unit_table == 1) {
			new_table = (CUnit*)&new_unit_table.units[0];
		}
		//
		//e79a3
		__asm {
			PUSHAD
			MOV ECX, unitToLoad
//			SUB ECX, unitTable
			SUB ECX, new_table
			MOV EAX, 0x86186187
			MUL ECX
			SUB ECX, EDX
			SHR ECX, 1
			ADD ECX, EDX
			SHR ECX, 0x08
			INC ECX
			MOV loaded_index, ECX
			POPAD
		}

		unitToLoad->connectedUnit = unit;
		/*int unit_array_length = UNIT_ARRAY_LENGTH;
		if (Globals.extended_unit_table == 1) {
			unit_array_length = NEW_ARRAY_LENGTH;
		}*/
		if (loaded_index < UNIT_ARRAY_LENGTH) {
			unit->loadedUnit[counter].index = loaded_index;
			unit->loadedUnit[counter].unitId = unitToLoad->targetOrderSpecial;
		}
		else {
			unit->loadedUnit[counter].index = 0;
			unit->loadedUnit[counter].unitId = 0;
		}

		if (unit->id == UnitId::ZergCreepColony
		|| unit->id == UnitId::ZergSunkenColony
		|| unit->id == UnitId::ZergSporeColony) {
			// TODO: inform creep about missing colony?
		} else if (unit->id == UnitId::ProtossPylon) {
			// TODO: enable nearby disabled structures, update energy sprite position
		} else if (unit->id == UnitId::TerranBunker) {
			for (int i = 0; i < 8; i++) {
				CUnit* cargoUnit = unit->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: disable cargo units
				//scbw::setUnitPosition(cargoUnit, pos.x, pos.y); // <- does not work
			}
		}

		unitToLoad->status |= UnitStatus::InTransport;
		hideAndDisableUnit(unitToLoad);
		unitToLoad->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);

		if (unit->id == UnitId::TerranBunker) {
			for (int i = 0; i < 8; i++) {
				CUnit* cargoUnit = unit->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: Disable cargo units
			}
		}

		scbw::refreshConsole();

		if (unit->status & UnitStatus::GroundedBuilding) {

			unitToLoad->status = (unitToLoad->status & ~UnitStatus::IsBuilding) | UnitStatus::InBuilding;

			if (unitToLoad->path != nullptr) {

				u32* path_0x4 = (u32*)((u32)unitToLoad->path + 4);

				*path_0x4 = ((u32)unitToLoad->path - *u32_0x006BEE8C) / 128 + 1;

				if (*u32_0x006BEE84 == 0)
					unitToLoad->path = nullptr;
				else
					unitToLoad->path = (void*)((*u32_0x006BEE84 - *u32_0x006BEE8C) / 128 + 1);

				*u32_0x006BEE84 = (u32)unitToLoad->path;
				unitToLoad->path = nullptr;

			}

			//E7A81
			unitToLoad->movementState = 0;

			if (unitToLoad->sprite->elevationLevel < 12)
				unitToLoad->pathingFlags |= 1;
			else
				unitToLoad->pathingFlags &= ~1;

			if (
				unitToLoad->subunit != nullptr &&
				units_dat::BaseProperty[unitToLoad->subunit->id] & UnitProperty::Subunit
				)
			{

				CUnit* subUnit = unitToLoad->subunit;

				scbw::setUnitPosition(subUnit, unit->sprite->position.x, unit->sprite->position.y);

				subUnit->status = (subUnit->status & ~UnitStatus::IsBuilding) | UnitStatus::InBuilding;

				if (subUnit->path != nullptr) {

					u32* path_0x4 = (u32*)((u32)subUnit->path + 4);

					*path_0x4 = ((u32)subUnit->path - *u32_0x006BEE8C) / 128 + 1;

					if (*u32_0x006BEE84 == 0)
						subUnit->path = nullptr;
					else
						subUnit->path = (void*)((*u32_0x006BEE84 - *u32_0x006BEE8C) / 128 + 1);

					*u32_0x006BEE84 = (u32)subUnit->path;
					subUnit->path = nullptr;

				}

				//E7B25
				subUnit->movementState = 0;

				if (subUnit->sprite->elevationLevel < 12)
					subUnit->pathingFlags |= 1;
				else
					subUnit->pathingFlags &= ~1;

			}
			else //E7B51
				scbw::setUnitPosition(unitToLoad, unit->sprite->position.x, unit->sprite->position.y);

		}

	}

	;

	//initial name was sub_4E7F70
	//the transported unit is "unit", the transport is
	//accessed through unit->connectedUnit
	//Has a boolean return value
	Bool32 unloadUnitProc(CUnit* unit) {

		Bool32 return_value = 0;

		if (unit->connectedUnit != nullptr) {

			CUnit* transport = unit->connectedUnit;

			if ((transport->mainOrderTimer == 0 || transport->status & UnitStatus::GroundedBuilding)
			 && !(transport->status & UnitStatus::DoodadStatesThing)
			 && transport->lockdownTimer == 0
			 && transport->stasisTimer == 0
			 && transport->maelstromTimer == 0)
			{ //E7FDC

				Point16 pos = { 0,0 };

				//probably check where unit can be spawned
				if (function_004E76C0(transport, unit, &pos) != 0) {

					if (transport->id == UnitId::TerranTrojan) {
						if (transport->getCurrentLifeInGame() <= (transport->getMaxHpInGame() * 0.5)) {
							transport->mainOrderTimer = 7;
						}
						else {
							transport->mainOrderTimer = 10;
						}
					}
					else {
						transport->mainOrderTimer = 15;
					}

					if ((unit->status & UnitStatus::IsBuilding) == 0) {
						u16* coords[2] = {&pos.x, &pos.y};
						double corrections[2] = {32.0, 32.0};

						if (unit->id == UnitId::TerranBunker) {
							corrections[0] = 48.0;
						}

						for (int i = 0; i < 2; i++) {
							*coords[i] = std::round(((double)*coords[i]) / corrections[i]) * corrections[i];
						}

						if (unit->id == UnitId::ZergCreepColony
						|| unit->id == UnitId::ZergSunkenColony
						|| unit->id == UnitId::ZergSporeColony) {
							// TODO: spawn creep
						} else if (unit->id == UnitId::ProtossPhotonCannon) {
							// TODO: check if pylon power is around, enable or disable the structure
						} else if (unit->id == UnitId::ProtossPylon) {
							// TODO: enable nearby disabled structures, update energy sprite position
						} else if (unit->id == UnitId::TerranBunker) {
							for (int i = 0; i < 8; i++) {
								CUnit* cargoUnit = unit->getLoadedUnit(i);
								if (cargoUnit == nullptr) continue;
								// TODO: enable and update cargo units and their position
								//scbw::setUnitPosition(cargoUnit, pos.x, pos.y); // <- does not work
							}
						}
					}
					scbw::setUnitPosition(unit, pos.x, pos.y);

					if (unit->subunit != nullptr)
						scbw::setUnitPosition(unit->subunit, pos.x, pos.y);

					IgnoreAllScriptAndGotoIdle(unit);

					function_004E7E10(unit, 0);

					if (unit->pAI == nullptr)
						unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
					else
						unit->orderComputerCL(OrderId::ComputerAI);

					scbw::refreshConsole();

					return_value = 1;

					if (transport->status & UnitStatus::GroundedBuilding) { //E806B

						if (unit->id != UnitId::ProtossAccantor) {

							u8 weaponId = unit->getGroundWeapon();

							if (weaponId != WeaponId::None)
								unit->groundWeaponCooldown = getUpgradedWpnCooldown(unit, weaponId);

							weaponId = unit->getAirWeapon();

							if (weaponId != WeaponId::None)
								unit->airWeaponCooldown = getUpgradedWpnCooldown(unit, weaponId);

							unit->spellCooldown = 30;

						}
						else
							unit->mainOrderTimer = 30;

					}

				}

			}

		}

		return return_value;

	}

	;

} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const u32 Func_getUpgradedWpnCooldown = 0x00475DC0;
	u32 getUpgradedWpnCooldown(CUnit* unit, u8 weaponId) {

		static u32 return_value;

		__asm {
			PUSHAD
			MOV AL, weaponId
			MOV ESI, unit
			CALL Func_getUpgradedWpnCooldown
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	const u32 Func_IgnoreAllScriptAndGotoIdle = 0x00476550;
	void IgnoreAllScriptAndGotoIdle(CUnit* unit) {

		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_IgnoreAllScriptAndGotoIdle
			POPAD
		}

	}

	;

	const u32 Func_unitDeathSomething_0 = 0x004E6340;
	void hideAndDisableUnit(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitDeathSomething_0
			POPAD
		}

	}

	;

	const u32 Func_Sub4E76C0 = 0x004E76C0;
	u32 function_004E76C0(CUnit* transport, CUnit* loaded_unit, Point16* pos) {

		static u32 result;

		__asm {
			PUSHAD
			MOV EAX, transport
			MOV ESI, loaded_unit
			PUSH pos
			CALL Func_Sub4E76C0
			MOV result, EAX
			POPAD
		}

		return result;

	}

	;

	const u32 Func_Sub4E7E10 = 0x004E7E10;
	void function_004E7E10(CUnit* unit, u32 unk) {

		__asm {
			PUSHAD
			MOV EAX, unit
			PUSH unk
			CALL Func_Sub4E7E10
			POPAD
		}

	}

	;

} //Unnamed namespace

//End of helper functions
