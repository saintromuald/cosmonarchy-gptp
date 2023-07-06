#include "target_related.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/unit_stats/weapon_range.h>
#include <globals.h>
#include "byte_operations_utils.h"
#include <limits.h>
#include "hooks/audiovisuals/sfxhooks.h"

namespace hooks
{
	bool is_unit_target_out_of_max_range(Unit* a1, Unit* a2) // 00476430
	{
		Unit* _unit;
		Unit* finalUnit;
		unsigned __int8 _wpn;
		__int16 _id;
		s32 _maxRange;
		s32 v8;

		_unit = a1;
		if (!is_target_visible(a2, a1))
		{
			return 1;
		}
		finalUnit = _unit->subunit;
		if (!finalUnit || !(units_dat::group_flags[(unsigned __int16)finalUnit->id].is_building))
		{
			finalUnit = _unit;
		}
		if (a2->status & UnitStatus::InAir)
		{
			_wpn = units_dat::air_weapon[(unsigned __int16)finalUnit->id];
		}
		else
		{
			_id = finalUnit->id;
			if (_id != UnitId::ZergLakizilisk || finalUnit->status & 0x10)
			{
				_wpn = units_dat::ground_weapon[_id];
			}
			else
			{
				_wpn = -126;
			}
		}
		if (_wpn == -126)
		{
			return 0;
		}

		_maxRange = helper::get_unit_max_weapon_range(_unit, _wpn);
		ByteOperations::SetLowByte(v8, helper::is_target_within_min_movement_range(a2, finalUnit, _maxRange));
		return v8 == 0;
	}

	s32 is_ready_to_attack(Unit* a1, unsigned __int8 a2) // 00476640
	{
		Unit* v2; // esi
		Unit* v4; // ecx
		u32 v5; // eax
		u32 v6; // eax
		u32 v7; // edi
		__int16 v8; // cx

		v2 = a1;
		if (a1->movement_flags & 8)
			return 0;
		v4 = a1->order_target.unit;
		if (v4)
			v5 = helper::get_distance_to_unit(a1, v4);
		else
			v5 = helper::distance_from_dbox_corner(a1, a1->order_target.pt.x, a1->order_target.pt.y);
		v7 = v5;
		v6 = weapons_dat::min_range[a2];
		if (v6 && v7 < v6 || v7 > helper::get_unit_max_weapon_range(v2, a2) || v2->pathing_flags & 2)
			return 0;
		v8 = v2->next_target_waypoint.x;
		if ((v8 != v2->position.x || v2->next_target_waypoint.y != v2->position.y)
			&& !helper::is_angle_in_attack_angle(v2->next_target_waypoint.y, v8, v2, a2))
		{
			v2->order_queue_timer = 0;
			return 0;
		}
		return 1;
	}
	
	// Hooked to fix hold position and cliff advantage. -Pr0nogo
	bool is_unit_in_weapon_range(Unit* target, Unit* attacker) // 00476870
	{
		Unit* final_target = target;
		if (!target)
		{
			final_target = attacker->order_target.unit;
			if (!final_target)
			{
				return 1;
			}
		}
		if (!is_target_visible(final_target, attacker))
		{
			return 0;
		}

		Unit* final_attacker = attacker->subunit;
		if (!final_attacker || !(units_dat::base_property[final_attacker->id] & UnitProperty::Subunit))
		{
			final_attacker = attacker;
		}

		u32 weapon_id = WeaponId::None;
		if (final_target->status & UnitStatus::InAir)
		{
			weapon_id = units_dat::air_weapon[final_attacker->id];
		} else {
			u32 attacker_id = final_attacker->id;
			if (attacker_id != UnitId::ZergLakizilisk || final_attacker->status & UnitStatus::Burrowed)
			{
				weapon_id = units_dat::ground_weapon[attacker_id];
			} else {
				weapon_id = WeaponId::None;
			}
		}

		if (weapon_id == WeaponId::None)
		{
			return 0;
		}
		u32 min_range = weapons_dat::min_range[weapon_id];
		if (min_range)
		{
			if (helper::is_target_within_min_range(final_attacker, min_range, final_target))
			{
				return 0;
			}
		}
		u32 max_range = hooks::get_max_weapon_range(attacker, weapon_id, final_target);
		return helper::is_target_within_min_movement_range(final_target, final_attacker, max_range);
	}

