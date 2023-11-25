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
#include "hooks/unit_stats/armor_bonus.h"
#include "weapon_damage.h"
#include "weapon_spellhit.h"
#include "weapon_cooldown.h"

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

Target make_target(Unit* unit, Point16 pos) {
	if (unit && !pos.x && !pos.y) {
		pos = unit->position; 
	}
	return {pos, unit};
}

Point8 peek_bullet_offset(Unit* unit, WeaponOffsets* weapon_offsets) {
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
		case UnitId::TerranDiadem:
		{
			image = unit->get_overlay(ImageId::DiademTurret);
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
	return offset;
}

Point8 get_next_bullet_offset(Unit* unit, WeaponOffsets* weapon_offsets) {
	Point8 offset = peek_bullet_offset(unit, weapon_offsets);
	UnitExt* unit_ext = get_unit_ext(unit);
	u32 rotate = unit_ext->values[weapon_offsets->value_id];
	rotate++;
	if (rotate >= weapon_offsets->count) {
		rotate = 0;
	}
	unit_ext->values[weapon_offsets->value_id] = rotate;
	return offset;
}

Point16 make_launch_pos(Point16 unit_position, Point8 offset) {
	unit_position.x += offset.x;
	unit_position.y += offset.y;
	return unit_position;
}

u32 get_correction_distance(Point16 unit_pos, u32 unit_id, Point16 source_pos, Point16 other_pos) {
	Point16 edge_point = scbw::get_rect_edge_point_towards(unit_pos, units_dat::unit_bounds[unit_id], other_pos);
	return scbw::get_distance_fast(source_pos, edge_point);
}

Bullet* fire_bullet(Unit* unit, u32 weapon_id, Target target, WeaponOffsets* weapon_offsets, bool apply_cooldown, bool ignore_max_range, bool dont_follow) {
	if (!unit->sprite) {
		return 0;
	}

	bool is_ion_cannon = unit->id == UnitId::TerranDiadem;
	if (is_ion_cannon) {
		UnitExt* unit_ext = get_unit_ext(unit);
		target.unit = 0;
		target.pt.x = unit_ext->values[ValueIonCannonTargetPosX];
		target.pt.y = unit_ext->values[ValueIonCannonTargetPosY];
	}

	Point16 launch_pos = unit->position;
	if (weapons_dat::behavior[weapon_id] == WeaponBehavior_AppearOnTargetUnit) {
		launch_pos = target.pt;
	}
	if (weapon_offsets) {
		launch_pos = make_launch_pos(launch_pos, get_next_bullet_offset(unit, weapon_offsets));
	}
	if (!ignore_max_range && weapons_dat::behavior[weapon_id] == WeaponBehavior_GoToMaxRange) {
		Vector32f diff = scbw::get_normalized_vector(scbw::get_vector_between(launch_pos, target.pt));
		u32 max_range = hooks::get_max_weapon_range(unit, weapon_id, target.unit) + 16;
		max_range += get_correction_distance(unit->position, unit->id, launch_pos, target.pt);
		Vector32f target_pos = { launch_pos.x + (diff.x * max_range), launch_pos.y + (diff.y * max_range) };

		if (target_pos.x < 0) {
			target_pos.y = (1 - abs(target_pos.x / (target_pos.x - target.pt.x))) * (target_pos.y - target.pt.y) + target.pt.y;
			target_pos.x = 0;
		}
		if (target_pos.y < 0) {
			target_pos.x = (1 - abs(target_pos.y / (target_pos.y - target.pt.y))) * (target_pos.x - target.pt.x) + target.pt.x;
			target_pos.y = 0;
		}
		target.pt.x = target_pos.x;
		target.pt.y = target_pos.y;
	}
	s32 angle = scbw::get_angle(launch_pos.x, launch_pos.y, target.pt.x, target.pt.y);

	Target original_target = unit->order_target;
	unit->order_target = target;
	Bullet* bullet = create_bullet_ex(weapon_id, unit, launch_pos.x, launch_pos.y, unit->player, angle, apply_cooldown);
	unit->order_target = original_target;
	if (bullet) {
		if (dont_follow || weapons_dat::behavior[weapon_id] == WeaponBehavior_GoToMaxRange) {
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

Bullet* create_bullet_ex(u8 weapon_id, Unit* source, s16 x, s16 y, u8 attacking_player, u8 direction, bool apply_cooldown) {
	Bullet* bullet = helper::create_bullet(weapon_id, source, x, y, attacking_player, direction);
	if (bullet) {
		scbw::decloak_passive_proc(source);
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

static Point16 set_target_range(Point16 origin, Point16 point, f32 distance) {
	Vector32f vector = scbw::get_vectorf_between(origin, point);
	f32 old_distance = scbw::get_vector_distance(vector);

	vector.x *= distance / old_distance;
	vector.y *= distance / old_distance;

	Point16 result = origin;
	result.x += vector.x;
	result.y += vector.y;
	return result;
}

// Fly & Follow is unsupported
void fire_line(Unit* unit, u32 weapon_id, u32 total_count, u32 forward_spread, u32 lines, u32 side_spread, WeaponOffsets* weapon_offsets) {
	UnitExt* unit_ext = get_unit_ext(unit);
	u32 index = unit_ext->values[ValueProjectilePattern];
	if (index == 0) {
		unit_ext->values[ValueProjectileAngle] = unit->current_direction_1;
		unit_ext->projectile_pattern_target = unit->order_target;
	}

	Target target = unit_ext->projectile_pattern_target;
	u32 forward_index = index / lines;
	u32 forward_count = total_count / lines;
	s32 forward_offset = (forward_spread * forward_index) - (forward_spread * (forward_count - 1) / 2);
	u32 forward_angle = unit_ext->values[ValueProjectileAngle];
	target.pt.x += scbw::get_polar_x(forward_offset, forward_angle);
	target.pt.y += scbw::get_polar_y(forward_offset, forward_angle);

	if (lines > 1) {
		u32 side_index = index % lines;
		s32 side_offset = (side_spread * side_index) + (side_spread * (lines - 1) / 2);
		u32 side_angle = (forward_angle + 64) % 256;
		target.pt.x -= scbw::get_polar_x(side_offset, side_angle);
		target.pt.y -= scbw::get_polar_y(side_offset, side_angle);
	}

	fire_bullet(unit, weapon_id, target, weapon_offsets, true, false, true);
	unit_ext->values[ValueProjectilePattern] = (index + 1) % total_count;
}

void fire_shotgun(Unit* unit, u32 weapon_id, u32 side_count, u32 angle_spread, u32 vertical_scatter, WeaponOffsets* weapon_offsets) {
	Target target = unit->order_target;

	u32 max_range = scbw::get_distance_fast(unit->position, target.pt);
	if (weapons_dat::behavior[weapon_id] == WeaponBehavior_GoToMaxRange) {
		u32 max_range = hooks::get_max_weapon_range(unit, weapon_id, target.unit) + 24;
	}
	{
		Point16 launch_pos = unit->position;
		if (weapon_offsets) {
			launch_pos = make_launch_pos(launch_pos, peek_bullet_offset(unit, weapon_offsets));
		}
		target.pt = set_target_range(launch_pos, target.pt, max_range);
		fire_bullet(unit, weapon_id, target, weapon_offsets);
	}

	for (s32 i = 1; i <= side_count; ++i) {
		s32 current_scatter = vertical_scatter * i;
		if (i % 2 == 1) {
			current_scatter *= -1;
		}
		u32 current_range = max_range + current_scatter;

		for (u32 j = 0; j < 2; ++j) {
			Point16 launch_pos = unit->position;
			if (weapon_offsets) {
				launch_pos = make_launch_pos(launch_pos, peek_bullet_offset(unit, weapon_offsets));
			}
			f64 angle = i * angle_spread;
			if (j == 0) {
				angle = 360 - angle;
			}
			Point16 target_pt = scbw::rotate_point(launch_pos, set_target_range(launch_pos, target.pt, current_range), angle);
			fire_bullet(unit, weapon_id, make_target(target.unit, target_pt), weapon_offsets, true, true, true);
		}
	}
}

bool fire_multiple(Unit* unit, u32 weapon_id, u32 max_targets, u32 filters, FireMultipleOptions options) {
	u32 attack_range = options.range_overwrite ? options.range_overwrite : hooks::get_max_weapon_range(unit, weapon_id, 0);
	u32 factor = options.use_weapon_factor ? weapons_dat::damage_factor[weapon_id] : 1; // @TODO: Find some more sane way of doing this, which may be challenging in BW... -Pr0nogo

	UnitExt* unit_ext = get_unit_ext(unit);
	bool has_stagger = options.stagger >= 0;
	auto& blacklist = unit_ext->unit_vectors[UnitVectorStaggerBlacklist];
	if (has_stagger) {
		options.apply_cooldown = false;
	}

	Unit* fired_at = 0;
	Unit* order_target_unit = unit->order_target.unit;
	if (!options.ignore_main
	&&   order_target_unit
	&&   unit->main_order_id != OrderId::Follow
	&&   unit->main_order_id != OrderId::Patrol
	&&   unit->is_target_within_min_range(order_target_unit, attack_range)
	&&   unit->can_attack_target(order_target_unit))
	{
		if (weapon_id_is_secondary(weapon_id) && scbw::is_allied_to(unit->player, order_target_unit->player)) {
			return false; // This is stupid, but the unit_flag_match handling causes tism with entering transports. Should make more elegant later. -Pr0nogo
		}
		if (unit_flag_match(order_target_unit, unit, filters & ~(FilterAlly | FilterEnemy))) { // We want to attack the order target regardless of alliance flags
			if (!(has_stagger && std::find(blacklist.begin(), blacklist.end(), order_target_unit) != blacklist.end())) {
				for (u32 i = 0; i < factor; i++) {
					Bullet* bullet = fire_bullet(unit, weapon_id, make_target(order_target_unit), options.weapon_offsets, options.apply_cooldown);
					if (bullet) {
						fired_at = order_target_unit;
						BulletExt* bullet_ext = get_bullet_ext(bullet);
						bullet_ext->divisor_multiplier = options.divisor_multiplier;
						if (has_stagger) {
							goto fired_stagger;
						}
					}
				}
			}
		}
	}

	EntryArray targets = {};
	if (options.center_on_target) {
		if (!order_target_unit) {
			return fired_at != 0;
		}
		unit_search_radial(&targets, order_target_unit->position, options.center_on_target_range_overwrite ? options.center_on_target_range_overwrite : attack_range);
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
	Point16 sort_center = order_target_unit ? order_target_unit->position : unit->position;
	unit_search_sort_by_distance(&targets, sort_center);

	u32 count = targets.count;
	if (max_targets && count > max_targets) {
		count = max_targets;
	}

	for (u32 i = 0; i < count; ++i) {
		Unit* target = targets.units[i];
		if  (target == unit
		||   target == order_target_unit
		||   scbw::is_allied_to(unit->player, target->player)
		||  !unit->can_attack_target(target)
		||  !unit->is_target_within_min_range(target, attack_range)) {
			continue;
		}

		if (has_stagger && std::find(blacklist.begin(), blacklist.end(), target) != blacklist.end()) {
			continue;
		}

		for (u32 i = 0; i < factor; i++) {
			Bullet* bullet = fire_bullet(unit, weapon_id, make_target(target), options.weapon_offsets, options.apply_cooldown);
			if (bullet) {
				fired_at = target;
				BulletExt* bullet_ext = get_bullet_ext(bullet);
				bullet_ext->divisor_multiplier = options.divisor_multiplier;
				if (has_stagger) {
					goto fired_stagger;
				}
			}
		}
	}

	if (has_stagger) {
		if (fired_at) {
		fired_stagger:
			blacklist.push_back(fired_at);
			unit->ground_weapon_cooldown = options.stagger;
			unit->air_weapon_cooldown = options.stagger;
			unit_ext->timers[TimerClearStaggerBlacklist] = options.stagger + 6; // Ideally we would get attack animation time instead of hardcoding 6. -Veeq7
			unit_ext->values[ValueStaggerWeaponId] = weapon_id;
		} else {
			blacklist.clear();
			unit_ext->timers[TimerClearStaggerBlacklist] = 0;
			apply_weapon_cooldown(unit, weapon_id);
		}
	}
	return fired_at != 0;
}

namespace
{
	typedef void(__stdcall* GetWeaponHitPosFunc)(Unit* unit, s32* x, s32* y);
	GetWeaponHitPosFunc const get_weapon_hit_pos = (GetWeaponHitPosFunc)0x004762C0;
}

namespace hooks {
	// This hook affects the following iscript opcodes: attackwith, attack, castspell
	// This also affects Unit::fire_weapon().
	void fire_weapon(Unit* unit, u8 weapon_id) { // 00479C90
		if (!unit || !unit->sprite) {
			return;
		}

		UnitExt* unit_ext = get_unit_ext(unit);
		unit_ext->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::damage_factor[weapon_id] + 1;
		unit_ext->values[ValueAI_PreventTargeting] = 0;

		// Retrieve the spawning position for the bullet.
		s32 x = 0;
		s32 y = 0;
		u8 behavior = weapons_dat::behavior[weapon_id];
		if (weapon_id == WeaponId::HypnagogueThroughTheLookingGlass) {
			behavior = WeaponBehavior_AppearOnTargetSite;
		}
		if (behavior == WeaponBehavior_AppearOnTargetUnit) {
			if (!unit->order_target.unit) {
				return;
			}
			get_weapon_hit_pos(unit, &x, &y);
		} else if (behavior == WeaponBehavior_AppearOnTargetSite) {
			x = unit->order_target.pt.x;
			y = unit->order_target.pt.y;
		} else {
			s32 forward_offset = weapons_dat::forward_offset[weapon_id];
			x = unit->position.x + scbw::get_polar_x(forward_offset, unit->current_direction_1);
			y = unit->position.y + scbw::get_polar_y(forward_offset, unit->current_direction_1) - weapons_dat::vertical_offset[weapon_id];
		}

		// Cloaked units - decloak
		// Reenable the conditional and check for special unit IDs if they are not meant to cloak. -Pr0nogo
	//	if (unit->id != UnitId::ProtossCabalist) {
			scbw::decloak_passive_proc(unit);
	//	}
		if (unit->id == UnitId::ProtossSteward && unit->hangar_unit.parent) {
			scbw::decloak_passive_proc(unit->hangar_unit.parent);
		}

		if (weapons_dat::flingy_id[weapon_id] != 0)
		{
			if (unit->id == UnitId::TerranSavant) {
				if (!scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit)) {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantEntropyFire01, SoundId::SavantEntropyFire05), unit, 1, 0);
				} else {
					helper::play_sound_from_unit(scbw::rand_between(SoundId::SavantSiphonFire01, SoundId::SavantSiphonFire05), unit, 1, 0);
				}
			}

			// Lakizilisk Seismic Spines behavior
			// Skithrokor behavior
			if (weapon_id_returns_to_attacker(weapon_id))
			{
				u32 timer = 21;
				if (weapon_id == WeaponId::SkithrokorSkitteringSpores) {
					timer = 9;
				}
				unit_ext->timers[TimerMissileReturnETA] = timer;
				u32 range = weapons_dat::max_range[weapon_id] + 16;
			//	u32 range = get_max_weapon_range(unit, weapon_id, unit->order_target.unit) + 16;
				Point16 unit_pos = { unit->position.x, unit->position.y };
				Point16 target_pos = { unit->order_target.unit->position.x, unit->order_target.unit->position.y };
				s32 angle = scbw::get_angle(unit_pos.x, unit_pos.y, target_pos.x, target_pos.y);
				unit_ext->values[ValueMissileReturnX] = unit_pos.x + scbw::get_polar_x(range, angle);
				unit_ext->values[ValueMissileReturnY] = unit_pos.y + scbw::get_polar_y(range, angle);
				if (unit->order_target.unit)
				{
					unit_ext->values[ValueMissileReturnTarget] = (u32)unit->order_target.unit;
				}
				if (weapon_id == WeaponId::LakiziliskSubterraneanSpines) {
					iquare_globals.clear_lakizilisk(unit);
				}
			}

			if (unit->id != UnitId::TerranWyvern
			&&	unit->id != UnitId::TerranWyvernTurret
			&&  unit->id != UnitId::ProtossAnthelion) // disable default weapon for custom behaviors
			{
				// Madrigal - Tempo Change weapon switch
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

				Target weapon_target = make_target(unit->order_target.unit);
				switch (weapon_id) {
					case WeaponId::CyprianTwinRailguns:
					case WeaponId::VultureFragmentationGrenade:
					case WeaponId::GoliathContenderMissiles:
					case WeaponId::PhalanxArcliteShockCannon:
					case WeaponId::RamessesPharoahCannons:
					case WeaponId::ClaymoreHarpoonRailgun:
					case WeaponId::PenumbraErebolShockCannon:
					case WeaponId::WraithBurstLasers:
					case WeaponId::SundogBlindsideRockets:
					case WeaponId::CentaurSunchaserMissiles:
					case WeaponId::MagnetarNewtonianShockCannon:
					case WeaponId::HippogriffChimeraCannons:
					case WeaponId::HippogriffPegasusMissiles:
					case WeaponId::SentinelCerberusRailgun:
					case WeaponId::DiademIonCannon:
					case WeaponId::DiademHammerfallBeacon:
					case WeaponId::WardenPrismaticCannon:
					case WeaponId::DracadinPhaseDisruptor:
					case WeaponId::CharlatanFerventBlaster:
					case WeaponId::OptectonSunsplitBeam:
					case WeaponId::PositronAntimatterMissile:
					case WeaponId::AccantorPlasmaShell:
					case WeaponId::PanoptusPlasmaMissiles:
					case WeaponId::ExemplarPlasmaTorpedo:
					case WeaponId::EpigraphLitanyCannons:
					case WeaponId::EmpyreanPlasmaBarrage:
					case WeaponId::StarSovereignCelestialCannon:
					case WeaponId::SpraithColonySeekerSpores:
					case WeaponId::LiiraliskHollowedSpores:
					case WeaponId::KalkaliskViscousAcid:
					case WeaponId::AxitriliskSearingVenom:
					case WeaponId::SkithrokorSkitteringSpores:
					case WeaponId::BactaliskCorrosiveAcid:
					case WeaponId::KeskathalorScreamingVenom:
					{
						fire_bullet(unit, weapon_id, weapon_target, get_weapon_offsets(weapon_id));
					} break;
					case WeaponId::PhobosArgosLaserBatteries:
					{
						// Phobos - Argent Capacitors passive
						if (unit->id == UnitId::TerranPhobos && unit_ext->values[ValuePhobosDischarge] == 0) {
							unit->add_unit_energy(5 * 256);
							if (unit->energy == unit->get_max_energy()) {
								unit_ext->values[ValuePhobosDischarge] = 1;
								unit->ground_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								unit->air_weapon_cooldown = get_modified_weapon_cooldown(unit, WeaponId::PhobosArgosLaserBatteries);
								helper::play_sound_from_unit(SoundId::PhobosArgentCapacitorsProc, unit, 1, 0);
								unit->sprite->create_overlay(ImageId::OpticalFlareHit_Large);
							}
						}
						fire_bullet(unit, weapon_id, weapon_target, get_weapon_offsets(weapon_id));
					} break;
					case WeaponId::HotshotDispatchShotgun: {
						// Offset: Cone
						fire_shotgun(unit, weapon_id, 1, 10, 10);
					} break;
					case WeaponId::SavantDiscoidRailgun: {
						// Entropy Gauntlets
						if (!scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit)) {
							helper::create_thingy(SpriteId::EntropyGauntletsOverlay, unit->position.x, unit->position.y, unit->player);
							fire_bullet(unit, WeaponId::SavantDiscoidRailgunPiercing, unit->order_target);
						} else {
							fire_bullet(unit, weapon_id, unit->order_target);
						}
					} break;
					case WeaponId::SouthpawKnockoutDrivers: {
						// Southpaw - Offset: Linear
						fire_line(unit, weapon_id, 3, 24, 1, 0);
					} break;
					case WeaponId::CataphractLonginiusCannons: {
						FireMultipleOptions options = {};
						options.weapon_offsets = get_weapon_offsets(weapon_id);
						options.stagger = -1;

						// Multi-Attack IV
						fire_multiple(unit, weapon_id, 4, FilterEnemy | FilterFrontalArc, options);
					} break;
					case WeaponId::PaladinCrusaderMissiles: {
						// Paladin - Offset: Linear
						fire_line(unit, weapon_id, 10, 32, 2, 16, get_weapon_offsets(weapon_id));
					} break;
					case WeaponId::AnticthonEncirclingMirror: {
						// Multi-Attack V
						fire_multiple(unit, weapon_id, 5, FilterEnemy, {});
					} break;
					case WeaponId::AionLazarusSalvo: {
						// Aion - Offset: Linear
						fire_line(unit, weapon_id, 4, 32, 1, 0);
					} break;
					case WeaponId::SalamanderPerditionBarrage: {
						// Salamander - Offset: Linear
						fire_line(unit, weapon_id, 3, 32, 1, 0);
					} break;
					case WeaponId::MinotaurAsterionLaserBattery: {
						// Monoceros Targeting - Minotaur
						fire_multiple(unit, weapon_id, 5, FilterEnemy | FilterFrontalArc, {});
					} break;
					case WeaponId::SirenCharybdalRifle: {
						// Offset: Cone
						fire_shotgun(unit, weapon_id, 2, 5, 10);
					} break;
					case WeaponId::MadrigalFuriaMissiles: {
						// Madrigal Furia Salvo
						FireMultipleOptions options = {};
						options.stagger = 2;
						fire_multiple(unit, WeaponId::MadrigalFuriaMissiles, 0, FilterEnemy, options);
					} break;
					case WeaponId::ValkyrieHaloRockets: {
						// Offset: Cone
						fire_shotgun(unit, weapon_id, 1, 3, 10);
					} break;
					case WeaponId::TosgriliskExplosiveSpores:
					case WeaponId::IzirokorEnergizedBioplasma:
					case WeaponId::KonviliskSowingSpines: {
						// Crushing Tide - Tosgrilisk, Izirokor, Konvilisk
						fire_multiple(unit, weapon_id, 3, FilterEnemy, {});
					} break;
					case WeaponId::MatrixKhaydarinRay: {
						FireMultipleOptions options = {};
						options.use_weapon_factor = false;
						options.stagger = -1;
						options.weapon_offsets = get_weapon_offsets(weapon_id);

						// Multi-Attack IV
						fire_multiple(unit, weapon_id, 4, FilterEnemy,  options);
					} break;
					case WeaponId::BarghestRayOfAgony: {
						FireMultipleOptions options = {};
						options.use_weapon_factor = false;
						options.stagger = -1;
						options.weapon_offsets = get_weapon_offsets(weapon_id);

						// Multi-Attack III
						fire_multiple(unit, weapon_id, 3, FilterEnemy | FilterFrontalArc, options);
					} break;
					case WeaponId::SkortrithColonySkortraTendril: {
						FireMultipleOptions options = {};
						options.weapon_offsets = get_weapon_offsets(weapon_id);
						options.ignore_main = true;
						options.use_weapon_factor = false;
						options.apply_cooldown = false;
						options.center_on_target = true;
						options.center_on_target_range_overwrite = 2 * 32;
						options.divisor_multiplier = 2;
						options.stagger = -1;

						fire_bullet(unit, weapon_id, unit->order_target);
						fire_multiple(unit, weapon_id, 2, FilterEnemy, options);
					} break;
					default: {
						create_bullet_ex(weapon_id, unit, x, y, unit->player, unit->current_direction_1);
					} break;
				}
			}
		}
	}

	static Box32 get_line_search_area(Bullet* bullet, Point16 old_position) {
		u32 search_margin = 6 * 32;
		Box32 search_area = {};
		search_area.left   = Min(old_position.x, bullet->position.x) - search_margin;
		search_area.top    = Min(old_position.y, bullet->position.y) - search_margin;
		search_area.right  = Max(old_position.x, bullet->position.x) + search_margin;
		search_area.bottom = Max(old_position.y, bullet->position.y) + search_margin;
		return search_area;
	}

	static bool apply_bullet_interception(Bullet* bullet, Point16 old_position, f32 line_width) {
		u32 intercept_type = weapon_id_intercepts(bullet->weapon_id);

		Unit* best_unit = 0;
		f32 best_t = INFINITY;
		Point16 best_cp = {};

		Box32 search_area = get_line_search_area(bullet, old_position);
		EntryArray units = {};
		unit_search_rect(&units, search_area);
		unit_search_filter(&units, 0);
		for (u32 i = 0; i < units.count; ++i) {
			Unit* unit = units.units[i];
			if (unit->status & UnitStatus::Invincible) {
				continue;
			}
			if (bullet->attack_target.unit != unit && scbw::is_allied_to(bullet->player, unit->player)) {
				continue;
			}
			if (intercept_type == InterceptType_GroundOnly) {
				if (unit->status & UnitStatus::InAir) {
					continue;
				}
				if (unit->status & UnitStatus::Burrowed && bullet->source_unit && !helper::unit_can_see_cloaked_target(unit, bullet->source_unit)) {
			//		continue;
				}
			} else if (intercept_type == InterceptType_AirOnly) {
				if (!(unit->status & UnitStatus::InAir)) {
					continue;
				}
			}

			f32 t = 0;
			Point16 cp = {};
			if (!scbw::line_intersects_rect(old_position, bullet->position, line_width, unit->get_box(), &t, &cp)) {
				continue;
			}
			if (t < best_t) {
				best_t = t;
				best_cp = cp;
				best_unit = unit;
			}
		}

		if (best_unit) {
			bullet->attack_target.pt = best_cp;
			bullet->attack_target.unit = best_unit;
			return true;
		}
		return false;
	}

	static void apply_bullet_effect(Bullet* bullet, Point16 cp, Unit* target) {
		switch (bullet->weapon_id) {
			// Same as default, but orange effect
			case WeaponId::CyprianTwinRailguns:
			case WeaponId::AutocratEminenceRifle:
			case WeaponId::MadrigalImperiosoMissiles:
			case WeaponId::RamessesPharoahCannons:
			case WeaponId::WyvernJacksonCannon:
			case WeaponId::MagnetarNewtonianShockCannon:
			case WeaponId::HippogriffChimeraCannons:
			{
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, cp.x, cp.y, bullet->player);
				hooks::weapon_bullet_hit(bullet, target, 1);
			} break;
			// Same as default, but red effect
			case WeaponId::SavantDiscoidRailgunPiercing:
			{
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::BurstLasersImpactRed_Damageless, cp.x, cp.y, bullet->player);
				hooks::weapon_bullet_hit(bullet, target, 1);
			} break;
			case WeaponId::PenumbraErebolShockCannon:
			{
				// Penumbra - Shroud Piercer
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, cp.x, cp.y, bullet->player);
				hooks::weapon_bullet_hit(bullet, target, bullet->attack_target.unit == target ? 1 : 2);
			} break;
			case WeaponId::ClaymoreHarpoonRailgun:
			{
				// Claymore - Kelvin Munitions
				// pierce targets
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::BurstLasersImpactDamageless, cp.x, cp.y, bullet->player);
				helper::play_sound_from_unit(scbw::rand_between(SoundId::ClaymoreWeaponHit01, SoundId::ClaymoreWeaponHit02), target, 1, 0);
				hooks::weapon_bullet_hit(bullet, target, 1);

				// slow pierced targets
				UnitExt* collision_target_ext = get_unit_ext(target);
				if (!collision_target_ext->timers[TimerKelvinMunitions]) {
					collision_target_ext->timers[TimerKelvinMunitions] = 3 * 24 + 1;
				}
			} break;
			case WeaponId::KalkaliskViscousAcid:
			{
				// Kalkalisk - Ablative Spores
				scbw::fire_incremental_bullet(bullet, bullet->weapon_id, target, cp);
			} break;
			case WeaponId::SkithrokorSkitteringSpores:
			{
				// Skithrokor - Ablative Whiplash
				scbw::fire_incremental_bullet(bullet, bullet->weapon_id, target, cp);
			} break;
			case WeaponId::ZarcavrokorBonesofReflection:
			{
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, cp.x, cp.y, bullet->player);
				hooks::weapon_bullet_hit(bullet, target, 1);
			} break;
			default:
			{
				iquare_globals.add_bullet_multihit(bullet, target);
				scbw::create_and_init_thingy(SpriteId::BurstLasersImpactBlue_Damageless, cp.x, cp.y, bullet->player);
				hooks::weapon_bullet_hit(bullet, target, 1);
			}
		}
	}

	struct UnitPiercingEntry {
		Unit* unit;
		Point16 cp;
		f32 t;
	};

	static s32 compare_unit_entry(const void* va, const void* vb) {
		UnitPiercingEntry* a = (UnitPiercingEntry*)va;
		UnitPiercingEntry* b = (UnitPiercingEntry*)vb;
		return a->t - b->t;
	}

	static bool apply_bullet_piercing(Bullet* bullet, Point16 old_position, f32 line_width) {
		u32 targets_count = 0;
		UnitPiercingEntry targets[32] = {};

		Box32 search_area = get_line_search_area(bullet, old_position);
		EntryArray units = {};
		unit_search_rect(&units, search_area);
		unit_search_filter(&units, 0);
		for (u32 i = 0; i < units.count; ++i) {
			Unit* unit = units.units[i];
			if (unit->status & UnitStatus::Invincible) {
				continue;
			}
			if (bullet->attack_target.unit != unit && scbw::is_allied_to(bullet->player, unit->player)) {
				continue;
			}
			if (!scbw::can_weapon_target_unit(bullet->weapon_id, unit)) {
				continue;
			}
			if (iquare_globals.was_already_hit_by_multihit_bullet(bullet, unit)) {
				continue;
			}
			if (bullet->hit_flags & 1) {
				continue;
			}

			f32 t = 0;
			Point16 cp = {};
			if (!scbw::line_intersects_rect(old_position, bullet->position, line_width, unit->get_box(), &t, &cp)) {
				continue;
			}

			if (targets_count < 32) {
				targets[targets_count++] = {unit, cp, t};
			} else {
				break;
			}
		}

		for (u32 i = 0; i < targets_count; ++i) {
			UnitPiercingEntry* target = &targets[i];
			bool stops_on_armor = weapon_id_piercing_stops_on_armor(bullet->weapon_id);
			if (stops_on_armor) {
				u32 armor = hooks::get_armor_internal(target->unit);
				u32 armor_pen = hooks::get_modified_armor_pen(bullet->source_unit, target->unit, bullet->weapon_id);
				if (armor >= armor_pen) {
					bullet->attack_target.pt = target->cp;
					bullet->attack_target.unit = target->unit;
					return true;
				}
			}
			apply_bullet_effect(bullet, target->cp, target->unit);
			
			if (stops_on_armor) {
				BulletExt* bullet_ext = get_bullet_ext(bullet);
				if (!bullet_ext->did_pierce) {
					bullet_ext->did_pierce = true;
					bullet_ext->divisor_multiplier *= 2;
				}
			}
		}
		return false;
	}

	void progress_bullet_flight(Bullet* bullet) { // 0048B3B0
		Point16 old_position = bullet->position;
		BulletExt* bullet_ext = get_bullet_ext(bullet);

		f32 z_climb = 0;
		switch (bullet->weapon_id) {
			case WeaponId::PhalanxArcliteShockCannon: {
				z_climb = 0.25;
			} break;
		}

		// Unsimulate Z
		if (bullet_ext->current_z_displacement) {
			bullet->position.y += bullet_ext->current_z_displacement;
			bullet->sprite->set_position(bullet->position.x, bullet->position.y);
			bullet_ext->current_z_displacement = 0;
		}

		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores) {
			if (!bullet->attack_target.unit) {
				u32 range = AlkajeliskParasiticSporesSeekRange;
				// TODO: Unit search
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

		// Simulate Z
		u32 old_distance_traveled = bullet_ext->distance_traveled;
		bullet_ext->distance_traveled += scbw::get_distance_fast(old_position, bullet->position);
		bullet_ext->time_traveled += 1;
		if (z_climb) {
			f32 distance_from_source = scbw::get_distance_fast(bullet_ext->bullet_source, bullet->position);
			f32 distance_to_target = scbw::get_distance_fast(bullet->position, bullet->target_position_1);
			f32 total_distance = distance_from_source + distance_to_target;
			f32 factor = distance_to_target / total_distance;

			bullet_ext->current_z_displacement = total_distance * z_climb * sin(factor * PI);
			bullet->position.y -= bullet_ext->current_z_displacement;
			bullet->sprite->set_position(bullet->position.x, bullet->position.y);
		}

		// Create explosions mid-flight
		if (bullet->weapon_id == WeaponId::PositronAntimatterMissile) {
			// Positron - Particle Annihilation
			u32 period = 1 * 32;
			if (bullet_ext->distance_traveled / period > old_distance_traveled / period) {
				scbw::create_and_init_thingy(SpriteId::AntiMatterMissileHit, bullet->position.x, bullet->position.y, bullet->player, bullet->sprite->elevation - 1);
				helper::play_sound_at_pos(scbw::rand_between(SoundId::PositronWeaponHit01, SoundId::PositronWeaponHit03), bullet->position, 1, 0);
				EntryArray targets = {};
				unit_search_radial(&targets, bullet->position, weapons_dat::outer_splash_radius[bullet->weapon_id]);
				unit_search_filter(&targets, 0, 0); // Let's filter with no flags, since that should remove untargetable stuff and what not
				for (u32 i = 0; i < targets.count; ++i) {
					Unit* target = targets.units[i];
					if (scbw::is_allied_to(bullet->player, target->player) || target == bullet->attack_target.unit) {
						continue;
					}
					u32 distance = scbw::get_distance_fast(bullet->position, target->position);
					u32 divisor = 1;
					if (distance <= weapons_dat::inner_splash_radius[bullet->weapon_id]) {
						divisor = 4;
					} else if (distance <= weapons_dat::medium_splash_radius[bullet->weapon_id]) {
						divisor = 2;
					}
					target->damage_with(bullet->weapon_id, -1, bullet->source_unit, bullet->player, bullet->current_direction_2, divisor);
				}
			}
		}

		u8 time = bullet->remove_timer--;
		if (bullet->weapon_id == WeaponId::AlkajeliskParasiticSpores) {
			if (bullet->remove_timer < AlkajeliskParasiticSporesTimer && !bullet->attack_target.unit) {
				time = 0;
			}
		}

		// Accelerate missiles over time
		if (weapon_id_accelerates(bullet->weapon_id)) {
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
						s32 new_angle = scbw::get_angle(bullet->position.x, bullet->position.y, target->position.x, target->position.y);
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

		bool despawn_bullet = false;
		bool despawn_bullet_special = false;
		if (time == 0 || (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y)) {
			despawn_bullet = true;
		}
		if (weapon_id_intercepts(bullet->weapon_id)) {
			f32 line_width = 16;
			switch (bullet->weapon_id) {
				// add special line_width cases here
			}

			if (apply_bullet_interception(bullet, old_position, line_width)) {
				despawn_bullet = true;
				despawn_bullet_special = true;
			}
		} else if (weapon_id_pierces(bullet->weapon_id)) {
			f32 line_width = 16;
			switch (bullet->weapon_id) {
				case WeaponId::WyvernJacksonCannon:
				{
					line_width = 24;
				} break;
				case WeaponId::SavantDiscoidRailgunPiercing:
				case WeaponId::ClaymoreHarpoonRailgun:
				case WeaponId::SentinelCerberusRailgun:
				{
					line_width = 32;
				} break;
				case WeaponId::PenumbraErebolShockCannon:
				case WeaponId::MagnetarNewtonianShockCannon:
				case WeaponId::HippogriffChimeraCannons:
				{
					line_width = 48;
				} break;
			}

			if (apply_bullet_piercing(bullet, old_position, line_width)) {
				despawn_bullet = true;
				despawn_bullet_special = true;
			}
		}

		if (despawn_bullet) {
			Unit* target = bullet->attack_target.unit;
			bullet->behavior_type = 5;
			if (!despawn_bullet_special) {
				if (target && !target->is_dead() && weapons_dat::behavior[bullet->weapon_id] != WeaponBehavior_GoToMaxRange) {
					bullet->attack_target.pt.x = target->position.x;
					bullet->attack_target.pt.y = target->position.y;
				} else {
					bullet->attack_target.unit = 0;
				}
			}
			bullet->order_signal |= 0x80; // new
			bullet->main_order_state = 0;
			bullet->order_unit_id = 228;
			for (Image* image = bullet->sprite->images.head; image; image = image->link.next) {
				helper::play_image_iscript(image, IscriptAnimation::Death);
			}
		}
	}

	void bullet_behavior_bounce(Bullet* bullet) { // 0048B2D0
		Unit* target_unit = bullet->attack_target.unit;
		if (target_unit && !target_unit->is_dead()) {
			if (!(bullet->hit_flags & 1)) {
				helper::change_move_pos(bullet, target_unit->position.y, target_unit->position.x);
			}
		}

		Point16 old_position = bullet->position;
		helper::progress_bullet_movement(bullet);

		BulletExt* bullet_ext = get_bullet_ext(bullet);
		u32 old_distance_traveled = bullet_ext->distance_traveled;
		bullet_ext->distance_traveled += scbw::get_distance_fast(old_position, bullet->position);
		bullet_ext->time_traveled += 1;
		if (weapon_id_bounces_caustic_fission(bullet->weapon_id) && !bullet_ext->bounced) {
			u32 period = 4 * 32;
			if (bullet_ext->distance_traveled / period > old_distance_traveled / period) {
				bullet->remaining_bounces += 1;
			}
		}

		if (bullet->move_target.pt.x == bullet->position.x && bullet->move_target.pt.y == bullet->position.y) {
			bullet_ext->bounced = true;
			u8 next_bounce = bullet->remaining_bounces - 1;
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
				bullet->behavior_type = 5;
				if (target_unit) {
					bullet->attack_target.pt.x = target_unit->position.x;
					bullet->attack_target.pt.y = target_unit->position.y;
				} else {
					bullet->attack_target.unit = 0;
				}
				Sprite* sprite = bullet->sprite;
				bullet->main_order_state = 0;
				bullet->order_unit_id = UnitId::None;

				for (Image* image = sprite->images.head; image; image = image->link.next) {
					helper::play_image_iscript(image, IscriptAnimation::Death);
				}
			}
		}
	}

	bool find_next_bounce_target_proc(Unit* target) // 0048AFD0
	{
		Unit* bounce_source = *g_bullet_bounce_missile_source;

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
		if (*g_bullet_bounce_missile_source && helper::unit_can_see_cloaked_target(target, (*g_bullet_bounce_missile_source)))
		{
			return 0;
		}

		Unit* bounce_source_unit = bounce_source;
		if (bounce_source && bounce_source->subunit && helper::unit_is_subunit(bounce_source->subunit))
		{
			bounce_source_unit = bounce_source->subunit;
		}
		if (!(helper::unit_is_carrier(bounce_source)))
		{
			bool is_valid = true;
			if (helper::unit_is_reaver(bounce_source))
			{
				if (target->status & UnitStatus::InAir)
				{
					return 0;
				}
				is_valid = helper::unit_has_path_to_target(bounce_source, target);
			} else {
				if (helper::unit_can_infest(bounce_source))
				{
					is_valid = helper::can_be_infested(0, target) != 0; // First arg is stupid?
				} else {
					if (target->status & UnitStatus::InAir)
					{
						is_valid = units_dat::air_weapon[bounce_source_unit->id] != WeaponId::None;
					} else {
						is_valid = units_dat::ground_weapon[bounce_source_unit->id] != WeaponId::None;
					}
				}
			}
			if (!is_valid)
			{
				return 0;
			}
		}

		if (helper::unit_can_see_cloaked_target(target, bounce_source)) {
			return 0;
		}

		u8 player = target->player == 11 ? target->sprite->player : target->player;
		if (scbw::is_allied_to(bounce_source->player, player) || *g_bullet_bounce_missile_target == target)
		{
			return 0;
		}

		// Gladius Binding Plasma behavior
		if (iquare_globals.bounce_bullet->weapon_id == WeaponId::GladiusPlasmaFlurry)
		{
			if (target && *g_bullet_bounce_missile_next && (*g_bullet_bounce_missile_next)->sprite && target->sprite)
			{
				if (iquare_globals.was_already_hit_by_multihit_bullet(iquare_globals.bounce_bullet, target))
				{
					return false;
				} else {
					iquare_globals.add_bullet_multihit(iquare_globals.bounce_bullet, target);
					return *g_bullet_bounce_missile_next != target;
				}
			}
		}
		return *g_bullet_bounce_missile_next != target;
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
		*g_bullet_bounce_missile_target = target;
		u16 pos_y = bullet->position.x;
		u16 pos_x = bullet->position.y;
		*g_bullet_bounce_missile_next = next_bounce;
		Box16 coords;
		coords.left = pos_x - 96;
		coords.top = pos_x + 96;
		coords.right = pos_y - 96;
		coords.bottom = pos_y + 96;
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
			} else {
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
		} else {
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
		bullet->order_unit_id = UnitId::None;
		bullet->main_order_state = 0;
		bullet->attack_target.unit = 0;
		bullet->attack_target.pt.x = 0;
		bullet->attack_target.pt.y = 0;
		bullet->order_signal = 0;
		bullet->main_order_timer = 0;
		bullet->weapon_id = weapon_id;
		bullet->remove_timer = weapons_dat::remove_after[weapon_id];
		bullet->order_unit_id = UnitId::None;
		bullet->remaining_bounces = 0;
		bullet->hit_flags = 0;
		bullet->movement_flags |= MovementFlags_StartingAttack;
		bullet->behavior_type = 0;
		bullet->attack_target.unit = 0;
		bullet->attack_target.pt.y = 0;
		bullet->attack_target.pt.x = 0;
		bullet->main_order_state = 0;
		helper::bullet_behavior_reassign(bullet);
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
			char launch_spin_direction = launch_spin_variance + bullet->velocity_direction_1;
			bullet->current_direction_1 = launch_spin_direction;
			bullet->velocity_direction_1 = launch_spin_direction;
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
		bullet->attack_target.unit = target_unit;
		if (target_unit && target_unit->sprite) {
			bullet->sprite->elevation = target_unit->sprite->elevation + 1;
			bullet->attack_target.pt.x = target_unit->position.x;
			bullet->attack_target.pt.y = target_unit->position.y;
		} else {
			bullet->sprite->elevation = parent->sprite->elevation + 1;
			bullet->attack_target.pt.x = parent->order_target.pt.x;
			bullet->attack_target.pt.y = parent->order_target.pt.y;
		}

		switch (weapons_dat::behavior[weapon_id]) {
			case WeaponBehavior_FlyDoNotFollowTarget:
			case WeaponBehavior_FlyFollowTarget:
			{
				if (target_unit) {
					if (parent) {
						helper::weapon_behavior_can_hit_target_check(target_unit, parent);
					}
				}
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
			case WeaponBehavior_AppearOnTargetUnit:
			case WeaponBehavior_AppearOnTargetSite:
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
			case WeaponBehavior_PersistOnTargetSite:
			{
				helper::appear_on_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);
				return 1;
			}
			case WeaponBehavior_AppearOnAttacker:
			{
				return 1;
			}
			case WeaponBehavior_AttackAndSelfDestruct:
			{
				if (!bullet->source_unit) {
					return 1;
				}
				bullet->source_unit->status |= UnitStatus::IsSelfDestructing;
				bullet->source_unit->user_action_flags |= 0x4;
				helper::orders_self_destructing(bullet->source_unit);
				return 1;
			}
			case WeaponBehavior_Bounce:
			{
				switch (weapon_id) {
					case WeaponId::BactaliskCorrosiveAcid:
					case WeaponId::AlkajeliskPlasmidSpores:
					{
						bullet->remaining_bounces = 1; // Additional bounces added later. -Veeq7
					} break;
					case WeaponId::IzirokorEnergizedBioplasma:
					case WeaponId::TosgriliskExplosiveSpores:
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
			case WeaponBehavior_AttackNearbyArea:
			{
				bullet->cyclic_missile_index = parent->offset_index_3_by_3;
				bullet->attack_target.pt.x -= (s16)g_attack_curve[parent->offset_index_3_by_3].x;
				bullet->attack_target.pt.y -= (s16)g_attack_curve[parent->offset_index_3_by_3].y;
				helper::attack_fly_to_target(bullet, bullet->attack_target.pt.x, bullet->attack_target.pt.y);

				s16 final_offset = parent->offset_index_3_by_3 + 1;
				if (!g_attack_curve[final_offset].x) {
					final_offset = 0;
				}
				parent->offset_index_3_by_3 = final_offset;
				return 1;
			}
			case WeaponBehavior_GoToMaxRange:
			{
				s32 max_range_var = (weapons_dat::max_range[bullet->weapon_id] + 20) * 256;
			//	s32 max_range_var = get_max_weapon_range(parent, weapon_id, parent->order_target.unit) + 16;
				Vector32 target_pos = {};
				target_pos.x = (s32)(parent->position.x + ((max_range_var * g_angle_distance[bullet->current_direction_1].x) >> 16));
				target_pos.y = (s32)(parent->position.y + ((max_range_var * g_angle_distance[bullet->current_direction_1].y) >> 16));
				if (target_pos.x < 0) {
					target_pos.y = (1 - abs(target_pos.x / (target_pos.x - target_unit->position.x))) * (target_pos.y - target_unit->position.y) + target_unit->position.y;
					target_pos.x = 0;
				}
				if (target_pos.y < 0) {
					target_pos.x = (1 - abs(target_pos.y / (target_pos.y - target_unit->position.y))) * (target_pos.x - target_unit->position.x) + target_unit->position.x;
					target_pos.y = 0;
				}
				bullet->attack_target.pt.x = target_pos.x;
				bullet->attack_target.pt.y = target_pos.y;
				helper::attack_fly_to_target(bullet, target_pos.x, target_pos.y);
				return 1;
			}
		}
		return 0;
	}

	// Hooked to investigate weapons.dat extender crashes. Disabled! - Pr0nogo
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
			u32 bullet_behavior = (u8)bullet->behavior_type;
			switch (bullet_behavior)
			{
				case 0:
				{
					helper::bullet_behavior_reassign(bullet);
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

	// Hooked to investigate first_free_bullet nonsense. - Pr0nogo
	void bullet_behavior_reassign(Bullet* bullet) { // 0048BD50
		while (2) {
			if (bullet->order_signal & 1) {
				bullet->order_signal = bullet->order_signal & 0xFE;
				Image* bullet_overlay = 0;
				switch (weapons_dat::behavior[bullet->weapon_id]) {
					case WeaponBehavior_AppearOnTargetUnit:
					case WeaponBehavior_AppearOnTargetSite:
					case WeaponBehavior_AppearOnAttacker:
					case WeaponBehavior_AttackAndSelfDestruct:
					{
						bullet->behavior_type = WeaponBehavior_AppearOnAttacker;
						if (bullet->attack_target.unit && bullet->attack_target.unit->sprite) {
							bullet->attack_target.pt.x = bullet->attack_target.unit->position.x;
							bullet->attack_target.pt.y = bullet->attack_target.unit->position.y;
						} else {
							bullet->attack_target.unit = 0;
						}
						bullet->main_order_state = 0;
						bullet->order_unit_id = UnitId::None;
						helper::play_sprite_iscript(bullet->sprite, IscriptAnimation::Death, 1);
					} break;
					case WeaponBehavior_FlyFollowTarget:
					{
						helper::assign_weapon_data(bullet, WeaponBehavior_AppearOnTargetUnit);
						helper::play_sprite_iscript(bullet->sprite, IscriptAnimation::GndAttkInit, 1);
					}  break;
					case WeaponBehavior_Bounce:
					{
						helper::assign_weapon_data(bullet, WeaponBehavior_PersistOnTargetSite);
						helper::play_sprite_iscript(bullet->sprite, IscriptAnimation::GndAttkInit, 1);
					}  break;
					case WeaponBehavior_PersistOnTargetSite:
					{
						helper::assign_weapon_data(bullet, WeaponBehavior_AppearOnTargetSite);
						Image* bullet_overlay = 0;
						bullet_overlay = bullet->sprite->images.head;
						if (bullet_overlay) {
							do {
								helper::play_image_iscript(bullet_overlay, IscriptAnimation::SpecialState2);
								bullet_overlay = bullet_overlay->link.next;
							} while (bullet_overlay);
						}
					} break;
					case WeaponBehavior_AttackNearbyArea:
					{
						helper::assign_weapon_data(bullet, WeaponBehavior_AttackAndSelfDestruct);
						Image* bullet_overlay = bullet->sprite->images.head;
						if (bullet_overlay) {
							do {
								helper::play_image_iscript(bullet_overlay, IscriptAnimation::GndAttkInit);
								bullet_overlay = bullet_overlay->link.next;
							} while (bullet_overlay);
						}
					} break;
					default:
					{
						helper::assign_weapon_data(bullet, WeaponBehavior_FlyFollowTarget);
						Image* bullet_overlay = bullet->sprite->images.head;
						if (bullet_overlay) {
							do {
								helper::play_image_iscript(bullet_overlay, IscriptAnimation::GndAttkInit);
								bullet_overlay = bullet_overlay->link.next;
							}
							while (bullet_overlay);
						}
					} break;
				}

				switch (bullet->behavior_type) {
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
			} else {
				if (unit->id != UnitId::ZergLakizilisk || unit->status & UnitStatus::Burrowed)
				{
					helper::use_weapon_iscript(unit, units_dat::ground_weapon[unit->id], 1, iscript_anim);
				} else {
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

		unit->movement_flags |= MovementFlags_StartingAttack;

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

	// Hooked to fix melee edge unit performance. -Pr0nogo
	b32 should_stay_at_range(Unit* unit) { // 00475A10
		if (unit_id_is_melee_air_unit(unit->id)) {
			return false;
		}
		u32 weapon_id = units_dat::ground_weapon[unit->id];
		if (weapon_id == WeaponId::None) {
			return true;
		} else {
			return weapons_dat::flingy_id[weapon_id] != 0;
		}
	}

	// Hooked to fix melee edge unit performance. -Pr0nogo
	void move_for_melee_range(Unit* target, Unit* attacker) // 00477F10
	{
		if (!helper::is_target_visible(attacker, target) || !(target->movement_flags & MovementFlags_Accelerating))
		{
			helper::move_towards(attacker, target);
			return;
		}

		s32 tgt_x = target->position.x;
		s32 tgt_y = target->position.y;

		s32 attacker_halt = helper::get_flingy_halt_distance(attacker);
		s32 target_vel = target->velocity_direction_1;
		s32 attacker_halt_off1 = 3 * attacker_halt;
		s32 attacker_halt_off2 = 3 * attacker_halt * g_angle_distance[target_vel].x;
		u32 tgt_x_final = tgt_x + (attacker_halt_off2 >> 16);
		u32 tgt_y_final = tgt_y + (attacker_halt_off1 * g_angle_distance[target_vel].y >> 16);

		if (tgt_x_final >= *g_map_width_pixels || tgt_y_final >= *g_map_height_pixels)
		{
			tgt_y_final = tgt_y;
		} else {
			tgt_x_final = tgt_x;
		}
		// Originally delayed the chasing using order timer. -Pr0nogo
		helper::set_move_target_xy(attacker, tgt_x_final, tgt_y_final);
	}

	// Hooked to safeguard from nullsprites. -Pr0nogo
	Bullet* assign_weapon_data(Bullet* bullet, char behavior) // 0048AA80
	{
		Unit* target = bullet->attack_target.unit;
		bullet->behavior_type = behavior;
		if (target)
		{
			bullet->attack_target.pt.x = target->position.x; // Was sprite
			bullet->attack_target.pt.y = target->position.y; // ^
		} else {
			bullet->attack_target.unit = 0;
		}
		bullet->main_order_state = 0;
		bullet->order_unit_id = UnitId::None;
		return bullet;
	}

	// Hooked to nullcheck sprites. -Pr0nogo
	void bullet_behavior_follow(Bullet* bullet) // 0048B500
	{
		Unit* target = bullet->attack_target.unit;
		if (!target || target->is_dead())
		{
			bullet->behavior_type = 1;
			bullet->attack_target.unit = 0;
			bullet->main_order_state = 0;
			bullet->order_unit_id = UnitId::None;
			helper::bullet_behavior_fly(bullet);
		} else {
			if (!(bullet->hit_flags & 1)) // miss
			{
				Bullet* changed_bullet = helper::change_move_pos(bullet, target->position.y, target->position.x);
				helper::bullet_behavior_fly(changed_bullet);
			} else {
				bullet->behavior_type = 1;
				bullet->attack_target.pt.x = target->position.x;
				bullet->attack_target.pt.y = target->position.y;
				bullet->main_order_state = 0;
				bullet->order_unit_id = UnitId::None;
				helper::bullet_behavior_fly(bullet);
			}
		}
	}


}