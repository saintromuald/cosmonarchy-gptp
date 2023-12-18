#include "weapon_damage.h"
#include "../../SCBW/api.h"
#include <algorithm>
#include <cmath>
#include "math_utils.h"
#include "Utils/captivating_claws.h"
#include "hooks/cosmonarchy_finder.h"
#include "globals.h"
#include "logger.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "hooks/orders/spells/hallucination_spell.h"
#include "hooks/limits/sfx_extender.h"
#include "DatExtend/datext.h"
#include "weapon_fire.h"
#include "trigger_system\trigger_system.h"
#include "hooks/unit_stats/armor_bonus.h"
#include "hooks/unit_destructor_special.h"

#include "SCBW\UnitFinder.h"

Bullet* weapon_damage_current_bullet;

void apply_ensnare_internal(Unit* unit, u32 ensnare_factor) {
	// Overlay
	if (!(unit->ensnare_timer) && !(unit->status & UnitStatus::Burrowed)) {
		scbw::add_unit_overlay(unit, ImageId::EnsnareOverlay_Small + scbw::get_overlay_offset(unit->id));
	}

	// Effect
	UnitExt* unit_ext = get_unit_ext(unit);
	unit_ext->values[ValueEnsnareFactor] = ensnare_factor;
	unit->ensnare_timer = 5;
	unit->update_speed();
}

bool carapace_swarm_blocks_damage(Unit* target, u32 weapon_id, u32 damage_divisor) {
	return scbw::is_under_carapace_swarm(target)
		&& weapons_dat::max_range[weapon_id] > 32
		&& (damage_divisor == 1 || weapon_id == WeaponId::GeszithalorAcidSpore)
		&& weapon_id != WeaponId::HarakanFlamethrower
		&& weapon_id != WeaponId::MatadorTwinstrikeFlamethrowers
		&& weapon_id != WeaponId::DurendalAbaddonFlamethrowers
		&& weapon_id != WeaponId::LakiziliskSubterraneanSpines
		&& weapon_id != WeaponId::Custom;
}

bool irradiate_target(Unit* unit)
{
	return true; // Originally prohibited targeting eggs
}

namespace hooks
{
	static void play_alert(Unit* unit, u32 sound_id) {
		u32 alert_id = SoundId::AlertZerg - *g_console_index;
		helper::play_sound_from_unit(alert_id, 0, g_options_list->flags & 1, 0);
		helper::do_unit_event_notification(unit, 16, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, alert_id);

		globals.alert.sound_id = sound_id;
		globals.alert.play_timer = 12;
	}


	static void reset_all_unit_attack_notify_timers_in_range(Unit* unit) {
		EntryArray units = {};
		unit_search_radial(&units, unit->position, 11 * 32);
		for (u32 i = 0; i < units.count; ++i) {
			Unit* other = units.units[i];
			other->attack_notify_timer = other->player == unit->player ? 100 : 1000;
		}
	}

	// Hooked to fix one-shots not notifying owners. -Pr0nogo
	void unit_attack_notify(Unit* unit) { // 0048F230
		if (!ignore_alert) {
			return;
		}
		u32 local_player = *g_local_nation_id;
		if (*g_is_in_replay || globals.players[local_player].witness_mode || unit->player == 11) {
			return;
		}
		if (unit->player == local_player) {
			unit->last_event_color = g_palette_index_reference[20];
			unit->last_event_timer = 20;
			if (!globals.alert.own_cooldown) {
				if (helper::unit_is_in_notify_range(unit) && !unit->attack_notify_timer) {
					reset_all_unit_attack_notify_timers_in_range(unit);
					globals.alert.own_cooldown = 100;

					bool is_building = units_dat::base_property[unit->id] & UnitProperty::Building;
					u32 sound_id = (is_building ? SoundId::BaseUnderAttack_Zerg : SoundId::ForcesUnderAttack_Zerg) + *g_console_index;
					if (!is_building) {
						u32 sound_first = units_dat::sound_annoyed_first[unit->id];
						u32 sound_last = units_dat::sound_annoyed_last[unit->id];
						if (sound_first && sound_last) {
							sound_id = scbw::rand_between_local(sound_first, sound_last);
						}
					}
					play_alert(unit, sound_id);
				}
			}
		} else if (scbw::are_mutual_allies(local_player, unit->player)) {
			unit->last_event_color = g_palette_index_reference[20];
			unit->last_event_timer = 20;
			if (!globals.alert.own_cooldown && !globals.alert.ally_cooldown) {
				if (helper::unit_is_in_notify_range(unit) && !unit->attack_notify_timer) {
					reset_all_unit_attack_notify_timers_in_range(unit);
					globals.alert.ally_cooldown = 1000;

					u32 sound_id = SoundId::AdvisorZergUnderAttackZerg + (*g_console_index * 3) + g_player_table[unit->player].race;
					play_alert(unit, sound_id);
				}
			}
		}
	}

	static void weapon_damage_damage_modifiers(WeaponDamageData& data);
	static void weapon_damage_lifesteal(const WeaponDamageData& data);
	static void weapon_damage_behaviors(const WeaponDamageData& data);
	static bool weapon_damage_fatal(const WeaponDamageData& data);

	static bool weapon_damage_is_lethal(const WeaponDamageData& data) {
		if (data.damage_type == DamageType::Independent) return false;
		// Also proc on deaths for hallucinations, since we know they always die in one hit
		if (data.target->status & UnitStatus::IsHallucination) return true;
		return data.damage >= data.target->hitpoints;
	}

	/// Hooks into the Unit::damage_with() function.
	void weapon_damage(s32 damage, Unit* target, u8 weapon_id, Unit* attacker, u8 attacking_player_id, s8 direction, u8 damage_divisor) {
		WeaponDamageData data = {};
		data.damage					= damage;
		data.damage_type			= weapons_dat::damage_type[weapon_id];
		data.damage_divisor			= damage_divisor;
		data.weapon_id				= weapon_id;
		data.direction				= direction;
		data.target					= target;
		data.attacker				= attacker;
		data.attacking_player_id	= attacking_player_id;
		weapon_damage_root(data);
	}

	void weapon_damage_custom(
		s32 damage,
		Unit* target,
		u32 armor_pen,
		Unit* attacker,
		u8 attacking_player_id,
		s8 direction,
		u8 damage_divisor,
		bool ignore_flat_armor
	) {
		WeaponDamageData data = {};
		data.damage					= damage;
		data.damage_type			= DamageType::Impact;
		data.damage_divisor			= damage_divisor;
		data.weapon_id				= WeaponId::Custom;
		data.bonus_armor_pen		= armor_pen;
		data.direction				= direction;
		data.target					= target;
		data.attacker				= attacker;
		data.attacking_player_id	= attacking_player_id;
		data.ignore_flat_armor		= ignore_flat_armor;
		weapon_damage_root(data);
	}

