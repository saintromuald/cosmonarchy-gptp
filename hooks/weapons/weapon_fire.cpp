#include "weapon_fire.h"
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "SCBW/UnitFinder.h"
#include "hooks/cosmonarchy_finder.h"
#include "math_utils.h"
#include "hooks/byte_operations_utils.h"
#include "globals.h"
#include "hooks/unit_stats/weapon_range.h"
#include "weapon_damage.h"
#include "weapon_spellhit.h"
#include <set>
#include "weapon_cooldown.h"

static bool madrigal_furia_match(Unit* madrigal, Unit* unit)
{
	if (unit == madrigal)
	{
		return false;
	}
	if (unit == madrigal->order_target.unit)
	{
		return false;
	}
	if (scbw::is_allied_to(madrigal->player, unit->player))
	{
		return false;
	}
	if (!madrigal->can_attack_target(unit))
	{
		return false;
	}
	return true;
}

static bool alkajelisk_spore_match(Bullet* bullet, Unit* unit)
{
	if (unit->status & UnitStatus::Invincible)
	{
		return false;
	}
	if (scbw::is_allied_to(bullet->srcPlayer, unit->player))
	{
		return false;
	}
	return true;
}

namespace
{
	typedef void(__stdcall* GetWeaponHitPosFunc)(Unit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
}

//-------- Actual hook functions --------//

namespace hooks
{
	static void launch_missile(Unit* attacker, UnitExt* attackerExt, Unit* target, u8 weaponId)
	{
		s32 posX;
		s32 posY;
		if (weapons_dat::Behavior[weaponId] & WeaponBehavior::AppearOnTargetUnit)
		{
			posX = target->get_x();
			posY = target->get_y();
		}
		else
		{
			posX = attacker->get_x();
			posY = attacker->get_y();
		}
		auto newAngle = scbw::get_angle(attacker->get_x(), attacker->get_y(), target->get_x(), target->get_y());
		helper::create_bullet(weaponId, attacker, posX, posY, attacker->player, newAngle);
		if (attackerExt->values[ValueDeconstruction])
		{
			scbw::do_deconstruction_damage(attacker, weaponId, false);
		}
	}

	bool fire_multiple(Unit* attacker, UnitExt* attackerExt, u32 weaponId, u32 maxTargets, u32 flags, bool ignoreMain)
	{
		bool fired = false;
		std::unordered_set<Unit*> targets;
		if (!ignoreMain
			&& attacker->order_target.unit
			&& attacker->main_order_id != OrderId::Follow
			&& attacker->main_order_id != OrderId::Patrol)
		{
			targets.insert(attacker->order_target.unit);
			launch_missile(attacker, attackerExt, attacker->order_target.unit, weaponId);
			fired = true;
		}
		Unit* old = attacker->order_target.unit;
		for (s32 i = 0; i < maxTargets || maxTargets <= 0; i++)
		{
			u32 range = hooks::get_max_weapon_range(attacker, weaponId, 0);
			auto filter = cosmonarchy_finder_match(attacker, flags, {});
			Unit* newTarget = scbw::UnitFinder::get_nearest_target(
				attacker->get_x() - range,
				attacker->get_y() - range,
				attacker->get_x() + range,
				attacker->get_y() + range,
				attacker,
				[&](Unit* target)
			{
				if (target == attacker
					|| targets.find(target) != targets.end()
					|| scbw::is_allied_to(attacker->player, target->player)
					|| !attacker->can_attack_target(target)
					|| !filter(target)
					|| (targets.size() >= maxTargets && maxTargets > 0))
				{
					return false;
				}
				return true;
			});
			if ((targets.size() >= maxTargets && maxTargets > 0) || !newTarget)
			{
				break;
			}
			attacker->order_target.unit = newTarget;
			launch_missile(attacker, attackerExt, newTarget, weaponId);
			fired = true;
			targets.insert(newTarget);
		}
		attacker->order_target.unit = old;
		return fired;
	}

	// Fires a weapon from the @p unit.
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fireWeapon().
	void fire_weapon(Unit* unit, u8 wId)
	{
		if (unit == nullptr)
		{
			return;
		}
		UnitExt* unitExt = globals.get_unit_ext(unit);
		u8 weaponId = wId;
		unitExt->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::DamageFactor[weaponId] + 1;
		unitExt->values[ValueAI_PreventTargeting] = 0;
		// Retrieve the spawning position for the bullet.
		s32 x, y;
		u8 behavior = weapons_dat::Behavior[weaponId];
		if (weaponId == WeaponId::SavantPowerSiphon)
		{
			behavior = WeaponBehavior::AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior::AppearOnTargetUnit)
		{
			if (unit->order_target.unit == nullptr)
			{
				return;
			}
			getWeaponHitPos(unit, &x, &y);
		}
		else if (behavior == WeaponBehavior::AppearOnTargetSite)
		{
			x = unit->order_target.pt.x;
			y = unit->order_target.pt.y;
		}
		else
		{
			s32 forwardOffset = weapons_dat::ForwardOffset[weaponId];

			x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
			y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1)
				- weapons_dat::VerticalOffset[weaponId];
		}

		// Cloaked Units - Decloak
		// Cabalist uses silent strike, so the check has to be in weapon damage
		// It is a melee unit, so this check is just a failsafe
		if (unit->id != UnitId::ProtossCabalist)
		{
			scbw::decloak_proc(unit);
		}

