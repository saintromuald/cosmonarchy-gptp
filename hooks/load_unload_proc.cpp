#include "load_unload_proc.h"
#include <SCBW/api.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"

namespace hooks
{
	static void special_load_unit(Unit* unit, Unit* transport) {
		if (helper::spreads_creep(unit->id, unit->status & UnitStatus::Completed)) {
			helper::update_creep_disappearance_unit(unit);
		}
	}

	static void special_unload_unit(Unit* unit, Unit* transport, Point16* pos) {
		if (units_dat::base_property[unit->id] & UnitProperty::Building) {
			Point16 size = units_dat::building_dimensions[unit->id];
			pos->x = (u16)(pos->x + 16) / 32 * 32;
			pos->y = (u16)(pos->y + 16) / 32 * 32;
		}

		if (helper::spreads_creep(unit->id, unit->status & UnitStatus::Completed)) {
			scbw::spawn_custom_creep_at(*pos, unit->id, false);
			helper::set_secondary_order(unit, OrderId::SpreadCreep);
		}
	}

	void load_unit_proc(Unit* transport, Unit* cargo_unit) // 004E78E0
	{
		s32 counter = 0;		
		UnitExt* transport_ext = get_unit_ext(transport);
		if (units_dat::space_provided[transport->id] != 0) {
			for (; counter < units_dat::space_provided[transport->id]; ++counter) {
				Unit* unit = get_loaded_unit_from_index(transport, counter);
				if (!unit) {
					break;
				}
			}
		}
		
		u8 unit_race_id = transport->get_race();
		scbw::decloak_proc(transport);
		scbw::decloak_proc(cargo_unit);
		helper::play_sound_from_unit(SoundId::Misc_ZOvTra00_wav + unit_race_id, transport, 1, 0);
		if (cargo_unit->id == UnitId::TerranMason || cargo_unit->id == UnitId::TerranTalos) {
			if (cargo_unit->unit_ai) {
				AiTown* town = cargo_unit->unit_ai->worker_ai.town;
				if (town->building_scv == cargo_unit) {
					town->building_scv = 0;
				}
			}
		}

		// CCMU extender
		u32 loaded_index;
		auto new_table = g_unit_table;

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
			transport_ext->loaded_unit[counter].index = loaded_index;
			transport_ext->loaded_unit[counter].unique_id = cargo_unit->unique_id;
		} else {
			transport_ext->loaded_unit[counter].index = 0;
			transport_ext->loaded_unit[counter].unique_id = 0;
		}

		cargo_unit->status |= UnitStatus::InTransport;
		helper::hide_and_disable_unit(cargo_unit);
		if (!(units_dat::base_property[cargo_unit->id] & UnitProperty::Building)) {
			cargo_unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
		}
		special_load_unit(cargo_unit, transport);

		if (unit_id_is_garrison(transport->id))
		{
			for (s32 i = 0; i < TransportCap; i++)
			{
				Unit* cargo_unit = transport->get_loaded_unit(i);
				if (!cargo_unit) continue;
				// TODO: Disable cargo units
			}
		}

		if (transport->player == 11)
		{
			UnitExt* unit_ext = get_unit_ext(transport);
			unit_ext->values[ValueIsNeutralBunker] = 1;
			transport->give_to(cargo_unit->player);
		}		

