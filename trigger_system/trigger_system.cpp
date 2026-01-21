#include "trigger_system.h"
#include <limits.h>
#include <time.h>

#include "trigger_parser.h"
#include "trigger_sound.h"
#include "trigger_recolor.h"
#include "trigger_objectives.h"
#include "trigger_spawn.h"
#include "trigger_vars.h"
#include "trigger_events.h"
#include "table_edit.h"

#include "ai_layouts.h"
#include "player_color.h"
#include "threshold.h"
#include "config.h"

#include "hooks/harvest.h"
#include "hooks/interface/uifeatures/hotkeys.h"
#include "graphics/graphics.h"
#include "aise_signal.h"

#include <ranges>

static void init_auto_celebration() {
    if (!g_config.celebrations_enabled) {
        return;
    }

    time_t now = time(0);
    tm* ltm = localtime(&now);
    u32 month = ltm->tm_mon + 1;
    switch (month) {
        case 4: {
        //	g_globals->celebration = Celebration_AprilFools; // Causes conflicts with replays atm. Will fix later. -Pr0nogo, April '24
        } break;
        case 12: {
            g_globals->celebration = Celebration_Christmas;
        } break;
        default: {
            g_globals->celebration = Celebration_None;
        } break;
    }
}

Advisor get_advisor_type_from_config(u32 race) {
    std::vector<Advisor> advisors = g_config.common_advisors; // Note this is a copy
    switch (race) {
        case RaceId::Terran: {
            advisors.insert_range(advisors.end(), g_config.terran_advisors);
        } break;
        case RaceId::Protoss: {
            advisors.insert_range(advisors.end(), g_config.protoss_advisors);
        } break;
        case RaceId::Zerg: {
            advisors.insert_range(advisors.end(), g_config.zerg_advisors);
        } break;
    }
    if (advisors.size()) {
        u32 index = bw::rand_between_local(0, advisors.size() - 1);
        return advisors[index];
    }
    return Advisor::Default;
}

void init_trigger_system() {
    g_trigger.init = true;
    g_trigger.game_paused = false;
    g_action_table[47] = (ActionPointer)parse_comment_action;

    init_auto_celebration();
    init_recolors();
    g_objectives.reset();
    init_sound();
    init_spawn();
    g_trigger_vars.reset();
    init_trigger_events();
}

static bool unit_can_change_sprite(Unit* unit) {
    if (!unit->sprite || !unit->sprite->main_graphic) {
        return false;
    }
    u32 flingy_id = units_dat::graphic[unit->id];
    u32 sprite_id = flingy_dat::sprite_id[flingy_id];
    if (unit->sprite->id != sprite_id) {
        return false;
    }
    u32 image_id = sprites_dat::image_id[sprite_id];
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit->sprite->main_graphic->id != image_id &&
        (!unit_ext->values[ValueCustomImage] || unit->sprite->main_graphic->id != unit_ext->values[ValueCustomImage] - 1)) {
        return false;
    }
    return true;
}

static u32 unit_get_desired_sprite_id(Unit* unit) {
    for (SpriteReplacement& sprite_replacement : g_globals->sprite_replacements) {
        if (unit->player > PlayerLimit || !sprite_replacement.players[unit->player]) {
            continue;
        }
        if (unit->id >= UnitId::Count || !sprite_replacement.unit_ids[unit->id]) {
            continue;
        }
        return sprite_replacement.sprite_id;
    }

    switch (g_globals->celebration) {
        case Celebration_AprilFools: {
            switch (unit->id) {
                case UnitId::ZergQuazilisk:					return SpriteId::AprilFools_Quazilisk;
            }

            // TODO: Sounds
            {
            //	SoundReplacement sound_replacement = {};
            //	sound_replacement.any_animation = true;
            //	sound_replacement.any_sound = true;
            //	sound_replacement.replace = false;
            }
        } break;
        case Celebration_Christmas: {
            switch (unit->id) {
                case UnitId::ResourceMineralField:		return SpriteId::Christmas_MineralField1;
                case UnitId::ResourceMineralFieldType2:	return SpriteId::Christmas_MineralField2;
                case UnitId::ResourceMineralFieldType3:	return SpriteId::Christmas_MineralField3;
                case UnitId::TerranMinistry:			return SpriteId::Christmas_CommandCenter;
                case UnitId::ProtossNexus:				return SpriteId::Christmas_Nexus;
                case UnitId::ZergHachirosk:				return SpriteId::Christmas_Hachirosk;
                case UnitId::ZergCaphrolosk:			return SpriteId::Christmas_Caphrolosk;
                case UnitId::ZergGathtelosk:			return SpriteId::Christmas_Gathtelosk;
            }
        } break;
    }
    return unit->standard_sprite_id();
}

static void replace_sprite(Unit* unit, u32 sprite_id) {
    u32 image_id = sprites_dat::image_id[sprite_id];
    if (!unit->sprite || !unit->sprite->main_graphic || unit->sprite->main_graphic->id == image_id) {
        return;
    }

    // This shouldn't be needed, since `unit_can_change_sprite` should protect us already and avoid visual flicker
    // Unfortunately without this shitty hardcode nexus never warps
    if (!(unit->status & UnitStatus::Completed)) {
        return;
    }

    // Remove old shadow, since the new image will create it's own
    // Would be better to do it only after we know new_image is created, but then the other shadow also gets removed
    // As of right now shadow would disappear if the main image replacement fails
    Image* next_image = 0;
    for (Image* image = unit->sprite->images.head; image; image = next_image) {
        next_image = image->link.next;
        if (image->palette_type == PaletteType::Shadow) {
            image->free();
        }
    }

    // Replace main image
    u8 direction = unit->current_direction_1;
    Image* new_image = unit->sprite->create_overlay(image_id, 0, 0, unit->sprite->main_graphic->direction);
    if (new_image) {
        new_image->play_iscript_animation((IscriptAnimation::Enum)unit->sprite->main_graphic->animation);
        unit->sprite->main_graphic->free();
        unit->sprite->main_graphic = new_image;

        UnitExt* unit_ext = get_unit_ext(unit);
        unit_ext->values[ValueCustomImage] = image_id + 1;
    }
    helper::turn_unit(unit, direction);
}

void update_trigger_system(bool in_game) {
    if (!g_trigger.init) {
        init_trigger_system();
    }

    if (in_game) {
        g_trigger.in_game = true;
        for (auto& [trigger_action, trigger_ext] : g_globals->trigger_exts) {
            for (u32 player = 0; player < PlayerLimit; player++) {
                OngoingWait& ongoing_wait = trigger_ext.ongoing_waits[player];
                if (ongoing_wait.timer > 0) {
                    ongoing_wait.timer--;
                }
            }
        }
    } else if (g_trigger.in_game) {
        // Leaving the game should reset music
        g_trigger.in_game = false;
        init_trigger_system();
    }

    if (in_game) {
        update_sound();
    }
    bool paused = bw::is_game_paused();
    if (g_trigger.game_paused && !paused) {
        set_palette_recolor(recolor_system.current_recolor.set);
    }
    if (in_game && !paused) {
        update_recolors();
        update_spawn();
        g_trigger_vars.update();
        g_objectives.update();
        buttons_draw();

        for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
            if (!unit_can_change_sprite(unit)) {
                continue;
            }
            u32 sprite_id = unit_get_desired_sprite_id(unit);
            replace_sprite(unit, sprite_id);
        }

        if (g_globals->alert.own_cooldown) {
            g_globals->alert.own_cooldown--;
        }
        if (g_globals->alert.ally_cooldown) {
            g_globals->alert.ally_cooldown--;
        }
        if (g_globals->alert.play_timer) {
            g_globals->alert.play_timer--;
            if (!g_globals->alert.play_timer) {
                bw::play_sound(g_globals->alert.sound_id);
            }
        }

        if (g_globals->advisor_state.cooldown) {
            g_globals->advisor_state.cooldown--;
        }
        if (g_globals->advisor_state.play_timer) {
            g_globals->advisor_state.play_timer--;
            if (!g_globals->advisor_state.play_timer) {
                bw::play_sound(g_globals->advisor_state.sound_id);
            }
        }
    }
    g_trigger.game_paused = paused;
}

