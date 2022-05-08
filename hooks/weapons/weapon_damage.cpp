#include "weapon_damage.h"
#include "../../SCBW/api.h"
#include <algorithm>
#include <cmath>
#include "math_utils.h"
#include "Utils/captivating_claws.h"
#include "hooks/cosmonarchy_finder.h"
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

bool darkswarm_blocks_damage(Unit* target, u32 weaponId, u32 damageDivisor) {
	return scbw::isUnderDarkSwarm(target)
		&& weapons_dat::MaxRange[weaponId] > 32
		&& (damageDivisor == 1 || weaponId == WeaponId::AcidSpore)
		&& weaponId != WeaponId::FlameThrower
		&& weaponId != WeaponId::SubterraneanSpines
		&& weaponId != WeaponId::Custom;
}

bool irradiateTarget(Unit* unit)
{
	return true; // Originally prohibited targeting eggs
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
		UnitExt* targetExt = globals.get_unit_ext(data.target);

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

		// TODO: remove this
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
			data.finalDamage /= 2;
			scbw::spreadClarionDamage(data);
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

		// Cliff Advantage - Armor | Disabled 10 Jan 2022
		/*
		if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor)
		{
			data.finalDamage -= std::min(scbw::getCliffAdvantage(data.attacker), 0) * -256; // not passing weapon allows for melee, which is desirable with armor
		}
		*/

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

			// Autocrat Controlled Opposition
			if (targetExt->values[ValueControlledOpposition])
			{
				armorTotal = std::max(armorTotal - (s32)targetExt->values[ValueControlledOpposition] * 256, 0);
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
					// Harakan Apollyoid Adhesive - Full damage to structures
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

		// Olympian Mythical Might - execute enemies below 10% HP
		if (!data.dontProcPhaseLink && (data.weaponId == WeaponId::OlympianMountainGun || data.weaponId == WeaponId::OlympianTitansFist))
		{
			if (data.target->hitPoints < data.target->getMaxHpRaw() * 0.1)
			{
				helper::playSoundFromSource(SoundId::OlympianMythicalMight01, data.target, 1, 0);
				data.finalDamage = data.target->hitPoints;
			}
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

		// New AI defense logic
		scbw::ask_for_ai_help(data.target, data.attacker);

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

		// Update unit strength data
		data.target->airStrength = helper::getUnitStrength(data.target, false);
		data.target->groundStrength = helper::getUnitStrength(data.target, true);
	}

	static void weaponDamage_damageModifiers(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.get_unit_ext(data.target);

		// Allied Invincibility Frames
		if (targetExt->timers[TimerAlliedInvFrames])
		{
			if (scbw::isAlliedTo(data.attackingPlayerId, targetExt->values[ValueAlliedInvPlayer]))
			{
				data.finalDamage = 0;
			}
		}

		// Isthrathaleth Dark Swarm
		if (darkswarm_blocks_damage(data.target, data.weaponId, data.damageDivisor))
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
					UnitExt* attackerExt = globals.get_unit_ext(data.attacker);
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
				// Searing Acid - Geszithalor behavior
				data.finalDamage += (targetExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction) + (data.damageDivisor * 10 * 256);
			} break;
			case WeaponId::LiiraliskHollowedSpores:
			case WeaponId::GlaveWurm:
			case WeaponId::CharlatanFerventBlaster:
			{
				// Lithe Organelle - bounce damage
				data.finalDamage *= 1 - data.damageDivisor * 0.33;
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
				data.finalDamage = data.finalDamage / divisor + (targetExt->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction);
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
		UnitExt* targetExt = globals.get_unit_ext(data.target);

		if (data.attacker != nullptr)
		{
			UnitExt* attackerExt = globals.get_unit_ext(data.attacker);

			// Ignore dark swarm!
			// START

			// Cabalist - Decloak, Silent Kill
			if (data.attacker->id == UnitId::ProtossCabalist)
			{
				if (data.finalDamage < data.target->hitPoints) {
					scbw::decloakProc(data.attacker);
				} else if (targetExt->timers[TimerDecloak] > 1) {
					targetExt->timers[TimerDecloak] = 1;
				}
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
				UnitExt* haluAttackerExt = globals.get_unit_ext(haluAttacker);
				if (haluAttackerExt->units[UnitHallucinationPointer] == nullptr
					&& !(haluAttacker->status & UnitStatus::IsHallucination))
				{
					if (scbw::isAlliedTo(haluAttacker->playerId, data.target->playerId))
					{
						return;
					}
					Unit* hal_unit = helper::createHallucination(haluAttacker, haluAttacker->playerId);
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

			// Ignore dark swarm!
			// END

			// Ensnaring Brood - Queen enchantment ability
			if (attackerExt->timers[TimerEnsnaringBrood] > 0
				&& !(units_dat::BaseProperty[data.target->id] & UnitProperty::Building)
				&& !scbw::isUnderDarkSwarm(data.target))
			{
				applyEnsnare(data.target);
			}

			switch (data.target->id)
			{
				case UnitId::ProtossSimulacrum:
				{
					// Autovitality - Simulacrum behavior
					if (!scbw::isAlliedTo(data.target->playerId, data.attacker->playerId)
						&& !scbw::isUnderDarkSwarm(data.target)
						&& data.shieldReduceAmount >= data.target->shields
						&& data.finalDamage
						&& targetExt->timers[TimerSimulacrumClone] == 0)
					{
						targetExt->timers[TimerSimulacrumClone] = 4 * 24;
					}
				} break;
				case UnitId::SharedStriga:
				{
					// Striga - Fleeting Focus
					if (!(units_dat::BaseProperty[data.attacker->id] & UnitProperty::Building)
						&& !scbw::isUnderDarkSwarm(data.target))
					{
						const u32 dist = scbw::getDistanceFast(data.attacker->getX(), data.attacker->getY(), data.target->getX(), data.target->getY());
						if (dist < 2 * 32)
						{
							const u32 displacementTime = 5;
							Vector32 diff = scbw::getVectorBetweenPoints(data.target->sprite->position, data.attacker->sprite->position);
							Vector32f displacement = scbw::getNormalizedVector(diff);
							displacement.x *= 1 * 32.0 / displacementTime;
							displacement.y *= 1 * 32.0 / displacementTime;

							attackerExt->timers[TimerStrigaRepulsion] = displacementTime;
							attackerExt->repulsionVector = displacement;
						}
					}
				} break;
				case UnitId::TerranCuirass:
				{
					// Cuirass - Stonewall Protocol
					if (!targetExt->values[ValueIsSelfReviving]
						&& data.target->hitPoints < data.finalDamage)
					{
						targetExt->values[ValueIsSelfReviving] = 1;
						data.target->hitPoints = CuirassSelfReviveHP;
						data.finalDamage = 0;
						data.target->orderComputer(OrderId::Stop);
						data.target->status |= UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
						helper::playSoundFromSource(SoundId::Misc_TPwrDown_wav, data.target, 1, 0);
					}
				} break;
			}

			// These bypass Dark Swarm
			switch (data.weaponId)
			{
				case WeaponId::AnthelionEventHorizon:
				{
					targetExt->timers[TimerAnthelionGravityWell] = 2;
				} break;
				case WeaponId::Claws:
				{
					// Adrenal Glands - Zergling behavior
					if (attackerExt->values[ValueAdrenalStacks] < 4)
					{
						attackerExt->values[ValueAdrenalStacks] += 1;
					}
					attackerExt->timers[TimerAdrenal] = ZethrokorAdrenalGlandsTimer;
				} break;
				case WeaponId::LuminaryAuralAssault:
				{
					// Aural Assault - Luminary ability
					u32 overlay = ImageId::OpticalFlareHit_Small + scbw::getOverlayOffset(data.target->id);
					Unit* overlayUnit = data.target;
					if (data.target->subunit != nullptr)
					{
						targetExt = globals.get_unit_ext(data.target->subunit);
						overlayUnit = data.target->subunit;
					}
					overlayUnit->sprite->createTopOverlay(overlay, 0, 0, 0);
					targetExt->timers[TimerAuralAssaultBlind] = 2 * 24 + 1;
				} break;
				case WeaponId::VorvalingClaws:
				{
					// Captivating Claws - Vorvaling behavior
					captivatingClawsApply(data.attacker, data.target);
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
			}

			if (!scbw::isUnderDarkSwarm(data.target))
			{
				switch (data.weaponId)
				{
					case WeaponId::EmpressFadingBioplasma:
					{
						// Empress - Anodyne Annex - add stack
						targetExt->units[UnitEmpressSource] = data.attacker;
						targetExt->values[ValueEmpressPassiveStacks]++;
						targetExt->values[ValueEmpressSourcePlayer] = data.attackingPlayerId;
					} break;
					case WeaponId::NathrokorPlasmidBreath:
					{
						// Nathrokor - Adrenal Frenzy
						targetExt->nathrokorTargetStacks[data.attackingPlayerId]++;
					} break;
					case WeaponId::DemiurgeGalacticWeaver:
					{
						// Demiurge - Worship
						if (data.target->playerId != data.attacker->playerId
							&& data.target->getCurrentHpInGame() <= data.target->getMaxHpInGame() / 2
							&& data.target->hitPoints > data.finalDamage)
						{
							auto& vec = attackerExt->unitVectors[UnitVectorWorshipersDemiurge];
							if (std::find(vec.begin(), vec.end(), data.target) == vec.end())
							{
								vec.push_back(data.target);
							}

							targetExt->values[ValueWorshipStack] = 1;
							scbw::mind_control(data.attacker, data.target);

							helper::playSoundFromSource(SoundId::DemiurgeWorship01, data.target, 1, 0);
							scbw::mind_control(data.attacker, data.target);
						}
					} break;
					case WeaponId::KonviliskSowingSpines:
					{
						// Fertile Spines - Konvilisk behavior
						scbw::applyParasite(data.target, data.attacker->playerId, data.attacker);
					} break;
					case WeaponId::GorgonArtificeBattery:
					{
						// Petrification - Gorgon behavior
						if (targetExt->values[ValuePetrificationStacks] < 20) {
							targetExt->values[ValuePetrificationStacks] += 1;
						}
						targetExt->timers[TimerPetrification] = 48 + 1;
					} break;
					case WeaponId::PazuzuJupiterDriver:
					{
						// Deconstruction - Pazuzu behavior - apply stack
						if (!scbw::unitIsPacifist(data.target))
						{
							targetExt->values[ValueDeconstruction]++;
							targetExt->values[ValueDeconstructionPlayer] = data.attacker->playerId;
							s32 overlayOffset = scbw::getOverlayOffset(data.target->id);
							if (data.target->subunit != nullptr)
							{
								UnitExt* subunitExt = globals.get_unit_ext(data.target->subunit);
								subunitExt->values[ValueDeconstruction]++;
								subunitExt->values[ValueDeconstructionPlayer] = data.attacker->playerId;
								if (!data.target->subunit->sprite->getOverlay(ImageId::DeconstructionSmall + overlayOffset))
								{
									data.target->subunit->sprite->createOverlay(ImageId::DeconstructionSmall + overlayOffset);
								}
							}
							else
							{
								if (!data.target->sprite->getOverlay(ImageId::DeconstructionSmall + overlayOffset))
								{
									data.target->sprite->createOverlay(ImageId::DeconstructionSmall + overlayOffset);
								}
							}
						}
					} break;
					case WeaponId::AutocratEminanceRifle:
					{
						// Apply Controlled Opposition
						targetExt->values[ValueControlledOpposition]++;
					} break;
					case WeaponId::SavantDiscoidRailgun:
					{
						u32 target_offset = scbw::getOverlayOffset(data.target->id);
						if (scbw::get_generic_rust_value(data.attacker, RustId::ToggleSavantOnhit)) {
							// Power Siphon
							u32 energy = std::min(data.target->energy, (u16)(10 * 256));
							data.target->energy -= energy;
							data.attacker->healEnergy(energy);
							helper::createThingy(SpriteId::Feedback_Hit_Small + target_offset,
								data.target->position.x, data.target->position.y, data.attackingPlayerId);
						} else {
							// Entropy Gauntlet
							u32 energy = std::min(data.attacker->energy, (u16)(10 * 256));
							data.attacker->energy -= energy;
							data.finalDamage += energy;
							helper::createThingy(SpriteId::EntropyGauntletsOverlay,
								data.attacker->position.x, data.attacker->position.y, data.attackingPlayerId);
						}
					} break;
					case WeaponId::HeraclesJavelinCannons:
					{
						data.finalDamage += attackerExt->values[ValueNemeanReactorsStack];
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						// Charlatan - Leading the Blind passive
						if (!scbw::isAlliedTo(data.attacker->playerId, data.target->playerId))
						{
							scbw::applyNewHallucination(data.target, data.attacker->playerId, CharlatanHallucinationTimer);
						}
					} break;
					case WeaponId::MadrigalImperiosoMissiles:
					{
						// Dirge - Madrigal behavior
						targetExt->timers[TimerDirgeFuse] = MadrigalDirgeTimer;
						targetExt->values[ValueDirgeStacks] += 1;
						targetExt->values[ValueDirgeSourcePlayer] = data.attackingPlayerId;
						targetExt->units[UnitDirgeSource] = data.attacker;
					} break;
					case WeaponId::HierophantPhaseSignifier:
					{
						targetExt->values[ValueSignifyStacks]++;
						targetExt->timers[TimerSignify] = 3 * 24 + 1;
						auto& vec = targetExt->valueVectors[ValueVectorSignifySourcePlayer];
						if (std::find(vec.begin(), vec.end(), data.attackingPlayerId) == vec.end())
						{
							vec.push_back(data.attackingPlayerId);
						}
						scbw::updateSignifyStatus(data.target, targetExt);
					} break;
					case WeaponId::EcclesiastAstralBlaster:
					{
						// Astral Blessing - Ecclesiast behavior
						const s32 range = 64;
						scbw::UnitFinder(
							data.target->getX() - range,
							data.target->getY() - range,
							data.target->getX() + range,
							data.target->getY() + range
						).forEach([&](Unit* unit)
							{
								if (data.attackingPlayerId != unit->playerId &&
									!scbw::isAlliedTo(data.attackingPlayerId, unit->playerId))
								{
									// Ally Check
									return;
								}
								if (units_dat::ShieldsEnabled[unit->id] && unit->shields < units_dat::MaxShieldPoints[unit->id] * 256)
								{ 
									// Shield Check for default behavior
									helper::addOverlayBelowMain(unit->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
								}
								else
								{ 
									// Return if both checks failed
									return;
								}
								if (units_dat::ShieldsEnabled[unit->id])
								{ 
									// Shield Heal
									unit->healShields(2 * 256, data.target);
								}
							});
					} break;
					case WeaponId::MagisterNeutronCannon:
					{
						// Stellar Enforcement - Magister behavior
						if (!targetExt->values[ValueStellarEnforcement])
						{
							targetExt->values[ValueStellarEnforcement] = 1;
						}
					} break;
					case WeaponId::VithriliskCorrosiveSpores:
					{
						// Vithrilisk - Critical Mass
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
									if (scbw::isAlliedTo(data.attacker->playerId, target->playerId))
									{
										return;
									}
									helper::ApplySpores(target);
									Thingy* thingy = helper::createThingy(SpriteId::Corrosive_Acid_Hit, target->getX(), target->getY(), 11);
									if (thingy != nullptr)
									{
										thingy->sprite->elevationLevel = target->sprite->elevationLevel + 1;
										helper::setThingyVisibilityFlags(thingy);
									}
								});
						}
					} break;
					case WeaponId::CyclopsFlakCannons:
					{
						// Cyclops - Flak Cannons
						targetExt->timerVectors[TimerVectorCyclopsFuse].push_back(1 * 24 + 1);
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						// Claymore - Kelvin Munitions - Slow primary target
						if (!targetExt->values[ValueKelvinMunitionsSlow])
						{
							targetExt->values[ValueKelvinMunitionsSlow] = 1;
						}
					} break;
					case WeaponId::MagnetarNewtonianShockCannon:
					{
						// Magnetar - Law of Attraction - Pull target to attacker
						if (!(units_dat::BaseProperty[data.target->id] & UnitProperty::Building)
							|| (data.target->status & UnitStatus::InAir
								&& data.target->mainOrderId != OrderId::Liftoff
								&& data.target->mainOrderId != OrderId::BuildingLand))
						{
							const u32 dist = scbw::getDistanceFast(data.attacker->getX(), data.attacker->getY(), data.target->getX(), data.target->getY());
							const u32 displacementTime = 5;
							Vector32 diff = scbw::getVectorBetweenPoints(data.target->sprite->position, data.attacker->sprite->position);
							Vector32f displacement = scbw::getNormalizedVector(diff);
							displacement.x *= 2 * 32.0 / displacementTime;
							displacement.y *= 2 * 32.0 / displacementTime;
							targetExt->timers[TimerMagnetarAttraction] = displacementTime;
							targetExt->repulsionVector = displacement;
						}
					} break;
				}
			}
		}
	}

	static void weaponDamage_lifesteal(WeaponDamageData& data)
	{
		u32 damageDealt = data.finalDamage + data.shieldReduceAmount;
		switch (data.weaponId)
		{
			case WeaponId::KaiserBlades:
			{
				// Ultrakor Saber Siphon passive
				data.attacker->heal(damageDealt * 0.5, data.attacker);
			} break;
			case WeaponId::AmaranthPsiCleaver:
			{
				// Eternal Service - Amaranth behavior
				const s32 hp = data.attacker->hitPoints / 256;
				const s32 maxHp = data.attacker->getMaxHpInGame();
				u32 shieldsHeal = damageDealt * MathUtils::map(hp, maxHp / 4, maxHp, 0.5, 0.2, true);
				data.attacker->healShields(shieldsHeal, data.attacker);
			} break;
			case WeaponId::AxitriliskSearingVenom:
			{
				// Axitrilisk passive
				const s32 shields = data.attacker->getCurrentShieldsInGame();
				const s32 maxShields = data.attacker->getMaxShieldsInGame();
				u32 lifeHeal = damageDealt * MathUtils::map(shields, 0, maxShields, 1.0, 0.0, true);
				data.attacker->heal(lifeHeal, data.attacker);

				const s32 hp = data.attacker->getCurrentHpInGame();
				const s32 maxHp = data.attacker->getMaxHpInGame();
				u32 shieldsHeal = damageDealt * MathUtils::map(hp, 0, maxHp, 1.0, 0.0, true);
				data.attacker->healShields(shieldsHeal, data.attacker);
			} break;
		}
	}

	static void weaponDamage_fatal(WeaponDamageData& data)
	{
		UnitExt* targetExt = globals.get_unit_ext(data.target);

		switch (data.weaponId)
		{
			case WeaponId::Plague:
			{
				// Plague - Don't kill
				data.finalDamage = 0;
				data.target->setHp(256);
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

		// Empress - Anodyne Annex - zombify target
		if (targetExt->values[ValueEmpressPassiveStacks]
			&& !targetExt->values[ValueIsZombified])
		{
			data.finalDamage = 0;
			data.target->hitPoints = 0;
			data.target->shields = 0;
			data.target->healOverflow(targetExt->values[ValueEmpressPassiveStacks] * 256 * 2.5, targetExt->units[UnitEmpressSource]);
			data.target->removeTimer = 1 + targetExt->values[ValueEmpressPassiveStacks] * 24 * 0.25;
			data.target->giveTo(targetExt->values[ValueEmpressSourcePlayer]);
			targetExt->values[ValueEmpressPassiveStacks] = 0;
			targetExt->values[ValueIsZombified] = 1;
			targetExt->values[ValueAlliedInvPlayer] = targetExt->values[ValueEmpressSourcePlayer];
			targetExt->timers[TimerAlliedInvFrames] = 24;
			data.target->sprite->createOverlay(ImageId::MindControlHit_Small + scbw::getOverlayOffset(data.target->id));
			helper::playSoundFromSource(scbw::randBetween(SoundId::EmpressZombify01, SoundId::EmpressZombify03), data.target, 1, 0);
		}

		// Apostle, Anticthon - Revive
		if (data.finalDamage > 0 && (targetExt->values[ValueLazarusAgent] || targetExt->timers[TimerAnticthonRevive]))
		{
			u32 hp = targetExt->values[ValueLazarusAgent] * ApostleLazarusAgentHeal;

			if (targetExt->timers[TimerAnticthonRevive])
			{
				const u32 radius = 8 * 32;
				scbw::UnitFinder(
					data.target->getX() - radius,
					data.target->getY() - radius,
					data.target->getX() + radius,
					data.target->getY() + radius
				).forEach([&](Unit* unit)
				{
					UnitExt* unitExt = globals.get_unit_ext(unit);
					u32 dist = scbw::getDistanceFast(data.target->position, unit->position);
					if (unit->id == UnitId::SharedAnticthon && dist <= 4 * 32)
					{
						if (scbw::isAlliedTo(data.target->playerId, unit->playerId))
						{
							hp += (1 + unitExt->values[ValueAnticthonStacks]) * 256;
							unitExt->values[ValueAnticthonStacks]++;
						}
					}
				});
			}

			WorkerAi* ai = ((WorkerAi*)data.target->pAI);
			AiTown* town = nullptr;
			if (ai)
			{
				town = ai->town;
			}

			globals.initRevive(data.target->position.x, data.target->position.y, data.target->playerId, data.target->id, data.target->energy, hp, town);
		}

		if (targetExt->values[ValueBlissfulEmbraceOriginalPlayer] && units_dat::BaseProperty[data.target->id] & UnitProperty::Building) {
			data.target->giveTo(targetExt->values[ValueBlissfulEmbraceOriginalPlayer] - 1);
			targetExt->values[ValueBlissfulEmbraceOriginalPlayer] = 0;

			data.finalDamage = 0;
			data.target->setHp(targetExt->values[ValueBlissfulEmbraceOriginalHealth]);

			// Make this a sprite
			u32 image_id = ImageId::ZergAirDeathExplosion_Small;
			if (units_dat::BaseProperty[data.target->id] & UnitProperty::LargeOverlay) {
				image_id = ImageId::ZergBuildingExplosion;
			} else if (units_dat::BaseProperty[data.target->id] & UnitProperty::MediumOverlay) {
				image_id = ImageId::ZergAirDeathExplosion_Large;
			}
			data.target->sprite->createOverlay(image_id);

			// Create busy overlays for buildings only
			if (units_dat::BaseProperty[data.target->id] & UnitProperty::Building) {
				u32 image_id = ImageId::InfestedOverlay_Large;
				if (scbw::unit_id_is_smaller_building(data.target->id)) {
					image_id = ImageId::InfestedOverlay_Small;
				}
				data.target->removeOverlay(image_id);
			}
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
					u32 value = ArchonAstralAegisShieldHeal;
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
						// Don't attack friendly / allied units
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
						if (!unit->isVisibleTo(data.target->playerId)) {
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
		}

		if (data.attacker != nullptr)
		{
			UnitExt* attackerExt = globals.get_unit_ext(data.attacker);
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
			}

			// Mind Control - energy restore
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

	u16 GetSporesOverlayId(Unit* target)
	{
		UnitExt* targetExt = globals.get_unit_ext(target);
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

		UnitExt* targetExt = globals.get_unit_ext(target);
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
			{
				targetFinal = target;
			}
			spriteFinal = targetFinal->sprite;
			spriteFinal->createOverlay(v6, 0, 0, 0);
		}

		result_sub_1 = 0;
		acidTimeMax = 99999;
		result = 0;

		targetExt->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		return result;
	}
}