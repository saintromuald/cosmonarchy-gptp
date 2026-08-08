#include "weapon_damage.h"
#include "../../BW/api.h"
#include <algorithm>
#include <cmath>
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
#include "GameModes/IGameMode.h"

#include "BW\UnitFinder.h"

Bullet* weapon_damage_current_bullet;

void apply_ensnare_internal(Unit* unit, u32 ensnare_factor, u32 ensnare_timer) {
    // Overlay
    if (!(unit->ensnare_timer) && !(unit->status & UnitStatus::Burrowed)) {
        bw::add_unit_overlay(unit, ImageId::EnsnareOverlay_Small + bw::get_overlay_offset(unit->id));
    }

    // Effect
    UnitExt* unit_ext = get_unit_ext(unit);
    unit_ext->values[ValueEnsnareFactor] = ensnare_factor;
    unit->ensnare_timer = ensnare_timer;
    unit->update_speed();
}

void apply_corrosive_spores_internal(Unit* unit, u32 timer) {
    auto& unit_ext = unit->ext();
    bool has_spores = unit_ext.timers[TimerCorrosiveTouch];
    if (!has_spores) {
        ImageId::Enum overlay_id = ImageId::AcidSpores_4_5_Overlay_Small;
        switch (bw::get_overlay_offset(unit->id, true)) {
        case 1: overlay_id = ImageId::AcidSpores_4_5_Overlay_Small;  break;
        case 2: overlay_id = ImageId::AcidSpores_4_5_Overlay_Medium; break;
        case 3: overlay_id = ImageId::AcidSpores_4_5_Overlay_Large;  break;
        case 4: overlay_id = ImageId::AcidSpores_6_9_Overlay_Large;  break;
        }
        bw::add_unit_overlay(unit, overlay_id);
    }
    unit_ext.timers[TimerCorrosiveTouch] = timer;
}

bool carapace_swarm_blocks_damage(Unit* target, u32 weapon_id, u32 damage_divisor) {
    return bw::is_under_carapace_swarm(target)
        && weapons_dat::max_range[weapon_id] > 32
        && (damage_divisor == 1 || weapon_id == WeaponId::GeszithalorAcidSpore)
        && !weapon_id_is_flamethrower(weapon_id)
        && weapon_id != WeaponId::LakiziliskSubterraneanSpines
        && weapon_id != WeaponId::OlympianTitansFist
        && weapon_id != WeaponId::Plague
        && weapon_id != WeaponId::Custom;
}

bool irradiate_target(Unit* unit) {
    return true; // Originally prohibited targeting eggs
}

static void corrosive_pathogen_proc(Unit* attacker, Unit* target) {
    u32 search_margin = Zerg::CorrosivePathogenRange;
    Box32 search_area = {};
    search_area.left = target->position.x - search_margin;
    search_area.top = target->position.y - search_margin;
    search_area.right = target->position.x + search_margin;
    search_area.bottom = target->position.y + search_margin;
    for (Unit* unit : g_unit_search.rect(search_area).filter(attacker, FilterEnemy | FilterIncludingSelf)) {
        helper::apply_corrosive_spores(unit);
        Thingy* thingy = helper::create_thingy(SpriteId::Corrosive_Acid_Hit, unit->get_x(), unit->get_y(), 11);
        if (thingy) {
            thingy->sprite->elevation = unit->sprite->elevation + 1;
            helper::set_thingy_visibility_flags(thingy);
        }
    }
}

static void swarming_omen_proc(Unit* target, u32 attacking_player_id) {
    auto& target_ext = target->ext();
    if (!bw::is_allied_to(attacking_player_id, target->player)) {
        target_ext.timers[TimerSwarmingOmenDebuff] = 1secs + 1;
        target_ext.values[ValueSwarmingOmenPlayer] = attacking_player_id;
    }
}

namespace hooks {
    static void play_alert(Unit* unit, u32 sound_id) {
        u32 alert_id = SoundId::None;
        switch (g_console_index) {
            case RaceId::Terran:  alert_id = SoundId::AlertTerran;  break;
            case RaceId::Protoss: alert_id = SoundId::AlertProtoss; break;
            case RaceId::Zerg:    alert_id = SoundId::AlertZerg;    break;
            case RaceId::Askosi:  alert_id = SoundId::AlertTerran;  break;
        }
        if (g_options_list->flags & 1 && !g_use_quiet_sounds) {
            bw::play_sound(alert_id);
        }
        helper::do_unit_event_notification(unit, 16, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 1, alert_id);

        g_globals->alert.sound_id = sound_id;
        g_globals->alert.play_timer = 12;
    }

    static void reset_all_unit_attack_notify_timers_in_range(Unit* unit) {
        for (Unit* other : g_unit_search.radial(unit->position, 11tiles)) {
            other->attack_notify_timer = other->player == unit->player ? 100 : 1000;
        }
    }

    static bool g_should_play_alert;

    void play_attack_notification(Unit* unit) {
        g_should_play_alert = true;
        unit_attack_notify_hook(unit);
        g_should_play_alert = false;
    }

