#include "globals.h"
#include "hook_tools.h"
#include "hooks/cosmonarchy_finder.h"
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapons/weapon_fire.h"
#include <algorithm>
#include <unordered_set>
#include <Utils/captivating_claws.h>
#include "hooks/weapons/weapon_damage.h"

void IquareGlobals::aise_save() {
	bw::print("WARNING: Saving doesn't work properly in Cosmonarchy.\n\t\t\tAny plugin specific data will be lost.", GameTextColor::Yellow);
	return;
}

void IquareGlobals::aise_load(const char* ptr, u32 len) {
	return;
}

void IquareGlobals::unit_death(Unit* unit) {
	for (auto& effect : m_effects_to_add) {
		if (effect.source == unit) {
			effect.source = nullptr;
		}
		if (effect.origin == unit) {
			effect.origin = nullptr;
		}
	}
	for (auto& effect : m_effects) {
		if (effect.source == unit) {
			effect.source = nullptr;
		}
		if (effect.origin == unit) {
			effect.origin = nullptr;
		}
	}

	m_lakizilisk_spine_hits[unit].clear();
	for (auto& a : m_lakizilisk_spine_hits) {
		for (s32 b = 0; b < a.second.size(); b++) {
			if (a.second[b] == unit) {
				a.second.erase(a.second.begin() + b);
				b--;
			}
		}
	}
}

void apply_irradiate(Unit* unit, u8 player, u32 time) {
	if (time == 0) {
		return;
	}
	// if not already irradiated and not burrowed, add the irradiate overlay
	if (!unit->irradiate_timer && !(unit->status & UnitStatus::Burrowed)) {
		u32 irradiate_image_id = ImageId::IrradiateNew_Small + bw::get_overlay_offset(unit->id);
		bw::add_unit_overlay(unit, irradiate_image_id);
	}
	unit->irradiate_caster = 0;
	unit->irradiate_player_id = player;
	unit->irradiate_timer = time;
	hooks::play_attack_notification(unit);
}

struct EffectInfo {
	bw::SoundSet start_sound;
	bw::SoundSet loop_sound;
	bw::SoundSet end_sound;
};

static const EffectInfo& get_effect_info(EffectId effect_id) {
	switch (effect_id) {
		case EffectId::PsiStorm: {
			static EffectInfo info{
				.start_sound = SoundId::PsiStormStart,
				.loop_sound = SoundId::PsiStormLoop,
				.end_sound = SoundId::PsiStormEnd,
			};
			return info;
		} break;
	}
	static EffectInfo info{};
	return info;
}

void EffectField::destroy() {
    if (playing_sound) {
        playing_sound.Stop();
    }
    const EffectInfo& effect_info = get_effect_info(id);
    if (effect_info.end_sound) {
        bw::play_sound(effect_info.end_sound);
    }

    current_time = 0;
    intervals = 0;
}

EffectRef IquareGlobals::create_effect(EffectId id, u32 time, Point16 position, SearchRadius radius, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras) {
	EffectField& effect_field = m_effects_to_add.emplace_back(EffectField{
		.ref = ++m_next_effect_field_ref,
		.id = id,
		.position = position,
		.radius = radius,
		.current_time = time + 1, // We decrement before tick, so first tick will have current_time == max_time
		.max_time = time,
		.sprite_id = sprite_id,
		.elevation = elevation,
		.player = player,
		.thingy = nullptr,
		.source = source,
		.origin = extras.origin,
		.intervals = extras.intervals,
		.move_target = extras.move_target,
		.move_speed = extras.move_speed,
	});
	const EffectInfo& effect_info = get_effect_info(id);
	if (effect_info.start_sound) {
		effect_field.playing_sound = bw::play_sound(effect_info.start_sound, effect_field.position);
	}
	return effect_field.ref;
}

EffectRef IquareGlobals::create_effect(EffectId id, u32 time, Point16 position, Point16 size, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras) {
	return create_effect(id, time, position, SearchRadius{size}, sprite_id, elevation, player, source, extras);
}

EffectRef IquareGlobals::create_effect(EffectId id, u32 time, Point16 position, u32 radius, SpriteId::Enum sprite_id, u32 elevation, u32 player, Unit* source, const EffectFieldExtras& extras) {
	return create_effect(id, time, position, SearchRadius{radius}, sprite_id, elevation, player, source, extras);
}

EffectField* IquareGlobals::get_effect(EffectRef ref) {
	if (!ref) {
		return 0;
	}
	if (auto it = std::ranges::find_if(m_effects, [ref](const EffectField& effect) { return effect.ref == ref; }); it != m_effects.end()) {
		return &*it;
	}
	if (auto it = std::ranges::find_if(m_effects_to_add, [ref](const EffectField& effect) { return effect.ref == ref; }); it != m_effects_to_add.end()) {
		return &*it;
	}
	return 0;
}

