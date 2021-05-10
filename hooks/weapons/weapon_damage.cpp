#include "weapon_damage.h"
#include "../../SCBW/api.h"
#include <algorithm>
#include <cmath>
#include "MathUtils.h"
#include "Utils/captivating_claws.h"
#include "hooks/hydraFinder.h"
#include "globals.h"
#include "logger.h"

namespace
{
	/// Definition of damage factors
	struct
	{
		s32 damageType;
		s32 unitSizeFactor[5];	 // Order: {independent, small, medium, large, titanic}
	} const damageFactor[] = {
		{0, 0, 0,	0,	 0,   0},			// Independent
		{1, 0, 128, 192, 256, 192},			// Explosive
		{2, 0, 256, 192, 128, 64},			// Concussive
		{3, 0, 256, 256, 192, 128},			// Normal
		{4, 0, 256, 256, 256, 256},			// IgnoreArmor
		{5, 0, 256, 256, 256, 256},			// Chronal
	};
}

#include "SCBW\UnitFinder.h"

bool irradiateTarget(Unit* unit)
{
	return true; // Originally prohibited targeting eggs
}

void augurSacrifice(Unit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	if (targetExt->values[ValueAugurSafetyCheck] == 0)
	{
		targetExt->values[ValueAugurSafetyCheck] = 1;
		const s32 radius = 6 * 32;
		Unit* nearest = scbw::UnitFinder::getNearestTarget(
			target->getX() - radius,
			target->getY() - radius,
			target->getX() + radius,
			target->getY() + radius,
			target, [&](Unit* unit)
		{
			// Don't attack friendly / allied units
			if (unit->sprite == nullptr)
			{
				return false;
			}
			if (unit->playerId == target->playerId)
			{
				return false;
			}
			if (scbw::isAlliedTo(unit->playerId, target->playerId))
			{
				return false;
			}
			if (unit->status & UnitStatus::Invincible)
			{
				return false;
			}
			return true;
		});
		if (nearest != nullptr)
		{
			target->orderTarget.unit = nearest;
			helper::createBullet(WeaponId::AugurFinalHour, target, target->position.x, target->position.y, target->playerId,
				scbw::getAngle(target->position.x, target->position.y, nearest->getX(), nearest->getY()));
			scbw::hideAndDisableUnit(target);
			target->status |= UnitStatus::Invincible;
			scbw::createThingy(SpriteId::AugurDeathExplosion, target->getX(), target->getY(), target->playerId);
			scbw::playSound(scbw::randBetween(SoundId::AugurDeath01, SoundId::AugurDeath04), target);
			target->removeTimer = 5 * 32;
			target->userActionFlags |= 0x4;
			return;
		}
		else
		{
			target->remove();
		}
	}
}

namespace hooks
{
	static void weaponDamage_damageModifiers(WeaponDamageData& data);
	static void weaponDamage_lifesteal(WeaponDamageData& data);
	static void weaponDamage_behaviors(WeaponDamageData& data);
	static void weaponDamage_fatal(WeaponDamageData& data);

	/// Hooks into the Unit::damageWith() function.
	void weaponDamageHook(
		s32 damage,
		Unit* target,
		u8 weaponId,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor
	)
	{
		WeaponDamageData data =
		{
			damage,
			damage,
			0,
			weapons_dat::DamageType[weaponId],
			damageDivisor,
			weaponId,
			direction,
			target,
			attacker,
			attackingPlayerId,
			false,
			false
		};
		weaponDamage_root(data);
	}

	void weaponDamageCustom(
		s32 damage,
		Unit* target,
		u8 damageType,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor,
		bool ignoreFlatArmor
	)
	{
		WeaponDamageData data =
		{
			damage,
			damage,
			0,
			damageType,
			damageDivisor,
			WeaponId::Custom,
			direction,
			target,
			attacker,
			attackingPlayerId,
			false,
			ignoreFlatArmor
		};
		weaponDamage_root(data);
	}

