#include "weapon_effect.h"
#include <BW/UnitFinder.h>
#include <BW/enumerations.h>
#include <BW/api.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "globals.h"
#include "weapons/weapon_damage.h"
#include "limits/ccmu.h"
#include "CM/cm.h"

namespace {
	// SC internal data for Lakizilisk spines
	struct SpineHitTable {
		struct {
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
		Unit* swarm = bw::get_dark_swarm_from_position(x, y, r);
		if (swarm) {
			u32 damage = (weapons_dat::damage_amount[weapon_id] * 256) +
				(weapons_dat::armor_pen[weapon_id] * 256 * bw::get_upgrade_level(bullet->player, weapons_dat::damage_upgrade[weapon_id]));
			if (bullet->hit_flags & 2) {
				damage /= 2;
			}
			if (damage >= swarm->hitpoints) {
				swarm->bwfunc_remove();
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

		s16 inner = weapons_dat::inner_splash_radius[weapon_id];
		s16 medium = weapons_dat::medium_splash_radius[weapon_id];
		s16 outer = weapons_dat::outer_splash_radius[weapon_id];

		Point16 pos = bullet->sprite->position;
		Unit* unit = bullet->source_unit;
		if (unit && unit->sprite) { // Sprite is new.
			auto& bullet_source_ext = unit->ext();
			if (bullet->weapon_id == WeaponId::CyclopsBrontesPayload && unit->id == UnitId::TerranCyclopsFuturist) { // So we don't need to use another weapon id. -Pr0nogo
				splash = SplashType::FriendlyNew;
			}
			if (unit->is_blind) {
				splash = SplashType::Radial;
			}

			// Create effect fields
			switch (weapon_id) {
				case WeaponId::DiademIonCannon: {
					g_iquare_globals->create_effect(EffectId::DiademIonField, 10 * 24, pos, 3 * 32, SpriteId::IonField, 5, bullet->player, unit);
				} break;
				case WeaponId::AionLazarusSalvo: {
					g_iquare_globals->create_effect(EffectId::ReclamationField, 36, pos, 2 * 32, SpriteId::CausticDischarge, 5, bullet->player, unit);
				} break;
				case WeaponId::SalamanderPerditionBarrage: {
					g_iquare_globals->create_effect(EffectId::SalamanderFire, 5 * 24, pos, 0.5 * 32, SpriteId::SalamanderImpact, 5, bullet->player, unit);
				} break;
				case WeaponId::CantavisHallucinationCast: {
					g_iquare_globals->create_effect(EffectId::HallucinatingField, 10 * 24, pos, 1.5 * 32, SpriteId::ProtossHallucinationEffect, 5, bullet->player, unit);
				} break;
				case WeaponId::KeskathalorScreamingVenom: {
					g_iquare_globals->create_effect(EffectId::KeskathalorCausticDischarge, 5 * 24, pos, 1.5 * 32, SpriteId::CausticDischarge, 5, bullet->player, unit);
				} break;
			}

			if (bullet_source_ext.values[ValueTriggerWeaponSplashInnerSet]) {
				inner = bullet_source_ext.values[ValueTriggerWeaponSplashInnerAmount];
			}
			if (bullet_source_ext.values[ValueTriggerWeaponSplashMediumSet]) {
				medium = bullet_source_ext.values[ValueTriggerWeaponSplashMediumAmount];
			}
			if (bullet_source_ext.values[ValueTriggerWeaponSplashOuterSet]) {
				outer = bullet_source_ext.values[ValueTriggerWeaponSplashOuterAmount];
			}
		}

		if (guaranteed_hit) {
			helper::weapon_bullet_shot(guaranteed_hit, bullet, 1);
		}
		carapace_swarm_damage(pos.x, pos.y, outer, bullet, weapon_id);

		if (!outer) {
			return;
		}

		BulletExt* bullet_ext = get_bullet_ext(bullet);

		const auto area = box32_rect(pos, get_splash_width_from_weapon_id(weapon_id), get_splash_height_from_weapon_id(weapon_id));
		for (Unit* target : g_unit_search.rect(area).filter(unit, FilterIncludingSelf)) {
			if (guaranteed_hit && target == guaranteed_hit) { // skips target check if has_guaranteed_hit_unit is unset, just for little bit of performance
				bullet_ext->hit_count++;
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves, unless...
			if (unit == target && !weapon_id_can_self_splash(bullet->weapon_id)) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player (also checks if type is 'splash enemy' or 'nuclear missile')
			// if (bw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;

			if (splash == SplashType::Enemy) {
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			} else if (splash == SplashType::FriendlyNew) {
				if (bw::is_unit_enemy(bullet->player, target) == false && target != bullet->attack_target.unit) continue;
			}

			// Can the unit be targeted?
			if (!bw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			if (!helper::is_unit_distance_within(target, bullet, outer)) continue;

			if (helper::is_unit_distance_within(target, bullet, inner)) {
				helper::weapon_bullet_shot(target, bullet, 1); // 100% damage
				bullet_ext->hit_count++;
				continue;
			}

			if (helper::is_unit_distance_within(target, bullet, medium)) {
				helper::weapon_bullet_shot(target, bullet, 2); // 50% damage
				bullet_ext->hit_count++;
				continue;
			}

			helper::weapon_bullet_shot(target, bullet, 4); // 25% damage
			bullet_ext->hit_count++;
		}
		if (bullet->weapon_id == WeaponId::PatriarchIronhandWeaver && bullet_ext->hit_count >= 8) {
			play_unit_banter(bullet->attack_target.unit, UnitId::ProtossPatriarch);
		}
	}

	void lakizilisk_splash(Bullet* bullet, u32 weapon_id, u8 splash) {
		s16 x = bullet->sprite->position.x;
		s16 y = bullet->sprite->position.y;
		s16 r = weapons_dat::outer_splash_radius[weapon_id];

		// Get units within the outer splash radius
		bw::UnitFinder units_in_splash(x - r, y - r, x + r, y + r);
		for (s32 i = 0; i < units_in_splash.get_unit_count(); i++) {
			Unit* target = units_in_splash.get_unit(i);

			// Units can't splash themselves
			if (target == bullet->source_unit) continue;

			// Don't hit allied units -- normally just checks target->player == bullet->player
			if (splash == SplashType::Enemy) {
				if (bullet->player == target->player && target != bullet->attack_target.unit) continue;
			} else if (splash == SplashType::FriendlyNew) {
				bool lakizilisk_override = false;
				if (bullet->source_unit) {
					auto& bullet_source_ext = bullet->source_unit->ext();
					if ((Unit*)bullet_source_ext.values[ValueMissileReturnTarget] == target) {
						lakizilisk_override = true;
					}
				}
					
				if (bw::is_unit_enemy(bullet->player, target) == false && !lakizilisk_override && target != bullet->attack_target.unit) {
					continue;
				}
			}

			// Can the unit be targeted?
			if (!bw::can_weapon_target_unit(bullet->weapon_id, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[bullet->weapon_id])) {
				continue;
			}

			if (bullet->source_unit) {
				bool skip = false;

				if (g_iquare_globals->spine_pair_exists(bullet->source_unit, target)) {
					skip = true;
					break;
				}
				if (skip) continue;
				g_iquare_globals->add_lakizilisk_spine_pair(bullet->source_unit, target);
			}
			helper::weapon_bullet_shot(target, bullet, 1);
		}
	}

	static void cleaving_attack(Bullet* bullet, u32 weapon_id) {
		Unit* attacker = bullet->source_unit;
		if (!attacker) {
			return;
		}

		Unit* attack_target = bullet->attack_target.unit;
		Point16 forward_pos = attack_target ? attack_target->position : bullet->position;
		Vec32f dir_forward = bw::dir_between(attacker->position, forward_pos);

		f32 outer    = weapons_dat::outer_splash_radius[weapon_id];
		f32 inner_t  = 1 - weapons_dat::inner_splash_radius[weapon_id] / outer;
		f32 medium_t = 1 - weapons_dat::medium_splash_radius[weapon_id] / outer;
		f32 outer_t  = 0;

		const auto area = box32_rect(attacker->position, get_splash_width_from_weapon_id(weapon_id), get_splash_height_from_weapon_id(weapon_id));
		for (Unit* target : g_unit_search.rect(area).filter()) {
			if (!helper::is_unit_distance_within(target, bullet, outer)) {
				continue;
			}
			if (!bw::can_weapon_target_unit(bullet->weapon_id, target, attacker)) {
				continue;
			}
			if (!bw::is_unit_enemy(bullet->player, target) && target != attack_target) {
				continue;
			}

			Vec32f dir_to_target = bw::dir_between(attacker->position, target->position);
			f32 t = bw::dot_product(dir_forward, dir_to_target);
			if (t >= inner_t) {
				helper::weapon_bullet_shot(target, bullet, 1); // 100% damage
			} else if (t >= medium_t) {
				helper::weapon_bullet_shot(target, bullet, 2); // 50% damage
			} else if (t >= outer_t) {
				helper::weapon_bullet_shot(target, bullet, 4); // 25% damage
			}
		}
	}

	// clone of 'Normal' damage for reference
	void normal_damage(Bullet* bullet, u32 weapon_id) {
		if (weapon_id_cleaves(weapon_id)) {
			cleaving_attack(bullet, weapon_id);
			return;
		}
		if (weapon_is_melee_with_aoe(bullet, weapon_id)) {
			splash(bullet, weapon_id, SplashType::FriendlyNew, bullet->attack_target.unit);
			return;
		}

		Unit* target = bullet->attack_target.unit;
		if (!target) {
			return;
		}

		carapace_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);

		// Kalkalisk behavior
		if (weapon_id_is_incremental(bullet->weapon_id)) {
			g_iquare_globals->add_incremental_bullet_multihit(bullet, target);
			u32 count = g_iquare_globals->get_multihit_count(bullet);
			target->damage_with(bullet->weapon_id, (weapons_dat::damage_amount[bullet->weapon_id] - count) * 256, bullet->source_unit, bullet->player);
			return;
		}

		u8 divisor = 1;
		if (weapon_id == WeaponId::ArchitectDispersiveSalvo) {
			divisor = g_iquare_globals->get_divider(bullet);
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
			if (weapons_dat::behavior[weapon_id] == WeaponBehavior_Bounce) {
				if (bullet) {
					if (bullet->source_unit && weapon_id_bounces_caustic_fission(bullet->weapon_id)) {
						divisor = 1;
					} else if (WeaponId::bounces_power_breach(static_cast<WeaponId::Enum>(bullet->weapon_id))) {
						const s32 bounce_count = 4;
						divisor = bounce_count - bullet->remaining_bounces;
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
	void air_splash(Bullet* bullet, u32 weapon_id) { // 0048B790
		Unit* target = bullet->attack_target.unit;
		if (!target || !target->sprite) {
			return;
		}

		s16 radius = weapons_dat::outer_splash_radius[weapon_id];
		Sprite* bullet_sprite = bullet->sprite;
		s16 pos_x = bullet_sprite->position.x;
		s16 pos_y = bullet_sprite->position.y;
		s32 list;
		Box16* search_radius = {};

		g_air_splash_list_count = 0;
		search_radius->left = pos_x - radius;
		search_radius->top = pos_y - radius;
		search_radius->right = radius + pos_x;
		search_radius->bottom = radius + pos_y;

		s16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::outer_splash_radius[weapon_id];
		// Get units within the outer splash radius
		bw::UnitFinder units_in_splash(x - r, y - r, x + r, y + r);
		carapace_swarm_damage(x, y, r, bullet, weapon_id);

		s32 result;
		for (s32 i = 0; i < units_in_splash.get_unit_count(); i++) {
			// air_splash_proc
			s8 weapon_id_loop;
			s32 list_count;
			Unit* target = units_in_splash.get_unit(i);

			weapon_id_loop = bullet->weapon_id;
			if (target != bullet->source_unit && (target->player != bullet->player || target == bullet->attack_target.unit)) {
				if (bw::can_weapon_target_unit(weapon_id_loop, target, target)) {
					if (helper::is_unit_distance_within(target, bullet, weapons_dat::outer_splash_radius[weapon_id_loop])) {
						if (helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[weapon_id_loop])
							&& target == bullet->attack_target.unit)
						{
							g_air_splash_list[0] = target;
							result = 1;
							g_air_splash_list_count = 1;
							break;
						}
						list_count = g_air_splash_list_count;
						g_air_splash_list[g_air_splash_list_count] = target;
						g_air_splash_list_count = list_count + 1;
					}
				}
			}
		}

		if (g_air_splash_list_count) {
			if (g_air_splash_list_count == 1) {
				list = 0;
			} else {
				list = bw::rand_between(0, g_air_splash_list_count - 1);
			}
			helper::weapon_bullet_shot(target, bullet, (s32)g_air_splash_list[list]);
		}
		// air_splash_proc_2
		splash(bullet, weapon_id, SplashType::FriendlyNew);
	}

	void nuke(Bullet* bullet, u32 weapon_id) {
		g_iquare_globals->create_effect(EffectId::MorningstarGroundZero, 24, bullet->position, 0, SpriteId::None, 5, bullet->player, bullet->source_unit);
		splash(bullet, weapon_id, SplashType::Radial);
	}

	// clone of 'Yamato' damage for reference
	void yamato_damage(Bullet* bullet, u32 weapon_id) {
		if (!bullet->attack_target.unit) {
			return;
		}
		carapace_swarm_damage(bullet->attack_target.unit->position.x, bullet->attack_target.unit->position.y, 1, bullet, weapon_id);
		splash(bullet, weapon_id, SplashType::Radial);
	}

	void parasite(Bullet* bullet, u32 weapon_id) {
		if (!bullet->source_unit) {
			return;
		}
		Unit* spawn = bw::create_unit_in_game(UnitId::ZergVilgoleth, bullet->player, bullet->position.x, bullet->position.y);
		if (spawn) {
			spawn->ext().timers[TimerAutophagySpawn] = 24;
		}
	}

	void broodlings(Bullet* bullet, u32 weapon_id) {
		if (!bullet->source_unit) {
			return;
		}
		Unit* target = bullet->attack_target.unit;
		hooks::broodling_hit(bullet->source_unit, target, bullet);
	}

	void lockdown(Bullet* bullet, u32 weapon_id) {
		s16 x = bullet->sprite->position.x;
		s16 y = bullet->sprite->position.y;
		s16 r = weapons_dat::outer_splash_radius[weapon_id];
		if (r) {
			bw::UnitFinder units_in_splash(x - r, y - r, x + r, y + r);
			for (s32 i = 0; i < units_in_splash.get_unit_count(); i++) {
				Unit* target = units_in_splash.get_unit(i);
				if (!target->is_dead() && !units_dat::group_flags[target->id].is_building
				&&  !unit_id_is_trap(target->id) && !unit_id_is_resource_node(target->id, false) && target->id != UnitId::TerranMorningstar)
				{
					hooks::lockdown_hit(target, TerranMine::LobotomyTimerVanilla, bullet);
				}
			}
		} else if (bullet->attack_target.unit) {
			hooks::lockdown_hit(bullet->attack_target.unit, TerranMine::LobotomyTimerVanilla, bullet);
			apply_lobotomy_slow(bullet->attack_target.pt);
		}
	}

	// Hooked to allow Valkyries to splash Stewards.
	// This is intended to return false always. -Pr0nogo
	bool splash_target_check(Bullet* bullet, Unit* target) { // 0048B570
		const auto weapon_id = bullet->weapon_id;
		if (target == bullet->source_unit && weapon_id != WeaponId::PsiStorm) {
			return false;
		}
		const auto explosion_type = weapons_dat::explosion_type[weapon_id];
		if (explosion_type == WeaponEffect::SplashEnemy || explosion_type == WeaponEffect::SplashAir) {
			if (target != bullet->attack_target.unit && (target->player == bullet->player || bw::is_allied_to(bullet->player, target->player))) {
				return false;
			}
		}
		if (!helper::can_weapon_target_unit(weapon_id, target, target)) {
			return false;
		}
		if (!helper::is_unit_distance_within(target, bullet, weapons_dat::outer_splash_radius[weapon_id])) {
			return false;
		}
		if (g_is_in_ground_splash_proc) {
			if (helper::is_unit_distance_within(target, bullet, weapons_dat::inner_splash_radius[weapon_id])) {
				helper::weapon_bullet_shot(target, bullet, 1);
				return false;
			}
		}
		if (!(target->status & UnitStatus::Burrowed) && (g_is_in_ground_splash_proc || target != bullet->attack_target.unit)) {
			if (helper::is_unit_distance_within(target, bullet, weapons_dat::medium_splash_radius[weapon_id])) {
				helper::weapon_bullet_shot(target, bullet, 2);
				return false;
			}
			helper::weapon_bullet_shot(target, bullet, 4);
		}
		return false;
	}
}