static void tick_effect(EffectField& effect) {
	const EffectInfo& effect_info = get_effect_info(effect.id);
    if (effect_info.loop_sound && !effect.playing_sound) {
        effect.playing_sound = bw::play_looping_sound(effect_info.loop_sound, effect.position);
    }

	if (effect.current_time && effect.sprite_id != SpriteId::None) {
		if (!effect.thingy || !effect.thingy->sprite) {
			effect.thingy = bw::create_and_init_thingy(effect.sprite_id, effect.position.x, effect.position.y, effect.player, effect.elevation);
		}
	}

    if (effect.move_target && effect.position != effect.move_target) {
        const auto distance = bw::distance_between(effect.position, effect.move_target);
        auto move_speed = effect.move_speed / 24.0;
        if (distance < move_speed) {
            effect.position = effect.move_target;
        } else {
            auto dir = bw::dir_between(effect.position, effect.move_target);
            effect.position.x += dir.x * move_speed;
            effect.position.y += dir.y * move_speed;
        }
        if (effect.thingy) {
            effect.thingy->sprite->set_position(effect.position.x, effect.position.y);
        }
    }

	const s32 update_period = 4;
	const u32 elapsed = effect.max_time - effect.current_time;
	if (elapsed % update_period == 0) {
		effect.tick++;
		const f64 tick_scalar     = f64(update_period) / 24;
		const f64 total_scalar    = f64(update_period) / effect.max_time;
		const f64 tick_scalar_hp  = 256.0 * tick_scalar;
		const f64 total_scalar_hp = 256.0 * total_scalar;
		switch (effect.id) {
			case EffectId::StarSovereignTheGriefOfAllGods: {
				if (!effect.origin) {
					effect.destroy();
					return;
				}

				for (Unit* unit : g_unit_search.square_ex(effect, FilterNoAir)) {
					unit->damage_with_custom(2500 * total_scalar_hp, -1, effect.source, effect.player);
				}

				avenge_or_ascend_corpses_in_area(effect.position, effect.radius, effect.player);
			} break;
			case EffectId::SalamanderFire: {
				for (Unit* target : g_unit_search.search_ex(effect, FilterNoAir)) {
					UnitExt* target_ext = get_unit_ext(target);
					if (!target_ext->timers[TimerSalamanderFire]) { // So it only applies to the same unit once every second.
						target_ext->values[ValueApollyoidAdhesiveStacks]++;
						target_ext->timers[TimerApollyoidAdhesive] = 2 * 24 + 1;
						target_ext->timers[TimerSalamanderFire] = 24;
					}
				}
			} break;
			case EffectId::AspirantCriticality: {
				for (Unit* unit : g_unit_search.search_ex(effect, FilterIncludingSelf | FilterNoAir)) {
					unit->heal_shields_then_hp(4 * tick_scalar_hp, effect.source, true);
				}
			} break;
			case EffectId::KeskathalorCausticDischarge: {
				for (Unit* unit : g_unit_search.search_ex(effect, FilterIncludingSelf | FilterNoAir)) {
					UnitExt* unit_ext = get_unit_ext(unit);
					unit->damage_with_custom(30 * total_scalar_hp, -1, effect.source, effect.player);
					unit_ext->timers[TimerCausticDischargeSpeed] = 3 * 24 + 1;
				}
			} break;
			case EffectId::DiademIonField: {
				for (Unit* unit : g_unit_search.search_ex(effect, FilterIncludingSelf | FilterNoAir)) {
					UnitExt* unit_ext = get_unit_ext(unit);
					unit->damage_with_custom(100 * total_scalar_hp, -1, effect.source, effect.player);
					unit_ext->timers[TimerIonFieldAttackSpeed] = 2 * 24 + 1;
				}
			} break;
			case EffectId::PsiStorm: {
				Unit* caster = effect.source;
				if (!caster) {
					effect.destroy();
					break;
				}
				UnitExt* caster_ext = get_unit_ext(caster);
				if (caster_ext->current_storm != effect.ref) {
					effect.destroy();
					break;
				}
				auto distance = bw::distance_between(caster->position, (Point16)effect.position);
				if (distance > weapons_dat::max_range[WeaponId::PsiStorm] + 64) {
					caster_ext->current_storm = 0;
					effect.destroy();
					break;
				}

				caster->spend_energy(20 * tick_scalar_hp);
				for (Unit* unit : g_unit_search.square_ex(effect, FilterIncludingSelf)) {
					if (unit->is_under_storm) continue;
					unit->is_under_storm = update_period;
					unit->damage_with(WeaponId::PsiStorm, 25 * tick_scalar_hp, effect.source, effect.player,
						helper::get_direction_from_points(effect.position, unit->position), units_dat::base_property[unit->id] & UnitProperty::Building ? 2 : 1);
				}

				if (!caster->energy) {
					caster_ext->current_storm = 0;
					effect.destroy();
					break;
				}
			} break;
			case EffectId::HallucinatingField: {
				for (Unit* unit : g_unit_search.search_ex(effect, FilterIncludingSelf)) {
					bw::apply_hallucinating(unit, effect.player, 3 * 24 + 1);
				}
			} break;
			case EffectId::Irradiate: {
				for (Unit* unit : g_unit_search.search_ex(effect, FilterIncludingSelf)) {
					apply_irradiate(unit, effect.player, 15);
				}
			} break;
			case EffectId::MorningstarGroundZero: {
				switch (effect.tick) {
					case 0: {
						g_iquare_globals->create_effect(EffectId::Irradiate, 20 * 24 - elapsed, effect.position, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect.player, effect.source);
					} break;
					case 1:
					case 2: {
						f32 distance = effect.tick * 2 * 32;
						u32 count = effect.tick * 8;

						Point16 pos = effect.position;
						pos.x += distance;
						for (u32 i = 0; i < count; i++) {
							pos = bw::rotate_point((Point16)effect.position, pos, (360.0 / count));
							g_iquare_globals->create_effect(EffectId::Irradiate, 20 * 24 - elapsed, pos, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, effect.player, effect.source);
						}
					} break;
				}
			} break;
			case EffectId::LanifectDisruptionField: {
				if (!effect.origin) {
					effect.destroy();
					break;
				}

				UnitExt* origin_ext = get_unit_ext(effect.origin);
				if (!origin_ext->values[ValueDisruptionFieldOn]) {
					effect.destroy();
					break;
				}

				bw::UnitFinder(
					effect.position.x - 60,
					effect.position.y - 40,
					effect.position.x + 60,
					effect.position.y + 40
				).for_each_in_match(cosmonarchy_finder_match(0, 0, {}), [&](Unit* unit) {
					UnitExt* unit_ext = get_unit_ext(unit);
					unit_ext->timers[TimerUnderDisruptionField] = 8;
					unit->status |= UnitStatus::CanNotAttack;
					if (unit->subunit) {
						unit->subunit->status |= UnitStatus::CanNotAttack;
					}
					if (unit_id_is_garrison(unit->id)) {
						for (s32 i = 0; i < TransportCap; i++) {
							Unit* cargo_unit = unit->get_loaded_unit(i);
							if (!cargo_unit) continue;
							if (cargo_unit) {
								cargo_unit->status |= UnitStatus::CanNotAttack;
								if (cargo_unit->subunit) {
									cargo_unit->subunit->status |= UnitStatus::CanNotAttack;
								}
							}
						}
					}
				});
			} break;
			case EffectId::IsthrathalethPlagueField: {
				bw::UnitFinder(
					effect.position.x - 1.5 * 32,
					effect.position.y - 1 * 32,
					effect.position.x + 1.5 * 32,
					effect.position.y + 1 * 32
				).for_each([&](Unit* unit) {
					helper::plague_proc(unit, 0);
					UnitExt* unit_ext = get_unit_ext(unit);
					unit_ext->units[UnitPlagueSource] = effect.origin;
				});
			} break;
			case EffectId::ReclamationField: {
				if (effect.current_time) {
					break;
				}

				Thingy* corpse = get_closest_active_corpse(effect.position, effect.radius);
				if (corpse) {
					reclaim_corpse(corpse, effect.player);
					effect.destroy();
					break;
				}
			} break;
			case EffectId::VilgolethHeal: {
				for (Unit* target : g_unit_search.search_ex(effect, FilterIncludingSelf | FilterNoAir)) {
					UnitExt* target_ext = get_unit_ext(target);
					target_ext->timers[TimerNathreliskMitosalFog] = 24;
				}
			} break;
			case EffectId::CyclopsRecursionRound: {
				if (effect.current_time || !effect.source) {
					break;
				}
				
				helper::create_bullet(WeaponId::CyclopsBrontesPayload, effect.source, effect.position.x, effect.position.y, effect.player, 0);
                for (Unit* target : g_unit_search.search_ex(effect, FilterEnemy)) {
                    target->sprite->create_top_overlay(ImageId::CyclopsFragmentaryExplosion);
                }
                u32 sound_id = bw::rand_between(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04);
                bw::play_sound(sound_id, effect.position);
                helper::create_thingy(SpriteId::FragmentaryExplosion, effect.position.x, effect.position.y, effect.player);
			} break;
			case EffectId::CyclopsFuturistRecursionRound: {
				if (effect.current_time || !effect.source) {
					break;
				}
				
				helper::create_bullet(WeaponId::CyclopsBrontesPayload, effect.source, effect.position.x, effect.position.y, effect.player, 0);
                for (Unit* target : g_unit_search.search_ex(effect, FilterEnemy)) {
                    target->sprite->create_top_overlay(ImageId::CyclopsFragmentaryExplosion);
                }
                u32 sound_id = bw::rand_between(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04);
                bw::play_sound(sound_id, effect.position);
                helper::create_thingy(SpriteId::FragmentaryExplosion, effect.position.x, effect.position.y, effect.player);
			} break;
			case EffectId::SevenstepMindMatter: {
				if (effect.source && effect.source->sprite && !effect.current_time) {
					if (effect.origin && effect.origin->sprite) {
						UnitExt* origin_ext = get_unit_ext(effect.origin);
						origin_ext->timers[TimerSevenstepMindMatterCarrier] = 2;
					}
					bw::artificial_bullet_from(WeaponId::SevenstepGeometricRiflePiercing, effect.player, effect.source, 0, effect.source->position, effect.position);
				}
			} break;
			case EffectId::GladiusBindingPlasma: {
				if (effect.current_time) {
					break;
				}

				EntryArray gladiuses = g_unit_search
					.radial(effect.position, 8 * 32)
					.filter(nullptr, 0, {UnitId::ProtossGladius})
                    .filter_by([&effect](Unit* unit) {
						return bw::is_allied_to(unit->player, effect.player);
					})
					.sort_by_distance(effect.position);
				if (!gladiuses.count()) {
					effect.destroy();
					break;
				}

				Unit* gladius = gladiuses.first();
				const auto targets = bw::binding_plasma_find_targets(effect.position, nullptr, effect.player, effect.intervals);
				if (targets.empty()) {
					effect.destroy();
					break;
				}
				assert(targets.size() <= effect.intervals);
				for (Unit* target : targets) {
					bw::artificial_bullet_from(WeaponId::GladiusPlasmaFlurry, effect.player, gladius, target, target->position, effect.position);
					u32 sound_id = bw::rand_between(SoundId::GladiusBindingPlasmaProc01, SoundId::GladiusBindingPlasmaProc03);
					helper::play_sound_at_pos(sound_id, effect.position, 1, 0);
					effect.intervals--;
				}
			} break;
		}
	}
}

