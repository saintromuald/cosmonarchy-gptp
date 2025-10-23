#include "cm_utils.h"
#include "BW\api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"
#include "BW\stat_txt_spec.h"

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
        case GameKind::Deathmarch: {
            return "Deathmarch";
        } break;
        case GameKind::Brotherhood: {
            return "Brotherhood";
        } break;
        case GameKind::Zombocalypse: {
            return "Zombocalypse";
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
        case GameKind::Deathmarch: {
            return "Deathmarch(1)";
        } break;
        case GameKind::Brotherhood: {
            return "Brotherhood(1)";
        } break;
        case GameKind::Zombocalypse: {
            return "Zombocalypse(1)";
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
    if (bw::in_replay_or_witness() || *g_active_player_id != *g_local_human_id) {
        return;
    }
    if (const char* string = helper::get_stat_txt_string(unit->get_race() + 864)) {
        helper::set_unit_stat_txt_error_message(string);
    }
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
        case UnitId::TerranClericFuturist: unit->mine_carrier.mine_count = 1; break;
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
                case UnitId::TerranTrojan:                return UnitId::TerranQilin;
            }
        } break;
        case Faction::TerranFuturistFederation: {
            switch (unit_id) {
                case UnitId::TerranMaverick:  return UnitId::TerranMaverickFuturist;
                case UnitId::TerranCleric:    return UnitId::TerranClericFuturist;
                case UnitId::TerranMadcap:    return UnitId::TerranBlackguard;
                case UnitId::TerranCyclops:   return UnitId::TerranCyclopsFuturist;
                case UnitId::TerranBlackjack: return UnitId::TerranTiangou;
                case UnitId::TerranWraith:    return UnitId::TerranPhiloc;
            }
        } break;
        case Faction::TerranGoldenRepublic: {
            switch (unit_id) {
                case UnitId::TerranHarakan:   return UnitId::TerranHarakanGolden;
                case UnitId::TerranGoliath:   return UnitId::TerranAspirant;
                case UnitId::TerranBlackjack: return UnitId::TerranBlackjackGolden;
                case UnitId::TerranSeraph:    return UnitId::TerranLuminary;
            }
        } break;
        case Faction::TerranNydusTrinity: {
            switch (unit_id) {
                case UnitId::TerranCyprian:         return UnitId::TerranCohort;
                case UnitId::TerranEidolon:         return UnitId::TerranEidolonNydus;
                case UnitId::TerranPhalanxTankMode: return UnitId::TerranTarasque;
            }
        } break;
        case Faction::TerranUpstart: {
            switch (unit_id) {
                case UnitId::TerranCyprian: return UnitId::TerranCannibal;
                case UnitId::TerranGoliath: return UnitId::TerranSouthpaw;
                case UnitId::TerranAncile:  return UnitId::TerranWendigo;
            }
        } break;
        case Faction::ProtossKhalendricKnights: {
            switch (unit_id) {
                case UnitId::ProtossEcclesiast: return UnitId::ProtossEcclesiastKhalendric;
                case UnitId::ProtossHierophant: return UnitId::ProtossAshguard;
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
            }
        } break;
        case Faction::ZergImicrenTendril: {
            switch (unit_id) {
                case UnitId::ZergQuazilisk:     return UnitId::ZergQuazrokor;
                case UnitId::ZergZoryusthaleth: return UnitId::ZergInzorthrokor;
            }
        } break;
        case Faction::ZergGoszirithTendril: {
            switch (unit_id) {
            //  case UnitId::ZergZethrokor: return UnitId::ZergSovrokor
            //  case UnitId::ZergVorvrokor: return UnitId::ZergGarvrokor;
            }
        } break;
        case Faction::ZergOrkotiTendril: {
            switch (unit_id) {
                case UnitId::ZergHydralisk: return UnitId::ZergHydraliskOrkoti;
            }
        } break;
        case Faction::ZergZorthosTendril: {
            switch (unit_id) {
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
        case UnitId::ProtossCicatus:
        case UnitId::ProtossVagrant: {
            return 24;
        }
    }
    return 0;
}

