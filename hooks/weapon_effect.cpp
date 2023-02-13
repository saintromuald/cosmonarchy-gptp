#include "weapon_effect.h"
#include <SCBW/UnitFinder.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "globals.h"
#include "weapons/weapon_damage.h"
#include "limits/ccmu.h"

namespace
{
	// SC internal data for Lakizilisk spines
	struct SpineHitTable
	{
		struct
		{
			Unit* source;
			Unit* target;
		} hits[32][16];
	};
	SpineHitTable* const spine_hit_table = (SpineHitTable*)(0x0064DEC8);
	u32* const spine_hit_row = (u32*)(0x64EEC8);
	u32* const spine_hit_column = (u32*)(0x64DEA8);
}

namespace hooks {
	void carapace_swarm_damage(u16 x, u16 y, u16 r, Bullet* bullet, u32 weapon_id) {
		Unit* swarm = scbw::get_dark_swarm_from_position(x, y, r);
		if (swarm) {
			u32 damage = (weapons_dat::damage_amount[weapon_id] * 256) +
				(weapons_dat::armor_pen[weapon_id] * 256 * scbw::get_upgrade_level(bullet->player, weapons_dat::damage_upgrade[weapon_id]));
			if (bullet->hit_flags & 2) {
				damage /= 2;
			}
			if (damage >= swarm->hitpoints) {
				swarm->remove();
			} else {
				u32 old = swarm->hitpoints;
				swarm->hitpoints -= damage;
			}
		}
	}

	void splash(Bullet* bullet, u32 weapon_id, u8 splash_type, Unit* guaranteed_hit) {
		s32 splash = splash_type;
		if (!bullet) {
			return;
		}
		Point16 pos = bullet->sprite->position;
		Unit* unit = bullet->source_unit;
		if (unit && unit->sprite) {
			UnitExt* bullet_source_ext = get_unit_ext(unit);
			if (unit->is_blind) {
				splash = SplashType::Radial;
			}

			// Create effect fields
			switch (weapon_id) {
				case WeaponId::IonCannon: {
					globals.create_effect(EffectId_IonField, 10 * 24, {pos.x, pos.y}, 3 * 32, SpriteId::IonField, 5, bullet->player, unit);
				} break;
				case WeaponId::AionLazarusSalvo: {
					globals.create_effect(EffectId_ReclamationField, 5 * 24, {pos.x, pos.y}, 2 * 32, SpriteId::CausticDischarge, 5, bullet->player, unit);
				} break;
				case WeaponId::SalamanderPerditionBarrage: {
					globals.create_effect(EffectId_SalamanderFire, 5 * 24, {pos.x, pos.y}, 0.5 * 32, SpriteId::SalamanderImpact, 5, bullet->player, unit);
				} break;
				case WeaponId::CantavisHallucinationCast: {
					globals.create_effect(EffectId_HallucinatingField, 10 * 24, {pos.x, pos.y}, 1.5 * 32, SpriteId::ProtossHallucinationEffect, 5, bullet->player, unit);
				} break;
				case WeaponId::KeskathalorScreamingVenom: {
					globals.create_effect(EffectId_CausticDischarge, 5 * 24, {pos.x, pos.y}, 1.5 * 32, SpriteId::CausticDischarge, 5, bullet->player, unit);
				} break;
			}
		}

		s16 inner = weapons_dat::inner_splash_radius[weapon_id];
		s16 medium = weapons_dat::medium_splash_radius[weapon_id];
		s16 outer = weapons_dat::outer_splash_radius[weapon_id];

		if (guaranteed_hit) {
			helper::weapon_bullet_shot(guaranteed_hit, bullet, 1);
		}
		carapace_swarm_damage(pos.x, pos.y, outer, bullet, weapon_id);

		if (!outer) {
			return;
		}

		EntryArray units_in_splash = {};
		unit_search_radial(&units_in_splash, pos, outer);
		unit_search_filter(&units_in_splash, unit);
		for (s32 i = 0; i < units_in_splash.count; i++) {
			Unit* target = units_in_splash.units[i];
			if (guaranteed_hit && target == guaranteed_hit) { // skips target check if has_guaranteed_hit_unit is unset, just for little bit of performance
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves -- except psi storm
			if (unit == target && bullet->weapon_id != WeaponId::PsiStorm) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player (also checks if type is 'splash enemy' or 'nuclear missile')
			// if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;

			if (splash == SplashType::Enemy) {
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			} else if (splash == SplashType::FriendlyNew) {
				if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;
			}

			// Can the unit be targeted?
			if (!scbw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			if (!helper::is_unit_distance_within(target, bullet, outer)) continue;

			// Don't storm a unit that's already being stormed
			if (bullet->weapon_id == WeaponId::PsiStorm) {
				if (target->is_under_storm) continue;
				target->is_under_storm = 1;
			}

			if (helper::is_unit_distance_within(target, bullet, inner)) {
				helper::weapon_bullet_shot(target, bullet, 1); // 100% damage
				continue;
			}

			if (helper::is_unit_distance_within(target, bullet, medium)) {
				helper::weapon_bullet_shot(target, bullet, 2); // 50% damage
				continue;
			}

			helper::weapon_bullet_shot(target, bullet, 4); // 25% damage
		}
	}

	void lakizilisk_splash(Bullet* bullet, u32 weapon_id, u8 splash) {
		s16 x = bullet->sprite->position.x;
		s16 y = bullet->sprite->position.y;
		s16 r = weapons_dat::outer_splash_radius[weapon_id];

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++) {
			Unit* target = unitsInSplash.get_unit(i);

			// Units can't splash themselves
			if (target == bullet->source_unit) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player
//			if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;
			if (splash == SplashType::Enemy) {
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			} else if (splash == SplashType::FriendlyNew) {
				bool lurkerOverride = false;
				if (bullet->source_unit) {
					UnitExt* bulletSourceExt = get_unit_ext(bullet->source_unit);
					if ((Unit*)bulletSourceExt->values[ValueLurkerPickTarget] == target) {
						lurkerOverride = true;
					}
				}
					
				if (scbw::is_unit_enemy(bullet->player, target) == false && !lurkerOverride && target != bullet->attack_target.unit) {
					continue;
				}
			}

			// Can the unit be targeted?
			if (!scbw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[bullet->weapon_id])) {
				continue;
			}

			if (bullet->source_unit) {
				bool skip = false;
				if (globals.spine_pair_exists(bullet->source_unit, target)) {
					skip = true;
					break;
				}
				if (skip) continue;
				globals.add_lakizilisk_spine_pair(bullet->source_unit, target);
			}
			helper::weapon_bullet_shot(target, bullet, 1);
		}
	}

