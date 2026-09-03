#include "cm_utils.h"
#include "BW\api.h"
#include "DatExtend/datext.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"
#include "BW\stat_txt_spec.h"

Box32 box32_rect(v2u16 pos, u32 w, u32 h) {
    Box32 box = {};
    box.left   = pos.x - w / 2;
    box.right  = pos.x + w / 2;
    box.top    = pos.y - h / 2;
    box.bottom = pos.y + h / 2;
    return box;
}

Box32 box32_square(v2u16 pos, u32 w) {
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
        v2u16 position = {};
        switch (event_type) {
            // TODO: Different ping types, maybe sound queue... Must be tasteful! -Pr0nogo
            case SpectatorEvent_Combat: 
            case SpectatorEvent_Grief: {
                position = {unit->position.x, unit->position.y};
            } break;
            case SpectatorEvent_Tech: {
                position = {unit->position.x, unit->position.y};
            //	helper::play_sound_global(units_dat::sound_yes_first[unit->id]);
            } break;
            case SpectatorEvent_Nuke: {
                position = {unit->order_target.pt.x, unit->order_target.pt.y};
            } break;
        }
        if (ping) {
            helper::minimap_ping(position.x, position.y, 17);
        }
    }
}

u32 get_construction_graphic(Unit* unit) {
    auto& unit_ext = unit->ext();
    bool is_ocsal = unit_ext.values[ValueRegressionUnstable];
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
        if (game_mode_race == GameModeRace::Default) {
            game_mode_race = GameModeRace::All;
        }
    }
}