UnitId::Enum get_factional_regression(Unit* unit) {
    Faction faction = unit->player < PlayerLimit ? g_globals->players[unit->player].faction : Faction::None;
    switch (unit->id) {
        case UnitId::ZergIroleth: {
            switch (faction) {
                case Faction::ZergZorthosTendril: return UnitId::ZergYblyleth;
            }
            return UnitId::ZergOvileth;
        } break;
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
                case Faction::ZergOrkotiTendril: return UnitId::ZergHydraliskOrkoti;
            }
            return UnitId::ZergHydralisk;
        } break;
        case UnitId::ZergTethzorokor: {
            switch (faction) {
                case Faction::ZergZorthosTendril: return UnitId::ZergIziralisk;
            }
            return UnitId::ZergIzirokor;
        } break;
        case UnitId::ZergUltrokor: {
            switch (faction) {
                case Faction::ZergImicrenTendril: return UnitId::ZergInzorthrokor;
            }
            return UnitId::ZergZoryusthaleth;
        } break;
    }
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

Faction get_random_faction(RaceId::Enum race) {
    switch (race) {
    case RaceId::Terran:  return static_cast<Faction>(bw::rand_between((u32)Faction::TerranOrionImperium, (u32)Faction::TerranUpstart));
    case RaceId::Protoss: return static_cast<Faction>(bw::rand_between((u32)Faction::ProtossDivinePrimacy, (u32)Faction::ProtossStarbound));
    case RaceId::Zerg:    return static_cast<Faction>(bw::rand_between((u32)Faction::ZergAtaltrethTendril, (u32)Faction::ZergZorthosTendril));
    }
    return Faction::None;
}