		scbw::refresh_console();
		if (unit_id_is_garrison(transport->id))
		{
			cargo_unit->status = (cargo_unit->status & ~UnitStatus::Reacts) | UnitStatus::InBuilding;
			if (cargo_unit->subunit) {
				cargo_unit->subunit->status = (cargo_unit->status & ~UnitStatus::Reacts) | UnitStatus::InBuilding;
			}
			if (cargo_unit->path)
			{
				u32* path_0x4 = (u32*)((u32)cargo_unit->path->next);
				*path_0x4 = (cargo_unit->path - *g_path_array_start) / 128 + 1;
				if (*g_first_free_path == 0)
				{
					cargo_unit->path = 0;
				} else {
					cargo_unit->path = (UnitPath*)((*g_first_free_path - *g_path_array_start) / 128 + 1);
				}
				*g_first_free_path = cargo_unit->path;
				cargo_unit->path = 0;
			}

			cargo_unit->movement_state = 0;
			if (cargo_unit->sprite->elevation < 12)
			{
				cargo_unit->pathing_flags |= 1;
			} else {
				cargo_unit->pathing_flags &= ~1;
			}

			if (cargo_unit->subunit && units_dat::base_property[cargo_unit->subunit->id] & UnitProperty::Subunit)
			{
				Unit* subunit = cargo_unit->subunit;
				scbw::set_unit_position(subunit, transport->sprite->position.x, transport->sprite->position.y);
				subunit->status = (subunit->status & ~UnitStatus::Reacts) | UnitStatus::InBuilding;
				if (subunit->path)
				{
					u32* path_0x4 = (u32*)((u32)subunit->path->next);
					*path_0x4 = (subunit->path - *g_path_array_start) / 128 + 1;
					if (*g_first_free_path == 0)
					{
						subunit->path = 0;
					} else {
						subunit->path = (UnitPath*)((*g_first_free_path - *g_path_array_start) / 128 + 1);
					}
					*g_first_free_path = subunit->path;
					subunit->path = 0;
				}

				subunit->movement_state = 0;
				if (subunit->sprite->elevation < 12)
				{
					subunit->pathing_flags |= 1;
				} else {
					subunit->pathing_flags &= ~1;
				}
			} else {
				scbw::set_unit_position(cargo_unit, transport->sprite->position.x, transport->sprite->position.y);
			}
		}
	}

	b32 unload_unit_proc(Unit* unit) // 004E7F70
	{
		b32 return_value = 0;
		if (unit->connected_unit)
		{
			Unit* transport = unit->connected_unit;
			if ((transport->main_order_timer == 0
			|| (units_dat::base_property[transport->id] & UnitProperty::Building && !(transport->status & UnitStatus::InAir))) // For Bunkers
			&& !transport->is_frozen())
			{
				Point16 pos = {0, 0};
				// probably check where unit can be spawned
				if (helper::can_unload(transport, unit, &pos) != 0)
				{
					if (transport->id == UnitId::TerranTrojan)
					{
						transport->main_order_timer = 7;
					} else {
						transport->main_order_timer = 15;
					}

					scbw::decloak_proc(unit);
					scbw::decloak_proc(transport);

					special_unload_unit(unit, transport, &pos);
					scbw::set_unit_position(unit, pos.x, pos.y);
					if (unit->subunit)
					{
						scbw::set_unit_position(unit->subunit, pos.x, pos.y);
					}

					helper::ignore_iscript_go_to_idle(unit);
					helper::unload_unit_ext(unit, 0);
					if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
						if (!unit->unit_ai)
						{
							unit->order_computer(units_dat::return_to_idle_order[unit->id]);
						} else {
							unit->order_computer(OrderId::ComputerAI);
						}
					}

					UnitExt* transport_ext = get_unit_ext(transport);
					if (transport_ext->values[ValueIsNeutralBunker])
					{
						if (!transport->get_first_loaded_unit())
						{
							transport->give_to(11);
							transport->sprite->player = 11;
						} else {
							bool is_owner_valid = true;
							for (u32 i = 0; i < 4; ++i)
							{
								Unit* cargo_unit = transport->get_loaded_unit(i);
								if (cargo_unit && cargo_unit->player != unit->player)
								{
									is_owner_valid = false;
								}
							}
							if (!is_owner_valid)
							{
								transport->give_to(transport->get_first_loaded_unit()->player);
							}
						}
					}

					scbw::refresh_console();
					return_value = 1;
					if (units_dat::base_property[transport->id] & UnitProperty::Building)
					{
						if (unit->id != UnitId::ProtossAccantorOld)
						{
							u8 weapon_id = unit->get_ground_weapon();
							if (weapon_id != WeaponId::None)
							{
								unit->ground_weapon_cooldown = 2; // helper::get_weapon_cooldown(unit, weapon_id);
							}
							weapon_id = unit->get_air_weapon();
							if (weapon_id != WeaponId::None)
							{
								unit->air_weapon_cooldown = 2; // helper::get_weapon_cooldown(unit, weapon_id);
							}
							unit->spell_cooldown = 30;
						} else {
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
		u32 pathing = cargo->status;
		if (cargo->status & UnitStatus::InTransport)
		{
			Unit* cargo_transport = cargo->connected_unit;
			if (cargo_transport)
			{
				UnitExt* cargo_transport_ext = get_unit_ext(cargo_transport);
				u32 cargo_index = cargo - g_unit_table + 1;
				u32 loaded_index = 0;
				if (cargo_index <= UnitLimit)
				{
					loaded_index = cargo_index | (cargo->unique_id << 12);
				}
				u32 i = 0;
				if (units_dat::space_provided[cargo_transport->id])
				{
					for (i = 0; i < units_dat::space_provided[cargo_transport->id]; ++i)
					{
						u16 index = cargo_transport_ext->loaded_unit[i].index;
						if (index == loaded_index)
						{
							break;
						}
					}
				}
				helper::move_unit_duplicate(cargo, cargo_transport->sprite->position.x, cargo_transport->sprite->position.y);
				cargo_transport_ext->loaded_unit[i].index = 0;
				u32 cargo_status_bool = cargo->status & ~UnitStatus::InTransport;
				cargo->connected_unit = 0;
				cargo->status = cargo_status_bool;
				helper::toggle_unit_pathing_properties(cargo);
				if (cargo->subunit)
				{
					if (units_dat::base_property[cargo->subunit->id] & UnitProperty::Subunit)
					{
						pathing = helper::toggle_unit_pathing_properties(cargo->subunit);
					}
				}
			} else {
				cargo->status = pathing & ~UnitStatus::InTransport;
				helper::toggle_unit_pathing_properties(cargo);
				if (cargo->subunit)
				{
					if (units_dat::base_property[cargo->subunit->id] & UnitProperty::Subunit)
					{
						pathing = helper::toggle_unit_pathing_properties(cargo->subunit);
					}
				}
			}
		}
		return pathing;
	}

	// Hooked to extend transports. -Pr0nogo
	void kill_all_loaded_units(Unit* transport) { // 0049FDD0
		UnitExt* transport_ext = get_unit_ext(transport);
		char transport_data = 0;
		if (transport->status & UnitStatus::InTransport) {
			transport_data = update_unit_transport_data(transport, 0);
		} else if (!(transport->status & UnitStatus::IsHallucination)) {
			// Originally had a check for the Ovileth transport upgrade. We enable it by default, so BYE!
			u8 transport_space = units_dat::space_provided[transport->id];
			if (transport_space) {
				for (u32 i = 0; i < TransportCap; ++i) {
					Unit* unit = get_loaded_unit_from_index(transport, i);
					if (unit) {
						transport->main_order_timer = 0;
						if (helper::unload_unit_proc(unit)) {
							if (!unit_id_is_garrison(transport->id) || transport_ext->timers[TimerEmergencyDetonation] == 1) {
								helper::orders_self_destructing(unit); // Just kill it! -Veeq7
							}
						} else {
							helper::orders_self_destructing(unit);
						}
					}
					transport_ext->loaded_unit[i].value = 0;
				}
			}
		}
	}

	// Hooked to extend transports. -Pr0nogo
	s32 get_doodad_state_unit(Unit* unit, FindUnitLocationParam* parameter) { // 004C6F70
		u32* player_id = &parameter->player;
		u32 do_check_height = parameter->check_height;
		FindUnitLocationParam* parameter_var = parameter;
		if (do_check_height && helper::is_unit_at_height(unit, parameter->location_flags)) {
			return 0;
		}

		u32 playerid_1_wrong = player_id[1];
		u32 playerid_2_wrong = player_id[2];
		FindUnitLocationParam param_2 = {};
		param_2.player = *player_id;
		u32 unit_id_2 = unit->id;
		param_2.check_height = playerid_2_wrong;
		u32 iterations = 0;
		*(u32*)& param_2.unit_id = playerid_1_wrong;
		param_2.check_height = 0;

		if (!(units_dat::base_property[unit_id_2] & UnitProperty::Worker)
		||	 !unit->building.addon
		||   !get_doodad_state_unit(unit->building.addon, &param_2))
		{
			if (helper::is_unit_owned_by_player_ex(*player_id, unit))
			{
				if (helper::unit_is_active_transport(unit))
				{
					u32 unit_id_3 = unit->id;
					u32 space_provided_1 = units_dat::base_property[unit_id_3];
					u32 space_provided_2 = units_dat::space_provided[unit_id_3];
					if (space_provided_1)
					{
						UnitExt* unit_ext = get_unit_ext(unit);
						u16 index = unit_ext->loaded_unit[0].index;
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
				GroupFlag group_flags = units_dat::group_flags[unit_id];
				u32 playerid_2_wrong_2 = *(u16*)(player_id + 2);
				switch (playerid_2_wrong_2)
				{
					case 0xE5:
					{
						return 1;
					} break;
					case 0xE6:
					{
						if (group_flags.is_men)
						{
							return 1;
						}
						return 0;
					} break;
					case 0xE7:
					{
						if (group_flags.is_building)
						{
							return 1;
						}
						return 0;
					} break;
					case 0xE8:
					{
						if (!(group_flags.is_factory))
						{
							return 0;
						}
						return 1;
					} break;
					default:
					{
						if (unit_id == playerid_2_wrong_2)
						{
							return 1;
						}
					} break;
				}
			}
			return 0;
		}
		return 1;
	}

	b32 call_kill_unit_args_func(void* func, u32 player, u32 unit_id, Unit* unit) {
		static b32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV ECX, player
			MOV EDX, unit_id
			CALL func
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// Hooked to extend transports. -Pr0nogo
	bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args) { // 004C7E20
		if (!unit) {
			return 0;
		}
		if (args->check_height && helper::is_unit_at_height(unit, args->flags)) {
			return 0;
		}

		KillUnitArgs* args_func = (KillUnitArgs*)&args->func;
		if (!(unit->status & UnitStatus::IsHallucination)) {
			u8 space_provided = units_dat::space_provided[unit->id];
			if (space_provided) {
				for (u32 i = 0; i < space_provided; ++i) {
					Unit* unit_reference = get_loaded_unit_from_index(unit, i);
					*trig_killing_additional_unit = 1;
					helper::kill_units_at_location_proc(unit_reference, args_func); // Was &args_func, now the address of is in the var
					*trig_killing_additional_unit = 0;
				}
			}
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
			Unit* powerup = unit->worker.powerup;
			if (powerup) {
				*trig_killing_additional_unit = 1;
				helper::kill_units_at_location_proc(powerup, args_func); // Was &args_func, now the address of is in the var
				*trig_killing_additional_unit = 0;
			}
		}
		if (!unit->sprite) {
			return 0;
		}
		if (!unit->main_order_id) {
			return 0;
		}
		if (!call_kill_unit_args_func(args->func, args->player, args->unit_id, unit)) {
			return 0;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::NeutralAccessories && unit->powerup_carrier) {
			helper::kill_powerup(unit->powerup_carrier);
		} else {
			if (!(unit->status & UnitStatus::IsHallucination) && units_dat::space_provided[unit->id]) {
				if (*trig_remove_unit_active) {
					unit->user_action_flags |= 4u;
				}
			} else if (*trig_remove_unit_active) {
				helper::hide_unit(unit);
			}
			helper::orders_self_destructing(unit);
		}
		if (!(*trig_killing_additional_unit)) {
			--(*trig_kill_unit_count);
		}
		return *trig_kill_unit_count == 0;
	}

	// Hooked to extend transports. -Pr0nogo
	Unit* get_loaded_unit_from_index(Unit* transport, u32 index) { // 004E6C40
		UnitExt* transport_ext = get_unit_ext(transport);
		u32 unit_index = transport_ext->loaded_unit[index].index;
		if (!unit_index) {
			return 0;
		}
		Unit* unit = &g_unit_table[(unit_index & UnitIDMask) - 1];
		if (!unit->sprite) {
			return 0;
		}
		if (unit->main_order_id == OrderId::Die && unit->main_order_state == 1) {
			return 0;
		}
		if (transport_ext->loaded_unit[index].unique_id != unit->unique_id) {
			return 0;
		}
		return unit;
	}

	// Hooked to extend transports. -Pr0nogo
	// Logic moved from Unit.cpp
	Unit* get_first_loaded_unit(Unit* transport) // 004E6C90
	{
		for (s32 i = 0; i < units_dat::space_provided[transport->id]; ++i)
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
		UnitExt* transport_ext = get_unit_ext(transport);
		Unit* loaded_unit = get_unit_from_index(transport_ext->loaded_unit[index].index);
		if  (loaded_unit && loaded_unit->sprite
		&& !(loaded_unit->main_order_id == OrderId::Die && loaded_unit->main_order_state == 1)
		&&   loaded_unit->unique_id == transport_ext->loaded_unit[index].unique_id)
		{
			return loaded_unit;
		}
		return nullptr;
	}

	// Moved from Unit.cpp
	Unit* get_unit_from_index(u16 index)
	{
		if (1 <= index && index <= UNIT_ARRAY_LENGTH)
		{
			return &g_unit_table[index - 1];
		}
		return nullptr;
	}

	// Hooked to extend transports. -Pr0nogo
	u32 iterate_loaded_units(Unit* transport, iterate_loaded_units_callback* func, Unit* cargo) { // 004E6D00
		u32 space_provided = units_dat::space_provided[transport->id];
		if (!space_provided) {
			return false;
		}
		for (u32 i = 0; i < space_provided; ++i) {
			Unit* unit = get_loaded_unit_from_index(transport, i);
			if (unit && func(unit, cargo)) {
				return true;
			}
		}
		return false;
	}

	// Hooked to extend transports. -Pr0nogo
	s32 get_first_loaded_worker(Unit* transport) { // 004E6D90
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* unit = get_loaded_unit_from_index(transport, i);
			if (unit && units_dat::base_property[unit->id] & UnitProperty::Worker) {
				return true;
			}
		}
		return false;
	}

	// Hooked to extend transports. -Pr0nogo
	s32 is_carrying_flag(Unit* unit) { // 004F3A80
		u16 unit_id = unit->id;
		if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
			Unit* powerup = unit->worker.powerup;
			if (powerup && powerup->id == UnitId::Powerup_Flag) {
				return 1;
			}
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination
			|| !units_dat::space_provided[unit_id]) {
			return 0;
		}

		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* flag = get_loaded_unit_from_index(unit, i);
			if (flag && flag->id == UnitId::Powerup_Flag) {
				return true;
			}
		}
		return false;
	}

	// Hooked to extend transports. -Pr0nogo
	s32 is_unit_loaded(Unit* transport) { // 004E7110
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* unit = get_loaded_unit_from_index(transport, i);
			if (unit) {
				return true;
			}
		}
		return false;
	}

	// Hooked to extend transports. -Pr0nogo
	u32 get_loaded_space_amount(Unit* transport) { // 004E7170
		u32 result = 0;
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* unit = get_loaded_unit_from_index(transport, i);
			if (unit) {
				result += units_dat::space_required[unit->id];
			}
		}
		return result;
	}

	// Hooked to extend transports. -Pr0nogo
	void unload_unit_ext(Unit* cargo, u32 transport_died) // 004E7E10
	{
		u32 cargo_status = cargo->status;
		Unit* cargo_subunit = 0;
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
				goto LABEL_24;
			}
			u32 cargo_index = cargo - g_unit_table + 1;
			u32 target_order_shifted = 0;
			if (cargo_index <= UnitLimit)
			{
				target_order_shifted = cargo_index | (cargo->unique_id << 12);
			}
			Unit* connected_unit = cargo->connected_unit;
			UnitExt* connected_unit_ext = get_unit_ext(connected_unit);
			u32 iterations = 0;
			u32 iterations_2 = 0;
			if (units_dat::space_provided[connected_unit->id])
			{
				u16* index = &connected_unit_ext->loaded_unit[0].value;
				do
				{
					if (*index == target_order_shifted)
					{
						break;
					}
					++iterations;
					++index;
				} while (iterations < units_dat::space_provided[connected_unit->id]);
				iterations_2 = iterations;
			}
			if (transport_died)
			{
				helper::move_unit_duplicate(cargo, connected_unit->sprite->position.x, connected_unit->sprite->position.y);
			}
			else
			{
				u8 sound_offset = connected_unit->get_race();
				helper::play_sound_from_unit(SoundId::Misc_ZOvTra01_wav + sound_offset, connected_unit, 1, 0);
				helper::show_and_enable_unit(cargo);
				iterations = iterations_2;
			}
			connected_unit_ext->loaded_unit[iterations].value = 0;
			u32 cargo_status_something = cargo->status & 0xFFFFFFBF;
			cargo->connected_unit = 0;
			cargo->status = cargo_status_something;
			helper::toggle_unit_pathing_properties(cargo);
			cargo_subunit = cargo->subunit;
			if (cargo_subunit)
			{
			LABEL_24:
				if (units_dat::base_property[cargo_subunit->id] & UnitProperty::Subunit)
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
		u32 status_something = status | UnitStatus::CanNotAttack;
		unit->status = status_something;
		u32 unit_id = unit->id;
		if (status_something & UnitStatus::IsHallucination
		|| !units_dat::space_provided[unit_id])
		{
			Unit* subunit = unit->subunit;
			if (subunit)
			{
				subunit->status |= UnitStatus::CanNotAttack;
			}
			return 0;
		}
		for (u32 i = 0; i < TransportCap; ++i) {
			Unit* cargo = get_loaded_unit_from_index(unit, i);
			if (cargo) {
				cargo->status |= 0x8000u;
			}
		}
		return 0;
	}
}