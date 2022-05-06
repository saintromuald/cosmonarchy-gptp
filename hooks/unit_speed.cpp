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
	f64 getSpeedMultiplier(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		f64 speed = 1;
		// Set additives based on status effects and movespeed upgrades
		s32 increase = (unit->stimTimer ? 1 : 0) + (unit->status & UnitStatus::SpeedUpgrade ? 1 : 0);
		s32 decrease = (unit->ensnareTimer ? 1 : 0) + (unit->unusedTimer ? 1 : 0);

		if (decrease > 0)
		{
			for (s32 i = 0; i < decrease; i++)
			{
				speed *= 0.5;
			}
		}

		// Crucible Aura
		speed /= 1 + 0.05 * unitExt->unitVectors[UnitVectorCrucibles].size();

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
		if (captivatingClawsGetTimer(unit) > 0)
		{
			speed *= 1 - 0.25 * captivatingClawsGetStacks(unit);
		}

		// Keskathalor Caustic Discharge: Speed Buff
		if (unitExt->timers[TimerCausticDischargeSpeed] > 0)
		{
			speed *= 1.5;
		}

		// Swarming Omen ability: 125% movement speed when moving towards Zoryusthaleth
		{
			auto r = 10 * 32;
			auto n = cosmonarchyFinderMatch(unit, Flags::Ally | Flags::FrontalArc, {UnitId::ZergZoryusthaleth});
			scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
				unit->position.x + r, unit->position.y + r);
			bool accelerate = false;
			for (s32 i = 0; i < unitsInFinder.getUnitCount(); i++)
			{
				auto picked_unit = unitsInFinder.getUnit(i);
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
			auto match = cosmonarchyFinderMatch(unit, 0, {});
			Vector32f dir = scbw::getUnitDirectionVector(unit);

			const u32 radius = 6 * 32;
			Unit* enemyUnit = scbw::UnitFinder::getNearestTarget(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius,
				unit,
				[&](Unit* enemy)
			{
				if (!match(enemy))
				{
					return false;
				}
				bool validPlayer = false;
				for (auto player : unitExt->valueVectors[ValueVectorSignifySourcePlayer])
				{
					if (scbw::isAlliedTo(player, enemy->playerId))
					{
						validPlayer = true;
						break;
					}
				}
				if(!validPlayer)
				{
					return false;
				}

				Vector32 diff = scbw::getVectorBetweenPoints(enemy->position, unit->position);
				Vector32f target = scbw::getNormalizedVector(diff);
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
					u32 range = unit->getSightRange(false) * 32;
					scbw::UnitFinder unitFinder(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range
					);
					unitFinder.forEach([&](Unit* target)
					{
						if (scbw::isAlliedTo(unit->playerId, target->playerId)
							|| units_dat::BaseProperty[target->id] & UnitProperty::Flyer)
						{
							return;
						}
						u16 angle = scbw::getAngle(unit->position.x, unit->position.y,
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
				auto tile = scbw::getActiveTileAt(unit->getX(), unit->getY());
				if (tile.isUnwalkable || !tile.isWalkable)
				{
					if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::IonPropulsion))
					{
						speed *= 0.5;
					}
				}
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Harmony Drive: gains increased movement speed based on shields
				f64 fraction = (f64)unit->shields / ((f64)units_dat::MaxShieldPoints[unit->id] * 256);
				speed *= 1.0 + fraction;
			} break;
			case UnitId::TerranBlackjack:
			{
				// Blackjack Wear and Tear: reduce movement speed based on missing health
				const u32 health = unit->hitPoints / 256;
				const u32 maxHealth = unit->getMaxHpInGame();
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
	u32 getModifiedUnitSpeedHook(Unit* unit, u32 baseSpeed)
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

		return baseSpeed * getSpeedMultiplier(unit);
	}

	/// Logically equivalent to getModifiedUnitAcceleration @ 0047B8A0 
	u32 getModifiedUnitAccelerationHook(Unit* unit)
	{
		return std::max(scbw::getFlingyData(FlingyDatEntries::Acceleration, scbw::getUnitData(UnitsDatEntries::Flingy, unit->id)) * getSpeedMultiplier(unit), 1.0);
	}

	/// Logically equivalent to getModifiedUnitTurnRadius @ 0047B850 
	u32 getModifiedUnitTurnSpeedHook(Unit* unit)
	{
		return std::max(scbw::getFlingyData(FlingyDatEntries::TurnRadius, scbw::getUnitData(UnitsDatEntries::Flingy, unit->id)) * getSpeedMultiplier(unit), 1.0);
	}

} //hooks