namespace WeaponId {
// TODO: YAML
const char* get_weapon_name(WeaponId::Enum weapon_id) {
    switch (weapon_id) {
        case WeaponId::MaverickGaussRifle:                  return "Gauss Rifle";
        case WeaponId::HeraclesJavelinCannons:              return "Javelin Cannons";
        case WeaponId::EidolonCoronerRifle:                 return "Coroner Rifle";
        case WeaponId::ApostleCharonRifle:                  return "Charon Rifle";
        case WeaponId::VultureFragmentationGrenade:         return "Fragmentation Grenade";
        case WeaponId::CuirassHaymakerAutogun:              return "Haymaker Autogun";
        case WeaponId::SpiderMines:                         return "Spider Mines";
        case WeaponId::GoliathTwinAutocannons:              return "Twin Autocannons";
        case WeaponId::GoliathContenderMissiles:            return "Contender Missiles";
        case WeaponId::MadcapDanannAutogun:                 return "Danaan Autogun";
        case WeaponId::PaladinCrusaderMissiles:             return "Crusader Missile Salvo";
        case WeaponId::PhalanxArcliteCannon:                return "Arclite Cannon";
        case WeaponId::TalosFusionBattery:                  return "Fusion Battery";
        case WeaponId::MasonFusionCutter:                   return "Fusion Cutter";
        case WeaponId::MasonFusionCutterHarvest:            return "Fusion Cutter";
        case WeaponId::BlackjackMonogun:                    return "Monogun";
        case WeaponId::WraithBurstLasers:                   return "Burst Lasers";
        case WeaponId::SalamanderPerditionBarrage:          return "Perdition Barrage";
        case WeaponId::GorgonArtificeBattery:               return "Artifice Battery";
        case WeaponId::MinotaurAsterionLaserBattery:        return "Asterion Laser Battery";
        case WeaponId::CohortGaussPistols:                  return "Gauss Pistols";
        case WeaponId::MadrigalFuriaMissiles:               return "Furia Missiles";
        case WeaponId::MadrigalImperiosoMissiles:           return "Imperioso Missiles";
        case WeaponId::CentaurArtaxCannon:                  return "Artax Cannon";
        case WeaponId::CentaurSunchaserMissiles:            return "Sunchaser Missile Salvo";
        case WeaponId::HarakanFlamethrower:                 return "Flamethrower";
        case WeaponId::CyprianTwinRailguns:                 return "Twin Railguns";
        case WeaponId::PhalanxArcliteShockCannon:           return "Arclite Shock Cannon";
        case WeaponId::ZoryusthalethMonstrousMaw:           return "Monstrous Maw";
        case WeaponId::WatchdogLongboltMissile:             return "Longbolt Missiles";
        case WeaponId::YamatoGun:                           return "Yamato Cannon";
        case WeaponId::NuclearMissile:                      return "Missile Strike";
        case WeaponId::GoskaLobotomyGrenade:                return "Lobotomy Grenade";
        case WeaponId::EMP_Shockwave:                       return "EMP Shockwave";
        case WeaponId::Irradiate:                           return "Irradiate";
        case WeaponId::ZethrokorClaws:                      return "Zethrok Claws";
        case WeaponId::VorvalClaws:                         return "Vorvrok Claws";
        case WeaponId::NathrokorVestigialStrike:            return "Vestigial Strike";
        case WeaponId::HydraliskNeedleSpines:               return "Needle Spines";
        case WeaponId::PenumbraErebolShockCannon:           return "Erebol Shock Cannon";
        case WeaponId::UltrokorKaiserBlades:                return "Kaiser Blades";
        case WeaponId::KeskathalorScreamingVenom:           return "Screaming Venom";
        case WeaponId::RilirokorToxicSpores:                return "Toxic Spores";
        case WeaponId::DrolethSpines:                       return "Spines";
        case WeaponId::DrolethSpinesHarvest:                return "Spines";
        case WeaponId::AlkajeliskPlasmidSpores:             return "Plasmid Spores";
        case WeaponId::GeszithalorAcidSpore:                return "Acid Spore";
        case WeaponId::GorgrokorSerratedCleavers:           return "Serrated Cleavers";
        case WeaponId::MutaliskGlaveWurm:                   return "Glave Wurm";
        case WeaponId::ProselyteDispatchPistols:            return "Dispatch Pistols";
        case WeaponId::BactaliskCorrosiveAcid:              return "Corrosive Acid";
        case WeaponId::PazuzuJupiterDriver:                 return "Jupiter Driver";
        case WeaponId::SpraithCesiantSeekerSpores:          return "Seeker Spores";
        case WeaponId::SurkithCesiantSubterraneanTentacle:  return "Subterranean Tentacle";
        case WeaponId::SkithrokorSkitteringSpores:          return "Skittering Spores";
        case WeaponId::SkryrokorPlasmaMetamorphosis:        return "Plasma Metamorphosis";
        case WeaponId::MatralethParasite:                   return "Parasitic Excision";
        case WeaponId::DyadTwintorch:                       return "Twintorch";
        case WeaponId::KeskathalorScreamingVenom2:          return "Screaming Venom";
        case WeaponId::CarapaceSwarm:                       return "Carapace Swarm";
        case WeaponId::Plague:                              return "Plague";
        case WeaponId::Consume:                             return "Consume";
        case WeaponId::ScribeParticleBeam:                  return "Particle Beam";
        case WeaponId::ScribeParticleBeamHarvest:           return "Particle Beam";
        case WeaponId::ZealotPhaseBlades:                   return "Phase Blades";
        case WeaponId::VassalPulseBlaster:                  return "Pulse Blaster";
        case WeaponId::DracadinPhaseDisruptor:              return "Phase Disruptor";
        case WeaponId::GladiusPlasmaFlurry:                 return "Plasma Flurry";
        case WeaponId::SimulacrumPhaseCannon:               return "Phase Cannon";
        case WeaponId::AspirantPotentiumCannon:             return "Potentium Cannon";
        case WeaponId::ArchonPsionicShockwave:              return "Psionic Shockwave";
        case WeaponId::EmpyreanPlasmaBarrage:               return "Plasma Barrage";
        case WeaponId::HierophantPhaseSignifier:            return "Phase Signifier";
        case WeaponId::AuroraPhotonBlasters:                return "Photon Blasters";
        case WeaponId::PanoptusPlasmaMissiles:              return "Plasma Missiles";
        case WeaponId::AugurAeonsMadness:                   return "Aeon's Madness";
        case WeaponId::ExemplarPlasmaTorpedo:               return "Plasma Torpedo";
        case WeaponId::MissionaryAruoTwinshot:              return "Aruo Twinshot";
        case WeaponId::MagisterNeutronCannon:               return "Neutron Cannon";
        case WeaponId::StewardPulseCannon:                  return "Pulse Cannons";
        case WeaponId::WardenPrismaticCannon:               return "Prismatic Cannon";
        case WeaponId::RililiskSwarmingSpores:              return "Swarming Spores";
        case WeaponId::CardinalInoaTwinaxe:                 return "Inoa Twinaxe";
        case WeaponId::StasisField:                         return "Stasis Field";
        case WeaponId::PsiStorm:                            return "Psionic Storm";
        case WeaponId::LiiraliskHollowedSpores:             return "Hollowed Glaves";
        case WeaponId::LegionnairePhaseSpear:               return "Phase Spear";
        case WeaponId::AkistrokorHarryingTalons:            return "Harrying Talons";
        case WeaponId::SavantDiscoidRailgun:                return "Discoid Railgun";
        case WeaponId::LobotomyMineExplosion:               return unit->id == UnitId::TerranVulture ? "Lobotomy Mines" : "Electromagnetic Explosion";
        case WeaponId::StarSovereignCelestialCannon:        return "Celestial Cannons";
        case WeaponId::StarSovereignInterventionMissiles:   return "Intervention Missiles";
        case WeaponId::SovrolethSpores:                     return "Sovrol Spores";
        case WeaponId::CyclopsFlakCannons:                  return "Flak Cannons";
        case WeaponId::PhobosArgosLaserBatteries:           return "Argos Laser Batteries";
        case WeaponId::KonviliskSowingSpines:               return "Sowing Spines";
        case WeaponId::SirenCharybdalRifle:                 return "Charybdal Rifle";
        case WeaponId::EcclesiastAstralBlaster:             return "Astral Blaster";
        case WeaponId::AtreusDauntlessDisruptorCannon:      return "Dauntless Disruption Cannon";
        case WeaponId::AmaranthPsiCleaver:                  return "Phase Cleaver";
        case WeaponId::LanifectNeutronFlare:                return "Neutron Flare";
        case WeaponId::CharlatanFerventBlaster:             return "Fervent Blaster";
        case WeaponId::QuaziliskAcridBarbs:                 return "Acrid Barbs";
        case WeaponId::ValkyrieHaloRockets:                 return "Halo Rockets";
        case WeaponId::VithriliskCorrosiveSpores:           return "Corrosive Spores";
        case WeaponId::MindTyrantMaledictus:                return "Maledictus";
        case WeaponId::DemiurgeGalacticWeaver:              return "Galactic Weaver";
        case WeaponId::CantavisHallucinationCast:           return "Hallucination";
        case WeaponId::MindTyrantMaelstrom:                 return "Maelstrom";
        case WeaponId::LakiziliskSubterraneanSpines:        return "Subterranean Spines";
        case WeaponId::AlmaksaliskPiercingSpines:           return "Piercing Spines";
        case WeaponId::CabalistWarpBlades:                  return "Warp Blades";
        case WeaponId::IdolPrismaticBlaster:                return "Prismatic Blaster";
        case WeaponId::GolemBloodlessAssault:               return "Phase Lances";
        case WeaponId::BarghestRayOfAgony:                  return "Ray of Agony";
        case WeaponId::PariahIconoclastBlades:              return "Iconoclast's Flurry";
        case WeaponId::AutocratEminenceRifle:               return "Eminence Rifle";
        case WeaponId::KalkaliskViscousAcid:                return "Viscous Acid";
        case WeaponId::ManifoldPhaseBlaster:                return "Phase Blaster";
        case WeaponId::ArchitectDispersiveSalvo:            return "Dispersive Salvo";
        case WeaponId::ClaymoreHarpoonRailgun:              return "Harpoon Railgun";
        case WeaponId::LuminaryAuralAssault:                return "Aural Assault";
        case WeaponId::VicarOspoTwinshard:                  return "Ospo Twinshard";
        case WeaponId::OlympianTitansFist:                  return "Titan's Fist";
        case WeaponId::OlympianMountainGun:                 return "Mountain Gun";
        case WeaponId::WyvernJacksonCannon:                 return "Jackson Cannon";
        case WeaponId::RoveElreScaleshot:                   return "Elre Scaleshot";
        case WeaponId::ObjectOrliMallet:                    return "Orli Mallet";
        case WeaponId::AugurFinalHour:                      return "Final Hour";
        case WeaponId::DiademHammerfallBeacon:              return "Hammerfall Beacon";
        case WeaponId::None:                                return "None";
        case WeaponId::LuminaryIcarusMissiles:              return "Icarus Missiles";
        case WeaponId::EpigraphLitanyCannons:               return "Litany Cannons";
        case WeaponId::StrigaPhaseSaber:                    return "Projection Saber";
        case WeaponId::AxitriliskSearingVenom:              return "Searing Venom";
        case WeaponId::TarasqueHedonCannon:                 return "Hedon Cannon";
        case WeaponId::TarasqueAcceleratedPlasmid:          return "Accelerated Plasmid";
        case WeaponId::AnticthonEncirclingMirror:           return "Encircling Mirror";
        case WeaponId::EmpressFadingBioplasma:              return "Fading Bioplasma";
        case WeaponId::AnthelionEventHorizon:               return "Event Horizon";
        case WeaponId::EmergencyDetonationExplosion:        return "Emergency Detonation";
        case WeaponId::MagnetarNewtonianShockCannon:        return "Newtonian Shock Cannon";
        case WeaponId::IzirokorEnergizedBioplasma:          return "Energized Bioplasma";
        case WeaponId::GosvilethGosvilSpines:               return "Gosvil Spines";
        case WeaponId::ArtisanNeutronSaber:                 return "Neutron Saber";
        case WeaponId::KagraliskRepeatingVenom:             return "Venemous Contagion";
        case WeaponId::EvigriliskBrutishTendrils:           return "Brutish Tendrils";
        case WeaponId::SentinelCerberusRailgun:             return "Cerberus Railgun";
        case WeaponId::VagrantVapidBlaster:                 return "Vapid Blaster";
        case WeaponId::MatrixKhaydarinRay:                  return "Khaydarin Ray";
        case WeaponId::SundogBlindsideRockets:              return "Blindside Rockets";
        case WeaponId::MatadorTwinstrikeFlamethrowers:      return "Twinstrike Flamethrowers";
        case WeaponId::ZarcavrokorBisectingBlades:          return "Bisecting Blades";
        case WeaponId::ZergBonesofReflection:               return "Bones of Reflection";
        case WeaponId::ScepterOrmaScrapray:                 return "Orma Scrapray";
        case WeaponId::IntendentErsaDriver:                 return "Ersa Driver";
        case WeaponId::HippogriffChimeraCannons:            return "Chimera Cannons";
        case WeaponId::IntendentAlruBurstray:               return "Alru Burstray";
        case WeaponId::HippogriffPegasusMissiles:           return "Pegasus Missiles";
        case WeaponId::AionLazarusSalvo:                    return "Lazarus Salvo";
        case WeaponId::HeraldStellarTorch:                  return "Stellar Torch";
        case WeaponId::CikraliskAcidSalvo:                  return "Acid Salvo";
        case WeaponId::TethzorokorSaberArray:               return "Saber Array";
        case WeaponId::ProtathalorPungentSpores:            return "Pungent Spores";
        case WeaponId::VilgorokorDissolvingSabers:          return "Dissolving Sabers";
        case WeaponId::TosgriliskExplosiveSpores:           return "Explosive Spores";
        case WeaponId::GhitorokorGnashingTendrils:          return "Gnashing Tendrils";
        case WeaponId::ZobrioliskRapidSpinesGround:         return "Rapid Spines";
        case WeaponId::WendigoFusionSaw:                    return "Fusion Saw";
        case WeaponId::SouthpawKnockoutDrivers:             return "Knockout Drivers";
        case WeaponId::RamessesPharoahCannons:              return "Pharoah Cannons";
        case WeaponId::DurendalAbaddonFlamethrowers:        return "Abaddon Flamethrowers";
        case WeaponId::DurendalPhilistineAutogun:           return "Philistine Autogun";
        case WeaponId::CataphractLonginiusCannons:          return "Longinius Cannons";
        case WeaponId::DilettanteUpstartRifle:              return "Upstart Rifle";
        case WeaponId::SevenstepGeometricRifle:             return "Geometric Rifle";
        case WeaponId::HypnagogueKaleidoscopeCannon:        return "Kaleidoscope Cannon";
        case WeaponId::TetcaerokorCleavingMaw:              return "Cleaving Maw";
        case WeaponId::SpirtithCesiantSpirtilSpores:        return "Spirtil Spores";
        case WeaponId::HelionUskoScrapray:                  return "Usko Scrapray";
        case WeaponId::SkortrithCesiantSkortraTendril:      return "Skortra Tentacle";
        case WeaponId::TiradeIktaDriver:                    return "Ikta Driver";
        case WeaponId::AvatarReformingCarbine:              return "Reforming Carbine";
        case WeaponId::HypnagogueThroughTheLookingGlass:    return "Through the Looking Glass";
        case WeaponId::SilvertongueWordsmithRailgun:        return "Wordsmith Railgun";
        case WeaponId::OptectonSunsplitBeam:                return "Sunsplit Beam";
        case WeaponId::AutocortParticleRay:                 return "Particle Ray";
        case WeaponId::PatriarchIronhandWeaver:             return "Ironhand Weaver";
        case WeaponId::MadrigalHecatonAngeltearMissiles:    return "Angeltear Missiles";
        case WeaponId::AgrilethMassiveMandible:             return "Massive Mandible";
        case WeaponId::TartarusMineApollyoidConflagration:  return "Apollyoid Conflagration";
        case WeaponId::AccantorPlasmaShell:                 return "Plasma Shell";
        case WeaponId::MortothrokorWickedIncisors:          return "Wicked Incisors";
        case WeaponId::ServitorRecycledBlaster:             return "Recycling Blaster";
        case WeaponId::PositronAntimatterMissile:           return "Antimatter Missile";
        case WeaponId::SycophantSparringSaber:              return "Sparring Saber";
        case WeaponId::SmoochieBoyKissingEmoji:             return "Kissing Emoji";
        case WeaponId::EngramKhaydarinPlasmaLaser:          return "Khaydarin Plasma Laser";
        case WeaponId::DiademIonCannon:                     return "Ion Cannon";
        case WeaponId::SavantDiscoidRailgunEntropy:         return "Discoid Railgun";
        case WeaponId::SevenstepGeometricRiflePiercing:     return "Geometric Rifle";
        case WeaponId::PietyWardProvidence:                 return unit->id == UnitId::TerranClericFuturist ? "Piety Wards" : "Providence";
        case WeaponId::AnalogueCylindricalBlaster:          return "Cylindrical Blaster";
        case WeaponId::PhilocHeirloomBatteries:             return "Heirloom Batteries";
        case WeaponId::NathrokorNathrokClaws:               return "Nathrok Claws";
        case WeaponId::GorgrokorCalcification:              return "Calcification";
        case WeaponId::PotentateTalismanicWeaver:           return "Talismanic Weaver";
        case WeaponId::Rematerialize:                       return "Rematerialize";
        case WeaponId::HotshotDispatchShotgun:              return "Dispatch Shotgun";
        case WeaponId::AurielQuarternaryRifle:              return "Quarternary Rifle";
        case WeaponId::AdherentPhaseBlade:                  return "Phase Blade";
        case WeaponId::NathreliskSignalingSpores:           return "Signaling Spores";
        case WeaponId::AlgoztaliskBottomlessSpinestacks:    return "Bottomless Spinestacks";
        case 213:                                           return "U...nimplemented";
        case WeaponId::OkrimniliskOmnispores:               return "Omnispores";
        case WeaponId::CherbrathalorVenemousBursts:         return "Contagion Bursts";
        case WeaponId::SalamanderAngeltearRockets:          return "Angeltear Rockets";
        case WeaponId::CyclopsBrontesPayload:               return "Brontes Payload";
        case WeaponId::CyclopsFuturistFlakCannons:          return "Flak Cannon";
        case WeaponId::VibranceSensoryOverload:             return "Sensory Overload";
        case WeaponId::HarakanGoldenFlamethrower:           return "Flamethrower";
        case WeaponId::IziraliskCorrosiveBioplasma:         return "Corrosive Bioplasma";
        case WeaponId::CicatusPhaseClaws:                   return "Phase Claws";
        case WeaponId::ZealotStarboundPhaseBlades:          return "Phase Blades";
        case WeaponId::EcclesiastKhalendricAvengingBlaster: return "Avenging Blaster";
        case WeaponId::OscilliantRotatingRay:               return "Rotating Ray";
        case WeaponId::RenesairRotatingBlaster:             return "Rotating Blaster";
        case WeaponId::ExtatusRepeatingBlaster:             return "Repeating Blaster";
        case WeaponId::UnarAutonomousRay:                   return "Autonomous Ray";
        case WeaponId::AshguardPhaseCremator:               return "Phase Cremator";
        case WeaponId::LakiziliskZorthosSuccessiveSpines:   return "Sucessive Spines";
        case WeaponId::ZobrioliskRapidSpinesAir:            return "Rapid Spines";
        case WeaponId::QuazrokorQuazrokSpray:               return "Quazrok Spray";
        case WeaponId::DracadinOrderPhaseDisruptor:         return "Phase Disruptor";
        case WeaponId::TiangouMoonboundSalvo:               return "Moonbound Salvo";
        case WeaponId::EidolonNydusRevenantRifle:           return "Revenant Rifle";
        case WeaponId::MaverickFuturistGaussSlugger:        return "Gauss Slugger";
        case WeaponId::HydraliskOrkotiNeedleSpores:         return "Needle Spores";
        case WeaponId::InzorthrokorInzorReflux:             return "Inzor Reflux";
        case WeaponId::PilgrimMiningSaber:                  return "Mining Saber";
        case WeaponId::BlackguardAsoniCarbine:              return "Asoni Carbine";
        case WeaponId::ArhaunPhlegethonCannon:              return "Plegethon Cannon";
        case WeaponId::LiiraliskHomingSpore:                return "Homing Spores";
        case WeaponId::CannibalZuluSidekicks:               return "Zulu Sidekicks";
        case 244:                                           return "U...nimplemented";
        case 245:                                           return "U...nimplemented";
        case 246:                                           return "U...nimplemented";
        case 247:                                           return "U...nimplemented";
        case WeaponId::AckmedMickenMelter:                  return "Micken Melter";
        case WeaponId::GoskaMedusaRifle:                    return "Medusa Rifle";
        case 250:                                           return "U...nimplemented";
        case WeaponId::StroluumMagnatesMaw:                 return "Magnate's Maw";
        case 252:                                           return "U...nimplemented";
        case WeaponId::KerriganC10CanisterRifle:            return "C-10 Canister Rifle";
        case WeaponId::KerriganClaws:                       return "Claws";
        case WeaponId::NoneNew:                             return "None";
    }
    return "U...nimplemented";
}

} // namespace WeaponId