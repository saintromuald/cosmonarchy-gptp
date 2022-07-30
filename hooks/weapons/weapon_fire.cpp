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
	if (scbw::is_allied_to(bullet->player, unit->player))
	{
		return false;
	}
	return true;
}

Bullet* create_bullet_and_apply_cooldown(u8 weapon_id, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction) {
	Bullet* bullet = helper::create_bullet(weapon_id, source, x, y, attackingPlayer, direction);
	if (bullet && source) {
		u32 cooldown = helper::get_weapon_cooldown(source, weapon_id);
		source->ground_weapon_cooldown = cooldown;
		source->air_weapon_cooldown = cooldown;
	}
	return bullet;
}

namespace
{
	typedef void(__stdcall* GetWeaponHitPosFunc)(Unit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const getWeaponHitPos = (GetWeaponHitPosFunc)0x004762C0;
}

namespace hooks
{
	static void launch_missile(Unit* attacker, UnitExt* attackerExt, Unit* target, u8 weapon_id)
	{
		s32 posX;
		s32 posY;
		if (weapons_dat::Behavior[weapon_id] & WeaponBehavior::AppearOnTargetUnit)
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
		create_bullet_and_apply_cooldown(weapon_id, attacker, posX, posY, attacker->player, newAngle);
		if (attackerExt->values[ValueDeconstruction])
		{
			scbw::do_deconstruction_damage(attacker, weapon_id, false);
		}
	}

	bool fire_multiple(Unit* attacker, UnitExt* attackerExt, u32 weapon_id, u32 maxTargets, u32 flags, bool ignoreMain)
	{
		bool fired = false;
		std::unordered_set<Unit*> targets;
		if (!ignoreMain
			&& attacker->order_target.unit
			&& attacker->main_order_id != OrderId::Follow
			&& attacker->main_order_id != OrderId::Patrol)
		{
			targets.insert(attacker->order_target.unit);
			launch_missile(attacker, attackerExt, attacker->order_target.unit, weapon_id);
			fired = true;
		}
		Unit* old = attacker->order_target.unit;
		for (s32 i = 0; i < maxTargets || maxTargets <= 0; i++)
		{
			u32 range = hooks::get_max_weapon_range(attacker, weapon_id, 0);
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
			launch_missile(attacker, attackerExt, newTarget, weapon_id);
			fired = true;
			targets.insert(newTarget);
		}
		attacker->order_target.unit = old;
		return fired;
	}

	// Fires a weapon from the @p unit.
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fire_weapon().
	void fire_weapon(Unit* unit, u8 weapon_id) // 00479C90
	{
		if (unit == nullptr)
		{
			return;
		}
		UnitExt* unitExt = get_unit_ext(unit);
		unitExt->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::DamageFactor[weapon_id] + 1;
		unitExt->values[ValueAI_PreventTargeting] = 0;
		// Retrieve the spawning position for the bullet.
		s32 x, y;
		u8 behavior = weapons_dat::Behavior[weapon_id];
		if (weapon_id == WeaponId::SavantPowerSiphon)
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
			s32 forwardOffset = weapons_dat::ForwardOffset[weapon_id];

			x = unit->get_x() + scbw::get_polar_x(forwardOffset, unit->current_direction_1);
			y = unit->get_y() + scbw::get_polar_y(forwardOffset, unit->current_direction_1)
				- weapons_dat::VerticalOffset[weapon_id];
		}

		// Cloaked Units - Decloak
		// Cabalist uses silent strike, so the check has to be in weapon damage
		if (unit->id != UnitId::ProtossCabalist) {
			scbw::decloak_proc(unit);
		}