	char can_cast_ordered_spell_now(Unit* a1) // 00491DB0
	{
		Unit* v1; // esi
		u32 v2; // eax
		Unit* v3; // edi
		s32 v4; // ebx
		s32 v5; // eax
		s32 v6; // edi
		s32 v7; // ST14_4
		s32 v8; // ebx
		u32 v9; // ebx

		v1 = a1;
		v2 = is_unit_visible(a1);
		if (v2)
		{
			v3 = v1->order_target.unit;
			if (v3)
			{
				v4 = helper::get_unit_max_weapon_range(v1, orders_dat::order_weapon_id[(unsigned __int8)v1->main_order_id]);
				if (v1->movement_flags & 2)
				{
					if (!(v3->movement_flags & 2))
						goto LABEL_14;
					v5 = (unsigned __int8)(v3->velocity_direction_1 - v1->velocity_direction_1);
					if (v5 > 128)
						v5 = 256 - v5;
					if (v5 > 32)
						LABEL_14:
					v4 += helper::get_flingy_halt_distance(v1) >> 8;
				}
				//ByteOperations::SetLowByte((UINT32)v2, (UINT8)is_target_within_min_range(v1, v4, v3)); // Needs fixing
			}
			else
			{
				v6 = v1->order_target.pt.y;
				v7 = v1->order_target.pt.x;
				v8 = helper::get_unit_max_weapon_range(v1, orders_dat::order_weapon_id[(unsigned __int8)v1->main_order_id]);
				v9 = (helper::get_flingy_halt_distance(v1) >> 8) + v8;
				v2 = v9 >= helper::get_distance_fast(v1->halt.x, v7 << 8, v1->halt.y, v6 << 8) >> 8;
			}
		}
		return v2;
	}

	void orders_watch_target(Unit* a1) // 0047BAB0
	{
		Unit* v1; // esi
		Unit* v2; // ebx
		s32 v3; // eax
		Unit* v4; // ebx
		Unit* v5; // edi
		unsigned __int8 v6; // bl
		unsigned __int16 v7; // ax
		u32 v8; // eax
		s32 v9; // eax
		s32 v10; // eax
		Unit* target; // [esp+Ch] [ebp-4h]

		v1 = a1;
		v2 = a1->order_target.unit;
		if (!v2)
		{
			goto LABEL_22;
		}
		ByteOperations::SetLowByte(v3, helper::unit_can_attack_target(v2, a1, 1));
		if (!v3)
		{
			goto LABEL_22;
		}
		v4 = v1->order_target.unit;
		target = v1->order_target.unit;
		if (v4)
		{
			if (!is_target_visible(v4, v1))
			{
				goto LABEL_23;
			}
			v5 = v1->subunit;
			if (!v5 || !(units_dat::group_flags[(unsigned __int16)v5->id].is_building))
			{
				v5 = v1;
			}
			if (v4->status & 4)
			{
				v6 = units_dat::air_weapon[(unsigned __int16)v5->id];
			}
			else
			{
				v7 = v5->id;
				if (v7 != 103 || v5->status & 0x10)
				{
					v6 = units_dat::ground_weapon[v7];
				}
				else
				{
					v6 = 0x82;
				}
			}
			if (v6 >= (unsigned __int8)0x82
				|| (v8 = weapons_dat::min_range[v6]) != 0 && helper::is_target_within_min_range(v5, v8, target)
				|| (v9 = helper::get_unit_max_weapon_range(v1, v6), ByteOperations::SetLowByte(v10, (helper::is_target_within_min_movement_range(target, v5, v9), !v10))))
			{
			LABEL_23:
				if (helper::get_right_click_action_order(v1) == 3)
			LABEL_22:
				{
					helper::unknown_orders_func((s32)v1, v1->order_target.pt.x, v1->order_target.pt.y);
				}
			}
		}
	}

