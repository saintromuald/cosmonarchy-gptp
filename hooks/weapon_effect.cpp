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
	// SC internal data for Lurker spines
	struct SpineHitTable
	{
		struct
		{
			Unit* source;
			Unit* target;
		} hits[32][16];
	};
	SpineHitTable* const spineHitTable = (SpineHitTable*)(0x0064DEC8);
	u32* const spineHitRow = (u32*)(0x64EEC8);
	u32* const spineHitCol = (u32*)(0x64DEA8);
}

namespace hooks
{
	void dark_swarm_damage(u16 x, u16 y, u16 r, Bullet* bullet, u32 weapon_id)
	{
		Unit* swarm = scbw::get_dark_swarm_from_position(x, y, r);
		if (swarm)
		{
			u32 damage = (weapons_dat::damage_amount[weapon_id] * 256) +
				(weapons_dat::armor_pen[weapon_id] * 256 * scbw::get_upgrade_level(bullet->player, weapons_dat::damage_upgrade[weapon_id]));
			if (bullet->hit_flags & 2)
			{
				damage /= 2;
			}
			if (damage >= swarm->hitpoints)
			{
				swarm->remove();
			}
			else
			{
				u32 old = swarm->hitpoints;
				swarm->hitpoints -= damage;
			}
		}
	}

	bool nathrokor_condition(Bullet* bullet, Unit* target)
	{
		if (!bullet->source_unit || !target)
		{
			return true;
		}
		if (bullet->weapon_id == WeaponId::NathrokorPlasmidBreath)
		{
			Unit* nathrokor = bullet->source_unit;
			u16 angle = scbw::get_angle(nathrokor->position.x, nathrokor->position.y,
				target->position.x, target->position.y);
			u16 unit_angle = nathrokor->velocity_direction_1;
			s32 arc = (f64)15 / 1.40625;
			// 30 degrees
			s32 delta = unit_angle - angle;
			delta = abs((delta + 128) % 256 - 128);
			if (delta > arc)
			{
				return false;
			}
			else
			{
			}
		}
		return true;
	}