		if (weapons_dat::FlingyId[weaponId] != 0)
		{
			// Lurker Seismic DrolethSpines behavior
			if (weaponId == WeaponId::LakiziliskSubterraneanSpines)
			{
				unitExt->timers[TimerLurkerETA] = 17;
				auto range = weapons_dat::MaxRange[weaponId] + 16;
				s32 angle = scbw::get_angle(unit->get_x(), unit->get_y(), unit->order_target.unit->get_x(), unit->order_target.unit->get_y());
				s32 targ_x = unit->get_x() + scbw::get_polar_x(range, angle);
				s32 targ_y = unit->get_y() + scbw::get_polar_y(range, angle);
				unitExt->values[ValueSpineX] = targ_x;
				unitExt->values[ValueSpineY] = targ_y;
				if (unit->order_target.unit != nullptr)
				{
					unitExt->values[ValueLurkerPickTarget] = (u32)unit->order_target.unit;
				}
				globals.clear_lakizilisk(unit);
			}

			if (unit->id != UnitId::TerranWyvern // disable default weapon for custom behaviors
				&& unit->id != UnitId::ProtossAnthelion)
			{
				// Madrigal Tempo Change weapon switch
				if (weaponId == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1)
				{
					weaponId = WeaponId::MadrigalImperiosoMissiles;
				}

				// Nemean Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weaponId == WeaponId::HeraclesJavelinCannons)
				{
					unitExt->values[ValueNemeanReactorsStack]++;
					unitExt->timers[TimerNemeanReactors] = 49;
					helper::play_sound_from_unit(1372, unit, 1, 0);
				}

				// Madcap - Maverick Feeders passive
				if (unit->id == UnitId::TerranMadcap)
				{
					s32 stacks = unitExt->values[ValueMadcapStacks];
					unitExt->values[ValueMadcapStacks] = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
				}

				// Track launch position
				unitExt->last_bullet_launch_position = unit->position;

				switch (weaponId)
				{
					// Missile spawn positions start
					case WeaponId::AxitriliskSearingVenom:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueAxitriliskMissilePos, 1, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueCharlatanMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ExemplarPlasmaTorpedo:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueExemplarMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::EmpyreanPlasmaBarrage:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueEmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::StarSovereignCelestialCannon:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueSovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueClaymoreMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::PenumbraErebolShockCannon:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValuePenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::WraithBurstLasers:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueWraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr, 0, 2);
					} break;
					case WeaponId::CentaurSunchaserMissiles:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueCentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::PhobosArgosLaserBatteries:
					{
						// Phobos - Argent Capacitors passive
						if (unit->id == UnitId::TerranPhobos && unitExt->values[ValuePhobosDischarge] == 0)
						{
							unit->addUnitEnergy(5 * 256);
							if (unit->energy == unit->get_max_energy())
							{
								unitExt->values[ValuePhobosDischarge] = 1;
								unit->ground_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								unit->air_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								helper::play_sound_from_unit(SoundId::PhobosArgentCapacitorsProc, unit, 1, 0);
								unit->sprite->create_overlay(ImageId::OpticalFlareHit_Large);
							}
						}
						scbw::set_missile_source_pos(unit, weaponId, ValuePhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::SkithrokorSkitteringSpores:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueSkithrokorMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::LiiraliskHollowedSpores:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueLiiraliskMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::BactaliskCorrosiveAcid:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueBactaliskMissilePos, 2, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::KalkaliskViscousAcid:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueKalkaliskMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::KeskathalorScreamingVenom:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueKeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr, 0, 1);
					} break;
					case WeaponId::SporeColonySeekerSpores:
					{
						scbw::set_missile_source_pos(unit, weaponId, ValueSporeColonyMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					// Missile spawn positions end
					case WeaponId::SalamanderPerditionBarrage:
					{
						// Salamander missile target positions
						auto rotate = unitExt->values[ValueSalamanderAtkPos];
						if (rotate == 1)
						{
							helper::create_bullet(weaponId, unit, x, y, unit->player, unit->current_direction_1);
						}
						else
						{
							scbw::range_modified_fire(weaponId, unit, x, y, 32 - 32 * rotate);
						}
						rotate++;
						if (rotate > 2)
						{
							rotate = 0;
						}
						unitExt->values[ValueSalamanderAtkPos] = rotate;
					} break;
					case WeaponId::MinotaurAsterionLaserBattery:
					{
						// Monoceros Targeting - Minotaur
						fire_multiple(unit, unitExt, weaponId, 5, Flags::Enemy | Flags::FrontalArc);
					} break;
					case WeaponId::SirenCharybdalRifle:
					{
						u32 count = 0;

						s32 radius = 3 * 32;
						scbw::UnitFinder(
							unit->position.x - radius,
							unit->position.y - radius,
							unit->position.x + radius,
							unit->position.y + radius
						).for_each([&](Unit* pickedUnit) {
							if (!(pickedUnit->status & UnitStatus::Completed)
								|| unit == pickedUnit
								|| scbw::is_allied_to(pickedUnit->player, unit->player)
								|| scbw::get_distance_fast(unit->position, pickedUnit->position) > radius)
							{
								return;
							}

							count += 1;
						});
						if (count) {
							helper::play_sound_from_unit(SoundId::SirenTertiaryEyesCast01, unit, 1, 0);
						}

						// Tertiary Eyes - Siren
						fire_multiple(unit, unitExt, weaponId, 1 + count, Flags::Enemy | Flags::FrontalArc);
					} break;
					case WeaponId::MadrigalFuriaMissiles:
					{
						// Madrigal Furia Salvo
						if (unit->order_target.unit != nullptr)
						{
							fire_multiple(unit, unitExt, WeaponId::MadrigalFuriaMissiles, 0, Flags::Enemy);
						}
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						// Crushing Tide - Izirokor
						fire_multiple(unit, unitExt, weaponId, 3, Flags::Enemy);
					} break;
					default:
					{
						helper::create_bullet(weaponId, unit, x, y, unit->player, unit->current_direction_1);
					} break;
				}
			}
		}