	void weapon_damage_root(WeaponDamageData& data) {
		// The unit must neither be already dead nor invincible
		if (!data.target || data.target->hitpoints == 0 || data.target->status & UnitStatus::Invincible) {
			return;
		}

		UnitExt* target_ext = get_unit_ext(data.target);

		// Ancile - Providence - Hitscan handling
		switch (weapons_dat::behavior[data.weapon_id]) {
			case WeaponBehavior_AppearOnAttacker:
			case WeaponBehavior_AppearOnTargetSite:
			case WeaponBehavior_AppearOnTargetUnit:
			{
				EntryArray units = {}; // @Veeq7, can't we filter for Ancile ids instead of checking for it later? Is that more or less performant? -Pr0nogo
				unit_search_radial(&units, data.target->position, 2 * 32);
				for (u32 i = 0; i < units.count; ++i) {
					Unit* unit = units.units[i];
					if (unit->id == UnitId::TerranAncile && unit->energy >= 1 * 256 && scbw::is_allied_to(unit->player, data.target->player)) {
						scbw::create_shield_overlay(data.target, data.direction);
						unit->spend_unit_energy(data.damage);
						helper::play_sound_from_unit(scbw::rand_between(SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03), unit, 1, 0);
						ignore_alert = true;
						unit_attack_notify(unit);
						ignore_alert = false;
						scbw::reveal_attacker(data.attacker, unit);
						return;
					}
				}
			} break;
		}

		// Ai_ReactToHit crash - null town
		if (data.target->unit_ai) {
			// Ai_ReactToHit crash - null town
			WorkerAi* ai = (WorkerAi*)data.target->unit_ai;
			if (ai->type == AiType::Worker || ai->type == AiType::Building) {
				if (!ai->town) {
					u32 attacker_id = data.attacker ? data.attacker->id : UnitId::None;
					scbw::printf("Ai_ReactToHit crash prevention - null town! Attacker id %d, Attacker player %d, Target id %d, Target player %d",
																              attacker_id, data.attacker->player, data.target->id, data.target->player);
					return; // Don't think this ever happens anymore, removed can_damage bool 11/27/2023 -Pr0nogo
				}
			}
		}

		if (!data.dont_proc_link_damage) { // damage from links was already calculated
			weapon_damage_damage_modifiers(data);
		}

		if (!data.dont_proc_link_damage && scbw::has_clarion_links(data.target)) {
			data.damage /= 2;
			scbw::spread_clarion_damage(data);
		}

		// Hallucinations deal half damage
		if (data.attacker) {
			if (data.attacker->status & UnitStatus::IsHallucination) {
				data.damage /= 2;
			}
		}

		// Minimum amount of damage that can be dealt
		if (data.damage < 256) {
			data.damage = 256;
		}

		// Reduce Defensive Matrix
		if (data.target->defensive_matrix_hp != 0) {
			u32 defensive_matrix_reduction = data.damage;
			if (data.target->defensive_matrix_hp <= data.damage) {
				defensive_matrix_reduction = data.target->defensive_matrix_hp;
			}
			data.damage -= defensive_matrix_reduction;
			data.target->reduce_defensive_matrix_hp(defensive_matrix_reduction);
		}

		// Apply armor
		bool shield_overlay = false;
		if (data.damage_type != DamageType::IgnoreArmor && !data.ignore_flat_armor) {
			s32 armor_pen = hooks::get_modified_armor_pen(data.attacker, data.target, data.weapon_id) + data.bonus_armor_pen;
			s32 armor_total = (hooks::get_armor_internal(data.target) - armor_pen);

			if (data.attacker) {
				u32 fore_castle_factor = scbw::get_fore_castle_factor(data.target);
				if (fore_castle_factor) {
					u16 angle = scbw::get_angle(data.target->position.x, data.target->position.y, data.attacker->position.x, data.attacker->position.y);
					u16 unit_angle = data.target->velocity_direction_1;
					s32 arc = (f64)45 / 1.40625; // 90 degrees
					s32 delta = unit_angle - angle;
					delta = abs((delta + 128) % 256 - 128);
					if (delta <= arc) {
						armor_total += fore_castle_factor;
						shield_overlay = true;
					}
				}
			}
			armor_total = std::max(armor_total, 0);
			if (data.weapon_id != WeaponId::PsiStorm) {
				data.damage -= std::min(data.damage, armor_total * 256);
			}
		}

		// Reduce shields
		if (units_dat::shields_enabled[data.target->id] && data.target->shields >= 256) {
			data.shield_reduce_amount = std::min(data.damage, data.target->shields);
			if (data.shield_reduce_amount < 256) {
				data.shield_reduce_amount = 256;
			}
			data.damage -= data.shield_reduce_amount;
			if (data.damage < 0) {
				data.damage = 0;
			}
		}

		// Set min damage
		if (data.shield_reduce_amount == 0 && data.damage < 256) {
			data.damage = 256;
		}

		weapon_damage_behaviors(data);
		if (data.attacker) {
			weapon_damage_lifesteal(data);
		}
		if (weapon_damage_is_lethal(data)) {
			if (!scbw::is_allied_to(data.target->player, data.attacking_player_id) && scbw::get_game_kind() == GameKind_Massacre) {
				scbw::add_massacre_resource(data.attacking_player_id);
			}
			if (!weapon_damage_fatal(data)) { // Prevent attack if it wasn't fatal
				data.damage = 0;
				data.damage_type = DamageType::Independent;
			}
		}

		// Deal damage, except for Independent damage, which always deals 0
		if (data.damage_type != DamageType::Independent && data.target->hitpoints != 0 && data.target->defensive_matrix_hp == 0) {
			if (data.target->status & UnitStatus::IsHallucination) {
				data.damage = data.target->hitpoints; // Increasing the damage, so we insta kill, but don't inflate other damage calcs
			}
			ignore_alert = true;
			data.target->damage_hp(data.damage, data.attacker, data.attacking_player_id, data.weapon_id != WeaponId::Irradiate); // Prevent Seraphs from being continuously revealed to the irradiated target
			ignore_alert = false;
			unit_attack_notify(data.target); // Previously excluded WeaponId::Irradiate
		}

		// New AI defense logic -Veeq7
		scbw::ask_for_ai_help(data.target, data.attacker);

		// Reduce shields
		if (data.shield_reduce_amount) {
			data.target->shields -= data.shield_reduce_amount;
			if (data.attacker) {
				scbw::update_on_damage_dealt(data.attacker, data.shield_reduce_amount);
			}
			scbw::update_on_damage_taken(data.target, data.shield_reduce_amount);
			if (data.damage_type != DamageType::Independent && data.target->shields != 0) {
				shield_overlay = true;
			}
		}

		if (shield_overlay) {
			scbw::create_shield_overlay(data.target, data.direction);
		}

		// Update unit strength data
		data.target->air_strength = helper::get_unit_strength(data.target, false);
		data.target->ground_strength = helper::get_unit_strength(data.target, true);
	}

