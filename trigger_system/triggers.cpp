#include "triggers.h"
#include "systems/audio_system.h"
#include "trigger_recolor.h"
#include "trigger_objectives.h"
#include "trigger_spawn.h"
#include "trigger_vars.h"
#include "trigger_events.h"
#include "table_edit.h"
#include "ai_layouts.h"
#include "player_color.h"
#include "threshold.h"
#include "graphics/graphics.h"
#include "aise_signal.h"
#include <limits>

namespace trigger {

enum class TriggerFaction : u8 {
    None                  = u8(Faction::None),

    OrionImperium         = u8(Faction::TerranOrionImperium),
    HecatonCoalition      = u8(Faction::TerranHecatonCoalition),
    FuturistFederation    = u8(Faction::TerranFuturistFederation),
    GoldenRepublic        = u8(Faction::TerranGoldenRepublic),
    NydusTrinity          = u8(Faction::TerranNydusTrinity),
    UpstartTerran         = u8(Faction::TerranUpstart),

    DivinePrimacy         = u8(Faction::ProtossDivinePrimacy),
    OrderOfTheSacredSaber = u8(Faction::ProtossOrderOfTheSacredSaber),
    KhalendricKnights     = u8(Faction::ProtossKhalendricKnights),
    Starbound             = u8(Faction::ProtossStarbound),
    AstralOmen            = u8(Faction::ProtossAstralOmen),

    Ataltreth             = u8(Faction::ZergAtaltrethTendril),
    Imicren               = u8(Faction::ZergImicrenTendril),
    Goszirith             = u8(Faction::ZergGoszirithTendril),
    Orkoti                = u8(Faction::ZergOrkotiTendril),
    Zorthos               = u8(Faction::ZergZorthosTendril),

    Count                 = u8(Faction::Count),
};

namespace {

void apply_unit_stat(Designator designator, u32 count, TriggerStat trigger_stat, s32 value) {
    for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
        auto& unit_ext = unit->ext();
        switch (trigger_stat) {
            case TriggerStat::Armor:            { unit_ext.values[ValueTriggerArmorBaseSet] = true;          unit_ext.values[ValueTriggerArmorBaseAmount] = value; } break;
            case TriggerStat::WeaponRange:      { unit_ext.values[ValueTriggerWeaponRangeBaseSet] = true;    unit_ext.values[ValueTriggerWeaponRangeBaseAmount] = value; } break;
            case TriggerStat::ArmorPenetration: { unit_ext.values[ValueTriggerArmorPenBaseSet] = true;       unit_ext.values[ValueTriggerArmorPenBaseAmount] = value; } break;
            case TriggerStat::Damage:           { unit_ext.values[ValueTriggerDamageBaseSet] = true;         unit_ext.values[ValueTriggerDamageBaseAmount] = value * 256; } break;
            case TriggerStat::SplashInner:      { unit_ext.values[ValueTriggerWeaponSplashInnerSet] = true;  unit_ext.values[ValueTriggerWeaponSplashInnerAmount] = value; } break;
            case TriggerStat::SplashMedium:     { unit_ext.values[ValueTriggerWeaponSplashMediumSet] = true; unit_ext.values[ValueTriggerWeaponSplashMediumAmount] = value; } break;
            case TriggerStat::SplashOuter:      { unit_ext.values[ValueTriggerWeaponSplashOuterSet] = true;  unit_ext.values[ValueTriggerWeaponSplashOuterAmount] = value; } break;
            case TriggerStat::TimedLife:        { unit->remove_timer = value; } break;
            case TriggerStat::Wireframe:        { unit_ext.values[ValueHasWireframeOverride] = true;         unit_ext.values[ValueWireframeOverride] = value; } break;
            case TriggerStat::Buttonset:        { unit_ext.values[ValueHasButtonsetOverride] = true;         unit_ext.values[ValueButtonsetOverride] = value; } break;
            case TriggerStat::HealRange:        { unit_ext.values[ValueTriggerHealRangeSet] = true;          unit_ext.values[ValueTriggerHealRangeAmount] = value; } break;
            case TriggerStat::Portrait:         { unit_ext.values[ValuePortraitOverrideSet] = true;          unit_ext.values[ValuePortraitOverrideId] = value; } break;
        }
    }
}

void update_trigger_var(TriggerVarType type, std::string_view name, s32 value, bool create, char operation) {
    const auto var = create ? &g_trigger_vars.find_or_create(type, name) : g_trigger_vars.find(type, name);
    if (!var) {
        return;
    }
    switch (operation) {
        case '+': var->set(var->value() + value); break;
        case '-': var->set(var->value() - value); break;
        case '*': var->set(var->value() * value); break;
        case '/': var->set(var->value() / value); break;
        case '%': var->set(var->value() % value); break;
    }
}

} // namespace

