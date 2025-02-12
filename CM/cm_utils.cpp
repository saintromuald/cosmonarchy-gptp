#include "cm_utils.h"
#include "BW\api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"

Box32 box32_rect(Point16 pos, u32 w, u32 h) {
    Box32 box = {};
    box.left   = pos.x - w / 2;
    box.right  = pos.x + w / 2;
    box.top    = pos.y - h / 2;
    box.bottom = pos.y + h / 2;
    return box;
}

Box32 box32_square(Point16 pos, u32 w) {
    Box32 box = {};
    box.left   = pos.x - w / 2;
    box.right  = pos.x + w / 2;
    box.top    = pos.y - w / 2;
    box.bottom = pos.y + w / 2;
    return box;
}

bool is_hostile_or_neutral(u32 player, u32 other_player) {
    if (other_player == NeutralPlayer) {
        return true;
    }
    return player != other_player && !bw::is_allied_to(player, other_player);
}

void replay_witness_event(Unit* unit, SpectatorEvent event_type) {
    if (g_is_in_replay || g_globals->players[g_local_nation_id].witness_mode) {
        bool ping = true;
        Point16 position = {};
        switch (event_type) {
            case SpectatorEvent_Combat: {
                position = { unit->position.x, unit->position.y };
            } break;
            case SpectatorEvent_Tech: {
                position = { unit->position.x, unit->position.y };
            //	helper::play_sound_global(units_dat::sound_yes_first[unit->id]);
            } break;
        }
        if (ping) {
            helper::minimap_ping(position.x, position.y, 17);
        }
    }
}

u32 get_construction_graphic(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    bool is_ocsal = unit_ext->values[ValueRegressionUnstable];
    switch (unit->id) {
        case UnitId::ZergIroleth: {
            if (is_ocsal) {
                return ImageId::IrolethBirthEgg;
            } else {
                return ImageId::IrolethBirthCocoon;
            }
        } break;
        case UnitId::ZergKalkalisk: {
            if (is_ocsal) {
                return ImageId::KalkaliskBirthEgg;
            } else {
                return ImageId::KalkaliskBirthCocoon;
            }
        } break;
        case UnitId::ZergTosgrilisk: {
            if (is_ocsal) {
                return ImageId::TosgriliskBirthEgg;
            } else {
                return ImageId::TosgriliskBirthCocoon;
            }
        } break;
        case UnitId::ZergAlgoztalisk: {
            if (is_ocsal) {
                return ImageId::AlgoztaliskBirthEgg;
            } else {
                return ImageId::AlgoztaliskBirthCocoon;
            }
        } break;
        case UnitId::ZergAlkajelisk: {
            if (is_ocsal) {
                return ImageId::AlkajeliskBirthEgg;
            } else {
                return ImageId::AlkajeliskBirthCocoon;
            }
        } break;
        case UnitId::ZergZarcavrokor: {
            if (is_ocsal) {
                return ImageId::ZarcavrokorBirthEgg;
            } else {
                return ImageId::ZarcavrokorBirthCocoon;
            }
        } break;
    }
    return units_dat::construction_graphic[unit->id];
}

void init_omnipresence() {
    for (u32 player = 0; player < PlayerLimit; player++) {
        if (g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot) {
            continue;
        }

        auto& game_mode_race = g_globals->players[player].game_mode_race;
        if (game_mode_race == GameModeRace_Default) {
            game_mode_race = GameModeRace_All;
        }
    }
}

void init_duopoly() {
    for (u32 player = 0; player < PlayerLimit; player++) {
        if (g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot) {
            continue;
        }

        auto& game_mode_race = g_globals->players[player].game_mode_race;
        if (game_mode_race == GameModeRace_Default) {
            game_mode_race = GameModeRace_None;
            switch (g_player_table[player].race) {
                case RaceId::Terran: {
                    game_mode_race = GameModeRace_TerranProtoss;
                } break;
                case RaceId::Protoss: {
                    game_mode_race = GameModeRace_ProtossZerg;
                } break;
                case RaceId::Zerg: {
                    game_mode_race = GameModeRace_ZergTerran;
                } break;
            }
        }
    }
}