	static void weapon_damage_damage_modifiers(WeaponDamageData& data) {
		UnitExt* target_ext = get_unit_ext(data.target);

		// Allied invincibility frames (for e.g. Akistrokor Blissful Embrace)
		if (target_ext->timers[TimerAlliedInvFrames]) {
			if (scbw::is_allied_to(data.attacking_player_id, target_ext->values[ValueAlliedInvPlayer])) {
				data.damage = 0;
			}
		}

		// Isthrathaleth Carapace Swarm
		if (carapace_swarm_blocks_damage(data.target, data.weapon_id, data.damage_divisor)) {
			data.damage_type = DamageType::Independent;
		}

		switch (data.weapon_id) { // Damage divisor calc
			case WeaponId::ArchitectDispersiveSalvo:
			{
				u32 divisor = data.damage_divisor ? data.damage_divisor : 1;
				data.damage /= divisor;
			} break;
			case WeaponId::GeszithalorAcidSpore:
			{
				// Searing Acid - Geszithalor behavior
				data.damage += data.damage_divisor * 10 * 256;
			} break;
			default:
			{
				if (weapon_id_bounces_lithe_organelle(data.weapon_id))
				{
					data.damage *= 1 - data.damage_divisor * 0.33;
					if (data.damage < 0) {
						data.damage = 0;
					}
				} else {
					u32 divisor = data.damage_divisor ? data.damage_divisor : 1;
					data.damage /= divisor;
				}
			}
		}
	//	data.damage += target_ext->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchArmorReduction;
		if (weapon_damage_current_bullet) {
			BulletExt* current_bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
			if (current_bullet_ext->is_phantom) {
				data.damage /= 2;
			}
		}

		switch (data.weapon_id) {
			// Cabalist was here
		}

		// Hallucinating targets - fire a phantom strike
		if (data.attacker && weapon_id_procs_onhits(data.weapon_id) && target_ext->timers[TimerHallucinating] >= 1 && !scbw::is_allied_to(data.attacker->player, data.target->player))
		{
			if (weapon_damage_current_bullet && data.weapon_id != WeaponId::GladiusPlasmaFlurry && data.weapon_id != WeaponId::PaladinCrusaderMissiles) {
				BulletExt* current_bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
				if (!current_bullet_ext->is_phantom) {
					data.attacker->sprite->create_overlay(ImageId::PhantomStrikeProc);
				//	WeaponOffsets offsets = get_weapon_offsets(data.weapon_id);
					Bullet* phantom_strike = fire_bullet(data.attacker, data.weapon_id, { data.attacker->order_target.pt, data.target }, 0, false);
					if (phantom_strike) {
						BulletExt* phantom_ext = get_bullet_ext(phantom_strike);
						phantom_ext->is_phantom = true;
					}
				}
			} else {
				data.damage *= 1.5; // Currently doesn't proc onhits with this solution! -Pr0nogo
				data.attacker->sprite->create_overlay(ImageId::PhantomStrikeProc);
			}
		}
	}

