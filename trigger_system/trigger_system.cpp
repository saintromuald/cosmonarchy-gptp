#include "trigger_system.h"
#include <limits.h>
#include <time.h>

#include "triggers.h"
#include "hooks/interface/uifeatures/hotkeys.h"

#include <ranges>

void init_auto_celebration() {
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
        case RaceId::Askosi: {
            advisors.insert_range(advisors.end(), g_config.askosi_advisors);
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
    init_spawn();
    g_trigger_vars.reset();
    AudioSystem::instance().reset();
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
                case UnitId::ResourceMineralField:		 return SpriteId::Christmas_MineralField1;
                case UnitId::ResourceMineralFieldType2:	 return SpriteId::Christmas_MineralField2;
                case UnitId::ResourceMineralFieldType3:	 return SpriteId::Christmas_MineralField3;
                case UnitId::TerranMinistry:			 return SpriteId::Christmas_CommandCenter;
                case UnitId::ProtossNexus:				 return SpriteId::Christmas_Nexus;
                case UnitId::ZergHachirosk:				 return SpriteId::Christmas_Hachirosk;
                case UnitId::ZergCaphrolosk:			 return SpriteId::Christmas_Caphrolosk;
                case UnitId::ZergGathtelosk:			 return SpriteId::Christmas_Gathtelosk;
                case UnitId::AskosiLandlock:             return SpriteId::Christmas_Landlock;
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
    if (!unit->is_completed()) {
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

        auto& unit_ext = unit->ext();
        unit_ext.values[ValueCustomImage] = image_id + 1;
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
        AudioSystem::instance().update();
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
            if (!g_globals->alert.play_timer && !g_use_quiet_sounds) {
                bw::play_sound(g_globals->alert.sound_id);
            }
        }

        if (g_globals->advisor_state.cooldown) {
            g_globals->advisor_state.cooldown--;
        }
        if (g_globals->advisor_state.play_timer) {
            g_globals->advisor_state.play_timer--;
            if (!g_globals->advisor_state.play_timer && !g_use_quiet_sounds) {
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
            TriggerVar& var = g_trigger_vars.find_or_create(TriggerVarType_Var, button.id);
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

        const auto& text = button.display_name;
        graphics::FontSize font_size = graphics::FONT_MEDIUM;

        graphics::Font* font = font_base[font_size];
        s32 w = font->getTextWidth(text.c_str());
        s32 h = font->getTextHeight(text.c_str());
        g_graphics.draw_text(rect.x + rect.w / 2 - w / 2, rect.y + rect.h / 2 - h / 2, text, font_size); // TODO: center
    }
}

b32 parse_comment_condition(TriggerAction* action) {
    trigger::Context ctx{trigger::g_triggers, action, g_current_trigger_node->trigger, (u32)*g_current_trigger_player_id};
    return ctx.evaluate_comment_conditions();
}

b32 __fastcall parse_comment_action(TriggerAction* action) {
    trigger::Context ctx{trigger::g_triggers, action, g_current_trigger_node->trigger, (u32)*g_current_trigger_player_id};
    return ctx.evaluate_comment_actions();
}

void Globals::sample_eco() {
    EcoMeasurements eco{};
    for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
        if (unit->player > eco.size()) {
            continue;
        }
        UnitExt* unit_ext = get_unit_ext(unit);
        eco[unit->player] += f32(unit->total_cost());
    }
    eco_measurements.push_back(eco);
}

f32 Globals::calculate_hype() const {
    const auto count = eco_measurements.size();
    std::vector<f32> values(count);
    for (size_t i = 0; i < values.size(); i++) {
        values[i] = log(std::accumulate(eco_measurements[i].begin(), eco_measurements[i].end(), 0));
    }

    auto changes_mean = 0.0f;
    std::vector<f32> changes(count - 1);
    for (size_t i = 0; i < changes.size(); i++) {
        changes[i] = values[i] = values[i];
        changes_mean += changes[i];
    }
    changes_mean /= count;

    f32 volatility = 0;
    for (auto change : changes) {
        auto mean_diff = change - changes_mean;
        volatility += mean_diff * mean_diff;
    }
    volatility = sqrt(volatility / changes.size());

    f32 sum_x = 0;
    f32 sum_y = 0;
    f32 sum_xx = 0;
    f32 sum_xy = 0;
    for (size_t x = 0; x < count; x++) {
        auto y = values[x];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }
    f32 slope = (count * sum_xy - sum_x * sum_y) / (count * sum_xx - sum_x * sum_x);

    return slope * (1 + volatility);
}