	void weaponDamage_root(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.GetUnitExt(data.target);

		// The unit must neither be already dead nor invincible
		if (data.target == nullptr || data.target->hitPoints == 0 || data.target->status & UnitStatus::Invincible)
		{
			return;
		}

		// Demiurge - Gods Made Flesh - target is impervious to damage
		if (targetExt->timers[TimerGodsMadeFlesh] > 0)
		{
			data.finalDamage = 0;
			data.damageType = DamageType::Independent;
		}

		bool canDamage = true; // debug only!
		// Ai_ReactToHit crash - null town
		if (data.target->pAI != nullptr)
		{
			// Ai_ReactToHit crash - null town
			WorkerAi* ai = (WorkerAi*)data.target->pAI;
			if (ai->type == 2 || ai->type == 3)
			{
				if (ai->town == nullptr)
				{
					scbw::printf("Ai_ReactToHit crash - null town! Attacker id %d, Attacker player %d, Target id %d, Target player %d",
						data.attacker->id, data.attacker->playerId, data.target->id, data.target->playerId);
					canDamage = false;
				}
			}
		}

		if (!data.dontProcPhaseLink) // damage from phase link was already calculated
		{
			weaponDamage_damageModifiers(data);
		}

		if (!data.dontProcPhaseLink && scbw::hasClarionLinks(data.target))
		{
			scbw::spreadClarionDamage(data);
			return;
		}

		// Hallucinations deal half damage
		if (data.attacker != nullptr)
		{
			if (data.attacker->status & UnitStatus::IsHallucination)
			{
				data.finalDamage /= 2;
			}
		}

		// Hallucinations take double damage
		if (data.target->status & UnitStatus::IsHallucination)
		{
			data.finalDamage *= 2;
		}

		// Minimum amount of damage that can be dealt
		if (data.finalDamage < 128)
		{
			data.finalDamage = 128;
		}

		// Reduce Defensive Matrix
		if (data.target->defensiveMatrixHp != 0)
		{
			s32 d_matrix_reduceAmount;
			if (data.target->defensiveMatrixHp > data.finalDamage)
			{
				d_matrix_reduceAmount = data.finalDamage;
			}
			else
			{
				d_matrix_reduceAmount = data.target->defensiveMatrixHp;
			}
			data.finalDamage -= d_matrix_reduceAmount;
			data.target->reduceDefensiveMatrixHp(d_matrix_reduceAmount);
		}

		// Cliff Advantage - Armor
		if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor)
		{
			data.finalDamage -= std::min(scbw::getCliffAdvantage(data.attacker), 0) * -256; // not passing weapon allows for melee, which is desirable with armor
		}