	// clone of 'Normal' damage for reference
	void normal_damage(Bullet* bullet, u32 weapon_id) {
		if (scbw::weapon_is_melee_with_aoe(bullet, weapon_id)) {
			splash(bullet, weapon_id, SplashType::FriendlyNew, bullet->attack_target.unit);
			return;
		}

		Unit* target = bullet->attack_target.unit;
		if (!target) {
			return;
		}

		carapace_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);

		// Kalkalisk behavior
		if (bullet->weapon_id == WeaponId::KalkaliskViscousAcid) {
			globals.add_incremental_bullet_multihit(bullet, target);
			auto count = globals.get_multihit_count(bullet);
			target->damage_with(WeaponId::KalkaliskViscousAcid, (weapons_dat::damage_amount[WeaponId::KalkaliskViscousAcid] - count) * 256, bullet->source_unit, bullet->player);
			return;
		}

		u8 divisor = 1;
		if (weapon_id == WeaponId::ArchitectDispersiveSalvo) {
			divisor = globals.get_divider(bullet);
		} else if (weapon_id == WeaponId::GeszithalorAcidSpore) {
			// Searing Acid - Geszithalor behavior
			BulletExt* bullet_ext = get_bullet_ext(bullet);
			divisor = bullet_ext->distance_traveled / (32 * 3);
		}