	void splash(Bullet* bullet, u32 weapon_id, u8 splashType, Unit* guaranteedHit)
	{
		s32 splash = splashType;
		if (!bullet)
		{
			return;
		}
		if (bullet->source_unit && bullet->source_unit->sprite)
		{
			UnitExt* bulletSourceExt = get_unit_ext(bullet->source_unit);

			if (bullet->source_unit->is_blind)
			{
				splash = SplashType::Radial;
			}

			Point16 hit = bullet->position;
			switch (weapon_id)
			{
				case WeaponId::SalamanderPerditionBarrage:
				{
					globals.create_effect(EffectId_SalamanderFire, 5 * 24, {hit.x, hit.y}, 0.5 * 32, SpriteId::SalamanderImpact, 5, bullet->player, bullet->source_unit);
					break;
				}
				case WeaponId::KeskathalorScreamingVenom:
				{
					globals.create_effect(EffectId_CausticDischarge, 5 * 24, {hit.x, hit.y}, 1.5 * 32, SpriteId::CausticDischarge, 5, bullet->player, bullet->source_unit);
					break;
				}
				case WeaponId::IonCannon:
				{
					globals.create_effect(EffectId_IonField, 10 * 24, {hit.x, hit.y}, 3 * 32, SpriteId::IonField, 5, bullet->player, bullet->source_unit);
					break;
				}
				case WeaponId::CantavisHallucinationCast:
				{
					globals.create_effect(EffectId_HallucinatingField, 10 * 24, {hit.x, hit.y}, 1.5 * 32, SpriteId::ProtossHallucinationEffect, 5, bullet->player, bullet->source_unit);
					break;
				}
			}
		}

		s16 x, y, radius;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		radius = weapons_dat::outer_splash_radius[weapon_id];

		// Nathrokor weapon
		if (weapon_id == WeaponId::NathrokorPlasmidBreath)
		{
			radius = 32;
		}
		// Tethzorokor weapon
		if (weapon_id == WeaponId::TethzorokorSaberArray)
		{
			radius = 32;
		}
		// Gorgrokor weapon
		if (weapon_id == WeaponId::ZarcavrokorBisectingBlades || is_charged_gorgrokor_bullet(bullet, weapon_id))
		{
			radius = 45;
		}

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(
			x - radius,
			y - radius,
			x + radius,
			y + radius
		);
		dark_swarm_damage(x, y, radius, bullet, weapon_id);

		bool has_guaranteed_hit_unit = guaranteedHit != 0;
		if (has_guaranteed_hit_unit && nathrokor_condition(bullet, bullet->attack_target.unit))
		{
			helper::weapon_bullet_shot(guaranteedHit, bullet, 1);
		}
		for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
		{
			Unit* target = unitsInSplash.get_unit(i);
			if (has_guaranteed_hit_unit && target == guaranteedHit)
			{ // skips target check if has_guaranteed_hit_unit is unset, just for little bit of performance
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves -- except psi storm
			if (target == bullet->source_unit && bullet->weapon_id != WeaponId::PsiStorm) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player (also checks if type is 'splash enemy' or 'nuclear missile')
			// if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;

			if (splash == SplashType::Enemy)
			{
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			}
			else if (splash == SplashType::FriendlyNew)
			{
				if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;
			}

			// Can the unit be targeted?
			if (!scbw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			s16 outer = weapons_dat::outer_splash_radius[bullet->weapon_id];

			// Melee attack area of effect
			if (weapon_id == WeaponId::ZarcavrokorBisectingBlades || is_charged_gorgrokor_bullet(bullet, weapon_id))
			{
				outer = 45;
			}

			// Nathrokor weapon
			if (weapon_id == WeaponId::NathrokorPlasmidBreath)
			{
				outer = 32;
			}

			if (!helper::is_unit_distance_within(target, bullet, outer)) continue;

			// Don't storm a unit that's already being stormed
			if (bullet->weapon_id == WeaponId::PsiStorm)
			{
				if (target->is_under_storm) continue;
				target->is_under_storm = 1;
			}

			// Inner radius 
			s16 inner = weapons_dat::inner_splash_radius[bullet->weapon_id];

			// Melee attack area of effect
			if (weapon_id == WeaponId::ZarcavrokorBisectingBlades || is_charged_gorgrokor_bullet(bullet, weapon_id))
			{
				inner = 30;
			}

			if (weapon_id == WeaponId::HeraclesJavelinCannons)
			{
				if (bullet->source_unit)
				{
					UnitExt* bulletSourceExt = get_unit_ext(bullet->source_unit);

					const f64 splashBase = weapons_dat::inner_splash_radius[WeaponId::HeraclesJavelinCannons];
					const u32 stacks = bulletSourceExt->values[ValueNemeanReactorsStack];
					inner = (s16)(splashBase * (1 + 0.1 * stacks));
				}
			}

			if (helper::is_unit_distance_within(target, bullet, inner) && nathrokor_condition(bullet, target))
			{
				helper::weapon_bullet_shot(target, bullet, 1); // 100% damage
				continue;
			}

			// Burrowed units can only be hit by inner radius
			if (target->status & UnitStatus::Burrowed) continue;

			// Medium radius
			s16 medium = weapons_dat::medium_splash_radius[bullet->weapon_id];

			// Melee attack area of effect
			if (weapon_id == WeaponId::ZarcavrokorBisectingBlades || is_charged_gorgrokor_bullet(bullet, weapon_id))
			{
				medium = 37;
			}

			if (weapon_id == WeaponId::NathrokorPlasmidBreath)
			{
				radius = 32;
			}

			if (helper::is_unit_distance_within(target, bullet, medium) && nathrokor_condition(bullet, target))
			{
				helper::weapon_bullet_shot(target, bullet, 2); // 50% damage
				continue;
			}

			// Outer radius
			if (nathrokor_condition(bullet, target))
			{
				helper::weapon_bullet_shot(target, bullet, 4); // 25% damage
			}
		}
	}

	void lakizilisk_splash(Bullet* bullet, u32 weapon_id, u8 splash)
	{
		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::outer_splash_radius[weapon_id];

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		for (s32 i = 0; i < unitsInSplash.get_unit_count(); i++)
		{
			Unit* target = unitsInSplash.get_unit(i);

			// Units can't splash themselves
			if (target == bullet->source_unit) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player
//			if (scbw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;
			if (splash == SplashType::Enemy)
			{
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			}
			else if (splash == SplashType::FriendlyNew)
			{
				bool lurkerOverride = false;
				if (bullet->source_unit)
				{
					UnitExt* bulletSourceExt = get_unit_ext(bullet->source_unit);
					if ((Unit*)bulletSourceExt->values[ValueLurkerPickTarget] == target)
					{
						lurkerOverride = true;
					}
				}
					
				if (scbw::is_unit_enemy(bullet->player, target) == false && !lurkerOverride && target != bullet->attack_target.unit)
				{
					continue;
				}
			}

			// Can the unit be targeted?
			if (!scbw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[bullet->weapon_id]))
			{
				continue;
			}

			if (bullet->source_unit != nullptr)
			{
				bool cont = false;
				if (globals.spine_pair_exists(bullet->source_unit, target))
				{
					cont = true;
					break;
				}
				if (cont)
					continue;
				globals.add_lurker_spine_pair(bullet->source_unit, target);
			}
			helper::weapon_bullet_shot(target, bullet, 1);
		}
	}

	bool is_charged_gorgrokor_bullet(Bullet* bullet, u32 weapon_id)
	{
		if (weapon_id == WeaponId::GorgrokorSerratedCleavers)
		{
			if (bullet && bullet->source_unit && bullet->source_unit->id == UnitId::ZergGorgrokor)
			{
				return true;
			}
		}
		return false;
	}

	// clone of 'Normal' damage for reference
	void normal_damage(Bullet* bullet, u32 weapon_id)
	{
		if (is_charged_gorgrokor_bullet(bullet, weapon_id)) // TODO: Check if this is even necessary (old version needed energy). -Pr0nogo
		{
			splash(bullet, weapon_id, SplashType::FriendlyNew, bullet->attack_target.unit);
			return;
		}

		Unit* target = bullet->attack_target.unit;
		if (target == nullptr)
		{
			return;
		}

		dark_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);

		// Kalkalisk behavior
		if (bullet->weapon_id == WeaponId::KalkaliskViscousAcid)
		{
			auto count = globals.get_multihit_count(bullet);
			if (count >= 4)
			{
				return;
			}
			target->damage_with_custom((weapons_dat::damage_amount[WeaponId::KalkaliskViscousAcid] - count) * 256, weapons_dat::damage_type[WeaponId::KalkaliskViscousAcid], bullet->source_unit, bullet->player, 0, 1, false);
			return;
		}

		u8 divisor = 1;
		if (weapon_id == WeaponId::ArchitectDispersiveSalvo)
		{
			divisor = globals.get_divider(bullet);
		}

		else if (weapon_id == WeaponId::GeszithalorAcidSpore
			&& bullet->source_unit
			&& bullet->source_unit->sprite
			&& target->sprite)
		{
			// Searing Acid - Geszithalor behavior
			UnitExt* bulletSourceExt = get_unit_ext(bullet->source_unit);
			auto X = bulletSourceExt->last_bullet_launch_position.x;
			auto Y = bulletSourceExt->last_bullet_launch_position.y;
			divisor = scbw::get_distance_fast(X, Y, target->get_x(), target->get_y()) / (32 * 3);
		}

		if (bullet->hit_flags & 1) // Miss?
		{
			if (!target->unit_ai)
			{
				return;
			}
			if (!bullet->source_unit)
			{
				return;
			}
			helper::do_weapon_hit(target, bullet->source_unit, divisor);
			return;
		}
		else
		{
			if (weapons_dat::behavior[weapon_id] == WeaponBehavior::Bounce)
			{
				if (bullet)
				{
					if (bullet->source_unit && bullet->source_unit->id == UnitId::ZergBactalisk)
					{
						divisor = 1;
					}
					else
					{
						const s32 bounceCount = 2;
						divisor = bounceCount - bullet->remaining_bounces;
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

		dark_swarm_damage(x, y, r, bullet, weapon_id);

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

	void nuke(Bullet* bullet, u32 weapon_id)
	{
		Vector32 pos = { bullet->position.x, bullet->position.y } ;

		globals.create_effect(EffectId_GroundZero, 24, pos, 0, -1, 5, bullet->player, bullet->source_unit);

		splash(bullet, weapon_id, SplashType::Radial);
	}

	// clone of 'Yamato' damage for reference
	void yamato_damage(Bullet* bullet, u32 weapon_id)
	{
		if (!bullet->attack_target.unit)
		{
			return;
		}
		dark_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);
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