void disconnect_addon_no_event(Unit* unit) {
    Unit* addon = unit->building.addon;
    unit->building.addon = 0;

    UnitExt* addon_ext = get_unit_ext(addon);
    addon_ext->values[ValueAddonInactive] = 1;
    bw::refresh_console();

    Unit* active_unit = *g_active_iscript_unit;
    *g_active_iscript_unit = addon;
    for (Image* i = addon->sprite->images.head; i; i = i->link.next) {
        helper::play_image_iscript(i, IscriptAnimation::LiftOff);
    }
    *g_active_iscript_unit = active_unit;
}

u32 get_bullet_damage(Bullet* bullet, u32 weapon_id) {
    BulletExt* bullet_ext = get_bullet_ext(bullet);
    if (bullet_ext->damage_override) {
        return bullet_ext->damage_override;
    }
    return weapons_dat::damage_amount[weapon_id] * 256;
}

const char* game_kind_to_string(GameKind game_kind) {
    switch (game_kind) {
        case GameKind::Competitive: {
            return "Competitive";
        } break;
        case GameKind::Debug: {
            return "Debug";
        } break;
        case GameKind::Scenario: {
            return "Scenario";
        } break;
        case GameKind::FactionWars: {
            return "Faction Wars";
        } break;
        case GameKind::Massacre: {
            return "Massacre";
        } break;
        case GameKind::Omnipresence: {
            return "Omnipresence";
        } break;
        case GameKind::TheTurning: {
            return "The Turning";
        } break;
        case GameKind::Uprising: {
            return "Uprising";
        } break;
        case GameKind::Duopoly: {
            return "Duopoly";
        } break;
        case GameKind::Embraced: {
            return "Embraced";
        } break;
        case GameKind::DarkAges: {
            return "Dark Ages";
        } break;
    }
    return "Custom";
}

const char* game_kind_to_filename_string(GameKind game_kind) {
    switch (game_kind) {
        case GameKind::Competitive: {
            return "Competitive(1)";
        } break;
        case GameKind::Debug: {
            return "Debug(1)";
        } break;
        case GameKind::Scenario: {
            return "Scenario(1)";
        } break;
        case GameKind::FactionWars: {
            return "Faction Wars(1)";
        } break;
        case GameKind::Massacre: {
            return "Massacre(1)";
        } break;
        case GameKind::Omnipresence: {
            return "Omnipresence(1)";
        } break;
        case GameKind::TheTurning: {
            return "The Turning(1)";
        } break;
        case GameKind::Uprising: {
            return "Uprising(1)";
        } break;
        case GameKind::Duopoly: {
            return "Duopoly(1)";
        } break;
        case GameKind::Embraced: {
            return "Embraced(1)";
        } break;
        case GameKind::DarkAges: {
            return "Dark Ages(1)";
        } break;
    }
    assert(false);
    return "";
}

const char* race_to_string(RaceId::Enum race) {
    switch (race) {
        case RaceId::Terran: {
            return "Terran";
        } break;
        case RaceId::Protoss: {
            return "Protoss";
        } break;
        case RaceId::Zerg: {
            return "Zerg";
        } break;
    }
    return "Custom";
}

bool order_id_ignores_collision(u32 order_id) {
    return order_id == OrderId::Burrow
        || order_id == OrderId::Unburrow;
}

bool unit_should_be_autoattacked_by_player(Unit* unit, u32 player) {
    UnitExt& unit_ext = unit->ext();
    if (unit_ext.timers[TimerMindControlRecede]) {
        u32 original_player = unit_ext.values[ValueMindControlOriginalPlayer];
        if (player == original_player || bw::is_allied_to(player, original_player)) {
            return false;
        }
    }
    return true;
}