		// Deconstruction - Pazuzu passive
		if (unitExt->values[ValueDeconstruction])
		{
			scbw::do_deconstruction_damage(unit, weaponId);
		}
	}

	void progress_bullet_flight(Bullet* bullet)
	{
		Bullet* v1;
		Unit* target;
		Sprite* bulletSprite;
		Image* i;

		if (bullet->weaponId == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->attackTarget.unit == nullptr)
			{
				u32 range = AlkajeliskParasiticSporesSeekRange; // seek distance
				auto seekTarget = scbw::UnitFinder::get_nearest_target(
					bullet->position.x - range,
					bullet->position.y - range,
					bullet->position.x + range,
					bullet->position.y + range,
					bullet,
					[&](Unit* unit)
				{
					return alkajelisk_spore_match(bullet, unit);
				});
				if (seekTarget != nullptr)
				{
					bullet->attackTarget.unit = seekTarget;
					bullet->attackTarget.pt = seekTarget->position;
					bullet->move_target.unit = seekTarget;
					bullet->move_target.pt = seekTarget->position;
					bullet->targetPosition = seekTarget->position;
					bullet->targetPosition2 = seekTarget->position;
					helper::progress_bullet_movement(bullet);
				}
			}
			else
			{
				helper::progress_bullet_movement(bullet);
			}
		}
		else
		{
			helper::progress_bullet_movement(bullet);
		}