static Box16 button_rect(u32 index) {
    Box16 rect = {};
    rect.x = 16;
    rect.y = 40 + index * 28;
    rect.w = 96;
    rect.h = 20;
    return rect;
}

static bool rect_is_hovered(Box16 rect) {
    return g_mouse->x >= rect.x && g_mouse->x <= rect.x + rect.w
        && g_mouse->y >= rect.y && g_mouse->y <= rect.y + rect.h;
}

bool buttons_hovers() {
    for (u32 i = 0; i < g_globals->trigger_buttons.size(); i++) {
        Box16 rect = button_rect(i);
        if (rect_is_hovered(rect)) {
            return true;
        }
    }
    return false;
}

bool buttons_handle_input(const KeyEvent& key_event) {
    for (u32 i = 0; i < g_globals->trigger_buttons.size(); i++) {
        Box16 rect = button_rect(i);
        if (!rect_is_hovered(rect)) {
            continue;
        }

        TriggerButton& button = g_globals->trigger_buttons[i];
        if (key_event.key == VK_LBUTTON && key_event.pressed()) {
            TriggerVar& var = g_trigger_vars.find_or_create(TriggerVarType_Var, button.id.sv);
            var.set(1);
            g_globals->trigger_buttons.erase(g_globals->trigger_buttons.begin() + i);
            return true;
        }
    }
    return false;
}

void buttons_draw() {
    for (u32 i = 0; i < g_globals->trigger_buttons.size(); i++) {
        Box16 rect = button_rect(i);
        bool hovered = rect_is_hovered(rect);

        TriggerButton& button = g_globals->trigger_buttons[i];
        g_graphics.draw_filled_box(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, hovered ? graphics::GREY : graphics::BLACK);
        g_graphics.draw_box(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, hovered ? graphics::WHITE : graphics::GREY);

        const std::string text{button.display_name.sv};
        graphics::FontSize font_size = graphics::FONT_MEDIUM;

        graphics::Font* font = font_base[font_size];
        s32 w = font->getTextWidth(text.c_str());
        s32 h = font->getTextHeight(text.c_str());
        g_graphics.draw_text(rect.x + rect.w / 2 - w / 2, rect.y + rect.h / 2 - h / 2, text, font_size); // TODO: center
    }
}

static bool parse_condition(Parser& parser) {
    if (parser.get_word()) {
        if (parser.equals("combat")) {
            Area area = parser.get_area();
            PlayerBitset source_players = parser.get_player_bitset();
            UnitIdBitset source_unit_ids = parser.get_unit_id_bitset();
            PlayerBitset target_players = parser.get_player_bitset();
            UnitIdBitset target_unit_ids = parser.get_unit_id_bitset();
            Quota time_quota = parser.get_time_quota(QuotaType_LessOrEqual);
            u32 scouting_player = parser.get_player();

            for (const TriggerEvent& event : combat_events) {
                if (!area.check(event.position, units_dat::base_property[event.target_unit_id] & UnitProperty::Flyer)) {
                    continue;
                }
                if (event.source_player >= FullPlayerLimit || event.target_player >= FullPlayerLimit) {
                    continue;
                }
                if ((!source_players[event.source_player] || !source_unit_ids[event.source_unit_id] || !target_players[event.target_player] || !target_unit_ids[event.target_unit_id])
                &&  (!source_players[event.target_player] || !source_unit_ids[event.target_unit_id] || !target_players[event.source_player] || !target_unit_ids[event.source_unit_id])) {
                    continue;
                }
                if (!time_quota.check(g_elapsed_time_frames - event.timestamp)) {
                    continue;
                }
                if (scouting_player) {
                    if (!(event.seen_by & 1 << (scouting_player - 1))) {
                        continue;
                    }
                }
                return true;
            }
            return false;
        } else if (parser.equals("kills")) {
            Area area = parser.get_area();
            PlayerBitset source_players = parser.get_player_bitset();
            UnitIdBitset source_unit_ids = parser.get_unit_id_bitset();
            PlayerBitset target_players = parser.get_player_bitset();
            UnitIdBitset target_unit_ids = parser.get_unit_id_bitset();
            Quota quota = parser.get_quota(QuotaType_GreaterOrEqual);
            Quota time_quota = parser.get_time_quota(QuotaType_LessOrEqual);
            u32 scouting_player = parser.get_player();

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
                if (scouting_player) {
                    if (!(event.seen_by & 1 << (scouting_player - 1))) {
                        continue;
                    }
                }
                count += 1;
            }

            if (quota.check(count)) {
                return true;
            }
            return false;
        } else if (parser.equals("deaths")) {
            Area area = parser.get_area();
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            Quota quota = parser.get_quota();
            Quota time_quota = parser.get_time_quota();
            u32 scouting_player = parser.get_player();

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
                if (scouting_player) {
                    if (!(event.seen_by & 1 << (scouting_player - 1))) {
                        continue;
                    }
                }
                count += 1;
            }

            if (quota.check(count)) {
                return true;
            }
            return false;
        } else if (parser.equals("unit_count")) {
            Designator designator = parser.get_designator();
            Quota quota = parser.get_quota();
            u32 scouting_player = parser.get_player();
            Quota scouting_time_quota = parser.get_time_quota();

            u32 count = 0;
            EntryArray<Unit> units = scouting_player ? designator.search_seen(scouting_player - 1) : designator.search();
            for (Unit* unit : units) {
                UnitExt* unit_ext = get_unit_ext(unit);
                Point16 position = unit->position;
                if (scouting_player) {
                    position = unit_ext->last_seen_position[scouting_player - 1];
                }
                if (!scouting_player || !scouting_time_quota.value || scouting_time_quota.check(unit_ext->last_seen_timer[scouting_player - 1])) {
                    count++;
                }
            }

            if (quota.check(count)) {
                return true;
            }
            return false;
        } else if (parser.equals("local_camera")) {
            Area area = parser.get_area();
            Token min_visibility = parser.get_word();

            Point16 center = {};
            center.x = g_move_to_x + 320;
            center.y = g_move_to_y + 200;

            if (center.x >= area.box.left && center.x <= area.box.right &&
                center.y >= area.box.top  && center.y <= area.box.bottom) {
                ActiveTile* tile = bw::get_active_tile_at_coords(center.x, center.y);
                // TODO: should we check area flags?
                if (sv_iequals(min_visibility.sv, "unexplored")) {
                    return true;
                } else if (sv_iequals(min_visibility.sv, "explored")) {
                    if (!(tile->explored_flags & (1 << g_local_nation_id))) {
                        return true;
                    }
                } else if (sv_iequals(min_visibility.sv, "visible") || true) {
                    if (!(tile->visibility_flags & (1 << g_local_nation_id))) {
                        return true;
                    }
                }
            }
            return false;
        } else if (parser.equals("var") || parser.equals("timer")) {
            bool is_timer = parser.equals("timer");
            TriggerVarType var_type = is_timer ? TriggerVarType_Timer : TriggerVarType_Var;

            Token name = parser.get_string();
            QuotaType quota_type = parser.get_quota_type(QuotaType_Equal);
            TriggerVar* var = parser.get_char('=') ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
            s32 quota_value = is_timer ? parser.get_time_ticks() : parser.get_s32();
            if (var) {
                return Quota{quota_type, quota_value}.check(var->value());
            }
            return false;
        } else if (parser.equals("unit_color")) {
            Designator designator = parser.get_designator();
            Quota quota = parser.get_quota();
            u32 color_id = parser.get_s32(); // Currently naive; need a way to get multiple pcolors and match them. -Pr0nogo

            u32 count = designator.search()
                .filter_by([color_id](Unit* unit) {
                    UnitExt* unit_ext = get_unit_ext(unit);
                    return unit_ext->values[ValuePlayerColor] == color_id;
                })
                .count();

            if (quota.check(count)) {
                return true;
            }
            return false;
        } else if (parser.equals("player_race")) {
            u32 player = parser.get_player();
            Quota op = parser.get_quota(QuotaType_Equal);
            const auto race = parser.get_enum(race_names, std::size(race_names));
            if (player) {
                player -= 1;
                switch (op.type) {
                    case QuotaType_Equal:    return g_player_table[player].race == race;
                    case QuotaType_NotEqual: return g_player_table[player].race != race;
                }
            }
            return false;
        } else if (parser.equals("player_type")) {
            auto player = parser.get_player();
            const Quota op = parser.get_quota(QuotaType_Equal);
            const auto type = parser.get_magic_enum<CrigPlayerType>();
            auto compare_type = PlayerType::Human;
            bool type_is_witness = false;
            switch (type) {
                case CrigPlayerType::Human:            compare_type = PlayerType::Human;            break;
                case CrigPlayerType::HumanDefeated:    compare_type = PlayerType::HumanDefeated;    break;
                case CrigPlayerType::Computer:         compare_type = PlayerType::Computer;         break;
                case CrigPlayerType::ComputerDefeated: compare_type = PlayerType::ComputerDefeated; break;

                case CrigPlayerType::Witness:
                case CrigPlayerType::WitnessDefeated: {
                    type_is_witness = true;
                }
            }
            if (player) {
                player -= 1;
                if (!type_is_witness) {
                    switch (op.type) {
                        case QuotaType_Equal:    return g_player_table[player].type == compare_type;
                        case QuotaType_NotEqual: return g_player_table[player].race != compare_type;
                    }
                } else {
                    auto witness_type = g_globals->players[player].witness_mode;
                    switch (op.type) {
                        case QuotaType_Equal:    return type == CrigPlayerType::Witness ? witness_type == WitnessMode_Pure : witness_type == WitnessMode_Defeated;
                        case QuotaType_NotEqual: return type == CrigPlayerType::Witness ? witness_type != WitnessMode_Pure : witness_type != WitnessMode_Defeated;
                    }
                }
            }
            return false;
        }/* else if (parser.eq("selected")) {
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            QuotaType quota_type = parser.get_quota_type(QuotaType_GreaterOrEqual);
            u32 quota = parser.get_s32();

            u32 count = 0;
            for (u32 i = 0; i < units.count; ++i) { // TODO: Replace with selection. -Pr0nogo
                Unit* unit = units.units[i];
                if (unit->player >= FullPlayerLimit || !players[unit->player]) {
                    continue;
                }
                if (unit->id >= UnitId::Count || !unit_ids[unit->id]) {
                    continue;
                }
                count++;
            }
            if (check_quota(quota_type, quota, count)) {
                return true;
            }
            return false;
        }*/
    }
    return false;
}

