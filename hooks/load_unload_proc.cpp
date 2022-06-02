#include "load_unload_proc.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"

namespace hooks
{
	// the transport is "unit", the target is "unitToLoad"
	void loadUnitProc(Unit* transport, Unit* cargo_unit)
	{
		u8 unitRaceId;
		u32 loaded_index;
		s32 counter = 0;
		// Old pointers
//		static const Unit* unitTable_0059CB58 = (Unit*)0x0059CB58;
//		static const Unit* unitTable_0059CB64 = (Unit*)0x0059CB64;
		static const Unit* unitTable_0059CB58 = (Unit*)0x0059CB58;
		static const Unit* unitTable_0059CB64 = (Unit*)0x0059CB64;
		if (globals.extended_unit_table == 1)
		{
			const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0]) - 336;
			unitTable_0059CB58 = (Unit*)index_start;
			unitTable_0059CB64 = (Unit*)(index_start + 12);
		}
		static u32* u32_0x006BEE84 = (u32*)(0x006BEE84);
		static u32* u32_0x006BEE8C = (u32*)(0x006BEE8C);

		if (units_dat::SpaceProvided[transport->id] != 0)
		{
			for (;;)
			{
				u32* loadedUnitOffset = (u32*)&transport->loadedUnit[counter];
				if (*loadedUnitOffset == 0 ||
					unitTable_0059CB64[transport->loadedUnit[counter].index].link.prev == nullptr)
				{
					break;
				}

				Unit* current_loaded_unit = (Unit*)&(unitTable_0059CB58[transport->loadedUnit[counter].index]);
				if ((current_loaded_unit->mainOrderId == OrderId::Die && current_loaded_unit->mainOrderState == 1) ||
					current_loaded_unit->targetOrderSpecial != transport->loadedUnit[counter].unitId)
				{
					break;
				}

				counter++;
				if (counter >= units_dat::SpaceProvided[transport->id])
				{
					break;
				}
			}
		}
		
		// 004E7952
		if (units_dat::GroupFlags[transport->id].isZerg)
		{
			unitRaceId = RaceId::Zerg;
		}
		else if (units_dat::GroupFlags[transport->id].isProtoss)
		{
			unitRaceId = RaceId::Protoss;
		}
		else if (units_dat::GroupFlags[transport->id].isTerran)
		{
			unitRaceId = RaceId::Terran;
		}
		else
		{
			unitRaceId = RaceId::Neutral;
		}

		scbw::decloak_proc(transport);
		scbw::decloak_proc(cargo_unit);
		helper::play_sound_from_unit(SoundId::Misc_ZOvTra00_wav + unitRaceId, transport, 1, 0);
		if (cargo_unit->id == UnitId::TerranMason)
		{
			if (cargo_unit->unit_ai != nullptr)
			{
				u32* pAI_0x14 = (u32*)((u32)cargo_unit->unit_ai + 0x14);
				if (*pAI_0x14 == (u32)cargo_unit)
				{
					*pAI_0x14 = NULL;
				}
			}
		}

		// CCMU extender
		auto new_table = unitTable;
		if (globals.extended_unit_table == 1)
		{
			new_table = (Unit*)&new_unit_table.units[0];
		}

		__asm
		{
			PUSHAD
			MOV ECX, cargo_unit
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

		cargo_unit->connectedUnit = transport;

		/*
		s32 unit_array_length = UNIT_ARRAY_LENGTH;
		if (globals.extended_unit_table == 1)
		{
			unit_array_length = NEW_ARRAY_LENGTH;
		}
		*/

		if (loaded_index < UNIT_ARRAY_LENGTH)
		{
			transport->loadedUnit[counter].index = loaded_index;
			transport->loadedUnit[counter].unitId = cargo_unit->targetOrderSpecial;
		}
		else
		{
			transport->loadedUnit[counter].index = 0;
			transport->loadedUnit[counter].unitId = 0;
		}

		// TODO: revisit those, seems like it should be cargo_unit
		if (transport->id == UnitId::ZergCreepColony
			|| transport->id == UnitId::ZergSunkenColony
			|| transport->id == UnitId::ZergSporeColony
			|| transport->id == UnitId::ZergLarvalColony)
		{
			// TODO: inform creep about missing colony?
		}
		else if (transport->id == UnitId::ProtossPylon)
		{
			// TODO: enable nearby disabled structures, update energy sprite position
		}
		else if (transport->id == UnitId::TerranBunker)
		{
			for (s32 i = 0; i < 8; i++)
			{
				Unit* cargoUnit = transport->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: disable cargo units
//				scbw::set_unit_position(cargoUnit, pos.x, pos.y); // <- does not work
			}
		}
		cargo_unit->status |= UnitStatus::InTransport;
		helper::hide_and_disable_unit(cargo_unit);
		cargo_unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);

		if (transport->id == UnitId::TerranBunker)
		{
			for (s32 i = 0; i < 8; i++)
			{
				Unit* cargoUnit = transport->getLoadedUnit(i);
				if (cargoUnit == nullptr) continue;
				// TODO: Disable cargo units
			}
		}

		if (transport->player == 11)
		{
			UnitExt* unitExt = globals.get_unit_ext(transport);
			unitExt->values[ValueIsNeutralBunker] = 1;
			transport->give_to(cargo_unit->player);
		}		

		scbw::refresh_console();
		if (transport->status & UnitStatus::GroundedBuilding)
		{
			cargo_unit->status = (cargo_unit->status & ~UnitStatus::IsBuilding) | UnitStatus::InBuilding;
			if (cargo_unit->path != nullptr)
			{
				u32* path_0x4 = (u32*)((u32)cargo_unit->path + 4);
				*path_0x4 = ((u32)cargo_unit->path - *u32_0x006BEE8C) / 128 + 1;
				if (*u32_0x006BEE84 == 0)
				{
					cargo_unit->path = nullptr;
				}
				else
				{
					cargo_unit->path = (void*)((*u32_0x006BEE84 - *u32_0x006BEE8C) / 128 + 1);
				}
				*u32_0x006BEE84 = (u32)cargo_unit->path;
				cargo_unit->path = nullptr;
			}

			// 004E7A81
			cargo_unit->movementState = 0;
			if (cargo_unit->sprite->elevationLevel < 12)
			{
				cargo_unit->pathingFlags |= 1;
			}
			else
			{
				cargo_unit->pathingFlags &= ~1;
			}
			if (cargo_unit->subunit != nullptr
				&& units_dat::BaseProperty[cargo_unit->subunit->id] & UnitProperty::Subunit)
			{
				Unit* subUnit = cargo_unit->subunit;
				scbw::set_unit_position(subUnit, transport->sprite->position.x, transport->sprite->position.y);
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
				scbw::set_unit_position(cargo_unit, transport->sprite->position.x, transport->sprite->position.y);
			}
		}
	}

	// the transported unit is "unit", the transport is accessed through unit->connectedUnit
	// Has a boolean return value
	b32 unloadUnitProc(Unit* unit)
	{
		b32 return_value = 0;
		if (unit->connectedUnit != nullptr)
		{
			Unit* transport = unit->connectedUnit;
			if ((transport->mainOrderTimer == 0 || transport->status & UnitStatus::GroundedBuilding)
				&& !(transport->status & UnitStatus::Frozen)
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
						if (transport->get_current_life_ingame() <= (transport->get_max_hp_ingame() * 0.5))
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
							|| unit->id == UnitId::ZergSporeColony
							|| unit->id == UnitId::ZergLarvalColony)
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
								Unit* cargoUnit = unit->getLoadedUnit(i);
								if (cargoUnit == nullptr) continue;
								// TODO: enable and update cargo units and their position
//								scbw::set_unit_position(cargoUnit, pos.x, pos.y); // <- does not work
							}
						}
					}
					scbw::decloak_proc(unit->connectedUnit);
					scbw::decloak_proc(unit);
					scbw::set_unit_position(unit, pos.x, pos.y);
					if (unit->subunit != nullptr)
					{
						scbw::set_unit_position(unit->subunit, pos.x, pos.y);
					}

					helper::IgnoreAllScriptAndGotoIdle(unit);
					helper::function_004E7E10(unit, 0);
					if (unit->unit_ai == nullptr)
					{
						unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						unit->order_computer(OrderId::ComputerAI);
					}

					UnitExt* transportExt = globals.get_unit_ext(transport);
					if (transportExt->values[ValueIsNeutralBunker])
					{
						if (!transport->getFirstLoadedUnit())
						{
							transport->give_to(11);
							transport->sprite->playerId = 11;
						}
						else
						{
							bool isOwnerValid = true;
							for (u32 i = 0; i < 4; ++i)
							{
								Unit* cargoUnit = transport->getLoadedUnit(i);
								if (cargoUnit && cargoUnit->player != unit->player)
								{
									isOwnerValid = false;
								}
							}
							if (!isOwnerValid)
							{
								transport->give_to(transport->getFirstLoadedUnit()->player);
							}
						}
					}

					scbw::refresh_console();
					return_value = 1;
					if (transport->status & UnitStatus::GroundedBuilding)
					{ // 004E806B
						if (unit->id != UnitId::ProtossAccantor)
						{
							u8 weaponId = unit->get_ground_weapon();
							if (weaponId != WeaponId::None)
							{
								unit->groundWeaponCooldown = helper::get_upgraded_weapon_cooldown(unit, weaponId);
							}
							weaponId = unit->getAirWeapon();
							if (weaponId != WeaponId::None)
							{
								unit->airWeaponCooldown = helper::get_upgraded_weapon_cooldown(unit, weaponId);
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