		u8 time = bullet->time_remaining--;
		if (bullet->weaponId == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->time_remaining < AlkajeliskParasiticSporesTimer && bullet->attackTarget.unit == nullptr)
			{
				time = 0;
			}
		}
		bool bEnd = false;

		// Accelerate missiles over time
		if (bullet->weaponId == WeaponId::EmpyreanPlasmaBarrage)
		{
			u8 time_current = 255 - time;
			if (time_current == 8)
			{
				bullet->flingy_top_speed *= 2;
				bullet->flingy_acceleration *= 2;
			}
		}

		if (bullet->weaponId == WeaponId::ArchitectDispersiveSalvo
			&& bullet->sourceUnit
			&& bullet->attackTarget.unit
			&& bullet->attackTarget.unit->sprite && bullet->sourceUnit->sprite)
		{
			Unit* targ = bullet->attackTarget.unit;

			UnitExt* unitExt = globals.get_unit_ext(bullet->sourceUnit);
			const u32 distFromSource = scbw::get_distance_fast(bullet->position, unitExt->last_bullet_launch_position);
			const u32 distFromTarget = scbw::get_distance_fast(bullet->position, targ->position);

			if (distFromSource > 5 * 32
				&& distFromTarget < 3 * 32)
			{
				if (!globals.multihit_bullet_is_set(bullet))
				{
					auto old = bullet->sourceUnit->order_target.unit;
					auto oldSpritePos = bullet->sourceUnit->sprite->position;
					auto oldPos = bullet->sourceUnit->position;
					bullet->sourceUnit->sprite->position = bullet->position;
					bullet->sourceUnit->position = bullet->position;
					auto r = 3 * 32;
					scbw::UnitFinder unitsInSplash(bullet->position.x - r, bullet->position.y - r,
						bullet->position.x + r, bullet->position.y + r);
					globals.add_multihit_empty_bullet(bullet, 1);
					for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
					{
						Unit* target = unitsInSplash.get_unit(i);
						if (target == nullptr)
						{
							continue;
						}
						if (target == targ)
						{
							continue;
						}
						if (!alkajelisk_spore_match(bullet, target))
						{
							continue;
						}
						bullet->sourceUnit->order_target.unit = target;
						auto newAngle = scbw::get_angle(bullet->position.x, bullet->position.y, target->get_x(), target->get_y());
						auto newBullet = helper::create_bullet(WeaponId::ArchitectDispersiveSalvo,
							bullet->sourceUnit, bullet->position.x, bullet->position.y, bullet->srcPlayer, newAngle);
						if (newBullet)
						{
							helper::play_sound_from_unit(SoundId::ArchitectWeaponSplit01, (Unit*)bullet, 1, 0);
							newBullet->position = bullet->position;
							globals.add_multihit_empty_bullet(newBullet, 1);
						}
					};
					bullet->sourceUnit->order_target.unit = old;
					bullet->sourceUnit->position = oldPos;
					bullet->sourceUnit->sprite->position = oldSpritePos;
				}
			}
		}

		if (time == 0 || (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y))
		{
			bEnd = true;
		}

		// Missile interception
		// Empyrean Heavenly Passage
		// Valkyrie Charon Afterburners
		else if (bullet->weaponId == WeaponId::EmpyreanPlasmaBarrage
			|| bullet->weaponId == WeaponId::ValkyrieHaloRockets)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::get_nearest_target(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (!(unit->status & UnitStatus::InAir))
				{
					return false;
				}
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (scbw::is_allied_to(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->get_left())
				{
					return false;
				}
				if (bullet->position.x > unit->get_right())
				{
					return false;
				}
				if (bullet->position.y < unit->get_top())
				{
					return false;
				}
				if (bullet->position.y > unit->get_bottom())
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}

		// Star Sovereign Intervention
		else if (bullet->weaponId == WeaponId::StarSovereignInterventionMissiles)
		{
			u32 range = 128;
			auto collisionTarget = scbw::UnitFinder::get_nearest_target(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (scbw::is_allied_to(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->get_left())
				{
					return false;
				}
				if (bullet->position.x > unit->get_right())
				{
					return false;
				}
				if (bullet->position.y < unit->get_top())
				{
					return false;
				}
				if (bullet->position.y > unit->get_bottom())
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				bullet->attackTarget.unit = collisionTarget;
				bEnd = true;
			}
		}

		// For missiles hitting targets mid-flight
		else if (scbw::weapon_pierces_targets(bullet->weaponId))
		{
			u32 range = 0;
			switch (bullet->weaponId)
			{
				case WeaponId::KalkaliskViscousAcid:
				{
					range = 96;
				} break;
				case WeaponId::PenumbraErebolShockCannon:
				{
					range = 128;
				} break;
				case WeaponId::ClaymoreHarpoonRailgun:
				{
					range = 160;
				} break;
				default:
				{
				}
			}
			auto collisionTarget = scbw::UnitFinder::get_nearest_target(
				bullet->position.x - range,
				bullet->position.y - range,
				bullet->position.x + range,
				bullet->position.y + range,
				bullet, [&](Unit* unit)
			{
				if (unit->status & UnitStatus::Invincible)
				{
					return false;
				}
				if (bullet->attackTarget.unit == unit)
				{
					return false;
				}
				if (scbw::is_allied_to(bullet->srcPlayer, unit->player))
				{
					return false;
				}
				auto box = units_dat::UnitBounds[unit->id];
				if (bullet->position.x < unit->get_left())
				{
					return false;
				}
				if (bullet->position.x > unit->get_right())
				{
					return false;
				}
				if (bullet->position.y < unit->get_top())
				{
					return false;
				}
				if (bullet->position.y > unit->get_bottom())
				{
					return false;
				}
				if (globals.was_already_hit_by_multihit_bullet(bullet, unit))
				{
					return false;
				}
				return true;
			}
			);
			if (collisionTarget != nullptr)
			{
				if (bullet->weaponId == WeaponId::PenumbraErebolShockCannon)
				{
					// Penumbra - Shroud Piercer - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::weapon_bullet_hit(bullet, collisionTarget, 2);
				}
				else if (bullet->weaponId == WeaponId::ClaymoreHarpoonRailgun)
				{
					// Claymore - Kelvin Munitions - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactBlue_Damageless, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					hooks::weapon_bullet_hit(bullet, collisionTarget, 1);

					// Claymore - Kelvin Munitions - slow pierced targets
					UnitExt* collisionTargetExt = globals.get_unit_ext(collisionTarget);
					if (!collisionTargetExt->values[ValueKelvinMunitionsSlow])
					{
						collisionTargetExt->values[ValueKelvinMunitionsSlow] = 1;
					}
				}
				else if (bullet->weaponId == WeaponId::KalkaliskViscousAcid) {
					// Kalkalisk behavior
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
					auto count = globals.get_multihit_count(bullet);
					helper::create_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->srcPlayer);
					collisionTarget->damage_with_custom((weapons_dat::DamageAmount[WeaponId::KalkaliskViscousAcid] - count + 1) * 256,
						weapons_dat::DamageType[WeaponId::KalkaliskViscousAcid],
						bullet->sourceUnit, bullet->srcPlayer, 0, 1, false);

					if (count >= 4)
					{
						bullet->time_remaining = 0;
					}
					else
					{
						bullet->move_target.unit = bullet->attackTarget.unit;
						if (bullet->move_target.unit != nullptr)
						{
							bullet->targetPosition = bullet->move_target.unit->position;
							bullet->targetPosition2 = bullet->move_target.unit->position;
						}
					}
				}
			}
		}

		if (bEnd)
		{
			target = bullet->attackTarget.unit;
			bullet->behaviourTypeInternal = 5;
			if (target != nullptr && target->sprite != nullptr)
			{
				bullet->attackTarget.pt.x = target->sprite->position.x;
				bullet->attackTarget.pt.y = target->sprite->position.y;
			}
			else
			{
				bullet->attackTarget.unit = 0;
			}
			bulletSprite = bullet->sprite;
			bullet->order_signal |= 0x80; // new
			bullet->unknown_0x4E = 0;
			bullet->someUnitType = 228;
			for (i = bulletSprite->images.head; i; i = i->link.next)
			{
				helper::play_image_iscript(i, IscriptAnimation::Death);
			}
		}
	}

	void bullet_behavior_bounce(Bullet* bullet) // 0048B2D0
	{
		Unit* v1;
		char nextBounce;
		Unit* target;
		Image* i;
		Unit* target_unit;
		Sprite* sprite;
		Image* j;

		v1 = bullet->attackTarget.unit;
		if (v1)
		{
			if (!(bullet->hit_flags & 1))
			{
				helper::change_move_pos(bullet, v1->sprite->position.y, v1->sprite->position.x);
			}
		}
		helper::progress_bullet_movement(bullet);
		if (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y)
		{
			nextBounce = bullet->remainingBounces - 1;
			// Gladius infinite bouncing
			if (bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
			{
				nextBounce = 1;
			}

			bullet->remainingBounces = nextBounce;
			globals.bounce_bullet = bullet;
			if (nextBounce && (target = helper::find_next_bounce_target(bullet), bullet->nextBounceUnit = bullet->attackTarget.unit, target))
			{
				for (i = bullet->sprite->images.head; i; i = i->link.next)
				{
					helper::play_image_iscript(i, IscriptAnimation::SpecialState1);
				}
				bullet->attackTarget.unit = target;
			}
			else
			{
				if (bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
				{
					globals.remove_multihit_bullet(bullet);
				}
				target_unit = bullet->attackTarget.unit;
				bullet->behaviourTypeInternal = 5;
				if (target_unit)
				{
					bullet->attackTarget.pt.x = target_unit->sprite->position.x;
					bullet->attackTarget.pt.y = target_unit->sprite->position.y;
				}
				else
				{
					bullet->attackTarget.unit = 0;
				}
				sprite = bullet->sprite;
				bullet->unknown_0x4E = 0;
				bullet->someUnitType = UnitId::None;

				for (j = sprite->images.head; j; j = j->link.next)
				{
					helper::play_image_iscript(j, IscriptAnimation::Death);
				}
			}
		}
	}

	bool find_next_bounce_target_proc(Unit* pIterator) // 0048AFD0
	{
		Unit* pSrc = *bulletBounceMissileSource;
		s32 nSrcStatus = (*bulletBounceMissileSource)->status;
		Unit* _pIterator = pIterator;

		// Previously disallowed bouncing if unit was disabled
		if (!pIterator)
		{
			return 0;
		}
		if (pIterator->status & UnitStatus::Invincible)
		{
			return 0;
		}
		if (pIterator->sprite->flags & SpriteFlags::Hidden)
		{
			return 0;
		}
		if (helper::unit_can_see_cloaked_target(pIterator, (*bulletBounceMissileSource)))
		{
			return 0;
		}

		Unit* bounceSource = pSrc;
		if (pSrc->subunit != nullptr && helper::unit_is_subunit(pSrc->subunit))
		{
			bounceSource = pSrc->subunit;
		}
		if (!(helper::unit_is_carrier(pSrc)))
		{
			bool miscBool;
			if (helper::unit_is_reaver(pSrc))
			{
				if (_pIterator->status & UnitStatus::InAir)
				{
					return 0;
				}
				miscBool = helper::unitHasPathToUnit(pSrc, _pIterator);
			}
			else
			{
				if (helper::unit_can_infest(pSrc))
				{
					Unit* unusedUnit = nullptr;
					miscBool = helper::can_be_infested(unusedUnit, _pIterator) != 0; // unusedUnit is not used..?
				}
				else
				{
					if (_pIterator->status & UnitStatus::InAir)
					{
						miscBool = units_dat::AirWeapon[(u16)bounceSource->id] != -126;
					}
					else
					{
						miscBool = units_dat::GroundWeapon[(u16)bounceSource->id] != -126;
					}
				}
			}
			if (!miscBool)
			{
				return 0;
			}
		}

		s32 unitStatus = _pIterator->status;
		if (unitStatus & 0x300 && !((1 << pSrc->player) & _pIterator->visibility_status))
		{
			return 0;
		}

		s8 player = _pIterator->player == 11 ? _pIterator->sprite->player : _pIterator->player;
		if (player_alliance[(u8)pSrc->player].flags[player] || *bulletBounceMissileTarget == _pIterator)
		{
			return 0;
		}

		// Gladius Binding Plasma behavior
		if (globals.bounce_bullet->weaponId == WeaponId::GladiusPlasmaFlurry)
		{
			if (pIterator != nullptr && *bulletBounceMissileNext != nullptr
				&& (*bulletBounceMissileNext)->sprite != nullptr
				&& _pIterator->sprite != nullptr)
			{
				if (globals.was_already_hit_by_multihit_bullet(globals.bounce_bullet, _pIterator))
				{
					return false;
				}
				else
				{
					globals.add_bullet_multihit(globals.bounce_bullet, _pIterator);
					return *bulletBounceMissileNext != _pIterator;

				}
			}
			else
			{
//				scbw::print("bad bullet");
			}
		}
		return *bulletBounceMissileNext != _pIterator;
	}

	Unit* find_next_bounce_target(Bullet* bullet) // 0048B1E0
	{
		if (!bullet->sourceUnit)
		{
			return 0;
		}
		Unit* nextBounce = bullet->nextBounceUnit;
		*bulletBounceMissileSource = bullet->sourceUnit;
		Unit* target = bullet->attackTarget.unit;
		Sprite* bulletSprite = bullet->sprite;
		*bulletBounceMissileTarget = target;
		u16 posY = bulletSprite->position.x;
		u16 posX = bulletSprite->position.y;
		*bulletBounceMissileNext = nextBounce;
		Box16 coords;
		coords.left = posX - 96;
		coords.top = posX + 96;
		coords.right = posY - 96;
		coords.bottom = posY + 96;
//		return (Unit*)helper::iterateUnitsAtLocationTargetProc(&coords, find_next_bounce_target_proc, 0);
	}

	// Hooked to resolve issues with multihit bullets - Pr0nogo
	Bullet* delete_bullet (Bullet* bullet) // 0048A560
	{
		if (!bullet->sprite)
		{
			if (*first_bullet == bullet)
			{
				*first_bullet = bullet->next;
			}
			if (*last_bullet == bullet)
			{
				*last_bullet = bullet->previous;
			}
			if (bullet->previous)
			{
				bullet->previous->next = bullet->next;
			}
			Bullet* v1 = bullet->next;
			if (v1)
			{
				v1->previous = bullet->previous;
			}
			bullet->previous = 0;
			bullet->next = 0;
			Bullet* nextBounce = *first_free_bullet;
			if (*first_free_bullet)
			{
				if (*last_free_bullet == *first_free_bullet)
				{
					*last_free_bullet = bullet;
				}
				bullet->previous = *first_free_bullet;
				bullet->next = nextBounce->next;
				Bullet* target = nextBounce->next;
				if (target)
				{
					target->previous = bullet;
				}
				nextBounce->next = bullet;
				*bullet_count -= 1;
			}
			else
			{
				*last_free_bullet = bullet;
				*first_free_bullet = bullet;
				*bullet_count -= 1;
			}
			globals.remove_multihit_bullet(bullet);
		}
		return bullet;
	}

	// Hooked to stop Valkyries from being cucked by lardcode - Pr0nogo
	Bullet* create_bullet(s32 weaponId, Unit* parent, s32 x, s16 y, char player, s32 angle) // 0048C260
	{
		Bullet* pBullet;
		s32 parentStatus;
		Bullet* result;
		Bullet* pNextBullet;
		Bullet* firstBulletVar;
		Bullet* nextBulletVar;

		pBullet = *first_free_bullet;
		if (!*first_free_bullet)
		{
			return 0;
		}
		parentStatus = parent->status;
		if (parentStatus & UnitStatus::CanNotAttack)
		{
			return 0;
		}
		if (!*first_free_bullet)
		{
			return 0;
		}
		if (!hooks::initialize_bullet(player, parent, y, *first_free_bullet, weaponId, x, angle))
		{
			return 0;
		}
		if (*first_free_bullet == pBullet)
		{
			*first_free_bullet = pBullet->next;
		}
		if (*last_free_bullet == pBullet)
		{
			*last_free_bullet = pBullet->previous;
		}
		if (pBullet->previous)
		{
			pBullet->previous->next = pBullet->next;
		}
		pNextBullet = pBullet->next;
		if (pNextBullet)
		{
			pNextBullet->previous = pBullet->previous;
		}
		pBullet->previous = 0;
		pBullet->next = 0;
		firstBulletVar = *first_bullet;
		if (*first_bullet)
		{
			if (*last_bullet == *first_bullet)
			{
				*last_bullet = pBullet;
			}
			pBullet->previous = *first_bullet;
			pBullet->next = firstBulletVar->next;
			nextBulletVar = firstBulletVar->next;
			if (nextBulletVar)
			{
				nextBulletVar->previous = pBullet;
			}
			firstBulletVar->next = pBullet;
			++*bullet_count;
			result = pBullet;
		}
		else
		{
			++*bullet_count;
			*last_bullet = pBullet;
			*first_bullet = pBullet;
			result = pBullet;
		}
		return result;
	}

	// Hooked so we don't need aise for Bactalisks - Pr0nogo
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weaponId, s32 x, s32 angle) // 0048BEC0
	{
		if (!helper::initialize_flingy(weapons_dat::FlingyId[weaponId], y, x, (Flingy*)bullet, player, angle)) // Unit/Bullet/Flingy have a shared header, so this is presumably fine
		{
			return 0;
		}

		bullet->srcPlayer = player;
		bullet->someUnitType = 228;
		bullet->unknown_0x4E = 0;
		bullet->attackTarget.unit = nullptr;
		bullet->attackTarget.pt.y = 0;
		bullet->attackTarget.pt.x = 0;
		bullet->unknown_0x54 = 0;
		bullet->weaponId = weaponId;
		bullet->time_remaining = weapons_dat::RemoveAfter[weaponId];
		bullet->someUnitType = 228;
		bullet->remainingBounces = 0;
		bullet->hit_flags = 0;
		bullet->movement_flags |= MovementFlags::StartingAttack;
		bullet->behaviourTypeInternal = 0;
		bullet->attackTarget.unit = nullptr;
		bullet->attackTarget.pt.y = 0;
		bullet->attackTarget.pt.x = 0;
		bullet->unknown_0x4E = 0;
		hooks::bullet_behavior_reassign(bullet);

		u8 launchSpin = weapons_dat::LaunchSpin[weaponId];
		if (launchSpin)
		{
			s32 launchSpinVariance = 0;
			if (parent == *lastBulletCreator)
			{
				launchSpinVariance = *launchSpinRandom == 0;
			}
			else
			{
				launchSpinVariance = helper::randomize_short(0) & 1;
			}

			*launchSpinRandom = launchSpinVariance;
			bool invertSpin = launchSpinVariance == 0;
			launchSpinVariance = launchSpin;
			if (invertSpin)
			{
				launchSpinVariance = -(s8)launchSpin;
			}
			char launchSpinDirection = launchSpinVariance + bullet->direction2;
			bullet->direction2 = launchSpinDirection;
			bullet->direction1 = launchSpinDirection;
			*lastBulletCreator = parent;
		}

		bullet->sourceUnit = parent;
		if (units_dat::BaseProperty[parent->id] & UnitProperty::Subunit)
		{
			bullet->sourceUnit = parent->subunit;
		}
		if (parent->id == UnitId::ProtossPlasmaShell)
		{
			bullet->sourceUnit = parent->hangar_unit.parent;
		}
		if (parent->status & UnitStatus::IsHallucination)
		{
			bullet->hit_flags |= 2u;
		}
		bullet->nextBounceUnit = nullptr;

		Unit* targetUnit = parent->order_target.unit;
		if (targetUnit)
		{
			bullet->sprite->elevation_level = targetUnit->sprite->elevation_level + 1;
			bullet->attackTarget.unit = targetUnit;
			bullet->attackTarget.pt.x = targetUnit->sprite->position.x;
			bullet->attackTarget.pt.y = targetUnit->sprite->position.y;
		}
		else
		{
			bullet->sprite->elevation_level = parent->sprite->elevation_level + 1;
			bullet->attackTarget.unit = 0;
			bullet->attackTarget.pt.x = parent->order_target.pt.x;
			bullet->attackTarget.pt.y = parent->order_target.pt.y;
		}

		switch (weapons_dat::Behavior[weaponId])
		{
			case WeaponBehavior::Fly_DoNotFollowTarget:
			case WeaponBehavior::Fly_FollowTarget:
			{
				if (targetUnit != nullptr)
				{
					if (parent != nullptr)
					{
						helper::weapon_behavior_can_hit_target_check(targetUnit, parent);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnTargetUnit:
			case WeaponBehavior::AppearOnTargetSite:
			{
				if (!targetUnit)
				{
					return 1;
				}
				if (!bullet->sourceUnit)
				{
					return 1;
				}
				helper::weapon_behavior_can_hit_target_check(targetUnit, bullet->sourceUnit);
				return 1;
			}
			case WeaponBehavior::PersistOnTargetSite:
			{
				helper::appear_on_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnAttacker:
			{
				return 1;
			}
			case WeaponBehavior::AttackAndSelfDestruct:
			{
				if (!bullet->sourceUnit)
				{
					return 1;
				}
				bullet->sourceUnit->status |= UnitStatus::IsSelfDestructing;
				bullet->sourceUnit->user_action_flags |= 0x4;
				helper::orders_self_destructing(bullet->sourceUnit);
				return 1;
			}
			case WeaponBehavior::Bounce:
			{
				switch (weaponId)
				{
					case WeaponId::BactaliskCorrosiveAcid:
					{
						const u32 dist = scbw::get_distance_fast(bullet->sourceUnit->position, bullet->attackTarget.pt);
						if (dist >= 6 * 32)
						{
							bullet->remainingBounces = 2;
						}
						else
						{
							bullet->remainingBounces = 1;
						}
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						bullet->remainingBounces = 2;
					} break;
					default:
					{
						bullet->remainingBounces = 3;
					}
				}
				if (targetUnit)
				{
					if (bullet->sourceUnit)
					{
						helper::weapon_behavior_can_hit_target_check(targetUnit, bullet->sourceUnit);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AttackNearbyArea:
			{
				bullet->cyclicMissileIndex = parent->offset_index_3_by_3;
				bullet->attackTarget.pt.x -= (s16)attackCurve[parent->offset_index_3_by_3].x;
				bullet->attackTarget.pt.y -= (s16)attackCurve[parent->offset_index_3_by_3].y;
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);

				s16 finalOffset = parent->offset_index_3_by_3 + 1;
				if (!attackCurve[finalOffset].x)
				{
					finalOffset = 0;
				}
				parent->offset_index_3_by_3 = finalOffset;
				return 1;
			}
			case WeaponBehavior::GoToMaxRange:
			{
				s32 bulletDirection = bullet->direction1;
				s32 maxRangeVar = (weapons_dat::MaxRange[bullet->weaponId] + 20) << 8;
				bullet->attackTarget.pt.x = parent->sprite->position.x + ((u32)(maxRangeVar * angleDistance[bulletDirection].x) >> 16);
				bullet->attackTarget.pt.y = parent->sprite->position.y + ((u32)(maxRangeVar * angleDistance[bulletDirection].y) >> 16);
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
		}
		return 0;
	}

	// Hooked to investigate weaponsdat extender crashes - Pr0nogo
	void progress_bullet_frames() // 0048C3D0
	{
		Bullet* bulletVar = *first_bullet;
		Sprite* bulletSprite = nullptr;
		s32 bulletBehavior = 0;
		Bullet* nextBullet = *first_bullet;
		while (nextBullet)
		{
			nextBullet = bulletVar->next;
			*activeIscriptEntity = (Unit*)bulletVar;
			*activeIscriptBullet = bulletVar;
			helper::set_thingy_visibility_flags((Thingy*)bulletVar);
			bulletSprite = bulletVar->sprite;
			if (bulletSprite)
			{
				helper::sprite_to_iscript_loop(bulletSprite);
				if (!bulletVar->sprite->images.head)
				{
					bulletVar->sprite = 0;
				}
			}
			bulletBehavior = (u8)bulletVar->behaviourTypeInternal;
			switch (bulletBehavior)
			{
				case 0:
				{
					hooks::bullet_behavior_reassign(bulletVar);
				} break;
				case 1:
				{
					helper::bullet_behavior_fly(bulletVar);
				} break;
				case 2:
				{
					helper::bullet_behavior_follow(bulletVar);
				} break;
				case 3:
				{
					helper::bullet_behavior_bounce(bulletVar);
				} break;
				case 4:
				{
					helper::bullet_behavior_persist(bulletVar);
				} break;
				case 5:
				{
					helper::bullet_behavior_instant(bulletVar);
				} break;
				case 6:
				{
					helper::bullet_behavior_area(bulletVar);
				} break;
				default:
				{
				} break;
			}
			bulletVar = nextBullet;
		}
		*activeIscriptEntity = 0;
		*activeIscriptBullet = 0;
	}

	// Hooked to investigate firstFreeBullet nonsense - Pr0nogo
	void bullet_behavior_reassign(Bullet* bullet) // 0048BD50
	{
		while (2) // xd
		{
			if (bullet->unknown_0x4E & 1)
			{
				bullet->unknown_0x4E = bullet->unknown_0x4E & 0xFE;
				Image* bulletOverlay = nullptr;
				switch (weapons_dat::Behavior[bullet->weaponId])
				{
					case WeaponBehavior::AppearOnTargetUnit:
					case WeaponBehavior::AppearOnTargetSite:
					case WeaponBehavior::AppearOnAttacker:
					case WeaponBehavior::AttackAndSelfDestruct:
					{
						bullet->behaviourTypeInternal = WeaponBehavior::AppearOnAttacker;
						if (bullet->attackTarget.unit)
						{
							bullet->attackTarget.pt.x = bullet->attackTarget.unit->sprite->position.x;
							bullet->attackTarget.pt.y = bullet->attackTarget.unit->sprite->position.y;
						}
						else
						{
							bullet->attackTarget.unit = nullptr;
						}
						bullet->unknown_0x4E = 0;
						bullet->someUnitType = 228;
						helper::play_sprite_iscript(bullet->sprite, 1, 1);
					} break;
					case WeaponBehavior::Fly_FollowTarget:
					{
						helper::assign_weapon_data(bullet, 2);
						helper::play_sprite_iscript(bullet->sprite, 2, 1);
					}  break;
					case WeaponBehavior::Bounce:
					{
						helper::assign_weapon_data(bullet, 3);
						helper::play_sprite_iscript(bullet->sprite, 2, 1);
					}  break;
					case WeaponBehavior::PersistOnTargetSite:
					{
						helper::assign_weapon_data(bullet, 4);
						Image* bulletOverlay = nullptr;
						bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::play_image_iscript(bulletOverlay, 14);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
					case WeaponBehavior::AttackNearbyArea:
					{
						helper::assign_weapon_data(bullet, 6);
						Image* bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::play_image_iscript(bulletOverlay, 2);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
					default:
					{
						helper::assign_weapon_data(bullet, 1);
						Image* bulletOverlay = bullet->sprite->images.head;
						if (bulletOverlay)
						{
							do
							{
								helper::play_image_iscript(bulletOverlay, 2);
								bulletOverlay = bulletOverlay->link.next;
							}
							while (bulletOverlay);
						}
					} break;
				}

				switch (bullet->behaviourTypeInternal)
				{
					case 0:
					{
						continue;
					}
					case 1:
					{
						helper::bullet_behavior_fly(bullet);
					} break;
					case 2:
					{
						helper::bullet_behavior_follow(bullet);
					} break;
					case 3:
					{
						helper::bullet_behavior_bounce(bullet);
					} break;
					case 4:
					{
						helper::bullet_behavior_persist(bullet);
					} break;
					case 5:
					{
						helper::bullet_behavior_instant(bullet);
					} break;
					case 6:
					{
						helper::bullet_behavior_area(bullet);
					} break;
					default:
					{
						return;
					}
				}
			}
			break;
		}
	}

	// Hooked to help with kiting AI. -Pr0nogo
	// Ended up useless sadly, as it calls way more often than I anticipated. Replaced with starting attack check from movement flags -Veeq7
	void get_weapon_iscript_init(Unit* unit, char iscript_anim) // 00476ED0
	{
		*lastBulletCreator = 0;
		if (!(unit->status & UnitStatus::CanNotAttack))
		{
			UnitExt* unit_ext = globals.get_unit_ext(unit);
			if (unit->order_target.unit && unit->order_target.unit->status & UnitStatus::InAir)
			{
				helper::use_weapon_iscript(unit, units_dat::AirWeapon[unit->id], 2, iscript_anim + 1);
			}
			else
			{
				if (unit->id != UnitId::ZergLakizilisk || unit->status & UnitStatus::Burrowed)
				{
					helper::use_weapon_iscript(unit, units_dat::GroundWeapon[unit->id], 1, iscript_anim);
				}
				else
				{
					helper::use_weapon_iscript(unit, WeaponId::None, 1, iscript_anim);
				}
			}
		}
	}
}