		if (bullet->hit_flags & 1) { // Miss
			if (!target->unit_ai) {
				return;
			}
			if (!bullet->source_unit) {
				return;
			}
			helper::do_weapon_hit(target, bullet->source_unit, divisor);
			return;
		} else {
			if (weapons_dat::behavior[weapon_id] == WeaponBehavior::Bounce) {
				if (bullet) {
					if (bullet->source_unit && bullet->weapon_id == WeaponId::BactaliskCorrosiveAcid) {
						divisor = 1;
					} else {
						const s32 bounce_count = 2;
						divisor = bounce_count - bullet->remaining_bounces;
					}
				}
			}
			helper::weapon_bullet_shot(target, bullet, divisor);
		}
	}

	// case WPNEffect_Splash_Air:
	void air_splash(Bullet* bullet, u32 weapon_id) // 0048B790
	{
		Unit* target = bullet->attack_target.unit;
		if (!target)
		{
			return;
		}
		s16 radius = weapons_dat::outer_splash_radius[weapon_id];
		Sprite* bulletSprite = bullet->sprite;
		s16 posX = bulletSprite->position.x;
		s16 posY = bulletSprite->position.y;
		s32 list;
		Box16* search_radius = {};

		*g_air_splash_list_count = 0;
		search_radius->left = posX - radius;
		search_radius->top = posY - radius;
		search_radius->right = radius + posX;
		search_radius->bottom = radius + posY;

		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::outer_splash_radius[weapon_id];
		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		carapace_swarm_damage(x, y, r, bullet, weapon_id);

		s32 result;
		for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
		{
			// airSplashProc
			s8 weaponIdLoop;
			s32 listCount;
			Unit* target = unitsInSplash.get_unit(i);

			weaponIdLoop = bullet->weapon_id;
			if (target != bullet->source_unit && (target->player != bullet->player || target == bullet->attack_target.unit))
			{
				if (scbw::can_weapon_target_unit(weaponIdLoop, target, target))
				{
					if (helper::is_unit_distance_within(target, bullet, weapons_dat::outer_splash_radius[weaponIdLoop]))
					{
						if (helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[weaponIdLoop])
							&& target == bullet->attack_target.unit)
						{
							if (globals.extended_unit_table)
							{
								new_unit_table.air_splash_results[0] = target;
							}
							else
							{
								g_air_splash_list[0] = target;
							}
							result = 1;
							*g_air_splash_list_count = 1;
							break;
						}
						listCount = *g_air_splash_list_count;
						if (globals.extended_unit_table)
						{
							new_unit_table.air_splash_results[*g_air_splash_list_count] = target;
						}
						else
						{
							g_air_splash_list[*g_air_splash_list_count] = target;
						}
						*g_air_splash_list_count = listCount + 1;
					}
				}
			}
		}

		if (g_air_splash_list_count)
		{
			if (*g_air_splash_list_count == 1)
			{
				list = 0;
			}
			else
			{
				list = helper::randomize_short(58) % *g_air_splash_list_count;
			}
			if (globals.extended_unit_table)
			{
				helper::weapon_bullet_shot(target, bullet, (s32)new_unit_table.air_splash_results[list]);
			}
			else
			{
				helper::weapon_bullet_shot(target, bullet, (s32)g_air_splash_list[list]);
			}
		}
		// airSplashProc2
		splash(bullet, weapon_id, SplashType::FriendlyNew);
	}

	void nuke(Bullet* bullet, u32 weapon_id) {
		globals.create_effect(EffectId_GroundZero, 24, bullet->position, 0, -1, 5, bullet->player, bullet->source_unit);

		splash(bullet, weapon_id, SplashType::Radial);
	}

	// clone of 'Yamato' damage for reference
	void yamato_damage(Bullet* bullet, u32 weapon_id)
	{
		if (!bullet->attack_target.unit)
		{
			return;
		}
		carapace_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);
		helper::weapon_bullet_shot(bullet->attack_target.unit, bullet, 1);
	}

	void parasite(Bullet* bullet, u32 weapon_id)
	{
		if (!bullet->source_unit)
		{
			return;
		}
		constexpr u32 radius = 96;
		scbw::UnitFinder(
			bullet->position.x - radius / 2,
			bullet->position.y - radius / 2,
			bullet->position.x + radius / 2,
			bullet->position.y + radius / 2
		).for_each([&](Unit* unit)
		{
			scbw::apply_parasite(unit, bullet->source_unit->player, bullet->source_unit);
		});
	}

	void broodlings(Bullet* bullet, u32 weapon_id)
	{
		if (!bullet->source_unit)
		{
			return;
		}
		Unit* target = bullet->attack_target.unit;
		hooks::broodling_hit(bullet->source_unit, target, bullet);
	}

	void lockdown(Bullet* bullet, u32 weapon_id)
	{
		auto targPos = bullet->targetPosition;
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::outer_splash_radius[WeaponId::LobotomyMineExplosion];
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
		{
			Unit* target = unitsInSplash.get_unit(i);
			if (!target->is_dead()
				&& !units_dat::group_flags[target->id].is_building
				&& !(units_dat::base_property[target->id] & UnitProperty::ResourceContainer)
				&& target->id != UnitId::TerranSpiderMine)
			{
				hooks::lockdown_hit(target, LobotomyMineLockdownTimer, bullet);
			}
		}
	}
}