void IquareGlobals::tick() {
	// Adds a frame of delay, but is necessary for effects spawning effects, where vector iterator would get invalidated otherwise
	m_effects.reserve(m_effects_to_add.size());
	std::ranges::move(m_effects_to_add, std::back_inserter(m_effects));
	m_effects_to_add.clear();

	for (EffectField& effect : m_effects) {
		if (effect.origin) {
			effect.position = effect.origin->position;
			if (effect.thingy) {
				effect.thingy->sprite->set_position(effect.origin->position.x, effect.origin->position.y);
			}
		}

        effect.current_time--;
		tick_effect(effect);

		if (!effect.current_time) {
			if (effect.thingy && effect.thingy->sprite) {
				effect.thingy->sprite->flags |= SpriteFlags::RemoveAfterDeath;
				effect.thingy->sprite->play_iscript_animation(IscriptAnimation::Death);
			}
			effect.thingy = nullptr;

			if (effect.intervals) {
				effect.intervals--;
			}
			if (effect.intervals) {
				effect.current_time = effect.max_time;
				effect.tick = 0;
			}
		}
	}
	std::erase_if(m_effects, [](EffectField& effect) { return !effect.current_time && !effect.intervals; });
}

void IquareGlobals::remove_unit_from_line_attack_table(Unit* unit, bool source_only) {
	for (s32 i = 0; i < m_line_attack_hits.size(); i++) {
		if (m_line_attack_hits[i].source == unit || (m_line_attack_hits[i].target == unit && !source_only)) {
			m_line_attack_hits.erase(m_line_attack_hits.begin() + i);
			i--;
		}
	}
}