Registry g_triggers = Registry()
    // -------------------------------------------------------------------------
    // Conditions
    // -------------------------------------------------------------------------
    .add_condition("combat", [](Area area, PlayerBitset source_players, UnitIdBitset source_unit_ids, PlayerBitset target_players, UnitIdBitset target_unit_ids, TimeQuota time_quota, PlayablePlayer scouting_player) {
        for (const TriggerEvent& event : combat_events) {
            if (!area.check(event.position, units_dat::base_property[event.target_unit_id] & UnitProperty::Flyer)) {
                continue;
            }
            if (event.source_player >= FullPlayerLimit || event.target_player >= FullPlayerLimit) {
                continue;
            }
            if ((!source_players[event.source_player] || !source_unit_ids[event.source_unit_id]
                    || !target_players[event.target_player] || !target_unit_ids[event.target_unit_id])
                && (!source_players[event.target_player] || !source_unit_ids[event.target_unit_id]
                    || !target_players[event.source_player] || !target_unit_ids[event.source_unit_id])) {
                continue;
            }
            if (!time_quota.check(g_elapsed_time_frames - event.timestamp)) {
                continue;
            }
            if (scouting_player && !(event.seen_by & 1 << *scouting_player)) {
                continue;
            }
            return true;
        }
        return false;
    })
    .add_condition("kills", [](Area area, PlayerBitset source_players, UnitIdBitset source_unit_ids, PlayerBitset target_players, UnitIdBitset target_unit_ids, Quota quota, TimeQuota time_quota, PlayablePlayer scouting_player) {
        u32 count = 0;
        for (const TriggerEvent& event : kill_events) {
            if (!area.check(event.position, units_dat::base_property[event.target_unit_id] & UnitProperty::Flyer)) {
                continue;
            }
            if (event.source_player >= FullPlayerLimit || !source_players[event.source_player]) {
                continue;
            }
            if (!source_unit_ids[event.source_unit_id]) {
                continue;
            }
            if (event.target_player >= FullPlayerLimit || !target_players[event.target_player]) {
                continue;
            }
            if (!target_unit_ids[event.target_unit_id]) {
                continue;
            }
            if (!time_quota.check(g_elapsed_time_frames - event.timestamp)) {
                continue;
            }
            if (scouting_player && !(event.seen_by & 1 << *scouting_player)) {
                continue;
            }
            count += 1;
        }
        return quota.check(count);
    })
    .add_condition("deaths", [](Area area, PlayerBitset players, UnitIdBitset unit_ids, Quota quota, TimeQuota time_quota, PlayablePlayer scouting_player) {
        u32 count = 0;
        for (TriggerEvent& event : kill_events) {
            if (!area.check(event.position, units_dat::base_property[event.target_unit_id] & UnitProperty::Flyer)) {
                continue;
            }
            if (event.target_player >= FullPlayerLimit || !players[event.target_player]) {
                continue;
            }
            if (!unit_ids[event.target_unit_id]) {
                continue;
            }
            if (!time_quota.check(g_elapsed_time_frames - event.timestamp)) {
                continue;
            }
            if (scouting_player && !(event.seen_by & 1 << *scouting_player)) {
                continue;
            }
            count += 1;
        }
        return quota.check(count);
    })
    .add_condition("unit_count", [](Designator designator, Quota quota, PlayablePlayer scouting_player, TimeQuota scouting_time_quota, bool completed) {
        u32 count = 0;
        const auto units = scouting_player ? designator.search_seen(*scouting_player) : designator.search();
        for (Unit* unit : units) {
            const auto unit_ext = get_unit_ext(unit);
            if (!scouting_player || !scouting_time_quota.value || scouting_time_quota.check(unit_ext->last_seen_timer[*scouting_player])) {
                if (completed && !unit->is_completed()) {
                    continue;
                }
                count++;
            }
        }
        return quota.check(count);
    })
    .add_condition("local_camera", [](Area area, Identifier min_visibility) {
        const auto center = v2u16{
            u16(g_move_to_x + 320),
            u16(g_move_to_y + 200),
        };

        if (center.x >= area.box.left && center.x <= area.box.right &&
            center.y >= area.box.top  && center.y <= area.box.bottom) {
            const auto tile = bw::get_active_tile_at_coords(center.x, center.y);
            if (sv_iequals(std::string_view{min_visibility}, "unexplored")) {
                return true;
            } else if (sv_iequals(std::string_view{min_visibility}, "explored")) {
                return !(tile->explored_flags & (1 << g_local_nation_id));
            } else {
                return !(tile->visibility_flags & (1 << g_local_nation_id));
            }
        }
        return false;
    })
    .add_condition("var", [](std::string_view name, VarQuota quota) {
        const auto var = quota.create ? &g_trigger_vars.find_or_create(TriggerVarType_Var, name) : g_trigger_vars.find(TriggerVarType_Var, name);
        return var && quota.check(var->value());
    })
    .add_condition("timer", [](std::string_view name, TimerQuota quota) {
        const auto var = quota.create ? &g_trigger_vars.find_or_create(TriggerVarType_Timer, name) : g_trigger_vars.find(TriggerVarType_Timer, name);
        return var && quota.check(var->value());
    })
    .add_condition("unit_color", [](Designator designator, Quota quota, s32 color_id) {
        u32 count = designator.search()
            .filter_by([color_id](Unit* unit) {
                return get_unit_ext(unit)->values[ValuePlayerColor] == color_id;
            })
            .count();
        return quota.check(count);
    })
    .add_condition("player_race", [](Context& ctx, PlayablePlayer player, EqualityQuota op, PlayableRaceId race) {
        if (!player) {
            ctx.error("Invalid player");
            return false;
        }
        switch (op.type) {
            case QuotaType_Equal:    return g_player_table[*player].race == to_race_id(race);
            case QuotaType_NotEqual: return g_player_table[*player].race != to_race_id(race);
        }
        return false;
    })
    .add_condition("player_type", [](Context& ctx, PlayablePlayer player, EqualityQuota op, CrigPlayerType type) {
        if (!player) {
            ctx.error("Invalid player");
            return false;
        }
        if (type == CrigPlayerType::None) {
            ctx.error("Invalid player type", type);
            return false;
        }
        const auto compare = [&](PlayerType::Enum t) {
            switch (op.type) {
                case QuotaType_Equal:    return g_player_table[*player].type == t;
                case QuotaType_NotEqual: return g_player_table[*player].type != t;
            }
            ctx.error("Invalid operator", op.type);
            return false;
        };
        const auto compare_witness = [&](WitnessMode mode) {
            switch (op.type) {
                case QuotaType_Equal:    return g_globals->players[*player].witness_mode == mode;
                case QuotaType_NotEqual: return g_globals->players[*player].witness_mode != mode;
            }
            ctx.error("Invalid operator", op.type);
            return false;
        };
        switch (type) {
            case CrigPlayerType::Human:            return compare(PlayerType::Human);
            case CrigPlayerType::HumanDefeated:    return compare(PlayerType::HumanDefeated);
            case CrigPlayerType::Computer:         return compare(PlayerType::Computer);
            case CrigPlayerType::ComputerDefeated: return compare(PlayerType::ComputerDefeated);
            case CrigPlayerType::Witness:          return compare_witness(WitnessMode_Pure);
            case CrigPlayerType::WitnessDefeated:  return compare_witness(WitnessMode_Defeated);
        }
        ctx.error("Invalid player type", type);
        return false;
    })
    .add_condition("is_multiplayer", [](bool value) {
        return g_multiplayer_mode == value;
    })
    // -------------------------------------------------------------------------
    // Local actions
    // -------------------------------------------------------------------------
    .add_local_action("play_music", [](std::string_view filename) {
        AudioSystem::instance().queue_music(std::string{filename});
    })
    .add_local_action("stop_music", []() {
        AudioSystem::instance().stop_music();
    })
    .add_local_action("pause_music", []() {
        AudioSystem::instance().pause_music();
    })
    .add_local_action("resume_music", []() {
        AudioSystem::instance().resume_music();
    })
    .add_local_action("music_fade_in", [](Seconds fade_in) {
        AudioSystem::instance().set_music_fade_in(f32(fade_in.value));
    })
    .add_local_action("music_fade_out", [](Seconds fade_out) {
        AudioSystem::instance().set_music_fade_out(f32(fade_out.value));
    })
    .add_local_action("music_volume", [](f32 volume, Seconds time) {
        AudioSystem::instance().set_music_volume(volume, f32(time.value));
    })
    .add_local_action("playlist_clear", []() {
        AudioSystem::instance().clear_playlist();
    })
    .add_local_action("playlist_add", [](std::string_view filename) {
        if (!AudioSystem::instance().add_to_playlist(std::string{filename})) {
            bw::printf("Error: Playlist already full! (%s)", std::string{filename}.c_str());
        }
    })
    .add_local_action("playlist_shuffle", [](s32 do_shuffle) {
        AudioSystem::instance().set_playlist_shuffle(do_shuffle != 0);
    })
    .add_local_action("set_player_name", [](Context& ctx, PlayablePlayer player, std::string_view name, s32 ignore_human_name) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        if (!ignore_human_name && (g_player_table[*player].type == PlayerType::Human || g_player_table[*player].type == PlayerType::HumanDefeated)) {
            const auto player_name = std::format("{} ({})", name, g_player_table[*player].name);
            set_player_name(*player, player_name.c_str(), true);
        } else {
            set_player_name(*player, std::string{name}.c_str(), true);
        }
    })
    .add_local_action("set_force_name", [](Context& ctx, PlayablePlayer player, std::string name) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        set_player_force_name(*player, name.c_str());
    })
    .add_local_action("recolor", [](RecolorSet set, Seconds time) {
        TimedRecolor recolor {
            .set  = set,
            .time = f32(time.value),
        };
        recolor_system.recolor_time = 0;
        recolor_system.source_recolor = recolor_system.current_recolor;
        recolor_system.target_recolor = recolor;
        if (!recolor.time) {
            recolor_system.current_recolor = recolor;
            set_palette_recolor(recolor.set);
        }
    })
    .add_local_action("recolor_loop_add", [](RecolorSet set, Seconds time) {
        TimedRecolor recolor = TimedRecolor{
            .set  = set,
            .time = f32(time.value),
        };
        if (recolor_system.recolor_list.count < MaxRecolors) {
            recolor_system.recolor_list.recolors[recolor_system.recolor_list.count++] = recolor;
        }
    })
    .add_local_action("recolor_loop_clear", []() {
        memset(&recolor_system.recolor_list, 0, sizeof(recolor_system.recolor_list));
    })
    .add_local_action("objective_set", [](s32 key, std::string_view text) {
        auto& objective = g_objectives.get(key);
        objective.text = std::string{text};
        objective.max_score = 1;
        objective.show = true;
    })
    .add_local_action("objective_text", [](s32 key, std::string_view text) {
        g_objectives.get(key).text = std::string{text};
    })
    .add_local_action("objective_desc", [](s32 key, std::string_view text) {
        // TODO: set objective description
    })
    .add_local_action("objective_show", [](s32 key, s32 show) {
        g_objectives.get(key).show = show;
    })
    .add_local_action("objective_score", [](s32 key, s32 score, s32 max_score) {
        auto& objective = g_objectives.get(key);
        objective.score = std::clamp(score, 0, max_score);
        objective.max_score = max_score;
    })
    .add_local_action("objective_score_add", [](s32 key, s32 score, s32 max_score) {
        auto& objective = g_objectives.get(key);
        objective.max_score = objective.max_score + max_score;
        objective.score = std::clamp(objective.score + score, 0, (s32)objective.max_score);
    })
    .add_local_action("objective_score_subtract", [](s32 key, s32 score, s32 max_score) {
        auto& objective = g_objectives.get(key);
        objective.max_score = objective.max_score - max_score;
        objective.score = std::clamp(objective.score - score, 0, (s32)objective.max_score);
    })
    .add_local_action("objective_timer", [](s32 key, Ticks time) {
        g_objectives.get(key).timer = time;
    })
    .add_local_action("objective_remove", [](s32 key) {
        g_objectives.remove(key);
    })
    .add_local_action("objectives_sync", []() {
        // TODO: sync objectives to mission objectives menu
    })
    .add_local_action("objectives_clear", []() {
        g_objectives.clear();
    })
    .add_local_action("show_player_layouts", [](PlayerBitset players, s32 value) {
        for (u32 i = 0; i < PlayerLimit; ++i) {
            if (!players[i]) {
                continue;
            }
            g_show_player_layouts[i] = value;
        }
    })
    .add_local_action("set_player_unit_name", [](Context& ctx, PlayablePlayer player, UnitId::Enum unit_id, std::string_view name) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        if (unit_id == UnitId::None) {
            ctx.error("Invalid unit id");
            return;
        }
        g_globals->players[*player].custom_unit_names[unit_id] = std::string{name};
    })
    .add_local_action("set_player_race_name", [](Context& ctx, PlayablePlayer player, std::string_view old_race, std::string_view new_race, UnitIdBitset unit_ids) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        auto& cm_player = g_globals->players[*player];
        for (u32 i = 0; i < UnitId::Count; ++i) {
            if (!unit_ids[i]) {
                continue;
            }
            const auto unit_id = (UnitId::Enum)i;
            if (!cm_player.custom_unit_names.contains(unit_id)) {
                cm_player.custom_unit_names[unit_id] = Unit::get_name(unit_id);
            }
            auto& name = cm_player.custom_unit_names[unit_id];
            const auto name_sv = std::string_view{name};
            if (name_sv.starts_with(old_race)) {
                std::stringstream ss;
                ss << new_race << std::string_view{name.begin() + old_race.size(), name.end()};
                name = ss.str();
            }
        }
    })
    .add_local_action("reset_player_unit_names", [](Context& ctx, PlayablePlayer player) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_globals->players[*player].custom_unit_names.clear();
    })
    .add_local_action("unit_sound", [](PlayerBitset players, UnitIdBitset unit_ids, Any<IscriptAnimation::Enum> animation, Any<SoundId::Enum> old_sound_id, std::string_view path, bool replace, OneOf<"clear", "add"> operation) {
        const auto replacement = SoundReplacement{
            .players = players,
            .unit_ids = unit_ids,
            .any_animation = animation.any,
            .any_sound = old_sound_id.any,
            .replace = replace,
            .animation = u32(animation.value),
            .old_sound_id = u32(old_sound_id.value),
            .new_sound = AudioSystem::instance().get_sound(std::string{path}.c_str()),
        };
        if (operation == "clear") {
            std::erase_if(g_globals->sound_replacements, [&](auto& it) { return it == replacement; });
        } else {
            g_globals->sound_replacements.push_back(replacement);
        }
    })
    .add_local_action("play_sound", [](Context& ctx, std::string_view path, PlayerBitset players, v2u16 position, OneOf<"unexplored", "explored", "visible"> min_visibility) {
        const auto trigger_player = ctx.player();
        if (trigger_player < PlayerLimit && players[trigger_player]) {
            if (position.x == 0 && position.y == 0) {
                AudioSystem::instance().play_sound(AudioSystem::instance().get_sound(std::string{path}.c_str()), {.bw_volume = AudioSystem::instance().get_sound_volume()});
            } else {
                auto play = false;
                const auto tile = bw::get_active_tile_at_coords(position.x, position.y);
                if (min_visibility == "unexplored") {
                    play = true;
                } else if (min_visibility == "explored") {
                    play = !(tile->explored_flags & (1 << g_local_nation_id));
                } else {
                    play = !(tile->visibility_flags & (1 << g_local_nation_id));
                }
                if (play) {
                    AudioSystem::instance().play_sound(AudioSystem::instance().get_sound(std::string{path}.c_str()), {.position = position});
                }
            }
        }
    })
    .add_local_action("forbid_replay_auto_upload", []() {
        g_globals->forbid_replay_auto_upload = true;
    })
    .add_local_action("set_transmission_waits", [](bool state) {
        g_globals->disable_transmission_waits = state;
    })
    // -------------------------------------------------------------------------
    // Actions
    // -------------------------------------------------------------------------
    .add_action("wait", [](Context& ctx, Ticks time) {
        auto& ongoing_wait = ctx.ongoing_wait();
        const auto init = ctx.action()->flags & 1;
        if (!init) {
            ongoing_wait.timer = time;
            ongoing_wait.parser_position = ctx.command_start_pos();
            ctx.action()->flags |= 1;
            return false;
        } else if (ongoing_wait.timer > 0) {
            return false;
        }
        ongoing_wait.parser_position = 0;
        ctx.action()->flags &= ~1;
        return true;
    })
    .add_action("wait_if", [](Context& ctx) {
        if (ctx.evaluate_condition()) {
            ctx.ongoing_wait().parser_position = ctx.command_start_pos();
            return false;
        }
        ctx.ongoing_wait().parser_position = 0;
        return true;
    })
    .add_action("wait_until", [](Context& ctx) {
        if (!ctx.evaluate_condition()) {
            ctx.ongoing_wait().parser_position = ctx.command_start_pos();
            return false;
        }
        ctx.ongoing_wait().parser_position = 0;
        return true;
    })
    .add_action("exit_if", [](Context& ctx) {
        if (ctx.evaluate_condition()) {
            ctx.trigger().current_action_index = 64;
            return false;
        }
        return true;
    })
    .add_action("exit_unless", [](Context& ctx) {
        if (!ctx.evaluate_condition()) {
            ctx.trigger().current_action_index = 64;
            return false;
        }
        return true;
    })
    .add_action("exit_preserve_if", [](Context& ctx) {
        if (ctx.evaluate_condition()) {
            ctx.trigger().flags |= 0x4;
            ctx.trigger().current_action_index = 64;
            return false;
        }
        return true;
    })
    .add_action("exit_preserve_unless", [](Context& ctx) {
        if (!ctx.evaluate_condition()) {
            ctx.trigger().flags |= 0x4;
            ctx.trigger().current_action_index = 64;
            return false;
        }
        return true;
    })
    .add_action("show_button", [](std::string_view id, std::string_view display_name, s32 priority) {
        const auto trigger_button = TriggerButton{
            .id = std::string{id},
            .display_name = std::string{!display_name.empty() ? display_name : id},
            .priority = priority,
        };
        auto insert_it = std::ranges::upper_bound(g_globals->trigger_buttons, trigger_button, [](const auto& a, const auto& b) {
            return a.priority > b.priority;
        });
        g_globals->trigger_buttons.insert(insert_it, trigger_button);
        g_trigger_vars.find_or_create(TriggerVarType_Var, id).set(0);
    })
    .add_action("hide_button", [](std::string_view id) {
        std::erase_if(g_globals->trigger_buttons, [&](auto& button) { return button.id == id; });
    })
    .add_action("set_mineral_yields", [](u32 depleted_yield, u32 yield) {
        g_globals->yields[(size_t)YieldType::MineralDepleted] = depleted_yield;
        g_globals->yields[(size_t)YieldType::Mineral]         = yield;
    })
    .add_action("set_gas_yields", [](u32 ridge_yield, u32 geyser_yield, u32 tier2_yield, u32 tier3_yield) {
        g_globals->yields[(size_t)YieldType::GasRidge]       = ridge_yield;
        g_globals->yields[(size_t)YieldType::GasGeyser]      = geyser_yield;
        g_globals->yields[(size_t)YieldType::GasGeyserTier2] = tier2_yield;
        g_globals->yields[(size_t)YieldType::GasGeyserTier3] = tier3_yield;
    })
    .add_action("unit_sprite", [](PlayerBitset players, UnitIdBitset unit_ids, SpriteSelector sprite_id) {
        if (!sprite_id.valid) {
            return;
        }
        g_globals->sprite_replacements.push_back(SpriteReplacement{
            .players = players,
            .unit_ids = unit_ids,
            .sprite_id = u32(sprite_id.value),
        });
    })
    .add_action("clear_unit_sprite", [](PlayerBitset players, UnitIdBitset unit_ids, SpriteSelector sprite_id) {
        if (!sprite_id.valid) {
            return;
        }
        std::erase_if(g_globals->sprite_replacements, [&](const auto& replacement) {
            return replacement.players == players && replacement.unit_ids == unit_ids && replacement.sprite_id == u32(sprite_id.value);
        });
    })
    .add_action("clear_creep", [](Area location) {
        bw::clear_creep(location.box);
    })
    .add_action("spawn", [](Context& ctx, s32 count, UnitId::Enum unit_id, PlayerOrNeutral player, SpawnType type, Ticks delay, Ticks stagger, v2u16 source_pos, v2u16 move_pos) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        const auto sequence = SpawnSequence{
            .count = u32(count),
            .unit_id = u32(unit_id),
            .player = *player,
            .type = u32(type),
            .delay = u32(delay),
            .stagger = u32(stagger),
            .source_pos = source_pos,
            .move_pos = move_pos,
        };
        if (spawn_sequence_count < SpawnSequenceCap) {
            spawn_sequences[spawn_sequence_count++] = sequence;
        }
    })
    .add_action("layouts_enable", [](Area area, PlayerBitset players, UnitIdBitset unit_ids, s32 enable) {
        for (u32 player = 0; player < PlayerLimit; ++player) {
            if (!players[player]) {
                continue;
            }
            auto& layouts = g_ai_layouts.get_player_layouts(player);
            for (AiPlaceholder& placeholder : layouts.placeholders) {
                if (placeholder.unit_id == UnitId::TownCenterMarker) {
                    continue;
                }
                if (placeholder.unit_id == UnitId::None || !unit_ids[placeholder.unit_id]) {
                    continue;
                }
                if (!area.check(placeholder.position, units_dat::base_property[placeholder.unit_id] & UnitProperty::Flyer)) {
                    continue;
                }
                placeholder.disabled = !enable;
            }
        }
    })
    .add_action("layouts_priority_add", [](Area area, PlayerBitset players, UnitIdBitset unit_ids, s32 priority) {
        if (priority) {
            for (u32 player = 0; player < PlayerLimit; ++player) {
                if (!players[player]) {
                    continue;
                }
                auto& layouts = g_ai_layouts.get_player_layouts(player);
                for (AiPlaceholder& placeholder : layouts.placeholders) {
                    if (placeholder.unit_id == UnitId::TownCenterMarker) {
                        continue;
                    }
                    if (!unit_ids[placeholder.unit_id]) {
                        continue;
                    }
                    if (!area.check(placeholder.position, units_dat::base_property[placeholder.unit_id] & UnitProperty::Flyer)) {
                        continue;
                    }
                    placeholder.priority = CLAMP(placeholder.priority + priority, 0, 255);
                    layouts.should_sort = true;
                }
            }
        }
    })
    .add_action("layouts_priority_set", [](Area area, PlayerBitset players, UnitIdBitset unit_ids, s32 priority) {
        u8 new_priority = CLAMP(priority, 0, 255);
        for (u32 player = 0; player < PlayerLimit; ++player) {
            if (!players[player]) {
                continue;
            }
            auto& layouts = g_ai_layouts.get_player_layouts(player);
            for (AiPlaceholder& placeholder : layouts.placeholders) {
                if (placeholder.unit_id == UnitId::TownCenterMarker) {
                    continue;
                }
                if (placeholder.unit_id >= UnitId::Count || !unit_ids[placeholder.unit_id]) {
                    continue;
                }
                if (!area.check(placeholder.position, units_dat::base_property[placeholder.unit_id] & UnitProperty::Flyer)) {
                    continue;
                }
                if (placeholder.priority != new_priority) {
                    placeholder.priority = new_priority;
                    layouts.should_sort = true;
                }
            }
        }
    })
    .add_action("layouts_assign", [](Area area, PlayerBitset players, UnitIdBitset unit_ids, std::optional<v2u16> resarea_position) {
        const auto resarea = bw::get_closest_resarea(resarea_position.value_or(area.box.center()));
        for (u32 player = 0; player < PlayerLimit; ++player) {
            if (!players[player]) {
                continue;
            }
            auto& layouts = g_ai_layouts.get_player_layouts(player);
            for (AiPlaceholder& placeholder : layouts.placeholders) {
                if (placeholder.unit_id == UnitId::TownCenterMarker) {
                    continue;
                }
                if (placeholder.unit_id >= UnitId::Count || !unit_ids[placeholder.unit_id]) {
                    continue;
                }
                if (!area.check(placeholder.position, units_dat::base_property[placeholder.unit_id] & UnitProperty::Flyer)) {
                    continue;
                }
                if (placeholder.resarea != resarea) {
                    placeholder.resarea = resarea;
                    layouts.should_sort = true;
                }
            }
        }
    })
    .add_action("create_town", [](Context& ctx, PlayablePlayer player, v2u16 pos, s32 worker_count, s32 attach_to_nearest_resarea, Area claim_placeholders_area, UnitIdBitset unit_ids) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        const auto town = helper::create_ai_town(*player, pos.x, pos.y);
        if (!town) {
            return;
        }
        town->inited = true;
        auto& layouts_town = g_ai_layouts.get_town_layouts(*town);
        layouts_town.worker_limit   = worker_count;
        layouts_town.worker_maynard = worker_count;
        if (worker_count) {
            helper::ai_get_old_peons(town, worker_count);
        }
        if (attach_to_nearest_resarea) {
            town->resarea = bw::get_closest_resarea(pos);
            layouts_town.wants_no_resarea = false;
        } else {
            town->resarea = 0;
            layouts_town.wants_no_resarea = true;
        }
        town->resource_units_not_set = town->resarea != 0;
        if (claim_placeholders_area.box.w && claim_placeholders_area.box.h) {
            auto& layouts = g_ai_layouts.get_player_layouts(*player);
            s32 idx = -1;
            for (AiPlaceholder& placeholder : layouts.placeholders) {
                idx++;
                if (placeholder.unit_id >= UnitId::Count || !unit_ids[placeholder.unit_id]) {
                    continue;
                }
                if (!claim_placeholders_area.check(placeholder.position, units_dat::base_property[placeholder.unit_id] & UnitProperty::Flyer)) {
                    continue;
                }
                if (!placeholder.town) {
                    placeholder.town = town;
                    if (g_show_player_layouts[*player]) {
                        bw::printf("Player %u claimed %s #%u from resarea %u", *player + 1, Unit::get_name(placeholder.unit_id), (u32)idx, placeholder.resarea);
                    }
                }
            }
        }
    })
    .add_action("set_start_location", [](Context& ctx, PlayablePlayer player, v2u16 position) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_start_positions[*player] = position;
    })
    .add_action("set_race", [](Context& ctx, PlayablePlayer player, PlayableRaceId race) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_player_table[*player].race = to_race_id(race);
        if (*player == g_local_nation_id) {
            g_globals->current_advisor = get_advisor_type_from_config(u32(to_race_id(race)));
        }
    })
    .add_action("set_player_color", [](Context& ctx, PlayablePlayer player, PlayerColor player_color) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        set_player_color(*player, player_color);
    })
    .add_action("set_ai_auto_burrow", [](Designator designator, s32 enable) {
        for (Unit* unit : designator.search()) {
            unit->ext().values[ValueDisableAutoBurrow] = !enable;
        }
    })
    .add_action("kill", [](Designator designator, u32 count, bool remove) {
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            if (remove) {
                unit->silent_remove();
            } else {
                unit->kill();
            }
        }
    })
    .add_action("set_unit_stat", [](Context& ctx, Designator designator, u32 count) {
        ctx.subcommands(
            Subcommand{"armor", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::Armor, value); }},
            Subcommand{"weapon_range", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::WeaponRange, value); }},
            Subcommand{"armor_pen", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::ArmorPenetration, value); }},
            Subcommand{"armor_penetration", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::ArmorPenetration, value); }},
            Subcommand{"damage", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::Damage, value); }},
            Subcommand{"splash_inner", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::SplashInner, value); }},
            Subcommand{"splash_medium", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::SplashMedium, value); }},
            Subcommand{"splash_outer", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::SplashOuter, value); }},
            Subcommand{"timed_life", [=](Ticks value) { apply_unit_stat(designator, count, TriggerStat::TimedLife, s32(value)); }},
            Subcommand{"wireframe", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::Wireframe, value); }},
            Subcommand{"buttonset", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::Buttonset, value); }},
            Subcommand{"heal_range", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::HealRange, value); }},
            Subcommand{"portrait", [=](s32 value) { apply_unit_stat(designator, count, TriggerStat::Portrait, value); }}
        );
    })
    .add_action("set_unit_behavior", [](Designator designator, s32 count, TriggerBehavior trigger_behavior, s32 enable) {
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            auto& unit_ext = unit->ext();
            switch (trigger_behavior) {
                case TriggerBehavior::CloakProximity: {
                    unit_ext.values[ValueTriggerBehaviorProximityCloak] = enable;
                    if (enable) {
                        unit_ext.timers[TimerDecloak] = 2secs + 1;
                    } else if (!unit_ext.timers[TimerFacadePryingEyesCloak]) {
                        bw::decloak_proximity_proc(unit);
                    }
                } break;
                case TriggerBehavior::CloakSteady: {
                    unit_ext.values[ValueTriggerBehaviorSteadyShroud] = enable;
                    if (!enable) {
                        bw::decloak_passive_proc(unit);
                    } else {
                        unit_ext.timers[TimerDecloak] = 2secs + 1;
                    }
                } break;
                case TriggerBehavior::Hallucination: {
                    if (enable && !(unit->status & UnitStatus::IsHallucination)) {
                        unit->status |= UnitStatus::IsHallucination;
                    } else if (!enable && unit->status & UnitStatus::IsHallucination) {
                        unit->status &= ~UnitStatus::IsHallucination;
                    }
                } break;
                case TriggerBehavior::DisablePassive:       { unit_ext.values[ValueDisablePassive] = enable != false; } break;
                case TriggerBehavior::Critter:              { if (unit_ext.values[ValueCritter] != enable) { unit_ext.values[ValueCritter] = enable; unit->order_to_idle(); } } break;
                case TriggerBehavior::NoCollision:          { if (unit_ext.values[ValueNoCollision] != enable) unit_ext.values[ValueNoCollision] = enable; } break;
                case TriggerBehavior::AirTransport:         { if (unit_ext.values[ValueAirTransport] != enable) unit_ext.values[ValueAirTransport] = enable; } break;
                case TriggerBehavior::NoEnergyOverlay:      { if (unit_ext.values[ValueNoEnergyOverlay] != enable) unit_ext.values[ValueNoEnergyOverlay] = enable; } break;
                case TriggerBehavior::InfrastructureBurrow: { if (unit_ext.values[ValueOverrideInfrastructureBurrow] != enable) unit_ext.values[ValueOverrideInfrastructureBurrow] = enable; } break;
            }
        }
    })
    .add_action("move_unit", [](Designator designator, Area area_dest, u32 count, b32 overlay) {
        const auto dest = area_dest.center();
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            const auto origin = unit->position;
            if (!bw::teleport_to_point(unit, dest)) {
                bw::printf("move_unit: Failed to move unit with id %u to coordinates %u, %u", unit->id, dest.x, dest.y);
            }
            if (overlay && !unit->get_overlay(ImageId::LegionnaireTeleportEffect)) {
                helper::create_thingy(SpriteId::LegionnaireTeleportEffect, origin.x, origin.y, unit->player);
                bw::add_unit_overlay(unit, ImageId::LegionnaireTeleportEffect);
            }
        }
    })
    .add_action("give_unit", [](Context& ctx, Designator designator, u32 count, PlayerOrNeutral target_player) {
        if (!target_player) {
            ctx.error("Invalid player");
            return;
        }
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            helper::give_unit(unit, *target_player, 1);
        }
    })
    .add_action("issue_order", [](Designator source, Designator target, u32 count, u32 order_id, u32 secondary_unit_id) {
        const auto dest = target.area.center();
        for (Unit* unit : source.search({.sort = count != 0}).take(count)) {
            switch (order_id) {
                case OrderId::BuildTerran:
                case OrderId::BuildProtoss1:
                case OrderId::DroneStartBuild:
                case OrderId::MorphUnit:
                case OrderId::MorphStructure:
                case OrderId::TrainUnit:
                case OrderId::TrainFighter:
                case OrderId::BuildNydusExit: {
                    if (secondary_unit_id == UnitId::None) {
                        continue;
                    }
                    unit->build_queue[unit->build_queue_slot] = secondary_unit_id;
                    helper::issue_order_targeting_ground(unit, order_id, dest.x, dest.y);
                } break;
                case OrderId::Move:
                case OrderId::AttackMove:
                case OrderId::Patrol:
                case OrderId::RallyPointToTile: {
                    helper::issue_order_targeting_ground(unit, order_id, dest.x, dest.y);
                } break;
                case OrderId::Burrow:
                case OrderId::Unburrow:
                case OrderId::BuildingLiftoff: {
                    helper::perform_another_order(order_id, 0, unit, UnitId::None, {}, 0);
                } break;
                case OrderId::GriefOfAllGods: {
                    unit->issue_custom_order(order_id);
                } break;
                case OrderId::ConstructingBuilding: {
                    for (Unit* target_unit : g_unit_search.rect(target.area.box)) {
                        if (target_unit && target_unit->is_building() && !target_unit->is_completed()) {
                            unit->order_to(order_id, target_unit);
                            break;
                        }
                    }
                } break;
            }
        }
    })
    .add_action("rescue_unit", [](Designator designator, u32 count, u32 target_player) {
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            rescue_unit(unit, target_player - 1);
        }
    })
    .add_action("highlight_unit", [](Designator designator, u32 count, s32 timer) {
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            unit->sprite->selection_timer = timer ? timer : 45;
        }
    })
    .add_action("turn_unit", [](Designator designator, u32 count, s32 direction) {
        for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
            helper::turn_unit(unit, direction);
        }
    })
    .add_action("set_nydus_link", [](Designator designator_entry, Designator designator_exit, b32 enable) {
        const auto entry = designator_entry.search().first();
        const auto exit  = designator_exit.search().first();
        if (!entry || !exit) {
            return;
        }
        if (enable) {
            entry->building.nydus_exit = exit;
            exit->building.nydus_exit  = entry;
            entry->ext().values[ValueIsNydusEntry] = true;
            exit->ext().values[ValueIsNydusExit]   = true;
        } else {
            entry->building.nydus_exit = 0;
            exit->building.nydus_exit  = 0;
            entry->ext().values[ValueIsNydusEntry] = false;
            exit->ext().values[ValueIsNydusExit]   = false;
        }
    })
    .add_action("var", [](Context& ctx, std::string_view name) {
        ctx.subcommands(
            Subcommand{"=", [=](VarValue value) { g_trigger_vars.find_or_create(TriggerVarType_Var, name).set(value); }},
            Subcommand{"+=", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, false, '+'); }},
            Subcommand{"+==", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, true, '+'); }},
            Subcommand{"-=", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, false, '-'); }},
            Subcommand{"-==", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, true, '-'); }},
            Subcommand{"*=", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, false, '*'); }},
            Subcommand{"*==", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, true, '*'); }},
            Subcommand{"/=", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, false, '/'); }},
            Subcommand{"/==", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, true, '/'); }},
            Subcommand{"%=", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, false, '%'); }},
            Subcommand{"%==", [=](VarValue value) { update_trigger_var(TriggerVarType_Var, name, value, true, '%'); }},
            Subcommand{"toggle", [=] { auto& var = g_trigger_vars.find_or_create(TriggerVarType_Var, name); var.set(!var.value()); }},
            Subcommand{"unset", [=] { g_trigger_vars.unset(TriggerVarType_Var, name); }}
        );
    })
    .add_action("timer", [](Context& ctx, std::string_view name) {
        ctx.subcommands(
            Subcommand{"=", [=](TimerValue value) { g_trigger_vars.find_or_create(TriggerVarType_Timer, name).set(value); }},
            Subcommand{"+=", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, false, '+'); }},
            Subcommand{"+==", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, true, '+'); }},
            Subcommand{"-=", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, false, '-'); }},
            Subcommand{"-==", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, true, '-'); }},
            Subcommand{"*=", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, false, '*'); }},
            Subcommand{"*==", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, true, '*'); }},
            Subcommand{"/=", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, false, '/'); }},
            Subcommand{"/==", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, true, '/'); }},
            Subcommand{"%=", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, false, '%'); }},
            Subcommand{"%==", [=](TimerValue value) { update_trigger_var(TriggerVarType_Timer, name, value, true, '%'); }},
            Subcommand{"toggle", [=] { auto& var = g_trigger_vars.find_or_create(TriggerVarType_Timer, name); var.set(!var.value()); }},
            Subcommand{"unset", [=] { g_trigger_vars.unset(TriggerVarType_Timer, name); }}
        );
    })
    .add_action("bank_link_var", [](Identifier bank, Identifier var, s32 default_value) {
        g_trigger_vars.bank_link_var(std::string{bank}, std::string{var}, default_value);
    })
    .add_action("bank_clear", [](Identifier bank) {
        g_trigger_vars.get_bank(std::string{bank}).clear();
    })
    .add_action("convert_to_ai", [](Context& ctx, PlayablePlayer player) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_player_table[*player].type = PlayerType::Computer;
        bw::run_default_ai(*player, g_start_positions[*player]);
    })
    .add_action("threshold_set_fraud_slots", [](PlayerBitset players) {
        threshold_config.enabled = true;
        for (u32 player = 0; player < PlayerLimit; ++player) {
            threshold_config.fraud_allowed[player] = players[player];
            if (players[player]) {
                threshold_config.witness_enabled[player] = false;
            }
        }
    })
    .add_action("threshold_set_witness_slots", [](PlayerBitset players) {
        for (u32 player = 0; player < PlayerLimit; ++player) {
            threshold_config.witness_enabled[player] = players[player];
            if (players[player]) {
                threshold_config.fraud_allowed[player] = false;
            }
        }
    })
    .add_action("threshold_autoscale",    [](f32 value)  { threshold_config.auto_scale    = value; })
    .add_action("threshold_initial_count",[](u32 count)  { threshold_config.initial_count = count; })
    .add_action("threshold_min_count",    [](u32 count)  { threshold_config.min_count     = count; })
    .add_action("threshold_total_count",  [](u32 count)  { threshold_config.total_count   = count; })
    .add_action("threshold_timer",        [](Ticks ticks) { threshold_config.timer = ticks; })
    .add_action("set_celebration", [](Context& ctx) {
        ctx.subcommands(
            Subcommand{"auto", [] { init_auto_celebration(); }},
            Subcommand{"none", [] { g_globals->celebration = Celebration_None; }},
            Subcommand{"april_fools", [] { g_globals->celebration = Celebration_AprilFools; }},
            Subcommand{"christmas", [] { g_globals->celebration = Celebration_Christmas; }}
        );
    })
    .add_action("spawn_cargo", [](Designator designator, s32 count, UnitId::Enum unit_id, s32 player, bool force) {
        for (Unit* unit : designator.search()) {
            for (u32 i = 0; i < count; ++i) {
                const auto cargo_unit = bw::create_unit_in_game(unit_id, player ? player : unit->player, 0, 0);
                if (cargo_unit) {
                    if (force ? !bw::transport_is_full(unit) : bw::can_be_entered_by(unit, cargo_unit)) {
                        helper::load_unit_proc(unit, cargo_unit);
                    } else {
                        cargo_unit->silent_remove();
                        break;
                    }
                }
            }
        }
    })
    .add_action("tyrannize", [](Designator designator, s32 tyrannizing_player, UnitId::Enum tyrant_unit_id) {
        for (Unit* unit : designator.search()) {
            mind_control(tyrannizing_player ? tyrannizing_player - 1 : unit->player, tyrant_unit_id, unit, true);
            bw::play_sound(SoundId::Protoss_Darchon_mind_wav, unit->position);
            unit->order_to_idle();
        }
    })
    .add_action("blissful_embrace", [](Designator designator, s32 infesting_player) {
        for (Unit* unit : designator.search()) {
            blissful_embrace(unit, infesting_player ? infesting_player - 1 : unit->player, true);
        }
    })
    .add_action("set_sprite_player", [](Context& ctx, Designator designator, PlayablePlayer player, std::optional<u32> count) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        const auto unit_count = count.value_or(std::numeric_limits<u32>::max());
        for (Unit* unit : designator.search({.sort = unit_count != 0}).take(unit_count)) {
            unit->sprite->player = *player + 1;
        }
    })
    .add_action("witness_mode", [](Context& ctx, s32 value) {
        WitnessMode new_value   = (WitnessMode)value;
        u32 trigger_player      = ctx.player();
        auto& cm_player         = g_globals->players[trigger_player];
        const auto old_value = cm_player.witness_mode;
        if (old_value == new_value) {
            return;
        }
        g_globals->players[trigger_player].witness_mode = new_value;
        g_graphics.reset_stat_res_graphics();
        if (new_value == WitnessMode_Pure) {
            g_player_name_list->player[trigger_player].type = 0;
            *g_score_screen_player_count -= 1;
        } else if (old_value == WitnessMode_Pure) {
            g_player_name_list->player[trigger_player].type = g_player_table[trigger_player].type;
            *g_score_screen_player_count += 1;
        }
        if (new_value && g_elapsed_time_frames == 0 && trigger_player == g_local_nation_id) {
            u32 first_player = 0;
            for (u32 i = 0; i < PlayerLimit; ++i) {
                if (g_globals->players[i].witness_mode) {
                    continue;
                }
                if (g_player_table[i].type == PlayerType::Human) { first_player = i + 1; break; }
            }
            if (!first_player) {
                for (u32 i = 0; i < PlayerLimit; ++i) {
                    if (g_globals->players[i].witness_mode) {
                        continue;
                    }
                    if (g_player_table[i].type == PlayerType::Computer) { first_player = i + 1; break; }
                }
            }
            if (first_player) {
                first_player -= 1;
                auto pos = g_start_positions[first_player];
                pos.x = pos.x > 320 ? pos.x - 320 : 0;
                pos.y = pos.y > 200 ? pos.y - 200 : 0;
                helper::move_screen(pos);
            }
        }
    })
    .add_action("set_faction", [](Context& ctx, PlayablePlayer player, TriggerFaction faction) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_globals->players[*player].faction          = Faction(u8(faction));
        g_globals->players[*player].faction_was_set  = true;
    })
    .add_action("set_advisor", [](Context& ctx, PlayablePlayer player, Advisor advisor) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        if (*player == g_local_nation_id) {
            g_globals->current_advisor = advisor;
        }
    })
    .add_action("neutral_hostile", [](Context& ctx, bool state) {
        auto& cm_player = g_globals->players[ctx.player()];
        if (cm_player.neutral_hostile != state) {
            cm_player.neutral_hostile = state;
        }
    })
    .add_action("set_player_team_ui", [](Context& ctx, PlayablePlayer player, bool state) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        auto& cm_player = g_globals->players[*player];
        if (cm_player.hide_in_team_ui != state) {
            cm_player.hide_in_team_ui = state;
        }
    })
    .add_action("set_game_mode_race", [](Context& ctx, PlayablePlayer player, GameModeRace omni_state) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_globals->players[*player].game_mode_race = omni_state;
    })
    .add_action("set_player_build_multiplier", [](Context& ctx, PlayablePlayer player, u32 multiplier) {
        if (!player) {
            ctx.error("Invalid player");
            return;
        }
        g_globals->players[*player].build_multiplier = multiplier;
    })
    .add_action("print", [](std::string_view text) {
        bw::print(std::string{text}.c_str());
    })
    .add_action("print_var", [](Identifier identifier) {
        const auto var = g_trigger_vars.find(TriggerVarType_Var, std::string_view{identifier});
        bw::printf("Variable %s = %s", std::string{identifier}.c_str(), var ? std::to_string(var->value()).c_str() : "unset");
    })
    .add_action("print_timer", [](Identifier identifier) {
        const auto timer = g_trigger_vars.find(TriggerVarType_Timer, std::string_view{identifier});
        bw::printf("Timer %s = %s", std::string{identifier}.c_str(), timer ? std::to_string(timer->value()).c_str() : "unset");
    })
    .add_action("set_cheats_allowed", [](bool value) {
        g_aise_state.cheats_allowed = value;
    })
    .add_action("set_liftoff_disabled", [](PlayerBitset players, bool value) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].disable_liftoff = value;
        }
    })
    .add_action("set_burrow_disabled", [](PlayerBitset players, bool value) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].disable_burrow = value;
        }
    })
    .add_action("set_burrow_infrastructure_override", [](PlayerBitset players, bool value) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].override_infrastructure_burrow = value;
        }
    })
    .add_action("disable_tactical_ai", [](PlayerBitset players, bool value) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].disable_tactical_ai = value;
        }
    })
    .add_action("disable_extended_waves", [](PlayerBitset players, bool value) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].disable_extended_waves = value;
        }
    })
    .add_action("set_units_disabled", [](PlayerBitset players, UnitIdBitset unit_ids) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            g_globals->players[player].disabled_units = unit_ids;
        }
    })
    .add_action("disable_units", [](PlayerBitset players, UnitIdBitset unit_ids, std::optional<bool> disable, bool hide) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            auto& cm_player = g_globals->players[player];
            if (disable.value_or(true)) {
                cm_player.disabled_units |= unit_ids;
                if (hide) {
                    cm_player.hidden_units |= unit_ids;
                }
            } else {
                cm_player.disabled_units &= ~unit_ids;
                cm_player.hidden_units &= ~unit_ids;
            }
        }
    })
    .add_action("announce_player_defeat", [](PlayerBitset players, std::optional<bool> bool_state) {
        for (u32 player = 0; player < PlayerLimit; player++) {
            if (!players[player]) {
                continue;
            }
            AnnounceDefeatCommand command = {
                command.type = GameCommand_Custom,
                command.subtype = NewGameCommand::AnnounceDefeat,
                command.player = player,
                command.show_player_number = bool_state.value_or(true),
            };
            helper::queue_game_command((u8*)&command, sizeof(command));
        }
    })
    .add_action("table_edit", [](Context& ctx) {
        TableEditor(ctx.parser()).run();
    })
    .add_action("move_location", [](Context& ctx, Location* location, Designator designator, s32 offset_left, s32 offset_top, s32 offset_right, s32 offset_bottom) {
        if (!location) { ctx.error("Invalid location"); return; }
        auto loc_offsets = location->dimensions.offsets_from_center();
        loc_offsets.left   += offset_left;
        loc_offsets.top    += offset_top;
        loc_offsets.right  += offset_right;
        loc_offsets.bottom += offset_bottom;
        if (const auto unit = designator.search({.sort = true}).first()) {
            location->dimensions.left   = unit->position.x + loc_offsets.left;
            location->dimensions.top    = unit->position.y + loc_offsets.top;
            location->dimensions.right  = unit->position.x + loc_offsets.right;
            location->dimensions.bottom = unit->position.y + loc_offsets.bottom;
        }
    })
    .add_action("recreate_units", [](Designator designator) {
        for (Unit* unit : designator.search()) {
            const auto unit_id = unit->id;
            const auto player  = unit->player;
            const auto pos     = unit->position;
            const auto had_ai  = unit->unit_ai;
            unit->silent_remove();
            bw::create_unit_in_game(unit_id, player, pos.x, pos.y, nullptr, had_ai);
        }
    });

} // namespace trigger
