#include "load_unload_proc.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"

namespace hooks
{
	// the transport is "unit", the target is "unitToLoad"
	void load_unit_proc(Unit* transport, Unit* cargo_unit)
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
				u32* loadedUnitOffset = (u32*)&transport->loaded_unit[counter];
				if (*loadedUnitOffset == 0 ||
					unitTable_0059CB64[transport->loaded_unit[counter].index].link.prev == nullptr)
				{
					break;
				}

				Unit* current_loaded_unit = (Unit*)&(unitTable_0059CB58[transport->loaded_unit[counter].index]);
				if ((current_loaded_unit->main_order_id == OrderId::Die && current_loaded_unit->main_order_state == 1) ||
					current_loaded_unit->target_order_special != transport->loaded_unit[counter].unit_id)
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
		auto new_table = unit_table;
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

		cargo_unit->connected_unit = transport;

		/*
		s32 unit_array_length = UNIT_ARRAY_LENGTH;
		if (globals.extended_unit_table == 1)
		{
			unit_array_length = NEW_ARRAY_LENGTH;
		}
		*/

		if (loaded_index < UNIT_ARRAY_LENGTH)
		{
			transport->loaded_unit[counter].index = loaded_index;
			transport->loaded_unit[counter].unit_id = cargo_unit->target_order_special;
		}
		else
		{
			transport->loaded_unit[counter].index = 0;
			transport->loaded_unit[counter].unit_id = 0;
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
				Unit* cargoUnit = transport->get_loaded_unit(i);
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
				Unit* cargoUnit = transport->get_loaded_unit(i);
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
			cargo_unit->movement_state = 0;
			if (cargo_unit->sprite->elevationLevel < 12)
			{
				cargo_unit->pathing_flags |= 1;
			}
			else
			{
				cargo_unit->pathing_flags &= ~1;
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
				subUnit->movement_state = 0;
				if (subUnit->sprite->elevationLevel < 12)
				{
					subUnit->pathing_flags |= 1;
				}
				else
				{
					subUnit->pathing_flags &= ~1;
				}
			}
			else // 004E7B51
			{
				scbw::set_unit_position(cargo_unit, transport->sprite->position.x, transport->sprite->position.y);
			}
		}
	}

	// the transported unit is "unit", the transport is accessed through unit->connected_unit
	// Has a boolean return value
	b32 unload_unit_proc(Unit* unit)
	{
		b32 return_value = 0;
		if (unit->connected_unit != nullptr)
		{
			Unit* transport = unit->connected_unit;
			if ((transport->main_order_timer == 0 || transport->status & UnitStatus::GroundedBuilding)
				&& !(transport->status & UnitStatus::Frozen)
				&& transport->lockdown_timer == 0
				&& transport->stasis_timer == 0
				&& transport->maelstrom_timer == 0)
			{ // 004E7FDC

				Point16 pos = {0,0};
				// probably check where unit can be spawned
				if (helper::canUnload(transport, unit, &pos) != 0)
				{
					if (transport->id == UnitId::TerranTrojan)
					{
						if (transport->get_current_life_ingame() <= (transport->get_max_hp_ingame() * 0.5))
						{
							transport->main_order_timer = 7;
						}
						else
						{
							transport->main_order_timer = 10;
						}
					}
					else
					{
						transport->main_order_timer = 15;
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
								Unit* cargoUnit = unit->get_loaded_unit(i);
								if (cargoUnit == nullptr) continue;
								// TODO: enable and update cargo units and their position
//								scbw::set_unit_position(cargoUnit, pos.x, pos.y); // <- does not work
							}
						}
					}
					scbw::decloak_proc(unit->connected_unit);
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
						if (!transport->get_first_loaded_unit())
						{
							transport->give_to(11);
							transport->sprite->playerId = 11;
						}
						else
						{
							bool isOwnerValid = true;
							for (u32 i = 0; i < 4; ++i)
							{
								Unit* cargoUnit = transport->get_loaded_unit(i);
								if (cargoUnit && cargoUnit->player != unit->player)
								{
									isOwnerValid = false;
								}
							}
							if (!isOwnerValid)
							{
								transport->give_to(transport->get_first_loaded_unit()->player);
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
								unit->ground_weapon_cooldown = helper::get_upgraded_weapon_cooldown(unit, weaponId);
							}
							weaponId = unit->get_air_weapon();
							if (weaponId != WeaponId::None)
							{
								unit->air_weapon_cooldown = helper::get_upgraded_weapon_cooldown(unit, weaponId);
							}
							unit->spell_cooldown = 30;
						}
						else
						{
							unit->main_order_timer = 30;
						}
					}
				}
			}
		}
		return return_value;
	}

	// Hooked to extend transports. -Pr0nogo
	char update_unit_transport_data(Unit* cargo, Unit* unused) // 00401EA0
	{
		u32 pathing = 0;
		if (cargo->status & UnitStatus::InTransport)
		{
			Unit* cargo_transport = cargo->connected_unit;
			if (cargo_transport)
			{
				u32 cargo_index = cargo - unit_table + 1;
				u32 loaded_index = 0;
				if (cargo_index <= 1700)
				{
					loaded_index = cargo_index | (cargo->target_order_special << 11);
				}
				u32 iterator = 0;
				if (units_dat::SpaceProvided[cargo_transport->id])
				{
					u16 cargo_index_bool = cargo_transport->loaded_unit[0].index;
					do
					{
						if (cargo_index_bool == loaded_index)
						{
							break;
						}
						++iterator;
						++cargo_index_bool;
					} while (iterator < units_dat::SpaceProvided[cargo_transport->id]);
				}
				helper::move_unit_duplicate(cargo, cargo_transport->sprite->position.x, cargo_transport->sprite->position.y);
				cargo->connected_unit->loaded_unit[iterator].index = 0;
				u32 cargo_status_bool = cargo->status & 0xFFFFFFBF;
				cargo->connected_unit = 0;
				cargo->status = cargo_status_bool;
				helper::toggle_unit_pathing_properties(cargo);
				if (cargo->subunit)
				{
					if (units_dat::BaseProperty[cargo->subunit->id] & UnitProperty::Subunit)
					{
						pathing = helper::toggle_unit_pathing_properties(cargo->subunit);
					}
				}
			}
			else
			{
				cargo->status = pathing & 0xFFFFFFBF;
				helper::toggle_unit_pathing_properties(cargo);
				if (cargo->subunit)
				{
					if (units_dat::BaseProperty[cargo->subunit->id] & UnitProperty::Subunit)
					{
						pathing = helper::toggle_unit_pathing_properties(cargo->subunit);
					}
				}
			}
		}
		return pathing;
	}

	// Hooked to extend transports. -Pr0nogo
	void kill_all_loaded_units(Unit* transport) // 0049FDD0
	{
		Unit* unit_reference; // edi
		int unload_result; // eax
		signed int times_to_iterate; // [esp+4h] [ebp-8h]

		char transport_data = 0;
		if (transport->status & UnitStatus::InTransport)
		{
			transport_data = update_unit_transport_data(transport, 0);
		}
		else if (!(transport->status & UnitStatus::IsHallucination))
		{
			// Originally had a check for the Ovileth transport upgrade. We enable it by default, so BYE!
			u8 transport_space = units_dat::SpaceProvided[transport->id];
			if (transport_space)
			{
				u16 index = transport->loaded_unit[0].index;
				u16 index_2 = transport->loaded_unit[0].index;
				times_to_iterate = 8; // Should be transport_space?
				do
				{
					if (index_2)
					{
						unit_reference = &unit_reference[index_2 & 0x7FF];
						if (unit_reference->sprite)
						{
							if ((unit_reference->main_order_id || unit_reference->main_order_state != 1)
								&& unit_reference->target_order_special == index_2 >> 11)
							{
								if (unit_reference->sprite)
								{
									if (unit_reference->main_order_id)
									{
										if (transport->id != UnitId::TerranBunker
										||  !helper::unload_unit_proc(unit_reference))
										{
											helper::orders_self_destructing(unit_reference);
										}
										index = transport->loaded_unit[0].index;
									}
								}
							}
						}
					}
					++index_2;
					--times_to_iterate;
				} while (times_to_iterate);
				/* IDK
				index[0] = 0;
				index[1] = 0;
				index[2] = 0;
				index[3] = 0;
				*/
			}
		}
	}

	// TODO: 004C6F70 get_doodad_state_unit

	// Hooked to extend transports. -Pr0nogo
	bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args) // 004C7E20
	{
		if (args->check_height && helper::is_unit_at_height(unit, args->flags))
		{
			return 0;
		}
		KillUnitArgs* args_func = (KillUnitArgs*)args->func;
		if ((unit->status & UnitStatus::IsHallucination) == 0)
		{
			u8 space_provided = units_dat::SpaceProvided[unit->id];
			if (space_provided)
			{
				u16 index = unit->loaded_unit[0].index;
				do
				{
					u32 index_lowbyte = index;
					if (index)
					{
						index_lowbyte = index_lowbyte;
						if (index_lowbyte)
						{
							u32 dumbness = 336 * (index_lowbyte & 0x7FF);
							void* array_insanity = unk_array_59CB64[dumbness / 4];
							Unit* unit_reference = &::unit_reference[dumbness / 0x150];
							if (array_insanity)
							{
								if (unit_reference->main_order_id || unit_reference->main_order_state != 1)
								{
									u8 order_special = unit_reference->target_order_special;
									if (order_special == index_lowbyte >> 11)
									{
										*trig_killing_additional_unit = 1;
										helper::kill_units_at_location_proc(unit_reference, args_func); // Was &args_func
										*trig_killing_additional_unit = 0;
									}
								}
							}
						}
					}
					++index;
					--space_provided;
				} while (space_provided);
			}
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			Unit* powerup = unit->worker.powerup;
			if (powerup)
			{
				*trig_killing_additional_unit = 1;
				helper::kill_units_at_location_proc(powerup, args_func); // Was &args_func
				*trig_killing_additional_unit = 0;
			}
		}
		if (!unit->sprite)
		{
			return 0;
		}
		if (!unit->main_order_id)
		{
			return 0;
		}
		/*
		if (!(args->func)(args->player, args->unit_id, unit))
		{
			return 0;
		}
		*/
		if (units_dat::BaseProperty[unit->id] & UnitProperty::NeutralAccessories && unit->powerup_carrier)
		{
			helper::kill_powerup(unit->powerup_carrier);
		}
		else
		{
			if (!(unit->status & UnitStatus::IsHallucination)
				&& units_dat::SpaceProvided[unit->id])
			{
				if (*trig_remove_unit_active)
				{
					unit->user_action_flags |= 4u;
				}
			}
			else if (*trig_remove_unit_active)
			{
				helper::hide_unit(unit);
			}
			helper::orders_self_destructing(unit);
		}
		if (!(*trig_killing_additional_unit))
		{
			--(*trig_kill_unit_count);
		}
		return *trig_kill_unit_count == 0;
	}

	// Hooked to extend transports. -Pr0nogo
	Unit* get_loaded_unit_from_index(Unit* transport, u32 index) // 004E6C40
	{
		int loaded_index = transport->loaded_unit[index].index;
		Unit* result = &unit_reference[loaded_index & 0x7FF];
		if (!loaded_index
		||  !result->sprite
		||  !result->main_order_id && result->main_order_state == 1
		||   result->target_order_special != loaded_index >> 11)
		{
			result = 0;
		}
		return result;
	}

	// Hooked to extend transports. -Pr0nogo
	// Logic moved from Unit.cpp
	Unit* get_first_loaded_unit(Unit* transport) // 004E6C90
	{
		for (s32 i = 0; i < units_dat::SpaceProvided[transport->id]; ++i)
		{
			Unit* first_loaded_unit = hooks::get_loaded_unit(transport, i);
			if (first_loaded_unit)
			{
				return first_loaded_unit;
			}
		}
		return 0;
	}
	
	// Moved from Unit.cpp
	Unit* get_loaded_unit(Unit* transport, u32 index) {
		Unit* loaded_unit = get_unit_from_index(transport->loaded_unit[index].index);
		if (loaded_unit && loaded_unit->sprite
			&& !(loaded_unit->main_order_id == OrderId::Die && loaded_unit->main_order_state == 1)
			&& loaded_unit->target_order_special == transport->loaded_unit[index].unit_id)
		{
			return loaded_unit;
		}
		return nullptr;
	}

	// Moved from Unit.cpp
	Unit* get_unit_from_index(u16 index)
	{
		if (globals.extended_unit_table == 1)
		{
			if (1 <= index && index <= UNIT_ARRAY_LENGTH)
			{
				return (Unit*)&new_unit_table.units[index - 1];
			}
			return nullptr;
		}
		if (1 <= index && index <= UNIT_ARRAY_LENGTH)
		{
			return &unit_table[index - 1];
		}
		return nullptr;
	}

	// Hooked to extend transports. -Pr0nogo
	u32 iterate_loaded_units(Unit* transport, void* func, Unit* cargo) // 004E6D00
	{
		u32 space_provided = units_dat::SpaceProvided[transport->id];
		if (!space_provided)
		{
			return 0;
		}
		u16 index = transport->loaded_unit[0].index;
		u32 iterations = 0;
		while (1)
		{
			u16 index_var = index;
			if (index)
			{
				Unit* func_transport = &unit_reference[index_var & 0x7FF];
				if (func_transport->sprite)
				{
					if ((func_transport->main_order_id || func_transport->main_order_state != 1)
					&&   func_transport->target_order_special == index_var >> 11
				//	&&   func(func_transport, cargo)
						)
					{
						break;
					}
				}
			}
			++iterations;
			++index;
			if (iterations >= space_provided)
			{
				return 0;
			}
		}
		return 1;
	}

	// Hooked to extend transports. -Pr0nogo
	s32 get_first_loaded_worker(Unit* transport) // 004E6D90
	{
		u32 iterations = 0;
		u16 index = transport->loaded_unit[0].index;
		while (1)
		{
			u16 index_var = index;
			if (index)
			{
				Unit* unit_reference = &::unit_reference[index_var & 0x7FF];
				if (unit_reference->sprite)
				{
					if ((unit_reference->main_order_id || unit_reference->main_order_state != 1)
						&& unit_reference->target_order_special == index_var >> 11
						&& units_dat::BaseProperty[unit_reference->id] & UnitProperty::Worker)
					{
						break;
					}
				}
			}
			++iterations;
			++index;
			if (iterations >= 8)
			{
				return 0;
			}
		}
		return 1;
	}

	// Hooked to extend transports. -Pr0nogo
	s32 is_carrying_flag(Unit* unit) // 004F3A80
	{
		u16 unit_id = unit->id;
		if (units_dat::BaseProperty[unit_id] & UnitProperty::Worker)
		{
			Unit* powerup = unit->worker.powerup;
			if (powerup && powerup->id == UnitId::Powerup_Flag)
			{
				return 1;
			}
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination
			|| !units_dat::SpaceProvided[unit_id])
		{
			return 0;
		}
		u32 iterations = 0;
		u16 index = unit->loaded_unit[0].index;
		while (1)
		{
			u16 index_var = index;
			if (index)
			{
				Unit* unit_reference = &::unit_reference[index_var & 0x7FF];
				if (unit_reference->sprite)
				{
					if ((unit_reference->main_order_id || unit_reference->main_order_state != 1)
						&& unit_reference->target_order_special == index_var >> 11)
					{
						if (units_dat::BaseProperty[unit_reference->id] & UnitProperty::Worker)
						{
							Unit* powerup_reference = unit_reference->worker.powerup;
							if (powerup_reference)
							{
								if (powerup_reference->id == UnitId::Powerup_Flag)
								{
									break;
								}
							}
						}
					}
				}
			}
			++iterations;
			++index;
			if (iterations >= 8)
			{
				return 0;
			}
		}
		return 1;
	}
}