void IquareGlobals::remove_multihit_bullet(Bullet* bullet) {
	for (s32 i = 0; i < m_multihit_bullets.size(); i++) {
		if (m_multihit_bullets[i].bullet == bullet) {
			m_multihit_bullets.erase(m_multihit_bullets.begin() + i);
			i--;
		}
	}
}

bool IquareGlobals::was_hit_by_line_attack(Unit* source, Unit* target) {
	for (auto a : m_line_attack_hits) {
		if (source == a.source && target == a.target) {
			return true;
		}
	}
	return false;
}

void IquareGlobals::register_line_attack_hit(Unit* source, Unit* target) {
	LineAttackHit hit{
		.source = source,
		.target = target,
	};
	m_line_attack_hits.push_back(hit);
}

bool IquareGlobals::was_already_hit_by_multihit_bullet(Bullet* bullet, Unit* target) {
	for (auto& multihit_bullet : m_multihit_bullets) {
		if (bullet == multihit_bullet.bullet) {
			for (auto already_hit : multihit_bullet.already_hit) {
				if (already_hit == target) {
					return true;
				}
			}
		}
	}
	return false;
}

bool IquareGlobals::multihit_bullet_is_set(Bullet* bullet) {
	for (auto& a : m_multihit_bullets) {
		if (bullet == a.bullet) {
			return true;
		}
	}
	return false;
}

void IquareGlobals::add_bullet_multihit(Bullet* bullet, Unit* target) {
	for (auto& a : m_multihit_bullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 0);
	m_multihit_bullets.push_back(gb);
}

void IquareGlobals::add_incremental_bullet_multihit(Bullet* bullet, Unit* target) {
	for (auto& a : m_multihit_bullets) {
		if (a.bullet == bullet) {
			a.already_hit.push_back(target);
			return;
		}
	}
	MultihitBullet gb(bullet, {target}, 1);
	m_multihit_bullets.push_back(gb);
}

s32 IquareGlobals::get_divider(Bullet* bullet) {
	for (auto& a : m_multihit_bullets) {
		if (a.bullet == bullet && a.divider != 0) {
			return a.divider;
		}
	}
	return 1;
}

u32 IquareGlobals::get_multihit_count(Bullet* bullet) {
	for (auto& a : m_multihit_bullets) {
		if (a.bullet == bullet) {
			return a.already_hit.size();
		}
	}
	return 0;
}

void IquareGlobals::add_multihit_empty_bullet(Bullet* bullet, s32 divider) {
	for (auto& a : m_multihit_bullets) {
		if (a.bullet == bullet) {
			a.divider = divider;
			return;
		}
	}
	MultihitBullet gb(bullet, {}, divider);
	m_multihit_bullets.push_back(gb);
}