void init_duopoly() {
    for (u32 player = 0; player < PlayerLimit; player++) {
        if (g_player_table[player].type == PlayerType::Computer || g_player_table[player].type == PlayerType::ComputerSlot) {
            continue;
        }

        auto& game_mode_race = g_globals->players[player].game_mode_race;
        if (game_mode_race == GameModeRace::Default) {
            game_mode_race = GameModeRace::None;
            switch (g_player_table[player].race) {
                case RaceId::Terran: {
                    game_mode_race = GameModeRace::TerranProtoss;
                } break;
                case RaceId::Protoss: {
                    game_mode_race = GameModeRace::ProtossZerg;
                } break;
                case RaceId::Zerg: {
                    game_mode_race = GameModeRace::ZergTerran;
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

u32 get_bullet_damage(Bullet* bullet, WeaponId::Enum weapon_id) {
    BulletExt* bullet_ext = get_bullet_ext(bullet);
    if (bullet_ext->damage_override) {
        return bullet_ext->damage_override;
    }
    return weapons_dat::damage_amount[weapon_id] * 256;
}

const char* game_kind_to_string(GameKind game_kind) {
    switch (game_kind) {
        case GameKind::Unranked: {
            return "Unranked";
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
        case GameKind::Deathmarch: {
            return "Deathmarch";
        } break;
        case GameKind::Brotherhood: {
            return "Brotherhood";
        } break;
        case GameKind::Zombocalypse: {
            return "Zombocalypse";
        } break;
        case GameKind::Ranked: {
            return "Ranked";
        } break;
    }
    return "Custom";
}

const char* game_kind_to_filename_string(GameKind game_kind) {
    switch (game_kind) {
        case GameKind::Unranked:     return "Unranked(1)";
        case GameKind::Debug:        return "Debug(1)";
        case GameKind::Scenario:     return "Scenario(1)";
        case GameKind::FactionWars:  return "Faction Wars(1)";
        case GameKind::Massacre:     return "Massacre(1)";
        case GameKind::Omnipresence: return "Omnipresence(1)";
        case GameKind::TheTurning:   return "The Turning(1)";
        case GameKind::Uprising:     return "Uprising(1)";
        case GameKind::Duopoly:      return "Duopoly(1)";
        case GameKind::Embraced:     return "Embraced(1)";
        case GameKind::DarkAges:     return "Dark Ages(1)";
        case GameKind::Deathmarch:   return "Deathmarch(1)";
        case GameKind::Brotherhood:  return "Brotherhood(1)";
        case GameKind::Zombocalypse: return "Zombocalypse(1)";
        case GameKind::Ranked:       return "Ranked(1)";
    }
    assert(false);
    return "";
}

const char* race_to_string(RaceId::Enum race) {
    switch (race) {
        case RaceId::Terran:  return "Terran";
        case RaceId::Protoss: return "Protoss";
        case RaceId::Zerg:    return "Zerg";
        case RaceId::Askosi:  return "Askosi";
    }
    return "Custom";
}

const char* race_to_table_string(RaceId::Enum race) {
    switch (race) {
        case RaceId::Zerg:    return g_network_table.get_string(7);
        case RaceId::Terran:  return g_network_table.get_string(8);
        case RaceId::Protoss: return g_network_table.get_string(9);
        case RaceId::Askosi:  return g_network_table.get_string(169);
        case RaceId::Random:  return g_network_table.get_string(10);
    }
    return g_string_empty;
}

bool order_id_ignores_collision(u32 order_id) {
    return order_id == OrderId::Burrow
        || order_id == OrderId::Unburrow;
}

bool unit_should_be_autoattacked_by_player(Unit* unit, u32 player) {
    auto& unit_ext = unit->ext();
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
        case UnitId::ProtossAmaranth: {
            bw::play_sound({SoundId::AmaranthBanter01, SoundId::AmaranthBanter04}, speaker);
        } break;
        case UnitId::ProtossVagrant: {
            bw::play_sound({SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04}, victim);
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
        u32 sound_id = SoundId::Misc_TRescue_wav; // Default
        switch (g_console_index) {
            case RaceId::Zerg:    sound_id = SoundId::Misc_ZRescue_wav; break;
            case RaceId::Terran:  sound_id = SoundId::Misc_TRescue_wav; break;
            case RaceId::Protoss: sound_id = SoundId::Misc_PRescue_wav; break;
            case RaceId::Askosi:  sound_id = SoundId::AskosiRescue01;   break;
        }
        bw::play_sound(sound_id, unit->position);
    }
    bw::refresh_console();
}

bool can_auto_heal(Unit* unit) {
    return !unit->is_frozen() && !unit->ext().values[ValueDisablePassive];
}

bool unit_can_be_repulsed(Unit* unit) {
    if (unit->is_building()) {
        return false;
    }
    if (unit->is_burrowed()) {
        return false;
    }
    if (unit->id == UnitId::ProtossSteward) {
        return false;
    }
    return true;
}

void specialist_energy_error(Unit* unit) {
    if (bw::in_replay_or_witness() || g_active_player_id != g_local_human_id) {
        return;
    }
    helper::set_unit_stat_txt_error_message("Not enough energy.");
}

bool unit_should_give_gas_in_massacre_mode(Unit* unit) {
    return unit_id_should_give_gas_in_massacre_mode(unit->id) && !unit->ext().values[ValueCancellingZergStructure];
}

bool game_kind_has_neutral_hostile(GameKind game_kind) {
    return game_kind == GameKind::TheTurning
        || game_kind == GameKind::Uprising
        || game_kind == GameKind::Deathmarch
        || game_kind == GameKind::Zombocalypse;
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
        case UnitId::TerranVultureHecaton: unit->mine_carrier.mine_count = 3; break;
        case UnitId::TerranVultureUpstart: unit->mine_carrier.mine_count = 3; break;
        case UnitId::TerranClericFuturist: unit->mine_carrier.mine_count = 1; break;
        case UnitId::AskosiProselyte:      unit->mine_carrier.mine_count = 1; break;
        case UnitId::RaynorVulture:        unit->mine_carrier.mine_count = 3; break;
    }
}

UnitId::Enum get_factional_unit_id(u32 player, UnitId::Enum unit_id) {
    switch (g_globals->players[player].faction) {
        case Faction::TerranHecatonCoalition: {
            switch (unit_id) {
                case UnitId::TerranMaverick:              return UnitId::TerranHotshot;
                case UnitId::TerranApostle:               return UnitId::TerranAuriel;
                case UnitId::TerranVulture:               return UnitId::TerranVultureHecaton;
                case UnitId::TerranGoliath:               return UnitId::TerranArhaun;
                case UnitId::TerranMadrigalImperiosoMode: return UnitId::TerranMadrigalHecatonImperiosoMode;
                case UnitId::TerranCataphract:            return UnitId::TerranArquebus;
                case UnitId::TerranValkyrie:              return UnitId::TerranValkyrieHecaton;
                case UnitId::TerranTrojan:                return UnitId::TerranQilin;
                case UnitId::TerranSeraph:                return UnitId::TerranSerathiel;
            }
        } break;
        case Faction::TerranFuturistFederation: {
            switch (unit_id) {
                case UnitId::TerranMaverick:        return UnitId::TerranMaverickFuturist;
                case UnitId::TerranCleric:          return UnitId::TerranClericFuturist;
                case UnitId::TerranMadcap:          return UnitId::TerranBlackguard;
                case UnitId::TerranCyclops:         return UnitId::TerranCyclopsFuturist;
                case UnitId::TerranBlackjack:       return UnitId::TerranTiangou;
                case UnitId::TerranMatadorTankMode: return UnitId::TerranCutlass;
                case UnitId::TerranWraith:          return UnitId::TerranPhiloc;
            }
        } break;
        case Faction::TerranGoldenRepublic: {
            switch (unit_id) {
                case UnitId::TerranHarakan:   return UnitId::TerranHarakanGolden;
                case UnitId::TerranGoliath:   return UnitId::TerranAspirant;
                case UnitId::TerranBlackjack: return UnitId::TerranBlackjackGolden;
                case UnitId::TerranValkyrie:  return UnitId::TerranQuadriga;
                case UnitId::TerranSeraph:    return UnitId::TerranLuminary;
            }
        } break;
        case Faction::TerranNydusTrinity: {
            switch (unit_id) {
                case UnitId::TerranHarakan:         return UnitId::TerranCamazotz;
                case UnitId::TerranCyprian:         return UnitId::TerranCohort;
                case UnitId::TerranEidolon:         return UnitId::TerranEidolonNydus;
                case UnitId::TerranPhalanxTankMode: return UnitId::TerranTarasque;
            }
        } break;
        case Faction::TerranUpstart: {
            switch (unit_id) {
                case UnitId::TerranMaverick:        return UnitId::TerranMaverickUpstart;
                case UnitId::TerranCyprian:         return UnitId::TerranCannibal;
                case UnitId::TerranVulture:         return UnitId::TerranVultureUpstart;
                case UnitId::TerranGoliath:         return UnitId::TerranSouthpaw;
                case UnitId::TerranPhalanxTankMode: return UnitId::TerranBroadside;
                case UnitId::TerranAncile:          return UnitId::TerranWendigo;
            }
        } break;
        case Faction::ProtossKhalendricKnights: {
            switch (unit_id) {
                case UnitId::ProtossEcclesiast: return UnitId::ProtossEcclesiastKhalendric;
                case UnitId::ProtossHierophant: return UnitId::ProtossAshguard;
                case UnitId::ProtossSimulacrum: return UnitId::ProtossSimulacrumKhalendric;
                case UnitId::ProtossIdol:       return UnitId::ProtossIdolKhalendric;
            }
        } break;
        case Faction::ProtossOrderOfTheSacredSaber: {
            switch (unit_id) {
                case UnitId::ProtossDracadin:    return UnitId::ProtossDracadinOrder;
                case UnitId::ProtossLegionnaire: return UnitId::ProtossAdherent;
                case UnitId::ProtossVassal:      return UnitId::ProtossRenesair;
                case UnitId::ProtossAurora:      return UnitId::ProtossExtatus;
            }
        } break;
        case Faction::ProtossStarbound: {
            switch (unit_id) {
                case UnitId::ProtossZealot:      return UnitId::ProtossZealotStarbound;
                case UnitId::ProtossLegionnaire: return UnitId::ProtossCicatus;
                case UnitId::ProtossManifold:    return UnitId::ProtossManifoldStarbound;
                case UnitId::ProtossCabalist:    return UnitId::ProtossVestige;
            }
        } break;
        case Faction::ProtossAstralOmen: {
            switch (unit_id) {
            //  case UnitId::ProtossEcclesiast:  return UnitId::ProtossEcclesiastAstral;
            //  case UnitId::ProtossHerald:      return UnitId::ProtossWarlock;
                case UnitId::ProtossSimulacrum:  return UnitId::ProtossSimulacrumAstral;
            }
        } break;
        case Faction::ZergImicrenTendril: {
            switch (unit_id) {
                case UnitId::ZergQuazilisk:     return UnitId::ZergQuazrokor;
                case UnitId::ZergIzirokor:      return UnitId::ZergRozirokor;
                case UnitId::ZergZoryusthaleth: return UnitId::ZergInzorthrokor;
                case UnitId::ZergUltrokor:      return UnitId::ZergAlzurokor;
            }
        } break;
        case Faction::ZergGoszirithTendril: {
            switch (unit_id) {
                case UnitId::ZergZethrokor: return UnitId::ZergSovrokor;
                case UnitId::ZergVorvrokor: return UnitId::ZergGarvrokor;
                case UnitId::ZergHydralisk: return UnitId::ZergHydrokor;
            }
        } break;
        case Faction::ZergOrkotiTendril: {
            switch (unit_id) {
                case UnitId::ZergVorvrokor: return UnitId::ZergZethalor;
                case UnitId::ZergHydralisk: return UnitId::ZergHydraliskOrkoti;
                case UnitId::ZergGorgrokor: return UnitId::ZergAzanthrokor;
            }
        } break;
        case Faction::ZergZorthosTendril: {
            switch (unit_id) {
                case UnitId::ZergAgrileth:   return UnitId::ZergYblyleth;
                case UnitId::ZergNathrokor:  return UnitId::ZergNathrelisk;
                case UnitId::ZergIzirokor:   return UnitId::ZergIziralisk;
            //  case UnitId::ZergLakizilisk: return UnitId::ZergLakiziliskZorthos;
            }
        } break;
    }
    return unit_id;
}

u32 get_modified_damage(Unit* unit, WeaponId::Enum weapon_id) {
    const auto& unit_ext = unit->ext();
    u32 damage = weapons_dat::damage_amount[weapon_id];
    if (unit_ext.values[ValueTriggerDamageBaseSet]) {
        damage = unit_ext.values[ValueTriggerDamageBaseAmount] / 256;
    }
    if (unit_ext.unar_solitary_focus_stacks) {
        damage += unit_ext.unar_solitary_focus_stacks;
    }
    return damage;
}

const char* get_unit_name_ext(UnitId::Enum unit_id) {
    if (unit_id >= UnitsIdLimit) {
        return (char*)g_string_empty;
    } else if (units_dat::map_string_id[unit_id] != 0) {
        return (char*)g_map_string_tbl->get_string(units_dat::map_string_id[unit_id]);
    } else if (DatExt::g_unit_names_tbl) {
        return (char*)((StringTbl*)&DatExt::g_unit_names_tbl)->get_string(unit_id + 1); // must be a pointer to a pointer, rather than just a pointer
    } else {
        return g_stat_txt_tbl_new->get_string(unit_id + 1);
    }
}

u32 get_banter_cooldown(UnitId::Enum unit_id) {
    switch (unit_id) {
        case UnitId::ProtossHierophant:
        case UnitId::ProtossPatriarch: {
            return 24 * 60;
        }
        case UnitId::ProtossAmaranth: {
            return 24 * 15;
        }
        case UnitId::ProtossCicatus:
        case UnitId::ProtossVagrant: {
            return 24;
        }
    }
    return 0;
}

UnitId::Enum get_factional_regression(UnitId::Enum unit_id, Faction faction) {
    switch (unit_id) {
        case UnitId::ZergLiiralisk: {
            switch (faction) {
                case Faction::ZergImicrenTendril: return UnitId::ZergQuazrokor;
            }
            return UnitId::ZergQuazilisk;
        } break;
        case UnitId::ZergKalkalisk: {
            switch (faction) {
                case Faction::ZergZorthosTendril: return UnitId::ZergNathrelisk;
            }
            return UnitId::ZergNathrokor;
        } break;
        case UnitId::ZergBactalisk: {
            switch (faction) {
                case Faction::ZergGoszirithTendril: return UnitId::ZergHydrokor;
                case Faction::ZergOrkotiTendril:    return UnitId::ZergHydraliskOrkoti;
            }
            return UnitId::ZergHydralisk;
        } break;
        case UnitId::ZergTethzorokor: {
            switch (faction) {
                case Faction::ZergImicrenTendril: return UnitId::ZergRozirokor;
                case Faction::ZergZorthosTendril: return UnitId::ZergIziralisk;
            }
            return UnitId::ZergIzirokor;
        } break;
        case UnitId::ZergZarcavrokor: {
            switch (faction) {
                case Faction::ZergOrkotiTendril: return UnitId::ZergAzanthrokor;
            }
            return UnitId::ZergGorgrokor;
        } break;
    }
    return UnitId::None;
}

UnitId::Enum get_factional_regression(Unit* unit) {
    const auto faction = unit->player < PlayerLimit ? g_globals->players[unit->player].faction : Faction::None;
    return get_factional_regression(UnitId::Enum(unit->id), faction);
}

void init_corpse(Thingy* corpse, SpriteId::Enum id, u32 player) {
    if (id == SpriteId::Egg_Spawn) {
        corpse->sprite->player = player;
    } else if (SpriteId::is_corpse(SpriteId::Enum(id))) {
        corpse->sprite->player = player;

        Unit* unit = *g_active_iscript_unit;
        if (unit) {
            ThingyExt* thingy_ext = get_thingy_ext(corpse);
            thingy_ext->player = unit->player;
            thingy_ext->unit_id = unit->id;
            thingy_ext->sight_range_before_death = unit->get_sight_range() * 32;
            thingy_ext->avenge_or_ascend_player = unit_avenges_or_ascends(unit);
            thingy_ext->create_origin = unit->create_origin;
            if (unit_id_is_mine_layer((UnitId::Enum)unit->id)) {
                thingy_ext->mine_count = unit->mine_carrier.mine_count;
            }

            auto& unit_ext = unit->ext();
            if (unit_ext.values[ValueRegressionUnstable]) {
                thingy_ext->is_regression = true;
            }
            if (unit_ext.timers[TimerAutocratControlledOpposition]) {
                reclaim_corpse(corpse, unit_ext.values[ValueAutocratControlledOppositionPlayer]);
            }
            if (unit_ext.timers[TimerGoldRush]) {
                salvage_corpse(corpse, unit_ext.values[ValueGoldRushPlayer]);
            }
            if (unit_ext.values[ValueNoCorpse]) {
                consume_corpse(corpse);
            }
        }
    }
    corpse->sprite->elevation = 0;
    helper::set_thingy_visibility_flags(corpse);
}

bool game_kind_has_zombies(GameKind game_kind) {
    return game_kind == GameKind::TheTurning
        || game_kind == GameKind::Deathmarch
        || game_kind == GameKind::Zombocalypse;
}

bool game_kind_doubles_units(GameKind game_kind) {
    return game_kind == GameKind::Massacre
        || game_kind == GameKind::Deathmarch;
}

bool chat_message_is_allowed(const char* text) {
    return stricmp(text, "GG")   == 0
        || stricmp(text, "GLHF") == 0
        || stricmp(text, "PP") == 0
        || stricmp(text, "PAUSE") == 0
        || stricmp(text, "R") == 0
        || stricmp(text, "RDY") == 0
        || stricmp(text, "READY") == 0;
}

void randomize_faction(u32 player_id) {
    auto& cm_player = g_globals->players[player_id];
    if (cm_player.witness_mode != WitnessMode::WitnessMode_Disabled || cm_player.faction_was_set) {
        return;
    }
    auto faction = get_random_faction(static_cast<RaceId::Enum>(g_player_table[player_id].race));
    SelectFactionCommand command = {};
    command.type = GameCommand_Custom;
    command.subtype = NewGameCommand::SelectFaction;
    command.player = player_id;
    command.faction = faction;
    helper::queue_game_command((u8*)&command, sizeof(command));

    // Set Mineral Field faction_value for replacing requests in scripts
    if (g_player_table[player_id].type != PlayerType::Computer) {
        return;
    }
    u32 faction_value = 0;
    switch (faction) {
        case Faction::TerranHecatonCoalition:
        case Faction::ProtossOrderOfTheSacredSaber:
        case Faction::ZergImicrenTendril: {
            faction_value = 1;
        } break;
        case Faction::TerranFuturistFederation:
        case Faction::ProtossKhalendricKnights:
        case Faction::ZergGoszirithTendril: {
            faction_value = 2;
        } break;
        case Faction::TerranGoldenRepublic:
        case Faction::ProtossStarbound:
        case Faction::ZergOrkotiTendril: {
            faction_value = 3;
        } break;
        case Faction::TerranNydusTrinity:
        case Faction::ProtossAstralOmen:
        case Faction::ZergZorthosTendril: {
            faction_value = 4;
        } break;
        case Faction::TerranUpstart: {
            faction_value = 5;
        }
    }
    g_unit_deaths_table[UnitId::ResourceMineralField].player[player_id] = faction_value;
}

Faction get_random_faction(RaceId::Enum race) {
    switch (race) {
        case RaceId::Terran:  return bw::rand_between(Faction::TerranOrionImperium, Faction::TerranUpstart);
        case RaceId::Protoss: return bw::rand_between(Faction::ProtossDivinePrimacy, Faction::ProtossAstralOmen);
        case RaceId::Zerg:    return bw::rand_between(Faction::ZergAtaltrethTendril, Faction::ZergZorthosTendril);
    }
    return Faction::None;
}

SoundId::Enum get_racial_error_sfx(RaceId::Enum race) {
    switch (race) {
        case RaceId::Terran:  return SoundId::Misc_Buzz_wav_1;
        case RaceId::Protoss: return SoundId::Misc_PError_WAV;
        case RaceId::Zerg:    return SoundId::Zerg_Drone_ZDrErr00_WAV;
        case RaceId::Askosi:  return SoundId::Misc_Buzz_wav_1;
        default:              return SoundId::Misc_Buzz_wav_1;
    }
}

namespace WeaponId {
} // namespace WeaponId

namespace FlingyId {
const char* get_name(flingy_id::Enum flingy_id) {
    return "";
}
} // namespace FlingyId