		if (weapons_dat::FlingyId[weapon_id] != 0)
		{
			if (unit->id == UnitId::TerranSavant) {
				if (!scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit)) {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantEntropyFire01, SoundId::SavantEntropyFire05), unit, 1, 0);
				}
				else {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantSiphonFire01, SoundId::SavantSiphonFire05), unit, 1, 0);
				}
			}

			// Lurker Seismic Spines behavior
			if (weapon_id == WeaponId::LakiziliskSubterraneanSpines)
			{
				unitExt->timers[TimerLurkerETA] = 17;
				auto range = weapons_dat::max_range[weapon_id] + 16;
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
				if (weapon_id == WeaponId::MadrigalFuriaMissiles
					&& scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1)
				{
					weapon_id = WeaponId::MadrigalImperiosoMissiles;
				}

				// Nemean Reactors - Heracles behavior - Refresh timer on attack and add stack
				if (weapon_id == WeaponId::HeraclesJavelinCannons)
				{
					unitExt->values[ValueNemeanReactorsStack]++;
					unitExt->timers[TimerNemeanReactors] = 49;
				//	helper::play_sound_from_unit(1372, unit, 1, 0);
				}

				// Madcap - Maverick Feeders passive
				if (unit->id == UnitId::TerranMadcap)
				{
					s32 stacks = unitExt->values[ValueMadcapStacks];
					unitExt->values[ValueMadcapStacks] = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
				}

				// Track launch position
				unitExt->last_bullet_launch_position = unit->position;

				switch (weapon_id)
				{
					// Missile spawn positions start
					case WeaponId::AxitriliskSearingVenom:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueAxitriliskMissilePos, 1, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueCharlatanMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ExemplarPlasmaTorpedo:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueExemplarMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::EmpyreanPlasmaBarrage:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueEmpyreanMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::StarSovereignCelestialCannon:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueSovereignCannonPos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::SentinelCerberusRailgun:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueSentinelMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueClaymoreMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::PenumbraErebolShockCannon:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValuePenumbraLaserPos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::WraithBurstLasers:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueWraithLaserPos, 3, ImagesDatEntries::OverlayAttackPtr, 0, 2);
					} break;
					case WeaponId::CentaurSunchaserMissiles:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueCentaurMissilePos, 4, ImagesDatEntries::OverlayAttackPtr);
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
						scbw::set_missile_source_pos(unit, weapon_id, ValuePhobosLaserPos, 4, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::SkithrokorSkitteringSpores:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueSkithrokorMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::LiiraliskHollowedSpores:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueLiiraliskMissilePos, 2, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::BactaliskCorrosiveAcid:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueBactaliskMissilePos, 2, ImagesDatEntries::OverlaySpecialPtr);
					} break;
					case WeaponId::KalkaliskViscousAcid:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueKalkaliskMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					case WeaponId::KeskathalorScreamingVenom:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueKeskathalorMissilePos, 2, ImagesDatEntries::OverlayAttackPtr, 0, 1);
					} break;
					case WeaponId::SporeColonySeekerSpores:
					{
						scbw::set_missile_source_pos(unit, weapon_id, ValueSporeColonyMissilePos, 1, ImagesDatEntries::OverlayAttackPtr);
					} break;
					// Missile spawn positions end
					case WeaponId::SalamanderPerditionBarrage:
					{
						// Salamander missile target positions
						auto rotate = unitExt->values[ValueSalamanderAtkPos];
						if (rotate == 1)
						{
							create_bullet_and_apply_cooldown(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
						}
						else
						{
							scbw::range_modified_fire(weapon_id, unit, x, y, 32 - 32 * rotate);
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
						fire_multiple(unit, unitExt, weapon_id, 5, Flags::Enemy | Flags::FrontalArc);
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
						fire_multiple(unit, unitExt, weapon_id, 1 + count, Flags::Enemy | Flags::FrontalArc);
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
						fire_multiple(unit, unitExt, weapon_id, 3, Flags::Enemy);
					} break;
					default:
					{
						create_bullet_and_apply_cooldown(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
					} break;
				}
			}
		}

		// Deconstruction - Pazuzu passive
		if (unitExt->values[ValueDeconstruction])
		{
			scbw::do_deconstruction_damage(unit, weapon_id);
		}
	}

	void progress_bullet_flight(Bullet* bullet) // 0048B3B0
	{
		Bullet* v1;
		Unit* target;
		Sprite* bulletSprite;
		Image* i;

		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores)
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
		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores)
		{
			if (bullet->time_remaining < AlkajeliskParasiticSporesTimer && bullet->attackTarget.unit == nullptr)
			{
				time = 0;
			}
		}
		bool bEnd = false;

		// Accelerate missiles over time
		if (bullet->weapon_id == WeaponId::EmpyreanPlasmaBarrage)
		{
			u8 time_current = 255 - time;
			if (time_current == 8)
			{
				bullet->flingy_top_speed *= 2;
				bullet->flingy_acceleration *= 2;
			}
		}

		if (bullet->weapon_id == WeaponId::ArchitectDispersiveSalvo
			&& bullet->source_unit
			&& bullet->attackTarget.unit
			&& bullet->attackTarget.unit->sprite && bullet->source_unit->sprite)
		{
			Unit* targ = bullet->attackTarget.unit;

			UnitExt* unitExt = get_unit_ext(bullet->source_unit);
			const u32 distFromSource = scbw::get_distance_fast(bullet->position, unitExt->last_bullet_launch_position);
			const u32 distFromTarget = scbw::get_distance_fast(bullet->position, targ->position);

			if (distFromSource > 5 * 32
				&& distFromTarget < 3 * 32)
			{
				if (!globals.multihit_bullet_is_set(bullet))
				{
					auto old = bullet->source_unit->order_target.unit;
					auto oldSpritePos = bullet->source_unit->sprite->position;
					auto oldPos = bullet->source_unit->position;
					bullet->source_unit->sprite->position = bullet->position;
					bullet->source_unit->position = bullet->position;
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
						bullet->source_unit->order_target.unit = target;
						auto newAngle = scbw::get_angle(bullet->position.x, bullet->position.y, target->get_x(), target->get_y());
						auto newBullet = create_bullet_and_apply_cooldown(WeaponId::ArchitectDispersiveSalvo,
							bullet->source_unit, bullet->position.x, bullet->position.y, bullet->player, newAngle);
						if (newBullet)
						{
							helper::play_sound_from_unit(SoundId::ArchitectWeaponSplit01, (Unit*)bullet, 1, 0);
							newBullet->position = bullet->position;
							globals.add_multihit_empty_bullet(newBullet, 1);
						}
					};
					bullet->source_unit->order_target.unit = old;
					bullet->source_unit->position = oldPos;
					bullet->source_unit->sprite->position = oldSpritePos;
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
		else if (bullet->weapon_id == WeaponId::EmpyreanPlasmaBarrage
		||		 bullet->weapon_id == WeaponId::ValkyrieHaloRockets)
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
				if (scbw::is_allied_to(bullet->player, unit->player))
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
		else if (bullet->weapon_id == WeaponId::StarSovereignInterventionMissiles)
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
				if (scbw::is_allied_to(bullet->player, unit->player))
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
		else if (scbw::weapon_pierces_targets(bullet->weapon_id))
		{
			u32 range = 0;
			switch (bullet->weapon_id)
			{
				case WeaponId::SentinelCerberusRailgun:
				{
					range = 64;
				} break;
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
				if (scbw::is_allied_to(bullet->player, unit->player))
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
				if (bullet->weapon_id == WeaponId::SentinelCerberusRailgun)
				{
					// Sentinel - Kinetic Penetration - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collisionTarget, 1);
				}
				else if (bullet->weapon_id == WeaponId::PenumbraErebolShockCannon)
				{
					// Penumbra - Shroud Piercer - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, bullet->position.x, bullet->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collisionTarget, 2);
				}
				else if (bullet->weapon_id == WeaponId::ClaymoreHarpoonRailgun)
				{
					// Claymore - Kelvin Munitions - pierce targets
					globals.add_bullet_multihit(bullet, collisionTarget);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactBlue_Damageless, bullet->position.x, bullet->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collisionTarget, 1);

					// Claymore - Kelvin Munitions - slow pierced targets
					UnitExt* collisionTargetExt = get_unit_ext(collisionTarget);
					if (!collisionTargetExt->values[ValueKelvinMunitionsSlow])
					{
						collisionTargetExt->values[ValueKelvinMunitionsSlow] = 1;
					}
				}
				else if (bullet->weapon_id == WeaponId::KalkaliskViscousAcid) {
					// Kalkalisk behavior
					globals.add_incremental_bullet_multihit(bullet, collisionTarget);
					auto count = globals.get_multihit_count(bullet);
					helper::create_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, bullet->position.x, bullet->position.y, bullet->player);
					collisionTarget->damage_with_custom((weapons_dat::damage_amount[WeaponId::KalkaliskViscousAcid] - count + 1) * 256,
						weapons_dat::damage_type[WeaponId::KalkaliskViscousAcid],
						bullet->source_unit, bullet->player, 0, 1, false);

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
			if (bullet->weapon_id == WeaponId::GladiusPlasmaFlurry)
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
				if (bullet->weapon_id == WeaponId::GladiusPlasmaFlurry)
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
		Unit* pSrc = *g_bullet_bounce_missile_source;
		s32 nSrcStatus = (*g_bullet_bounce_missile_source)->status;
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
		if (helper::unit_can_see_cloaked_target(pIterator, (*g_bullet_bounce_missile_source)))
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
				miscBool = helper::unit_has_path_to_unit(pSrc, _pIterator);
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
		if (unitStatus & 0x300 && !((1 << pSrc->player) & _pIterator->detection_status))
		{
			return 0;
		}

		s8 player = _pIterator->player == 11 ? _pIterator->sprite->player : _pIterator->player;
		if (player_alliance[(u8)pSrc->player].flags[player] || *g_bullet_bounce_missile_target == _pIterator)
		{
			return 0;
		}

		// Gladius Binding Plasma behavior
		if (globals.bounce_bullet->weapon_id == WeaponId::GladiusPlasmaFlurry)
		{
			if (pIterator != nullptr && *g_bullet_bounce_missile_next != nullptr
				&& (*g_bullet_bounce_missile_next)->sprite != nullptr
				&& _pIterator->sprite != nullptr)
			{
				if (globals.was_already_hit_by_multihit_bullet(globals.bounce_bullet, _pIterator))
				{
					return false;
				}
				else
				{
					globals.add_bullet_multihit(globals.bounce_bullet, _pIterator);
					return *g_bullet_bounce_missile_next != _pIterator;

				}
			}
			else
			{
//				scbw::print("bad bullet");
			}
		}
		return *g_bullet_bounce_missile_next != _pIterator;
	}

	Unit* find_next_bounce_target(Bullet* bullet) // 0048B1E0
	{
		if (!bullet->source_unit)
		{
			return 0;
		}
		Unit* nextBounce = bullet->nextBounceUnit;
		*g_bullet_bounce_missile_source = bullet->source_unit;
		Unit* target = bullet->attackTarget.unit;
		Sprite* bulletSprite = bullet->sprite;
		*g_bullet_bounce_missile_target = target;
		u16 posY = bulletSprite->position.x;
		u16 posX = bulletSprite->position.y;
		*g_bullet_bounce_missile_next = nextBounce;
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
	Bullet* create_bullet(s32 weapon_id, Unit* parent, s32 x, s16 y, char player, s32 angle) // 0048C260
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
		if (!hooks::initialize_bullet(player, parent, y, *first_free_bullet, weapon_id, x, angle))
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
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle) // 0048BEC0
	{
		if (!helper::initialize_flingy(weapons_dat::FlingyId[weapon_id], y, x, (Flingy*)bullet, player, angle)) // Unit/Bullet/Flingy have a shared header, so this is presumably fine
		{
			return 0;
		}

		bullet->player = player;
		bullet->someUnitType = 228;
		bullet->unknown_0x4E = 0;
		bullet->attackTarget.unit = nullptr;
		bullet->attackTarget.pt.y = 0;
		bullet->attackTarget.pt.x = 0;
		bullet->unknown_0x54 = 0;
		bullet->weapon_id = weapon_id;
		bullet->time_remaining = weapons_dat::RemoveAfter[weapon_id];
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

		u8 launchSpin = weapons_dat::LaunchSpin[weapon_id];
		if (launchSpin)
		{
			s32 launchSpinVariance = 0;
			if (parent == *g_last_bullet_creator)
			{
				launchSpinVariance = *g_launch_spin_random == 0;
			}
			else
			{
				launchSpinVariance = helper::randomize_short(0) & 1;
			}

			*g_launch_spin_random = launchSpinVariance;
			bool invertSpin = launchSpinVariance == 0;
			launchSpinVariance = launchSpin;
			if (invertSpin)
			{
				launchSpinVariance = -(s8)launchSpin;
			}
			char launchSpinDirection = launchSpinVariance + bullet->direction2;
			bullet->direction2 = launchSpinDirection;
			bullet->direction1 = launchSpinDirection;
			*g_last_bullet_creator = parent;
		}

		bullet->source_unit = parent;
		if (units_dat::base_property[parent->id] & UnitProperty::Subunit)
		{
			bullet->source_unit = parent->subunit;
		}
		if (parent->id == UnitId::ProtossPlasmaShell)
		{
			bullet->source_unit = parent->hangar_unit.parent;
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

		switch (weapons_dat::Behavior[weapon_id])
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
				if (!bullet->source_unit)
				{
					return 1;
				}
				helper::weapon_behavior_can_hit_target_check(targetUnit, bullet->source_unit);
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
				if (!bullet->source_unit)
				{
					return 1;
				}
				bullet->source_unit->status |= UnitStatus::IsSelfDestructing;
				bullet->source_unit->user_action_flags |= 0x4;
				helper::orders_self_destructing(bullet->source_unit);
				return 1;
			}
			case WeaponBehavior::Bounce:
			{
				switch (weapon_id)
				{
					case WeaponId::BactaliskCorrosiveAcid:
					{
						const u32 dist = scbw::get_distance_fast(bullet->source_unit->position, bullet->attackTarget.pt);
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
					if (bullet->source_unit)
					{
						helper::weapon_behavior_can_hit_target_check(targetUnit, bullet->source_unit);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);
				return 1;
			}
			case WeaponBehavior::AttackNearbyArea:
			{
				bullet->cyclicMissileIndex = parent->offset_index_3_by_3;
				bullet->attackTarget.pt.x -= (s16)g_attack_curve[parent->offset_index_3_by_3].x;
				bullet->attackTarget.pt.y -= (s16)g_attack_curve[parent->offset_index_3_by_3].y;
				helper::attack_fly_to_target(bullet, bullet->attackTarget.pt.x, bullet->attackTarget.pt.y);

				s16 finalOffset = parent->offset_index_3_by_3 + 1;
				if (!g_attack_curve[finalOffset].x)
				{
					finalOffset = 0;
				}
				parent->offset_index_3_by_3 = finalOffset;
				return 1;
			}
			case WeaponBehavior::GoToMaxRange:
			{
				s32 bulletDirection = bullet->direction1;
				s32 maxRangeVar = (weapons_dat::max_range[bullet->weapon_id] + 20) << 8;
				bullet->attackTarget.pt.x = parent->sprite->position.x + ((u32)(maxRangeVar * g_angle_distance[bulletDirection].x) >> 16);
				bullet->attackTarget.pt.y = parent->sprite->position.y + ((u32)(maxRangeVar * g_angle_distance[bulletDirection].y) >> 16);
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
			*g_active_iscript_entity = (Unit*)bulletVar;
			*g_active_iscript_bullet = bulletVar;
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
		*g_active_iscript_entity = 0;
		*g_active_iscript_bullet = 0;
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
				switch (weapons_dat::Behavior[bullet->weapon_id])
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
		*g_last_bullet_creator = 0;
		if (!(unit->status & UnitStatus::CanNotAttack))
		{
			UnitExt* unit_ext = get_unit_ext(unit);
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