void IquareGlobals::clear_lakizilisk(Unit* lakizilisk) {
	m_lakizilisk_spine_hits[lakizilisk].clear();
}

void IquareGlobals::add_lakizilisk_spine_pair(Unit* lakizilisk, Unit* target) {
	m_lakizilisk_spine_hits[lakizilisk].push_back(target);
}

bool IquareGlobals::spine_pair_exists(Unit* lakizilisk, Unit* target) {
	for (auto a : m_lakizilisk_spine_hits) {
		for (auto b : a.second) {
			if (a.first == lakizilisk && b == target) {
				return true;
			}
		}
	}
	return false;
}

void __declspec(naked) globals_save_state_wrapper() {
	static void* state;
	__asm {
		PUSHAD
	}
	g_iquare_globals->aise_save();
	__asm {
		POPAD
		MOV EAX, state
		RETN
	}
}

void __declspec(naked) globals_load_state_wrapper() {
	static const char* pointer;
	static u32 len;
	__asm {
		MOV len, ECX
		MOV pointer, EAX
		PUSHAD
	}
	g_iquare_globals->aise_load(pointer, len);
	__asm {
		POPAD
		RETN
	}
}

void inject_globals_hooks() {
	jump_patch(globals_save_state_wrapper, 0x004AAF23);
	jump_patch(globals_load_state_wrapper, 0x004AAE94);
}

namespace bw {

void refresh_debuffs(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);

	// NOTE: classic period formula is (([real seconds / 0.042) / 8)
	// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

	// Refresh Ensnare
	if (unit->ensnare_timer) {
		unit->ensnare_timer = MatralethEnsnareTimer;
		// specific update following ensnare removal
		unit->update_speed();
	}

	// Refresh Plague
	if (unit->plague_timer) {
		unit->plague_timer = IsthrathalethPlagueTimer;
	}

	// Refresh Observance
	if (unit_ext->timers[TimerObservanceDebuff]) {
		unit_ext->timers[TimerObservanceDebuff] = SeraphObservanceDebuffTimer;
	}

	// Refresh Hallucination
	if (unit_ext->timers[TimerHallucinating]) {
		unit_ext->timers[TimerHallucinating] = 3 * 24 + 1;
	}

	// Refresh Captivating ZethrokorClaws
	if (unit_ext->timers[TimerCaptivatingClaws]) {
		unit_ext->timers[TimerCaptivatingClaws] = VorvrokorCaptivatingClawsTimer;
	}

	// Refresh Lockdown
	if (unit->lockdown_timer) {
		unit->lockdown_timer = LobotomyMineLockdownTimer;
	}

	// Refresh Maelstrom
	if (unit->maelstrom_timer) {
		unit->maelstrom_timer = MindTyrantMaelstromTimer;
	}

	// Refresh Devourer Corrosive Spores
	if (unit_ext->timers[TimerCorrosiveTouch]) {
		unit_ext->timers[TimerCorrosiveTouch] = VithriliskCorrosiveTouchTimer;
	}

	// Refresh Irradiate
	if (unit->irradiate_timer) {
		unit->irradiate_timer = SeraphIrradiateTimer;
	}

	// Refresh Hierophant Signify
	if (unit_ext->timers[TimerSignify]) {
		unit_ext->timers[TimerSignify] = 3 * 24 + 1;
	}

	//was hardcoded in original code
	bw::refresh_console();
}