    // Hooked to fix one-shots not notifying owners. -Pr0nogo
    void unit_attack_notify_hook(Unit* unit) { // 0048F230
        if (!g_should_play_alert) {
            return;
        }
        u32 local_player = g_local_nation_id;
        if (unit->player == 11) {
            return;
        }
        bool witness_or_replay = g_is_in_replay || g_globals->players[local_player].witness_mode;
        if (unit->player == local_player || witness_or_replay) {
            unit->last_event_color = g_palette_index_reference[20];
            unit->last_event_timer = 20;
            if (!g_globals->alert.own_cooldown) {
                if (helper::unit_is_in_notify_range(unit) && !unit->attack_notify_timer) {
                    if (!witness_or_replay) {
                        reset_all_unit_attack_notify_timers_in_range(unit);
                        g_globals->alert.own_cooldown = 100;
                        bool is_building = units_dat::base_property[unit->id] & UnitProperty::Building;
                        u32 sound_id = SoundId::None;
                        AdvisorEvent type = is_building ? AdvisorEvent_BaseUnderAttack : AdvisorEvent_ArmyUnderAttack;
                        if (!is_building) {
                            u32 sound_first = units_dat::sound_annoyed_first[unit->id];
                            u32 sound_last = units_dat::sound_annoyed_last[unit->id];
                            if (sound_first && sound_last) {
                                sound_id = helper::get_random_unit_response(sound_first, sound_last);
                            } else {
                                sound_id = bw::get_advisor_sound_from_event_type(local_player, type);
                            }
                        } else {
                            sound_id = bw::get_advisor_sound_from_event_type(local_player, type);
                        }
                        play_alert(unit, sound_id);
                    } else {
                        g_globals->alert.own_cooldown = 100;
                        replay_witness_event(unit, SpectatorEvent_Combat);
                    }
                }
            }
        } else if (bw::are_mutual_allies(local_player, unit->player) && !g_globals->players->ally_alert_blacklist[unit->player]) {
            unit->last_event_color = g_palette_index_reference[20];
            unit->last_event_timer = 20;
            if (!g_globals->alert.own_cooldown && !g_globals->alert.ally_cooldown) {
                if (helper::unit_is_in_notify_range(unit) && !unit->attack_notify_timer) {
                    reset_all_unit_attack_notify_timers_in_range(unit);
                    g_globals->alert.ally_cooldown = 1000;

                    AdvisorEvent type;
                    switch (g_player_table[unit->player].race) {
                        case RaceId::Zerg:    type = AdvisorEvent_AllyUnderAttack_Z; break;
                        case RaceId::Protoss: type = AdvisorEvent_AllyUnderAttack_P; break;
                        case RaceId::Terran:  type = AdvisorEvent_AllyUnderAttack_T; break;
                        case RaceId::Askosi:  type = AdvisorEvent_AllyUnderAttack_A; break;
                    }
                    u32 sound_id = bw::get_advisor_sound_from_event_type(local_player, type);
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
    void weapon_damage(s32 damage, Unit* target, WeaponId::Enum weapon_id, Unit* attacker, u8 attacking_player_id, s8 direction, u8 damage_divisor) {
        WeaponDamageData data{
            .damage					= damage,
            .damage_type			= weapons_dat::damage_type[weapon_id],
            .damage_divisor			= damage_divisor,
            .weapon_id				= weapon_id,
            .direction				= direction,
            .target					= target,
            .attacker				= attacker,
            .attacking_player_id	= attacking_player_id,
        };
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
        WeaponDamageData data{
            .damage					= damage,
            .bonus_armor_pen		= armor_pen,
            .damage_type			= DamageType::Impact,
            .damage_divisor			= damage_divisor,
            .weapon_id				= WeaponId::Custom,
            .direction				= direction,
            .target					= target,
            .attacker				= attacker,
            .attacking_player_id	= attacking_player_id,
            .ignore_flat_armor		= ignore_flat_armor,
        };
        weapon_damage_root(data);
    }

    void weapon_damage_root(WeaponDamageData& data) {
        // The unit must neither be already dead nor invincible
        if (!data.target || data.target->hitpoints == 0 || data.target->status & UnitStatus::Invincible) {
            return;
        }

        auto& target_ext = data.target->ext();
        if (target_ext.values[ValueMindControllerUnitId]) {
            data.damage = static_cast<s32>(data.damage * 1.5fp);
        }

        if (data.attacker) {
            auto& attacker_ext = data.attacker->ext();
            if (attacker_ext.values[ValueTriggerDamageBaseSet] && weapon_id_can_proc_onhits(data.weapon_id)) {
                data.damage = attacker_ext.values[ValueTriggerDamageBaseAmount];
            }

            if (data.attacker->is_hallucination()) {
                data.damage /= 2;
            }
        }

        if (weapon_id_can_proc_onhits(data.weapon_id)) {
            // Ancile & Piety Ward - Providence - Hitscan handling
            auto units = g_unit_search
                .radial_ex(data.target, 2tiles, FilterIncludingSelf | FilterAlly, {UnitId::TerranAncile, UnitId::TerranPietyWard})
                .sort_by_distance(data.target->position);

            for (Unit* unit : units) {
                if (unit->is_frozen()) {
                    continue;
                }
                if (unit->energy < 1 * 256) {
                    continue;
                }
                if (unit->id == UnitId::TerranPietyWard && !unit->is_burrowed()) {
                    continue;
                }

                u32 energy_to_spend = std::min<u32>(unit->energy, data.damage);
                unit->spend_energy(energy_to_spend);
                bw::create_shield_overlay(data.target, data.direction);
                bw::play_sound({SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03}, unit);
                play_attack_notification(unit);
                bw::reveal_attacker(data.attacker, unit);

                data.damage -= energy_to_spend;
                if (data.damage == 0) {
                    return;
                }
            }
            // Object, Petract - Shouldered Burden
            if (data.target->id != UnitId::AskosiObject && data.target->id != UnitId::AskosiPetract && data.target->player != 11) {
                auto shoulders = g_unit_search
                    .radial_ex(data.target, 4tiles, FilterNotSelf | FilterAlly | FilterEnabled, {UnitId::AskosiObject, UnitId::AskosiPetract})
                    .sort_by_distance(data.target->position);

                for (Unit* shoulder : shoulders) {
                    if (shoulder == data.attacker) {
                        continue;
                    }

                    shoulder->damage_with(data.weapon_id, data.damage, data.attacker, data.attacking_player_id, data.direction, data.damage_divisor);
                    bw::create_shield_overlay(data.target, data.direction);
                    bw::play_sound({SoundId::AncilePassiveProc01, SoundId::AncilePassiveProc03}, shoulder); // TODO: Own audio
                    play_attack_notification(shoulder);
                    bw::reveal_attacker(data.attacker, shoulder);
                    return;
                }
            }
        }

        // Ai_ReactToHit crash - null town
        if (data.target->unit_ai) {
            // Ai_ReactToHit crash - null town
            WorkerAi* ai = (WorkerAi*)data.target->unit_ai;
            if (ai->type == AiType::Worker || ai->type == AiType::Building) {
                if (!ai->town) {
                    u32 attacker_id = data.attacker ? data.attacker->id : UnitId::None;
                    bw::printf("Ai_ReactToHit crash prevention - null town! Attacker id %d, Attacker player %d, Target id %d, Target player %d",
                                                                              attacker_id, data.attacker->player, data.target->id, data.target->player);
                    return; // Don't think this ever happens anymore, removed can_damage bool 11/27/2023 -Pr0nogo
                }
            }
        }

        weapon_damage_damage_modifiers(data);
        // Minimum amount of damage that can be dealt.
        // Phase Link: only apply min damage on main target
        // @Veeq7 - should this be above the Ancile code, to prevent dealing less than 1 damage to Ancile energy? -Pr0nogo
        if (!data.is_phase_link_damage_instance && data.damage < 256) {
            data.damage = data.damage == 0 ? 0 : 256;
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
        // Phase Link: Only count armor of target unit
        bool shield_overlay = false;
        if (!data.is_phase_link_damage_instance && data.damage_type != DamageType::IgnoreArmor && !data.ignore_flat_armor) {
            s32 armor_pen = hooks::get_modified_armor_pen(data.attacker, data.target, data.weapon_id) + data.bonus_armor_pen;
            if (weapon_damage_current_bullet && weapon_damage_current_bullet->weapon_id == WeaponId::AlgoztaliskBottomlessSpinestacks) {
                BulletExt* bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
                armor_pen += bullet_ext->distance_traveled / 32;
            }
            s32 armor_total = (hooks::get_armor_internal(data.target) - armor_pen);

            u32 fore_castle_factor = bw::get_fore_castle_armor_bonus(data.target);
            if (fore_castle_factor) {
                bool apply_fore_castle = !data.attacker || data.target->is_building();
                if (!apply_fore_castle) {
                    v2 dir = data.target->position.dir_to(data.attacker->position);
                    v2 facing_dir = bw::facing_dir(data.target);
                    apply_fore_castle = dir.dot(facing_dir) >= bw::deg_to_dot(60.0fp);
                }
                if (apply_fore_castle) {
                    armor_total += fore_castle_factor;
                    if (data.weapon_id != WeaponId::PsiStorm) {
                        shield_overlay = true;
                    }
                }
            }
            armor_total = std::max(armor_total, 0);
            if (data.attacker && data.attacker->ext().timers[TimerRejoinderOfService]) {
                armor_total = 0;
            }
            if (data.weapon_id != WeaponId::PsiStorm) {
                data.damage -= std::min(data.damage, armor_total * 256);
            }
        }

        // Phase Link: spread the damage before dealing it to shields
        if (!data.is_phase_link_damage_instance && bw::has_clarion_links(data.target)) {
            data.damage = bw::spread_clarion_damage(data, 0.75fp);
        }

        // Mind Tyrant - Villanous Fervor - Reduce Mind Tyrant energy first
        if (target_ext.values[ValueIsMalefactor] == 2) {
            Unit* mind_tyrant = target_ext.units[UnitMindTyrantLink];
            if (mind_tyrant) {
                s32 energy_reduce_amount = std::min(data.damage, (s32)mind_tyrant->energy);
                mind_tyrant->spend_energy(energy_reduce_amount);
                data.damage -= energy_reduce_amount;
                if (data.damage < 0) {
                    data.damage = 0;
                    data.damage_type = DamageType::Independent;
                }
            }
        }

        // Reduce shields
        if (units_dat::shields_enabled[data.target->id] && data.target->shields >= 256) {
            if (!(helper::is_under_carapace_swarm(data.target) && carapace_swarm_blocks_damage(data.target, data.weapon_id, data.damage_divisor))) {
                data.shield_reduce_amount = std::min(data.damage, data.target->shields);
                // Phase Link: only apply min damage on main target
                if (!data.is_phase_link_damage_instance && data.shield_reduce_amount < 256) {
                    data.shield_reduce_amount = 256;
                }
                data.damage -= data.shield_reduce_amount;
                if (data.damage < 0) {
                    data.damage = 0;
                }
            }
        }

        // Set min damage
        // Phase Link: only apply min damage on main target
        if (!data.is_phase_link_damage_instance && data.shield_reduce_amount == 0 && data.damage < 256) {
            data.damage = 256;
        }

        // Phase Link: No On-hits
        if (!data.is_phase_link_damage_instance) {
            weapon_damage_behaviors(data);
        }
        if (data.attacker) {
            weapon_damage_lifesteal(data);
        }

        if (weapon_damage_is_lethal(data)) {
            if (!bw::is_allied_to(data.target->player, data.attacking_player_id) && game_kind_doubles_units()) {
                bw::add_massacre_resource(data.attacking_player_id);
            }
            if (!weapon_damage_fatal(data)) { // Prevent attack if it wasn't fatal
                data.damage = 0;
                data.damage_type = DamageType::Independent;
            } else if (data.attacker) {
                switch (data.attacker->id) {
                    case UnitId::AskosiZonguard: {
                        data.attacker->ext().timers[TimerMechanicalRage] = AskosiZonguard::MechanicalRageDuration;
                    } break;
                }
            }
        }

        // Deal damage, except for Independent damage, which always deals 0
        if (data.damage_type != DamageType::Independent && data.target->hitpoints != 0 && data.target->defensive_matrix_hp == 0) {
            if (data.target->is_hallucination()) {
                data.damage = data.target->hitpoints; // Increasing the damage, so we insta kill, but don't inflate other damage calcs
            }
            data.target->damage_hp(data.damage, data.attacker, data.attacking_player_id, data.weapon_id != WeaponId::Irradiate); // Prevent Seraphs from being continuously revealed to the irradiated target
            g_should_play_alert = true;
            play_attack_notification(data.target); // Previously excluded WeaponId::Irradiate
            g_should_play_alert = false;
        }

        // New AI defense logic -Veeq7
        bw::ask_for_ai_help(data.target, data.attacker);

        // Reduce shields
        if (data.shield_reduce_amount) {
            data.target->shields -= data.shield_reduce_amount;
            if (data.attacker) {
                bw::update_on_damage_dealt(data.attacker, data.shield_reduce_amount);
            }
            bw::update_on_damage_taken(data.target, data.shield_reduce_amount);
            if (data.damage_type != DamageType::Independent && data.target->shields != 0) {
                shield_overlay = true;
            }
        }

        if (shield_overlay) {
            bw::create_shield_overlay(data.target, data.direction);
        }

        // Update unit strength data
        data.target->air_strength = helper::get_unit_strength(data.target, false);
        data.target->ground_strength = helper::get_unit_strength(data.target, true);

        if (data.weapon_id == WeaponId::Lockdown) {
            bw::play_sound(SoundId::GoskaGrenadeImpact, weapon_damage_current_bullet->position);
        }

        if (game_kind_has_neutral_hostile() && data.target->player == NeutralHostilePlayer && unit_id_is_burrowed_attacker(data.target->id)) {
            helper::ai_order_lurker_burrow(data.target);
        }
        bw::play_manual_weaponhit_sound(data.target, data.weapon_id);
    }

    static void weapon_damage_damage_modifiers(WeaponDamageData& data) {
        // Target based calcs
        auto& target_ext = data.target->ext();

        if (g_active_game_mode) {
            g_active_game_mode->on_unit_damage_modifiers(data);
        }

        if (weapon_id_deals_no_damage(data.weapon_id)) {
            data.damage = 0;
        }

        // Allied invincibility frames (for e.g. Blissful Embrace)
        if (target_ext.timers[TimerAlliedInvFrames]) {
            if (bw::is_allied_to(data.attacking_player_id, target_ext.values[ValueAlliedInvPlayer])) {
                data.damage = 0;
            }
        }

        // Isthrathaleth Carapace Swarm
        if (carapace_swarm_blocks_damage(data.target, data.weapon_id, data.damage_divisor)) {
            data.damage_type = DamageType::Independent;
        }

        // Phase Link: no on-hits
        if (data.is_phase_link_damage_instance) {
            return; // Skip weapon & attacker based calcs, since we already did those for Phase Link
        }

        // Weapon & Attacker based calcs
        
        if (data.attacker) {
            switch (data.weapon_id) {
                // @TODO: Extend weapons and remove these
                case WeaponId::SentinelCerberusRailgun: {
                    if (data.attacker->id == UnitId::TerranCutlass|| data.attacker->id == UnitId::TerranCutlassTurret) {
                        data.damage -= 9 * 256;
                    }
                } break;
                case WeaponId::PhalanxArcliteCannon: {
                    if (data.attacker->id == UnitId::TerranBroadside || data.attacker->id == UnitId::TerranBroadsideTurret) {
                        data.damage -= 5 * 256;
                    }
                } break;
                case WeaponId::ZealotPhaseBlades: {
                    if (data.attacker->id == UnitId::ProtossCicatus) {
                        data.damage -= 3 * 256;
                    }
                } break;
                case WeaponId::VassalPulseBlaster: {
                    if (data.attacker->id == UnitId::ProtossRenesair) {
                        data.damage -= 2 * 256;
                    }
                } break;
                case WeaponId::SimulacrumPhaseCannon: {
                    if (data.attacker->id == UnitId::ProtossSimulacrumAstral) {
                        data.damage -= 2 * 256;
                    }
                } break;
                case WeaponId::VorvrokorVorvrokClaws: {
                    if (data.attacker->id == UnitId::ZergGarvrokor) {
                        data.damage += 1 * 256;
                    }
                } break;
                case WeaponId::NathrokorNathrokClaws: {
                    if (data.attacker->id == UnitId::AskosiAkreno) {
                        data.damage += 6 * 256;
                    }
                } break;
                case WeaponId::KethrostalorStaggeredSpores: {
                    if (data.attacker->id == UnitId::ZergZethalor) {
                        data.damage -= 4 * 256;
                    }
                } break;
                case WeaponId::UltrokorKaiserBlades: {
                    if (data.attacker->id == UnitId::AskosiZonguard) {
                        data.damage -= 5 * 256;
                    }
                } break;
                case WeaponId::GrandstandArteDriver: {
                    if (data.attacker->id == UnitId::AskosiZonguard) {
                        data.damage -= 3 * 256;
                    }
                } break;
                case WeaponId::RoveElreScaleshot: {
                    if (data.attacker->id == UnitId::AskosiThaumalogue) {
                        data.damage += 2 * 256;
                    }
                } break;
                case WeaponId::IntendentErsaDriver: {
                    if (data.attacker->id == UnitId::AskosiPhontis) {
                        data.damage -= 2 * 256;
                    }
                } break;
                case WeaponId::VicarOspoTwinshard: {
                    if (data.attacker->id == UnitId::AskosiIconoclast) {
                        data.damage += 1 * 256;
                    }
                    if (data.attacker->id == UnitId::AskosiParaclete) {
                        data.damage += 3 * 256;
                    }
                } break;
                case WeaponId::CrematoriumOkliQuadshard: {
                    if (data.attacker->id == UnitId::AskosiAvatar) {
                        data.damage += 10 * 256;
                    }
                } break;

                // Behaviors
                case WeaponId::CabalistWarpBlades: {
                    auto& attacker_ext = data.attacker->ext();
                    if (attacker_ext.values[ValueCabalistSilentStridesDamage] || data.attacker->status & UnitStatus::Cloaked) {
                        data.damage = static_cast<s32>(data.damage * 1.5fp);
                        attacker_ext.values[ValueCabalistSilentStridesDamage] = 0;
                    }
                } break;
                case WeaponId::UnarAutonomousRay: {
                    auto& attacker_ext = data.attacker->ext();
                    if (attacker_ext.unar_solitary_focus_stacks) {
                        data.damage += attacker_ext.unar_solitary_focus_stacks * 256;
                    }
                } break;
            }
        }

        // Divisor calcs and other modifiers
        switch (data.weapon_id) {
            case WeaponId::AshguardPhaseCremator: {
                data.damage += KhalendricAshguard::IndomitableAnnexDamageMultiplier * target_ext.indomitable_annex_stacks * 256;
            } break;
            case WeaponId::ArchitectDispersiveSalvo: {
                u32 divisor = data.damage_divisor ? data.damage_divisor : 1;
                data.damage /= divisor;
            } break;
            case WeaponId::GeszithalorAcidSpore: {
                // Searing Acid - Geszithalor behavior
                data.damage += data.damage_divisor * 10 * 256;
            } break;
            default: {
                if (weapon_id_bounces_lithe_organelle(data.weapon_id)) {
                    data.damage = static_cast<s32>(data.damage * (1 - data.damage_divisor * 0.33fp));
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

        // Hallucinating targets - fire a phantom strike
        if (data.attacker && weapon_id_can_proc_onhits(data.weapon_id) && target_ext.timers[TimerHallucinating] >= 1 && !bw::is_allied_to(data.attacker->player, data.target->player)) {
            if (weapon_damage_current_bullet && data.weapon_id != WeaponId::GladiusPlasmaFlurry && data.weapon_id != WeaponId::PaladinCrusaderMissiles) {
                BulletExt* current_bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
                if (!current_bullet_ext->is_phantom) {
                    data.attacker->sprite->create_overlay(ImageId::PhantomStrikeProc);
                //	WeaponOffsets offsets = get_weapon_offsets(data.weapon_id);
                    Bullet* phantom_strike = fire_bullet(data.attacker, data.weapon_id, { data.attacker->order_target.pt, data.target }, {.apply_cooldown = false});
                    if (phantom_strike) {
                        BulletExt* phantom_ext = get_bullet_ext(phantom_strike);
                        phantom_ext->is_phantom = true;
                    }
                }
            } else {
                data.damage = static_cast<s32>(data.damage * 1.5fp); // Currently doesn't proc onhits with this solution! -Pr0nogo
                data.attacker->sprite->create_overlay(ImageId::PhantomStrikeProc);
            }
        }
    }

    static void weapon_damage_behaviors(const WeaponDamageData& data) {
        auto& target_ext = data.target->ext();
        bool should_update_speed = false;

        if (g_active_game_mode) {
            g_active_game_mode->on_unit_damage(data);
        }

        // No attacker here
        switch (data.weapon_id) {
            case WeaponId::AurielQuarternaryRifle:
            case WeaponId::ArhaunPhlegethonCannon:
            case WeaponId::ValkyrieHecatonLogiRockets: {
                target_ext.values[ValueIncineratePlayer] = data.attacking_player_id;
                target_ext.timers[TimerIncinerateDamage] = HecatonAuriel::IncinerateTimer;
            } break;
            case WeaponId::CyclopsBrontesPayload: {
                data.target->sprite->create_top_overlay(ImageId::CyclopsFragmentaryExplosion);
            } break;
            case WeaponId::WendigoFusionSaw: {
                target_ext.values[ValueWendigoShatteredWindsStacks]++;
                target_ext.timers[TimerWendigoShatteredWinds] = UpstartWendigo::ShatteredWindsDuration;
            } break;
            case WeaponId::LegionnairePhaseSpear: {
                target_ext.timers[TimerBladeVortex] = 2secs + 1;
            } break;
            case WeaponId::OscilliantRotatingRay: {
                if (target_ext.timers[TimerOscilliantBindingPlasma]) {
                    u32 source_player = target_ext.values[ValueOscilliantBindingPlasmaPlayer];
                    if (bw::is_allied_to(data.attacking_player_id, source_player)) {
                        target_ext.values[ValueOscilliantBindingPlasmaStacks]++;
                    }
                } else {
                    target_ext.timers[TimerOscilliantBindingPlasma] = Protoss::BindingPlasmaTimer;
                    target_ext.values[ValueOscilliantBindingPlasmaPlayer] = data.attacking_player_id;
                    target_ext.values[ValueOscilliantBindingPlasmaStacks] = 1;
                }
            } break;
            case WeaponId::GladiusPlasmaFlurry: {
                if (target_ext.timers[TimerGladiusBindingPlasma]) {
                    u32 source_player = target_ext.values[ValueGladiusBindingPlasmaPlayer];
                    if (bw::is_allied_to(data.attacking_player_id, source_player)) {
                        target_ext.values[ValueGladiusBindingPlasmaStacks]++;
                    }
                } else {
                    target_ext.timers[TimerGladiusBindingPlasma] = Protoss::BindingPlasmaTimer;
                    target_ext.values[ValueGladiusBindingPlasmaPlayer] = data.attacking_player_id;
                    target_ext.values[ValueGladiusBindingPlasmaStacks] = 1;
                }
            } break;
            case WeaponId::PariahIconoclastBlades: {
                target_ext.timers[TimerPariahHit] = 2secs;
                target_ext.values[ValuePariahHitPlayer] = data.attacking_player_id;
            } break;
            case WeaponId::KagraliskRepeatingVenom:
            case WeaponId::CherbrathalorVenemousBursts: {
                if (!data.target->plague_timer) {
                    u32 overlay_offset = bw::get_overlay_offset(data.target->id);
                    if (!(data.target->sprite->get_overlay(ImageId::PlagueOverlay_Small + overlay_offset))) {
                        data.target->sprite->create_overlay(ImageId::PlagueOverlay_Small + overlay_offset);
                    }
                }
                data.target->plague_timer = Plague::MiasmaTimer;
                target_ext.values[ValueMiasma] = 1;
            } break;
            case WeaponId::PilgrimMiningSaber:
            case WeaponId::ScepterOrmaScrapray:
            case WeaponId::HelionUskoScrapray: {
                target_ext.timers[TimerGoldRush] = Askosi::ScrapworldGoldRushTimer;
                target_ext.values[ValueGoldRushPlayer] = data.attacking_player_id;
            } break;
            case WeaponId::RemoteHijack: {
                Unit* auxine = bw::create_unit_from_bullet(UnitId::AskosiAuxine, data.attacking_player_id, data.target->position, data.attacker);
                target_ext.timers[TimerIsBeingHijacked] = 5secs + 1;
                target_ext.units[UnitHijacker] = auxine;
                auxine->ext().units[UnitHijackTarget] = data.target;
            } break;
        }

        switch (data.target->id) {
            case UnitId::ProtossGolem: {
                // Golem - Tranquility Array
                if (!target_ext.timers[TimerTranquilityArray] && data.target->shields >= 128) {
                    bw::shield_heal_proc(data.target, data.target->player, 64, 2);
                    target_ext.timers[TimerTranquilityArray] = 4;
                }
            } break;
            case UnitId::ZergZoryusthaleth: {
                if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                    incubate(data.target, 1, UnitId::ZergRilirokor, data.target->player);
                }
            } break;
            case UnitId::ZergVithrilisk: {
                if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                    incubate(data.target, 1, UnitId::ZergSkryrokor, data.target->player);
                }
            } break;
            case UnitId::ZergVilgorokor:
            case UnitId::ZergAzanthrokor: {
                if (data.damage > 0 && !bw::is_under_carapace_swarm(data.target) && !target_ext.timers[TimerReconstitutionDelay]) {
                    target_ext.timers[TimerReconstitutionDelay] = 2secs + 1;
                    helper::play_sound_from_unit(SoundId::Zerg_Ultra_ZUlRor00_WAV, data.target, 1, 0);
                }
            } break;
            case UnitId::AskosiMissionary:
            case UnitId::AskosiDoronin:
            case UnitId::AskosiRove:
            case UnitId::AskosiAgonist:
            case UnitId::AskosiEmissary:
            case UnitId::AskosiAkreno: {
                if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                    target_ext.timers[TimerImpactDissipation] = Askosi::ImpactDissipationDuration;
                    data.target->update_speed();
                }
            } break;
            case UnitId::AskosiThaumalogue: {
                if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                    if (target_ext.values[ValueReassemblyActive]) {
                        target_ext.timers[TimerRetaliationAssembly] = AskosiThaumalogue::RetaliationAssemblyDuration;
                    } else {
                        target_ext.timers[TimerImpactDissipation] = Askosi::ImpactDissipationDuration;
                        data.target->update_speed();
                    }
                }
            } break;
        }

        if (target_ext.values[ValueReclaimed]) {
            target_ext.values[ValueReclamationRegen] += data.damage / 2;
        }

        // Refresh Apollyoid Adhesive on damage taken
        if (weapon_id_can_proc_onhits(data.weapon_id) && target_ext.timers[TimerApollyoidAdhesive]) {
            target_ext.timers[TimerApollyoidAdhesive] = ApollyoidAdhesive::Timer;
        }

        if (data.attacker) {
            auto& attacker_ext = data.attacker->ext();

            // Ignore Carapace Swarm!
            // START
            Unit* malefactor = data.attacker;
            auto& malefactor_ext = attacker_ext;
            if (malefactor->id == UnitId::ProtossSteward) {
                if (malefactor->hangar_unit.parent) {
                    malefactor = malefactor->hangar_unit.parent;
                    malefactor_ext = malefactor->ext();
                }
            }
            Unit* tyrant = malefactor_ext.units[UnitMindTyrantLink];
            if (tyrant) {
                if (tyrant->energy >= Protoss::TyrannyEnergyCost && !bw::are_mutual_allies(data.target->player, data.attacking_player_id) && !data.target->is_building() && malefactor_ext.values[ValueIsMalefactor] == 2) {
                    target_ext.timers[TimerMalefactorTyrannize] = ProtossMindTyrant::MalefactorTyrannizeDuration;
                    target_ext.last_malefactor_player = data.attacking_player_id;
                    // Drain energy for every tyrannized unit
                    tyrant->spend_energy(Protoss::TyrannyEnergyCost);
                }
            }

            switch (data.attacker->id) {
                case UnitId::ProtossEcclesiastKhalendric: {
                    if (!bw::is_allied_to(data.target->player, data.attacker->player)) {
                        ascend_in_area(data.attacker->player, data.attacker->position, Protoss::AstralChargeRadius);
                    }
                } break;
                case UnitId::ProtossAugur: {
                    // Augur spend shields on attack
                    data.attacker->shields = std::max(data.attacker->shields - 10 * 256, 0);
                    attacker_ext.shield_regen_timer = 0;
                } break;
                case UnitId::ProtossManifoldStarbound: {
                    target_ext.timers[TimerDiscordantChorus] = 2secs + 1;
                } break;
                case UnitId::ZergQuazrokor:
                case UnitId::ZergNathrokor: {
                    attacker_ext.timers[TimerZergAdrenalFrenzy] = 2secs + 1;
                    data.attacker->update_speed();
                } break;
                case UnitId::ZergRilirokor:
                case UnitId::ZergRililisk: {
                    data.attacker->remove_timer += 1secs;
                    if (data.attacker->remove_timer > Broodspawn::LifeTimer) {
                        data.attacker->remove_timer = Broodspawn::LifeTimer;
                    }
                } break;
            }

            switch (data.weapon_id) {
                case WeaponId::OlympianMountainGun:
                case WeaponId::OlympianTitansFist: {
                    if (!data.target->is_building() && bw::unit_is_exposed_to(data.target, data.attacker->position)) {
                        s32 bonus_damage = hooks::get_modified_armor_pen(data.attacker, data.target, data.weapon_id) + data.bonus_armor_pen - hooks::get_armor_internal(data.target);
                        if (bonus_damage > 0) {
                            auto& target_ext = data.target->ext();
                            if (!target_ext.timers[TimerOlympianMythicalMightSFX]) {
                                helper::play_sound_from_unit(SoundId::OlympianMythicalMight01, data.target, 1, 0);
                                target_ext.timers[TimerOlympianMythicalMightSFX] = 3;
                            }
                            u32 overlay_id = bw::rand_between(SpriteId::OlympianDishonorableDischarge1, SpriteId::OlympianDishonorableDischarge3);
                            bw::create_and_init_thingy(overlay_id, data.target->position.x, data.target->position.y, data.attacking_player_id, data.target->sprite->elevation + 1);
                            data.target->inflict_damage(bonus_damage * 256, data.attacker, data.attacking_player_id, false);
                        }
                    }
                } break;
                case WeaponId::AutocratEminenceRifle: {
                    if (data.attacker->energy < Reclamation::FieldEnergyCost) {
                        break;
                    }
                    data.attacker->spend_energy(Reclamation::FieldEnergyCost);
                    g_iquare_globals->create_effect(EffectId::ReclamationField, 1.5secs, data.target->position, 2tiles, SpriteId::CausticDischarge,
                        data.target->sprite->elevation + 1, data.attacker->player, data.attacker);
                } break;
                case WeaponId::MindTyrantMaledictus: {
                    helper::mind_control_hit(data.attacker, data.target);
                } break;
                case WeaponId::VorvrokorVorvrokClaws: {
                    if (data.attacker->id == UnitId::ZergVorvrokor) {
                        if (!target_ext.values[ValueVorvalHidebreaker]) {
                            target_ext.values[ValueVorvalHidebreaker] = 1;
                            // TODO: Overlay
                        }
                        target_ext.timers[TimerHidebreaker] = Zerg::HidebreakerDuration;
                    }
                    if (data.attacker->id == UnitId::ZergGarvrokor) {
                        if (!target_ext.values[ValueGarvrolHidebreaker]) {
                            target_ext.values[ValueGarvrolHidebreaker] = 2;
                            // TODO: Overlay
                        }
                        target_ext.timers[TimerHidebreaker] = Zerg::HidebreakerDuration;
                    }
                } break;
                case WeaponId::NathrokorVestigialStrike:
                case WeaponId::NathrokorNathrokClaws: {
                    if (data.attacker->id == UnitId::ZergNathrokor) {
                        if (!target_ext.values[ValueNathrokHidebreaker]) {
                            target_ext.values[ValueNathrokHidebreaker] = 1;
                            // TODO: Overlay
                        }
                        target_ext.timers[TimerHidebreaker] = Zerg::HidebreakerDuration;
                    }
                } break;
            }
            // Ignore Carapace Swarm!
            // END

            // Ensnaring Brood - Matraleth enchantment ability
            if (unit_applies_ensnare(data.attacker) && unit_can_be_ensnared(data.target) && !carapace_swarm_blocks_damage(data.target, data.weapon_id, data.damage_divisor)) {
                apply_ensnare_internal(data.target, bw::get_ensnare_factor(data.attacker));
            }

            if (bw::get_game_kind() == GameKind::Brotherhood && data.target->can_proc_autovitality(data.attacker->player, data.shield_reduce_amount, data.damage)) {
                fp32 time_cost{units_dat::time_cost[data.target->id]};
                time_cost /= data.target->has_shields() ? 3.75fp : 4.5fp;
                target_ext.timers[TimerAutovitalityClone] = static_cast<u32>(time_cost);
                bw::add_unit_overlay(data.target, ImageId::LegionnaireTeleportEffect);
                helper::play_sound_from_unit(SoundId::SimulacrumCloneStart01, data.target, 1, 0);
            }

            switch (data.target->id) {
                case UnitId::TerranStriga: {
                    // Striga - Fleeting Focus
                    if (unit_can_be_repulsed(data.attacker) && !bw::is_under_carapace_swarm(data.target)) {
                        const u32 dist = bw::distance_between(data.attacker->get_x(), data.attacker->get_y(), data.target->get_x(), data.target->get_y());
                        if (dist < 2tiles) {
                            const u32 displacement_time = 5;
                            v2 displacement = data.target->position.dir_to(data.attacker->position);
                            displacement *= 32.0fp / fp32(displacement_time);

                            attacker_ext.timers[TimerStrigaRepulsion] = displacement_time;
                            attacker_ext.repulsion_vector = displacement;
                        }
                    }
                } break;
                case UnitId::ProtossRenesair:
                case UnitId::ProtossSimulacrum:
                case UnitId::ProtossSimulacrumAstral: {
                    // Autovitality
                    if (data.target->can_proc_autovitality(data.attacker->player, data.shield_reduce_amount, data.damage)) {
                        target_ext.timers[TimerAutovitalityClone] = 4secs;
                        bw::add_unit_overlay(data.target, ImageId::LegionnaireTeleportEffect);
                        helper::play_sound_from_unit(SoundId::SimulacrumCloneStart01, data.target, 1, 0); // TODO: Make sound dependent on unit id
                    }
                } break;
                case UnitId::ProtossAnalogue: {
                    // Empathy Core - Analogue behavior
                    if (weapon_damage_current_bullet) {
                        BulletExt* current_bullet_ext = get_bullet_ext(weapon_damage_current_bullet);
                        if (current_bullet_ext->is_phantom) {
                            break;
                        }
                    }
                    if (!weapon_id_can_proc_onhits(data.weapon_id) || weapon_damage_is_lethal(data)) {
                        break;
                    }
                    if (data.target->energy >= ProtossEmpathyCore::EnergyAmount) {
                        data.target->spend_energy(ProtossEmpathyCore::EnergyAmount);

                        u32 damage = data.damage + data.shield_reduce_amount;
                        u32 image_id = analogue_image_id_overwrite(data.weapon_id);
                        if (image_id == ImageId::None) {
                            Bullet* phantom = fire_bullet(data.target, data.weapon_id, make_target(data.attacker), {.apply_cooldown = false, .ignore_weapon_offsets = true});
                            if (phantom) {
                                BulletExt* phantom_ext = get_bullet_ext(phantom);
                                phantom_ext->is_phantom = true;
                                phantom_ext->damage_override = damage;
                            }
                        } else {
                            u32 inv_direction = 255 - data.direction;
                            // attkshiftproj probably does something specific that avoids the iscript errors. -Pr0nogo
                        //	data.target->sprite->create_overlay(image_id, 0, 0, inv_direction);
                            if (data.attacker) {
                                data.attacker->damage_with(data.weapon_id, damage, data.attacker, data.attacking_player_id, inv_direction, 2);
                            }
                        }
                    }
                } break;
                case UnitId::ZergTethzorokor:
                case UnitId::ZergZarcavrokor: {
                    // Fan of Teeth
                    if (data.target != data.attacker && !target_ext.timers[TimerZarcavrokorSecondaryFire]) {
                        fire_bullet(data.target, WeaponId::ZergBonesofReflection, make_target(data.attacker), {.apply_cooldown = false});
                        target_ext.timers[TimerZarcavrokorSecondaryFire] = weapons_dat::cooldown[WeaponId::ZergBonesofReflection] - 1;
                    }
                } break;
                case UnitId::AskosiPhontis: {
                    // Warring Winds
                    if (data.target != data.attacker && !bw::are_mutual_allies(data.target->player, data.attacking_player_id) && !target_ext.timers[TimerWarringWindsCooldown]) {
                        target_ext.timers[TimerWarringWindsCooldown] = Askosi::WarringWindsCooldown;
                        target_ext.values[ValueWarringWindsStacks]++;
                        data.target->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
                        for (Unit* ally : g_unit_search.radial_ex(data.target, Askosi::WarringWindsRange, FilterAlly | FilterIncludingSelf)) {
                            ally->heal_hp(Askosi::WarringWindsHealAmount);

                            // TODO: Own audiovisuals
                            ally->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
                        }
                        if (target_ext.values[ValueWarringWindsStacks] >= 10) {
                            target_ext.values[ValueWarringWindsStacks] = 0;
                            for (Unit* pull_victim : g_unit_search.radial_ex(data.target, Askosi::WarringWindsRange, FilterNoBuilding)) {
                                if (unit_can_be_repulsed(pull_victim)) {
                                    pull_victim->pull_towards(data.target);

                                    // TODO: Own audiovisuals
                                    pull_victim->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
                                }
                            }
                        }
                    }
                } break;
            }

            // These bypass Carapace Swarm
            switch (data.weapon_id) {
                case WeaponId::HarakanFlamethrower:
                case WeaponId::StrigaPhaseSaber:
                case WeaponId::MatadorTwinstrikeFlamethrowers:
                case WeaponId::MadrigalHecatonAngeltearMissiles:
                case WeaponId::DurendalAbaddonFlamethrowers:
                case WeaponId::SalamanderAngeltearRockets:
                case WeaponId::AckmedMickenMelter:
                case WeaponId::YamatoGun: {
                    if (data.attacker->id == UnitId::TerranHarakanGolden) {
                        break;
                    }
                    if (target_ext.values[ValueApollyoidAdhesiveStacks] < ApollyoidAdhesive::StackCap) {
                        target_ext.values[ValueApollyoidAdhesiveStacks]++;
                    }
                    target_ext.timers[TimerApollyoidAdhesive] = ApollyoidAdhesive::Timer;
                } break;
                case WeaponId::CyclopsFlakCannons: {
                    g_iquare_globals->create_effect(EffectId::CyclopsRecursionRound, 2secs, data.target->position, 2tiles, SpriteId::CyclopsPassiveArmingOverlay,
                                                 data.target->sprite->elevation + 1, data.attacker->player, data.attacker);
                } break;
                case WeaponId::CyclopsFuturistFlakCannons: {
                    g_iquare_globals->create_effect(EffectId::CyclopsFuturistRecursionRound, 2secs, data.target->position, 2tiles, SpriteId::CyclopsPassiveArmingOverlay,
                                                 data.target->sprite->elevation + 1, data.attacker->player, data.attacker);
                } break;
            }

            if (!bw::is_under_carapace_swarm(data.target)) {
                // Astral Rule - Patriarch behavior
                if (target_ext.timers[TimerPatriarchAstralRule] > 1
                && units_dat::shields_enabled[data.attacker->id]
                && data.attacker->shields < units_dat::max_shield_points[data.attacker->id] * 256) {
                    data.attacker->heal_shields(5 * 256);
                    helper::add_overlay_below_main(data.attacker->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
                }

                if (target_ext.timers[TimerTriumphantRiseDebuff]) {
                    bw::astral_blessing_proc(data.target, data.attacking_player_id, WeaponId::HeraldStellarTorch);
                }

                switch (data.weapon_id) {
                    case WeaponId::InzorthrokorInzorReflux:
                    case WeaponId::AkistrokorHarryingTalons: {
                        if (data.attacker->id != UnitId::AskosiObject) { // TODO: Extend weapons.datex and remove this
                            mark_infesting(data.attacking_player_id, data.target);
                        }
                    } break;
                    case WeaponId::SavantDiscoidRailgun: {
                        // Power Siphon
                        if (data.attacker->id == UnitId::TerranSavantPowerSiphon && unit_is_combatant(data.target)) {
                            attacker_ext.values[ValueSavantPowerSiphonBuff]++;
                            attacker_ext.timers[TimerSavantPowerSiphonBuff] = 2secs + 1;

                            Unit* final_target = data.target;
                            if (final_target->subunit && final_target->subunit->is_subunit()) {
                                final_target = final_target->subunit;
                            }
                            auto& final_target_ext = final_target->ext();
                            final_target_ext.values[ValueSavantPowerSiphonDebuff]++;
                            final_target_ext.timers[TimerSavantPowerSiphonDebuff] = 2secs + 1;
                            if (final_target != data.target) { // For tooltips. -Pr0nogo
                                target_ext.values[ValueSavantPowerSiphonDebuff]++;
                                target_ext.timers[TimerSavantPowerSiphonDebuff] = 2secs + 1;
                            }
                            Thingy* overlay = helper::create_thingy(SpriteId::PowerSiphonHit_Small + bw::get_overlay_offset(data.target->id), data.target->position.x, data.target->position.y, data.attacking_player_id);
                            overlay->sprite->elevation = data.target->sprite->elevation + 1;
                        }
                    } break;
                    case WeaponId::SevenstepGeometricRifle: {
                        g_iquare_globals->create_effect(EffectId::SevenstepMindMatter, 1secs, data.target->position, 1, SpriteId::SevenstepMindMatterOverlay,
                            data.target->sprite->elevation + 1, data.attacker->player, data.attacker, { .origin = data.target });
                    } break;
                    case WeaponId::SevenstepGeometricRiflePiercing: {
                        if (!target_ext.timers[TimerSevenstepMindMatterCarrier]) {
                            g_iquare_globals->create_effect(EffectId::SevenstepMindMatter, 1secs, data.target->position, 1, SpriteId::SevenstepMindMatterOverlay,
                                data.target->sprite->elevation + 1, data.attacker->player, data.attacker, { .origin = data.target });
                        }
                    } break;
                    case WeaponId::PazuzuJupiterDriver: {
                        // Deconstruction - Pazuzu behavior - apply stack
                        if (unit_is_combatant(data.target)) {
                            target_ext.values[ValueDeconstruction]++;
                            target_ext.timers[TimerDeconstruction] = 5secs + 1;
                            target_ext.values[ValueDeconstructionPlayer] = data.attacker->player;
                            u32 overlay = ImageId::DeconstructionSmall + bw::get_overlay_offset(data.target->id);
                            if (data.target->subunit) {
                                UnitExt* subunit_ext = get_unit_ext(data.target->subunit);
                                subunit_ext->values[ValueDeconstruction]++;
                                subunit_ext->timers[TimerDeconstruction] = 5secs + 1;
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
                    case WeaponId::HeraclesJavelinCannons:
                    case WeaponId::ClaymoreHarpoonRailgun:
                    case WeaponId::TritonHydriaCannons: {
                        // Kelvin Munitions - Slow targets
                        target_ext.timers[TimerKelvinMunitions] = 3secs + 1;
                    } break;
                    case WeaponId::VicarOspoTwinshard: {
                        if (data.attacker->id == UnitId::AskosiParaclete) {
                            break;
                        }
                        if (!target_ext.timers[TimerBoomingBlast]) {
                            target_ext.timers[TimerBoomingBlast] = Askosi::BoomingBlastTimer;
                            target_ext.values[ValueBoomingBlastPlayer] = data.attacking_player_id;
                        }
                        if (data.attacker->id == UnitId::AskosiIconoclast) {
                            target_ext.timers[TimerGoldRush] = Askosi::ScrapworldGoldRushTimer;
                            target_ext.values[ValueGoldRushPlayer] = data.attacking_player_id;
                        }
                    } break;
                    case WeaponId::GrandstandArteDriver:
                    case WeaponId::IntendentErsaDriver: {
                        if (data.attacker && data.attacker->id == UnitId::AskosiZonguard) {
                            break;
                        }
                        if (!target_ext.timers[TimerPowerOverload]) {
                            target_ext.timers[TimerPowerOverload] = Askosi::PowerOverloadTimer * hooks::get_armor_internal(data.target) + 1;
                        }
                    } break;
                    case WeaponId::AspirantPotentiumCannon: {
                        // Aspirant - Criticality
                        target_ext.timers[TimerCriticality] = 2secs + 1;
                    } break;
                    case WeaponId::GorgonArtificeBattery: {
                        // Petrification - Gorgon behavior
                        if (target_ext.values[ValuePetrificationStacks] < 20) {
                            target_ext.values[ValuePetrificationStacks] += 1;
                        }
                        target_ext.timers[TimerPetrification] = 2secs + 1;
                    } break;
                    case WeaponId::SirenCharybdalRifle:
                    case WeaponId::MagnetarNewtonianShockCannon: {
                        // Magnetar - Law of Attraction - Pull target to attacker
                        if (unit_can_be_repulsed(data.target)) {
                            const u32 displacement_time = 5;
                            v2 displacement = data.target->sprite->position.dir_to(data.attacker->sprite->position);
                            displacement *= 2fp * 32.0fp / fp32(displacement_time);
                            target_ext.timers[TimerMagnetarAttraction] = displacement_time;
                            target_ext.repulsion_vector = displacement;

                            u32 overlay_id = bw::rand_between(SpriteId::MagnetarLawOfAttraction1, SpriteId::MagnetarLawOfAttraction3);
                            bw::create_and_init_thingy(overlay_id, data.target->position.x, data.target->position.y, data.attacker->player, data.target->sprite->elevation + 1);
                        }
                    } break;
                    case WeaponId::HeraldStellarTorch:
                    case WeaponId::ArchonPsionicShockwave: {
                        if (!bw::is_allied_to(data.attacking_player_id, data.target->player)) {
                            // TODO: Audiovisuals
                            target_ext.timers[TimerTriumphantRiseDebuff] = 1.5secs;
                        }
                    } break;
                    case WeaponId::AshguardPhaseCremator: {
                        // Vengeful March
                        attacker_ext.timers[TimerVengefulMarch] = Protoss::VengefulMarchDuration;

                        // Ashguard - Indomitable Annex - add stack
                        target_ext.units[UnitAnnexSource] = data.attacker;
                        if (target_ext.indomitable_annex_stacks < 5) {
                            target_ext.indomitable_annex_stacks++;
                        }
                        target_ext.values[ValueAnnexSourcePlayer] = data.attacking_player_id;
                        target_ext.timers[TimerAnnex] = 2secs + 1;
                    } break;
                    case WeaponId::EmpressFadingBioplasma: {
                        // Empress - Anodyne Annex - add stack
                        if (!bw::is_allied_to(data.attacking_player_id, data.target->player)) {
                            target_ext.units[UnitAnnexSource] = data.attacker;
                            target_ext.values[ValueEmpressAnodyneAnnexStacks]++;
                            target_ext.values[ValueAnnexSourcePlayer] = data.attacking_player_id;
                            target_ext.timers[TimerAnnex] = 2secs + 1;
                        }
                    } break;
                    case WeaponId::GhitorokorGnashingTendrils: {
                        attacker_ext.values[ValueGhitorokorRelentlessAggressionStacks]++;
                        attacker_ext.timers[TimerGhitorokorRelentlessAggression] = 2secs + 1;
                    } break;
                    case WeaponId::IziraliskCorrosiveBioplasma: {
                        helper::apply_corrosive_spores(data.target);
                    } break;
                    case WeaponId::VithriliskCorrosiveSpores:
                    case WeaponId::SkryrokorPlasmaMetamorphosis: {
                        corrosive_pathogen_proc(data.attacker, data.target);
                    } break;
                    case WeaponId::OgroszalethAcidBarrage: {
                        corrosive_pathogen_proc(data.attacker, data.target);
                        swarming_omen_proc(data.target, data.attacking_player_id);
                    }
                    case WeaponId::ZoryusthalethMonstrousMaw: {
                        swarming_omen_proc(data.target, data.attacking_player_id);
                    } break;
                    case WeaponId::KonviliskSowingSpines: {
                        // Fertile Spines - incubate
                        if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                            incubate(data.target, 1, UnitId::ZergRilirokor, data.attacking_player_id);
                        }
                    } break;
                    case WeaponId::TetcaerokorCleavingMaw: {
                        // Virile Coating - incubate
                        if (!bw::is_allied_to(data.target->player, data.attacking_player_id)) {
                            incubate(data.target, 1, UnitId::ZergRililisk, data.attacking_player_id);
                        }
                    } break;
                    case WeaponId::HierophantPhaseSignifier:
                    case WeaponId::EpigraphLitanyCannons:
                    case WeaponId::PatriarchIronhandWeaver: {
                        target_ext.values[ValueSignifyStacks]++;
                        target_ext.timers[TimerSignify] = 3secs + 1;
                        target_ext.values[ValueSignifyPlayer] = data.attacking_player_id;
                        bw::update_signify_status(data.target, &target_ext);
                    } break;
                    case WeaponId::ApostleCharonRifle: {
                        if (!bw::is_allied_to(data.attacking_player_id, data.target->player)) {
                            if (data.attacker->energy < TerranApostle::BloodTitheEnergyCost) {
                                break;
                            }
                            bool healed = false;
                            for (auto ally : g_unit_search.radial_ex(data.attacker, TerranApostle::BloodTitheRange, FilterIncludingSelf | FilterAlly | FilterCompleted)) {
                                if (ally->hitpoints < ally->get_max_hp_raw()) {
                                    ally->heal_hp(TerranApostle::BloodTitheHeal, data.attacker);
                                    helper::add_overlay_below_main(ally->sprite, ImageId::HealingPulseUnderlay, 0, 0, 0);
                                    healed = true;
                                }
                            }
                            if (healed) {
                                data.attacker->spend_energy(TerranApostle::BloodTitheEnergyCost);
                            }
                            if (data.attacker->energy < TerranApostle::LazarusAgentEnergyCost) {
                                break;
                            }
                            Thingy* corpse = get_closest_active_corpse(data.attacker->position, 3tiles);
                            if (corpse) {
                                reclaim_corpse(corpse, data.attacking_player_id);
                                data.attacker->spend_energy(TerranApostle::LazarusAgentEnergyCost);
                            }
                        }
                    } break;
                    case WeaponId::EcclesiastAstralBlaster:
                    case WeaponId::VibranceSensoryOverload:
                    case WeaponId::ExtatusRepeatingBlaster:
                    case WeaponId::MatrixKhaydarinRay: {
                        // Astral Blessing
                        if (data.attacker->id != UnitId::ProtossEcclesiastKhalendric && data.attacking_player_id != data.target->player && !bw::is_allied_to(data.attacking_player_id, data.target->player)) {
                            bw::astral_blessing_proc(data.target, data.attacking_player_id, data.weapon_id);
                        }
                    } break;
                    case WeaponId::CharlatanFerventBlaster: {
                        // Charlatan - Leading the Blind passive
                        if (!bw::is_allied_to(data.attacker->player, data.target->player)) {
                            bw::apply_hallucinating(data.target, data.attacker->player, 25);
                        }
                    } break;
                    case WeaponId::MagisterNeutronCannon: {
                        // Stellar Enforcement - Magister behavior
                        if (!bw::is_allied_to(data.attacker->player, data.target->player) && !target_ext.timers[TimerStellarEnforcement]) {
                            target_ext.timers[TimerStellarEnforcement] = 3secs + 1;
                            target_ext.timers[TimerStellarEnforcementOverlay] = 30;
                            bw::add_unit_overlay(data.target, ImageId::StellarEnforcementDebuff_Small + bw::get_overlay_offset(data.target->id, true));  // Inlined here because otherwise the unit would have the overlay 1 frame late! -Pr0nogo
                        }
                    } break;
                    case WeaponId::GoskaMedusaRifle: {
                        bw::refresh_debuffs(data.target);
                    } break;
                    case WeaponId::CardinalInoaTwinaxe: {
                        // Every second strike, the Cardinal's weapon releases a restorative pulse, granting to all allies within 3 range a heal equal to damage dealt.
                        if (data.attacker->order_signal & 0x20) {
                            data.attacker->order_signal &= ~0x20;
                            bw::heal_proc(data.attacker, data.attacking_player_id, AskosiCardinal::SustainingSermonRange, AskosiCardinal::SustainingSermonHeal);
                        }
                    } break;
                    case WeaponId::CrematoriumOkliQuadshard: {
                        target_ext.timers[TimerAshHeap] = Askosi::ScrapworldAshHeapDuration;
                        target_ext.timers[TimerAshHeapArmorRend] += Askosi::ScrapworldAshHeapDuration - 1;
                        target_ext.values[ValueAshHeapPlayer] = data.attacking_player_id;
                    } break;
                }
            }
        }
    }

    static void weapon_damage_lifesteal(const WeaponDamageData& data) {
        s32 damage_dealt = data.damage + data.shield_reduce_amount;
        fp32 hp_factor{};
        fp32 shields_factor{};

        auto& attacker_ext = data.attacker->ext();
        if (attacker_ext.timers[TimerLucilleriPheremones]) {
            hp_factor += 0.5fp;
        }
        switch (data.weapon_id) {
            case WeaponId::GhitorokorGnashingTendrils:
            case WeaponId::UltrokorKaiserBlades: {
                if (data.attacker->id == UnitId::AskosiZonguard) {
                    break;
                }
                hp_factor += 1;
            } break;
            case WeaponId::TethzorokorSaberArray: {
                hp_factor += 1;
            } break;
            case WeaponId::MortothrokorWickedIncisors: {
                hp_factor += attacker_ext.values[ValueMortothrokorStacks] * 0.05fp;
            } break;
            case WeaponId::AdherentPhaseBlade:
            case WeaponId::AmaranthPsiCleaver: {
                shields_factor += 1;
            } break;
        }

        if (g_active_game_mode) {
            g_active_game_mode->on_unit_damage_lifesteal(data, hp_factor, shields_factor);
        }
        if (hp_factor) {
            data.attacker->heal_hp(static_cast<s32>(damage_dealt * hp_factor), data.attacker);
        }
        if (shields_factor) {
            auto healing_amount = static_cast<s32>(damage_dealt * shields_factor);
            data.attacker->heal_shields(healing_amount, data.attacker);
            if (data.attacker->id == UnitId::ProtossAmaranth) {
                attacker_ext.values[ValueAmaranthHealAmount] += healing_amount;
            }
        }
    }

    static bool weapon_damage_fatal(const WeaponDamageData& data) {
        bool fatal_deal_damage = true;
        auto& target_ext = data.target->ext();

        // Note that the order here matters a lot
        //
        // 1. Plague
        // 2. Game Mode
        // 3. Unit-specific behaviors
        // 4. Hijacker exit
        // 5. Infestation
        // 6. Regression
        // 7. Tyranny of the Sun
        // 8. Zombification
        // 9. Miscellaneous lesser behaviors
        // 
        // We exit early to not trigger multiple different fatal damage handling at once

        if (g_active_game_mode) {
            if (!g_active_game_mode->on_unit_damage_fatal(data)) {
                return false;
            }
        }

        switch (data.weapon_id) {
            case WeaponId::Plague: {
                // Plague - Don't kill
                fatal_deal_damage = false;
                data.target->set_hitpoints(256);
                return fatal_deal_damage;
            } break;
        }

        switch (data.target->id) { // TARGET SWITCH! MUST BE UNIT
            case UnitId::TerranCuirassTankMode: {
                // Cuirass - Stonewall Protocol
                if (!target_ext.values[ValueCuirassIsSelfReviving]) {
                    target_ext.values[ValueCuirassIsSelfReviving] = 1;
                    data.target->hitpoints = Cuirass::SelfReviveHP;
                    fatal_deal_damage = false;
                    data.target->order_computer(OrderId::Stop);
                    data.target->status |= UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack;
                    helper::play_sound_from_unit(SoundId::Misc_TPwrDown_wav, data.target, 1, 0);
                }
            } break;
            case UnitId::ProtossCicatus: {
                if (target_ext.values[ValueIsShade]) {
                    break;
                }
                helper::increment_unit_death_scores(data.target, data.target->player);
                data.target->shields = units_dat::max_shield_points[data.target->id] * 256;
                data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id]);
                data.target->sprite->create_top_overlay(ImageId::ExplosionSmallBlueReversed);
                target_ext.values[ValueIsShade] = true;

                if (game_kind_doubles_units() && unit_should_give_gas_in_massacre_mode(data.target)) {
                    bw::add_massacre_resource(data.target->player);
                }

                u32 sound = bw::rand_between(SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04);
                if (helper::play_sound_from_unit(sound, data.target, g_options_list->flags & 1, 99) && !g_is_in_replay && data.target->player == g_local_nation_id) {
                    helper::do_unit_event_notification(data.target, 17, sfx_dat::portrait_duration[sound] + 1000 * (sfx_dat::sound_data[sound].duration_ms / 1000), 0, sound);
                }
                fatal_deal_damage = false;
            } break;
            case UnitId::ProtossVagrant: {
                helper::increment_unit_death_scores(data.target, data.target->player);
                helper::replace_unit_with_id(data.target, UnitId::ProtossVagrantShade);
                data.target->shields = units_dat::max_shield_points[data.target->id] * 256;
                data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id]);
                data.target->sprite->create_top_overlay(ImageId::ExplosionSmallBlueReversed);

                if (game_kind_doubles_units() && unit_should_give_gas_in_massacre_mode(data.target)) {
                    bw::add_massacre_resource(data.target->player);
                }

                u32 sound = bw::rand_between(SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04);
                if (helper::play_sound_from_unit(sound, data.target, g_options_list->flags & 1, 99) && !g_is_in_replay && data.target->player == g_local_nation_id) {
                    helper::do_unit_event_notification(data.target, 17, sfx_dat::portrait_duration[sound] + 1000 * (sfx_dat::sound_data[sound].duration_ms / 1000), 0, sound);
                }
                fatal_deal_damage = false;
            } break;
            case UnitId::ProtossAurora: {
                // Last Orders - Aurora behavior
                if (target_ext.values[ValueLastOrdersInit] == 0) {
                    target_ext.values[ValueLastOrdersInit] = 1;

                    auto targets = g_unit_search
                        .radial_ex(data.target, 9tiles, FilterEnemy | FilterInvincible | FilterVisible)
                        .sort_by_distance(data.target->position);
                    if (targets.count()) {
                        Unit* nearest = (Unit*)targets[0];
                        if (nearest) {
                            fatal_deal_damage = false;
                            data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id] / 2);
                            data.target->order_to(OrderId::Move, nearest->position.x, nearest->position.y);
                            data.target->status |= UnitStatus::CanNotReceiveOrders;
                            data.target->sprite->create_top_overlay(ImageId::GodsMadeFlesh_Small);
                        }
                    }
                }
            } break;
            case UnitId::ZergAxitrilisk: {
                helper::increment_unit_death_scores(data.target, data.target->player);
                helper::replace_unit_with_id(data.target, UnitId::ZergAxitriliskWeaponless);
                data.target->set_hitpoints(units_dat::max_hitpoints[data.target->id]);
                data.target->sprite->create_top_overlay(ImageId::ZergAirDeathExplosion_Small);
                helper::play_sound_from_unit(SoundId::Zerg_Hydra_ZHyDth00_WAV, data.target, 1, 0);
                target_ext.timers[TimerAxitriliskPerpetualWind] = Axitrilisk::PerpetualWindTimer;
                fatal_deal_damage = false;
            } break;
            case UnitId::ZergVilgorokor: {
                // Vilgorokor - On Death behavior
                bw::create_unit_from_ability(UnitId::ZergVilgoleth, 1, data.target->player, data.target->position);
                bw::create_unit_from_ability(UnitId::ZergRilirokor, 2, data.target->player, data.target->position);
            } break;
            case UnitId::TerranCohort: {
                if (target_ext.values[ValueCohortServileVigor]) {
                    break;
                }
                target_ext.values[ValueNoCorpse] = true;
                Unit* cohort = bw::create_unit_in_game(UnitId::TerranCohort, data.target->player, data.target->position.x, data.target->position.y, data.target);
                if (cohort) {
                    bw::inherit_debuffs(data.target, cohort);
                    auto& cohort_ext = cohort->ext();
                    cohort_ext.values[ValueCohortServileVigor] = true;
                    cohort_ext.values[ValueNoCorpse] = false;
                    cohort_ext.timers[TimerCorpseConsumptionDelay] = 1secs;
                    cohort->remove_timer = 240;
                }
                return true;
            } break;
            case UnitId::AskosiThaumalogue:
            case UnitId::AskosiObject:
            case UnitId::AskosiScepter:
            case UnitId::AskosiPetract:
            case UnitId::AskosiTirade: {
                if (!target_ext.values[ValueReassemblyActive]) {
                    target_ext.values[ValueReassemblyActive] = true;
                    data.target->heal_hp(data.target->get_max_hp_raw());
                    data.target->play_iscript_animation(IscriptAnimation::SpecialState1);
                    data.target->update_speed();
                    bw::play_sound({SoundId::AskosiDeploy01, SoundId::AskosiDeploy03}, data.target->position);
                    data.target->sprite->create_top_overlay(ImageId::ArchitectWeaponChannel);
                    if (data.target->id == UnitId::AskosiObject) {
                        data.target->status |= UnitStatus::CanNotReceiveOrders;
                    }
                    fatal_deal_damage = false;
                }
            } break;
        }

        if (fatal_deal_damage) {
            // Askosi Ash Heap
            if (target_ext.timers[TimerAshHeap]) {
                for (Unit* nearby : g_unit_search.radial_ex(data.target, 2tiles, FilterNotSelf)) {
                    auto ash_heap_player = target_ext.values[ValueAshHeapPlayer];
                    if (bw::is_allied_to(nearby->player, ash_heap_player)) {
                        continue;
                    }
                    auto& nearby_ext = nearby->ext();
                    nearby_ext.values[ValueAshHeapPlayer] = ash_heap_player;
                    nearby_ext.timers[TimerAshHeap] = Askosi::ScrapworldAshHeapDuration;
                    nearby_ext.timers[TimerAshHeapArmorRend] += Askosi::ScrapworldAshHeapDuration;
                }
            }

            Unit* hijacker = target_ext.units[UnitHijacker];
            if (hijacker) {
                // Hijack target on fatal damage
                if (target_ext.timers[TimerIsBeingHijacked] && !data.target->is_askosi_embedded()) {
                    data.target->hijack(hijacker);
                    target_ext.timers[TimerIsBeingHijacked] = 0;
                    data.target->update_speed();
                    return false;
                }
                // Hijacker escapes fallen target
                helper::move_unit(hijacker, data.target->position.x, data.target->position.y);
                helper::show_and_enable_unit(hijacker);
                hijacker->status &= ~UnitStatus::NoCollide;
                target_ext.units[UnitHijacker] = 0;
            }

            // Askosi Booming Blast
            if (target_ext.timers[TimerBoomingBlast] > 1) {
                data.target->booming_blast_proc(false);
            }

            // Matraleth, Akistokor - Blissful Embrace - infest target
            if (target_ext.timers[TimerInfesting]) {
                fatal_deal_damage = false;
                blissful_embrace(data.target, target_ext.values[ValueInfestingPlayer]);
                data.target->set_hitpoints(100 * 256 + units_dat::max_hitpoints[data.target->id] / 3);
                return fatal_deal_damage;
            }

            // Zerg Regression
            u32 regression_id = bw::get_zerg_regression(data.target);
            if (data.damage > 0 && regression_id != UnitId::None) {
                if (data.target->id != UnitId::ZergEskigant && data.target->id != UnitId::ZergExcisant && !(units_dat::base_property[regression_id] & UnitProperty::Worker) && !unit_id_is_larva(regression_id)) {
                    fatal_deal_damage = false;
                }

                // Kill flying -> ground regressions if placement is impossible. Otherwise, no corpse is created! -Pr0nogo
                if (!(units_dat::base_property[regression_id] & UnitProperty::Flyer) && units_dat::base_property[data.target->id] & UnitProperty::Flyer) {
                    v2u16 position = data.target->position;
                    position.y += AirToGroundDistance;
                    const auto& tile = bw::get_tile_at_coords(position);
                    if (tile.is_unwalkable) {
                        return true;
                    }
                }
                if (data.target->is_valid_nydus()) {
                    Unit* nydus_exit = data.target->building.nydus_exit;
                    data.target->building.nydus_exit = 0;
                    nydus_exit->building.nydus_exit = 0;

                    if (!target_ext.values[ValueIsNydusExit] == 1) {
                        nydus_exit->bwfunc_remove();
                    }
                }
                bw::regress(data.target, regression_id);
                if (data.target) {
                    unit_on_death_behaviors(data.target);
                    if (data.attacker && !bw::is_allied_to(data.attacking_player_id, data.target->player)) { // Personally I think it's fine if kills are tracked regardless of allegiance,
                                                                                                                // but we need to hook 004797B0 to make this a global rule. -Pr0nogo
                        helper::increment_unit_kill_count(data.attacker);
                    }
                }

                // Fix burrowed regression / transforms. -DF
                if (data.target->is_burrowed()) {
                    data.target->main_order_id = OrderId::Unburrow;
                }
                return fatal_deal_damage;
            }

            // Malefactor Tyrannize
            if (data.attacker) {
                if (target_ext.timers[TimerMalefactorTyrannize] && !target_ext.values[ValueLastOrdersInit]) {
                    fatal_deal_damage = false;
                    mind_control(target_ext.last_malefactor_player, UnitId::ProtossMindTyrant, data.target);
                    bw::play_sound(SoundId::Protoss_Darchon_mind_wav, data.target->position);
                    data.target->set_hitpoints(100 * 256 + units_dat::max_hitpoints[data.target->id] / 3);
                    data.target->freeze();
                    target_ext.timers[TimerMalefactorStun] = 2secs + 1;
                    data.target->order_to_idle();
                    return fatal_deal_damage;
                }
            }

            bool energy_spent = false;
            auto masters = g_unit_search
                .radial_ex(data.target, 6tiles, 0, {UnitId::ProtossDemiurge})
                .filter_by([&data, &energy_spent](Unit* master) {
                    if (bw::is_allied_to(data.target->player, master->player)) {
                        return false;
                    }
                    if (master->energy < ProtossDemiurge::WorshipEnergyCost) {
                        return false;
                    }
                    if (!energy_spent) {
                        master->spend_energy(ProtossDemiurge::WorshipEnergyCost);
                        energy_spent = true;
                    }
                    return true;
                });
            if (!data.target->is_building() && masters.count() && !target_ext.values[ValueLastOrdersInit]) {
                fatal_deal_damage = false;
                target_ext.timers[TimerDemiurgeWorship] = 5secs + 1;
                bw::remove_unit_overlay(data.target, ImageId::Malediction_Small + bw::get_overlay_offset(data.target->id));
                u32 overlay_id = ImageId::DemiurgeWorshipProc_Small + bw::get_overlay_offset(data.target->id, true);
                if (overlay_id == ImageId::ControlledDemolition) {
                    overlay_id = ImageId::DemiurgeWorshipProc_Large;
                }
                bw::add_unit_overlay(data.target, overlay_id);
                bw::play_sound(SoundId::DemiurgeWorship01, data.target);
                target_ext.timers[TimerDemiurgeWorshipOverlayDelay] = 1.75secs + 1;
                data.target->freeze();
                data.target->status |= UnitStatus::Invincible;
                return fatal_deal_damage;
            }
            
            // Empress, Ashguard - Zombify target
            bool can_be_annexed = target_ext.values[ValueEmpressAnodyneAnnexStacks] || target_ext.indomitable_annex_stacks == KhalendricAshguard::IndomitableAnnexCap;
            if (can_be_annexed && !target_ext.values[ValueIsZombified] && !target_ext.values[ValueLastOrdersInit]) {
                fatal_deal_damage = false;
                auto stack_count = target_ext.values[ValueEmpressAnodyneAnnexStacks];
                if (!stack_count) {
                    stack_count = KhalendricAshguard::IndomitableAnnexTimedLife;
                }
                empress_zombify(target_ext.values[ValueAnnexSourcePlayer], target_ext.units[UnitAnnexSource], data.target, stack_count);
                return fatal_deal_damage;
            }

            // Create effect fields
            if (target_ext.timers[TimerCriticality]) {
                g_iquare_globals->create_effect(EffectId::AspirantCriticality, 5secs, { data.target->position.x, data.target->position.y }, 2tiles, SpriteId::Criticality, 2, data.attacking_player_id, data.target);
            }
        }
        return fatal_deal_damage;
    }

    void apply_ensnare(Unit* unit) { // 004F45E0
        // Overlay
        if (!(unit->ensnare_timer) && !unit->is_burrowed()) {
            bw::add_unit_overlay(unit, ImageId::EnsnareOverlay_Small + bw::get_overlay_offset(unit->id));
        }

        // Effect
        unit->ensnare_timer = 5;
        unit->update_speed();
    }

    u16 get_spores_overlay_id(Unit* target) {
        u32 stacks = target->ext().values[ValueCorrosiveTouchStacks];
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
        return overlay_multiplier + 4 * bw::get_overlay_offset(target->id) + ImageId::AcidSpores_1_Overlay_Small;
    }

    // Hooked to change Corrosive Spores timer - Pr0nogo
    s32 apply_corrosive_spores(Unit* target) { // 004F4480
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

        target_ext->timers[TimerCorrosiveTouch] = CorrosiveTouch::Timer;
        return 0;
    }

    // Hooked to reveal fog around attackers when they attack allies. -Pr0nogo
    void unit_was_hit(Unit* victim, Unit* attacker, u32 reveal) { // 004795D0
        if (!attacker || attacker->player == victim->player) {
            return;
        }
        victim->last_attacking_player = attacker->player;
        if (victim->unit_ai && !victim->auto_target_unit) {
            victim->auto_target_unit = attacker;
            if (victim->subunit && victim->subunit->is_subunit()) {
                victim->subunit->last_attacking_player = victim->last_attacking_player;
                victim->subunit->auto_target_unit = attacker;
            }
        }
        helper::ai_unit_was_hit(victim, attacker, 1, 0);
        if (reveal && attacker->sprite) {
            if (!helper::unit_can_see_cloaked_target(attacker, victim)) {
                bw::reveal_attacker(attacker, victim);
                if (attacker->id == UnitId::ProtossSteward && attacker->hangar_unit.parent) {
                    bw::reveal_attacker(attacker->hangar_unit.parent, victim);
                }
            }
        }
        if (attacker->is_garrisoned() && attacker->connected_unit) { // This nullcheck is temporary until we fix the source of the issue. -Pr0nogo
            attacker = attacker->connected_unit;
        }
        if (victim->main_order_id != OrderId::HoldPositionNormal && helper::is_better_target(victim, victim->auto_target_unit, attacker)) {
            helper::react_to_hit(attacker, victim);
        }
        if (!victim->unit_ai) {
            for (Unit* unit : g_unit_search.radial(victim->position, 96)) {
                if (unit != victim && unit->player == victim->player) {
                    helper::ask_for_help(attacker, unit);
                }
            }
        }
    }

    // Internal version of is_better_target 004769D0
    bool is_better_target2(Unit* self, Unit* auto_target, Unit* compare) {
        if (compare != auto_target) {
            u32 cmp_player = compare->player == 11 ? compare->sprite->player : compare->player;
            if (!bw::is_allied_to(self->player, cmp_player)) {
                if (!auto_target) {
                    return self->can_attack_target(compare, 1);
                }
                return !self->can_attack_target(auto_target, 1) || !self->is_target_enemy(auto_target)
                    || (helper::is_unit_in_weapon_range(self, compare) && helper::is_unit_in_weapon_range(self, auto_target));
            }
        }
        return false;
    }

    // Hooked to investigate crashes. -Pr0nogo
    u32 ask_for_help(Unit* attacker, Unit* helper) { // 00476E60
        // Originally had code to prohibit Didact and Danimoth unit IDs.
        if (helper->is_worker() || helper->is_burrowed() || !helper->has_weapon()) {
            return false;
        }
        if (!hooks::is_better_target2(helper, helper->auto_target_unit, attacker)) {
            return false;
        }

        Unit* subunit = helper->subunit;
        helper->auto_target_unit = attacker;
        if (subunit->is_subunit()) {
            subunit->last_attacking_player = helper->last_attacking_player;
            helper->subunit->auto_target_unit = attacker;
        }
        return true;
    }

    // Hooked to stop damage from forcing units to unburrow. -Pr0nogo
    void react_to_hit(Unit* attacker, Unit* victim) { // 004776C0
        if (unit_id_is_larva(victim->id)) {
            return;
        }
        bool is_burrowed_attacker = unit_id_is_burrowed_attacker(victim->id);
        if (victim->is_burrowed() && !victim->irradiate_timer && !is_burrowed_attacker) {
            if (victim->can_burrow() && victim->is_computer_controlled()) {
                helper::unburrow(victim); // For now, we unburrow AI units only. -Pr0nogo
            }
            return;
        }
        if (victim->is_building() || (helper::unit_can_attack_target(attacker, victim, 1) && !victim->is_worker())) {
            victim->auto_target_unit = attacker;
            Unit* subunit = victim->subunit;
            if (subunit && subunit->is_subunit()) {
                subunit->last_attacking_player = victim->last_attacking_player;
                subunit->auto_target_unit = attacker;
            }
        } else if (!is_burrowed_attacker || !(victim->status & (UnitStatus::Burrowed | UnitStatus::InBuilding)) || victim->unit_ai && !victim->irradiate_timer) {
            if (victim->is_subunit()) {
                victim = victim->subunit;
            }
            if (bw::unit_can_flee(victim)) {
                Target flee_target = helper::ai_prepare_to_flee(victim, attacker);
                helper::order_interrupt(victim, 0, flee_target.pt.x, flee_target.pt.y, OrderId::Move);
            }
        }
    }
}