b32 parse_comment_condition(TriggerAction* action) {
    Parser parser{helper::get_chk_string(action->text_string)};
    while (parser.get_command()) {
        if (parser.equals("condition")) {
            if (!parse_condition(parser)) {
                return false;
            }
        }
    }
    return true;
}

b32 __fastcall parse_comment_action(TriggerAction* action) {
    Trigger& trigger = g_current_trigger_node->trigger;
    u32 trigger_player = *g_current_trigger_player_id;
    bool is_local_player = trigger_player == g_local_nation_id;

    TriggerExt& trigger_ext = g_globals->trigger_exts[action];
    OngoingWait& ongoing_wait = trigger_ext.ongoing_waits[trigger_player];

    Parser parser{helper::get_chk_string(action->text_string)};
    parser.load_command_pos(ongoing_wait.parser_position);
    while (parser.get_command()) {
        // Wait 
        if (parser.equals("wait")) {
            u32 time = parser.get_time_ticks();

            bool init = action->flags & 1;
            if (!init) {
                ongoing_wait.timer = time;
                ongoing_wait.parser_position = parser.command_start_pos();
                action->flags |= 1;
                return false;
            } else if (ongoing_wait.timer > 0) {
                return false;
            } else {
                ongoing_wait.parser_position = 0;
                action->flags &= ~1;
            }
        // Conditional Waits
        } else if (parser.equals("wait_if")) {
            if (parse_condition(parser)) {
                ongoing_wait.parser_position = parser.command_start_pos();
                return false;
            } else {
                ongoing_wait.parser_position = 0;
            }
        } else if (parser.equals("wait_until")) {
            if (!parse_condition(parser)) {
                ongoing_wait.parser_position = parser.command_start_pos();
                return false;
            } else {
                ongoing_wait.parser_position = 0;
            }
        // Conditional Exits
        } else if (parser.equals("exit_if")) {
            if (parse_condition(parser)) {
                trigger.current_action_index = 64;
                return false;
            }
        } else if (parser.equals("exit_unless")) {
            if (!parse_condition(parser)) {
                trigger.current_action_index = 64;
                return false;
            }
        } else if (parser.equals("exit_preserve_if")) {
            if (parse_condition(parser)) {
                trigger.flags |= 0x4;
                trigger.current_action_index = 64;
                return false;
            }
        } else if (parser.equals("exit_preserve_unless")) {
            if (!parse_condition(parser)) {
                trigger.flags |= 0x4;
                trigger.current_action_index = 64;
                return false;
            }
        // Buttons
        } else if (parser.equals("show_button")) {
            Token id = parser.get_string();
            Token display_name = parser.get_string();
            s32 priority = parser.get_s32();

            TriggerButton trigger_button = {};
            trigger_button.id = id;
            trigger_button.display_name = !display_name.sv.empty() ? display_name : id;
            trigger_button.priority = priority;
            auto insert_it = std::ranges::upper_bound(g_globals->trigger_buttons, trigger_button, [](const auto& a, const auto& b) {
                return a.priority > b.priority;	
            });
            g_globals->trigger_buttons.insert(insert_it, trigger_button);

            TriggerVar& var = g_trigger_vars.find_or_create(TriggerVarType_Var, id.sv);
            var.set(0);
        } else if (parser.equals("hide_button")) {
            Token id = parser.get_string();

            std::erase_if(g_globals->trigger_buttons, [&](auto& button) { return button.id == id; });
        // Local Only Triggers
        } else if (parser.equals("play_music") && is_local_player) {
            Token filename = parser.get_string();
            g_music_playback.next_music = get_sound(filename.string().c_str());
            g_music_playback.target_fade = 0;
        } else if (parser.equals("stop_music") && is_local_player) {
            g_music_playback.target_fade = 0;
        } else if (parser.equals("pause_music") && is_local_player) {
            g_music_playback.target_fade = 0;
            g_music_playback.paused = true;
        } else if (parser.equals("resume_music") && is_local_player) {
            g_music_playback.target_fade = 1;
            g_music_playback.paused = false;
            play_music(true);
        } else if (parser.equals("music_fade_in") && is_local_player) {
            f32 fade_in = parser.get_time_seconds();
            g_music_playback.fade_in = fade_in;
        } else if (parser.equals("music_fade_out") && is_local_player) {
            f32 fade_out = parser.get_time_seconds();
            g_music_playback.fade_out = fade_out;
        } else if (parser.equals("music_volume") && is_local_player) {
            f32 volume = parser.get_f32();
            f32 time = parser.get_time_seconds();
            if (time == 0) {
                g_music_playback.volume = 0;
            }
            g_music_playback.target_volume = volume;
            g_music_playback.volume_time = time;
        } else if (parser.equals("playlist_clear") && is_local_player) {
            memset(&g_current_playlist, 0, sizeof(g_current_playlist));
        } else if (parser.equals("playlist_add") && is_local_player) {
            Token filename = parser.get_string();
            if (g_current_playlist.size < MaxSound) {
                SoundMainBuffer* music = get_sound(filename.string().c_str());
                if (music) {
                    g_current_playlist.tracks[g_current_playlist.size++] = music;
                }
            } else {
                bw::printf("Error: Playlist already full! (%s)", filename.string().c_str());
            }
        } else if (parser.equals("playlist_shuffle") && is_local_player) {
            s32 do_shuffle = parser.get_s32(); // 0 - false, 1 - true
            g_current_playlist.shuffle = do_shuffle;
        } else if (parser.equals("set_player_name") && is_local_player) {
            u32 player = parser.get_player();
            Token name = parser.get_string();
            s32 ignore_human_name = parser.get_s32();
            if (player) {
                player -= 1;
                if (!ignore_human_name && (g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::HumanDefeated)) {
                    std::string player_name = std::format("{} ({})", name.string(), g_player_table[player].name);
                    set_player_name(player, player_name.c_str(), true);
                } else {
                    set_player_name(player, name.string().c_str(), true);
                }
            }
        } else if (parser.equals("set_force_name") && is_local_player) {
            u32 player = parser.get_player();
            Token name = parser.get_string();
            if (player) {
                player -= 1;
                set_player_force_name(player, name.string().c_str());
            }
        } else if (parser.equals("recolor") && is_local_player) {
            TimedRecolor recolor = {};
            recolor.set  = parser.get_recolor_set();
            recolor.time = parser.get_time_seconds();

            recolor_system.recolor_time = 0;
            recolor_system.source_recolor = recolor_system.current_recolor;
            recolor_system.target_recolor = recolor;
            if (!recolor.time) {
                recolor_system.current_recolor = recolor;
                set_palette_recolor(recolor.set);
            }
        } else if (parser.equals("recolor_loop_add") && is_local_player) {
            TimedRecolor recolor = {};
            recolor.set  = parser.get_recolor_set();
            recolor.time = parser.get_time_seconds();

            if (recolor_system.recolor_list.count < MaxRecolors) {
                recolor_system.recolor_list.recolors[recolor_system.recolor_list.count++] = recolor;
            }
        } else if (parser.equals("recolor_loop_clear") && is_local_player) {
            memset(&recolor_system.recolor_list, 0, sizeof(recolor_system.recolor_list));
        } else if (parser.equals("objective_set") && is_local_player) {
            s32 key = parser.get_s32();
            Token text = parser.get_string();

            Objective& objective = g_objectives.get(key);
            objective.text = text.string();
            objective.max_score = 1;
            objective.show = true;
        } else if (parser.equals("objective_text") && is_local_player) {
            s32 key = parser.get_s32();
            Token text = parser.get_string();

            Objective& objective = g_objectives.get(key);
            objective.text = text.string();
        } else if (parser.equals("objective_desc") && is_local_player) {
            s32 key = parser.get_s32();
            Token text = parser.get_string();

            // TODO: set objective description
        } else if (parser.equals("objective_show") && is_local_player) {
            s32 key = parser.get_s32();
            s32 show = parser.get_s32();

            Objective& objective = g_objectives.get(key);
            objective.show = show;
        } else if (parser.equals("objective_score") && is_local_player) {
            s32 key = parser.get_s32();
            s32 score = parser.get_s32();
            s32 max_score = parser.get_s32();

            Objective& objective = g_objectives.get(key);
            objective.score = std::clamp(score, 0, max_score);
            objective.max_score = max_score;
        } else if (parser.equals("objective_score_add") && is_local_player) {
            s32 key = parser.get_s32();
            s32 score = parser.get_s32();
            s32 max_score = parser.get_s32();

            Objective& objective = g_objectives.get(key);
            objective.score = std::clamp(objective.score + score, 0, max_score);
            objective.max_score = max_score;
        } else if (parser.equals("objective_score_subtract") && is_local_player) {
            s32 key = parser.get_s32();
            s32 score = parser.get_s32();
            s32 max_score = parser.get_s32();

            Objective& objective = g_objectives.get(key);
            objective.score = std::clamp(objective.score - score, 0, max_score);
            objective.max_score = max_score;
        } else if (parser.equals("objective_timer") && is_local_player) {
            s32 key = parser.get_s32();
            u32 time = parser.get_time_ticks();

            Objective& objective = g_objectives.get(key);
            objective.timer = time;
        } else if (parser.equals("objective_remove") && is_local_player) {
            s32 key = parser.get_s32();

            g_objectives.remove(key);
        } else if (parser.equals("objectives_sync") && is_local_player) {
            // TODO: sync objectives to mission objectives menu
        } else if (parser.equals("objectives_clear") && is_local_player) {
            g_objectives.clear();
        } else if (parser.equals("show_player_layouts") && is_local_player) {
            PlayerBitset players = parser.get_player_bitset();
            s32 value = parser.get_s32();
            for (u32 i = 0; i < PlayerLimit; ++i) {
                if (!players[i]) {
                    continue;
                }
                g_show_player_layouts[i] = value;
            }
        } else if (parser.equals("set_player_unit_name") && is_local_player) {
            u32 player = parser.get_player();
            u32 unit_id = parser.get_unit_id();
            Token name = parser.get_string();
            if (player && unit_id != UnitId::None) {
                player -= 1;
                g_globals->players[player].custom_unit_names[(UnitId::Enum)unit_id] = name.string();
            }
        } else if (parser.equals("set_player_race_name") && is_local_player) {
            u32 player = parser.get_player();
            Token old_race = parser.get_string();
            Token new_race = parser.get_string();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();

            if (player) {
                player -= 1;

                auto& cm_player = g_globals->players[player];
                for (u32 i = 0; i < UnitId::Count; ++i) {
                    if (!unit_ids[i]) {
                        continue;
                    }
                    const auto unit_id = (UnitId::Enum)i;
                    if (!cm_player.custom_unit_names.contains((UnitId::Enum)unit_id)) {
                        cm_player.custom_unit_names[unit_id] = Unit::get_name(unit_id);
                    }

                    auto& name = cm_player.custom_unit_names[unit_id];
                    const std::string_view name_sv{name};
                    const std::string_view old_race_sv{old_race.sv};
                    if (name_sv.starts_with(old_race_sv)) {
                        std::stringstream ss;
                        ss << new_race.sv << std::string_view{name.begin() + old_race.sv.size(), name.end()};
                        name = ss.str();
                    }
                }
            }
        } else if (parser.equals("reset_player_unit_names") && is_local_player) {
            u32 player = parser.get_player();
            if (player) {
                player -= 1;
                g_globals->players[player].custom_unit_names.clear();
            }
        } else if (parser.equals("unit_sound") && is_local_player) {
            SoundReplacement sound_replacement = {};
            sound_replacement.players = parser.get_player_bitset();
            sound_replacement.unit_ids = parser.get_unit_id_bitset();
            sound_replacement.animation = parser.get_enum(animation_type_names, ARRAY_SIZE(animation_type_names));
            if (parser.equals("*")) {
                sound_replacement.any_animation = true;
            }
            sound_replacement.old_sound_id = parser.get_s32();
            if (parser.equals("*")) {
                sound_replacement.any_sound = true;
            }
            Token path = parser.get_string();
            sound_replacement.new_sound = get_sound(path.string().c_str());
            sound_replacement.replace = parser.get_s32();

            parser.get_word();
            if (parser.equals("clear")) {
                std::erase_if(g_globals->sound_replacements, [&](auto& it) { return it == sound_replacement; });
            } else {
                g_globals->sound_replacements.push_back(sound_replacement);
            }
        // Bitset Local Only Triggers
        } else if (parser.equals("play_sound") && is_local_player) {
            Token path = parser.get_string();
            PlayerBitset players = parser.get_player_bitset();
            Point16 position = parser.get_position();
            Token min_visibility = parser.get_word();

            if (trigger_player < PlayerLimit && players[trigger_player]) {
                if (position.x == 0 && position.y == 0) {
                    play_sound_ext(get_sound(path.string().c_str()), {.bw_volume = get_sound_volume()});
                } else {
                    bool play = false;
                    ActiveTile* tile = bw::get_active_tile_at_coords(position.x, position.y);

                    if (sv_iequals(min_visibility.sv, "unexplored")) {
                        play = true;
                    } else if (sv_iequals(min_visibility.sv, "explored")) {
                        if (!(tile->explored_flags & (1 << g_local_nation_id))) {
                            play = true;
                        }
                    } else if (sv_iequals(min_visibility.sv, "visible") || true) {
                        if (!(tile->visibility_flags & (1 << g_local_nation_id))) {
                            play = true;
                        }
                    }

                    if (play) {
                        play_sound_ext(get_sound(path.string().c_str()), {.position = position});
                    }
                }
            }
        // Global Flag
        } else if (parser.equals("forbid_replay_auto_upload") && is_local_player) {
            g_globals->forbid_replay_auto_upload = true;
        // Gameplay Actions
        } else if (parser.equals("set_mineral_yields")) {
            u32 depleted_yield = parser.get_s32();
            u32 yield = parser.get_s32();

            g_globals->yields[(size_t)YieldType::MineralDepleted] = depleted_yield;
            g_globals->yields[(size_t)YieldType::Mineral] = yield;
        } else if (parser.equals("set_gas_yields")) {
            u32 ridge_yield = parser.get_s32();
            u32 geyser_yield = parser.get_s32();
            u32 tier2_yield = parser.get_s32();
            u32 tier3_yield = parser.get_s32();

            g_globals->yields[(size_t)YieldType::GasRidge] = ridge_yield;
            g_globals->yields[(size_t)YieldType::GasGeyser] = geyser_yield;
            g_globals->yields[(size_t)YieldType::GasGeyserTier2] = tier2_yield;
            g_globals->yields[(size_t)YieldType::GasGeyserTier3] = tier3_yield;
        } else if (parser.equals("unit_sprite")) {
            SpriteReplacement sprite_replacement = {};
            sprite_replacement.players = parser.get_player_bitset();
            sprite_replacement.unit_ids = parser.get_unit_id_bitset();
            parser.get_word();
            if (parser.equals("sprite_id")) {
                sprite_replacement.sprite_id = parser.get_s32();
            } else if (parser.equals("unit_id")) {
                u32 unit_id = parser.get_unit_id();
                u32 flingy_id = units_dat::graphic[unit_id];
                sprite_replacement.sprite_id = flingy_dat::sprite_id[flingy_id];
            } else {
                continue;
            }

            g_globals->sprite_replacements.push_back(sprite_replacement);
        } else if (parser.equals("clear_unit_sprite")) {
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            u32 sprite_id = 0;
            parser.get_word();
            if (parser.equals("sprite_id")) {
                sprite_id = parser.get_s32();
            } else if (parser.equals("unit_id")) {
                u32 unit_id = parser.get_unit_id();
                u32 flingy_id = units_dat::graphic[unit_id];
                sprite_id = flingy_dat::sprite_id[flingy_id];
            } else {
                continue;
            }

            std::erase_if(g_globals->sprite_replacements, [&](const auto& sprite_replacement) {
                return sprite_replacement.players == players
                    && sprite_replacement.unit_ids == unit_ids
                    && sprite_replacement.sprite_id == sprite_id;
            });
        } else if (parser.equals("clear_creep")) {
            Area location = parser.get_area();
            bw::clear_creep(location.box);
        } else if (parser.equals("spawn")) {
            SpawnSequence sequence = {};
            sequence.count = parser.get_s32();
            sequence.unit_id = parser.get_unit_id();
            sequence.player = parser.get_player(true);
            sequence.type = parser.get_enum(spawn_type_names, SpawnType_Count);
            sequence.delay = parser.get_time_ticks();
            sequence.stagger = parser.get_time_ticks();
            sequence.source_pos = parser.get_position();
            sequence.move_pos = parser.get_position();
            
            if (sequence.player) {
                sequence.player -= 1;
                if (spawn_sequence_count < SpawnSequenceCap) {
                    spawn_sequences[spawn_sequence_count++] = sequence;
                }
            }
        } else if (parser.equals("layouts_enable")) {
            Area area = parser.get_area();
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            s32 enable = parser.get_s32();

            for (u32 player = 0; player < PlayerLimit; ++player) {
                if (!players[player]) {
                    continue;
                }
                PlayerLayouts& layouts = g_ai_layouts.get_player_layouts(player);
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
        } else if (parser.equals("layouts_priority_add")) {
            Area area = parser.get_area();
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            s32 priority = parser.get_s32();

            if (priority) {
                for (u32 player = 0; player < PlayerLimit; ++player) {
                    if (!players[player]) {
                        continue;
                    }
                    PlayerLayouts& layouts = g_ai_layouts.get_player_layouts(player);
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
        } else if (parser.equals("layouts_priority_set")) {
            Area area = parser.get_area();
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            s32 priority = parser.get_s32();

            u8 new_priority = CLAMP(priority, 0, 255);
            for (u32 player = 0; player < PlayerLimit; ++player) {
                if (!players[player]) {
                    continue;
                }
                PlayerLayouts& layouts = g_ai_layouts.get_player_layouts(player);
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
        } else if (parser.equals("layouts_assign")) {
            Area area = parser.get_area();
            PlayerBitset players = parser.get_player_bitset();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();
            Point16 resarea_position = parser.get_position(area.box.center());

            u32 resarea = bw::get_closest_resarea(resarea_position);
            for (u32 player = 0; player < PlayerLimit; ++player) {
                if (!players[player]) {
                    continue;
                }
                PlayerLayouts& layouts = g_ai_layouts.get_player_layouts(player);
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
        } else if (parser.equals("create_town")) {
            u32 player = parser.get_player();
            Point16 pos = parser.get_position();
            s32 worker_count = parser.get_s32();
            s32 attach_to_nearest_resarea = parser.get_s32();
            Area claim_placeholders_area = parser.get_area();
            UnitIdBitset unit_ids = parser.get_unit_id_bitset();

            if (player) {
                player -= 1;
                AiTown* town = helper::create_ai_town(player, pos.x, pos.y);
                if (town) {
                    town->inited = true;
                    TownLayouts& layouts_town = g_ai_layouts.get_town_layouts(*town);
                    layouts_town.worker_limit = worker_count;
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
                        PlayerLayouts& layouts = g_ai_layouts.get_player_layouts(player);
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
                                if (g_show_player_layouts[player]) {
                                    bw::printf("Player %u claimed %s #%u from resarea %u", player + 1, Unit::get_name(placeholder.unit_id), (u32)idx, placeholder.resarea);
                                }
                            }
                        }
                    }
                }
            }
        } else if (parser.equals("set_start_location")) {
            u32 player = parser.get_player();
            Point16 position = parser.get_position();

            if (player) {
                player -= 1;
                g_start_positions[player] = position;
            }
        } else if (parser.equals("set_race")) {
            u32 player = parser.get_player();
            u32 race = parser.get_enum(race_names, ARRAY_SIZE(race_names));

            if (player) {
                player -= 1;
                g_player_table[player].race = race;
                if (player == g_local_nation_id) {
                    g_globals->current_advisor = get_advisor_type_from_config(race);
                }
            }
        } else if (parser.equals("set_player_color")) {
            u32 player = parser.get_player();
            PlayerColor player_color = (PlayerColor)parser.get_enum(player_color_names, ARRAY_SIZE(player_color_names));

            if (player) {
                player -= 1;
                set_player_color(player, player_color);
            }
        } else if (parser.equals("set_ai_auto_burrow")) {
            Designator designator = parser.get_designator();
            s32 enable = parser.get_s32();

            for (Unit* unit : designator.search()) {
                UnitExt* unit_ext = get_unit_ext(unit);
                unit_ext->values[ValueDisableAutoBurrow] = !enable;
            }
        } else if (parser.equals("kill")) {
            Designator designator = parser.get_designator();
            u32 count = parser.get_s32();
            bool remove = parser.get_s32();

            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                if (remove) {
                    unit->silent_remove();
                } else {
                    unit->kill();
                }
            }
        } else if (parser.equals("set_unit_stat")) {
            Designator designator = parser.get_designator();
            u32 count = parser.get_s32();
            TriggerStat trigger_stat = (TriggerStat)parser.get_enum(trigger_stat_names, ARRAY_SIZE(trigger_stat_names));
            // We should separately make "modify_unit_stat" and have operator parsing for stat amount. -Pr0nogo
            // Not really... We would just have assign operators before value i.e. =|+=|-=|*=|/= -Veeq7
            s32 value = 0;
            if (trigger_stat == TriggerStat_TimedLife) {
                value = parser.get_time_ticks();
            } else {
                value = parser.get_s32();
            }

            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                auto& unit_ext = unit->ext();
                switch (trigger_stat) {
                    case TriggerStat_Armor: {
                        unit_ext.values[ValueTriggerArmorBaseSet] = 1;
                        unit_ext.values[ValueTriggerArmorBaseAmount] = value;
                    } break;
                    case TriggerStat_WeaponRange: {
                        unit_ext.values[ValueTriggerWeaponRangeBaseSet] = 1;
                        unit_ext.values[ValueTriggerWeaponRangeBaseAmount] = value;
                    } break;
                    case TriggerStat_ArmorPenetration: {
                        unit_ext.values[ValueTriggerArmorPenBaseSet] = 1;
                        unit_ext.values[ValueTriggerArmorPenBaseAmount] = value;
                    } break;
                    case TriggerStat_Damage: {
                        unit_ext.values[ValueTriggerDamageBaseSet] = 1;
                        unit_ext.values[ValueTriggerDamageBaseAmount] = value * 256;
                    } break;
                    case TriggerStat_SplashInner: {
                        unit_ext.values[ValueTriggerWeaponSplashInnerSet] = 1;
                        unit_ext.values[ValueTriggerWeaponSplashInnerAmount] = value;
                    } break;
                    case TriggerStat_SplashMedium: {
                        unit_ext.values[ValueTriggerWeaponSplashMediumSet] = 1;
                        unit_ext.values[ValueTriggerWeaponSplashMediumAmount] = value;
                    } break;
                    case TriggerStat_SplashOuter: {
                        unit_ext.values[ValueTriggerWeaponSplashOuterSet] = 1;
                        unit_ext.values[ValueTriggerWeaponSplashOuterAmount] = value;
                    } break;
                    case TriggerStat_TimedLife: {
                        unit->remove_timer = value;
                    } break;
                    case TriggerStat_Wireframe: {
                        unit_ext.values[ValueHasWireframeOverride] = true;
                        unit_ext.values[ValueWireframeOverride] = value;
                    } break;
                    case TriggerStat_Buttonset: {
                        unit_ext.values[ValueHasButtonsetOverride] = true;
                        unit_ext.values[ValueButtonsetOverride] = value;
                    } break;
                    case TriggerStat_HealRange: {
                        unit_ext.values[ValueTriggerHealRangeSet] = 1;
                        unit_ext.values[ValueTriggerHealRangeAmount] = value;
                    } break;
                }
            }
        } else if (parser.equals("set_unit_behavior")) {
            Designator designator = parser.get_designator();
            u32 count = parser.get_s32();
            TriggerBehavior trigger_behavior = (TriggerBehavior)parser.get_enum(trigger_behavior_names, ARRAY_SIZE(trigger_behavior_names));
            b32 enable = parser.get_s32();

            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                auto& unit_ext = unit->ext();
                switch (trigger_behavior) {
                    case TriggerBehavior_CloakProximity: {
                        unit_ext.values[ValueTriggerBehaviorProximityCloak] = enable;
                    } break;
                    case TriggerBehavior_CloakSteady: {
                        unit_ext.values[ValueTriggerBehaviorSteadyShroud] = enable;
                    } break;
                    case TriggerBehavior_Hallucination: {
                        if (enable && !(unit->status & UnitStatus::IsHallucination)) {
                            unit->status |= UnitStatus::IsHallucination;
                        } else if (!enable && unit->status & UnitStatus::IsHallucination) {
                            unit->status &= ~UnitStatus::IsHallucination;
                        }
                    } break;
                    case TriggerBehavior_DisablePassive: {
                        unit_ext.values[ValueDisablePassive] = enable != false;
                    } break;
                    case TriggerBehavior_Critter: {
                        if (unit_ext.values[ValueCritter] != enable) {
                            unit_ext.values[ValueCritter] = enable;
                            unit->order_to_idle();
                        }
                    } break;
                    case TriggerBehavior_NoCollision: {
                        if (unit_ext.values[ValueNoCollision] != enable) {
                            unit_ext.values[ValueNoCollision] = enable;
                        }
                    } break;
                    case TriggerBehavior_AirTransport: {
                        if (unit_ext.values[ValueAirTransport] != enable) {
                            unit_ext.values[ValueAirTransport] = enable;
                        }
                    } break;
                    case TriggerBehavior_NoEnergyOverlay: {
                        if (unit_ext.values[ValueNoEnergyOverlay] != enable) {
                            unit_ext.values[ValueNoEnergyOverlay] = enable;
                        }
                    } break;
                }
            }
        } else if (parser.equals("move_unit")) {
            Designator designator = parser.get_designator();
            Area area_dest = parser.get_area();
            u32 count = parser.get_s32();
            b32 overlay = parser.get_s32();

            Point16 dest = area_dest.center();
            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                Point16 origin = unit->position;
                if (!bw::teleport_to_point(unit, dest)) {
                    bw::printf("move_unit: Failed to move unit with id %u to coordinates %u, %u", unit->id, dest.x, dest.y);
                }
                if (overlay) {
                    if (!unit->get_overlay(ImageId::LegionnaireTeleportEffect)) {
                        helper::create_thingy(SpriteId::LegionnaireTeleportEffect, origin.x, origin.y, unit->player);
                        bw::add_unit_overlay(unit, ImageId::LegionnaireTeleportEffect);
                    }
                }
            }
        } else if (parser.equals("give_unit")) {
            Designator designator = parser.get_designator();
            u32 count = parser.get_s32();
            u32 target_player = parser.get_player(true);

            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                helper::give_unit(unit, target_player - 1, 1);
            }
        } else if (parser.equals("issue_order")) {
            Designator source = parser.get_designator();
            Designator target = parser.get_designator();
            u32 count = parser.get_s32();
            u32 order_id = parser.get_s32();
            u32 secondary_unit_id = parser.get_s32();

            // TODO: Handle finding & filtering target units, and actually targeting said units. -Pr0nogo
        //	EntryArray target_units = {};
        //	unit_search_rect(&target_units, area_dest.box);

            Point16 dest = target.area.center();
            for (Unit* unit : source.search({.sort = count != 0}).take(count)) {
                // TODO: Handle orders other than build / train. -Pr0nogo
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
                    case OrderId::Patrol: {
                        helper::issue_order_targeting_ground(unit, order_id, dest.x, dest.y);
                    } break;
                    case OrderId::Burrow:
                    case OrderId::Unburrow: {
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
        } else if (parser.equals("rescue_unit")) {
            Designator designator = parser.get_designator();
            u32 count = parser.get_s32();
            u32 target_player = parser.get_s32();

            for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                rescue_unit(unit, target_player - 1);
            }
        } else if (parser.equals("set_nydus_link")) {
            Designator designator_entry = parser.get_designator();
            Designator designator_exit = parser.get_designator();
            b32 enable = parser.get_s32();

            for (Unit* entry : designator_entry.search().take(1)) {
                for (Unit* exit : designator_exit.search().take(1)) {
                    if (entry && exit) {
                        if (enable) {
                            entry->building.nydus_exit = exit;
                            exit->building.nydus_exit = entry;
                        //  exit->sprite->player = entry->sprite->player;
                            entry->ext().values[ValueIsNydusEntry] = true;
                            exit->ext().values[ValueIsNydusExit] = true;
                        } else {
                            entry->building.nydus_exit = 0;
                            exit->building.nydus_exit = 0;
                            entry->ext().values[ValueIsNydusEntry] = false;
                            exit->ext().values[ValueIsNydusExit] = false;
                        }
                    }
                }
            }
        } else if (parser.equals("var") || parser.equals("timer")) {
            bool is_timer = parser.equals("timer");
            TriggerVarType var_type = is_timer ? TriggerVarType_Timer : TriggerVarType_Var;

            Token name = parser.get_string();
            Token var_operator = parser.get_string();

            if (var_operator == "=") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar& var = g_trigger_vars.find_or_create(var_type, name.sv);
                var.set(value);
            } else if (var_operator == "+=" || var_operator == "+==") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar* var = parser.token().sv.size() == 3 ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
                if (var) {
                    var->set(var->value() + value);
                }
            } else if (var_operator == "-=" || var_operator == "-==") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar* var = parser.token().sv.size() == 3 ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
                if (var) {
                    var->set(var->value() - value);
                }
            } else if (var_operator == "*=" || var_operator == "*==") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar* var = parser.token().sv.size() == 3 ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
                if (var) {
                    var->set(var->value() * value);
                }
            } else if (var_operator == "/=" || var_operator == "/==") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar* var = parser.token().sv.size() == 3 ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
                if (var) {
                    var->set(var->value() / value);
                }
            } else if (var_operator == "%=" || var_operator == "%==") {
                s32 value = parser.get_var_value(is_timer);
                TriggerVar* var = parser.token().sv.size() == 3 ? &g_trigger_vars.find_or_create(var_type, name.sv) : g_trigger_vars.find(var_type, name.sv);
                if (var) {
                    var->set(var->value() % value);
                }
            } else if (var_operator == "toggle") {
                TriggerVar& var = g_trigger_vars.find_or_create(var_type, name.sv);
                var.set(!var.value());
            } else if (var_operator == "unset") {
                g_trigger_vars.unset(var_type, name.sv);
            }
        } else if (parser.equals("bank_link_var")) {
            Token bank = parser.get_identifier();
            Token var = parser.get_identifier();
            s32 default_value = parser.get_s32();
            g_trigger_vars.bank_link_var(bank.string(), var.string(), default_value);
        } else if (parser.equals("bank_clear")) {
            Token bank = parser.get_identifier();
            g_trigger_vars.get_bank(bank.string()).clear();
        } else if (parser.equals("convert_to_ai")) {
            u32 player = parser.get_player();

            if (player) {
                player -= 1;
                g_player_table[player].type = PlayerType::Computer;
                bw::run_default_ai(player, g_start_positions[player]);
            };
        } else if (parser.equals("threshold_set_fraud_slots")) {
            PlayerBitset players = parser.get_player_bitset();

            threshold_config.enabled = true;
            for (u32 player = 0; player < PlayerLimit; ++player) {
                threshold_config.fraud_allowed[player] = players[player];
                if (players[player]) {
                    threshold_config.witness_enabled[player] = false;
                }
            }
        } else if (parser.equals("threshold_set_witness_slots")) {
            PlayerBitset players = parser.get_player_bitset();

            for (u32 player = 0; player < PlayerLimit; ++player) {
                threshold_config.witness_enabled[player] = players[player];
                if (players[player]) {
                    threshold_config.fraud_allowed[player] = false;
                }
            }
        } else if (parser.equals("threshold_autoscale")) {
            f32 value = parser.get_f32();

            threshold_config.auto_scale = value;
        } else if (parser.equals("threshold_initial_count")) {
            u32 count = parser.get_s32();

            threshold_config.initial_count = count;
        } else if (parser.equals("threshold_min_count")) {
            u32 count = parser.get_s32();

            threshold_config.min_count = count;
        } else if (parser.equals("threshold_total_count")) {
            u32 count = parser.get_s32();

            threshold_config.total_count = count;
        } else if (parser.equals("threshold_timer")) {
            u32 timer = parser.get_time_ticks();

            threshold_config.timer = timer;
        } else if (parser.equals("set_celebration")) {
            parser.get_word();
            if (parser.iequals("auto")) {
                init_auto_celebration();
            } else if (parser.iequals("none")) {
                g_globals->celebration = Celebration_None;
            } else if (parser.iequals("april_fools")) {
                g_globals->celebration = Celebration_AprilFools;
            } else if (parser.iequals("christmas")) {
                g_globals->celebration = Celebration_Christmas;
            }
        } else if (parser.equals("spawn_cargo")) {
            Designator designator = parser.get_designator();
            s32 count = parser.get_s32(0);
            u32 unit_id = parser.get_unit_id();
            s32 player = parser.get_s32(0);
            bool force = parser.get_s32(0);

            for (Unit* unit : designator.search()) {
                for (u32 i = 0; i < count; ++i) {
                    Unit* cargo_unit = bw::create_unit_in_game(unit_id, player ? player : unit->player, 0, 0);
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
        } else if (parser.equals("blissful_embrace")) {
            Designator designator = parser.get_designator();
            s32 infesting_player = parser.get_s32(0);

            for (Unit* unit : designator.search()) {
                blissful_embrace(unit, infesting_player ? infesting_player - 1 : unit->player, true);
            }
        } else if (parser.equals("set_sprite_player")) {
            const auto designator = parser.get_designator();
            const auto player = parser.get_player();
            const auto count = parser.get_count();

            if (player != -1) {
                for (Unit* unit : designator.search({.sort = count != 0}).take(count)) {
                    unit->sprite->player = player;
                }
            }
        } else if (parser.equals("witness_mode")) {
            WitnessMode new_value = (WitnessMode)parser.get_s32();

            const auto& cm_player = g_globals->players[trigger_player];
            WitnessMode old_value = cm_player.witness_mode;
            if (old_value != new_value) {
                g_globals->players[trigger_player].witness_mode = new_value;
                g_graphics.reset_stat_res_graphics(); // Must call this to reset custom graphics data for statres display. -Veeq7
                if (new_value == WitnessMode_Pure) {
                    // Hide Pure Witnesses from scoreboard
                    g_player_name_list->player[trigger_player].type = 0;
                    *g_score_screen_player_count -= 1;
                } else if (old_value == WitnessMode_Pure) {
                    // Unhide from scoreboard
                    g_player_name_list->player[trigger_player].type = g_player_table[trigger_player].type; // This stores the old player type, resets to CPU or human, etc. -Pr0nogo
                    *g_score_screen_player_count += 1;
                }

                if (new_value && g_elapsed_time_frames == 0 && trigger_player == g_local_nation_id) {
                    u32 first_player = 0;
                    for (u32 i = 0; i < PlayerLimit; ++i) {
                        if (g_globals->players[i].witness_mode) continue;
                        if (g_player_table[i].type == PlayerType::Human) {
                            first_player = i + 1;
                            break;
                        }
                    }
                    if (!first_player) {
                        for (u32 i = 0; i < PlayerLimit; ++i) {
                            if (g_globals->players[i].witness_mode) continue;
                            if (g_player_table[i].type == PlayerType::Computer) {
                                first_player = i + 1;
                                break;
                            }
                        }
                    }
                    if (first_player) {
                        first_player -= 1;
                        Point16 pos = g_start_positions[first_player];
                        pos.x = pos.x > 320 ? pos.x - 320 : 0;
                        pos.y = pos.y > 200 ? pos.y - 200 : 0;
                        helper::move_screen(pos);
                    }
                }
            }
        } else if (parser.equals("set_faction")) {
            u32 player = parser.get_player();
            const auto faction = parser.get_typed_enum<Faction>(faction_identifiers);

            if (player) {
                player -= 1;
                g_globals->players[player].faction = faction;
                g_globals->players[player].faction_was_set = true;
            }
        } else if (parser.equals("set_advisor")) {
            u32 player = parser.get_player();
            Advisor advisor = (Advisor)parser.get_enum(advisor_names, ARRAY_SIZE(advisor_names));

            if (player) {
                player -= 1;
                if (player == g_local_nation_id) {
                    g_globals->current_advisor = advisor;
                }
            }
        } else if (parser.equals("neutral_hostile")) {
            bool state = (bool)parser.get_s32();
            auto& cm_player = g_globals->players[trigger_player];
            if (cm_player.neutral_hostile != state) {
                cm_player.neutral_hostile = (bool)state;
            }
        } else if (parser.equals("set_player_team_ui")) {
            u32 player = parser.get_player();
            bool state = (bool)parser.get_s32();
            if (player) {
                player -= 1;
                auto& cm_player = g_globals->players[player];
                if (cm_player.hide_in_team_ui != state) {
                    cm_player.hide_in_team_ui = (bool)state;
                }
            }
        } else if (parser.equals("set_game_mode_race")) {
            u32 player = parser.get_player();
            GameModeRace omni_state = (GameModeRace)parser.get_enum(game_mode_race_symbol_names, ARRAY_SIZE(game_mode_race_symbol_names));

            if (player) {
                player -= 1;
                auto& cm_player = g_globals->players[player];
                cm_player.game_mode_race = omni_state;
            }
        } else if (parser.equals("set_player_build_multiplier")) {
            u32 player = parser.get_player();
            u32 multiplier = parser.get_s32();

            if (player) {
                player -= 1;
                auto& cm_player = g_globals->players[player];
                cm_player.build_multiplier = multiplier;
            }
        } else if (parser.equals("print")) {
            Token text = parser.get_string();
            bw::print(text.string().c_str());
        } else if (parser.equals("print_var")) {
            Token identifier = parser.get_identifier();

            const auto var = g_trigger_vars.find(TriggerVarType_Var, identifier.sv);
            bw::printf("Variable %s = %s", identifier.string().c_str(), var ? std::to_string(var->value()).c_str() : "unset");
        } else if (parser.equals("print_timer")) {
            Token identifier = parser.get_identifier();

            const auto timer = g_trigger_vars.find(TriggerVarType_Timer, identifier.sv);
            bw::printf("Timer %s = %s", identifier.string().c_str(), timer ? std::to_string(timer->value()).c_str() : "unset");
        } else if (parser.equals("set_cheats_allowed")) {
            const auto value = parser.get_bool();

            g_aise_state.cheats_allowed = value;
        } else if (parser.equals("set_liftoff_disabled")) {
            const auto players = parser.get_player_bitset();
            const auto value = parser.get_bool();

            // Currently only disables liftoff buttons, ergo won't work on AI and on hackers
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (!players[player]) continue;

                auto& cm_player = g_globals->players[player];
                cm_player.disable_liftoff = value;
            }
        } else if (parser.equals("set_burrow_disabled")) {
            const auto players = parser.get_player_bitset();
            const auto value = parser.get_bool();

            // Currently only disables burrow buttons, ergo won't work on AI and on hackers
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (!players[player]) continue;

                auto& cm_player = g_globals->players[player];
                cm_player.disable_burrow = value;
            }
        } else if (parser.equals("set_units_disabled")) {
            const auto players = parser.get_player_bitset();
            const auto unit_ids = parser.get_unit_id_bitset();

            // Currently only disables train buttons, ergo won't work on AI and on hackers
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (!players[player]) continue;

                auto& cm_player = g_globals->players[player];
                cm_player.disabled_units = unit_ids;
            }
        } else if (parser.equals("disable_units")) {
            const auto players = parser.get_player_bitset();
            const auto unit_ids = parser.get_unit_id_bitset();
            const auto disable = parser.get_bool(true);

            // Currently only disables train buttons, ergo won't work on AI and on hackers
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (!players[player]) continue;

                auto& cm_player = g_globals->players[player];
                if (disable) {
                    cm_player.disabled_units |= unit_ids;
                } else {
                    cm_player.disabled_units &= ~unit_ids;
                }
            }
        } else if (parser.equals("announce_player_defeat")) {
            const auto players = parser.get_player_bitset();
            const auto bool_state = parser.get_bool(true);
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (!players[player]) continue;

                AnnounceDefeatCommand command = {
                    command.type = GameCommand_Custom,
                    command.subtype = NewGameCommand::AnnounceDefeat,
                    command.player = player,
                    command.show_player_number = bool_state,
                };
                helper::queue_game_command((u8*)&command, sizeof(command));
            }
        } else if (parser.equals("table_edit")) {
            TableEditor(parser).run();
        } else if (parser.equals("move_location")) {
            const auto location = parser.get_location();
            const auto designator = parser.get_designator();
            const auto offset_left   = parser.get_s32();
            const auto offset_top    = parser.get_s32();
            const auto offset_right  = parser.get_s32();
            const auto offset_bottom = parser.get_s32();
            if (!location) {
                parser.error("Invalid location");
                continue;
            }
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
        } else if (parser.equals("recreate_units")) {
            const auto designator = parser.get_designator();
            for (Unit* unit : designator.search()) {
                const auto unit_id = unit->id;
                const auto player = unit->player;
                const auto pos = unit->position;
                const bool had_ai = unit->unit_ai;
                unit->silent_remove();

                bw::create_unit_in_game(unit_id, player, pos.x, pos.y, nullptr, had_ai);
            }
        }
    }
    return true;
}