void inherit_debuffs(Unit* parent, Unit* child) {
	spread_debuffs(parent, child);

	UnitExt* parent_ext = get_unit_ext(parent);
	UnitExt* child_ext = get_unit_ext(child);

	// NOTE: classic period formula is (([real seconds / 0.042) / 8)
	// e.g. (([15] / 0.042) / 8) = 44.642, rounded up to 45

	// Get overlay info
	s32 overlay_offset = get_overlay_offset(child->id);

	// Inherit Ensnare
	if (parent->ensnare_timer) {
		child->ensnare_timer = parent->ensnare_timer;
		if (!(child->sprite->get_overlay(ImageId::EnsnareOverlay_Small + overlay_offset))) {
			child->sprite->create_overlay(ImageId::EnsnareOverlay_Small + overlay_offset);
		}
		child->update_speed();
	}

	// Inherit Plague
	if (parent->plague_timer != 0) {
		child->plague_timer = parent->plague_timer;
		if (!(child->sprite->get_overlay(ImageId::PlagueOverlay_Small + overlay_offset))) {
			child->sprite->create_overlay(ImageId::PlagueOverlay_Small + overlay_offset);
		}
	}

	// Inherit Incubation
	if (child_ext->timers[TimerIncubationArm]) {
		child_ext->timers[TimerIncubationArm] = parent_ext->timers[TimerIncubationArm];
		child_ext->values[ValueIncubationCount] = parent_ext->values[ValueIncubationCount];
		child_ext->values[ValueIncubationUnitId] = parent_ext->values[ValueIncubationUnitId];
		child_ext->values[ValueIncubationPlayer] = parent_ext->values[ValueIncubationPlayer];
		child->sprite->create_overlay(ImageId::ParasiteOverlay);
	}

	// Inherit Observance
	if (parent_ext->timers[TimerObservanceDebuff] > 0) {
		child_ext->timers[TimerObservanceDebuff] = parent_ext->timers[TimerObservanceDebuff];
		child_ext->timers[TimerObservancePeriod] = parent_ext->timers[TimerObservancePeriod];
		child_ext->values[ValueObservanceRend] = parent_ext->values[ValueObservanceRend];
	}

	// Inherit Knockout Drivers
	if (parent_ext->timers[TimerPetrification] > 0) {
		child_ext->timers[TimerPetrification] = parent_ext->timers[TimerPetrification];
	}

	// Inherit Hallucination
	if (parent_ext->timers[TimerHallucinating] > 0) {
		child_ext->timers[TimerHallucinating] = parent_ext->timers[TimerHallucinating];
	}

	// Inherit Captivating ZethrokorClaws
	if (auto timer = parent_ext->timers[TimerCaptivatingClaws]) {
		child_ext->timers[TimerCaptivatingClaws] = timer;
		parent_ext->unit_vectors[UnitVectorCaptivatingClaws] = child_ext->unit_vectors[UnitVectorCaptivatingClaws];
	}

	// Inherit Lockdown
	if (parent->lockdown_timer != 0) {
		child->lockdown_timer = parent->lockdown_timer;
	}

	// Inherit Maelstrom
	if (parent->maelstrom_timer != 0) {
		child->maelstrom_timer = parent->maelstrom_timer;
	}

	// Inherit Devourer Corrosive Spores
	if (parent_ext->timers[TimerCorrosiveTouch]) {
		child_ext->values[ValueCorrosiveTouchStacks] = parent_ext->values[ValueCorrosiveTouchStacks];
		child_ext->timers[TimerCorrosiveTouch] = parent_ext->timers[TimerCorrosiveTouch];
	}

	// Inherit Irradiate
	if (parent->irradiate_timer) {
		child->irradiate_timer = parent->irradiate_timer;
	}

	// Inherit Defensive Matrix
	if (parent->defensive_matrix_timer > 0) {
		child->defensive_matrix_hp = parent->defensive_matrix_hp;
		child->defensive_matrix_timer = parent->defensive_matrix_timer;
	}

	// Inherit Stasis Field
	if (parent->stasis_timer > 0) {
		child->stasis_timer = parent->stasis_timer;
	}

	// Inherit Deconstruction
	if (parent_ext->values[ValueDeconstruction]) {
		child_ext->values[ValueDeconstruction] = parent_ext->values[ValueDeconstruction];
		child_ext->timers[TimerDeconstruction] = parent_ext->timers[TimerDeconstruction];
		Sprite* sprite = child->sprite;
		if (child->subunit) {
			UnitExt* subunit_ext = get_unit_ext(child->subunit);
			subunit_ext->values[ValueDeconstruction] = parent_ext->values[ValueDeconstruction];
			subunit_ext->timers[TimerDeconstruction] = parent_ext->timers[TimerDeconstruction];
			sprite = child->subunit->sprite;
		}
		if (!sprite->get_overlay(ImageId::DeconstructionSmall + overlay_offset)) {
			sprite->create_overlay(ImageId::DeconstructionSmall + overlay_offset);
		}
	}

	// Inherit Terminal Surge
	if (parent_ext->values[ValueTerminalSurge]) {
		child_ext->values[ValueTerminalSurge] = parent_ext->values[ValueTerminalSurge];
	}

	// Inherit Stellar Enforcement
	if (parent_ext->timers[TimerStellarEnforcement]) {
		child_ext->timers[TimerStellarEnforcement] = parent_ext->timers[TimerStellarEnforcement];
	}

	// Inherit Swarming Omen debuff
	if (parent_ext->timers[TimerSwarmingOmenDebuff]) {
		child_ext->timers[TimerSwarmingOmenDebuff] = parent_ext->timers[TimerSwarmingOmenDebuff];
	}

	// Inherit Kaiser Rampage
	if (parent_ext->timers[TimerKaiserRampage]) {
		child_ext->timers[TimerKaiserRampage] = parent_ext->timers[TimerKaiserRampage];
	}

	// Inherit Kelvin Munitions
	if (parent_ext->timers[TimerKelvinMunitions]) {
		child_ext->timers[TimerKelvinMunitions] = parent_ext->timers[TimerKelvinMunitions];
	}

	// Inherit Empress Passive Stacks
	if (parent_ext->values[ValueEmpressAnodyneAnnexStacks]) {
		child_ext->values[ValueEmpressAnodyneAnnexStacks] = parent_ext->values[ValueEmpressAnodyneAnnexStacks];
		child_ext->values[ValueEmpressAnodyneAnnexSourcePlayer] = parent_ext->values[ValueEmpressAnodyneAnnexSourcePlayer];
	}

	// Inherit Hierophant Signify
	child_ext->timers[TimerSignify] = parent_ext->timers[TimerSignify];
	child_ext->values[ValueSignifyStacks] = parent_ext->values[ValueSignifyStacks];
	child_ext->values[ValueSignifyPlayer] = parent_ext->values[ValueSignifyPlayer];
	bw::update_signify_status(child, child_ext);

	// Inherit Worship
	child_ext->timers[TimerDemiurgeWorship] = parent_ext->timers[TimerDemiurgeWorship];
	child_ext->values[ValueDemiurgePlayer] = parent_ext->values[ValueDemiurgePlayer];

	// Inherit "No corpse" status
	child_ext->values[ValueNoCorpse] = parent_ext->values[ValueNoCorpse];

	bw::refresh_console();
}