	static void weapon_damage_behaviors(const WeaponDamageData& data) {
		UnitExt* target_ext = get_unit_ext(data.target);
		bool should_update_speed = false;

		// No attacker here
		switch (data.weapon_id) {
			case WeaponId::WendigoFusionSaw:
			{
				target_ext->values[ValueWendigoShatteredWindsStacks]++;
				target_ext->timers[TimerWendigoShatteredWinds] = 2 * 24 + 1;
			} break;
			case WeaponId::LegionnairePhaseSpear: {
				target_ext->timers[TimerBladeVortex] = 2 * 24 + 1; // Temporary? Or at least needs to be looked at again when aise no longer manages Blade Vortex marks. -Pr0nogo
			} break;
			case WeaponId::GladiusPlasmaFlurry: {
				if (target_ext->timers[TimerBindingPlasma]) {
					u32 source_player = target_ext->values[ValueGladiusBindingPlasmaPlayer];
					if (scbw::is_allied_to(data.attacking_player_id, source_player)) {
						target_ext->values[ValueGladiusBindingPlasmaStacks]++;
					}
				} else {
					target_ext->timers[TimerBindingPlasma] = 12;
					target_ext->values[ValueGladiusBindingPlasmaPlayer] = data.attacking_player_id;
					target_ext->values[ValueGladiusBindingPlasmaStacks] = 1;
				}
			} break;
			case WeaponId::PariahIconoclastBlades: {
				target_ext->timers[TimerPariahHit] = 2 * 24;
				target_ext->values[ValuePariahHitPlayer] = data.attacking_player_id;
			} break;
		}

		switch (data.target->id) {
			case UnitId::ProtossGolem: {
				// Golem - Tranquility Array
				if (data.target->shields >= 128) {
					scbw::shield_heal_proc(data.target, data.target->player, 64, 2);
				}
			} break;
			case UnitId::ZergZoryusthaleth: {
				// Zoryusthaleth - Incubate on damage taken
				if (!scbw::is_allied_to(data.target->player, data.attacking_player_id)) {
					incubate(data.target, 1, UnitId::ZergRilirokor, data.target->player);
				}
			} break;
		}

		if (data.attacker) {
			UnitExt* attacker_ext = get_unit_ext(data.attacker);

			// Ignore Carapace Swarm!
			// START
			switch (data.attacker->id) {
				case UnitId::ZergNathrokor:
				{
					attacker_ext->timers[TimerZergAdrenalFrenzy] = 2 * 24 + 1;
					data.attacker->update_speed();
				} break;
				case UnitId::ProtossAugur: {
					// Augur spend shields on attack
					data.attacker->shields = std::max(data.attacker->shields - 10 * 256, 0);
					attacker_ext->shield_regen_timer = 0;
				} break;
			}

			// Ensnaring Brood - Matraleth enchantment ability
			if (unit_applies_ensnare(data.attacker) && unit_can_be_ensnared(data.target))
			{
				 apply_ensnare_internal(data.target, scbw::get_ensnare_factor(data.attacker));
			}

			switch (data.weapon_id) {
				case WeaponId::PotentatePhaseSaber:
				{
					if (!scbw::is_allied_to(data.target->player, data.attacker->player)) {
						ascend_in_area(data.attacker->player, data.attacker->position, 4 * 32);
					}
				} break;
				case WeaponId::MindTyrantTyranny:
				{
					helper::mind_control_hit(data.attacker, data.target);
				} break;
			}
			// Ignore Carapace Swarm!
			// END

			switch (data.target->id) {
				case UnitId::TerranStriga:
				{
					// Striga - Fleeting Focus
					if (!(units_dat::base_property[data.attacker->id] & UnitProperty::Building)
					&&	!(data.attacker->status & UnitStatus::Burrowed)
					&&   !scbw::is_under_carapace_swarm(data.target))
					{
						const u32 dist = scbw::get_distance_fast(data.attacker->get_x(), data.attacker->get_y(), data.target->get_x(), data.target->get_y());
						if (dist < 2 * 32) {
							const u32 displacement_time = 5;
							Vector32 diff = scbw::get_vector_between(data.target->sprite->position, data.attacker->sprite->position);
							Vector32f displacement = scbw::get_normalized_vector(diff);
							displacement.x *= 1 * 32.0 / displacement_time;
							displacement.y *= 1 * 32.0 / displacement_time;

							attacker_ext->timers[TimerStrigaRepulsion] = displacement_time;
							attacker_ext->repulsion_vector = displacement;
						}
					}
				} break;
				case UnitId::ProtossSimulacrum:
				{
					// Autovitality - Simulacrum behavior
					if (!scbw::is_allied_to(data.target->player, data.attacker->player)
					&&  !scbw::is_under_carapace_swarm(data.target)
					&&   data.shield_reduce_amount >= data.target->shields
					&&   data.damage
					&&   target_ext->timers[TimerSimulacrumClone] == 0)
					{
						target_ext->timers[TimerSimulacrumClone] = 4 * 24;
						scbw::add_unit_overlay(data.target, ImageId::LegionnaireTeleportEffect);
						helper::play_sound_from_unit(SoundId::SimulacrumCloneStart01, data.target, 1, 0);
					}
				} break;
				case UnitId::ProtossAnalogue:
				{
					// Empathy Core - Analogue behavior
					if (weapon_damage_current_bullet) {
						BulletExt* current_bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
						if (current_bullet_ext->is_phantom) {
							break;
						}
					}
					if (!weapon_id_procs_onhits(data.weapon_id) || weapon_damage_is_lethal(data)) {
						break;
					}
					if (data.target->energy >= 10 * 256) {
						data.target->spend_unit_energy(10 * 256);

						u32 image_id = analogue_image_id_overwrite(data.weapon_id);
						if (image_id == ImageId::None) {
							Bullet* phantom = fire_bullet(data.target, data.weapon_id, make_target(data.attacker));
							if (phantom) {
								BulletExt* phantom_ext = get_bullet_ext(phantom);
								phantom_ext->is_phantom = true;
							}
						} else {
							u32 inv_direction = 255 - data.direction;
							// attkshiftproj probably does something specific that avoids the iscript errors. -Pr0nogo
						//	data.target->sprite->create_overlay(image_id, 0, 0, inv_direction);
							if (data.attacker) {
								data.attacker->damage_with(data.weapon_id, -1, data.attacker, data.attacking_player_id, inv_direction, 2);
							}
						}
					}
				} break;
				case UnitId::ZergZarcavrokor:
				{
					// Fan of Teeth - Zarcavrokor behavior
					if (data.target != data.attacker && !target_ext->timers[TimerZarcavrokorSecondaryFire]) {
						fire_bullet(data.target, WeaponId::ZarcavrokorBonesofReflection, make_target(data.attacker), 0, false);
						target_ext->timers[TimerZarcavrokorSecondaryFire] = weapons_dat::cooldown[WeaponId::ZarcavrokorBonesofReflection] - 1;
					}
				} break;
			}

			// These bypass Carapace Swarm
			switch (data.weapon_id) {
				case WeaponId::HarakanFlamethrower:
				case WeaponId::StrigaPhaseSaber:
				case WeaponId::MatadorTwinstrikeFlamethrowers:
				case WeaponId::DurendalAbaddonFlamethrowers:
				{
					target_ext->values[ValueApollyoidAdhesiveStacks]++;
					target_ext->timers[TimerApollyoidAdhesive] = 2 * 24;
				} break;
				case WeaponId::CyclopsFlakCannons:
				{
					iquare_globals.create_effect(EffectId_CyclopsRecursionRound, 48, data.target->position, 2 * 32, SpriteId::CyclopsPassiveArmingOverlay,
												 data.target->sprite->elevation + 1, data.attacker->player, data.attacker);
				} break;
			}

			if (!scbw::is_under_carapace_swarm(data.target)) {
				// Astral Rule - Patriarch behavior
				if (target_ext->timers[TimerPatriarchAstralRule] > 1
				&&  units_dat::shields_enabled[data.attacker->id]
				&&	data.attacker->shields < units_dat::max_shield_points[data.attacker->id] * 256)
				{
					data.attacker->heal_shields(5 * 256);
					helper::add_overlay_below_main(data.attacker->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
				}

				if (target_ext->timers[TimerTriumphantRiseDebuff])
				{
					scbw::astral_blessing_proc(data.target, data.attacking_player_id, WeaponId::HeraldStellarTorch);
				}

				switch (data.weapon_id) {
					case WeaponId::SavantDiscoidRailgun:
					{
						// Power Siphon
						if  (scbw::get_generic_rust_value(data.attacker, RustId::ToggleSavantOnhit)
						&&	unit_is_combatant(data.target))
						{
							attacker_ext->values[ValueSavantPowerSiphonBuff]++;
							attacker_ext->timers[TimerSavantPowerSiphonBuff] = 2 * 24 + 1;

							Unit* final_target = data.target;
							if (final_target->subunit && final_target->subunit->is_subunit()) {
								final_target = final_target->subunit;
							}
							UnitExt* final_target_ext = get_unit_ext(final_target);
							final_target_ext->values[ValueSavantPowerSiphonDebuff]++;
							final_target_ext->timers[TimerSavantPowerSiphonDebuff] = 2 * 24 + 1;
							if (final_target != data.target) { // For tooltips. -Pr0nogo
								target_ext->values[ValueSavantPowerSiphonDebuff]++;
								target_ext->timers[TimerSavantPowerSiphonDebuff] = 2 * 24 + 1;
							}
							helper::create_thingy(SpriteId::Feedback_Hit_Small + scbw::get_overlay_offset(data.target->id), data.target->position.x, data.target->position.y, data.attacking_player_id);
						}
					} break;
					case WeaponId::AutocratEminenceRifle:
					{
						// Apply Controlled Opposition
						target_ext->values[ValueAutocratControlledOppositionStacks]++;
						target_ext->values[ValueAutocratControlledOppositionPlayer] = data.attacking_player_id;
						target_ext->timers[TimerAutocratControlledOpposition] = 24 * 2 + 1;
					} break;
					case WeaponId::SevenstepGeometricRifle:
					case WeaponId::SevenstepGeometricRiflePiercing: {
						if (!target_ext->timers[TimerSevenstepMindMatter])
						{
							target_ext->timers[TimerSevenstepMindMatter] = 24 + 1;
						}
						if (attacker_ext->units[UnitSevenstepMindMatterSource]) {
							target_ext->units[UnitSevenstepMindMatterSource] = attacker_ext->units[UnitSevenstepMindMatterSource];
						} else {
							target_ext->units[UnitSevenstepMindMatterSource] = data.attacker;
						}
					} break;
					case WeaponId::PazuzuJupiterDriver:
					{
						// Deconstruction - Pazuzu behavior - apply stack
						if (unit_is_combatant(data.target)) {
							target_ext->values[ValueDeconstruction]++;
							target_ext->timers[TimerDeconstruction] = 5 * 24 + 1;
							target_ext->values[ValueDeconstructionPlayer] = data.attacker->player;
							u32 overlay = ImageId::DeconstructionSmall + scbw::get_overlay_offset(data.target->id);
							if (data.target->subunit) {
								UnitExt* subunit_ext = get_unit_ext(data.target->subunit);
								subunit_ext->values[ValueDeconstruction]++;
								subunit_ext->timers[TimerDeconstruction] = 5 * 24 + 1;
								subunit_ext->values[ValueDeconstructionPlayer] = data.attacker->player;
								if (!data.target->subunit->sprite->get_overlay(overlay)) {
									data.target->subunit->sprite->create_overlay(overlay);
								}
							} else {
								if (!data.target->sprite->get_overlay(overlay)) {
									data.target->sprite->create_overlay(overlay);
								}
							}
						}
					} break;
					case WeaponId::ClaymoreHarpoonRailgun:
					{
						// Claymore - Kelvin Munitions - Slow primary target
						target_ext->timers[TimerKelvinMunitions] = 3 * 24 + 1;
					} break;
					case WeaponId::GorgonArtificeBattery:
					{
						// Petrification - Gorgon behavior
						if (target_ext->values[ValuePetrificationStacks] < 20) {
							target_ext->values[ValuePetrificationStacks] += 1;
						}
						target_ext->timers[TimerPetrification] = 48 + 1;
					} break;
					case WeaponId::MagnetarNewtonianShockCannon:
					{
						// Magnetar - Law of Attraction - Pull target to attacker
						if ((!(units_dat::base_property[data.target->id] & UnitProperty::Building)
						||    (data.target->status & UnitStatus::InAir && data.target->main_order_id != OrderId::Liftoff && data.target->main_order_id != OrderId::BuildingLand))
						&& !(data.target->status & UnitStatus::Burrowed))
						{
							const u32 dist = scbw::get_distance_fast(data.attacker->get_x(), data.attacker->get_y(), data.target->get_x(), data.target->get_y());
							const u32 displacement_time = 5;
							Vector32 diff = scbw::get_vector_between(data.target->sprite->position, data.attacker->sprite->position);
							Vector32f displacement = scbw::get_normalized_vector(diff);
							displacement.x *= 2 * 32.0 / displacement_time;
							displacement.y *= 2 * 32.0 / displacement_time;
							target_ext->timers[TimerMagnetarAttraction] = displacement_time;
							target_ext->repulsion_vector = displacement;
						}
					} break;
					case WeaponId::HeraldStellarTorch:
					{
						if (!scbw::is_allied_to(data.attacking_player_id, data.target->player)) {
							// TODO: Audiovisuals
							target_ext->timers[TimerTriumphantRiseDebuff] = 36;
						}
					} break;
					case WeaponId::DemiurgeGalacticWeaver:
					{
						// Demiurge - Worship
						if (!scbw::is_allied_to(data.attacking_player_id, data.target->player)) {
							if (target_ext->timers[TimerDemiurgeWorship] == 0) {
								target_ext->timers[TimerDemiurgeWorship] = 10 * 24 + 1;
							}
							else if (target_ext->timers[TimerDemiurgeWorship] > 24) {
								target_ext->timers[TimerDemiurgeWorship] -= 24;
							}
							else if (target_ext->timers[TimerDemiurgeWorship] < 24) {
								target_ext->timers[TimerDemiurgeWorship] = 2;
							}
							target_ext->units[UnitDemiurge] = data.attacker;
						}
					} break;
					case WeaponId::EmpressFadingBioplasma:
					{
						// Empress - Anodyne Annex - add stack
						target_ext->units[UnitEmpressAnodyneAnnexSource] = data.attacker;
						target_ext->values[ValueEmpressAnodyneAnnexStacks]++;
						target_ext->values[ValueEmpressAnodyneAnnexSourcePlayer] = data.attacking_player_id;
						target_ext->timers[TimerEmpressAnodyneAnnex] = 2 * 24 + 1;
					} break;
					case WeaponId::GhitorokorGnashingTendrils:
					{
						attacker_ext->values[ValueGhitorokorRelentlessAggressionStacks]++;
						attacker_ext->timers[TimerGhitorokorRelentlessAggression] = 48 + 1;
					} break;
					case WeaponId::VithriliskCorrosiveSpores:
					{
						// Vithrilisk - Critical Mass
						// TODO: Unit search
						if (target_ext->values[ValueCorrosiveTouchStacks]) {
							s32 range = 160;
							scbw::UnitFinder(
								data.target->get_x() - 96,
								data.target->get_y() - 96,
								data.target->get_x() + 96,
								data.target->get_y() + 96
							).for_each([&](Unit* target) {
								if (target->status & UnitStatus::Burrowed) {
									return;
								}
								if (target->status & UnitStatus::Invincible) {
									return;
								}
								if (scbw::is_allied_to(data.attacker->player, target->player)) {
									return;
								}
								helper::apply_spores(target);
								Thingy* thingy = helper::create_thingy(SpriteId::Corrosive_Acid_Hit, target->get_x(), target->get_y(), 11);
								if (thingy) {
									thingy->sprite->elevation = target->sprite->elevation + 1;
									helper::set_thingy_visibility_flags(thingy);
								}
							});
						}
					} break;
					case WeaponId::ZoryusthalethMonstrousMaw: {
						// Zoryusthaleth - Swarming Omen
						if (!scbw::is_allied_to(data.attacking_player_id, data.target->player)) {
							target_ext->values[ValueSwarmingOmenPlayer] = data.attacking_player_id + 1;
						}
					} break;
					case WeaponId::KonviliskSowingSpines: {
						// Fertile Spines - incubate
						incubate(data.target, 1, UnitId::ZergRilirokor, data.attacking_player_id);
					} break;
					case WeaponId::TetcaerokorCleavingMaw: {
						// Virile Coating - incubate
						incubate(data.target, 1, UnitId::ZergRililisk, data.attacking_player_id);
					} break;
					case WeaponId::HierophantPhaseSignifier:
					{
						target_ext->values[ValueSignifyStacks]++;
						target_ext->timers[TimerSignify] = 3 * 24 + 1;
						target_ext->values[ValueSignifyPlayer] = data.attacking_player_id;
						scbw::update_signify_status(data.target, target_ext);
					} break;
					case WeaponId::EcclesiastAstralBlaster:
					{
						// Astral Blessing - Ecclesiast behavior
						if (data.attacking_player_id != data.target->player && !scbw::is_allied_to(data.attacking_player_id, data.target->player)) {
							scbw::astral_blessing_proc(data.target, data.attacking_player_id, data.weapon_id);
						}
					} break;
					case WeaponId::CharlatanFerventBlaster:
					{
						// Charlatan - Leading the Blind passive
						if (!scbw::is_allied_to(data.attacker->player, data.target->player)) {
							scbw::apply_hallucinating(data.target, data.attacker->player, 25);
						}
					} break;
					case WeaponId::PatriarchIronhandWeaver:
					{
						if (data.attacking_player_id != data.target->player
						&& !scbw::is_allied_to(data.attacking_player_id, data.target->player)
						&&  target_ext->timers[TimerPatriarchAstralRule] == 0)
						{
							target_ext->values[ValuePatriarchAstralRulePlayer] = data.attacking_player_id;
							target_ext->timers[TimerPatriarchAstralRule] = 24 + 1;
						}
					} break;
					case WeaponId::MagisterNeutronCannon:
					{
						// Stellar Enforcement - Magister behavior
						if (!scbw::is_allied_to(data.attacker->player, data.target->player) && !target_ext->values[TimerStellarEnforcement]) {
							target_ext->timers[TimerStellarEnforcement] = 3 * 24 + 1;
							target_ext->timers[TimerStellarEnforcementOverlay] = 30;
							scbw::add_unit_overlay(data.target, ImageId::StellarEnforcementDebuffSmall + scbw::get_overlay_offset(data.target->id, true));  // Inlined here because otherwise the unit would have the overlay 1 frame late! -Pr0nogo
						}
					} break;
				}
			}
		}
	}

	static void weapon_damage_lifesteal(const WeaponDamageData& data) {
		u32 damage_dealt = data.damage + data.shield_reduce_amount;
		UnitExt* attacker_ext = get_unit_ext(data.attacker);
		switch (data.weapon_id) {
			case WeaponId::GhitorokorGnashingTendrils:
			case WeaponId::TethzorokorSaberArray:
			case WeaponId::RilirokorToxicSpores:
			case WeaponId::RililiskSwarmingSpores:
			case WeaponId::UltrokorKaiserBlades:
			{
				// Broodspawn Death Cycle, Ghitorokor / Tethzorokor / Ultrokor Saber Siphon
				data.attacker->heal_hp(damage_dealt, data.attacker);
			} break;
			case WeaponId::MortothrokorWickedIncisors:
			{
				// Mortothrokor Whetted Appetite
				u32 percentage = attacker_ext->values[ValueMortothrokorStacks] * 0.05;
				data.attacker->heal_hp(damage_dealt * percentage, data.attacker);
			} break;
			case WeaponId::AmaranthPsiCleaver:
			{
				// Eternal Service - Amaranth behavior
				data.attacker->heal_shields(damage_dealt, data.attacker);
			} break;
		}
	}

	static bool weapon_damage_fatal(const WeaponDamageData& data) {
		bool fatal_deal_damage = true;

		UnitExt* target_ext = get_unit_ext(data.target);
		switch (data.weapon_id) {
			case WeaponId::Plague: {
				// Plague - Don't kill
				fatal_deal_damage = false;
				data.target->set_hitpoints(256);
			} break;
		}

		// Empress - Anodyne Annex - zombify target
		if (target_ext->values[ValueEmpressAnodyneAnnexStacks] && !target_ext->values[ValueIsZombified]) {
			fatal_deal_damage = false;
			scbw::empress_zombify(target_ext->values[ValueEmpressAnodyneAnnexSourcePlayer], target_ext->units[UnitEmpressAnodyneAnnexSource], data.target, target_ext->values[ValueEmpressAnodyneAnnexStacks]);
		}

		if (target_ext->values[ValueBlissfulEmbraceOriginalPlayer]) {
			// Update creep
			scbw::update_custom_creep_disappearance(data.target);

			// Remove creep from beneath
			//if (!helper::spreads_creep(data.target->id, true)) {
			//	scbw::clear_creep(scbw::unit_area(data.target));
			//}

			// Remove connected larva
			u16 larva_id = UnitId::ZergLarva;
			if (data.target->id == UnitId::ZergCaphrolosk || data.target->id == UnitId::ZergOcsalCesiant)
			{
				larva_id = UnitId::ZergOcsalLarva;
			}

			EntryArray larvae = {};
			unit_search_radial(&larvae, data.target->position, 4 * 32);
			unit_search_filter(&larvae, data.target, 0, {larva_id});
			for (u32 i = 0; i < larvae.count; ++i) {
				Unit* larva = larvae.units[i];
				if (larva->connected_unit == data.target) {
					larva->connected_unit = 0;
				}
			}

			scbw::unload_or_kill_loaded_units(data.target);

			// Recede control
			data.target->give_to(target_ext->values[ValueBlissfulEmbraceOriginalPlayer] - 1);
			target_ext->values[ValueBlissfulEmbraceOriginalPlayer] = 0;

			fatal_deal_damage = false;
			data.target->set_hitpoints(target_ext->values[ValueBlissfulEmbraceOriginalHealth]);

			// Make this a sprite
			u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
			if (units_dat::base_property[data.target->id] & UnitProperty::LargeOverlay) {
				effect_image_id = ImageId::ZergBuildingExplosion;
			} else if (units_dat::base_property[data.target->id] & UnitProperty::MediumOverlay) {
				effect_image_id = ImageId::ZergAirDeathExplosion_Large;
			}
			data.target->sprite->create_overlay(effect_image_id);
			helper::play_sound_from_unit(SoundId::Zerg_Bldg_ZBldgDth_WAV, data.target, true, 0);

			// Create busy overlays for buildings only
			u32 infested_image_id = ImageId::InfestedOverlay_Large;
			if (unit_id_is_smaller_building(data.target->id)) {
				infested_image_id = ImageId::InfestedOverlay_Small;
			}
			scbw::remove_unit_overlay(data.target, infested_image_id);
		}

		switch (data.target->id) { // TARGET SWITCH! MUST BE UNIT
			case UnitId::ZergVilgorokor: {
				// Vilgorokor - On Death behaviorw
				scbw::create_unit_from_ability(UnitId::ZergRilirokor, 2, data.target->player, data.target->position);
				scbw::transform_unit(data.target, UnitId::ZergVilgoleth);
				data.target->set_hitpoints(data.target->get_max_hp_raw());
				fatal_deal_damage = false;
			} break;
			case UnitId::TerranCuirassTankMode:
			{
				// Cuirass - Stonewall Protocol
				if (!target_ext->values[ValueCuirassIsSelfReviving]) {
					target_ext->values[ValueCuirassIsSelfReviving] = 1;
					data.target->hitpoints = CuirassSelfReviveHP;
					fatal_deal_damage = false;
					data.target->order_computer(OrderId::Stop);
					data.target->status |= UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
					helper::play_sound_from_unit(SoundId::Misc_TPwrDown_wav, data.target, 1, 0);
				}
			} break;
			case UnitId::ProtossVagrant:
			{
				helper::increment_unit_death_scores(data.target, data.target->player);
				helper::replace_unit_with_id(data.target, UnitId::ProtossVagrantShade);
				data.target->shields = units_dat::max_shield_points[data.target->id] * 256;
				data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id]);
				data.target->sprite->create_top_overlay(ImageId::ExplosionSmallBlueReversed);

				u32 sound = helper::get_random_sfx_between(SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04);
				helper::play_sound_from_unit(sound, data.target, g_options_list->flags & 1, 99);
				helper::do_unit_event_notification(data.target, 17, sfx_dat::portrait_duration[sound] + 1000 * (sfx_dat::sound_data[sound].duration_ms / 1000), 0, sound);
				fatal_deal_damage = false;
			} break;
			case UnitId::ProtossArchon:
			{
				// Archon - Astral Aegis
				scbw::UnitFinder(
					data.target->get_x() - 96,
					data.target->get_y() - 96,
					data.target->get_x() + 96,
					data.target->get_y() + 96
				).for_each([&](Unit* unit) {
					if (!units_dat::shields_enabled[unit->id]) {
						return;
					}
					u32 value = ArchonAstralAegisShieldHeal;
					unit->heal_shields(value, data.target);
				});
			} break;
			case UnitId::ProtossAurora:
			{
				// Last Orders - Aurora behavior
				if (target_ext->values[ValueLastOrdersInit] == 0) {
					target_ext->values[ValueLastOrdersInit] = 1;
					Unit* nearest = scbw::UnitFinder(
						data.target->get_x() - 9 * 32,
						data.target->get_y() - 9 * 32,
						data.target->get_x() + 9 * 32,
						data.target->get_y() + 9 * 32
					).get_nearest_target(data.target, [&](Unit* unit) {
						// Don't attack friendly / allied units
						if (unit->player == data.target->player) {
							return false;
						}
						if (scbw::is_allied_to(data.target->player, unit->player)) {
							return false;
						}
						if (unit->status & UnitStatus::Invincible) {
							return false;
						}
						if (unit->status & UnitStatus::InAir) {
							return false;
						}
						if (!unit->is_visible_to(data.target->player)) {
							return false;
						}
						if (scbw::get_distance_fast(data.target->position.x, data.target->position.y, unit->position.x, unit->position.y) > 9 * 32) {
							return false;
						}
						return true;
					});
					if (nearest) {
						fatal_deal_damage = false;
						data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id] / 2);
						data.target->order_to(OrderId::Move, nearest->position.x, nearest->position.y);
						data.target->status |= UnitStatus::CanNotReceiveOrders;
						data.target->sprite->create_top_overlay(ImageId::GodsMadeFlesh_Small);
					}
				}
			} break;
			case UnitId::ZergAxitrilisk:
			{
				helper::increment_unit_death_scores(data.target, data.target->player);
				helper::replace_unit_with_id(data.target, UnitId::ZergAxitriliskWeaponless);
				data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id]);
				data.target->sprite->create_top_overlay(ImageId::ZergAirDeathExplosion_Small);
				helper::play_sound_from_unit(SoundId::Zerg_Hydra_ZHyDth00_WAV, data.target, 1, 0);
				target_ext->timers[TimerAxitriliskPerpetualWind] = 8 * 24 + 1;
				fatal_deal_damage = false;
			} break;
		}

		if (data.attacker) {
			UnitExt* attacker_ext = get_unit_ext(data.attacker);
			switch (data.weapon_id) {
				// Alkajelisk Parasitic Spores were here
			}

			// Mind Control - energy restore
			if (attacker_ext->units[UnitMindController]) {
				Unit* mind_controller = attacker_ext->units[UnitMindController];
				mind_controller->add_unit_energy(MindControlEnergyGain);
				mind_controller->sprite->create_underlay(ImageId::SmallExplosion_Unused);
			}
		}

		// Fix burrowed regression / transforms. -DF
		if (data.target->status & UnitStatus::Burrowed)
		{
			data.target->main_order_id = OrderId::Unburrow;
		}

		u32 regression_id = scbw::get_zerg_regression(data.target);
		if (data.damage > 0 && regression_id != UnitId::None) {
			if (data.target->id != UnitId::ZergExcisant) {
				fatal_deal_damage = false;
			}
			if (data.target->id == UnitId::ZergNydusCesiant) {
				Unit* nydus_exit = data.target->building.nydus_exit;
				if (nydus_exit) {
					data.target->building.nydus_exit = 0;
					nydus_exit->building.nydus_exit = 0;

					if (!target_ext->values[ValueIsNydusExit] == 1) {
						nydus_exit->remove();
					}
				}
			}
			scbw::regress(data.target, regression_id);
			if (data.target) {
				unit_on_death_behaviors(data.target);
			}
		}

		return fatal_deal_damage;
	}

	void apply_ensnare(Unit* unit) { // 004F45E0
		// Overlay
		if (!(unit->ensnare_timer) && !(unit->status & UnitStatus::Burrowed)) {
			scbw::add_unit_overlay(unit, ImageId::EnsnareOverlay_Small + scbw::get_overlay_offset(unit->id));
		}

		// Effect
		unit->ensnare_timer = 5;
		unit->update_speed();
	}

	u16 get_spores_overlay_id(Unit* target) {
		UnitExt* target_ext = get_unit_ext(target);
		u32 stacks = target_ext->values[ValueCorrosiveTouchStacks];
		u32 overlay_multiplier = 0;
		if (stacks > 18) {
			overlay_multiplier++;
		}
		if (stacks > 35) {
			overlay_multiplier++;
		}
		if (stacks > 50) {
			overlay_multiplier++;
		}
		return overlay_multiplier + 4 * scbw::get_overlay_offset(target->id) + ImageId::AcidSpores_1_Overlay_Small;
	}

	// Hooked to change Corrosive Spores timer - Pr0nogo
	s32 apply_spores_hook(Unit* target) { // 004F4480
		UnitExt* target_ext = get_unit_ext(target);
		target_ext->values[ValueCorrosiveTouchStacks]++;
		Image* image = target->get_overlay(get_spores_overlay_id(target));
		u32 overlay = 0;
		if (image) {
			overlay = image->id;
		}

		if (!overlay) {
			target->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
			u32 spores_image = helper::get_corrosive_spores_image(target);
			Unit* final_unit = target->subunit;
			if (!final_unit) {
				final_unit = target;
			}
			Sprite* final_sprite = final_unit->sprite;
			final_sprite->create_overlay(spores_image, 0, 0, 0);
		}

		target_ext->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
		return 0;
	}

	// Hooked to reveal fog around attackers when they attack allies. -Pr0nogo
	void unit_was_hit(Unit* victim, Unit* attacker, u32 reveal) { // 004795D0
		if (attacker) {
			if (attacker->player != victim->player) {
				victim->last_attacking_player = attacker->player;
				if (victim->unit_ai) {
					if (!victim->auto_target_unit) {
						victim->auto_target_unit = attacker;
						if (victim->subunit) {
							if (units_dat::base_property[victim->subunit->id] & UnitProperty::Subunit) {
								victim->subunit->last_attacking_player = victim->last_attacking_player;
								victim->subunit->auto_target_unit = attacker;
							}
						}
					}
				}
				helper::ai_unit_was_hit(victim, attacker, 1, 0);
				if (reveal && attacker->sprite) {
					if (!helper::unit_can_see_cloaked_target(attacker, victim)) {
						scbw::reveal_attacker(attacker, victim);
						if (attacker->id == UnitId::ProtossSteward && attacker->hangar_unit.parent) {
							scbw::reveal_attacker(attacker->hangar_unit.parent, victim);
						}
					}
				}
				if (attacker->status & UnitStatus::InBuilding && attacker->connected_unit) { // This nullcheck is temporary until we fix the source of the issue. -Pr0nogo
					attacker = attacker->connected_unit;
				}
				if (victim->main_order_id != OrderId::HoldPositionNormal && helper::is_better_target(victim, victim->auto_target_unit, attacker)) {
					helper::react_to_hit(attacker, victim);
				}
				if (!victim->unit_ai) {
					EntryArray units_in_bounds = {};
					unit_search_radial(&units_in_bounds, victim->position, 96);
					for (u32 i = 0; i < units_in_bounds.count; ++i) {
						Unit* unit = units_in_bounds.units[i];
						if (unit != victim && unit->player == victim->player) {
							helper::ask_for_help(attacker, unit);
						}
					}
				}
			}
		}
	}

	// Hooked to investigate crashes. -Pr0nogo
	u32 ask_for_help(Unit* attacker, Unit* helper) { // 00476E60
		// Originally had code to prohibit Didact and Danimoth unit IDs.
		if (!(units_dat::base_property[helper->id] & UnitProperty::Worker)
		&&  !(helper->status & UnitStatus::Burrowed)) {
			if (helper::ai_unit_can_attack(helper)) {
				if (helper::is_better_target(helper, helper->auto_target_unit, attacker)) {
					Unit* subunit = helper->subunit;
					helper->auto_target_unit = attacker;
					if (helper::unit_is_subunit(subunit)) {
						subunit->last_attacking_player = helper->last_attacking_player;
						helper->subunit->auto_target_unit = attacker;
					}
					return true;
				}
			}
		}
		return false;
	}

	// Hooked to stop damage from forcing units to unburrow. -Pr0nogo
	void react_to_hit(Unit* attacker, Unit* victim) { // 004776C0
		if (!unit_id_is_larva(victim->id)) {
			if (!(victim->status & UnitStatus::Burrowed) || victim->irradiate_timer || victim->id == UnitId::ZergLakizilisk) {
				if  (units_dat::base_property[victim->id] & UnitProperty::Building || helper::unit_can_attack_target(attacker, victim, 1)
				&& !(units_dat::base_property[victim->id] & UnitProperty::Worker)) {
					victim->auto_target_unit = attacker;
					Unit* subunit = victim->subunit;
					if (subunit && units_dat::base_property[subunit->id] & UnitProperty::Subunit) {
						subunit->last_attacking_player = victim->last_attacking_player;
						subunit->auto_target_unit = attacker;
					}
				} else if (victim->id != UnitId::ZergLakizilisk || !(victim->status & UnitStatus::Burrowed) || victim->unit_ai && !victim->irradiate_timer) {
					if (units_dat::base_property[victim->id] & UnitProperty::Subunit) {
						victim = victim->subunit;
					}

					UnitExt* victim_ext = get_unit_ext(victim);
					if (scbw::unit_can_flee(victim))
					{
						Target flee_target = helper::ai_prepare_to_flee(victim, attacker);
						helper::order_interrupt(victim, 0, flee_target.pt.x, flee_target.pt.y, OrderId::Move);
					}
				}
			} else if (units_dat::base_property[victim->id] & UnitProperty::Burrowable && g_player_table[victim->player].type == PlayerType::Computer)
			{
				helper::unburrow(victim); // For now, we unburrow AI units only. -Pr0nogo
			}
		}
	}
}