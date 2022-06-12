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
		if (units_dat::GroupFlags[transport->id].is_zerg)
		{
			unitRaceId = RaceId::Zerg;
		}
		else if (units_dat::GroupFlags[transport->id].is_protoss)
		{
			unitRaceId = RaceId::Protoss;
		}
		else if (units_dat::GroupFlags[transport->id].is_terran)
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
			if (cargo_unit->sprite->elevation_level < 12)
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
				if (subUnit->sprite->elevation_level < 12)
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

	// Hooked to extend transports. -Pr0nogo
	s32 get_doodad_state_unit(Unit* unit, FindUnitLocationParam* parameter) // 004C6F70
	{
		u32* player_id = &parameter->player;
		u32 do_check_height = parameter->check_height;
		FindUnitLocationParam* parameter_var = parameter;
		if (do_check_height && helper::is_unit_at_height(unit, parameter->location_flags))
		{
			return 0;
		}

		u32 playerid_1_wrong = player_id[1];
		u32 playerid_2_wrong = player_id[2];
		FindUnitLocationParam param_2 = {};
		param_2.player = *player_id;
		u32 unit_id_2 = unit->id;
		param_2.check_height = playerid_2_wrong;
		u32 iterations = 0;
		*&param_2.unit_id = playerid_1_wrong;
		param_2.check_height = 0;

		if (!(units_dat::BaseProperty[unit_id_2] & UnitProperty::Worker)
		||	  unit->building.addon == 0
		||   !get_doodad_state_unit(unit->building.addon, &param_2))
		{
			if (helper::is_unit_owned_by_player_ex(*player_id, unit))
			{
				if (helper::unit_is_active_transport(unit))
				{
					u32 unit_id_3 = unit->id;
					u32 space_provided_1 = units_dat::BaseProperty[unit_id_3];
					u32 space_provided_2 = units_dat::SpaceProvided[unit_id_3];
					if (space_provided_1)
					{
						u16 index = unit->loaded_unit[0].index;
						do
						{
							if (index)
							{
								Unit* loaded_unit = get_loaded_unit_from_index(unit, iterations);
								if (loaded_unit && get_doodad_state_unit(loaded_unit, &param_2))
								{
									return 1;
								}
								space_provided_1 = space_provided_2;
							}
							++iterations;
							++index;
						} while (iterations < space_provided_1);
						player_id = &parameter_var->player;
					}
				}
				u32 unit_id = unit->id;
				GroupFlag group_flags = units_dat::GroupFlags[unit_id];
				u32 playerid_2_wrong_2 = *(player_id + 2);
				switch (playerid_2_wrong_2)
				{
				case 0xE5:
				{
					return 1;
				}
				case 0xE6:
				{
					if (group_flags.is_men)
					{
						return 1;
					}
					return 0;
				}
				case 0xE7:
				{
					if (group_flags.is_building)
					{
						return 1;
					}
					return 0;
				}
				case 0xE8:
				{
					if (!(group_flags.is_factory))
					{
						return 0;
					}
					return 1;
				}
				default:
					if (unit_id == playerid_2_wrong_2)
					{
						return 1;
					}
					break;
				}
			}
			return 0;
		}
		return 1;
	}

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

	// Hooked to extend transports. -Pr0nogo
	s32 is_unit_loaded(Unit* transport) // 004E7110
	{
		u32 iterations = 0;
		u16 index = transport->loaded_unit[0].index;
		s32 result = 1;
		while (1)
		{
			u32 index_var = index;
			if (index)
			{
				u32 array_insanity = 336 * (index_var & 0x7FF);
				void* array_result = unk_array_59CB64[array_insanity / 4];
				Unit* reference = &unit_reference[array_insanity / 0x150];
				if (array_result)
				{
					if ((reference->main_order_id || reference->main_order_state != 1)
						&& reference->target_order_special == index_var >> 11)
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
		return result;
	}

	// Hooked to extend transports. -Pr0nogo
	u32 get_loaded_space_amount(Unit* transport) // 004E7170
	{
		u32 result = 0;
		u16 index_1 = transport->loaded_unit[1].index;
		u32 two_iterations = 2;
		do
		{
			u32 index_var_1 = (index_1 - 1);
			if ((index_1 - 1))
			{
				u32 array_insanity_1 = 336 * (index_var_1 & 0x7FF);
				void* array_result_1 = unk_array_59CB64[array_insanity_1 / 4];
				Unit* reference_1 = &unit_reference[array_insanity_1 / 0x150];
				if (array_result_1)
				{
					if ((reference_1->main_order_id || reference_1->main_order_state != 1)
						&& reference_1->target_order_special == index_var_1 >> 11)
					{
						result += units_dat::SpaceRequired[reference_1->id];
					}
				}
			}
			u32 index_var_2 = index_1;
			if (index_1)
			{
				u32 array_insanity_2 = 336 * (index_var_2 & 0x7FF);
				void* array_result_2 = unk_array_59CB64[array_insanity_2 / 4];
				Unit* reference_2 = &unit_reference[array_insanity_2 / 0x150];
				if (array_result_2)
				{
					if ((reference_2->main_order_id || reference_2->main_order_state != 1)
						&& reference_2->target_order_special == index_var_2 >> 11)
					{
						result += units_dat::SpaceRequired[reference_2->id];
					}
				}
			}
			u32 index_var_3 = index_1 + 1;
			if (index_1 + 1)
			{
				u32 array_insanity_3 = 336 * (index_var_3 & 0x7FF);
				void* array_result_3 = unk_array_59CB64[array_insanity_3 / 4];
				Unit* reference_3 = &unit_reference[array_insanity_3 / 0x150];
				if (array_result_3)
				{
					if ((reference_3->main_order_id || reference_3->main_order_state != 1)
						&& reference_3->target_order_special == index_var_3 >> 11)
					{
						result += units_dat::SpaceRequired[reference_3->id];
					}
				}
			}
			u32 index_var_4 = index_1 + 2;
			if (index_1 + 2)
			{
				u32 array_insanity_4 = 336 * (index_var_4 & 0x7FF);
				void* array_result_4 = unk_array_59CB64[array_insanity_4 / 4];
				Unit* reference_4 = &unit_reference[array_insanity_4 / 0x150];
				if (array_result_4)
				{
					if ((reference_4->main_order_id || reference_4->main_order_state != 1)
						&& reference_4->target_order_special == index_var_4 >> 11)
					{
						result += units_dat::SpaceRequired[reference_4->id];
					}
				}
			}
			index_1 += 4;
			--two_iterations;
		} while (two_iterations);
		return result;
	}

	// Hooked to extend transports. -Pr0nogo
	void unload_unit_ext(Unit* cargo, u32 transport_died) // 004E7E10
	{
		u32 cargo_status = cargo->status;
		Unit* cargo_subunit = 0;
		bool is_not_subunit = true;
		if (cargo_status & UnitStatus::InTransport)
		{
			if (transport_died && !cargo->connected_unit)
			{
				cargo->status = cargo_status & 0xFFFFFFBF;
				helper::toggle_unit_pathing_properties(cargo);
				cargo_subunit = cargo->subunit;
				if (!cargo_subunit)
				{
					return;
				}
				is_not_subunit = (units_dat::BaseProperty[cargo_subunit->id] & UnitProperty::Subunit) == 0;
				goto LABEL_24;
			}
			u32 cargo_index = cargo - unit_table + 1;
			u32 target_order_shifted = 0;
			if (cargo_index <= 1700)
			{
				target_order_shifted = cargo_index | (cargo->target_order_special << 11);
			}
			Unit* connected_unit = cargo->connected_unit;
			u32 iterations = 0;
			u32 iterations_2 = 0;
			if (units_dat::SpaceProvided[connected_unit->id])
			{
				u16 index = connected_unit->loaded_unit[0].index;
				do
				{
					if (index == target_order_shifted)
					{
						break;
					}
					++iterations;
					++index;
				} while (iterations < units_dat::SpaceProvided[connected_unit->id]);
				iterations_2 = iterations;
			}
			if (transport_died)
			{
				helper::move_unit_duplicate(cargo, connected_unit->sprite->position.x, connected_unit->sprite->position.y);
			}
			else
			{
				GroupFlag group_flags = units_dat::GroupFlags[connected_unit->id];
				u8 sound_offset = 0; // Zerg
				if (group_flags.is_protoss)
				{
					sound_offset = 2;
				}
				else
				{
					sound_offset = (group_flags.is_terran) != 0 ? 1 : 4;
				}
				helper::play_sound_from_unit(SoundId::Misc_ZOvTra01_wav + sound_offset, connected_unit, 1, 0);
				helper::show_and_enable_unit(cargo);
				iterations = iterations_2;
			}
			cargo->connected_unit->loaded_unit[iterations].index = 0;
			u32 cargo_status_something = cargo->status & 0xFFFFFFBF;
			cargo->connected_unit = 0;
			cargo->status = cargo_status_something;
			helper::toggle_unit_pathing_properties(cargo);
			cargo_subunit = cargo->subunit;
			if (cargo_subunit)
			{
				is_not_subunit = (units_dat::BaseProperty[cargo_subunit->id] & UnitProperty::Subunit) == 0;
			LABEL_24:
				if (!is_not_subunit)
				{
					helper::toggle_unit_pathing_properties(cargo_subunit);
				}
				return;
			}
		}
	}

	// Hooked to extend transports. -Pr0nogo
	u32 set_under_disruption_field(Unit* unit) // 004EB170
	{
		u32 status = unit->status;
		if (status & 4)
		{
			return 0;
		}
		u32 status_something = status | 0x8000;
		unit->status = status_something;
		u32 unit_id = unit->id;
		if (status_something & 0x40000000
		|| !units_dat::SpaceProvided[unit_id])
		{
			Unit* subunit = unit->subunit;
			if (subunit)
			{
				subunit->status |= 0x8000u;
			}
			return 0;
		}
		u16 index = unit->loaded_unit[0].index;
		u32 iterations = 8;
		do
		{
			u32 index_var = index;
			if (index)
			{
				u32 array_insanity = 336 * (index_var & 0x7FF);
				void* array_result = unk_array_59CB64[array_insanity / 4];
				Unit* reference = &unit_reference[array_insanity / 0x150];
				if (array_result)
				{
					if ((reference->main_order_id || reference->main_order_state != 1)
						&& reference->target_order_special == index_var >> 11)
					{
						reference->status |= 0x8000u;
					}
				}
			}
			++index;
			--iterations;
		} while (iterations);
		return 0;
	}
}