// TODO: update
bool is_debuffed(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	bool has_parasite = false;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		if (!bw::is_allied_to(i, unit->player)) {
			has_parasite = true;
		}
	}

	return unit_ext->timers[TimerPetrification]
		|| unit_ext->timers[TimerObservancePeriod]
		|| unit_ext->timers[TimerCorrosiveTouch]
		|| unit_ext->values[ValueDeconstruction]
		|| unit_ext->values[ValueEntropyGauntlets]
		|| unit_ext->timers[TimerKelvinMunitions] >= 1
		|| unit_ext->values[ValueSignifyStacks]
		|| captivating_claws_get_timer(unit) > 0
		|| unit->irradiate_timer != 0
		|| unit->ensnare_timer != 0
		|| unit->lockdown_timer != 0
		|| unit->maelstrom_timer != 0
		|| unit->plague_timer != 0
		|| unit->is_blind
		|| has_parasite;
}

void restore_func(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);

	Unit* overlayTargetUnit = unit;
	if (unit->subunit) {
		overlayTargetUnit = unit->subunit;
	}
	u32 overlayImageId = ImageId::RestorationHit_Small + bw::get_overlay_offset(unit->id);;
	(overlayTargetUnit->sprite)->create_top_overlay(overlayImageId, 0, 0, 0);

	// remove status effects and corresponding overlays

	// Remove Incubation
	if (!bw::is_allied_to(unit_ext->values[ValueIncubationPlayer], unit->player) && unit_ext->timers[TimerIncubationArm]) {
		unit_ext->timers[TimerIncubationArm] = 0;
		unit->remove_overlay(ImageId::ParasiteOverlay);
	}

	// Remove default flags
	unit->parasite_flags = 0;
	unit->is_blind = 0;

	// Remove Blind
	if (unit_ext->timers[TimerAuralAssaultBlind]) {
		unit_ext->timers[TimerAuralAssaultBlind] = 0;

		u32 overlay = ImageId::OpticalFlareHit_Small + bw::get_overlay_offset(unit->id);
		Unit* overlay_unit = unit->subunit ? unit->subunit : unit;
		overlay_unit->remove_overlay(overlay);
		unit->status &= ~UnitStatus::CanNotReceiveOrders;
	}

	// Remove Ensnare
	if (unit->ensnare_timer != 0) {
		unit->remove_overlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
		unit->ensnare_timer = 0;
		//specific update following ensnare removal
		unit->update_speed();
	}

	// Remove Plague
	if (unit->plague_timer != 0) {
		unit->remove_overlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
		unit->plague_timer = 0;
	}

	// Remove Irradiate
	if (unit->irradiate_timer != 0) {
		unit->remove_overlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
		unit->irradiate_timer = 0;

		// specific updates following irradiate removal
		unit->irradiate_caster = 0;
		unit->irradiate_player_id = 8;
	}

	// Remove Observance
	if (unit_ext->timers[TimerObservanceDebuff]) {
		unit_ext->timers[TimerObservanceDebuff] = 1;
	}

	// Remove Knockout Drivers
	if (unit_ext->timers[TimerPetrification]) {
		unit_ext->timers[TimerPetrification] = 1;
	}

	// Remove Hallucination
	if (unit_ext->timers[TimerHallucinating]) {
		unit_ext->timers[TimerHallucinating] = 1;
	}

	// Remove Captivating Claws
	if (unit_ext->timers[TimerCaptivatingClaws]) {
		unit_ext->timers[TimerCaptivatingClaws] = 1;
		unit_ext->unit_vectors[UnitVectorCaptivatingClaws] = {};
	}

	// Remove Controlled Opposition
	unit_ext->values[ValueAutocratControlledOppositionStacks] = 0;

	// Remove Empress stacks
	unit_ext->values[ValueEmpressAnodyneAnnexStacks] = 0;

	// Remove Lockdown
	unit->remove_lockdown();

	// Remove Maelstrom
	unit->remove_maelstrom();

	// Remove Devourer Corrosive Spores
	unit->remove_acid_spores();

	// Remove Deconstruction
	unit_ext->values[ValueDeconstruction] = 0;
	unit_ext->timers[TimerDeconstruction] = 0;
	unit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
	if (unit->subunit) {
		UnitExt* subunit_ext = get_unit_ext(unit->subunit);
		subunit_ext->values[ValueDeconstruction] = 0;
		subunit_ext->timers[TimerDeconstruction] = 0;
		unit->subunit->remove_overlay(ImageId::DeconstructionSmall, ImageId::DeconstructionLarge);
	}

	// Remove Kelvin Munitions
	if (unit_ext->timers[TimerKelvinMunitions]) {
		unit_ext->timers[TimerKelvinMunitions] = 0;
	}

	// Remove Hierophant Signify
	unit_ext->timers[TimerSignify] = 0;
	unit_ext->values[ValueSignifyStacks] = 0;
	unit_ext->values[ValueSignifyPlayer] = 0;
	bw::update_signify_status(unit, unit_ext);

	// Remove Stellar Enforcement
	unit_ext->timers[TimerStellarEnforcement] = 0;

	bw::refresh_console();
}

