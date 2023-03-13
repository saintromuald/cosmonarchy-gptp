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
		speed /= 1 + 0.25 * unit_ext->unit_vectors[UnitVectorCrucibles].size();

		// Vassal Terminal Surge: increase movespeed when any nearby Vassal dies
		if (unit_ext->timers[TimerTerminalSurge] > 1)
		{
			speed *= 1.25;
		}

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
			speed *= 1 - (0.10 * unit_ext->values[ValuePetrificationStacks]);
		}

		// Claymore - Kelvin Munitions - slow affected targets
		if (unit_ext->timers[TimerKelvinMunitions] >= 1)
		{
			speed *= 0.666;
		}

		// Hierophant Signify: Cowardice
		if (unit_ext->values[ValueSignifyStacks])
		{
			auto match = cosmonarchy_finder_match(unit, 0, {});
			Vector32f dir = scbw::get_unit_direction_vector(unit);

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
			case UnitId::TerranBlackjack:
			{
				// Blackjack Wear and Tear: reduce movement speed based on missing health
				const u32 health = unit->hitpoints / 256;
				const u32 maxHealth = unit->get_max_hp_ingame();
				const f64 multiplier = MathUtils::map(health, (u32)0, maxHealth, 0.66, 1.0, true); // nice casting tbh
				speed *= multiplier;
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Harmony Drive: gains increased movement speed based on shields
				f64 fraction = (unit->shields / 256.0) / units_dat::max_shield_points[unit->id];
				speed *= 1.0 + fraction / 2;
			} break;
			case UnitId::ProtossGolem:
			{
				// Golem Deathless Procession: gains increased movespeed based on missing shields
				const u32 health = unit->shields / 256;
				const u32 maxHealth = units_dat::max_shield_points[unit->id];
				const f64 multiplier = MathUtils::map(health, maxHealth, maxHealth - maxHealth, 1.0, 1.5, true);
				speed *= multiplier;
			} break;
			case UnitId::ZergVorvrokor:
			{
				// Vorvaling Adrenal Surge: gains f64 movespeed when moving towards enemies
				if (unit->id == UnitId::ZergVorvrokor)
				{
					u32 range = unit->get_sight_range(false) * 32;
					scbw::UnitFinder unit_finder(
						unit->get_x() - range,
						unit->get_y() - range,
						unit->get_x() + range,
						unit->get_y() + range
					);
					unit_finder.for_each([&](Unit* target)
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
						speed * 1.2;
					}
				}
			} break;
			case UnitId::ZergMortothrokor:
			{
				if (unit_ext->values[ValueMortothrokorStacks])
				{
					speed *= 1 + (0.05 * unit_ext->values[ValueMortothrokorStacks]);
				}
			} break;
			case UnitId::ZergUltrokor:
			{
				// Ultrakor Kaiser Rampage: gains 200% movespeed on kill for 1 second
				speed *= 1.2;
				if (unit_ext->timers[TimerKaiserRampage])
				{
					speed *= 2;
				}
			} break;
			// Hardcode ideal movement speeds
			case UnitId::TerranClaymore:
			{
				speed *= 0.9;
			} break;
			case UnitId::TerranHarakan:
			case UnitId::TerranSavant:
			case UnitId::TerranApostle:
			case UnitId::TerranCataphract:
			case UnitId::ProtossIdol:
			case UnitId::ZergKonvilisk:
			{
				speed *= 1.1;
			} break;
			case UnitId::TerranCyclops:
			case UnitId::TerranHeracles:
			{
				speed *= 1.15;
			} break;
			case UnitId::TerranCyprian:
			case UnitId::TerranEidolon:
			case UnitId::ProtossCabalist:
			case UnitId::ZergIzirokor:
			case UnitId::ZergKagralisk:
			case UnitId::ZergZoryusthaleth:
			{
				speed *= 1.2;
			} break;
			case UnitId::TerranStriga:
			case UnitId::ProtossZealot:
			case UnitId::ProtossSycophant:
			case UnitId::ProtossManifold:
			case UnitId::ZergZethrokor:
			{
				speed *= 1.25;
			} break;
			case UnitId::ProtossAmaranth:
			{
				speed *= 1.4;
			} break;
			case UnitId::ProtossLegionnaire:
			case UnitId::ProtossVagrantShade:
			case UnitId::ZergHydralisk:
			{
				speed *= 1.5;
			} break;
		}

		if (speed < 0) {
			speed = 0;
		}
		return speed;
	}

	u32 get_modified_unit_speed(Unit* unit, u32 baseSpeed) // 0047B5F0
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

	// Hooked to fix the speed tooltip for iscript units. -Pr0nogo
	void update_iscript_speed(Unit* unit) // 0042D8C0
	{
		if (unit->flingy_movement_type == 2) // Uses iscript for movement
		{
			Image* image = unit->sprite->main_graphic;
			*g_temp_iscript_header = image->iscript_header_offset;
			*g_iscript_speed_something = image->return_address;
			if (helper::set_temp_iscript_animation(IscriptAnimation::Walking))
			{
				Unit* active_unit = *g_active_iscript_unit;
				u32 v10 = 0;
				u32 opcode_value = 0;
				u32 tile_size = 32;
				bool tile_size_bool = 0;
				u32 base_speed = 0;
				do
				{
					Image* loop_image = unit->sprite->main_graphic;
					*g_active_iscript_unit = unit;
				//	helper::iscript_opcode_cases(loop_image, g_temp_iscript_header, 1, (u32)&opcode_value); // This is probably wrong, compare to IDA (our helper has 1 less arg)
					char stim_timer = unit->stim_timer;
					u32 opcode_value_copy = opcode_value;
					u32 buff_or_debuff = 0;
					*g_active_iscript_unit = active_unit;
					if (stim_timer)
					{
						buff_or_debuff = 1;
					}
					if (unit->status & UnitStatus::SpeedUpgrade)
					{
						++buff_or_debuff;
					}
					if (unit->ensnare_timer)
					{
						--buff_or_debuff;
					}
					if (buff_or_debuff <= 0)
					{
						if (buff_or_debuff < 0)
						{
							opcode_value_copy = opcode_value - (opcode_value >> 2);
						}
					}
					else
					{
						opcode_value_copy = 2 * opcode_value;
					}
					base_speed = opcode_value_copy + v10;
					tile_size_bool = tile_size == 1;
					v10 += opcode_value_copy;
					opcode_value = 0;
					--tile_size;
				} while (!tile_size_bool);
				unit->flingy_top_speed = base_speed / 32;
			}
		}
	}

	// Hooked to investigate melee air unit perforamnce. -Pr0nogo
	s32 get_flingy_halt_distance(Unit* unit) // 00494F90
	{
		s32 result = 0;
		s32 speed = unit->current_speed_2;
		if (!speed || unit->flingy_movement_type)
		{
			return result;
		}
		u32 flingy_id = unit->flingy_id;
		if (speed != flingy_dat::top_speed[flingy_id] || unit->flingy_acceleration != flingy_dat::acceleration[flingy_id])
		{
			result = speed * speed / (2 * unit->flingy_acceleration);
		}
		else
		{
			result = flingy_dat::halt_distance[flingy_id];
		}
		return result;
	}
}