		// Reduce Plasma Shields
		if (units_dat::ShieldsEnabled[data.target->id] && data.target->shields >= 256)
		{
			if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor)
			{
				s32 plasmaShieldUpg = scbw::getUpgradeLevel(data.target->playerId, UpgradeId::ProtossPlasmaShields) * 256;
				if (data.finalDamage > plasmaShieldUpg)
				{
					data.finalDamage -= plasmaShieldUpg;
				}
				else
				{
					data.finalDamage = 128;
				}
			}
			data.shieldReduceAmount = std::min(data.finalDamage, data.target->shields);
			data.finalDamage -= data.shieldReduceAmount;
		}

		// Apply armor
		bool shield_overlay = false;
		if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor)
		{
			s32 armorTotal = data.target->getArmor() * 256;
			if (data.attacker != nullptr)
			{
				armorTotal += scbw::get_aise_value(data.target, nullptr, AiseId::GetShepherds, 0, 0) * 256;
				if (data.target->id == UnitId::TerranMinotaur)
				{
					u16 angle = scbw::getAngle(data.target->position.x, data.target->position.y,
						data.attacker->position.x, data.attacker->position.y);
					u16 unit_angle = data.target->velocityDirection1;
					s32 arc = (f64)45 / 1.40625; // 90 degrees
					s32 delta = unit_angle - angle;
					delta = abs((delta + 128) % 256 - 128);
					if (delta <= arc)
					{
						armorTotal += 512;
						shield_overlay = true;
					}
				}
			}
			data.finalDamage -= std::min(data.finalDamage, armorTotal);
		}

		// Apply damage type/unit size factor
		s32 damage_factor = damageFactor[data.damageType].unitSizeFactor[units_dat::SizeType[data.target->id]];
		if (data.attacker != nullptr)
		{
			switch (data.attacker->id)
			{
				case UnitId::TerranHarakan:
				{
					if (units_dat::BaseProperty[data.target->id] & UnitProperty::Building)
					{
						damage_factor = 256;
					}
				} break;
			}
		}

		data.finalDamage = (data.finalDamage * damage_factor) / 256;
		if (data.shieldReduceAmount == 0 && data.finalDamage < 128)
		{
			data.finalDamage = 128;
		}

		weaponDamage_behaviors(data);
		if (data.attacker != nullptr)
		{
			weaponDamage_lifesteal(data);
		}
		if (data.finalDamage >= data.target->hitPoints)
		{
			weaponDamage_fatal(data);
		}

		// Deal damage, except for Independent damage, which always deals 0
		if (data.damageType != DamageType::Independent && data.target->hitPoints != 0 && data.target->defensiveMatrixHp == 0 && canDamage )
		{
			data.target->damageHp(data.finalDamage, data.attacker, data.attackingPlayerId, data.weaponId != WeaponId::Irradiate); // Prevent Seraphs from being continuously revealed to the irradiated target
		}

		// Reduce shields
		if (data.shieldReduceAmount != 0 && canDamage)
		{
			data.target->shields -= data.shieldReduceAmount;
			if (data.damageType != DamageType::Independent && data.target->shields != 0)
			{
				shield_overlay = true;
			}
		}

		if (shield_overlay)
		{
			scbw::createShieldOverlay(data.target, data.direction);
		}

		//Update unit strength data
		data.target->airStrength = helper::getUnitStrength(data.target, false);
		data.target->groundStrength = helper::getUnitStrength(data.target, true);
	}

	static void weaponDamage_damageModifiers(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.GetUnitExt(data.target);

		// Isthrathaleth Dark Swarm
		if (scbw::isUnderDarkSwarm(data.target) && weapons_dat::MaxRange[data.weaponId] > 32
			&& (data.damageDivisor == 1 || data.weaponId == WeaponId::AcidSpore)
			&& data.weaponId != WeaponId::SubterraneanSpines)
		{
			data.damageType = DamageType::Independent;
		}

		switch (data.weaponId) // Damage Calc
		{
			case WeaponId::SubterraneanTentacle:
			case WeaponId::SeekerSpores:
			{
				if (data.attacker)
				{
					UnitExt* attackerExt = globals.GetUnitExt(data.attacker);
					if (attackerExt->values[ValueAmitosis] == 1)
					{
						data.finalDamage /= 2;
						attackerExt->values[ValueAmitosis] = 0;
					}
				}
			} break;
		}

		// Observance Armor Rend
		data.finalDamage += 256 * targetExt->values[ValueObservanceRend];

		switch (data.weaponId) // Damage Divisor Calc
		{
			case WeaponId::ArchitectDispersiveSalvo:
			{
				// Reactive Payload - Architect behavior
				f64 div = 1.0;
				if (data.damageDivisor != 0)
				{
					div = (f64)data.damageDivisor;
				}
				s32 dmg = ((f64)data.finalDamage / div);
				data.finalDamage = dmg + targetExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction;
			} break;
			case WeaponId::AcidSpore:
			{
				// Searing Acid - Guardian behavior
				data.finalDamage += (targetExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction) + (data.damageDivisor * 10 * 256);
			} break;
			case WeaponId::GlaveWurm:
			case WeaponId::CharlatanFerventBlaster:
			{
				// Lithe Organelle - bounce damage
				f64 mul = 1.0;
				switch (data.damageDivisor)
				{
					case 0:
					{
						mul = 1.0;
					} break;
					case 1:
					{
						mul = 1.0 / 3.0; // glave wurm 1st bounce
					} break;
					case 2:
					{
						mul = 1.0 / 6.0;
					} break;
					default:
					{
						mul = 1.0 / 9.0; // glave wurm 4th bounce
					} break;
				};
				data.finalDamage *= mul;
				data.finalDamage += (s32)targetExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction;
			} break;
			default:
			{
				u32 divisor = data.damageDivisor;
				if (divisor == 0)
				{
					divisor = 1;
					scbw::printf("Incorrect weapon hit %d", data.weaponId);
				}
				data.finalDamage = data.finalDamage / divisor + (targetExt->values[ValueCorrosiveTouchStacks]* VithriliskCorrosiveTouchArmorReduction);
			}
		}

		switch (data.weaponId)
		{
			case WeaponId::QuaziliskAcridBarbs:
			{
				// Quazilisk - Melting Points
				f32 missingFactor = 1 - ((f32)data.target->hitPoints / data.target->getMaxHpRaw());
				f32 damageIncrease = floor(missingFactor / QuazaliskPassiveFactor);
				data.finalDamage += damageIncrease * HealthMultiplier;
			} break;
			case WeaponId::AlmaksaliskPiercingSpines:
			{
				// Almaksalisk - Flanking Spines
				if (scbw::isDebuffed(data.target))
				{
					data.damageType = DamageType::IgnoreArmor;
				}
			} break;
		}
	}

	static void weaponDamage_behaviors(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.GetUnitExt(data.target);

		if (data.attacker != nullptr)
		{
			UnitExt* attackerExt = globals.GetUnitExt(data.attacker);

			// Vithrilisk - Critical Mass (procced by any attack now)
			if (targetExt->values[ValueCorrosiveTouchStacks])
			{
				s32 range = 160;
				scbw::UnitFinder(
					data.target->getX() - 96,
					data.target->getY() - 96,
					data.target->getX() + 96,
					data.target->getY() + 96
				).forEach([&](Unit* target)
				{
					if (target->status & UnitStatus::Burrowed)
					{
						return;
					}
					if (target->status & UnitStatus::Invincible)
					{
						return;
					}
					helper::ApplySpores(target);
					Thingy* thingy = scbw::createThingy(SpriteId::Corrosive_Acid_Hit, target->getX(), target->getY(), 11);
					if (thingy != nullptr)
					{
						thingy->sprite->elevationLevel = target->sprite->elevationLevel + 1;
						helper::setThingyVisibilityFlags(thingy);
					}
				});
			}

			// Cabalist - Decloak, Silent Kill
			if (data.attacker->id == UnitId::ProtossCabalist && data.finalDamage < data.target->hitPoints)
			{
				scbw::decloakProc(data.attacker);
			}

			// Cantavis - Hallucinating
			if (targetExt->timers[TimerHallucinating] >= 1)
			{
				Unit* haluAttacker = data.attacker;
				if ((haluAttacker->id == UnitId::ProtossSteward
					|| haluAttacker->id == UnitId::ProtossPlasmaShell)
					&& data.attacker->interceptor.parent != nullptr)
				{
					haluAttacker = data.attacker->interceptor.parent;
				}
				UnitExt* haluAttackerExt = globals.GetUnitExt(haluAttacker);
				if (haluAttackerExt->units[UnitHallucinationPointer] == nullptr
					&& !(haluAttacker->status & UnitStatus::IsHallucination))
				{
					u32 haluPlayer = targetExt->values[ValueHallucinatingSourcePlayer];
					if (haluPlayer != haluAttacker->playerId && !scbw::isAlliedTo(haluPlayer, haluAttacker->playerId))
					{
						return;
					}
					Unit* hal_unit = helper::createHallucination(haluAttacker, haluPlayer);
					if (hal_unit != nullptr)
					{
						hal_unit->sprite->createOverlay(ImageId::HallucinationDeath3);
						haluAttackerExt->units[UnitHallucinationPointer] = hal_unit;

						if (!helper::placeHallucination(hal_unit))
						{
							helper::UnitDestructor(hal_unit);
						}
					}
				}
			}

			// Petrification - Gorgon behavior
			if (GorgonCondition(data.attacker, data.target))
			{
				scbw::refreshDebuffs(data.target);
			}

			// Ensnaring Brood - Queen enchantment ability
			if (attackerExt->timers[TimerEnsnaringBrood] > 0
				&& !(units_dat::BaseProperty[data.target->id] & UnitProperty::Building))
			{
				applyEnsnare(data.target);
			}

			switch (data.target->id)
			{
				case UnitId::ProtossSimulacrum:
				{
					// Autovitality - Simulacrum behavior
					if (!scbw::isAlliedTo(data.target->playerId, data.attacker->playerId)
						&& targetExt->timers[TimerSimulacrumClone] == 0)
					{
						targetExt->timers[TimerSimulacrumClone] = 5 * 24;
					}
				} break;
			}

			switch (data.weaponId)
			{
				case WeaponId::DemiurgeGalacticWeaver:
				{
					// Demiurge - Worship
					if (data.target->playerId != data.attacker->playerId
						&& data.target->getCurrentHpInGame() <= (data.target->getMaxHpInGame() / 2))
					{
						targetExt->values[ValueWorshipStack] = 1;
						targetExt->units[UnitWorshipDemiurge] = data.attacker;
						targetExt->values[ValueWorshipOriginalPlayer] = data.target->playerId;
						data.target->giveTo(data.attacker->playerId);
					}
				} break;
				case WeaponId::KonviliskSowingSpines:
				{
					// Fertile Spines - Konvilisk behavior
					scbw::applyParasite(data.target, data.attacker->playerId, data.attacker);
				} break;
				case WeaponId::PazuzuJupiterDriver:
				{
					// Deconstruction - Pazuzu behavior - apply stack
					if (!(scbw::unitIsPacifist(data.target))
						&& targetExt->values[ValueDeconstruction] == 0)
					{
						targetExt->values[ValueDeconstruction] = 1;
						targetExt->values[ValueDeconstructionPlayer] = data.attacker->playerId;
						s32 overlayOffset = scbw::getOverlayOffset(data.target->id);
						if (data.target->subunit != nullptr)
						{
							UnitExt* subunitExt = globals.GetUnitExt(data.target->subunit);
							subunitExt->values[ValueDeconstruction] = 1;
							subunitExt->values[ValueDeconstructionPlayer] = data.attacker->playerId;
							data.target->subunit->sprite->createOverlay(1332 + overlayOffset);
						}
						else
						{
							data.target->sprite->createOverlay(1332 + overlayOffset);
						}
					}
				} break;
				case WeaponId::SavantDiscoidRailgun:
				{
					// Savant Entropy Gauntlets behavior - Apply timer and overlay
					if (units_dat::BaseProperty[data.target->id] & UnitProperty::Spellcaster)
					{
						targetExt->timers[TimerEntropyGauntlets] = (5 * 24) + 1;
						targetExt->units[UnitDecaySavant] = data.attacker;
						u32 overlayImageId = ImageId::EntropyGauntletsOverlaySmall + scbw::getOverlayOffset(data.target->id);
						if (data.target->subunit != nullptr)
						{
							data.target->subunit->sprite->removeOverlay(overlayImageId);
							data.target->subunit->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
						}
						else
						{
							data.target->sprite->removeOverlay(overlayImageId);
							data.target->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
						}
					}
				} break;
				case WeaponId::CharlatanFerventBlaster:
				{
					// Charlatan - Leading the Blind passive
					if (!scbw::isAlliedTo(data.attacker->playerId, data.target->playerId))
					{
						scbw::applyNewHallucination(data.target, data.attacker->playerId, CharlatanHallucinationTimer);
					}
				} break;
				case WeaponId::Claws:
				{
					// Adrenal Glands - Zergling behavior
					if (attackerExt->values[ValueAdrenalStacks] < 4)
					{
						attackerExt->values[ValueAdrenalStacks] += 1;
					}
					attackerExt->timers[TimerAdrenal] = 49; // 49 is 2 seconds (48) + 1
				} break;
				case WeaponId::SouthpawUppercutLaser:
				{
					// Knockout Drivers - Southpaw behavior
					if (targetExt->values[ValueKnockoutDriversStacks] < 20)
					{
						targetExt->values[ValueKnockoutDriversStacks] += 1;
					}
					targetExt->timers[TimerKnockoutDrivers] = 49; // 49 is 2 seconds (48) + 1
					data.target->updateSpeed();
				} break;
				case WeaponId::MadrigalImperiosoMissiles:
				{
					// Dirge - Madrigal behavior
					scbw::sendGPTP_aise_cmd(data.target, nullptr, GptpId::DirgeImpale, 24, 0, 0);
					scbw::set_generic_rust_unit(data.target, RustId::DirgeAttacker, data.attacker);
				} break;
				case WeaponId::VorvalingClaws:
				{
					// Captivating Claws - Vorvaling behavior
					captivatingClawsApply(data.attacker, data.target);
				} break;
				case WeaponId::EcclesiastAstralBlaster:
				{
					// Astral Blessing - Ecclesiast behavior
					targetExt->values[ValueEcclesiastStack] += 1;
					targetExt->timers[TimerEcclesiast] = (24 * 5) + 1;
					targetExt->values[ValueEcclesiastPlayer] = data.attacker->playerId;
				} break;
				case WeaponId::SeraphObservance:
				{
					// Observance - Science Vessel ability
					if (data.weaponId == WeaponId::SeraphObservance
						&& (!scbw::isAlliedTo(data.attacker->playerId, data.target->playerId)))
					{
						attackerExt->timers[TimerObservanceOverlay] = 2;
						targetExt->timers[TimerObservanceDebuff] = 1 + (5 * 24);
						targetExt->timers[TimerObservancePeriod] = 1 + (5 * 24);
						s32 rend = targetExt->values[ValueObservanceRend];
						if (rend == 0)
						{
							s32 overlay = ImageId::ObservanceOverlaySmall - scbw::getOverlayOffset(data.target->id);
							data.target->sprite->createOverlay(overlay);
						}
						targetExt->values[ValueObservanceRend] += 1;
					}
				} break;
				case WeaponId::GorgrokorSerratedCleavers:
				{
					// Essence Drain - Gorgrokor Energy Steal
					if (units_dat::BaseProperty[data.target->id] & UnitProperty::Spellcaster)
					{
						u16 energy = std::min(data.target->energy, (u16)(data.finalDamage + data.shieldReduceAmount));
						data.target->energy -= energy;
						data.attacker->energy += energy;
					}
				} break;
				case WeaponId::MagisterNeutronCannon:
				{
					// Stellar Enforcement - Magister behavior
					if (!targetExt->values[ValueStellarEnforcement])
					{
						targetExt->values[ValueStellarEnforcement] = 1;
					}
				} break;
				case WeaponId::ApostleCharonRifle:
				{
					// Blood Tithe - Apostle behavior
					u32 range = 3 * 32;
					scbw::UnitFinder(
						data.target->getX() - range,
						data.target->getY() - range,
						data.target->getX() + range,
						data.target->getY() + range
					).forEach([&data](Unit* foundUnit)
					{
						if (foundUnit->playerId != data.attacker->playerId && !scbw::isAlliedTo(data.attacker->playerId, foundUnit->playerId))
						{
							return;
						}
						if (foundUnit == data.target)
						{
							return;
						}
						foundUnit->heal(data.finalDamage, data.target);
					});
				} break;
			}
		}
	}

	static void weaponDamage_lifesteal(WeaponDamageData& data)
	{
		u32 lifesteal = data.finalDamage + data.shieldReduceAmount;
		switch (data.weaponId)
		{
			case WeaponId::KaiserBlades:
			{
				// Ultrakor Saber Siphon passive
				lifesteal *= 0.5;
				data.attacker->heal(lifesteal, data.attacker);
			} break;
			case WeaponId::WendigoFusionSaw:
			{
				// Wendigo Automedicus passive (HP component)
				lifesteal *= 0.25;
				data.attacker->heal(lifesteal, data.attacker);
			} break;
			case WeaponId::AmaranthPsiCleaver:
			{
				// Eternal Service - Amaranth behavior
				const s32 hp = data.attacker->hitPoints / 256;
				const s32 maxHp = data.attacker->getMaxHpInGame();
				lifesteal *= MathUtils::map(hp, maxHp / 4, maxHp, 0.5, 0.2, true);
				data.attacker->healShields(lifesteal, data.attacker);
			} break;
		}
	}

	static void weaponDamage_fatal(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.GetUnitExt(data.target);

		switch (data.weaponId)
		{
			case WeaponId::Plague:
			{
				// Plague - Don't kill
				if (data.finalDamage >= data.target->hitPoints)
				{
					data.finalDamage = 0;
					data.target->setHp(256);
				}
			} break;
			case WeaponId::PsiStorm:
			{
				// Cantavis - Ascendancy
				if (data.attacker != nullptr && (data.attacker->status & UnitStatus::IsHallucination))
				{
					scbw::UnitFinder(
						data.target->getX() - 288,
						data.target->getY() - 288,
						data.target->getX() + 288,
						data.target->getY() + 288
					).forEach([&](Unit* unit)
					{
						if (unit->id != UnitId::ProtossCantavis)
						{
							return;
						}
						if (unit->isFrozen())
						{
							return;
						}
						bool show = true;
						if (unit->energy == unit->getMaxEnergy())
						{
							show = false;
						}
						unit->addUnitEnergy(5 * 256);
						if (show)
						{
							helper::addOverlayBelowMain(unit->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
						}
					});
				}
			} break;
		}

		// Apostle - Lazarus Agent
		if (targetExt->values[ValueLazarusAgent])
		{
			globals.initLazarusAgent(data.target->position.x, data.target->position.y,
				data.target->playerId, data.target->id, data.target->energy, targetExt->values[ValueLazarusAgent],
				(Unit*)targetExt->values[ValueLazarusAgentApostleSrc]);
		}

		// Ecclesiast - Astral Blessing proc
		if (targetExt->values[ValueEcclesiastStack])
		{
			s32 stacks = targetExt->values[ValueEcclesiastStack];
			s32 player = targetExt->values[ValueEcclesiastPlayer];
			const s32 range = 64;
			scbw::UnitFinder(
				data.target->getX() - range,
				data.target->getY() - range,
				data.target->getX() + range,
				data.target->getY() + range
			).forEach([&](Unit* unit)
			{
				if (player != unit->playerId && !scbw::isAlliedTo(player, unit->playerId))
				{
					return; // Ally Check
				}
				if (units_dat::ShieldsEnabled[unit->id] && unit->shields < units_dat::MaxShieldPoints[unit->id] * 256)
				{ // Shield Check for default behavior
					helper::addOverlayBelowMain(unit->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
				}
				else
				{ // Return if both checks failed
					return;
				}
				s32 healAmount = (stacks + 1) * 256;
				if (units_dat::ShieldsEnabled[unit->id])
				{ // Shield Heal
					unit->healShields(healAmount, data.target);
				}
			});
		}

		// Akistrokor - Enthralling Assault
		if (targetExt->values[ValueEnthrallingTalons] > 0)
		{
			// Preserve unit and start healing
			data.finalDamage = 0;
			data.target->setHp(256);
			targetExt->timers[TimerEnthrallingAssaultHeal] = 24 * 10;
			// Transfer unit ownership
			Unit* akistrokor = targetExt->units[UnitAttachedAkistrokor];
			data.target->giveTo(akistrokor->playerId);
			// Housekeeping
			targetExt->units[UnitAttachedAkistrokor] = nullptr;
			targetExt->values[ValueInfestedByAkistrokor] = 1;
		}

		switch (data.target->id)
		{ // TARGET SWITCH! MUST BE UNIT
			case UnitId::ProtossArchon:
			{
				// Archon - Astral Aegis
				scbw::UnitFinder(
					data.target->getX() - 96,
					data.target->getY() - 96,
					data.target->getX() + 96,
					data.target->getY() + 96
				).forEach([&](Unit* unit)
				{
					if (!units_dat::ShieldsEnabled[unit->id])
					{
						return;
					}
					u32 value = 50 * 256;
					unit->healShields(value, data.target);
				});
			} break;
			case UnitId::ProtossAurora:
			{
				// Last Orders - Aurora behavior
				if (targetExt->values[ValueLastOrdersInit] == 0)
				{
					targetExt->values[ValueLastOrdersInit] = 1;
					Unit* nearest = scbw::UnitFinder(
						data.target->getX() - 288,
						data.target->getY() - 288,
						data.target->getX() + 288,
						data.target->getY() + 288
					).getNearestTarget(data.target,
						[&](Unit* unit)
					{
						//Don't attack friendly / allied units
						if (unit->playerId == data.target->playerId)
						{
							return false;
						}
						if (scbw::isAlliedTo(data.target->playerId, unit->playerId))
						{
							return false;
						}
						if (unit->status & UnitStatus::Invincible)
						{
							return false;
						}
						if (unit->status & UnitStatus::InAir)
						{
							return false;
						}
						// TODO: iquare consultation
						// Is this needed? Seems like it's already guaranteed
						if (scbw::getDistanceFast(data.target->position.x, data.target->position.y, unit->position.x, unit->position.y) > 288)
						{
							return false;
						}
						return true;
					});
					if (nearest != nullptr)
					{
						data.finalDamage = 0;
						data.target->setHp(units_dat::MaxHitPoints[data.target->id] / 2);
						data.target->orderTo(OrderId::Move, nearest->position.x, nearest->position.y);
						data.target->status |= UnitStatus::CanNotReceiveOrders;
					}
				}
			} break;
			case UnitId::ProtossAugur:
			{
				// Augur - Death behavior
				augurSacrifice(data.target);
			} break;
		}

		if (data.attacker != nullptr)
		{
			UnitExt* attackerExt = globals.GetUnitExt(data.attacker);
			switch (data.weaponId)
			{
				case WeaponId::AlkajeliskPlasmidSpores:
				case WeaponId::AlkajeliskParasiticSpores:
				{
					// Alkajelisk - Parasite
					targetExt->values[ValueAlkajeliskParasite] = 1;
					targetExt->values[ValueAlkajeliskParasitePlayer] = data.attacker->playerId;
				} break;
				case WeaponId::KaiserBlades:
				{
					// Kaiser Rampage - Ultralisk behavior
					attackerExt->timers[TimerKaiserRampage] = 1 + (2 * 24);
					data.attacker->updateSpeed();
				} break;
				case WeaponId::NathrokorPlasmidBreath:
				{
					// Adrenal Frenzy - Nathrokor behavior
					s32 player = data.attacker->playerId;
					const s32 range = 48;
					scbw::UnitFinder(
						data.attacker->getX() - range,
						data.attacker->getY() - range,
						data.attacker->getX() + range,
						data.attacker->getY() + range
					).forEach([&](Unit* unit)
					{
						if (unit == nullptr
							|| (player != unit->playerId && !scbw::isAlliedTo(player, unit->playerId)))
						{
							return;
						}
						UnitExt* forEachExt = globals.GetUnitExt(unit);
						forEachExt->timers[TimerNathrokor] = (4 * 24) + 1;
						unit->updateSpeed();
					});
				} break;
				case WeaponId::ToxicSpores:
				case WeaponId::BroodliskSwarmingSpores:
				{
					// Incubators - Broodling/Broodlisk behavior
					s32 type = data.attacker->id;
					s32 supplyCount = scbw::canSpawnBroodling(data.target) ? 2 : 0;// scbw::getBroodlingCountFromSupply(data.target);
					scbw::createUnitFromAbility(type, supplyCount, data.attacker->playerId,
						data.target->position.x, data.target->position.y, data.attacker->removeTimer,
						scbw::getSourceQueens(data.attacker)[0]);
				} break;
				case WeaponId::CyclopsFlakCannons:
				{
					// Cyclops - Flak Cannon passive
					auto newAngle = scbw::getAngle(data.attacker->getX(), data.attacker->getY(), data.target->getX(), data.target->getY());
					helper::createBullet(WeaponId::CyclopsFragmentaryShells, data.attacker, data.target->getX(), data.target->getY(), data.attacker->playerId, newAngle);
				} break;
			}

			if (attackerExt->units[UnitMindController] != nullptr)
			{
				Unit* mindController = attackerExt->units[UnitMindController];
				mindController->addUnitEnergy(MindControlEnergyGain);
				mindController->sprite->createUnderlay(ImageId::SmallExplosion_Unused);
			}
		}
	}

	void applyEnsnare(Unit* unit) // 004F45E0
	{
		s32 baseProperty;
		u8 overlayPadding;
		Unit* victim;

		if (!(unit->ensnareTimer)
			&& !(unit->status & UnitStatus::Burrowed))
		{
			// Calculate overlay ID offset based on units_dat flags
			overlayPadding = 0;
			baseProperty = units_dat::BaseProperty[unit->id];
			if (baseProperty & UnitProperty::MediumOverlay)
			{
				overlayPadding = 1;
			}
			else if (baseProperty & UnitProperty::LargeOverlay)
			{
				overlayPadding = 2;
			}
			victim = unit;
			if (unit->subunit != nullptr)
			{
				victim = unit->subunit;
			}
			// Create overlay
			victim->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayPadding);
		}
		// Apply effect
		unit->ensnareTimer = 37;
		unit->updateSpeed();
	}

	bool GorgonCondition(Unit* attacker, Unit* target)
	{
		if (attacker != nullptr && target != nullptr)
		{
			if (attacker->id == UnitId::TerranGorgon)
			{
				if (target->isFrozen() || scbw::isSlowed(target))
				{
					return true;
				}
			}
		}
		return false;
	}

	u16 GetSporesOverlayId(Unit* target)
	{
		UnitExt* targetExt = globals.GetUnitExt(target);
		auto stacks = targetExt->values[ValueCorrosiveTouchStacks];
		auto overlayMultiplier = 0;
		if (stacks > 18)
		{
			overlayMultiplier++;
		}
		if (stacks > 35)
		{
			overlayMultiplier++;
		}
		if (stacks > 50)
		{
			overlayMultiplier++;
		}
		return overlayMultiplier + 4 * scbw::getOverlayOffset(target->id) + ImageId::AcidSpores_1_Overlay_Small;
	}

	// TODO: Refactor
	// Hooked to change Corrosive Spores timer - Pr0nogo
	s32 ApplySpores_hook(Unit* target) // 004F4480
	{
		u8 sporeCount;
		u32 overlayMultiplier;
		u8 overlayOffset;
		s32 overlay = 0;
		s32 v6;
		Unit* targetFinal;
		u32 result_sub_1;
		u32 acidTimeMax;
		u32 result;
		u32 v11_acidSporeTime;
		Image* image;
		Sprite* spriteFinal;

		UnitExt* targetExt = globals.GetUnitExt(target);
		targetExt->values[ValueCorrosiveTouchStacks]++;
		image = target->getOverlay(GetSporesOverlayId(target));
		if (image)
		{
			overlay = image->id;
		}
		if (!overlay)
		{
			target->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
			v6 = helper::getCorrosiveSporesImage(target);
			targetFinal = target->subunit;
			if (!targetFinal)
				targetFinal = target;
			spriteFinal = targetFinal->sprite;
			spriteFinal->createOverlay(v6, spriteFinal->position.x, spriteFinal->position.y, 0);
		}

		result_sub_1 = 0;
		acidTimeMax = 99999;
		result = 0;

		targetExt->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		return result;
	}
}