bool is_slowed(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	if (unit->ensnare_timer) {
		return true;
	}
	if (unit_ext->timers[TimerSignify]) {
		return true;
	}
	if (unit_ext->timers[TimerPetrification]) {
		return true;
	}
	if (unit_ext->timers[TimerCaptivatingClaws]) {
		return true;
	}
	if (unit_ext->timers[TimerKelvinMunitions] >= 1) {
		return true;
	}
	return false;
}

void do_deconstruction_damage(Unit* unit, u16 weapon_id, bool play_sound) {
	// Deconstruction - Pazuzu passive
	UnitExt* unit_ext = get_unit_ext(unit);
	f64 self_damage = (unit_ext->values[ValueDeconstruction] * 0.05 * (f64) (weapons_dat::damage_amount[weapon_id] * 256.0));
	Unit* deconstruction_target = unit;
	if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
		deconstruction_target = unit_ext->units[UnitTurretParentLink]; // TODO: port as unit
		if (!deconstruction_target) {
			return;
		}
	}
	unit_ext->values[ValueTakingDeconstructionDamage] = true;
	hooks::weapon_damage(self_damage, deconstruction_target, (WeaponId::Enum)weapon_id, deconstruction_target, deconstruction_target->player, 0, 1);
	if (play_sound) {
		helper::play_sound_from_unit(SoundId::PazuzuDeconstructionDamage, unit, 1, 0);
	}
}

void clear_clarion_links(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);

	auto& links = unit_ext->unit_vectors[UnitVectorPhaseLink];
	for (Unit* link_unit : links) {
		UnitExt* link_unit_ext = get_unit_ext(link_unit);
		auto& clarions = link_unit_ext->unit_vectors[UnitVectorPhaseLinkClarions];
		std::erase_if(clarions, [&](Unit* target) { return target == unit; });
	}
	links.clear();
	unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].clear();
}

void place_clarion_links(Unit* unit) {
	clear_clarion_links(unit);

	UnitExt* unit_ext = get_unit_ext(unit);
	auto& links = unit_ext->unit_vectors[UnitVectorPhaseLink];

	auto targets = g_unit_search
		.radial_ex(unit, 5 * 32)
		.filter_by([unit](Unit* target) {
            if (!bw::is_allied_to(unit->player, target->player)) {
                return false;
            }
            if (units_dat::base_property[target->id] & UnitProperty::Building) {
                return false;
            }
            if (!target->shields_enabled()) {
                return false;
            }
            if (target->shields < 256) {
                return false;
            }
            return true;
        });

	for (Unit* target : targets) {
		if (!std::ranges::contains(links, target)) {
			links.push_back(target);
		}

		UnitExt* target_ext = get_unit_ext(target);
		auto& target_links = target_ext->unit_vectors[UnitVectorPhaseLinkClarions];
		if (!std::ranges::contains(target_links, unit)) {
			target_links.push_back(unit);
		}
	}
}

bool has_clarion_links(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	return !unit_ext->unit_vectors[UnitVectorPhaseLinkClarions].empty();
}

static void link_damage(Unit* unit, s32 damage, s8 direction, const hooks::WeaponDamageData& data) {
	hooks::WeaponDamageData new_data = data;
	new_data.damage = damage;
	new_data.direction = direction;
	new_data.shield_reduce_amount = 0;
	new_data.target = unit;
	new_data.is_phase_link_damage_instance = true;
	hooks::weapon_damage_root(new_data);
}

s32 spread_clarion_damage(const hooks::WeaponDamageData& data, f32 factor) {
	Unit* target = data.target;
	s32 damage = data.damage;
	s32 damage_to_spread = std::min(s32(damage * factor), target->shields);

	// Min damage to main target
	s32 estimated_damage = damage - damage_to_spread;
	if (estimated_damage < 256) {
		s32 missing_damage = 256 - estimated_damage;
		damage += missing_damage;
		damage_to_spread -= missing_damage;
	}

	if (damage_to_spread <= 0) {
		return damage;
	}

    auto links = get_clarion_links(target); // Has to be copy
	if (links.empty()) {
		return damage;
	}

    s32 damage_per_unit = damage_to_spread / links.size();
    Unit* damage_origin = data.attacker ? data.attacker : target;
    for (Unit* unit : links) {
        s32 unit_damage = std::min(damage_per_unit, unit->shields);
        damage -= unit_damage;
        link_damage(unit, unit_damage, bw::get_angle(damage_origin->position, unit->position), data);
    }
	return damage;
}

std::vector<Unit*> get_clarion_links(Unit* unit) {
	std::unordered_set<Unit*> links = {};
	UnitExt* unit_ext = get_unit_ext(unit);
	for (Unit* clarion : unit_ext->unit_vectors[UnitVectorPhaseLinkClarions]) {
		if (!clarion) {
			continue;
		}

		UnitExt* clarion_ext = get_unit_ext(clarion);
		links.insert_range(clarion_ext->unit_vectors[UnitVectorPhaseLink]);
	}
	links.erase(unit);
	return std::vector<Unit*>{links.begin(), links.end()};
}
}