void Globals::sample_eco() {
    EcoMeasurements eco{};
    for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
        if (unit->player > eco.size()) {
            continue;
        }
        UnitExt* unit_ext = get_unit_ext(unit);
        eco[unit->player] += unit->total_cost();
    }
    eco_measurements.push_back(eco);
}

f32 Globals::calculate_hype() const {
    const auto count = eco_measurements.size();
    std::vector<f32> values(count);
    for (size_t i = 0; i < values.size(); i++) {
        values[i] = log(std::accumulate(eco_measurements[i].begin(), eco_measurements[i].end(), 0));
    }

    f32 changes_mean = 0;
    std::vector<f32> changes(count-1);
    for (size_t i = 0; i < changes.size(); i++) {
        changes[i] = values[i] = values[i];
        changes_mean += changes[i];
    }
    changes_mean /= count;

    f32 volatility = 0;
    for (f32 change : changes) {
        f32 mean_diff = change - changes_mean;
        volatility += mean_diff * mean_diff;
    }
    volatility = sqrt(volatility / changes.size());

    f32 sum_x = 0;
    f32 sum_y = 0;
    f32 sum_xx = 0;
    f32 sum_xy = 0;
    for (size_t x = 0; x < count; x++) {
        f32 y = values[x];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }
    f32 slope = (count * sum_xy - sum_x * sum_y) / (count * sum_xx - sum_x * sum_x);

    return slope * (1 + volatility);
}