	s32 generic_right_click_unit_proc_2(Unit* unit, s32 a2, s16 a3, Unit* target, s16 unitID) // 00455E00
	{
		s32 result = 0;
		if (unit->status & UnitStatus::GroundedBuilding)
		{
			if (unit->is_factory())
			{
				result = OrderId::RallyPointToTile - (target != 0);
			} else {
				result = OrderId::Nothing2;
			}
			return result;
		}
		if (!target) {
			return OrderId::Move;
		}
		if (helper::unit_is_active_transport(unit) && helper::is_order_valid(OrderId::Pickup2, unit, 0, target, 0)) {
			return OrderId::Pickup2;
		}
		if (helper::unit_is_active_transport(target) && helper::can_enter_transport(target, unit)) {
			return OrderId::EnterTransport;
		}
		if (target->status & UnitStatus::Burrowed) {
			return OrderId::Move;
		}
		if (target->status & UnitStatus::Invincible || helper::get_unit_ability_error_stat_string(target, unit->player, 12) != 0)
		{
			result = (unit->id != UnitId::TerranCleric ? 0 : 0x80) + OrderId::Follow;
		} else {
			result = OrderId::InfestMine2;
		}
		return result;
	}

	Unit* cmdrecv_hold_position(s32 a1) // 004C20C0
	{
		*((s8*)&g_map_height_pixels + 2) = 0;
		Unit* result = helper::get_active_player_next_selection();
		for (Unit* i = result; result; i = result)
		{
			u32 unit_id = i->id;
			u32 order_id = 0;
			if (unit_id != UnitId::ProtossSolarion)
			{
				if (unit_id != UnitId::ProtossAccantorOld)
				{
					if (unit_id == UnitId::TerranCleric)
					{
						order_id = OrderId::MedicHoldPosition;
					} else {
						order_id = OrderId::HoldPosition4;
					}
				} else {
					order_id = OrderId::ReaverHold;
				}
			} else {
				order_id = OrderId::HoldPosition1;
			}
			if (helper::order_allowed(i, order_id, *g_active_nation_id) == 1)
			{
				helper::targeted_order(0, UnitId::None, i, 0, 1, *(s8*)(a1 + 1) != 0, order_id, 0, *g_order_type_4_position, *g_order_type_4_target);
			}
			result = helper::get_active_player_next_selection();
		}
		return result;
	}

