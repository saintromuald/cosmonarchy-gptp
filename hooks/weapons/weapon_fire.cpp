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

Point8 get_next_bullet_offset(Unit* unit, WeaponOffsets* weapon_offsets) {
	UnitExt* unit_ext = get_unit_ext(unit);
	u32 rotate = unit_ext->values[weapon_offsets->value_id];
	Image* image = unit->sprite->main_graphic;
	switch (unit->id) {
		case UnitId::TerranWatchdog:
		{
			image = unit->get_overlay(ImageId::MissileTurret_Turret);
		} break;
		case UnitId::TerranSentinel:
		{
			image = unit->get_overlay(ImageId::SentinelTurret);
		} break;
		case UnitId::TerranIonCannon:
		{
			image = unit->get_overlay(ImageId::IonCannonTurret);
		} break;
	}

	LO_Header* lo_file = lo_files->attack_overlays[image->id];
	if (weapon_offsets->use_special_overlay) {
		lo_file = lo_files->special_overlays[image->id];
	}

	u32 frameset_length = 17;
	if (image->flags & ImageFlags::Omnidirectional) {
		frameset_length = 32;
	}

	Point8 offset = lo_file->get_offset(image->direction + (frameset_length * weapon_offsets->frameset_offset), rotate);
	if (image->flags & ImageFlags::Mirrored) {
		offset.x = -offset.x;
	}
	rotate++;
	if (rotate >= weapon_offsets->count) {
		rotate = 0;
	}
	unit_ext->values[weapon_offsets->value_id] = rotate;
	return offset;
}