void cancel_all_training(Unit* unit) {
    u32 offset = 0;
    for (u32 i = 1; i < 5; ++i) {
        u32 slot = i + unit->build_queue_slot - offset;
        u32 unit_id = unit->build_queue[slot];
        if (unit_id == UnitId::None) {
            continue;
        }
        helper::refund_queue_slot(unit, slot);
        offset++;
    }
    helper::cancel_last_training(unit);
}

bool play_unit_banter(Unit* victim, u16 speaker_unit_id) {
    if (!victim) {
        return false;
    }
    auto speakers = g_unit_search.radial_ex(victim, 16 * 32, FilterEnemy, {speaker_unit_id}).sort_by_distance(victim->position);

    Unit* speaker = speakers.first([victim](Unit* speaker) {
        if (bw::distance_between(victim->position, speaker->position) > speaker->get_sight_range() * 32) {
            return false;
        }
        UnitExt& speaker_ext = speaker->ext();
        if (speaker_ext.timers[TimerUnitBanter]) {
            return false;
        }
        return true;
    });

    if (!speaker) {
        return false;
    }

    // Play banter
    switch (speaker_unit_id) {
        case UnitId::ProtossHierophant: {
            bw::play_sound({SoundId::HierophantBanterB01, SoundId::HierophantBanterB03}, speaker);
        } break;
        case UnitId::ProtossPatriarch: {
            bw::play_sound({SoundId::PatriarchBanter01, SoundId::PatriarchBanter04}, speaker);
        } break;
    }

    // Cooldown
    for (Unit* unit : speakers) {
        UnitExt* unit_ext = get_unit_ext(unit);
        unit_ext->timers[TimerUnitBanter] = 24 * 60;
    }

    return true;
}

void rescue_unit(Unit* unit, u32 player) {
    u32 idle_order = unit->status & UnitStatus::InBuilding ? idle_order = OrderId::BunkerGuard : idle_order = units_dat::human_idle_order[unit->id];
    helper::give_unit(unit, player, 1);
    unit->order_to(idle_order);

    // Audiovisuals
    if (!bw::in_replay_or_witness()) {
        unit->sprite->selection_timer = 45;
        helper::play_sound_from_unit(g_console_index + 32, unit, 1, 0);
    }
    bw::refresh_console();
}

bool can_auto_heal(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    return !unit->is_frozen() && !unit_ext->values[ValueDisablePassive];
}

bool unit_can_be_repulsed(Unit* unit) {
    if (unit->status & UnitStatus::GroundedBuilding) {
        return false;
    }
    if (unit->status & UnitStatus::Burrowed) {
        return false;
    }
    return true;
}

void specialist_energy_error(Unit* unit) {
    if (bw::in_replay_or_witness() || *g_active_player_id != *g_local_human_id) {
        return;
    }
    if (const char* string = helper::get_stat_txt_string(unit->get_race() + 864)) {
        helper::set_unit_stat_txt_error_message(string);
    }
}

bool unit_should_give_gas_in_massacre_mode(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    return unit_id_should_give_gas_in_massacre_mode(unit->id) && !unit_ext->values[ValueCancellingZergStructure];
}

bool game_kind_has_neutral_hostile(GameKind game_kind) {
    return game_kind == GameKind::TheTurning || game_kind == GameKind::Uprising;
}

bool player_detects_unit(u32 player, Unit* unit) {
    if (player >= PlayerLimit) {
        return false;
    }
    if (bw::in_replay_or_witness()) {
        return true;
    }
    if ((1 << player) & helper::get_detecting_players(unit)) {
        return true;
    }
    return false;
}

