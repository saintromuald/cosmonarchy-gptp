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
		UnitExt* unitExt = globals.get_unit_ext(unit);

		f64 speed = 1;
		// Set additives based on status effects and movespeed upgrades
		s32 increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		s32 decrease = (unit->ensnareTimer ? 1 : 0);

		if (decrease > 0)
		{
			for (s32 i = 0; i < decrease; i++)
			{
				speed *= 0.5;
			}
		}

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			UnitArray sovroleths = scbw::get_writhing_web_sovroleths(unit);
			speed *= pow(0.95, sovroleths.size);
		}

		// Crucible Aura
		speed /= 1 + 0.1 * unitExt->unit_vectors[UnitVectorCrucibles].size();

		// Mind Tyrant Malediction: Apply movespeed slow to non-organic targets
		if (unitExt->timers[TimerMaledictionSlow] > 1)
		{
			speed *= 0.5;
		}

		// Lobotomy Mine slow
		if (unitExt->timers[TimerLobotomySlow] > 1)
		{
			speed *= 0.5;
		}

		// Vassal Terminal Surge: increase movespeed when any nearby Vassal dies
		speed *= 1.0 + unitExt->values[ValueTerminalSurge] * 0.05;

		// Nathrokor Adrenal Frenzy: increase movespeed of all nearby allies after kill
		if (unitExt->timers[TimerNathrokorBuff])
		{
			speed *= 1 + NathrokorAdrenalFrenzySpeedFactor * unitExt->values[ValueNathrokorBuffStacks];
		}

		// Luminary - Wax Wings
		if (unitExt->timers[TimerWaxWingsDebuff] > 1)
		{
			speed *= 0.6666;
		}

		// Vorvaling Captivating Claws: reduces movement speed by 25% per unique attacker
		if (captivating_claws_get_timer(unit) > 0)
		{
			speed *= 1 - 0.25 * captivating_claws_get_stacks(unit);
		}

		// Keskathalor Caustic Discharge: Speed Buff
		if (unitExt->timers[TimerCausticDischargeSpeed] > 0)
		{
			speed *= 1.5;
		}

		// Swarming Omen ability: 125% movement speed when moving towards Zoryusthaleth
		{
			auto r = 10 * 32;
			auto n = cosmonarchy_finder_match(unit, Flags::Ally | Flags::FrontalArc, {UnitId::ZergZoryusthaleth});
			scbw::UnitFinder unitsInFinder(unit->get_x() - r, unit->get_y() - r,
				unit->position.x + r, unit->position.y + r);
			bool accelerate = false;
			for (s32 i = 0; i < unitsInFinder.get_unit_count(); i++)
			{
				auto picked_unit = unitsInFinder.get_unit(i);
				if (n(picked_unit))
				{
					UnitExt* pickedExt = globals.get_unit_ext(picked_unit);
					if (pickedExt->timers[TimerSwarmingOmen])
					{
						accelerate = true;
						unitExt->values[ValueSwarmingOmenStatus]= 25;
						break;
					}
				}
			}
			if (accelerate)
			{
				speed *= 1.25;
			}
		}

		// Gorgon - Petrification behavior
		if (unitExt->timers[TimerPetrification])
		{
			speed *= 1 - (0.05 * unitExt->values[ValuePetrificationStacks]);
		}

		// Claymore - Kelvin Munitions - slow affected targets
		if (unitExt->values[ValueKelvinMunitionsSlow])
		{
			speed *= 0.666;
		}

		// Hierophant Signify: Cowardice
		if (unitExt->values[ValueSignifyStacks])
		{
			auto match = cosmonarchy_finder_match(unit, 0, {});
			Vector32f dir = scbw::getUnitDirectionVector(unit);

			const u32 radius = 6 * 32;
			Unit* enemyUnit = scbw::UnitFinder::getNearestTarget(
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
				bool validPlayer = false;
				for (auto player : unitExt->value_vectors[ValueVectorSignifySourcePlayer])
				{
					if (scbw::is_allied_to(player, enemy->player))
					{
						validPlayer = true;
						break;
					}
				}
				if(!validPlayer)
				{
					return false;
				}

				Vector32 diff = scbw::get_vector_between_points(enemy->position, unit->position);
				Vector32f target = scbw::get_normalized_vector(diff);
				f32 dot = scbw::getDotProduct(dir, target);

				if (dot < 0)
				{
					return false;
				}
				return true;
			});

			if (enemyUnit)
			{
				speed *= 0.5;
			}
		}

		switch (unit->id)
		{
			case UnitId::ZergUltrokor:
			{
				// Ultrakor Kaiser Rampage: gains 150% movespeed on kill for 1 second
				if (unitExt->timers[TimerKaiserRampage])
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
							|| units_dat::BaseProperty[target->id] & UnitProperty::Flyer)
						{
							return;
						}
						u16 angle = scbw::get_angle(unit->position.x, unit->position.y,
							target->position.x, target->position.y);
						u16 unit_angle = unit->velocityDirection1;
						s32 arc = (f64)60 / 1.40625;
						// 120 degrees
						s32 delta = unit_angle - angle;
						delta = abs((delta + 128) % 256 - 128);
						if (delta <= arc)
						{
							unit->_padding_0x132 |= 0x2;
						}
					});
					if (unit->_padding_0x132 & 0x2)
					{
						speed *= 2;
						unit->_padding_0x132 &= ~0x2;
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
				f64 fraction = (unit->shields / 256.0) / units_dat::MaxShieldPoints[unit->id];
				speed *= 1.0 + fraction / 2;
			} break;
			case UnitId::TerranBlackjack:
			{
				// Blackjack Wear and Tear: reduce movement speed based on missing health
				const u32 health = unit->hitPoints / 256;
				const u32 maxHealth = unit->get_max_hp_ingame();
				const f64 multiplier = MathUtils::map(health, (u32)0, maxHealth, 0.66, 1.0, true); // nice casting tbh
				speed *= multiplier;
			} break;
			case UnitId::ProtossGolem:
			{
				// Golem Deathless Procession: gains increased movespeed based on missing shields
				const u32 health = unit->shields / 256;
				const u32 maxHealth = units_dat::MaxShieldPoints[unit->id];
				const f64 multiplier = MathUtils::map(health, maxHealth, maxHealth - maxHealth, 1.0, 1.5, true);
				speed *= multiplier;
			} break;
			// Hardcode ideal movement speeds
			case UnitId::TerranClaymore:
			{
				speed *= 0.9;
			} break;
			case UnitId::SharedTarasque:
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

		s32 increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
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
		return std::max(scbw::get_flingy_data(FlingyDatEntries::Acceleration, scbw::get_unit_data(UnitsDatEntries::Flingy, unit->id)) * get_speed_multiplier(unit), 1.0);
	}

	u32 get_modified_unit_turn_speed(Unit* unit)
	{
		return std::max(scbw::get_flingy_data(FlingyDatEntries::TurnRadius, scbw::get_unit_data(UnitsDatEntries::Flingy, unit->id)) * get_speed_multiplier(unit), 1.0);
	}
}