#include "load_unload_proc.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"

namespace hooks
{
	// the transport is "unit", the target is "unitToLoad"
	void loadUnitProc(CUnit* unit, CUnit* unitToLoad)
	{
		u8 unitRaceId;
		u32 loaded_index;
		s32 counter = 0;
		// Old pointers
//		static const CUnit* unitTable_0059CB58 = (CUnit*)0x0059CB58;
//		static const CUnit* unitTable_0059CB64 = (CUnit*)0x0059CB64;
		static const CUnit* unitTable_0059CB58 = (CUnit*)0x0059CB58;
		static const CUnit* unitTable_0059CB64 = (CUnit*)0x0059CB64;
		if (globals.extended_unit_table == 1)
		{
			const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0]) - 336;
			unitTable_0059CB58 = (CUnit*)index_start;
			unitTable_0059CB64 = (CUnit*)(index_start + 12);
		}
		static u32* u32_0x006BEE84 = (u32*)(0x006BEE84);
		static u32* u32_0x006BEE8C = (u32*)(0x006BEE8C);

		if (units_dat::SpaceProvided[unit->id] != 0)
		{
			for (;;)
			{
				u32* loadedUnitOffset = (u32*)&unit->loadedUnit[counter];
				if (*loadedUnitOffset == 0 ||
					unitTable_0059CB64[unit->loadedUnit[counter].index].link.prev == nullptr)
				{
					break;
				}

				CUnit* current_loaded_unit = (CUnit*)&(unitTable_0059CB58[unit->loadedUnit[counter].index]);
				if ((current_loaded_unit->mainOrderId == OrderId::Die && current_loaded_unit->mainOrderState == 1) ||
					current_loaded_unit->targetOrderSpecial != unit->loadedUnit[counter].unitId)
				{
					break;
				}

				counter++;
				if (counter >= units_dat::SpaceProvided[unit->id])
				{
					break;
				}
			}
		}
		
		// 004E7952
		if (units_dat::GroupFlags[unit->id].isZerg)
		{
			unitRaceId = RaceId::Zerg;
		}
		else if (units_dat::GroupFlags[unit->id].isProtoss)
		{
			unitRaceId = RaceId::Protoss;
		}
		else if (units_dat::GroupFlags[unit->id].isTerran)
		{
			unitRaceId = RaceId::Terran;
		}
		else
		{
			unitRaceId = RaceId::Neutral;
		}

		scbw::decloakProc(unit);
		scbw::playSound(SoundId::Misc_ZOvTra00_wav + unitRaceId, unit);
		if (unitToLoad->id == UnitId::TerranMason)
		{
			if (unitToLoad->pAI != nullptr)
			{
				u32* pAI_0x14 = (u32*)((u32)unitToLoad->pAI + 0x14);
				if (*pAI_0x14 == (u32)unitToLoad)
				{
					*pAI_0x14 = NULL;
				}
			}
		}

		// CCMU extender
		auto new_table = unitTable;
		if (globals.extended_unit_table == 1)
		{
			new_table = (CUnit*)&new_unit_table.units[0];
		}

		__asm
		{
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

		/*
		s32 unit_array_length = UNIT_ARRAY_LENGTH;
		if (globals.extended_unit_table == 1)
		{
			unit_array_length = NEW_ARRAY_LENGTH;
		}
		*/

		if (loaded_index < UNIT_ARRAY_LENGTH)
		{
			unit->loadedUnit[counter].index = loaded_index;
			unit->loadedUnit[counter].unitId = unitToLoad->targetOrderSpecial;
		}
		else
		{
			unit->loadedUnit[counter].index = 0;
			unit->loadedUnit[counter].unitId = 0;
		}

		if (unit->id == UnitId::ZergCreepColony
			|| unit->id == UnitId::ZergSunkenColony
			|| unit->id == UnitId::ZergSporeColony
			|| unit->id == UnitId::ZergLarvalColony)
		{
			// TODO: inform creep about missing colony?
		}
		else if (unit->id == UnitId::ProtossPylon)
		{
			// TODO: enable nearby disabled structures, update energy sprite position
		}
		else if (unit->id == UnitId::TerranBunker)
		{
			for (s32 i = 0; i < 8; i++)
			{
				CUnit* cargoUnit = unit->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: disable cargo units
//				scbw::setUnitPosition(cargoUnit, pos.x, pos.y); // <- does not work
			}
		}
		unitToLoad->status |= UnitStatus::InTransport;
		helper::hideAndDisableUnit(unitToLoad);
		unitToLoad->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);

		if (unit->id == UnitId::TerranBunker)
		{
			for (s32 i = 0; i < 8; i++)
			{
				CUnit* cargoUnit = unit->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: Disable cargo units
			}
		}

		scbw::refreshConsole();
		if (unit->status & UnitStatus::GroundedBuilding)
		{
			unitToLoad->status = (unitToLoad->status & ~UnitStatus::IsBuilding) | UnitStatus::InBuilding;
			if (unitToLoad->path != nullptr)
			{
				u32* path_0x4 = (u32*)((u32)unitToLoad->path + 4);
				*path_0x4 = ((u32)unitToLoad->path - *u32_0x006BEE8C) / 128 + 1;
				if (*u32_0x006BEE84 == 0)
				{
					unitToLoad->path = nullptr;
				}
				else
				{
					unitToLoad->path = (void*)((*u32_0x006BEE84 - *u32_0x006BEE8C) / 128 + 1);
				}
				*u32_0x006BEE84 = (u32)unitToLoad->path;
				unitToLoad->path = nullptr;
			}

			// 004E7A81
			unitToLoad->movementState = 0;
			if (unitToLoad->sprite->elevationLevel < 12)
			{
				unitToLoad->pathingFlags |= 1;
			}
			else
			{
				unitToLoad->pathingFlags &= ~1;
			}
			if (unitToLoad->subunit != nullptr
				&& units_dat::BaseProperty[unitToLoad->subunit->id] & UnitProperty::Subunit)
			{
				CUnit* subUnit = unitToLoad->subunit;
				scbw::setUnitPosition(subUnit, unit->sprite->position.x, unit->sprite->position.y);
				subUnit->status = (subUnit->status & ~UnitStatus::IsBuilding) | UnitStatus::InBuilding;
				if (subUnit->path != nullptr)
				{
					u32* path_0x4 = (u32*)((u32)subUnit->path + 4);
					*path_0x4 = ((u32)subUnit->path - *u32_0x006BEE8C) / 128 + 1;
					if (*u32_0x006BEE84 == 0)
					{
						subUnit->path = nullptr;
					}
					else
					{
						subUnit->path = (void*)((*u32_0x006BEE84 - *u32_0x006BEE8C) / 128 + 1);
					}
					*u32_0x006BEE84 = (u32)subUnit->path;
					subUnit->path = nullptr;
				}

				// 004E7B25
				subUnit->movementState = 0;
				if (subUnit->sprite->elevationLevel < 12)
				{
					subUnit->pathingFlags |= 1;
				}
				else
				{
					subUnit->pathingFlags &= ~1;
				}
			}
			else // 004E7B51
			{
				scbw::setUnitPosition(unitToLoad, unit->sprite->position.x, unit->sprite->position.y);
			}
		}
	}

	// the transported unit is "unit", the transport is accessed through unit->connectedUnit
	// Has a boolean return value
	b32 unloadUnitProc(CUnit* unit)
	{
		b32 return_value = 0;
		if (unit->connectedUnit != nullptr)
		{
			CUnit* transport = unit->connectedUnit;
			if ((transport->mainOrderTimer == 0 || transport->status & UnitStatus::GroundedBuilding)
				&& !(transport->status & UnitStatus::DoodadStatesThing)
				&& transport->lockdownTimer == 0
				&& transport->stasisTimer == 0
				&& transport->maelstromTimer == 0)
			{ // 004E7FDC

				Point16 pos = {0,0};
				// probably check where unit can be spawned
				if (helper::canUnload(transport, unit, &pos) != 0)
				{
					if (transport->id == UnitId::TerranTrojan)
					{
						if (transport->getCurrentLifeInGame() <= (transport->getMaxHpInGame() * 0.5))
						{
							transport->mainOrderTimer = 7;
						}
						else
						{
							transport->mainOrderTimer = 10;
						}
					}
					else
					{
						transport->mainOrderTimer = 15;
					}

					if ((unit->status & UnitStatus::IsBuilding) == 0)
					{
						u16* coords[2] = {&pos.x, &pos.y};
						f64 corrections[2] = {32.0, 32.0};

						if (unit->id == UnitId::TerranBunker)
						{
							corrections[0] = 48.0;
						}

						for (s32 i = 0; i < 2; i++)
						{
							*coords[i] = std::round(((f64)*coords[i]) / corrections[i]) * corrections[i];
						}

						if (unit->id == UnitId::ZergCreepColony
							|| unit->id == UnitId::ZergSunkenColony
							|| unit->id == UnitId::ZergSporeColony)
						{
							// TODO: spawn creep
						}
						else if (unit->id == UnitId::ProtossPhotonCannon)
						{
							// TODO: check if pylon power is around, enable or disable the structure
						}
						else if (unit->id == UnitId::ProtossPylon)
						{
							// TODO: enable nearby disabled structures, update energy sprite position
						}
						else if (unit->id == UnitId::TerranBunker)
						{
							for (s32 i = 0; i < 8; i++)
							{
								CUnit* cargoUnit = unit->getLoadedUnit(i);
								if (cargoUnit == nullptr) continue;
								// TODO: enable and update cargo units and their position
//								scbw::setUnitPosition(cargoUnit, pos.x, pos.y); // <- does not work
							}
						}
					}
					scbw::decloakProc(unit->connectedUnit);
					scbw::setUnitPosition(unit, pos.x, pos.y);
					if (unit->subunit != nullptr)
					{
						scbw::setUnitPosition(unit->subunit, pos.x, pos.y);
					}

					helper::IgnoreAllScriptAndGotoIdle(unit);
					helper::function_004E7E10(unit, 0);
					if (unit->pAI == nullptr)
					{
						unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						unit->orderComputerCL(OrderId::ComputerAI);
					}

					scbw::refreshConsole();
					return_value = 1;
					if (transport->status & UnitStatus::GroundedBuilding)
					{ // 004E806B
						if (unit->id != UnitId::ProtossAccantor)
						{
							u8 weaponId = unit->getGroundWeapon();
							if (weaponId != WeaponId::None)
							{
								unit->groundWeaponCooldown = helper::getUpgradedWpnCooldown(unit, weaponId);
							}
							weaponId = unit->getAirWeapon();
							if (weaponId != WeaponId::None)
							{
								unit->airWeaponCooldown = helper::getUpgradedWpnCooldown(unit, weaponId);
							}
							unit->spellCooldown = 30;
						}
						else
						{
							unit->mainOrderTimer = 30;
						}
					}
				}
			}
		}
		return return_value;
	}
}