void fire_amitosis_weapon(Unit* attacker, WeaponId::Enum weapon_id) {
    fire_multiple(attacker, weapon_id, 2, FilterEnemy, {
        .fire_opts = {
            .apply_cooldown = false,
        },
        .ignore_main = true,
        .use_weapon_factor = false,
        .divisor_multiplier = 2,
        .stagger = -1,
        .center_on_target = true,
        .center_on_target_range_overwrite = Zerg::AmitosisSearchRange,
    });
}

void set_up_mines(Unit* unit) {
    switch (unit->id) {
        case UnitId::TerranVulture:        unit->mine_carrier.mine_count = 3; break;
        case UnitId::TerranClericFuturist: unit->mine_carrier.mine_count = 1; break;
    }
}

UnitId::Enum get_faction_replacement(u32 player, UnitId::Enum unit_id) {
    Faction faction = g_globals->players[player].faction;
    switch (faction) {
        case Faction::TerranHecatonCoalition: {
            switch (unit_id) {
                case UnitId::TerranMaverick: return UnitId::TerranHotshot;
                case UnitId::TerranApostle:  return UnitId::TerranAuriel;
                case UnitId::TerranTrojan:   return UnitId::TerranQilin;
            }
        } break;
        case Faction::TerranFuturistFederation: {
            switch (unit_id) {
                case UnitId::TerranCleric:  return UnitId::TerranClericFuturist;
                case UnitId::TerranCyclops: return UnitId::TerranCyclopsFuturist;
                case UnitId::TerranWraith:  return UnitId::TerranPhiloc;
            }
        } break;
        case Faction::TerranGoldenRepublic: {
            switch (unit_id) {
                case UnitId::TerranHarakan: return UnitId::TerranHarakanGolden;
                case UnitId::TerranGoliath: return UnitId::TerranAspirant;
                case UnitId::TerranSeraph:  return UnitId::TerranLuminary;
            }
        } break;
        case Faction::TerranNydusTrinity: {
            switch (unit_id) {
                case UnitId::TerranCyprian:         return UnitId::TerranCohort;
                case UnitId::TerranPhalanxTankMode: return UnitId::TerranTarasque;
            }
        } break;
        case Faction::TerranUpstart: {
            switch (unit_id) {
                case UnitId::TerranGoliath: return UnitId::TerranSouthpaw;
                case UnitId::TerranAncile:  return UnitId::TerranWendigo;
            }
        } break;
        case Faction::ProtossKhalendricKnights: {
            switch (unit_id) {
                case UnitId::ProtossEcclesiast: return UnitId::ProtossEcclesiastKhalendric;
            }
        } break;
        case Faction::ProtossOrderOfTheSacredSaber: {
            switch (unit_id) {
                case UnitId::ProtossLegionnaire: return UnitId::ProtossAdherent;
            //  case UnitId::ProtossVassal:      return UnitId::ProtossRenesair;
            //  case UnitId::ProtossAurora:      return UnitId::ProtossExtatus;
            }
        } break;
        case Faction::ProtossStarbound: {
            switch (unit_id) {
                case UnitId::ProtossZealot:      return UnitId::ProtossZealotStarbound;
                case UnitId::ProtossLegionnaire: return UnitId::ProtossCicatus;
            }
        } break;
        case Faction::ZergImicrenTendril: {
            switch (unit_id) {
            //  case UnitId::ZergQuazilisk:     return UnitId::ZergQuazrokor;
            //  case UnitId::ZergZoryusthaleth: return UnitId::ZergInzorthrokor;
            }
        } break;
        case Faction::ZergGoszirithTendril: {
            switch (unit_id) {
            //  case UnitId::ZergVorvrokor: return UnitId::ZergGorvrokor;
            }
        } break;
        case Faction::ZergZorthosTendril: {
            switch (unit_id) {
                case UnitId::ZergNathrokor: return UnitId::ZergNathrelisk;
                case UnitId::ZergIzirokor:  return UnitId::ZergIziralisk;
            }
        } break;
    }
    return unit_id;
}