Bullet* fire_bullet(Unit* unit, u32 weapon_id, Unit* target_unit, WeaponOffsets* weapon_offsets, bool apply_cooldown) {
	if (!unit->sprite) {
		return 0;
	}

	bool is_ion_cannon = unit->id == UnitId::TerranIonCannon;
	Target target = unit->order_target;
	if (is_ion_cannon) {
		UnitExt* unit_ext = get_unit_ext(unit);
		target.unit = 0;
		target.pt.x = unit_ext->values[ValueIonCannonTargetPosX];
		target.pt.y = unit_ext->values[ValueIonCannonTargetPosY];
	} else if (target_unit) {
		target.unit = target_unit;
		target.pt = target_unit->position;
	}
	if (!target.unit && !is_ion_cannon) {
		return 0;
	}

	Point16 launch_pos = unit->position;
	if (weapons_dat::behavior[weapon_id] == WeaponBehavior::AppearOnTargetUnit) {
		launch_pos = target_unit->position;
	}
	if (weapon_offsets) {
		Point8 offset = get_next_bullet_offset(unit, weapon_offsets);
		launch_pos.x += offset.x;
		launch_pos.y += offset.y;
	}
	if (weapons_dat::behavior[weapon_id] == WeaponBehavior::GoToMaxRange) {
		Vector32f diff = scbw::get_normalized_vector(scbw::get_vector_between_points(launch_pos, target.pt));
		u32 max_range = hooks::get_max_weapon_range(unit, weapon_id, target.unit);
		target.pt = launch_pos;
		target.pt.x += diff.x * max_range;
		target.pt.y += diff.y * max_range;
	}
	s32 angle = scbw::get_angle(launch_pos.x, launch_pos.y, target.pt.x, target.pt.y);

	Target original_target = unit->order_target;
	unit->order_target = target;
	Bullet* bullet = create_bullet_ex(weapon_id, unit, launch_pos.x, launch_pos.y, unit->player, angle, apply_cooldown);
	unit->order_target = original_target;
	if (bullet) {
		if (weapons_dat::behavior[weapon_id] == WeaponBehavior::GoToMaxRange) {
			scbw::set_bullet_target_absolute(bullet, target);
		} else {
			scbw::set_bullet_target_follow(bullet, target);
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		if (is_ion_cannon) {
			unit_ext->values[ValueIonCannonTargeting] = false;
			unit->order_target.unit = unit_ext->units[UnitIonCannonTarget]; // Restore original target
		}
		if (weapon_offsets && unit_ext->values[weapon_offsets->value_id] < weapon_offsets->fake_count) {
			bullet->hit_flags |= 0x1;
		}
	}
	return bullet;
}

void apply_weapon_cooldown(Unit* unit, u8 weapon_id) {
	u32 cooldown = helper::get_weapon_cooldown(unit, weapon_id);
	unit->ground_weapon_cooldown = cooldown;
	unit->air_weapon_cooldown = cooldown;
}

Bullet* create_bullet_ex(u8 weapon_id, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction, bool apply_cooldown) {
	Bullet* bullet = helper::create_bullet(weapon_id, source, x, y, attackingPlayer, direction);
	if (bullet) {
		scbw::decloak_proc(source);
		if (apply_cooldown) {
			apply_weapon_cooldown(source, weapon_id);
		}

		UnitExt* source_ext = get_unit_ext(source);
		if (source_ext->values[ValueDeconstruction]) {
			scbw::do_deconstruction_damage(source, weapon_id, false);
		}
	}
	return bullet;
}

bool fire_multiple(Unit* unit, u32 weapon_id, u32 max_targets, u32 filters, bool ignore_main, bool use_weapon_factor, bool apply_cooldown, WeaponOffsets* weapon_offsets, bool center_on_target, u32 range_overwrite, u32 divisor_overwrite) {
	u32 attack_range = range_overwrite ? range_overwrite : hooks::get_max_weapon_range(unit, weapon_id, 0);
	u32 factor = use_weapon_factor ? weapons_dat::damage_factor[weapon_id] : 1; // @TODO: Find some more sane way of doing this, which may be challenging in BW... -Pr0nogo

	bool fired = false;
	Unit* order_target = unit->order_target.unit;
	if (!ignore_main
		&& order_target
		&& unit->main_order_id != OrderId::Follow
		&& unit->main_order_id != OrderId::Patrol
		&& unit->is_target_within_min_range(order_target, attack_range)
		&& unit->can_attack_target(order_target)) {
		if (unit_flag_match(order_target, unit, filters & ~(FilterAlly | FilterEnemy))) { // We want to attack the order target regardless of alliance flags
			for (u32 i = 0; i < factor; i++) {
				Bullet* bullet = fire_bullet(unit, weapon_id, order_target, weapon_offsets, apply_cooldown);
				if (bullet) {
					fired = true;
					BulletExt* bullet_ext = get_bullet_ext(bullet);
					bullet_ext->divisor_overwrite = divisor_overwrite;
				}
			}
		}
	}

	EntryArray targets = {};
	if (center_on_target) {
		if (!order_target) {
			return fired;
		}
		unit_search_radial(&targets, order_target->position, attack_range);
	} else {
		Box32 area = {
			unit->get_left()   - attack_range - 8 * 32, // We can't account for size of all possible other units, but we want border to border search,
			unit->get_top()    - attack_range - 8 * 32, // so we will just use an additional constant offset here.
			unit->get_right()  + attack_range + 8 * 32, // This is not really meant to be a rect area, more like rect extended circle, just like normal weapons,
			unit->get_bottom() + attack_range + 8 * 32,	// for both of those reasons we later check the min range between units -Veeq7
		};
		unit_search_rect(&targets, area);
	}
	unit_search_filter(&targets, unit, filters);
	Point16 sort_center   = order_target ? order_target->position : unit->position;
	unit_search_sort_by_distance(&targets, sort_center);

	u32 count = targets.count;
	if (max_targets && count > max_targets) {
		count = max_targets;
	}

	for (u32 i = 0; i < count; ++i) {
		Unit* target = targets.units[i];
		if (target == unit
		||  target == order_target
		||  scbw::is_allied_to(unit->player, target->player)
		|| !unit->can_attack_target(target)
		|| !unit->is_target_within_min_range(target, attack_range)) {
			continue;
		}

		for (u32 i = 0; i < factor; i++) {
			Bullet* bullet = fire_bullet(unit, weapon_id, target, weapon_offsets, apply_cooldown);
			if (bullet) {
				fired = true;
				BulletExt* bullet_ext = get_bullet_ext(bullet);
				bullet_ext->divisor_overwrite = divisor_overwrite;
			}
		}
	}

	return fired;
}

namespace
{
	typedef void(__stdcall* GetWeaponHitPosFunc)(Unit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const get_weapon_hit_pos = (GetWeaponHitPosFunc)0x004762C0;
}

namespace hooks {
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fire_weapon().
	void fire_weapon(Unit* unit, u8 weapon_id) // 00479C90
	{
		if (!unit)
		{
			return;
		}
		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::damage_factor[weapon_id] + 1;
		unit_ext->values[ValueAI_PreventTargeting] = 0;
		// Retrieve the spawning position for the bullet.
		s32 x, y;
		u8 behavior = weapons_dat::behavior[weapon_id];
		if (weapon_id == WeaponId::SavantPowerSiphon)
		{
			behavior = WeaponBehavior::AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior::AppearOnTargetUnit)
		{
			if (!unit->order_target.unit)
			{
				return;
			}
			get_weapon_hit_pos(unit, &x, &y);
		}
		else if (behavior == WeaponBehavior::AppearOnTargetSite)
		{
			x = unit->order_target.pt.x;
			y = unit->order_target.pt.y;
		}
		else
		{
			s32 forward_offset = weapons_dat::forward_offset[weapon_id];

			x = unit->get_x() + scbw::get_polar_x(forward_offset, unit->current_direction_1);
			y = unit->get_y() + scbw::get_polar_y(forward_offset, unit->current_direction_1)
				- weapons_dat::vertical_offset[weapon_id];
		}

		// Cloaked Units - Decloak
		// Cabalist has Silent Strikes, so the check has to be in weapon damage
		if (unit->id != UnitId::ProtossCabalist) {
			scbw::decloak_proc(unit);
		}
		if (unit->id == UnitId::ProtossSteward && unit->hangar_unit.parent) {
			scbw::decloak_proc(unit->hangar_unit.parent);
		}

		if (weapons_dat::flingy_id[weapon_id] != 0)
		{
			if (unit->id == UnitId::TerranSavant) {
				if (!scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit)) {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantEntropyFire01, SoundId::SavantEntropyFire05), unit, 1, 0);
				}
				else {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantSiphonFire01, SoundId::SavantSiphonFire05), unit, 1, 0);
				}
			}

			// Lakizilisk Seismic Spines behavior
			if (weapon_id == WeaponId::LakiziliskSubterraneanSpines)
			{
				unit_ext->timers[TimerLakiziliskETA] = 17;
				auto range = weapons_dat::max_range[weapon_id] + 16;
				s32 angle = scbw::get_angle(unit->get_x(), unit->get_y(), unit->order_target.unit->get_x(), unit->order_target.unit->get_y());
				s32 targ_x = unit->get_x() + scbw::get_polar_x(range, angle);
				s32 targ_y = unit->get_y() + scbw::get_polar_y(range, angle);
				unit_ext->values[ValueSpineX] = targ_x;
				unit_ext->values[ValueSpineY] = targ_y;
				if (unit->order_target.unit)
				{
					unit_ext->values[ValueLurkerPickTarget] = (u32)unit->order_target.unit;
				}
				iquare_globals.clear_lakizilisk(unit);
			}

			if (unit->id != UnitId::TerranWyvern // disable default weapon for custom behaviors
			&&  unit->id != UnitId::ProtossAnthelion)
			{
				// Madrigal Tempo Change weapon switch
				if (weapon_id == WeaponId::MadrigalFuriaMissiles && unit_ext->values[ValueMadrigalTempoDown] == 1)
				{
					weapon_id = WeaponId::MadrigalImperiosoMissiles;
				}

				// Madcap - Maverick Feeders passive
				if (unit->id == UnitId::TerranMadcap)
				{
					s32 stacks = unit_ext->values[ValueMadcapStacks];
					unit_ext->values[ValueMadcapStacks] = MathUtils::clamp(stacks + MadcapPassiveDecayTime, 0, MadcapMaxStacks * MadcapPassiveDecayTime);
				}

				// Track launch position
				unit_ext->last_bullet_launch_position = unit->position;

				switch (weapon_id)
				{
					// Missile spawn positions start
					case WeaponId::CyprianTwinRailguns:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueCyprianLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::GoliathContenderMissiles:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueGoliathLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::RamessesPharoahCannons:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueRamessesLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::PaladinCrusaderMissiles:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValuePaladinLaunchPos;
						offsets.count = 5;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueClaymoreLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::PenumbraErebolShockCannon:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValuePenumbraLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::WraithBurstLasers:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueWraithLaunchPos;
						offsets.count = 2;
						offsets.fake_count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::SundogBlindsideRockets:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueSundogLaunchPos;
						offsets.count = 4;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::CentaurSunchaserMissiles:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueCentaurLaunchPos;
						offsets.count = 4;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::PhobosArgosLaserBatteries:
					{
						// Phobos - Argent Capacitors passive
						if (unit->id == UnitId::TerranPhobos && unit_ext->values[ValuePhobosDischarge] == 0)
						{
							unit->add_unit_energy(5 * 256);
							if (unit->energy == unit->get_max_energy())
							{
								unit_ext->values[ValuePhobosDischarge] = 1;
								unit->ground_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								unit->air_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								helper::play_sound_from_unit(SoundId::PhobosArgentCapacitorsProc, unit, 1, 0);
								unit->sprite->create_overlay(ImageId::OpticalFlareHit_Large);
							}
						}
						WeaponOffsets offsets = {};
						offsets.value_id = ValuePhobosLaunchPos;
						offsets.count = 4;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::SentinelCerberusRailgun:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueSentinelLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::IonCannonIonCannon:
					case WeaponId::IonCannonHammerfallBeacon:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueIonCannonLaunchPos;
						offsets.count = 1;
						offsets.use_special_overlay = true;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::WardenPrismaticCannon:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueWardenLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueCharlatanLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::OptectonSunsplitBeam:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueOptectonLaunchPos1;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::PositronAntimatterMissile:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValuePositronLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::AccantorPlasmaShell:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueAccantorLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::PanoptusPlasmaMissiles:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValuePanoptusLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::ExemplarPlasmaTorpedo:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueExemplarLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::EpigraphLitanyCannons:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueEpigraphLaunchPos;
						offsets.count = 4;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::EmpyreanPlasmaBarrage:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueEmpyreanLaunchPos;
						offsets.count = 4;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::StarSovereignCelestialCannon:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueSovereignLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::SpraithColonySeekerSpores:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueSpraithColonyLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::LiiraliskHollowedSpores:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueLiiraliskLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::KalkaliskViscousAcid:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueKalkaliskLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::AxitriliskSearingVenom:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueAxitriliskLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::SkithrokorSkitteringSpores:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueSkithrokorLaunchPos;
						offsets.count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::BactaliskCorrosiveAcid:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueBactaliskLaunchPos;
						offsets.count = 2;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					case WeaponId::KeskathalorScreamingVenom:
					{
						WeaponOffsets offsets = {};
						offsets.value_id = ValueKeskathalorLaunchPos;
						offsets.count = 2;
						offsets.fake_count = 1;
						fire_bullet(unit, weapon_id, 0, &offsets);
					} break;
					// Missile spawn positions end

					case WeaponId::CataphractLonginiusCannons:
					{
						// Multi-Attack IV
						WeaponOffsets offsets = {};
						offsets.value_id = ValueCataphractLaunchPos;
						offsets.count = 4;
						fire_multiple(unit, weapon_id, 4, FilterEnemy | FilterFrontalArc, false, true, true, &offsets);
					} break;
					case WeaponId::AnticthonEncirclingMirror:
					{
						// Multi-Attack V
						fire_multiple(unit, weapon_id, 5, FilterEnemy);
					} break;
					case WeaponId::AionLazarusSalvo:
					{
						auto rotate = unit_ext->values[ValueAionLaunchPos];
						if (rotate == 1)
						{
							create_bullet_ex(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
						}
						else
						{
							scbw::range_modified_fire(weapon_id, unit, x, y, 32 + 32 * rotate);
						}
						rotate++;
						if (rotate > 3)
						{
							rotate = 0;
						}
						unit_ext->values[ValueAionLaunchPos] = rotate;
					} break;
					case WeaponId::SalamanderPerditionBarrage:
					{
						// Salamander missile target positions
						auto rotate = unit_ext->values[ValueSalamanderLaunchPos];
						if (rotate == 1)
						{
							create_bullet_ex(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
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
						unit_ext->values[ValueSalamanderLaunchPos] = rotate;
					} break;
					case WeaponId::MinotaurAsterionLaserBattery:
					{
						// Monoceros Targeting - Minotaur
						fire_multiple(unit, weapon_id, 5, FilterEnemy | FilterFrontalArc);
					} break;
					case WeaponId::SirenCharybdalRifle:
					{
						// Tertiary Eyes - Siren
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
						fire_multiple(unit, weapon_id, 1 + count, FilterEnemy | FilterFrontalArc);
					} break;
					case WeaponId::MadrigalFuriaMissiles:
					{
						// Madrigal Furia Salvo
						fire_multiple(unit, WeaponId::MadrigalFuriaMissiles, 0, FilterEnemy);
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						// Crushing Tide - Izirokor
						fire_multiple(unit, weapon_id, 3, FilterEnemy);
					} break;
					case WeaponId::MatrixKhaydarinRay:
					{
						// Multi-Attack IV
						WeaponOffsets offsets = {};
						offsets.value_id = ValueWardenLaunchPos;
						offsets.count = 4;
						fire_multiple(unit, weapon_id, 4, FilterEnemy, false, false, true, &offsets);
					} break;
					case WeaponId::BarghestRayOfAgony:
					{
						// Multi-Attack III
						fire_multiple(unit, weapon_id, 3, FilterEnemy | FilterFrontalArc, false, false);
					} break;
					case WeaponId::SkortrithColonySkortraTendril:
					{
						fire_bullet(unit, weapon_id, unit->order_target.unit);
						fire_multiple(unit, weapon_id, 2, FilterEnemy, true, false, false, 0, true, 2 * 32, 2);
					} break;
					default:
					{
						create_bullet_ex(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
					} break;
				}
			}
		}
	}

	static void apply_bullet_piercing(Bullet* bullet, Point16 pos, u32 range) {
		EntryArray collisions = {};
		unit_search_radial(&collisions, pos, range);
		unit_search_filter(&collisions, 0);
		for (u32 i = 0; i < collisions.count; ++i) {
			if (bullet->time_remaining == 0) {
				break;
			}
			Unit* collision_target = collisions.units[i];
			if (collision_target->status & UnitStatus::Invincible) {
				continue;
			}
			if (bullet->attack_target.unit != collision_target && scbw::is_allied_to(bullet->player, collision_target->player)) { // TODO: Consider what to do with unsafe projectiles
				continue;
			}
			if (iquare_globals.was_already_hit_by_multihit_bullet(bullet, collision_target)) {
				continue;
			}

			switch (bullet->weapon_id) {
				case WeaponId::CyprianTwinRailguns: // Same as default, but orange effect
				{
					iquare_globals.add_bullet_multihit(bullet, collision_target);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, collision_target->position.x, collision_target->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collision_target, 1);
				} break;
				case WeaponId::PenumbraErebolShockCannon:
				{
					// Penumbra - Shroud Piercer
					iquare_globals.add_bullet_multihit(bullet, collision_target);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, collision_target->position.x, collision_target->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collision_target, 2);
				} break;
				case WeaponId::ClaymoreHarpoonRailgun:
				{
					// Claymore - Kelvin Munitions
					// pierce targets
					iquare_globals.add_bullet_multihit(bullet, collision_target);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, collision_target->position.x, collision_target->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collision_target, 1);

					// slow pierced targets
					UnitExt* collision_target_ext = get_unit_ext(collision_target);
					if (!collision_target_ext->timers[TimerKelvinMunitions]) {
						collision_target_ext->timers[TimerKelvinMunitions] = 3 * 24 + 1;
					}
				} break;
				case WeaponId::KalkaliskViscousAcid:
				{
					// Kalkalisk - Ablative Spores
					iquare_globals.add_incremental_bullet_multihit(bullet, collision_target);
					auto count = iquare_globals.get_multihit_count(bullet);
					scbw::create_and_init_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, collision_target->position.x, collision_target->position.y, bullet->player);
					collision_target->damage_with(WeaponId::KalkaliskViscousAcid, (weapons_dat::damage_amount[WeaponId::KalkaliskViscousAcid] - count) * 256, bullet->source_unit, bullet->player);

					if (count >= 4) {
						bullet->time_remaining = 0;
					} else {
						bullet->move_target.unit = bullet->attack_target.unit;
						if (bullet->move_target.unit) {
							bullet->target_position = bullet->move_target.unit->position;
							bullet->target_position_2 = bullet->move_target.unit->position;
						}
					}
				} break;
				case WeaponId::ZarcavrokorBonesofReflection:
				{
					iquare_globals.add_bullet_multihit(bullet, collision_target);
					scbw::create_and_init_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, collision_target->position.x, collision_target->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collision_target, 1);
				} break;
				default:
				{
					iquare_globals.add_bullet_multihit(bullet, collision_target);
					scbw::create_and_init_thingy(SpriteId::BurstLasersImpactBlue_Damageless, collision_target->position.x, collision_target->position.y, bullet->player);
					hooks::weapon_bullet_hit(bullet, collision_target, 1);
				}
			}
		}
	}

	void progress_bullet_flight(Bullet* bullet) { // 0048B3B0
		Point16 old_position = bullet->position;
		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores) {
			if (!bullet->attack_target.unit) {
				u32 range = AlkajeliskParasiticSporesSeekRange; // seek distance
				auto seek_target = scbw::UnitFinder::get_nearest_target(
					bullet->position.x - range,
					bullet->position.y - range,
					bullet->position.x + range,
					bullet->position.y + range,
					bullet, [&](Unit* unit) {
					return alkajelisk_spore_match(bullet, unit);
				});
				if (seek_target) {
					scbw::set_bullet_target(bullet, seek_target);
					helper::progress_bullet_movement(bullet);
				}
			} else {
				helper::progress_bullet_movement(bullet);
			}
		} else {
			helper::progress_bullet_movement(bullet);
		}

		BulletExt* bullet_ext = get_bullet_ext(bullet);

		// Create explosions mid-flight
		u32 old_distance_traveled = bullet_ext->distance_traveled;
		bullet_ext->distance_traveled += scbw::get_distance_fast(old_position, bullet->position);
		bullet_ext->time_traveled += 1;
		if (bullet->weapon_id == WeaponId::PositronAntimatterMissile) {
			u32 period = 2 * 32;
			if (bullet_ext->distance_traveled / period > old_distance_traveled / period) {
				scbw::create_and_init_thingy(SpriteId::AntiMatterMissileHit, bullet->position.x, bullet->position.y, bullet->player, bullet->sprite->elevation - 1);
				helper::play_sound_at_pos(scbw::rand_between(SoundId::PositronWeaponHit01, SoundId::PositronWeaponHit03), bullet->position, 1, 0);
				EntryArray targets = {};
				unit_search_radial(&targets, bullet->position, weapons_dat::outer_splash_radius[bullet->weapon_id]);
				unit_search_filter(&targets, 0, 0); // Let's filter with no flags, since that should remove untargetable stuff and what not
				for (u32 i = 0; i < targets.count; ++i) {
					Unit* target = targets.units[i];
					if (scbw::is_allied_to(bullet->player, target->player)) {
						continue;
					}
					u32 distance = scbw::get_distance_fast(bullet->position, target->position);
					u32 divisor = 1;
					if (distance <= weapons_dat::inner_splash_radius[bullet->weapon_id]) {
						divisor = 4;
					} else if (distance <= weapons_dat::medium_splash_radius[bullet->weapon_id]) {
						divisor = 2;
					}
					target->damage_with(bullet->weapon_id, -1, bullet->source_unit, bullet->player, bullet->current_direction, divisor);
				}
			}
		}

		u8 time = bullet->time_remaining--;
		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores) {
			if (bullet->time_remaining < AlkajeliskParasiticSporesTimer && !bullet->attack_target.unit) {
				time = 0;
			}
		}
		bool despawn_bullet = false;

		// Accelerate missiles over time
		if (scbw::weapon_id_accelerates(bullet->weapon_id)) {
			u8 time_current = 255 - time;
			if (time_current == 8) {
				bullet->flingy_top_speed *= 2;
				bullet->flingy_acceleration *= 2;
			}
		}

		// Split missiles
		if (bullet->weapon_id == WeaponId::ArchitectDispersiveSalvo
		&&  bullet->source_unit
		&&  bullet->attack_target.unit
		&&  bullet->attack_target.unit->sprite && bullet->source_unit->sprite) {
			Unit* main_target = bullet->attack_target.unit;
			UnitExt* unit_ext = get_unit_ext(bullet->source_unit);
			const u32 distance_from_source = bullet_ext->distance_traveled;
			const u32 distance_from_target = scbw::get_distance_fast(bullet->position, main_target->position);
			if (distance_from_source > 5 * 32 && distance_from_target < 3 * 32) {
				if (!iquare_globals.multihit_bullet_is_set(bullet)) {
					Unit* old_target = bullet->source_unit->order_target.unit;
					Point16 old_sprite_pos = bullet->source_unit->sprite->position;
					Point16 old_pos = bullet->source_unit->position;
					bullet->source_unit->sprite->position = bullet->position;
					bullet->source_unit->position = bullet->position;
					u32 radius = 3 * 32;
					scbw::UnitFinder units_in_splash(
						main_target->position.x - radius,
						main_target->position.y - radius,
						main_target->position.x + radius,
						main_target->position.y + radius);
					iquare_globals.add_multihit_empty_bullet(bullet, 1);
					u32 fired = 0;
					for (s32 i = 0; i < units_in_splash.get_unit_count(); i++) {
						Unit* target = units_in_splash.get_unit(i);
						if (!target) {
							continue;
						}
						if (target == main_target) {
							continue;
						}
						if (!bullet->source_unit->can_attack_target(target)) { // Check detection status
							continue;
						}
						if (!alkajelisk_spore_match(bullet, target)) {
							continue;
						}
						bullet->source_unit->order_target.unit = target;
						s32 new_angle = scbw::get_angle(bullet->position.x, bullet->position.y, target->get_x(), target->get_y());
						Bullet* new_bullet = create_bullet_ex(WeaponId::ArchitectDispersiveSalvo,
							bullet->source_unit, bullet->position.x, bullet->position.y, bullet->player, new_angle);
						if (new_bullet) {
							helper::play_sound_from_unit(SoundId::ArchitectWeaponSplit01, (Unit*)bullet, 1, 0);
							new_bullet->position = bullet->position;
							iquare_globals.add_multihit_empty_bullet(new_bullet, 1);
							if (++fired == 2) {
								break;
							}
						}
					};
					bullet->source_unit->order_target.unit = old_target;
					bullet->source_unit->position = old_pos;
					bullet->source_unit->sprite->position = old_sprite_pos;
				}
			}
		}

		if (time == 0 || (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y)) {
			despawn_bullet = true;
		} else if (bullet->weapon_id == WeaponId::EmpyreanPlasmaBarrage || bullet->weapon_id == WeaponId::ValkyrieHaloRockets) {
			// Missile interception - Empyrean Heavenly Passage, Valkyrie Charon Afterburners
			u32 range = 128;
			auto collision_target = scbw::UnitFinder::get_nearest_target(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit) {
				if (!(unit->status & UnitStatus::InAir)) {
					return false;
				}
				if (unit->status & UnitStatus::Invincible) {
					return false;
				}
				if (scbw::is_allied_to(bullet->player, unit->player)) {
					return false;
				}
				auto box = units_dat::unit_bounds[unit->id];
				if (bullet->position.x < unit->get_left()) {
					return false;
				}
				if (bullet->position.x > unit->get_right()) {
					return false;
				}
				if (bullet->position.y < unit->get_top()) {
					return false;
				}
				if (bullet->position.y > unit->get_bottom()) {
					return false;
				}
				return true;
			});
			if (collision_target) {
				bullet->attack_target.unit = collision_target;
				despawn_bullet = true;
			}
		} else if (bullet->weapon_id == WeaponId::StarSovereignInterventionMissiles) {
			// Star Sovereign Intervention
			u32 range = 128;
			auto collision_target = scbw::UnitFinder::get_nearest_target(
				bullet->position.x - range, bullet->position.y - range,
				bullet->position.x + range, bullet->position.y + range,
				bullet, [&](Unit* unit) {
				if (unit->status & UnitStatus::Invincible) {
					return false;
				}
				if (scbw::is_allied_to(bullet->player, unit->player)) {
					return false;
				}
				auto box = units_dat::unit_bounds[unit->id];
				if (bullet->position.x < unit->get_left()) {
					return false;
				}
				if (bullet->position.x > unit->get_right()) {
					return false;
				}
				if (bullet->position.y < unit->get_top()) {
					return false;
				}
				if (bullet->position.y > unit->get_bottom()) {
					return false;
				}
				return true;
			});
			if (collision_target) {
				bullet->attack_target.unit = collision_target;
				despawn_bullet = true;
			}
		} else if (scbw::weapon_pierces_targets(bullet->weapon_id)) {
			// Prolonged Arms - for projectiles hitting targets mid-flight
			bool enabled = true;
			if (bullet->weapon_id == WeaponId::SilvertongueWordsmithRailgun) {
				enabled = bullet->source_unit && get_unit_ext(bullet->source_unit)->timers[TimerDecloak] > 1;
			}

			if (enabled) {
				Vector32 diff = scbw::get_vector_between_points(old_position, bullet->position); 
				for (u32 i = 1; i <= 4; ++i) {
					Point16 position = old_position;
					position.x += diff.x * i / 4;
					position.y += diff.y * i / 4;
					apply_bullet_piercing(bullet, position, 16);
				}
			}
		}

		if (despawn_bullet) {
			Unit* target = bullet->attack_target.unit;
			bullet->behavior_type_internal = 5;

			bool hit = true;
			if (scbw::weapon_pierces_targets(bullet->weapon_id)) {
				apply_bullet_piercing(bullet, bullet->position, 16);
				if (iquare_globals.was_already_hit_by_multihit_bullet(bullet, bullet->attack_target.unit)) {
					hit = false;
				}
			}

			if (target && target->sprite && hit) {
				bullet->attack_target.pt.x = target->sprite->position.x;
				bullet->attack_target.pt.y = target->sprite->position.y;
			} else {
				bullet->attack_target.unit = 0;
			}
			Sprite* bullet_sprite = bullet->sprite;
			bullet->order_signal |= 0x80; // new
			bullet->unknown_0x4E = 0;
			bullet->some_unit_type = 228;
			for (Image* image = bullet_sprite->images.head; image; image = image->link.next) {
				helper::play_image_iscript(image, IscriptAnimation::Death);
			}
		}
	}

	void bullet_behavior_bounce(Bullet* bullet) { // 0048B2D0
		Unit* target_unit = bullet->attack_target.unit;
		if (target_unit) {
			if (!(bullet->hit_flags & 1)) {
				helper::change_move_pos(bullet, target_unit->sprite->position.y, target_unit->sprite->position.x);
			}
		}

		Point16 old_position = bullet->position;
		helper::progress_bullet_movement(bullet);

		BulletExt* bullet_ext = get_bullet_ext(bullet);
		u32 old_distance_traveled = bullet_ext->distance_traveled;
		bullet_ext->distance_traveled += scbw::get_distance_fast(old_position, bullet->position);
		bullet_ext->time_traveled += 1;
		if (bullet->weapon_id == WeaponId::BactaliskCorrosiveAcid && !bullet_ext->bounced) {
			u32 period = 4 * 32;
			if (bullet_ext->distance_traveled / period > old_distance_traveled / period) {
				bullet->remaining_bounces += 1;
			}
		}

		if (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y) {
			bullet_ext->bounced = true;
			char next_bounce = bullet->remaining_bounces - 1;
			// Gladius infinite bouncing
			if (bullet->weapon_id == WeaponId::GladiusPlasmaFlurry) {
				next_bounce = 1;
			}

			bullet->remaining_bounces = next_bounce;
			iquare_globals.bounce_bullet = bullet;
			Unit* target = helper::find_next_bounce_target(bullet);
			bullet->next_bounce_unit = bullet->attack_target.unit;
			if (next_bounce && target) {
				for (Image* image = bullet->sprite->images.head; image; image = image->link.next) {
					helper::play_image_iscript(image, IscriptAnimation::SpecialState1);
				}
				bullet->attack_target.unit = target;
			} else {
				if (bullet->weapon_id == WeaponId::GladiusPlasmaFlurry) {
					iquare_globals.remove_multihit_bullet(bullet);
				}
				target_unit = bullet->attack_target.unit;
				bullet->behavior_type_internal = 5;
				if (target_unit) {
					bullet->attack_target.pt.x = target_unit->sprite->position.x;
					bullet->attack_target.pt.y = target_unit->sprite->position.y;
				} else {
					bullet->attack_target.unit = 0;
				}
				Sprite* sprite = bullet->sprite;
				bullet->unknown_0x4E = 0;
				bullet->some_unit_type = UnitId::None;

				for (Image* image = sprite->images.head; image; image = image->link.next) {
					helper::play_image_iscript(image, IscriptAnimation::Death);
				}
			}
		}
	}

	bool find_next_bounce_target_proc(Unit* target) // 0048AFD0
	{
		Unit* bounce_source = *g_bullet_bounce_missile_source;
		s32 bounce_source_status = (*g_bullet_bounce_missile_source)->status;
		Unit* target_var = target;

		// Previously disallowed bouncing if unit was disabled
		if (!target)
		{
			return 0;
		}
		if (target->status & UnitStatus::Invincible)
		{
			return 0;
		}
		if (target->sprite->flags & SpriteFlags::Hidden)
		{
			return 0;
		}
		if (helper::unit_can_see_cloaked_target(target, (*g_bullet_bounce_missile_source)))
		{
			return 0;
		}

		Unit* bounce_source_unit = bounce_source;
		if (!bounce_source->subunit && helper::unit_is_subunit(bounce_source->subunit))
		{
			bounce_source_unit = bounce_source->subunit;
		}
		if (!(helper::unit_is_carrier(bounce_source)))
		{
			bool miscBool;
			if (helper::unit_is_reaver(bounce_source))
			{
				if (target_var->status & UnitStatus::InAir)
				{
					return 0;
				}
				miscBool = helper::unit_has_path_to_unit(bounce_source, target_var);
			}
			else
			{
				if (helper::unit_can_infest(bounce_source))
				{
					Unit* unusedUnit = nullptr;
					miscBool = helper::can_be_infested(unusedUnit, target_var) != 0; // unusedUnit is not used..?
				}
				else
				{
					if (target_var->status & UnitStatus::InAir)
					{
						miscBool = units_dat::air_weapon[(u16)bounce_source_unit->id] != -126;
					}
					else
					{
						miscBool = units_dat::ground_weapon[(u16)bounce_source_unit->id] != -126;
					}
				}
			}
			if (!miscBool)
			{
				return 0;
			}
		}

		s32 unitStatus = target_var->status;
		if (unitStatus & 0x300 && !((1 << bounce_source->player) & target_var->detection_status))
		{
			return 0;
		}

		s8 player = target_var->player == 11 ? target_var->sprite->player : target_var->player;
		if (player_alliance[(u8)bounce_source->player].flags[player] || *g_bullet_bounce_missile_target == target_var)
		{
			return 0;
		}

		// Gladius Binding Plasma behavior
		if (iquare_globals.bounce_bullet->weapon_id == WeaponId::GladiusPlasmaFlurry)
		{
			if (target != nullptr && *g_bullet_bounce_missile_next != nullptr
				&& (*g_bullet_bounce_missile_next)->sprite != nullptr
				&& target_var->sprite != nullptr)
			{
				if (iquare_globals.was_already_hit_by_multihit_bullet(iquare_globals.bounce_bullet, target_var))
				{
					return false;
				}
				else
				{
					iquare_globals.add_bullet_multihit(iquare_globals.bounce_bullet, target_var);
					return *g_bullet_bounce_missile_next != target_var;

				}
			}
			else
			{
//				scbw::print("bad bullet");
			}
		}
		return *g_bullet_bounce_missile_next != target_var;
	}

	Unit* find_next_bounce_target(Bullet* bullet) // 0048B1E0
	{
		if (!bullet->source_unit)
		{
			return 0;
		}
		Unit* next_bounce = bullet->next_bounce_unit;
		*g_bullet_bounce_missile_source = bullet->source_unit;
		Unit* target = bullet->attack_target.unit;
		Sprite* bullet_sprite = bullet->sprite;
		*g_bullet_bounce_missile_target = target;
		u16 posY = bullet_sprite->position.x;
		u16 posX = bullet_sprite->position.y;
		*g_bullet_bounce_missile_next = next_bounce;
		Box16 coords;
		coords.left = posX - 96;
		coords.top = posX + 96;
		coords.right = posY - 96;
		coords.bottom = posY + 96;
//		return (Unit*)helper::iterate_units_at_location_target_proc(&coords, find_next_bounce_target_proc, 0);
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
			Bullet* next_bounce_bullet = *first_free_bullet;
			if (*first_free_bullet)
			{
				if (*last_free_bullet == *first_free_bullet)
				{
					*last_free_bullet = bullet;
				}
				bullet->previous = *first_free_bullet;
				bullet->next = next_bounce_bullet->next;
				Bullet* target = next_bounce_bullet->next;
				if (target)
				{
					target->previous = bullet;
				}
				next_bounce_bullet->next = bullet;
				*bullet_count -= 1;
			}
			else
			{
				*last_free_bullet = bullet;
				*first_free_bullet = bullet;
				*bullet_count -= 1;
			}
			iquare_globals.remove_multihit_bullet(bullet);
		}
		return bullet;
	}

	// Hooked to stop Valkyries from being cucked by lardcode. - Pr0nogo
	Bullet* create_bullet(s32 weapon_id, Unit* parent, s32 x, s16 y, u8 player, s32 angle) // 0048C260
	{
		Bullet* free_bullet = *first_free_bullet;
		if (!*first_free_bullet)
		{
			return 0;
		}
		if (parent->status & UnitStatus::CanNotAttack)
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
		if (*first_free_bullet == free_bullet)
		{
			*first_free_bullet = free_bullet->next;
		}
		if (*last_free_bullet == free_bullet)
		{
			*last_free_bullet = free_bullet->previous;
		}
		if (free_bullet->previous)
		{
			free_bullet->previous->next = free_bullet->next;
		}
		
		Bullet* next_bullet = free_bullet->next;
		if (next_bullet)
		{
			next_bullet->previous = free_bullet->previous;
		}
		free_bullet->previous = 0;
		free_bullet->next = 0;
		Bullet* result = 0;
		
		Bullet* first_bullet_var = *first_bullet;
		if (*first_bullet)
		{
			if (*last_bullet == *first_bullet)
			{
				*last_bullet = free_bullet;
			}
			free_bullet->previous = *first_bullet;
			free_bullet->next = first_bullet_var->next;
			Bullet* next_bullet_var = first_bullet_var->next;
			if (next_bullet_var)
			{
				next_bullet_var->previous = free_bullet;
			}
			first_bullet_var->next = free_bullet;
			++*bullet_count;
			result = free_bullet;
		}
		else
		{
			++*bullet_count;
			*last_bullet = free_bullet;
			*first_bullet = free_bullet;
			result = free_bullet;
		}
		return result;
	}

	// Hooked so we don't need aise for Bactalisks. - Pr0nogo
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle) { // 0048BEC0
		if (!helper::initialize_flingy(weapons_dat::flingy_id[weapon_id], y, x, (Flingy*)bullet, player, angle)) { // Unit/Bullet/Flingy have a shared header, so this is presumably fine
			return 0;
		}

		bullet->player = player;
		bullet->some_unit_type = UnitId::None;
		bullet->unknown_0x4E = 0;
		bullet->attack_target.unit = nullptr;
		bullet->attack_target.pt.y = 0;
		bullet->attack_target.pt.x = 0;
		bullet->unknown_0x54 = 0;
		bullet->weapon_id = weapon_id;
		bullet->time_remaining = weapons_dat::remove_after[weapon_id];
		bullet->some_unit_type = UnitId::None;
		bullet->remaining_bounces = 0;
		bullet->hit_flags = 0;
		bullet->movement_flags |= MovementFlags::StartingAttack;
		bullet->behavior_type_internal = 0;
		bullet->attack_target.unit = nullptr;
		bullet->attack_target.pt.y = 0;
		bullet->attack_target.pt.x = 0;
		bullet->unknown_0x4E = 0;
		hooks::bullet_behavior_reassign(bullet);
		reset_bullet_ext(bullet);
		BulletExt* bullet_ext = get_bullet_ext(bullet);
		bullet_ext->bullet_source = bullet->position;

		u8 launch_spin = weapons_dat::launch_spin[weapon_id];
		if (launch_spin) {
			s32 launch_spin_variance = 0;
			if (parent == *g_last_bullet_creator) {
				launch_spin_variance = *g_launch_spin_random == 0;
			} else {
				launch_spin_variance = helper::randomize_short(0) & 1;
			}

			*g_launch_spin_random = launch_spin_variance;
			bool invert_spin = launch_spin_variance == 0;
			launch_spin_variance = launch_spin;
			if (invert_spin) {
				launch_spin_variance = -(s8)launch_spin;
			}
			char launch_spin_direction = launch_spin_variance + bullet->direction2;
			bullet->direction_1 = launch_spin_direction;
			bullet->direction2 = launch_spin_direction;
			*g_last_bullet_creator = parent;
		}

		bullet->source_unit = parent;
		if (units_dat::base_property[parent->id] & UnitProperty::Subunit) {
			bullet->source_unit = parent->subunit;
		}
		if (parent->id == UnitId::ProtossPlasmaShell) {
			bullet->source_unit = parent->hangar_unit.parent;
		}
		if (parent->status & UnitStatus::IsHallucination) {
			bullet->hit_flags |= 2u;
		}
		bullet->next_bounce_unit = 0;

		Unit* target_unit = parent->order_target.unit;
		if (target_unit) {
			bullet->sprite->elevation = target_unit->sprite->elevation + 1;
			bullet->attack_target.unit = target_unit;
			bullet->attack_target.pt.x = target_unit->sprite->position.x;
			bullet->attack_target.pt.y = target_unit->sprite->position.y;
		} else {
			bullet->sprite->elevation = parent->sprite->elevation + 1;
			bullet->attack_target.unit = 0;
			bullet->attack_target.pt.x = parent->order_target.pt.x;
			bullet->attack_target.pt.y = parent->order_target.pt.y;
		}

		switch (weapons_dat::behavior[weapon_id]) {
			case WeaponBehavior::Fly_DoNotFollowTarget:
			case WeaponBehavior::Fly_FollowTarget:
			{
				if (target_unit) {
					if (parent) {
						helper::weapon_behavior_can_hit_target_check(target_unit, parent);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnTargetUnit:
			case WeaponBehavior::AppearOnTargetSite:
			{
				if (!target_unit) {
					return 1;
				}
				if (!bullet->source_unit) {
					return 1;
				}
				helper::weapon_behavior_can_hit_target_check(target_unit, bullet->source_unit);
				return 1;
			}
			case WeaponBehavior::PersistOnTargetSite:
			{
				helper::appear_on_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
			case WeaponBehavior::AppearOnAttacker:
			{
				return 1;
			}
			case WeaponBehavior::AttackAndSelfDestruct:
			{
				if (!bullet->source_unit) {
					return 1;
				}
				bullet->source_unit->status |= UnitStatus::IsSelfDestructing;
				bullet->source_unit->user_action_flags |= 0x4;
				helper::orders_self_destructing(bullet->source_unit);
				return 1;
			}
			case WeaponBehavior::Bounce:
			{
				switch (weapon_id) {
					case WeaponId::BactaliskCorrosiveAcid:
					{
						bullet->remaining_bounces = 1; // Additional bounces added later. -Veeq7
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					{
						bullet->remaining_bounces = 2;
					} break;
					default:
					{
						bullet->remaining_bounces = 3;
					}
				}
				if (target_unit) {
					if (bullet->source_unit) {
						helper::weapon_behavior_can_hit_target_check(target_unit, bullet->source_unit);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
			case WeaponBehavior::AttackNearbyArea:
			{
				bullet->cyclic_missile_index = parent->offset_index_3_by_3;
				bullet->attack_target.pt.x -= (s16)g_attack_curve[parent->offset_index_3_by_3].x;
				bullet->attack_target.pt.y -= (s16)g_attack_curve[parent->offset_index_3_by_3].y;
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);

				s16 finalOffset = parent->offset_index_3_by_3 + 1;
				if (!g_attack_curve[finalOffset].x) {
					finalOffset = 0;
				}
				parent->offset_index_3_by_3 = finalOffset;
				return 1;
			}
			case WeaponBehavior::GoToMaxRange:
			{
				s32 max_range_var = (weapons_dat::max_range[bullet->weapon_id] + 20) << 8;
				bullet->attack_target.pt.x = parent->sprite->position.x + ((u32)(max_range_var * g_angle_distance[bullet->direction_1].x) >> 16);
				bullet->attack_target.pt.y = parent->sprite->position.y + ((u32)(max_range_var * g_angle_distance[bullet->direction_1].y) >> 16);
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
		}
		return 0;
	}

	// Hooked to investigate weaponsdat extender crashes. Disabled! - Pr0nogo
	void progress_bullet_frames() { // 0048C3D0
		Bullet* bullet = *first_bullet;
		Bullet* next_bullet = *first_bullet;
		while (next_bullet) {
			next_bullet = bullet->next;
			*g_active_iscript_entity = (Unit*)bullet;
			*g_active_iscript_bullet = bullet;
			helper::set_thingy_visibility_flags((Thingy*)bullet);
			Sprite* bullet_sprite = bullet->sprite;
			if (bullet_sprite)
			{
				helper::sprite_to_iscript_loop(bullet_sprite);
				if (!bullet->sprite->images.head)
				{
					bullet->sprite = 0;
				}
			}
			u32 bullet_behavior = (u8)bullet->behavior_type_internal;
			switch (bullet_behavior)
			{
				case 0:
				{
					hooks::bullet_behavior_reassign(bullet);
				} break;
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
			}
			bullet = next_bullet;
		}
		*g_active_iscript_entity = 0;
		*g_active_iscript_bullet = 0;
	}

	// Hooked to investigate first_free_bullet nonsense. Disabled! - Pr0nogo
	void bullet_behavior_reassign(Bullet* bullet) { // 0048BD50
		while (2) { // xd
			if (bullet->unknown_0x4E & 1) {
				bullet->unknown_0x4E = bullet->unknown_0x4E & 0xFE;
				Image* bullet_overlay = 0;
				switch (weapons_dat::behavior[bullet->weapon_id]) {
					case WeaponBehavior::AppearOnTargetUnit:
					case WeaponBehavior::AppearOnTargetSite:
					case WeaponBehavior::AppearOnAttacker:
					case WeaponBehavior::AttackAndSelfDestruct:
					{
						bullet->behavior_type_internal = WeaponBehavior::AppearOnAttacker;
						if (bullet->attack_target.unit) {
							bullet->attack_target.pt.x = bullet->attack_target.unit->sprite->position.x;
							bullet->attack_target.pt.y = bullet->attack_target.unit->sprite->position.y;
						} else {
							bullet->attack_target.unit = 0;
						}
						bullet->unknown_0x4E = 0;
						bullet->some_unit_type = 228;
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
						if (bulletOverlay) {
							do {
								helper::play_image_iscript(bulletOverlay, 14);
								bulletOverlay = bulletOverlay->link.next;
							} while (bulletOverlay);
						}
					} break;
					case WeaponBehavior::AttackNearbyArea:
					{
						helper::assign_weapon_data(bullet, 6);
						Image* bullet_overlay = bullet->sprite->images.head;
						if (bullet_overlay) {
							do {
								helper::play_image_iscript(bullet_overlay, 2);
								bullet_overlay = bullet_overlay->link.next;
							} while (bullet_overlay);
						}
					} break;
					default:
					{
						helper::assign_weapon_data(bullet, 1);
						Image* bullet_overlay = bullet->sprite->images.head;
						if (bullet_overlay) {
							do {
								helper::play_image_iscript(bullet_overlay, 2);
								bullet_overlay = bullet_overlay->link.next;
							}
							while (bullet_overlay);
						}
					} break;
				}

				switch (bullet->behavior_type_internal) {
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

	// Hooked to help with kiting AI. Disabled! -Pr0nogo
	// Ended up useless sadly, as it calls way more often than I anticipated. Replaced with starting attack check from movement flags. -Veeq7
	void get_weapon_iscript_init(Unit* unit, char iscript_anim) // 00476ED0
	{
		*g_last_bullet_creator = 0;
		if (!(unit->status & UnitStatus::CanNotAttack))
		{
			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit->order_target.unit && unit->order_target.unit->status & UnitStatus::InAir)
			{
				helper::use_weapon_iscript(unit, units_dat::air_weapon[unit->id], 2, iscript_anim + 1);
			}
			else
			{
				if (unit->id != UnitId::ZergLakizilisk || unit->status & UnitStatus::Burrowed)
				{
					helper::use_weapon_iscript(unit, units_dat::ground_weapon[unit->id], 1, iscript_anim);
				}
				else
				{
					helper::use_weapon_iscript(unit, WeaponId::None, 1, iscript_anim);
				}
			}
		}
	}

	// Hooked to remove randomization and move cooldown handling. -Pr0nogo
	s32 use_weapon_iscript(Unit* unit, u8 weapon_id, u32 attack_type, char iscript_anim) { // 00476C90
		if (weapon_id == WeaponId::None) {
			return false;
		}
		u8 cooldown = attack_type == 1 ? unit->ground_weapon_cooldown : unit->air_weapon_cooldown;
		if (cooldown) {
			if (unit->order_queue_timer > cooldown - 1) {
				unit->order_queue_timer = cooldown - 1;
			}
			return false;
		}
		if (!helper::is_ready_to_attack(unit, weapon_id)) {
			return false;
		}

		unit->movement_flags |= MovementFlags::StartingAttack;

		// Previously had randomized values and cooldown handling.
		// Now applied in fire bullet and attack melee.
	//	u8 cooldown = helper::get_weapon_cooldown(unit, weapon_id);
	//	unit->ground_weapon_cooldown = cooldown;
	//	unit->air_weapon_cooldown = cooldown;

		for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
			helper::play_image_iscript(image, iscript_anim);
		}
		return true;
	}

	// Hooked to fix melee air unit performance. -Pr0nogo
	b32 should_stay_at_range(Unit* unit) // 00475A10
	{
		if (scbw::unit_id_is_melee_air_unit(unit->id))
		{
			return 0;
		}
		u32 weapon_id = units_dat::ground_weapon[unit->id];
		if (weapon_id == WeaponId::None)
		{
			return 1;
		}
		else
		{
			return weapons_dat::flingy_id[weapon_id] != 0;
		}
	}

	// Hooked to fix melee air unit performance. -Pr0nogo
	void move_for_melee_range(Unit* target, Unit* attacker) // 00477F10
	{
		if (!helper::is_target_visible(attacker, target) || !(target->movement_flags & MovementFlags::Accelerating))
		{
			helper::move_towards(attacker, target);
			return;
		}

		Sprite* target_sprite = target->sprite;
		s32 tgt_x = target_sprite->position.x;
		s32 tgt_y = target_sprite->position.y;

		s32 attacker_halt = helper::get_flingy_halt_distance(attacker);
		s32 target_vel = target->velocity_direction_1;
		s32 attacker_halt_off1 = 3 * attacker_halt;
		s32 attacker_halt_off2 = 3 * attacker_halt * g_angle_distance[target_vel].x;
		u32 tgt_x_final = tgt_x + (attacker_halt_off2 >> 16);
		u32 tgt_y_final = tgt_y + (attacker_halt_off1 * g_angle_distance[target_vel].y >> 16);

		if (tgt_x_final >= *g_map_width_pixels || tgt_y_final >= *g_map_height_pixels)
		{
			tgt_y_final = tgt_y;
		}
		else
		{
			tgt_x_final = tgt_x;
		}

		// Disabled to make the chasing more responsive. -Pr0nogo
	//	char timer = attacker->recent_order_timer;
	//	if (!timer)
	//	{
			helper::set_move_target_xy(attacker, tgt_x_final, tgt_y_final);
	//	}
	}
}