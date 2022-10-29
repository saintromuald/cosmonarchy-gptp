//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/scbwdata.h"
#include "hooks/cosmonarchy_finder.h"
#include <SCBW/UnitFinder.h>
#include "SCBW/api.h"
#include "globals.h"
#include "math_utils.h"
#include "Utils/captivating_claws.h"

namespace hooks
{
	f64 get_speed_multiplier(Unit* unit)
	{
		UnitExt* unit_ext = get_unit_ext(unit);

		f64 speed = 1;
		// Set additives based on status effects and movespeed upgrades
		s32 increase = (unit->stim_timer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		s32 decrease = (unit->ensnare_timer ? 1 : 0);

		if (decrease > 0)
		{
			for (s32 i = 0; i < decrease; i++)
			{
				speed *= 0.5;
			}
		}

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			speed *= pow(0.95, unit_ext->values[ValueSovrolethLinkCount]);
		}

		// Crucible Aura
		speed /= 1 + 0.15 * unit_ext->unit_vectors[UnitVectorCrucibles].size();

		// Vassal Terminal Surge: increase movespeed when any nearby Vassal dies
		speed *= 1.0 + unit_ext->values[ValueTerminalSurge] * 0.05;

		// Vorvrokor Captivating Claws: reduces movement speed by 25% per unique attacker
		if (captivating_claws_get_timer(unit) > 0)
		{
			speed *= 1 - 0.25 * captivating_claws_get_stacks(unit);
		}

		// Keskathalor Caustic Discharge: Speed Buff
		if (unit_ext->timers[TimerCausticDischargeSpeed] > 0)
		{
			speed *= 1.5;
		}

		// Swarming Omen ability: 125% movement speed when moving towards Zoryusthaleth
		/*
		if (unit_ext->values[ValueSwarmingOmenPlayer]) {
			u32 player = unit_ext->values[ValueSwarmingOmenPlayer] - 1;
			EntryArray targets = {};
			unit_search_radial(&targets, unit->position, 10 * 32);
			for (u32 i = 0; i < targets.count; ++i) {
				Unit* target = targets.units[i];
				if (!scbw::is_allied_to(player, target->player)) {
					continue;
				}
				UnitExt* target_ext = get_unit_ext(target);
				target_ext->timers[TimerSwarmingOmenSearch] = 24;
			}
		}
		*/
		if (unit_ext->timers[TimerSwarmingOmenSearch]) {
			bool accelerate = false;
			EntryArray targets = {};
			unit_search_radial(&targets, unit->position, 10 * 32);
			unit_search_filter(&targets, unit, FilterFrontalArc);
			for (u32 i = 0; i < targets.count; ++i) {
				Unit* target = targets.units[i];
				UnitExt* target_ext = get_unit_ext(target);
				if (!target_ext->values[ValueSwarmingOmenPlayer]) {
					continue;
				}
				u32 player = target_ext->values[ValueSwarmingOmenPlayer] - 1;
				if (!scbw::is_allied_to(player, unit->player)) {
					continue;
				}
				accelerate = true;
			}
			if (accelerate) {
				speed *= 1.25;
			}
		}

		// Gorgon - Petrification behavior
		if (unit_ext->timers[TimerPetrification])
		{
			speed *= 1 - (0.05 * unit_ext->values[ValuePetrificationStacks]);
		}

		// Claymore - Kelvin Munitions - slow affected targets
		if (unit_ext->values[ValueKelvinMunitionsSlow])
		{
			speed *= 0.666;
		}

		// Hierophant Signify: Cowardice
		if (unit_ext->values[ValueSignifyStacks])
		{
			auto match = cosmonarchy_finder_match(unit, 0, {});
			Vector32f dir = scbw::getUnitDirectionVector(unit);

			const u32 radius = 6 * 32;
			Unit* enemy_unit = scbw::UnitFinder::get_nearest_target(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius,
				unit,
				[&](Unit* enemy)
			{
				if (!match(enemy))
				{
					return false;
				}
				bool valid_player = false;
				for (auto player : unit_ext->value_vectors[ValueVectorSignifySourcePlayer])
				{
					if (scbw::is_allied_to(player, enemy->player))
					{
						valid_player = true;
						break;
					}
				}
				if(!valid_player)
				{
					return false;
				}

				Vector32 diff = scbw::get_vector_between_points(enemy->position, unit->position);
				Vector32f target = scbw::get_normalized_vector(diff);
				f32 dot = scbw::dot_product(dir, target);

				if (dot < 0)
				{
					return false;
				}
				return true;
			});

			if (enemy_unit)
			{
				speed *= 0.5;
			}
		}

		switch (unit->id)
		{
			case UnitId::ZergUltrokor:
			{
				// Ultrakor Kaiser Rampage: gains 150% movespeed on kill for 1 second
				if (unit_ext->timers[TimerKaiserRampage])
				{
					speed *= 2;
				}
			} break;
			case UnitId::TerranWyvern:
			{
				// Wyvern Reverse Thrust ability: reduce movespeed
				if (scbw::get_generic_rust_value(unit, RustId::ToggleReverseThrust) == 1)
				{
					speed *= 0.5;
				}
			} break;
			case UnitId::ZergVorvrokor:
			{
				// Vorvaling Adrenal Surge: gains f64 movespeed when moving towards enemies
				if (unit->id == UnitId::ZergVorvrokor)
				{
					u32 range = unit->get_sight_range(false) * 32;
					scbw::UnitFinder unitFinder(
						unit->get_x() - range,
						unit->get_y() - range,
						unit->get_x() + range,
						unit->get_y() + range
					);
					unitFinder.for_each([&](Unit* target)
					{
						if (scbw::is_allied_to(unit->player, target->player)
							|| units_dat::base_property[target->id] & UnitProperty::Flyer)
						{
							return;
						}
						u16 angle = scbw::get_angle(unit->position.x, unit->position.y,
							target->position.x, target->position.y);
						u16 unit_angle = unit->velocity_direction_1;
						s32 arc = (f64)60 / 1.40625;
						// 120 degrees
						s32 delta = unit_angle - angle;
						delta = abs((delta + 128) % 256 - 128);
						if (delta <= arc)
						{
							unit->new_flags_3 |= 0x2;
						}
					});
					if (unit->new_flags_3 & 0x2)
					{
						speed *= 2;
						unit->new_flags_3 &= ~0x2;
					}
					else
					{
						speed * 1.25;
					}
				}
			} break;
			case UnitId::TerranVulture:
			{
				// Vulture Ion Propulsion: reduce movespeed, gains hover-mover status
				auto tile = scbw::get_stupid_reference_to_active_tile_at(unit->get_x(), unit->get_y());
				if (tile.is_unwalkable || !tile.is_walkable)
				{
					if (scbw::get_upgrade_level(unit->player, UpgradeId::IonPropulsion))
					{
						speed *= 0.5;
					}
				}
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Harmony Drive: gains increased movement speed based on shields
				f64 fraction = (unit->shields / 256.0) / units_dat::max_shield_points[unit->id];
				speed *= 1.0 + fraction / 2;
			} break;
			case UnitId::TerranBlackjack:
			{
				// Blackjack Wear and Tear: reduce movement speed based on missing health
				const u32 health = unit->hitpoints / 256;
				const u32 maxHealth = unit->get_max_hp_ingame();
				const f64 multiplier = MathUtils::map(health, (u32)0, maxHealth, 0.66, 1.0, true); // nice casting tbh
				speed *= multiplier;
			} break;
			case UnitId::ProtossGolem:
			{
				// Golem Deathless Procession: gains increased movespeed based on missing shields
				const u32 health = unit->shields / 256;
				const u32 maxHealth = units_dat::max_shield_points[unit->id];
				const f64 multiplier = MathUtils::map(health, maxHealth, maxHealth - maxHealth, 1.0, 1.5, true);
				speed *= multiplier;
			} break;
			// Hardcode ideal movement speeds
			case UnitId::TerranClaymore:
			{
				speed *= 0.9;
			} break;
			case UnitId::TerranHarakan:
			case UnitId::ProtossIdol:
			case UnitId::ZergKonvilisk:
			{
				speed *= 1.1;
			} break;
			case UnitId::TerranHeracles:
			{
				speed *= 1.15;
			} break;
			case UnitId::TerranCyprian:
			case UnitId::TerranEidolon:
			case UnitId::ProtossAmaranth:
			case UnitId::ProtossCabalist:
			case UnitId::ZergKagralisk:
			case UnitId::ZergZoryusthaleth:
			{
				speed *= 1.2;
			} break;
			case UnitId::ProtossZealot:
			case UnitId::ProtossManifold:
			case UnitId::ZergZethrokor:
			{
				speed *= 1.25;
			} break;
			case UnitId::ProtossLegionnaire:
			case UnitId::ZergHydralisk:
			case UnitId::ProtossVagrantShade:
			{
				speed *= 1.5;
			} break;
		}

		if (speed < 0) {
			speed = 0;
		}
		return speed;
	}