	u32 unit_can_see_cloaked_target(Unit* target, Unit* unit) // 00401D60, hooked for cloak rewrite - Pr0nogo
	{
		return target->status & 0x300 && !((1 << unit->player) & target->detection_status);
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	u32 is_target_visible(Unit* target, Unit* unit) // 004E5E30
	{
		s32 player = 1 << unit->player;
		if (!target)
		{
			return player & ~(g_active_tile_array[unit->order_target.pt.x / 32 + g_map_tile_size->width * (unit->order_target.pt.y / 32)]->visibility_flags);
		}

		u32 result;
		if (!(target->status & 0x300) && (1 << unit->player) & target->detection_status)
		{
			result = (u8)(player & target->sprite->visibility_status);
		} else {
			result = 0;
		}
		return result;
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	u32 is_unit_visible(Unit* unit) // 004E5DB0
	{
		Unit* target;
		s32 playerUnit;
		u32 result;

		target = unit->order_target.unit;
		playerUnit = 1 << unit->player;
		if (!target)
		{
			return playerUnit & ~(g_active_tile_array[unit->order_target.pt.x / 32 + g_map_tile_size->width * (unit->order_target.pt.y / 32)]->visibility_flags);
		}

		if (unit->status & 0x300)
		{
			result = (u8)(playerUnit & target->sprite->visibility_status);
		}
		else
		{
			result = 0;
		}
		return result;
	}

	void update_unit_visibility(Unit* unit) {
		if (*player_visions & unit->detection_status) {
			helper::make_sprite_visible(unit->sprite);
			Unit* subunit = unit->subunit;
			if (subunit) {
				helper::make_sprite_visible(subunit->sprite);
			}
		} else {
			if (unit->sprite->flags & SpriteFlags::Selected)
			{
				helper::update_building_land_unit_selection(unit->detection_status, unit);
			}
			helper::remove_selection_circle(unit->sprite);
			helper::make_sprite_invisible(unit->sprite);
			Unit* subunit = unit->subunit;
			if (subunit) {
				helper::make_sprite_invisible(subunit->sprite);
			}
		}
	}

	// Hooked to help with cloak rewrite - Pr0nogo
	void check_unit_visibility(Unit* unit) { // 00443390
		u32 detecting_players = helper::get_detecting_players(unit);
		if (unit->status & UnitStatus::Burrowed) {
			Box32 rect = {};
			rect.left	= unit->get_left()	 - 16;
			rect.right	= unit->get_right()	 + 16;
			rect.top	= unit->get_top()	 - 16;
			rect.bottom = unit->get_bottom() + 16;

			EntryArray units = {};
			unit_search_rect(&units, rect);
			for (u32 i = 0; i < units.count; ++i) {
				Unit* found_unit = units.units[i];
				if (found_unit->status & UnitStatus::InAir || found_unit->player > PlayerLimit) {
					continue;
				}
				detecting_players |= (1 << found_unit->player) | player_vision->flags[found_unit->player];
			}
		}

		if (unit->detection_status != detecting_players) {
			if (unit->detection_status == 0x80000000) {
				unit->detection_status = ~(u8)detecting_players;
			}
			u32 delta_flags = unit->detection_status ^ detecting_players;
			unit->detection_status = detecting_players;

			u32 was_hidden = delta_flags & ~detecting_players;
			if (was_hidden) {
				helper::iterate_player_units_something(was_hidden, unit);
			}
			if (*player_visions & delta_flags) {
				update_unit_visibility(unit);
			}
		}

	}

	// Hooked to complete the cloak rewrite - Pr0nogo
	s32 is_unit_cloaking(Unit* unit) // 004916E0
	{
		if (unit->is_cloaked)
		{
			return 0;
		}

		s32 redraw = 0;
		if (*g_first_invisible_unit == unit)
		{
			*g_first_invisible_unit = unit->burrow_link.next;
			redraw = 1;
		}

		Unit* previousInvisibleUnit = unit->burrow_link.prev;
		if (previousInvisibleUnit)
		{
			previousInvisibleUnit->burrow_link.next = unit->burrow_link.next;
			redraw = 1;
		}

		Unit* nextInvisibleUnit = unit->burrow_link.next;
		if (nextInvisibleUnit)
		{
			nextInvisibleUnit->burrow_link.prev = unit->burrow_link.prev;
			redraw = 1;
		}

		unit->status &= 0xFFFFF7FF;
		unit->burrow_link.prev = 0;
		unit->burrow_link.next = 0;

		if (redraw)
		{
			*g_force_button_refresh = 1;
			g_portrait_redraw[0] = (s8)1;
			*g_status_screen_redraw = 1;
			*g_control_under_mouse = 0;
			*g_control_under_mouse_value = 0;
		}
		return 1;
	}

	bool unit_can_attack_target(Unit* target, Unit* attacker, int check_detection) // 00476730
	{
		if (attacker->is_frozen()
		|| !target
		|| !target->sprite // new
		||  target->status & UnitStatus::Invincible
		||  target->sprite->flags & SpriteFlags::Hidden
		||  check_detection && helper::unit_can_see_cloaked_target(target, attacker))
		{
			return false;
		}

		if (units_dat::base_property[attacker->id] & UnitProperty::Building)
		{
			if (!(attacker->status & UnitStatus::GroundedBuilding)) {
				return false;
			}
		}

		Unit* context = attacker;
		if (helper::unit_is_subunit(attacker->subunit)) {
			context = attacker->subunit;
		}

		if (helper::unit_is_carrier(attacker)) {
			return true;
		}

		if (helper::unit_is_reaver(attacker)) {
			if (!(target->status & UnitStatus::InAir)) {
				if (helper::unit_has_path_to_target(attacker, target)) {
					return true;
				}
			}
			return false;
		}

		// Originally had code related to infestation, and to disable AI Didact attacks

		// GAME RULE: Ground titanic units can be targeted by air-only weapons
		// Doesn't work currently.
		/*
		if (units_dat::size_type[target->id] == UnitSize::Titanic) {
			return units_dat::air_weapon[context->id] != WeaponId::None
				|| units_dat::ground_weapon[context->id] != WeaponId::None;
		}
		*/

		if (unit_id_is_bunker(attacker->id)) {
			u32 max_range = 0;
			for (u32 i = 0; i < TransportCap; ++i) {
				Unit* cargo_unit = attacker->get_loaded_unit(i);
				if (cargo_unit) {
					u32 weapon = target->status & UnitStatus::InAir ? cargo_unit->get_air_weapon() : cargo_unit->get_ground_weapon();
					u32 range = hooks::get_max_weapon_range(attacker, weapon, target);
					if (range > max_range) {
						max_range = range;
					}
				}
			}

			if (attacker->is_target_within_min_range(target, max_range)) {
				return true;
			}
			return false;
		}

		bool air_unit = target->status & UnitStatus::InAir;
		u32 weapon = air_unit ? context->get_air_weapon() : context->get_ground_weapon();
		if (air_unit && scbw::unit_id_has_custom_air_weapon(attacker->id)) {
			weapon = scbw::get_custom_secondary_weapon_from_unit_id(attacker->id);
		}
		if (attacker->status & UnitStatus::GroundedBuilding || attacker->status & UnitStatus::Burrowed || attacker->id == UnitId::TerranPhalanxSiegeMode) {
			if (!attacker->is_target_within_min_range(target, hooks::get_max_weapon_range(attacker, weapon, target))) {
				return false;
			}
		}
		return weapon != WeaponId::None;
	}

	// Hooked to help with the Titanic count game rule - Pr0nogo
	void ai_attack_unit(Unit* unit) // 0043FFD0
	{
		if (!unit)
		{
			return;
		}

		u16 unit_id = unit->id;
		if (!unit_id || unit_id == 32)
		{
			Unit* unit_target = unit->order_target.unit;
			if (unit_target)
			{
				s32 cooldown = 0;
				if (unit->stim_timer)
				{
					cooldown = 1;
				}
				if (unit->status & UnitStatus::SpeedUpgrade)
				{
					++cooldown;
				}
				if (unit->ensnare_timer)
				{
					--cooldown;
				}
				if (cooldown <= 0)
				{
					s32 unit_hp = units_dat::max_hitpoints[unit_id] >> 8;
					if (!unit_hp)
					{
						unit_hp = (unit->hitpoints + 255) >> 8;
						if (!unit_hp)
						{
							unit_hp = 1;
						}
					}
					if ((unit->hitpoints + 255) >> 8 >= unit_hp / 2)
					{
						if (unit_can_attack_target(unit, unit_target, 1))
						{
							unit_target = unit->order_target.unit;
							if (!unit_target)
							{
								if (helper::tech_use_allowed(unit, 0, unit->player) == 1
								&&  helper::get_current_hp_plus_shields(unit_target) >= 100)
								{
									helper::stimpack_proc(unit);
								}
								return;
							}
							if (helper::is_target_visible(unit_target, unit))
							{
								Unit* subunit = unit->subunit;
								if (!subunit || !(units_dat::base_property[subunit->id] & UnitProperty::Subunit))
								{
									subunit = unit;
								}

								u8 air_weapon;
								if (unit_target->status & 4)
								{
									air_weapon = units_dat::air_weapon[subunit->id];
								} else {
									u16 subunit_id = subunit->id;
									if (subunit_id != UnitId::ZergLakizilisk || subunit->status & 0x10)
									{
										air_weapon = units_dat::ground_weapon[subunit_id];
									} else {
										air_weapon = WeaponId::None;
									}
								}
								if (air_weapon != WeaponId::None)
								{
									u32 air_min_range = weapons_dat::min_range[air_weapon];
									if (!air_min_range || !(helper::is_target_within_min_range(subunit, air_min_range, unit_target)))
									{
										s32 air_max_range = weapons_dat::max_range[air_weapon];
										if (helper::is_target_within_min_movement_range(unit_target, subunit, air_max_range))
										{
											if (helper::tech_use_allowed(unit, 0, unit->player) == 1
											&&  helper::get_current_hp_plus_shields(unit_target) >= 100)
											{
												helper::stimpack_proc(unit);
											}
											return;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked to add nullcheck. -Pr0nogo
	s32 return_interceptor_if_cannot_attack(Unit* hangar_unit) // 00465810
	{
		s32 result = 0;
		Unit* steward_target = hangar_unit->order_target.unit;
		if (steward_target && !(steward_target->status & UnitStatus::Invincible) || helper::attack_recent_attacker(hangar_unit))
		{
			Unit* unit_with_hangar = hangar_unit->hangar_unit.parent;
			steward_target = hangar_unit->order_target.unit;
			if (steward_target->sprite) {
				hangar_unit->order_target.pt.x = steward_target->sprite->position.x;
				hangar_unit->order_target.pt.y = steward_target->sprite->position.y;
			}
			if (!unit_with_hangar)
			{
				unit_with_hangar = hangar_unit;
			}
			u32 range = helper::get_target_acquisition_range_upgrade(unit_with_hangar) + 2;
			if (!unit_with_hangar || helper::is_target_within_min_movement_range(steward_target, unit_with_hangar, range * 32))
			{
				result = 1;
			}
			else
			{
				helper::issue_order_targeting_nothing(hangar_unit, OrderId::Return);
				result = 0;
			}
		}
		else
		{
			u8 steward_order_id = hangar_unit->main_order_id;
			hangar_unit->user_action_flags |= 1u;
			if (steward_order_id)
			{
				while (1)
				{
					Order* order_tail = hangar_unit->order_queue_tail;
					if (!order_tail)
					{
						break;
					}
					if (!orders_dat::can_be_interrupted[order_tail->order_id] && order_tail->order_id != OrderId::Return)
					{
						break;
					}
					helper::remove_order_from_unit_queue(order_tail, hangar_unit);
				}
				helper::perform_another_order(OrderId::Return, 0, hangar_unit, UnitId::None, {}, 0);
			}
			helper::do_next_queued_order(hangar_unit);
			result = 0;
		}
		return result;
	}

	static Box32 get_thingy_box(Thingy* thingy) {
		Box32 box = {};
		box.left   = thingy->sprite->position.x - units_dat::unit_bounds[thingy->unit_id].left;
		box.right  = thingy->sprite->position.x + units_dat::unit_bounds[thingy->unit_id].right;
		box.top    = thingy->sprite->position.y - units_dat::unit_bounds[thingy->unit_id].top;
		box.bottom = thingy->sprite->position.y + units_dat::unit_bounds[thingy->unit_id].bottom;
		return box;
	}

	// Hooked to fix hit tests on structures in fog of war. -Pr0nogo
	Thingy* find_fow_target(u32 x_pos, u32 y_pos) { // 00487A10
		for (Thingy* thingy = *g_first_fow_sprite; thingy; thingy = thingy->next) {
			if (!helper::unit_is_unselectable(thingy->unit_id)) {
				Box32 box = get_thingy_box(thingy);
				if (box.left <= x_pos && x_pos <= box.right
				&&	box.top  <= y_pos && y_pos <= box.bottom) {
					return thingy;
				}
			}
		}
		return 0;
	}

	// Hooked to investigate crashes. -Pr0nogo
	bool is_better_target(Unit* self, Unit* auto_target, Unit* compare_unit) // 004769D0
	{
		bool result = 0;
		if (compare_unit != auto_target)
		{
			u32 compare_player = compare_unit->player == 11 ? compare_unit->sprite->player : compare_unit->player;
			if (!scbw::is_allied_to(self->player, compare_player))
			{
				if (!auto_target
				||  !helper::unit_can_attack_target(auto_target, self, 1)
				||  !helper::is_unit_enemy(self, auto_target)
				||   helper::is_unit_in_weapon_range(compare_unit, self) && !helper::is_unit_in_weapon_range(auto_target, self))
				{
					result = 1;
				}
			}
		}
		return result;
	}

	// Hooked to allow for aggressive unit responses. -Pr0nogo
	// Arg sound_offset was cucko'd by IDA, but we don't need the arg anyway since it gets written to before being used in both cases.
	// TODO: Figure out how to properly access g_client_selection_group in this context
	void do_targeted_order(void* unused, u32 sound_offset, u32 pos_x, u32 pos_y, Unit* target, u32 fow_unit_id) // 0046F5B0
	{
		if (!*g_is_targeting)
		{
			return;
		}
		s32 error = 0xFFFF;
		helper::cancel_target_order();
		u8 order_id = *g_order_id_obstructed;
		if (target)
		{
			order_id = *g_order_id_unit;
		}
		else if (fow_unit_id == UnitId::None)
		{
			order_id = *g_order_id_ground;
		}

		char use_weapon = orders_dat::use_weapon_targeting[order_id];
		u8 weapon_id = orders_dat::order_weapon_id[order_id];
		u8 tech_id = orders_dat::tech_used[order_id];
		u32 energy_cost = 0;
		if (tech_id != TechId::None)
		{
			energy_cost = techdata_dat::energy_cost[tech_id] << 8;
		}
		if (orders_dat::obscured_order[order_id] == OrderId::None)
		{
			fow_unit_id = UnitId::None;
		}

		char can_attack_final = 0;
		char not_enough_energy = 0;
		char can_target_spell = 0;
		Unit** selected_units = &g_client_selection_group->unit[11];
		Unit** selected_units_2 = &g_client_selection_group->unit[11];
		while (1) {
			Unit* unit = *selected_units; // Accessed incorrectly, unit = 0x100
			if (*selected_units)
			{
				if (!unit->is_frozen())
				{
					if (!use_weapon)
					{
						if (tech_id == TechId::None)
						{
							if (fow_unit_id == UnitId::None)
							{
								helper::is_order_valid(order_id, unit, target, target, &error);
							}
							else
							{
								helper::can_target_order_on_fow_unit(unit, order_id, fow_unit_id, &error);
							}
						}
						else
						{
							scbw::print("can target spell");
							if (helper::can_target_spell(tech_id, target, fow_unit_id, unit, pos_x, pos_y, &error))
							{
								scbw::print("can target spell pass");
								if (!unit->has_energy(energy_cost))
								{
									scbw::print("no energy");
									not_enough_energy = 1;
								}
								can_target_spell = 1;
							}
						}
						goto iterate_selection;
					}
					if (weapon_id == WeaponId::None)
					{
						if (!target)
						{
							if (helper::unit_can_attack_ground_type(fow_unit_id, unit))
							{
								can_attack_final = 1;
							}
							goto iterate_selection;
						}
						if (helper::get_right_click_action_order(unit) == 3)
						{
							if (is_unit_target_out_of_max_range(unit, target))
							{
								error = 888;
								goto iterate_selection;
							}
							if (helper::target_is_inside_minimum_range(unit, target))
							{
								error = 889;
								goto iterate_selection;
							}
						}
						can_attack_final = unit_can_attack_target(target, unit, 1);
						goto iterate_selection;
					}
					if (helper::can_weapon_target_unit(weapon_id, target, unit) || fow_unit_id < UnitId::None && ~(units_dat::base_property[fow_unit_id] >> 29) & 1)
					{
						if (!unit->has_energy(energy_cost))
						{
							not_enough_energy = 1;
						}
						can_attack_final = 1;
					}
				}
			}
		iterate_selection:
			selected_units = selected_units_2 + 1;
			selected_units_2++;
			if (selected_units_2 >= &g_client_selection_group_end) {
				break;
			}
		}

		u32 error_type_2 = error;
		Unit* active_unit = *g_active_portrait_unit;
		u8 race = active_unit->get_race();
		if (use_weapon)
		{
			if (!can_attack_final || error != -1)
			{
				helper::get_targeting_error_string(weapon_id, &error, target);
				helper::display_error_message_id_with_sfx(active_unit->player, error, race + 138);
				if (weapon_id == WeaponId::None)
				{
					helper::cmdact_target_order(order_id, target, pos_x, pos_y, fow_unit_id, g_is_keycode_used[16]);
				}
				return;
			}
		check_energy:
			if (energy_cost && !not_enough_energy)
			{
				helper::display_error_message_id_with_sfx(active_unit->player, race + 864, race + 156);
				return;
			}
		}
		else
		{
			if (energy_cost)
			{
				if (!can_target_spell)
				{
					if (error == -1)
					{
						error = 876;
					}
					u16 error_type_1 = error - 1;
					const char* error_string_2 = 0;
					if (error)
					{
						if (error_type_1 < *g_stat_txt_tbl)
						{
							error_string_2 = (char*)g_stat_txt_tbl + g_stat_txt_tbl[error_type_1 + 1];
						}
						else
						{
							error_string_2 = "";
						}
					}
					helper::get_stat_txt_string(active_unit, *g_active_nation_id, (char*)error_string_2);
					return;
				}
				error_type_2 = error;
				goto check_energy;
			}
			error_type_2 = error;
		}
		if (error_type_2 == -1)
		{
			UnitExt* unit_ext = get_unit_ext(active_unit);
			unit_ext->values[ValueUnitResponseOrder] = order_id;
			hooks::play_yes_unit_response_internal(active_unit, UnitResponseContext::URC_OrderId);
			helper::cmdact_target_order(order_id, target, pos_x, pos_y, fow_unit_id, g_is_keycode_used[16]);
		}
		else
		{
			u8 active_player = active_unit->player;
			u16 error_string_id = error_type_2 - 1;
			if (error_type_2)
			{
				if (error_string_id < *g_stat_txt_tbl)
				{
					helper::get_stat_txt_string(active_unit, active_player, (char*)(g_stat_txt_tbl + g_stat_txt_tbl[error_string_id + 1]));
				}
				else
				{
					helper::get_stat_txt_string(active_unit, active_player, "");
				}
			}
			else
			{
				helper::get_stat_txt_string(active_unit, active_player, 0);
			}
		}
	}

	// Hooked to add support for new refineries. -Pr0nogo
	b32 fow_refinery_check(Unit* self, u16 fow_id) // 00474500
	{
		char race = self->get_race(); // TODO: Add support for custom races
		switch (race)
		{
			case RaceId::Zerg:
			{
				return fow_id == UnitId::ZergExcisant;
			}
			case RaceId::Protoss:
			{
				return fow_id == UnitId::ProtossAquifer;
			}
			case RaceId::Terran:
			{
				return fow_id == UnitId::TerranReservoir;
			}
			default:
			{
				return 0;
			}
		}
	}

	// Hooked to fix infinite attack range on hold position. -Pr0nogo
	void pick_new_target_if_needed(Unit* unit) { // 00477510, unused Unit* arg1
		if (unit->status & UnitStatus::AttackMove) {
			Unit* best_target = 0;
			s32 best_score = INT_MAX;

			Unit* order_target = unit->order_target.unit;
			if (order_target) {
				if (unit->unit_ai && !helper::is_unit_enemy(unit, order_target)) {
					return;
				}
				if (unit_can_attack_target(order_target, unit, true)) {
					if (!helper::is_ai_target_related(unit, order_target)) {
						best_target = order_target;
						best_score = helper::get_target_priority(unit, order_target);
						if (!best_score) {
							if (helper::is_unit_in_weapon_range(order_target, unit)) {
								return;
							}
							best_score = 1;
						}
					}
				}
			}

			Unit* auto_target = unit->auto_target_unit;
			if (auto_target) {
				if (helper::unit_can_attack_target(auto_target, unit, 1)
				&&  helper::is_unit_enemy(unit, unit->auto_target_unit)
				&& !helper::is_ai_target_related(unit, auto_target)) {
					s32 auto_target_score = helper::get_target_priority(unit, unit->auto_target_unit);
					if (!auto_target_score && !helper::is_unit_in_weapon_range(unit->auto_target_unit, unit)) {
						auto_target_score = 1;
					}
					if (auto_target_score < best_score) {
						best_target = unit->auto_target_unit;
						best_score = auto_target_score;
					}
				} else {
					unit->auto_target_unit = 0;
				}
			}

			if (!best_target || best_score) {
				if (unit->unit_ai && helper::ai_update_attack_target(unit, 0, 1, 0)) {
					return;
				}
				Unit* best_attack_target = helper::find_best_attack_target(unit);
				if (best_attack_target) {
					if (helper::get_target_priority(unit, best_attack_target) < best_score) {
						best_target = best_attack_target;
					}
				}
			}

			if (!best_target || helper::unit_can_attack_target(best_target, unit, true)) {
				unit->order_target.unit = best_target;
				Unit* subunit = unit->subunit;
				if (subunit) {
					if (subunit->main_order_id == units_dat::attack_unit_order[subunit->id] || subunit->main_order_id == OrderId::HoldPosition2) {
						subunit->order_target.unit = best_target;
					}
				}
			}
		}
	}

	// Hooked to make units like the Paladin not turn to face targets after starting their attacks. -Pr0nogo
	void change_direction_to_move_waypoint(Unit* unit) // 00495840
	{
		if (unit->next_movement_waypoint.x == unit->position.x && unit->next_movement_waypoint.y == unit->position.y)
		{
			if (unit->next_target_waypoint.x == unit->position.x && unit->next_target_waypoint.y == unit->position.y)
			{
				unit->velocity_direction_2 = unit->current_direction_1;
				helper::set_unit_movement_speed_ex(unit);
			} else if (unit->id != UnitId::TerranPaladin || !(unit->movement_flags & MovementFlags::StartingAttack))
			{
				unit->velocity_direction_2 = helper::get_direction_from_points(unit->next_target_waypoint.y, unit->position.x, unit->next_target_waypoint.x, unit->position.y);
				helper::set_unit_movement_speed_ex(unit);
			}
		} else {
			unit->velocity_direction_2 = helper::get_direction_from_points(unit->next_movement_waypoint.y, unit->position.x, unit->next_movement_waypoint.x, unit->position.y);
			helper::set_unit_movement_speed_ex(unit);
		}
	}
}