	/// Logically equivalent to GetModifiedUnitSpeed @ 0047B5F0  
	u32 get_modified_unit_speed(Unit* unit, u32 baseSpeed)
	{
		if (unit == nullptr)
		{
			return 1;
		}

		s32 increase = (unit->stim_timer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		switch (unit->id)
		{
			case UnitId::ProtossPanoptus:
			{
				// Leftover movement speed code
				baseSpeed = 1707;
			} break;
			default:
			{
				// Flingy-related code
				if (increase > 0)
				{
					for (s32 i = 0; i < increase; i++)
					{
						baseSpeed *= 1.5;
					}
					if (baseSpeed < 853)
					{
						baseSpeed = 853;
					}
				}
			}
		}
		return baseSpeed * get_speed_multiplier(unit);
	}

	u32 get_modified_unit_acceleration(Unit* unit)
	{
		return std::max(flingy_dat::acceleration[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0);
	}

	u32 get_modified_unit_turn_speed(Unit* unit)
	{
		return std::max(flingy_dat::turn_speed[units_dat::graphic[unit->id]] * get_speed_multiplier(unit), 1.0);
	}

	// Hooked to investigate crashes. -Pr0nogo
	Unit* set_unit_movement_speed(Unit* unit, u32 speed) // 004951C0
	{
		/*
		   Crash occurs on the very first ASM instruction because "unit" is null. Appears exceedingly rarely.
		   Call stack from fagguto:
				0049794A @ sprite_to_iscript_loop (hooked in update_unit_state)
				0048C405 @ progress_bullet_frames (hooked but disabled in weapon_fire)
				012C0B93 @ some plugin or windows thing?
				004D9908 @ game_loop_top (not hooked)
				004D9A6F @ begin_game (not hooked)
				004E07FA @ preload_game (not hooked)
				004E0B20 @ win_main (cannot be hooked?)
				00404DA5 @ start (not hooked)
				761C344D 0018FF94 0001:0000344D C:\Windows\syswow64\kernel32.dll
				77999802 0018FFD4 0001:00029802 C:\Windows\SysWOW64\ntdll.dll
				779997D5 0018FFEC 0001:000297D5 C:\Windows\SysWOW64\ntdll.dll
		*/
		bool speed_is_equal = unit->current_speed_1 == speed;
		unit->current_speed_2 = speed;
		if (!speed_is_equal)
		{
			u32 direction = unit->current_direction_2;
			unit->current_speed_1 = speed;
			unit->current_velocity.x = speed * g_angle_distance[direction].x >> 8;
			unit->current_velocity.y = speed * g_angle_distance[direction].y >> 8;
		}
		return unit;
	}
}