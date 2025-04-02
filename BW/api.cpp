#include <algorithm>
#include <cassert>
#include <utility>
#include <string>
#include <random>

#include "api.h"
#include "bwdata.h"
#include "BW/UnitFinder.h"
#include "hooks/cosmonarchy_finder.h"
#include "hooks/weapons/weapon_damage.h"
#include "globals.h"
#include "hooks/target_related.h"
#include "hooks/unit_ai.h"
#include "ai_layouts.h"
#include "config.h"
#include "hooks/limits/sfx_extender.h"
#include "hooks/update_unit_sight.h"
#include "hooks/rally_point.h"
#include "hooks/interface/uifeatures/hotkeys.h"
#include "trigger_system/trigger_sound.h"
#include "hooks/unit_stats/weapon_range.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <hooks\unit_movement.h>

bool __assert_sprite(Sprite* sprite, const char* function, const char* file, u64 line) {
    if (!sprite) {
        bw::printf("Sprite Nullcheck failed at %s@%s:%llu", function, file, line);
        return false;
    }
    return true;
}

namespace bw {
std::filesystem::path get_starcraft_path() {
    wchar_t buffer[MAX_PATH] = {};
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

bool is_witness() {
    return !bw::in_replay() && g_globals->players[g_local_nation_id].witness_mode;
}

Bullet* artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, Point16 target_pt, Point16 origin_pt) {
    Unit* target_unit = source->order_target.unit;
    Point16 target_pos = source->order_target.pt;
    source->order_target.unit = target;
    source->order_target.pt = target_pt;
    if (target && target_unit) {
        source->order_target.pt = target_unit->position;
    } else {
        source->order_target.pt = target_pt;
    }
    Point16 bullet_source_pos = {source->position.x, source->position.y};
    if (origin_pt.x != 0 || origin_pt.y != 0) {
        bullet_source_pos = origin_pt;
    }
    Bullet* bullet = helper::create_bullet(weapon_id, source, bullet_source_pos.x, bullet_source_pos.y, player,
        get_angle(bullet_source_pos.x, bullet_source_pos.y, source->order_target.pt.x, source->order_target.pt.y));
    source->order_target.unit = target_unit;
    source->order_target.pt = target_pos;
    return bullet;
}

Bullet* artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, u16 x, u16 y) {
    return artificial_bullet_from(weapon_id, player, source, target, {x, y});
}

void teleport_internal(Unit* unit, Point16 target, u16 sprite_id, bool preserve_order) {
    // Finish the teleporting (and cut the recalled unit from a possible unit targeted by an order)
    bw::prepare_unit_move(unit, false);
    if (!preserve_order) {
        unit->order_target.unit = 0;
    }
    helper::move_unit(unit, target.x, target.y);
    helper::refresh_unit_after_move(unit);

    // Stop some currently active orders
    if (unit->id != UnitId::ZergCocoon && !preserve_order) {
        unit->order_computer(units_dat::return_to_idle_order[unit->id]);
    }

    // Create the Dimensional Recall animation over the unit
    if (sprite_id) {
        Thingy* recall_effect_sprite = helper::create_thingy(sprite_id, target.x, target.y, 0);
        if (recall_effect_sprite) {
            recall_effect_sprite->sprite->elevation = unit->sprite->elevation + 1;
            helper::set_thingy_visibility_flags(recall_effect_sprite);
        }
    }

    // Disconnect a nuke from a unit using it
    if (bw::unit_can_nuke(unit)) {
        if (unit->connected_unit && unit->connected_unit->id == UnitId::TerranMorningstar) {
            unit->connected_unit->connected_unit = 0;
            unit->connected_unit = 0;
        }
    }
}

bool teleport_to_point(Unit* unit, Point16 target, u16 sprite_id, bool preserve_order) {
    Point16 new_pos = {};
    if (!bw::check_unit_collision_pos(unit, &target, &new_pos, 0, false, 0)) {
        return false;
    }

    teleport_internal(unit, new_pos, sprite_id, preserve_order);
    return true;
}

bool teleport_to_unit(Unit* unit, Unit* target, u32 radius, u16 sprite_id, bool preserve_order) {
    Box16 move_area = {};
    move_area.left = target->get_left() - radius;
    move_area.top = target->get_top() - radius;
    move_area.right = target->get_right() + radius;
    move_area.bottom = target->get_bottom() + radius;
    move_area = clamp_box16(move_area);

    Point16 new_pos = {};
    if (!bw::check_unit_collision_pos(unit, &unit->position, &new_pos, &move_area, false, 0)) {
        return false;
    }
    if (target->get_distance_to_point(new_pos) > radius * 1.5) {
        return false;
    }

    teleport_internal(unit, new_pos, sprite_id, preserve_order);
    return true;
}

Thingy* create_and_init_thingy(u32 spriteId, s16 x, s16 y, u32 player, s32 elevation_level) {
    Thingy* thingy = helper::create_thingy(spriteId, x, y, player);
    if (thingy) {
        thingy->sprite->elevation = elevation_level;
        helper::set_thingy_visibility_flags(thingy);
    }
    return thingy;
}

void print(const char* text, TextColor color) {
    const u32 Func_PrintText = 0x0048CD30;
    if (!text) {
        return;
    }

    DWORD gtc = GetTickCount() + 7000;
    __asm {
        PUSHAD
        PUSH 0;//unknown
        MOV eax, text
            PUSH gtc
            PUSH color
            CALL Func_PrintText
            POPAD
    }
}

void printf(const char* format, ...) {
    char buf[256] = {};

    va_list _ArgList;
    va_start(_ArgList, format);
    vsnprintf(buf, sizeof(buf), format, _ArgList);
    __crt_va_end(_ArgList);

    print(buf);
}

void printf(TextColor color, const char* format, ...) {
    char buf[256] = {};

    va_list _ArgList;
    va_start(_ArgList, format);
    vsnprintf(buf, sizeof(buf), format, _ArgList);
    va_end(_ArgList);

    print(buf, color);
}

void printf_console(const char* format, ...) {
    char buf[1025] = {};

    va_list _ArgList;
    va_start(_ArgList, format);
    vsnprintf(buf, sizeof(buf), format, _ArgList);
    va_end(_ArgList);

    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}

//-------- Unit checks --------//

// Identical to function @ 0x00475CE0
bool can_weapon_target_unit(u8 weapon_id, Unit* target, Unit* attacker) {
    if (weapon_id >= WeaponsIdLimit) {
        return false;
    }
    if (!target) {
        return weapons_dat::target_flags[weapon_id].terrain;
    }
    if (target->status & UnitStatus::Invincible) {
        return false;
    }

    const TargetFlag target_flags = weapons_dat::target_flags[weapon_id];
    const u32 targetProps = units_dat::base_property[target->id];
    if (target->status & UnitStatus::InAir ? !target_flags.air : !target_flags.ground) {
        return false;
    }
    if (target_flags.mechanical && !(targetProps & UnitProperty::Mechanical)) {
        return false;
    }
    if (target_flags.organic && !(targetProps & UnitProperty::Organic)) {
        return false;
    }
    if (target_flags.non_building && targetProps & UnitProperty::Building) {
        return false;
    }
    if (target_flags.non_robotic && targetProps & UnitProperty::RoboticUnit) {
        return false;
    }
    if (target_flags.organic_or_mechanical && !(targetProps & (UnitProperty::Organic | UnitProperty::Mechanical))) {
        return false;
    }
    if (target_flags.player_owned && target->player != attacker->player) {
        return false;
    }
    return true;
}

bool is_under_carapace_swarm(Unit* unit) {
    return !(unit->status & UnitStatus::InAir) && get_carapace_swarm_unit(unit);
}

bool is_dark_swarm(Unit* unit) {
    return unit->id == UnitId::Spell_CarapaceSwarm || unit->id == UnitId::Spell_CarapaceSwarmSmall;
}

Unit* get_carapace_swarm_unit(Unit* unit) {
    if (unit->is_dead()) return 0;
    static UnitFinder dark_swarm_finder;
    dark_swarm_finder.search(unit->get_left(), unit->get_top(), unit->get_right(), unit->get_bottom());
    return dark_swarm_finder.get_first(is_dark_swarm);
}

Unit* get_dark_swarm_from_position(u16 x, u16 y, u16 radius) {
    static UnitFinder dark_swarm_finder;
    dark_swarm_finder.search(x - radius, y - radius, x + radius, y + radius);
    return dark_swarm_finder.get_first(is_dark_swarm);
}

//-------- Graphics and geometry --------//

// Improved code from BWAPI's include/BWAPI/Position.h: getApproxDistance()
// Logically same as function @ 0x0040C360
// x1 = ECX;x2 = EDX;y1 = [EBP+08];y2 = [EBP+0C];
u32 distance_between(s32 x1, s32 y1, s32 x2, s32 y2) {
    s32 dMax = abs(x1 - x2), dMin = abs(y1 - y2);
    if (dMax < dMin) {
        std::swap(dMax, dMin);
    }
    if (dMin <= (dMax >> 2)) {
        return dMax;
    }
    return (dMin * 3 >> 3) + (dMin * 3 >> 8) + dMax - (dMax >> 4) - (dMax >> 6);
}

u32 distance_manhattan_between(s32 x1, s32 y1, s32 x2, s32 y2) {
    return std::max(abs(x1 - x2), abs(y1 - y2));
}

// Identical to function @ 0x00494C10
s32 arc_tangent(s32 slope) {
    const u32 tangentTable[] = {
             7,	 13,	 19,	 26,	 32,	 38,	 45,	 51,
            58,	 65,	 71,	 78,	 85,	 92,	 99,	107,
         114,	122,	129,	137,	146,	154,	163,	172,
         181,	190,	200,	211,	221,	233,	244,	256,
         269,	283,	297,	312,	329,	346,	364,	384,
         405,	428,	452,	479,	509,	542,	578,	619,
         664,	716,	775,	844,	926, 1023, 1141, 1287,
        1476, 1726, 2076, 2600, 3471, 5211, 10429, -1
    };

    bool isNegative = false;
    if (slope < 0) {
        isNegative = true;
        slope = -slope;
    }

    s32 min = 0, max = 64, angle = 32;
    do {
        if ((u32) slope <= tangentTable[angle]) {
            max = angle;
        } else {
            min = angle + 1;
        }
        angle = (min + max) / 2;
    } while (min != max);
    return (isNegative ? -angle : angle);
}

// Identical to function @ 0x00495300
//					EDX		[EBP+08]	ECX			EAX
s32 get_angle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd) {
    s32 dx = xEnd - xStart, dy = yEnd - yStart;
    if (dx == 0) {
        return dy > 0 ? 128 : 0;
    }

    s32 angle = arc_tangent((dy << 8) / dx);
    if (dx < 0) {
        angle += 192;
        return angle == 256 ? 0 : angle;
    } else {
        return angle + 64;
    }
}

Point16 rotate_point(Point16 anchor, Point16 point, f64 angle_degrees) {
    f64 x1 = point.x - anchor.x;
    f64 y1 = point.y - anchor.y;
    f64 radians = angle_degrees / 57.296;
    f64 x2 = x1 * cos(radians) - y1 * sin(radians);
    f64 y2 = x1 * sin(radians) + y1 * cos(radians);
    return {u16(x2 + anchor.x), u16(y2 + anchor.y)};
}

Vec32 rotate_point(Vec32 anchor, Vec32 point, f64 angle_degrees) {
    f64 x1 = point.x - anchor.x;
    f64 y1 = point.y - anchor.y;
    f64 radians = angle_degrees / 57.296;
    f64 x2 = x1 * cos(radians) - y1 * sin(radians);
    f64 y2 = x1 * sin(radians) + y1 * cos(radians);
    Vec32 product = {};
    product.x = x2 + anchor.x;
    product.y = y2 + anchor.y;
    return {s32(x2 + anchor.x), s32(y2 + anchor.y)};
}

Vec32f rotate_point(Vec32f anchor, Vec32f point, f64 angle_degrees) {
    f64 x1 = point.x - anchor.x;
    f64 y1 = point.y - anchor.y;
    f64 radians = angle_degrees / 57.296;
    f64 x2 = x1 * cos(radians) - y1 * sin(radians);
    f64 y2 = x1 * sin(radians) + y1 * cos(radians);
    Vec32f product = {};
    product.x = x2 + anchor.x;
    product.y = y2 + anchor.y;
    return {f32(x2 + anchor.x), f32(y2 + anchor.y)};
}

f64 bw_angle_to_normal(s32 bw_angle) {
    return (f64)bw_angle * 1.40625;
}

s32 degrees_to_bw_angle(f64 degrees) {
    return degrees / 1.40625;
}

s32 get_polar_x(s32 distance, u8 angle) {
    return distance * g_angle_distance[angle].x >> 8;
}

s32 get_polar_y(s32 distance, u8 angle) {
    return distance * g_angle_distance[angle].y >> 8;
}

Point16 polar_point(Point16 center, f32 rads, f32 distance) {
    return {
        static_cast<u16>(center.x + cosf(rads) * distance),
        static_cast<u16>(center.y + sinf(rads) * distance),
    };
}

Point16 random_polar_point(Point16 center, f32 max_distance) {
    return polar_point(center, bw::random_float() * 2 * PI, max_distance * sqrt(bw::random_float()));
}

//-------- Player information --------//

s32 get_supply_remaining(u8 player, u8 raceId) {
    assert(raceId <= 2);
    assert(player < PLAYER_COUNT);

    s32 supply_provided;
    if (is_cheat_enabled(CheatFlags::FoodForThought)) {
        supply_provided = g_race_supply[RaceId::Terran].max[player];
    } else {
        supply_provided = std::min(g_race_supply[RaceId::Terran].max[player], g_race_supply[RaceId::Terran].provided[player]);
    }
    return supply_provided - g_race_supply[RaceId::Terran].used[player];
}

bool has_tech_researched(u8 player, u16 techId) {
    assert(player < PLAYER_COUNT);
    assert(techId != TechId::None);
    if (techId < TechId::Restoration) {
        return TechSc->is_researched[player][techId] != 0;
    } else {
        return TechBw->is_researched[player][techId - TechId::Restoration] != 0;
    }
}

bool has_tech_available(u8 player, u16 techId) {
    assert(player < PLAYER_COUNT);
    assert(techId != TechId::None);
    if (techId < TechId::Restoration) {
        return TechSc->is_enabled[player][techId] != 0;
    } else {
        return TechBw->is_enabled[player][techId - TechId::Restoration] != 0;
    }
}

void set_tech_research_state(u8 player, u16 techId, bool is_researched) {
    assert(player < PLAYER_COUNT);
    assert(techId != TechId::None);

    if (techId < TechId::Restoration) {
        TechSc->is_researched[player][techId] = is_researched;
    } else {
        TechBw->is_researched[player][techId - TechId::Restoration] = is_researched;
    }
}

const u32 Func_DoWeaponDamage = 0x00479930;
void send_gptp_aise_command(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2, u32 v3) {
    _asm {
        PUSHAD
        MOV EAX, v3
        MOV EDI, unit2
        PUSH v2
        PUSH unit1
        PUSH 50000
        PUSH command
        PUSH v1
        CALL Func_DoWeaponDamage
        POPAD
    }
}

/*
u32 get_flingy_data(u32 entry, u32 index) {
    return get_aise_value(nullptr, nullptr, AiseId::GetFlingyData, entry, index);
}

u32 get_sprite_data(u32 entry, u32 index) {
    return get_aise_value(nullptr, nullptr, AiseId::GetSpriteData, entry, index);
}

u32 get_image_data(u32 entry, u32 index) {
    return get_aise_value(nullptr, nullptr, AiseId::GetImageData, entry, index);
}

u32 get_unit_data(u32 entry, u32 index) {
    return get_aise_value(nullptr, nullptr, AiseId::GetUnitData, entry, index);
}
*/

bool is_larval_egg(Unit* unit) {
    if (unit->id == UnitId::ZergEgg) {
        u8 origin = unit->create_origin;
        if (origin == UnitId::ZergLarva || origin == UnitId::ZergOcsalLarva) {
            return true;
        }
    }
    return false;
}

bool is_zerg_egg_new(Unit* unit, bool allow_larval_eggs, bool allow_larvae, u16 explicit_unit_id) {
    u32 id = UnitId::None;
    if (explicit_unit_id != UnitId::None) {
        id = explicit_unit_id;
    } else if (unit) {
        id = unit->id;
    }
    switch (id) {
        case UnitId::ZergEgg: {
            if (unit && !allow_larval_eggs && (unit->create_origin == UnitId::ZergLarva || unit->create_origin == UnitId::ZergOcsalLarva)) {
                return false;
            }
            return true;
        }
        case UnitId::ZergCocoon: {
            return true;
        }
    }

    if (allow_larvae && (id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva)) {
        return true;
    }
    return false;
}

bool is_zerg_egg(u16 id, bool allow_normal_eggs, bool allow_larvae) {
    switch (id) {
        case UnitId::ZergCocoon: {
            return true;
        }
    }
    if ((id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva) && allow_larvae) {
        return true;
    }
    if (id == UnitId::ZergEgg && allow_normal_eggs) {
        return true;
    }
    return false;
}

bool is_conventional_unit_morph_result(u16 id) {
    switch (id) {
        case UnitId::ZergGosvileth:
        case UnitId::ZergIroleth:
        case UnitId::ZergVorvrokor:
        case UnitId::ZergLiiralisk:
        case UnitId::ZergKalkalisk:
        case UnitId::ZergSovroleth:
        case UnitId::ZergGhitorokor:
        case UnitId::ZergBactalisk:
        case UnitId::ZergTethzorokor:
        case UnitId::ZergVithrilisk:
        case UnitId::ZergUltrokor:
        case UnitId::ZergZobriolisk:
        case UnitId::ZergZarcavrokor:
        case UnitId::ZergIsthrathaleth:
        case UnitId::ZergAkistrokor:
        case UnitId::ZergAlkajelisk:
        case UnitId::ZergOgroszaleth: {
            return true;
        }
    }
    return false;
}

bool is_any_morph_result(u16 id) {
    if (is_conventional_unit_morph_result(id)) {
        return true;
    }
    return false;
}

bool is_conventional_unit_morph_source(u16 id, bool allow_larvae) {
    switch (id) {
        case UnitId::ZergDroleth:
        case UnitId::ZergOvileth:
        case UnitId::ZergOthstoleth:
        case UnitId::ZergZethrokor:
        case UnitId::ZergQuazilisk:
        case UnitId::ZergNathrokor:
        case UnitId::ZergNathrelisk:
        case UnitId::ZergCikralisk:
        case UnitId::ZergProtathalor:
        case UnitId::ZergMutalisk:
        case UnitId::ZergAxitrilisk:
        case UnitId::ZergMortothrokor:
        case UnitId::ZergHydralisk:
        case UnitId::ZergHydraliskOrkoti:
        case UnitId::ZergIzirokor:
        case UnitId::ZergIziralisk:
        case UnitId::ZergSkithrokor:
        case UnitId::ZergKagralisk:
        case UnitId::ZergEvigrilisk:
        case UnitId::ZergAlmaksalisk:
        case UnitId::ZergKeskathalor:
        case UnitId::ZergGeszithalor:
        case UnitId::ZergZoryusthaleth:
        case UnitId::ZergInzorthrokor:
        case UnitId::ZergLakizilisk:
        case UnitId::ZergGorgrokor:
        case UnitId::ZergKonvilisk:
        case UnitId::ZergVilgorokor:
        case UnitId::ZergMatraleth:
        case UnitId::ZergIsthrathaleth:
        case UnitId::ZergElthrazalisk: {
            return true;
        }
    }
    if ((id == UnitId::ZergLarva || id == UnitId::ZergOcsalLarva) && allow_larvae) {
        return true;
    }
    return false;
}

bool is_zerg_building_morph_result(u16 id) {
    switch (id) {
        case UnitId::ZergCaphrolosk:
        case UnitId::ZergGathtelosk:
        case UnitId::ZergEzigrisant:
        case UnitId::ZergSurkithCesiant:
        case UnitId::ZergSpraithCesiant:
        case UnitId::ZergLarvoskCesiant:
        case UnitId::ZergKazitrithCesiant:
        case UnitId::ZergSkortrithCesiant:
        case UnitId::ZergSpirtithCesiant:
        case UnitId::ZergNydusCesiant:
        case UnitId::ZergOcsalCesiant:
        case UnitId::ZergOthstolOviform:
        case UnitId::ZergVornathPond:
        case UnitId::ZergKalkiirLake:
        case UnitId::ZergTosvrolHaunt:
        case UnitId::ZergSkibactScab:
        case UnitId::ZergTethvithSwamp:
        case UnitId::ZergChokralChazsal:
        case UnitId::ZergAlmakisAntre:
        case UnitId::ZergAlkajChasm:
        case UnitId::ZergUltravCavern: {
            return true;
        }
    }
    return false;
}

bool is_zerg_building_morph_source(u16 id) {
    switch (id) {
        case UnitId::ZergHachirosk:
        case UnitId::ZergExcisant:
        case UnitId::ZergCagrantCesiant:
        case UnitId::ZergSurkithCesiant:
        case UnitId::ZergSpraithCesiant:
        case UnitId::ZergLarvoskCesiant:
        case UnitId::ZergIrolIris:
        case UnitId::ZergQuazethPool:
        case UnitId::ZergVornathPond:
        case UnitId::ZergHydrithDen:
        case UnitId::ZergSkibactScab:
        case UnitId::ZergAlkagIteth:
        case UnitId::ZergMutathSpire:
        case UnitId::ZergMatravilNest:
        case UnitId::ZergZorkizShroud:
        case UnitId::ZergGeszkathGrotto: {
            return true;
        }
    }
    return false;
}

std::string read_samase_file(char* name) {
    const char* data = 0;
    u32 len = 0;
    auto data_ptr = &data;
    auto len_ptr = &len;
    u32 val = reinterpret_cast<u32>(name);
    auto length = bw::get_aise_value(0, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
    char* buf = (char*) bw::get_aise_value(0, 0, AiseId::ReadSamaseFile_Ptr, 0, 0);
    std::string str;
    if (length == 0) {
        return str;
    } else {
        str = std::string(buf, length);
    }
    return str;
}

s32 get_generic_variable(u32 id) {
    return get_aise_value(0, 0, AiseId::GenericVariable, id, 0);
}

Unit* get_generic_rust_unit(Unit* unit, u32 id) {
    u32 val = get_aise_value(unit, 0, AiseId::GenericUnit, id, 0);
    return reinterpret_cast<Unit*>(val);
}

void set_generic_rust_unit(Unit* unit, u32 id, Unit* targ) {
    bw::send_gptp_aise_command(unit, targ, GptpId::SetGenericUnit, id, 0, 0);
}

void set_generic_variable(u32 id, s32 val) {
    bw::send_gptp_aise_command(0, 0, GptpId::SetGenericVariable, id, val, 0);
}

u32 get_generic_rust_value(Unit* u, u32 val) {
    return get_aise_value(u, 0, AiseId::GenericValue, val, 0);
}

void set_generic_rust_value(Unit* u, u32 id, u32 val) {
    bw::send_gptp_aise_command(u, 0, GptpId::SetGenericValue, id, val, 0);
}

void set_sprite_data(u32 entry, u32 index, u32 value) {
    return send_gptp_aise_command(0, 0, GptpId::SetSpriteData, entry, index, value);
}

const u32 Func_CanTargetSpell = 0x00492020;
u32 get_aise_value(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2) {
    u32 result = 0;
    u32 v3 = 0; // pointer
    _asm {
        PUSHAD
        MOV ESI, v2 // fow_unit_id
        MOV EDI, unit2 // target
        MOV EDX, 50000 // tech
        PUSH v3 // outerror
        PUSH v1 // y
        PUSH command // x
        PUSH unit1 // unit
        CALL Func_CanTargetSpell
        MOV result, EAX
        POPAD
    }
    return result;
}

const u32 Func_PsiPlacement = 0x00473920;
u8 get_upgrade_level(u8 player, u8 upgradeId) {
    assert(player < PLAYER_COUNT);
    //		assert(upgradeId < UpgradeId::None);
    assert(upgradeId < 256);

    if (upgradeId < UpgradeId::PrismaticShield) {
        return UpgradesSc->current_level[player][upgradeId];
        /*
        } else if (upgradeId<UpgradeId::None) {
            return UpgradesBw->current_level[player][upgradeId - UpgradeId::PStructurePlating];
        */
    } else {
        u32 result = 0;
        u32 player_u32 = player;
        u32 upgrade_u32 = upgradeId;
        _asm {
            PUSHAD
            MOV ESI, upgrade_u32
            MOV EAX, player_u32
            PUSH 0
            PUSH 50000
            CALL Func_PsiPlacement
            MOV result, EAX
            POPAD
        }
        return result;
    }
}
// EBX = max/current upgrade read, ECX - player, EDI - id

u8 get_upgrade_max_level(u8 player, u8 upgradeId) {
    assert(player < PLAYER_COUNT);
    //		assert(upgradeId < UpgradeId::None);
    assert(upgradeId < 256);
    if (upgradeId < UpgradeId::PrismaticShield) {
        return UpgradesSc->max_level[player][upgradeId];
        /*
        } else if (upgradeId < UpgradeId::None) {
            return UpgradesBw->max_level[player][upgradeId - UpgradeId::PStructurePlating];
        */
    } else {
        u32 result = 0;
        u32 player_u32 = player;
        u32 upgrade_u32 = upgradeId;
        _asm {
            PUSHAD
            MOV ESI, upgrade_u32
            MOV EAX, player_u32
            PUSH 1
            PUSH 50000
            CALL Func_PsiPlacement
            MOV result, EAX
            POPAD
        }
        return result;
    }
}

void set_upgrade_level(u8 player, u8 upgradeId, u8 level) {
    assert(player < PLAYER_COUNT);
    assert(upgradeId < UpgradeId::None);
    if (upgradeId < UpgradeId::PrismaticShield) {
        UpgradesSc->current_level[player][upgradeId] = level;
    } else {
        UpgradesBw->current_level[player][upgradeId - UpgradeId::PrismaticShield] = level;
    }
}

void adjust_addon_x(u16 addonId, u16 sourceId, u16* x) {
    if (addonId == UnitId::TerranQuarry) {
        *x = 128;
    }
}

void adjust_addon_y(u16 addonId, u16 sourceId, u16* y) {
    if (addonId == UnitId::TerranQuarry) {
        *y = 32;
    }
}

void adjust_addon_coordinates(u16 addonId, u16 sourceId, u16* x, u16* y) {
    adjust_addon_x(addonId, sourceId, x);
    adjust_addon_y(addonId, sourceId, y);
}

//-------- Moving Units --------//

bool move_unit(Unit* unit, s16 x, s16 y) {
    Point16 targetPos = {x, y};
    Point16 actualPos = {};
    Point16 prevPos = unit->sprite->position;

    // Based on the code for DimensionalRecall/Nydus Canal/Move Unit trigger
    helper::move_unit(unit, x, y);

    if (check_unit_collision_pos(unit, &targetPos, &actualPos)) {
        prepare_unit_move(unit);
        helper::move_unit(unit, actualPos.x, actualPos.y);
        helper::refresh_unit_after_move(unit);
        return true;
    } else {
        helper::move_unit(unit, prevPos.x, prevPos.y);
        return false;
    }
}

bool move_unit_force(Unit* unit, s16 x, s16 y) {
    prepare_unit_move(unit);
    hooks::move_unit(unit, x, y, true);
    return true;
}

bool move_unit_improved(Unit* unit, s16 x, s16 y) {
    if (!(unit->status & UnitStatus::InAir)) {
        ActiveTile* original_tile = bw::get_active_tile_at_coords(unit->position.x, unit->position.y);
        ActiveTile* final_tile = bw::get_active_tile_at_coords(x, y);

        if (final_tile->is_unwalkable || original_tile->ground_height != final_tile->ground_height) {
            return false;
        }

        Unit* collision = bw::UnitFinder(
            x - units_dat::unit_bounds[unit->id].left,
            y - units_dat::unit_bounds[unit->id].top,
            x + units_dat::unit_bounds[unit->id].right,
            y + units_dat::unit_bounds[unit->id].bottom
        ).get_first(cosmonarchy_finder_match(unit, Flags::NotSelf | Flags::NoAir, {}));

        if (collision) {
            return false;
        }
    }

    prepare_unit_move_custom(unit);
    helper::move_unit(unit, x, y);
    helper::refresh_unit_after_move(unit);
    return true;
}

void prepare_unit_move_custom(Unit* unit) {
    helper::sort_unit(unit);
    helper::refresh_grp_image(unit->sprite);
    if (unit->subunit) {
        helper::refresh_grp_image(unit->subunit->sprite);
    }

    helper::toggle_unit_path(unit);
    u8 pathing_flags = unit->pathing_flags;
    unit->movement_state = 0;
    unit->pathing_flags = (pathing_flags ^ (unit->sprite->elevation < 12)) & 1 ^ pathing_flags;
}

//-------- Utility functions --------//

Unit* create_unit_in_game(u16 unit_id, u8 player, u32 x, u32 y, Unit* creator, bool init_ai, bool give_massacre_resource) {
    if (*g_unit_count >= UnitLimit) {
        return 0; // Prevent crashes, move to create unit later. -Veeq7
    }
    Point16 pos = {x, y};
    helper::fix_target_location(&pos, unit_id);
    Unit* unit = helper::create_unit(unit_id, pos.x, pos.y, player);
    if (!unit) {
        helper::display_last_network_error_for_player(player);
        return 0;
    }
    if (!give_massacre_resource) {
        UnitExt* unit_ext = get_unit_ext(unit);
        unit_ext->values[ValueNoMassacreResource] = 1;
    }

    helper::update_unit_stats_and_finish_building(unit);
    helper::update_unit_strength(unit);
    if (init_ai) {
        bw::give_ai_ex(unit); // no longer need creator if we are using this robust AI function, still keeping the argument for legacy reasons
    }
    return unit;
}

Unit* create_hangar_unit(Unit* source, u32 unit_id) {
    u32 count = source->unit_with_hangar.in_hangar_count;
    if (source->id == UnitId::ProtossSolarion) {
        count += source->unit_with_hangar.out_hangar_count;
    }
    if (count >= source->get_max_hangar_space()) {
        return 0;
    }

    Unit* unit = create_unit_in_game(unit_id, source->player, source->position.x, source->position.y, source, false);
    if (!unit) {
        return 0;
    }
    unit->hangar_unit.parent = source;
    helper::add_hangar_unit(source, unit);
    return unit;
}

void create_hangar_units(Unit* source, u32 count, u32 unit_id) {
    for (u32 i = 0; i < count; ++i) {
        create_hangar_unit(source, unit_id);
    }
}

u32 get_unit_overlay_adjustment(Unit* const unit) {
    if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
        return 2;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
        return 1;
    }
    return 0;
}

void refresh_screen(s32 left, s32 top, s32 right, s32 bottom) {
    left >>= 4; right = (right + 15) >> 4;
    top >>= 4; bottom = (bottom + 15) >> 4;

    if (left > right) {
        std::swap(left, right);
    }
    if (top > bottom) {
        std::swap(top, bottom);
    }

    // Rect out of bounds
    if (left >= 40 || right < 0 || top >= 30 || bottom < 0) {
        return;
    }

    left = std::max(left, 0); right = std::min(right, 40 - 1);
    top = std::max(top, 0); bottom = std::min(bottom, 30 - 1);

    for (s32 y = top; y <= bottom; ++y) {
        memset(&g_refresh_regions[40 * y + left], 1, right - left + 1);
    }
}

void refresh_screen() {
    memset(g_refresh_regions, 1, 1200);
}

static u32 log_random(gptp::GameLogger& logger, u32 value, std::source_location src, u32 old_last_random) {
    if (g_config.log_randoms) {
        logger << src.function_name() << "\n";
        logger << src.file_name() << ":" << src.line() << "\n";
        logger << "g_last_random_number was " << old_last_random << "\n";
        logger << "Randomized " << value << " on frame " << g_elapsed_time_frames << "\n\n";
    }
    return value;
}

u32 rand_between(u32 min, u32 max, std::source_location src) {
    assert(min <= max);
    return min + ((max - min + 1) * random(src) >> 15);
}

u32 rand_between_local(u32 min, u32 max, std::source_location src) {
    assert(min <= max);
    struct LocalRandomContext {
        std::random_device rd;
        std::mt19937 mt{rd()};
    };
    static LocalRandomContext context;
    return log_random(gptp::g_local_random_logger, context.mt() % (max - min + 1) + min, src, 3);
}

u16 random(std::source_location src) {
    if (g_is_in_game_loop) {
        u32 old_last_random = g_last_random_number;
        g_last_random_number = 22695477 * g_last_random_number + 1;
        return log_random(gptp::g_sync_random_logger, (g_last_random_number >> 16) % 32768, src, old_last_random);
    }
    return 0;
}

f32 random_float(std::source_location src) {
    return random() / MaxRandom;
}

void refresh_console() { // 004C36C0
    g_force_button_refresh = 1;
    g_portrait_redraw = 1;
    g_status_screen_redraw = 1;
    g_control_under_mouse = 0;
    g_control_under_mouse_value = 0;
}

void create_unit_from_ability(s32 unit_id, s32 count, u16 player, Point16 position, Unit* creator) {
    count *= bw::get_game_kind() == GameKind::Massacre ? 2 : 1;
    u32 remove_timer = 0;
    switch (unit_id) {
        case UnitId::ZergRililisk:
        case UnitId::ZergRilirokor: {
            remove_timer = Broodspawn::LifeTimer;
        } break;
    }

    for (s32 i = 0; i < count; i++) {
        Unit* unit = bw::create_unit_in_game(unit_id, player, position.x, position.y, creator, false);
        if (!unit) {
            return;
        }
        bw::give_ai_ex(unit);
        if (creator) {
            spread_debuffs(creator, unit);
            if (creator->status & UnitStatus::IsHallucination) {
                helper::apply_hallucination_stats(unit);
                unit->status |= UnitStatus::IsHallucination;
                unit->remove_timer = 1350;
                unit->sprite->create_overlay(ImageId::HallucinationDeath3);
            }
        }
        UnitExt* unit_ext = get_unit_ext(unit);

        switch (unit_id) {
            case UnitId::ProtossRenesair:
            case UnitId::ProtossSimulacrum: {
                if (creator) {
                    unit->hitpoints = std::max(256, creator->hitpoints);
                    unit->shields = std::max(256, creator->shields);
                    bw::inherit_debuffs(creator, unit);
                    inherit_selection(creator, unit);
                }
                helper::play_sound_from_unit(SoundId::Simulacrum_Clone_01, unit, 1, 0);
            } break;
            case UnitId::ZergRilirokor:
            case UnitId::ZergRililisk: {
                helper::play_sound_from_unit(SoundId::BroodspawnSpawnProc, unit, 1, 0);
                if (unit->remove_timer == 0 || unit->remove_timer > remove_timer) {
                    unit->remove_timer = remove_timer;
                }
            } break;
            case UnitId::ZergVilgoleth: {
                unit_ext->timers[TimerVilgolethSpawn] = 24;
            } break;
        }
    }
}

bool can_spawn_broodling(Unit* target) {
    if (units_dat::group_flags[target->id].is_building) return false;
    return true;
}

s32 dir_saturate(s32 direction, s32 value) {
    s32 d = direction + value;
    if (d > 255) {
        return d - 255;
    }
    if (d < 0) {
        return d + 255;
    }
    return d;
}

u32 get_sized_overlay(Unit* unit, u32 initial) {
    if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
        return initial + 2;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
        return initial + 1;
    }
    return initial;
}

static bool can_use_cliff_advantage(Unit* unit, u8 weapon_id) {
    switch (unit->id) {
        case UnitId::TerranHarakan:
        case UnitId::TerranHarakanGolden:
        case UnitId::TerranStriga:
        case UnitId::TerranMatadorTankMode:
        case UnitId::TerranMatadorTurretTankMode:
        case UnitId::TerranMatadorCinderMode:
        case UnitId::TerranMatadorTurretCinderMode:
        case UnitId::Ackmed:
        case UnitId::ZergIzirokor: {
            return false;
        }
    }
    return !unit->is_in_air() && (weapon_id == WeaponId::None || weapons_dat::max_range[weapon_id] > 32) && weapon_id != WeaponId::DurendalAbaddonFlamethrowers;
}

f32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon, bool default_to_max_bonus) {
    if (!can_use_cliff_advantage(unit, weapon)) {
        return 0;
    }
    Point16 unit_pos;
    if (unit_id_is_burrowed_attacker(unit->id)) {
        unit_pos = unit->move_target.pt;
    } else {
        unit_pos = unit->position;
    }
    s32 source_height = g_height_table[g_map_tile_size->width * (unit_pos.y >> 5) + (unit_pos.x >> 5)];
    f32 multiplier = bw::get_game_kind() == GameKind::DarkAges ? 0.5 : 1;
    if (!target_unit) {
        return std::max(source_height, 0) * multiplier;
    } else if (target_unit->is_in_air()) {
        return 0;
    }
    s32 target_height = g_height_table[g_map_tile_size->width * (target_unit->position.y >> 5) + (target_unit->position.x >> 5)];
    return std::max(source_height - target_height, 0) * multiplier;
}

void apply_ensnare(Unit* unit) { // 004F45E0
    if (!(unit->ensnare_timer) && !unit->is_burrowed()) {
        Unit* victim = unit->subunit ? unit->subunit : unit;
        victim->sprite->create_overlay(ImageId::EnsnareOverlay_Small + bw::get_overlay_offset(unit->id));
    }
    unit->ensnare_timer = 37;
    unit->update_speed();
}

bool is_enabled(Unit* unit) {
    if (unit->is_frozen() || unit->status & UnitStatus::Disabled) {
        return false;
    }
    return true;
}

s32 get_upgrade_type(Unit* unit) {
    s32 race = unit->get_race();
    switch (race) {
        case RaceId::Terran: {
            return UpgradeId::TerranInfantryWeapons;
        }
        case RaceId::Zerg: {
            return UpgradeId::ZergMeleeAttacks;
        } break;
        case RaceId::Protoss: {
            return UpgradeId::ProtossGroundWeapons;
        } break;
    }
    return UpgradeId::TerranInfantryWeapons;
}

s32 get_overlay_offset(u16 unit_id, bool xlarge_allowed) {
    if (xlarge_allowed && unit_id_uses_xlarge_overlays(unit_id)) {
        return 3;
    }
    if (units_dat::base_property[unit_id] & UnitProperty::LargeOverlay) {
        return 2;
    }
    if (units_dat::base_property[unit_id] & UnitProperty::MediumOverlay) {
        return 1;
    }
    return 0;
}

s32 get_final_damage(Unit* unit, u16 weapon_id) {
    s32 upgrade_type = get_upgrade_type(unit);
    s32 base_damage = weapons_dat::damage_amount[weapon_id];
    f64 bonus_damage = (f64) weapons_dat::armor_pen[weapon_id] * (f64) bw::get_upgrade_level(unit->player, upgrade_type);
    f64 final_damage = base_damage;
    if (bonus_damage) {
        final_damage = base_damage + bonus_damage;
    }
    return final_damage;
}

// Replacement for order_is_spell @ 00436930 -- Pr0nogo
bool order_is_spell(s32 order_id) {
    return order_id == OrderId::Plague
        || order_id == OrderId::CarapaceSwarm
    //	|| order_id == OrderId::Ensnare				// Land Grab
        || order_id == OrderId::Irradiate
        || order_id == OrderId::StasisField
        || order_id == OrderId::EMPShockwave		// Observance	 (unused)
        || order_id == OrderId::Lockdown			// Lobotomy Mine (unused)
        || order_id == OrderId::PsiStorm
    //	|| order_id == OrderId::SummonBroodlings	// unused
        || order_id == OrderId::FireYamatoGun1;
}

// Creates the shield flickering effect. 004E6140
void create_shield_overlay(Unit* unit, u32 attack_direction) {
    const LO_Header* shield_lo = g_shield_overlays[unit->sprite->main_graphic->id];
    u32 frame_angle = (attack_direction - 124) / 8 % 32;

    s32 direction = unit->sprite->main_graphic->direction;
    if (direction >= shield_lo->frame_count) {
        bw::printf("Unit id %u has an invalid shield overlay!", unit->id);
    } else {
        Point8 offset = shield_lo->get_offset(direction, frame_angle);
        unit->sprite->create_overlay(ImageId::ShieldOverlay, offset.x, offset.y, frame_angle);
    }
}

void decloak_proc(Unit* unit, u32 timer) {
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->timers[TimerDecloak] == 0) {
        u32 sound_id = get_decloak_sound_id(unit);
        helper::end_cloak(unit, sound_id);
    }
    if (unit_ext->timers[TimerDecloak] != 1) {
        unit_ext->timers[TimerDecloak] = timer + 1;
    }
}

void decloak_passive_proc(Unit* unit) {
    if (is_passive_decloakable(unit)) {
        decloak_proc(unit, 60);
    }
    if (unit->is_subunit() && unit->subunit) {
        decloak_passive_proc(unit->subunit);
    }
}

void decloak_proximity_proc(Unit* unit) {
    if (is_proximity_decloakable(unit)) {
        decloak_proc(unit, 48);
        unit->status &= ~UnitStatus::CloakingForFree;
    }
    if (unit->is_subunit() && unit->subunit) {
        decloak_proximity_proc(unit->subunit);
        unit->subunit->status &= ~UnitStatus::CloakingForFree;
    }
}

bool is_passive_decloakable(Unit* unit) {
    if (unit->is_burrowed() || unit_id_has_proximity_cloak(unit->id)) {
        return false;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::PermanentCloak) {
        return true;
    }
    auto& unit_ext = unit->ext();
    if (unit_ext.values[ValueTriggerBehaviorSteadyShroud]) {
        return true;
    }
    return unit->status & UnitStatus::CloakingForFree; // Didact
}

bool is_proximity_decloakable(Unit* unit) {
    if (unit->is_burrowed()) {
        return false;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::PermanentCloak && unit_id_has_proximity_cloak(unit->id)) {
        return true;
    }
    auto& unit_ext = unit->ext();
    if (unit_ext.values[ValueTriggerBehaviorProximityCloak]) {
        return true;
    }
    return false;
}

// Replacement for unit_is_ghost
bool unit_can_nuke(Unit* unit) {
    return unit->id == UnitId::TerranEidolon || unit->id == UnitId::TerranSeraph;
}

UnitId::Enum get_trap_id(Unit* unit) {
    switch (unit->id) {
        case UnitId::TerranVulture:        return UnitId::TerranLobotomyMine;
        case UnitId::TerranClericFuturist: return UnitId::TerranPietyWard;
    }
    return UnitId::None;
}

bool unit_is_ranged_attacker(Unit* unit) {
    if (unit->connected_unit && unit->connected_unit->is_in_air()) {
        return false;
    }

    // Would be nice to somehow avoid ID hardcode
    switch (unit->id) {
        // Prohibited IDs
        case UnitId::ProtossAmaranth:
        case UnitId::ZergAkistrokor:
        case UnitId::ZergUltrokor: {
            return false;
        } break;
        // Allowed IDs
        case UnitId::TerranHarakan:
        case UnitId::TerranHarakanGolden:
        case UnitId::TerranGoliathTurret:
        case UnitId::TerranMatadorTankMode:
        case UnitId::TerranMatadorCinderMode:
        case UnitId::TerranMatadorTurretTankMode:
        case UnitId::TerranMatadorTurretCinderMode:
        case UnitId::TerranPhalanxTankMode:
        case UnitId::TerranPhalanxSiegeMode:
        case UnitId::TerranPhalanxTankModeTurret:
        case UnitId::TerranPhalanxSiegeModeTurret:
        case UnitId::TerranMadrigalImperiosoMode:
        case UnitId::TerranMadrigalFuriaMode:
        case UnitId::TerranMadrigalTurretImperiosoMode:
        case UnitId::TerranMadrigalTurretFuriaMode:
        case UnitId::Ackmed: {
            return true;
        } break;
    }

    u16 ground_weapon_id = units_dat::ground_weapon[unit->id];
    if (unit->subunit) {
        ground_weapon_id = units_dat::ground_weapon[unit->subunit->id];
    }
    if (ground_weapon_id != WeaponId::None) {
        if (weapons_dat::max_range[ground_weapon_id] > 32) {
            return true;
        }
    }
    u16 air_weapon_id = units_dat::air_weapon[unit->id];
    if (unit->subunit) {
        air_weapon_id = units_dat::air_weapon[unit->subunit->id];
    }
    if (air_weapon_id != WeaponId::None) {
        if (weapons_dat::max_range[air_weapon_id] > 32) {
            return true;
        }
    }
    return false;
}

void play_work_animation(Unit* unit) {
    Image* current_image = unit->sprite->images.head;
    unit->secondary_order_state = 2;
    while (current_image) {
        current_image->play_iscript_animation(IscriptAnimation::IsWorking);
        current_image = current_image->link.next;
    }
}

bool unit_id_is_available_without_an_addon(u16 id) {
    return id == UnitId::TerranMason
        || id == UnitId::TerranTalos
        || id == UnitId::TerranArmistice
        || id == UnitId::TerranPythean

        || id == UnitId::TerranMaverick
        || id == UnitId::TerranHarakan
        || id == UnitId::TerranCyprian
        || id == UnitId::TerranMadcap
        || id == UnitId::TerranHeracles
        || id == UnitId::TerranApostle
        || id == UnitId::TerranDilettante
        || id == UnitId::TerranSevenstep
        || id == UnitId::TerranSilvertongue
        || id == UnitId::TerranHypnagogue

        || id == UnitId::TerranVulture
        || id == UnitId::TerranCyclops
        || id == UnitId::TerranGoliath
        || id == UnitId::TerranRamesses
        || id == UnitId::TerranDurendal
        || id == UnitId::TerranCataphract
        || id == UnitId::TerranPaladin
        || id == UnitId::TerranCuirassTankMode
        || id == UnitId::TerranPazuzu

        || id == UnitId::TerranWraith
        || id == UnitId::TerranAncile
        || id == UnitId::TerranTrojan
        || id == UnitId::TerranGorgon
        || id == UnitId::TerranWyvern
        || id == UnitId::TerranValkyrie
        || id == UnitId::TerranCentaur
        || id == UnitId::TerranMinotaur
        || id == UnitId::TerranMagnetar

        || id == UnitId::TerranHotshot
        || id == UnitId::TerranHarakanGolden
        || id == UnitId::TerranCohort
        || id == UnitId::TerranAspirant
        || id == UnitId::TerranSouthpaw
        || id == UnitId::TerranPhiloc
        || id == UnitId::TerranWendigo
        || id == UnitId::TerranQilin;
}

void apply_hallucinating(Unit* unit, u8 player, u32 time) {
    UnitExt* unit_ext = get_unit_ext(unit);

    Unit* context = unit->subunit ? unit->subunit : unit;
    if (!context->get_overlay(ImageId::HallucinationHit)) {
        // Overlay
        context->sprite->create_overlay(ImageId::HallucinationHit, 0, 0, 0);

        // Sound
        if (!unit_ext->timers[TimerHallucinating]) {
            helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal00_WAV, unit, 1, 0);
        } else {
            helper::play_sound_from_unit(SoundId::Protoss_TEMPLAR_PTeHal01_WAV, unit, 1, 0);
        }
    }

    // Value
    if (unit_ext->timers[TimerHallucinating] < time) {
        unit_ext->timers[TimerHallucinating] = time;
    }
}

bool can_be_entered_by(Unit* transport, Unit* unit) {
    const u32 func_address = 0x004E6E00; // AKA CanEnterTransport()
    static u32 result;
    __asm {
        PUSHAD
        MOV EAX, transport
        PUSH unit
        CALL func_address
        MOV result, EAX
        POPAD
    }
    return result != 0;
}

u16 get_tech_user_error_message(Unit* target, u8 casting_player, u16 tech_id) {
    const u32 func_address = 0x00491E80;
    u32 tech_id_xd = tech_id;
    static u16 result;
    __asm {
        PUSHAD
        PUSH tech_id_xd
        MOV BL, casting_player
        MOV EAX, target
        CALL func_address
        MOV result, AX
        POPAD
    }
    return result;
}

u16 get_last_queue_slot_id(Unit* a1) {
    const u32 func_address = 0x0047B270;
    static s32 result;
    __asm {
        PUSHAD
        MOV EDX, a1
        CALL func_address
        MOV result, EAX
        POPAD
    }
    return (u16) result;
}

void set_offset_to_parent_special_overlay(Image* image) {
    const u32 func_address = 0x004D5A00;
    __asm {
        PUSHAD
        MOV ECX, image
        CALL func_address
        POPAD
    }
}

u32 get_ground_height_at_pos(s32 x, s32 y) {
    const u32 func_address = 0x004BD0F0;
    static u32 height;
    __asm {
        PUSHAD
        MOV EAX, y
        MOV ECX, x
        CALL func_address
        MOV height, EAX
        POPAD
    }
    return height;
}

void disable_dialog(BinDlg* dialog) {
    const u32 func_address = 0x00418640;
    __asm {
        PUSHAD
        MOV ESI, dialog
        CALL func_address
        POPAD
    }
}

void show_dialog(BinDlg* dialog) {
    const u32 func_address = 0x004186A0;
    __asm {
        PUSHAD
        MOV ESI, dialog
        CALL func_address
        POPAD
    }
}

void hide_dialog(BinDlg* dialog) {
    const u32 func_address = 0x00418700;
    __asm {
        PUSHAD
        MOV ESI, dialog
        CALL func_address
        POPAD
    }
}

void enable_control(BinDlg* dialog) {
    const u32 func_address = 0x00418E00;
    __asm {
        PUSHAD
        MOV ESI, dialog
        CALL func_address
        POPAD
    }
}

void update_dialog(BinDlg* dialog) {
    const u32 func_address = 0x0041C400;
    __asm {
        PUSHAD
        MOV EAX, dialog
        CALL func_address
        POPAD
    }
}

void prepare_unit_move(Unit* unit, bool hide_unit) {
    const u32 func_address = 0x00493CA0;
    assert(unit);
    b32 hide_unit_xd = hide_unit ? 1 : 0;
    __asm {
        PUSHAD
        PUSH hide_unit_xd
        MOV EDI, unit
        CALL func_address
        POPAD
    }
}

bool check_unit_collision_pos(Unit* unit, const Point16* in_pos, Point16* out_pos, Box16* move_area, bool hide_error_message, u32 some_flag) {
    const u32 func_address = 0x0049D3E0;
    assert(unit);
    assert(in_pos);
    assert(out_pos);
    static u32 result;
    u32 hide_error_message_xd = hide_error_message;
    __asm {
        PUSHAD
        PUSH some_flag;
        PUSH hide_error_message_xd
            PUSH out_pos
            PUSH in_pos
            PUSH unit
            MOV EAX, move_area
            CALL func_address
            MOV result, EAX
            POPAD
    }
    return result != 0;
}

void make_to_hold_position(Unit* unit) {
    const u32 func_address = 0x004EB5B0;
    __asm {
        PUSHAD
        MOV ESI, unit
        CALL func_address
        POPAD
    }
}

// Player check added based on how others functions test it. Values above or equal to 8 means all players.
void move_screen_to_unit(Unit* unit, u32 player) { // 004E6020
    const u32 func_address = 0x0049C440;
    if (player >= PlayerLimit || player == g_local_nation_id) {
        s32 x = unit->sprite->position.x;
        s32 y = unit->sprite->position.y;
        if (x < 0) {
            x += 31;
        }
        if (y < 0) {
            y += 31;
        }
        x = ((x / 32) - 10) * 32;
        y = ((y / 32) - 6) * 32;
        __asm {
            PUSHAD
            MOV ECX, y
            MOV EAX, x
            CALL func_address
            POPAD
        }
    }
}

// Player check added based on how others functions test it
// Values above or equal to 8 means all players
// x and y are coords as gotten from unit->sprite->position
void minimap_ping(u32 x, u32 y, s32 color, u32 player) {
    const u32 func_address = 0x004A34C0;
    if (player >= PlayerLimit || player == g_local_nation_id) {
        __asm {
            PUSHAD
            PUSH color
            PUSH y
            PUSH x
            CALL func_address
            POPAD
        }
    }
}

u8 overlay_size(Unit* unit) {
    const u32 func_address = 0x0047B720;
    static u8 result;
    __asm {
        PUSHAD
        MOV EAX, unit
        CALL func_address
        MOV result, AL
        POPAD
    }
    return result;
}

void hide_and_disable_unit(Unit* unit) {
    const u32 func_address = 0x004E6340;
    __asm {
        PUSHAD
        MOV EAX, unit
        CALL func_address
        POPAD
    }
}

void madrigal_dirge_fuse(Unit* unit, UnitExt* unit_ext) {
    bw::add_unit_overlay(unit, ImageId::MadrigalDirgeProc);
    const u32 radius = 2 * 32;
    // TODO: Unit search
    bw::UnitFinder(
        unit->get_x() - radius,
        unit->get_y() - radius,
        unit->get_x() + radius,
        unit->get_y() + radius
    ).for_each_in_match(cosmonarchy_finder_match(unit, Flags::IncludingSelf, {}), [&unit_ext](Unit* target) {
        target->damage_with_custom(unit_ext->values[ValueDirgeStacks] * Madrigal::DirgeDamage * 256, 5, unit_ext->units[UnitDirgeSource], unit_ext->values[ValueDirgeSourcePlayer]);
    });
    unit_ext->values[ValueDirgeStacks] = 0;
}

void update_signify_status(Unit* unit, UnitExt* unit_ext) {
    unit->update_speed();
    const u32 cowardice_id = ImageId::Cowardice_Small + bw::get_overlay_offset(unit->id);
    const u32 malice_id = ImageId::Malice_Small + bw::get_overlay_offset(unit->id);
    if (unit_ext->values[ValueSignifyStacks] == 0) {
        bw::remove_unit_overlay(unit, cowardice_id);
        bw::remove_unit_overlay(unit, malice_id);
    } else if (!unit->is_building() || units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding || unit->has_weapon()) {
        if (unit_ext->values[ValueSignifyStacks] < 5 && !unit->get_overlay(cowardice_id)) {
            bw::add_unit_overlay(unit, cowardice_id);
        } else if (unit_ext->values[ValueSignifyStacks] >= 5 && !unit->get_overlay(malice_id)) {
            bw::remove_unit_overlay(unit, cowardice_id);
            bw::add_unit_overlay(unit, malice_id);
        }
    }
}

bool is_unit_isolated(Unit* unit) {
    u32 ally_count = 0;
    u32 enemy_count = 0;
    auto match = cosmonarchy_finder_match(unit, Flags::NotSelf, {});
    const u32 radius = 5 * 32;
    bw::UnitFinder(
        unit->get_x() - radius,
        unit->get_y() - radius,
        unit->get_x() + radius,
        unit->get_y() + radius
    ).for_each_in_match(match, [&](Unit* target) {
        if (bw::is_allied_to(target->player, unit->player)) {
            ally_count++;
        } else {
            enemy_count++;
        }
    });

    return ally_count < enemy_count;
}

bool transport_is_full(Unit* transport) {
    UnitExt& transport_ext = transport->ext();
    for (u32 i = 0; i < TransportCap; ++i) {
        if (!transport_ext.loaded_unit[i].index) {
            return false;
        }
    }
    return true;
}

bool transport_is_empty(Unit* transport) {
    UnitExt& transport_ext = transport->ext();
    for (u32 i = 0; i < TransportCap; ++i) {
        if (transport_ext.loaded_unit[i].index) {
            return false;
        }
    }
    return true;
}

void set_ai_preference(u16 unit_id, u32 player) {
    g_unit_deaths_table[unit_id].player[player] = 1;
}

Box32 unit_area(Unit* unit) {
    Box32 area = {};
    area.w = units_dat::building_dimensions[unit->id].x;
    area.h = units_dat::building_dimensions[unit->id].y;
    area.x = unit->position.x - area.w / 2;
    area.y = unit->position.y - area.h / 2;
    return area;
}

void clear_creep(Box32 area) {
    for (u32 x = 0; x < area.w / 32; ++x) {
        for (u32 y = 0; y < area.h / 32; ++y) {
            u32 final_x = area.x + x * 32;
            u32 final_y = area.y + y * 32;
            helper::remove_creep_at_unit(final_x, final_y, UnitId::ZergEgg);
            // TODO: update creep so it respreads if possible, also maybe don't remove creep from underneath zerg structures?
        }
    }
}

u32 get_closest_resarea(Point16 position) {
    u32 min_distance = UINT32_MAX;
    u32 best_resarea = 0;
    for (u32 i = 1; i <= g_resource_areas->count; ++i) {
        ResourceArea* resarea = &g_resource_areas->areas[i];
        u32 distance = bw::distance_between(position, resarea->position);
        if (distance < min_distance) {
            min_distance = distance;
            best_resarea = i;
        }
    }
    return best_resarea;
}

AiTown* get_closest_town(u32 player, Point16 position) {
    u32 min_distance = UINT32_MAX;
    AiTown* best_town = 0;
    for (AiTown* town = g_ai_town_list[player].first; town; town = town->prev) {
        if (!town->inited) {
            continue;
        }
        u32 distance = bw::distance_between(position, town->position);
        if (distance < min_distance) {
            min_distance = distance;
            best_town = town;
        }
    }
    return best_town;
}

Point16 get_region_center(Region* region) {
    Point16 center = {};
    center.x = region->region_center_x >> 8;
    center.y = region->region_center_y >> 8;
    return center;
}

Box32 get_region_area(Region* region) {
    Box32 area = {};
    area.x = region->left;
    area.y = region->top;
    area.w = region->right;
    area.h = region->bottom;
    return area;
}

u32 get_ai_region_military_count(AiRegion* ai_region) {
    u32 count = 0;
    for (MilitaryAi* military_ai = ai_region->military.first; military_ai; military_ai = military_ai->link.prev) {
        count += 1;
    }
    return count;
}

void ai_defend(u32 player, u32 region_id) {
    AiRegion* ai_region = bw::get_ai_region(player, region_id);
    switch (ai_region->state) {
        case RegionState::Nothing:
        case RegionState::EnemySighted:
        case RegionState::TownRegionLow:
        case RegionState::TownRegionHigh:
        case RegionState::Defending:
        case RegionState::ChaseAway: {
            helper::ai_set_region_state(RegionState::Defending, ai_region); // Priority is 110 when defending regions with buildings/workers, 100 otherwise
            ai_region->defense_priority += 15; // More than both of the vanilla defense priorities
            ai_region->defend_time = 2; // The minimal time (around ~60 frames)
        } break;
    }
}

void ask_for_ai_help(Unit* victim, Unit* attacker) {
    if (!victim || !attacker) {
        return;
    }

    auto allied_structures = g_unit_search.radial_ex(victim, 20 * 32, FilterIncludingSelf | FilterAlly | FilterBuilding);
    if (allied_structures.count()) {
        for (u32 ai_player = 0; ai_player < 8; ++ai_player) {
            if (g_player_table[ai_player].type == PlayerType::Computer && victim->is_visible_to(ai_player) &&
                !bw::is_allied_to(ai_player, attacker->player) && bw::is_allied_to(ai_player, victim->player)) {
                bw::ai_defend(ai_player, bw::get_region_id(attacker->position));
            }
        }
    }
}

void spawn_custom_creep_at(Point16 pos, u32 unit_id, bool more_creep) {
    u32 w = units_dat::building_dimensions[unit_id].x / 32;
    u32 h = units_dat::building_dimensions[unit_id].y / 32;
    u32 x = pos.x / 32 - w / 2;
    u32 y = pos.y / 32 - h / 2;

    helper::clear_tile_flags_from_area(0x8000000, x, y, w, h);
    if (more_creep) {
        helper::set_creep(x - 2, y - 1, w + 4, h + 2, true);
    } else {
        helper::set_creep(x, y, w, h, false);
    }
    helper::set_map_tile_flags(0x8000000, x, y, w, h);
}

void spawn_custom_creep(Unit* unit) {
    spawn_custom_creep_at(unit->position, unit->id, units_dat::group_flags[unit->id].is_factory);
}

static void stop_custom_creep_disappearing(Unit* unit) {
    Box32 rect = {};
    bool spreads_creep = helper::get_creep_spread_area(UnitId::ZergGathtelosk, unit->position.x, unit->position.y, true, &rect);
    if (rect.top > rect.bottom) {
        return;
    }

    for (u32 y_tile = rect.top; y_tile <= rect.bottom; ++y_tile) {
        for (u32 x_tile = rect.left; x_tile <= rect.right; ++x_tile) {
            if ((*g_active_tile_array_int)[x_tile + y_tile * g_map_tile_size->width] & 0x10000000) {
                u32 x = x_tile * 32 - unit->position.x + 16;
                u32 y = y_tile * 32 - unit->position.y + 16;
                if (!spreads_creep || 100 * x * x + (y * y << 8) <= 10240000) {
                    helper::stop_creep_disappearing(x_tile, y_tile);
                }
            }
        }
    }
}

void spread_custom_creep(Unit* unit) {
    bw::stop_custom_creep_disappearing(unit);
    helper::spread_creep(UnitId::ZergGathtelosk, unit->position.x, unit->position.y);
}

void update_custom_creep_disappearance(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_could_spread_custom_creep(unit)) {
        helper::update_creep_disappearance_unit_id(UnitId::ZergGathtelosk, unit->position.x, unit->position.y, true);
    }
}

AiScript* run_ai(u32 player, Box32 area, s32 name) {
    s32 is_bw = 0;
    if (!helper::is_ai_script_name_valid(player, name)) {
        return 0;
    }
    s32 block = helper::parse_aiscript_name(name, &is_bw);
    if (block) {
        AiScript* ai_script = helper::run_aiscript(&area, player, block, is_bw);
        if (ai_script) {
            Point16 center = {};
            center.x = area.left + (area.right - area.left) / 2;
            center.y = area.top + (area.top - area.bottom) / 2;
            ai_script->flags |= bw::get_closest_resarea(center) << 3;
        }
        return ai_script;
    } else {
        bw::printf("scenario.cpp, run_ai: Invalid AI block!");
    }
    return 0;
}

AiScript* run_ai(u32 player, Point16 pos, s32 name) {
    Box32 area = {};
    area.left = pos.x - 2 * 32;
    area.right = pos.x + 2 * 32;
    area.top = pos.y - 1.5 * 32;
    area.bottom = pos.y + 1.5 * 32;
    return run_ai(player, area, name);
}

AiScript* run_default_ai(u32 player, Point16 pos) {
    u32 race = g_player_table[player].race;
    switch (race) {
        case RaceId::Terran:	bw::printf("Running Terran AI for player %u", player + 1);  return bw::run_ai(player, pos, 'xCMT');
        case RaceId::Protoss:	bw::printf("Running Protoss AI for player %u", player + 1); return bw::run_ai(player, pos, 'xCMP');
        case RaceId::Zerg:		bw::printf("Running Zerg AI for player %u", player + 1);    return bw::run_ai(player, pos, 'xCMZ');
    }
    return 0;
}

void give_ai_ex(Unit* unit) {
    if (!unit || unit->unit_ai || g_player_table[unit->player].type != PlayerType::Computer) {
        return;
    }
    if (units_dat::computer_ai_internal[unit->id] & 2) {
        helper::give_ai(unit);
        return;
    }
    if (unit_id_has_cleric_heal(unit->id) && !g_aiscript_controller[unit->player].main_medic) {
        g_aiscript_controller[unit->player].main_medic = unit;
    }
    helper::add_military_ai(bw::get_ai_region(unit->player, bw::get_region_id(unit->position)), unit, true);
}

u32 unit_get_creep_spreader_id(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    if (helper::spreads_creep(unit->id, unit->is_completed())) {
        return unit->id;
    }
    if (unit_ext->values[ValueInfested] && unit->status & UnitStatus::GroundedBuilding) {
        return UnitId::ZergHachirosk;
    }
    if (units_dat::base_property[unit->id] & UnitProperty::CreepBuilding) {
        return unit->id;
    }
    return UnitId::None;
}

bool unit_queue_full(Unit* unit) {
    for (u32 i = 0; i < 5; ++i) {
        if (unit->build_queue[i] == UnitId::None) {
            return false;
        }
    }
    return true;
}

bool all_selected_units_have_addon(u32 unit_id) {
    bool has_selection = false;
    for (u32 i = 0; i < SelectionCap; ++i) {
        Unit* unit = g_client_selection_group->unit[i];
        if (!unit) {
            break;
        }

        if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
            return false;
        }

        has_selection = true;
        if (!unit->building.addon || unit->building.addon->id != unit_id) {
            return false;
        }
    }
    return has_selection;
}

bool unit_is_stunned(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    return  unit->status & UnitStatus::Frozen || unit->lockdown_timer || unit->stasis_timer || unit->maelstrom_timer
        || unit_ext->timers[TimerStun] > 1 || unit_ext->values[ValueReviveStarted];
}

void dump_sync_data() {
    FILE* file = fopen("desync.text", "wb");
    for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
        fprintf(file, "Unit %s #%u", unit->get_name(), unit->get_unit_index() - 1);
        fprintf(file, "\tPlayer %u", unit->player);

        Sprite* sprite = unit->sprite;
        fprintf(file, "Sprite");
        fprintf(file, "\tSprite Id: %u", (u32) sprite->id);
        fprintf(file, "\tVisibility: %u", (u32) sprite->visibility_status);
    }
    fclose(file);
}

u32 get_zerg_regression(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->values[ValueRegressionUnstable]
    ||	unit_ext->values[ValueIsNydusExit]
    ||	unit_ext->values[ValueIsNeutralNydus]
    ||  unit_id_is_larva(unit->id)) {
        // The above cases do not regress!
        return UnitId::None;
    }

    switch (unit->id) {
        // Structure Morphs
        case UnitId::ZergCaphrolosk:			return UnitId::ZergHachirosk;
        case UnitId::ZergGathtelosk:			return UnitId::ZergHachirosk;
        case UnitId::ZergEzigrisant:			return UnitId::ZergExcisant;
        case UnitId::ZergSurkithCesiant:		return UnitId::ZergCagrantCesiant;
        case UnitId::ZergSpraithCesiant:		return UnitId::ZergCagrantCesiant;
        case UnitId::ZergLarvoskCesiant:		return UnitId::ZergCagrantCesiant;
        case UnitId::ZergKazitrithCesiant:		return UnitId::ZergCagrantCesiant;
        case UnitId::ZergNydusCesiant:			return UnitId::ZergCagrantCesiant;
        case UnitId::ZergSkortrithCesiant:		return UnitId::ZergSurkithCesiant;
        case UnitId::ZergSpirtithCesiant:		return UnitId::ZergSpraithCesiant;
        case UnitId::ZergOcsalCesiant:			return UnitId::ZergLarvoskCesiant;
        case UnitId::ZergOthstolOviform:		return UnitId::ZergIrolIris;
        case UnitId::ZergVornathPond:			return UnitId::ZergQuazethPool;
        case UnitId::ZergKalkiirLake:			return UnitId::ZergVornathPond;
        case UnitId::ZergTosvrolHaunt:			return UnitId::ZergMutathSpire;
        case UnitId::ZergSkibactScab:			return UnitId::ZergHydrithDen;
        case UnitId::ZergTethvithSwamp:			return UnitId::ZergSkibactScab;
        case UnitId::ZergBegrozithBarrow:		return UnitId::ZergAxtothAxis;
        case UnitId::ZergAlkajChasm:			return UnitId::ZergGeszkathGrotto;
        case UnitId::ZergChokralChazsal:		return UnitId::ZergAlkagIteth;
        case UnitId::ZergUltravCavern:			return UnitId::ZergZorkizShroud;
        case UnitId::ZergAlmakisAntre:			return UnitId::ZergMatravilNest;
        case UnitId::ZergRakzogthRoost:			return UnitId::ZergElthisthMound;

        // Base Structures
        case UnitId::ZergHachirosk:
        case UnitId::ZergEskigant:
        case UnitId::ZergCagrantCesiant:
        case UnitId::ZergIrolIris:
        case UnitId::ZergExcisant:
        case UnitId::ZergQuazethPool:
        case UnitId::ZergMutathSpire:
        case UnitId::ZergAxtothAxis:
        case UnitId::ZergHydrithDen:
        case UnitId::ZergAlkagIteth:
        case UnitId::ZergGeszkathGrotto:
        case UnitId::ZergZorkizShroud:
        case UnitId::ZergMatravilNest:
        case UnitId::ZergElthisthMound: {
        //  return UnitId::None;			// Bugs: Larva don't unlink, cagra doesn't recede
                                            // For now we circumvent these by creating the Droleth instead of transforming the structure into the Droleth
        	return unit->is_completed() && !unit_ext->values[ValueOriginatedFromGosvileth] ? UnitId::ZergDroleth : UnitId::None;
        }

        // Unit Morphs
        case UnitId::ZergIroleth:				return UnitId::ZergOvileth;
        case UnitId::ZergVorvrokor:				return UnitId::ZergZethrokor;
        case UnitId::ZergGhitorokor:			return UnitId::ZergCikralisk;
        case UnitId::ZergSovroleth:				return UnitId::ZergProtathalor;
        case UnitId::ZergTosgrilisk:			return UnitId::ZergMutalisk;
        case UnitId::ZergVithrilisk:			return UnitId::ZergSkithrokor;
        case UnitId::ZergCherbrathalor:			return UnitId::ZergKagralisk;
        case UnitId::ZergOkrimnilisk:			return UnitId::ZergEvigrilisk;
        case UnitId::ZergAlgoztalisk:			return UnitId::ZergAlmaksalisk;
        case UnitId::ZergKethrostalor:			return UnitId::ZergKeskathalor;
        case UnitId::ZergAlkajelisk:			return UnitId::ZergGeszithalor;
        case UnitId::ZergZobriolisk:			return UnitId::ZergLakizilisk;
        case UnitId::ZergZarcavrokor:			return UnitId::ZergGorgrokor;
        case UnitId::ZergTetcaerokor:			return UnitId::ZergKonvilisk;
        case UnitId::ZergIsthrathaleth:			return UnitId::ZergVilgorokor;
        case UnitId::ZergAkistrokor:			return UnitId::ZergMatraleth;

        // Imagine, eventually we will have everything using this...  -Pr0nogo
        case UnitId::ZergLiiralisk:
        case UnitId::ZergKalkalisk:
        case UnitId::ZergBactalisk:
        case UnitId::ZergTethzorokor:
        case UnitId::ZergUltrokor: {
            return get_factional_regression(unit);
        }
        default: {
            // Enable to allow non-dual birth units on cagra to regress into larva. -Pr0nogo
        //  if (!unit->is_dual_requisition() && bw::get_active_tile_at_coords(unit->position)->has_cagra) {
        //      return UnitId::ZergLarva;
        //  }
        }
    }
    return UnitId::None;
}

bool unit_is_allied_or_detected(Unit* unit, u32 player) {
    if (!unit->sprite) {
        return false;
    }
    if (bw::is_allied_to(unit->player, player)) {
        return true;
    }
    if (helper::get_detecting_players(unit) & (1 << player)) {
        return true;
    }
    return false;
}

u32 get_ground_path_distance_tiles(Point16 a, Point16 b) {
    return helper::get_tile_distance(a.x, a.y, b.x, b.y);
}

bool order_id_is_aggressive(u32 order_id) {
    return order_id == OrderId::Attack1
        || order_id == OrderId::Attack2
        || order_id == OrderId::AttackFixedRange
        || order_id == OrderId::AttackMove
        || order_id == OrderId::AttackMoveComputer
        || order_id == OrderId::AttackTile
        || order_id == OrderId::AttackUnit
        || order_id == OrderId::CarrierAttack1
        || order_id == OrderId::CarrierAttack2
        || order_id == OrderId::ReaverAttack1
        || order_id == OrderId::ReaverAttack2
        || order_id == OrderId::TowerAttack
        || order_id == OrderId::TurretAttack;
}

bool ai_unit_id_can_flee_three_quarters(u32 unit_id) {
    // Just like default, for now... -Pr0nogo
    return unit_id == UnitId::ProtossDidact
        || unit_id == UnitId::TerranWraith
        || unit_id == UnitId::ZergMutalisk;
}

bool ai_unit_id_can_flee_two_thirds(u32 unit_id) {
    // Just like default, for now... -Pr0nogo
    return unit_id == UnitId::TerranMinotaur
        || unit_id == UnitId::ProtossSolarion
        || unit_id == UnitId::ProtossPanoptus;
}

bool buttonset_id_is_submenu(u32 buttonset_id) {
    return buttonset_id == UnitId::Buttons_BasicZergBuildings
        || buttonset_id == UnitId::Buttons_AdvancedZergBuildings
        || buttonset_id == UnitId::Buttons_SupremeZergBuildings
        || buttonset_id == UnitId::Buttons_BasicTerranBuildings
        || buttonset_id == UnitId::Buttons_AdvancedTerranBuildings
        || buttonset_id == UnitId::Buttons_SupremeTerranBuildings
        || buttonset_id == UnitId::Buttons_BasicProtossBuildings
        || buttonset_id == UnitId::Buttons_AdvancedProtossBuildings
        || buttonset_id == UnitId::Buttons_RoboticProtossStructures
        || buttonset_id == UnitId::Buttons_StellarProtossStructures
        || buttonset_id == UnitId::Buttons_ProfaneProtossStructures
        || buttonset_id == UnitId::Buttons_LarvaSwarm
        || buttonset_id == UnitId::Buttons_LarvaAssault
        || buttonset_id == UnitId::Buttons_LarvaControl
        || buttonset_id == UnitId::Buttons_OcsalLarvaSwarm
        || buttonset_id == UnitId::Buttons_OcsalLarvaAssault
        || buttonset_id == UnitId::Buttons_OcsalLarvaControl;
}

u32 get_custom_secondary_weapon_from_unit_id(u32 unit_id) {
    switch (unit_id) {
        case UnitId::TerranTarasque:	return WeaponId::TarasqueAcceleratedPlasmid;
        case UnitId::TerranDurendal:	return WeaponId::DurendalPhilistineAutogun;
        case UnitId::TerranCentaur:		return WeaponId::CentaurSunchaserMissiles;
        case UnitId::ZergOkrimnilisk:	return WeaponId::OkrimniliskOmnispores;
        case UnitId::ZergZobriolisk:    return WeaponId::ZobrioliskRapidSpinesAir;
        case UnitId::ZergZarcavrokor:	return WeaponId::ZarcavrokorBonesofReflection;
    }
    return WeaponId::None;
}

void set_bullet_target(Bullet* bullet, Point16 target) {
    bullet->attack_target.unit = 0;
    bullet->attack_target.pt = target;
    bullet->move_target.unit = 0;
    bullet->move_target.pt = target;
    bullet->target_position_1 = target;
    bullet->target_position_2 = target;
}

void set_bullet_target(Bullet* bullet, Unit* target) {
    bullet->attack_target.unit = target;
    bullet->attack_target.pt = target->position;
    bullet->move_target.unit = target;
    bullet->move_target.pt = target->position;
    bullet->target_position_1 = target->position;
    bullet->target_position_2 = target->position;
}

void set_bullet_target_follow(Bullet* bullet, Target target) {
    if (target.unit) {
        set_bullet_target(bullet, target.unit);
    } else {
        set_bullet_target(bullet, target.pt);
    }
}

void set_bullet_target_absolute(Bullet* bullet, Target target) {
    set_bullet_target(bullet, target.pt);
    bullet->attack_target.unit = target.unit;
    bullet->move_target.unit = target.unit;
}

void set_bullet_direction(Bullet* bullet, Point16 target) {
    u32 direction = helper::get_direction_from_points(bullet->position, target);
    bullet->current_direction_1 = direction;
    bullet->velocity_direction_1 = direction;
    for (Image* image = bullet->sprite->images.head; image; image = image->link.next) {
        helper::set_image_direction(image, direction);
    }
}

void set_bullet_direction(Bullet* bullet, Unit* target) {
    return set_bullet_direction(bullet, target->position);
}

bool move_unit_ai(Unit* unit, AiRegion* ai_region) {
    if (!unit->unit_ai) {
        return false;
    }
    if (unit->unit_ai->type == AiType::Military) {
        MilitaryAi* military_ai = &unit->unit_ai->military_ai;
        AiRegion* current_ai_region = military_ai->region;
        helper::delete_military_ai(military_ai, military_ai->region);
        unit->unit_ai = 0;
        helper::add_military_ai(ai_region, unit, 1);
        if (unit->is_garrisoned()) {
            helper::unload_unit_proc(unit);
        }
        helper::ai_recalculate_region_strength(current_ai_region);
        helper::ai_recalculate_region_strength(ai_region); // New
        return true;
    } else if (unit->unit_ai->type == AiType::Guard) {
        unit->unit_ai->guard_ai.parent = 0;
        unit->unit_ai = 0;
        helper::add_military_ai(ai_region, unit, 1);
        helper::ai_recalculate_region_strength(bw::get_ai_region(unit->player, bw::get_region_id(unit->position)));
        helper::ai_recalculate_region_strength(ai_region); // New
        return true;
    }
    return false;
}

void astral_blessing_proc(Unit* victim, u32 attacking_player_id, u32 weapon_id) {
    u32 heal_amount = 3;
    u32 range = 96;
    switch (weapon_id) {
        case WeaponId::EcclesiastAstralBlaster:
        case WeaponId::VibranceSensoryOverload: {
            heal_amount = 3;
            range = 96;
        } break;
        case WeaponId::ExtatusRepeatingBlaster:
        case WeaponId::MatrixKhaydarinRay: {
            heal_amount = 5;
            range = 96;
        } break;
        case WeaponId::HeraldStellarTorch: {
            heal_amount = 2;
            range = 96;
        } break;
    }
    shield_heal_proc(victim, attacking_player_id, range, heal_amount);
}

void shield_heal_proc(Unit* heal_source, u32 heal_source_player, u32 range, u32 heal_amount) {
    Box32 search_area = {};
    search_area.left = heal_source->position.x - range;
    search_area.top = heal_source->position.y - range;
    search_area.right = heal_source->position.x + range;
    search_area.bottom = heal_source->position.y + range;

    for (Unit* unit : g_unit_search.rect(search_area).filter(heal_source)) {
        if (heal_source_player != unit->player && !is_allied_to(heal_source_player, unit->player)) {
            continue;
        }
        bool has_shields = bw::unit_has_shields(unit);
        if (has_shields && unit->shields < units_dat::max_shield_points[unit->id] * 256) {
            // Shield check for default behavior
            helper::add_overlay_below_main(unit->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
            helper::add_overlay_below_main(heal_source->sprite, ImageId::SmallExplosion_Unused, 0, 0, 0);
        } else {
            continue;
        }
        if (has_shields) {
            // Shield heal
            unit->heal_shields(heal_amount * 256, heal_source);
        }
    }
}

bool is_attack_region(AiRegion* ai_region) {
    switch (ai_region->state) {
        case RegionState::ContinueAttackHere:
        case RegionState::Attacking:
        case RegionState::PreparingAttack:
        case RegionState::PreparingAttackOldUnreachable: {
            return true;
        }
    }
    return false;
}

void prepare_building_area(Point16 pos, u32 unit_id, Unit* builder) {
    Box32 rect = {};
    rect.x = pos.x - units_dat::building_dimensions[unit_id].x / 2;
    rect.y = pos.y - units_dat::building_dimensions[unit_id].y / 2;
    rect.w = pos.x + units_dat::building_dimensions[unit_id].x / 2;
    rect.h = pos.y + units_dat::building_dimensions[unit_id].y / 2;

    for (Unit* unit : g_unit_search.rect(rect)) {
        if (unit == builder) {
            continue;
        }
        if (unit_id_is_siege_mode((UnitId::Enum)unit->id) && unit->main_order_id != OrderId::TankMode || unit->main_order_id == OrderId::SiegeMode) {
            unit->order_to(OrderId::TankMode);
        }
        if (unit->is_burrowed() || unit->main_order_id == OrderId::Burrow) {
            helper::unburrow(unit);
        }
        if (unit_id_is_trap(unit->id)) {
            unit->remove();
        }
        if (unit->id == UnitId::TerranCleric && unit->status & UnitStatus::AttackMove) {
            helper::order_return_to_idle(unit);
        }
        // Fix AI workers getting stuck
        if (unit->id == UnitId::ZergDroleth || unit->id == UnitId::ZergGosvileth) {
            if (unit->main_order_id == OrderId::DroneLand) {
                helper::order_return_to_idle(unit);
            }
        }
    }
}

void town_add_spending_request(AiTown& town, u32 count, u32 unit_id, u32 priority, bool request_base) {
    if (count < 0) return;

    if (request_base) {
        UnitId::Enum base_unit_id = find_equivalent((UnitId::Enum) unit_id);
        if (base_unit_id != UnitId::None) {
            u32 base_count = town_count_units(town, base_unit_id, false);
            if (base_count < count) {
                town_add_spending_request(town, count - base_count, base_unit_id, priority, request_base);
            }
        }
    }

    u32 request_type = RequestType::Building;
    if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
        request_type = RequestType::Worker;
    } else if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
        request_type = RequestType::Addon;
    }
    g_aiscript_controller[town.player].minerals_needed += units_dat::mineral_cost[unit_id] * count;
    g_aiscript_controller[town.player].vespene_needed += units_dat::vespene_cost[unit_id] * count;
    for (u32 i = 0; i < count; ++i) {
        helper::ai_add_spending_request(unit_id, town.player, request_type, priority, &town);
    }
}

void military_add_spending_request(u32 player, u32 unit_id, Point16 position, u32 priority, bool request_base) {
    if (unit_id == UnitId::TerranPhalanxSiegeMode) {
        unit_id = UnitId::TerranPhalanxTankMode;
    }
    if (request_base) {
        UnitId::Enum base_unit_id = find_equivalent((UnitId::Enum) unit_id);
        if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
            military_add_spending_request(player, unit_id, position, priority, request_base);
        }
    }
    g_aiscript_controller[player].minerals_needed += units_dat::mineral_cost[unit_id];
    g_aiscript_controller[player].vespene_needed += units_dat::vespene_cost[unit_id];
    helper::ai_add_spending_request(unit_id, player, RequestType::Military, priority, bw::get_ai_region(player, bw::get_region_id(position)));
}

void guard_add_spending_request(u32 player, GuardAi* guard_ai, u32 priority, bool request_base) {
    u32 unit_id = guard_ai->unit_id;
    if (unit_id == UnitId::TerranPhalanxSiegeMode) {
        unit_id = UnitId::TerranPhalanxTankMode;
    }
    if (request_base) {
        UnitId::Enum base_unit_id = find_equivalent((UnitId::Enum) unit_id);
        if (base_unit_id != UnitId::None && !player_count_units(player, base_unit_id, false)) {
            military_add_spending_request(player, unit_id, guard_ai->current_home, priority, request_base); // Intentionally using military, since we don't want the base unit to take over the guard
        }
    }
    g_aiscript_controller[player].minerals_needed += units_dat::mineral_cost[unit_id];
    g_aiscript_controller[player].vespene_needed += units_dat::vespene_cost[unit_id];
    helper::ai_add_spending_request(unit_id, player, RequestType::Guard, priority, guard_ai);
}

// Based on the function at 0x00462670 -Veeq7
GuardAi* create_guard_ai(u32 player, u32 unit_id, Point16 position) {
    GuardAiList* guard_list = &g_player_guard_list->player[player];
    GuardAi* guard_ai = guard_list->array->first_free_guard;
    if (guard_ai) {
        // Remove from free list
        guard_list->array->first_free_guard = guard_ai->link.prev;
        if (guard_ai->link.prev) {
            guard_ai->link.prev->link.next = 0;
        }

        // Zero init
        *guard_ai = {};

        // Add to active list
        guard_ai->link.prev = guard_list->first;
        if (guard_list->first) {
            guard_list->first->link.next = guard_ai;
        }
        guard_list->first = guard_ai;

        // Set vars
        guard_ai->unit_id = unit_id;
        guard_ai->type = 1;
        guard_ai->current_home = position;
        guard_ai->original_home = position;
    }
    return guard_ai;
}

// Returns the special damage multiplier factor for units that don't use the "Damage Factor" property in weapons.dat.
u8 get_damage_factor_for_tooltip(u8 weapon_id, Unit* unit) {
    switch (weapon_id) { {
        case WeaponId::HotshotDispatchShotgun:
        case WeaponId::SirenCharybdalRifle: {
            return 3;
        }
        case WeaponId::AurielQuarternaryRifle:
        case WeaponId::HypnagogueKaleidoscopeCannon:
        case WeaponId::VultureFragmentationGrenade:
        case WeaponId::PazuzuJupiterDriver:
        case WeaponId::WraithBurstLasers:
        case WeaponId::SundogBlindsideRockets:
        case WeaponId::CohortGaussPistols:
        case WeaponId::ZealotPhaseBlades:
        case WeaponId::CicatusPhaseClaws:
        case WeaponId::AmaranthPsiCleaver:
        case WeaponId::GolemBloodlessAssault:
        case WeaponId::IdolPrismaticBlaster:
        case WeaponId::PariahIconoclastBlades:
        case WeaponId::StarSovereignCelestialCannon:
        case WeaponId::NathrokorNathrokClaws:
        case WeaponId::LiiraliskHollowedSpores:
        case WeaponId::CikraliskAcidSalvo:
        case WeaponId::TosgriliskExplosiveSpores:
        case WeaponId::AxitriliskSearingVenom:
        case WeaponId::MortothrokorWickedIncisors:
        case WeaponId::AlmaksaliskPiercingSpines:
        case WeaponId::KeskathalorScreamingVenom: {
            return 2;
        }
        case WeaponId::OptectonSkybondRay:
        case WeaponId::StarSovereignInterventionMissiles: {
            return 1;
        }
        default: {
            return weapons_dat::damage_factor[weapon_id];
        }
    }
}

void delete_ai_script(AiScript* ai_script) {
    if (*g_first_active_aiscript == ai_script) {
        *g_first_active_aiscript = ai_script->link.next;
    }

    if (ai_script->link.next) {
        ai_script->link.prev->link.prev = ai_script->link.prev;
    }
    if (ai_script->link.next) {
        ai_script->link.next->link.prev = ai_script->link.prev;
    }

    ai_script->link.prev = 0;
    ai_script->link.next = g_aiscript_array->first_free;
    if (g_aiscript_array->first_free) {
        g_aiscript_array->first_free->link.prev = ai_script;
    }
    g_aiscript_array->first_free = ai_script;
}

void delete_town(AiTown* ai_town) {
    helper::delete_aiscripts_using_town(ai_town);
    for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
        if (unit->sprite && unit->main_order_id && units_dat::base_property[unit->id] & UnitProperty::ResourceContainer) {
            unit->building.resource.resarea = 0;
        }
    }

    u8 resarea = ai_town->resarea;
    if (resarea) {
        g_base_array[resarea].flags &= ~0x2;
    }

    s32 player = ai_town->player;
    if (g_ai_town_list[player].first == ai_town) {
        g_ai_town_list[player].first = ai_town->prev;
    }

    if (ai_town->next) {
        ai_town->next->prev = ai_town->prev;
    }
    if (ai_town->prev) {
        ai_town->prev->next = ai_town->next;
    }

    ai_town->next = 0;
    ai_town->prev = g_ai_town_list[player].array->first_free;
    if (g_ai_town_list[player].array->first_free) {
        g_ai_town_list[player].array->first_free->next = ai_town;
    }
    g_ai_town_list[player].array->first_free = ai_town;
}

void delete_fow_sprite(Thingy* thingy) {
    helper::remove_selection_circle(thingy->sprite);
    helper::sprite_destructor(thingy->sprite);
    thingy->sprite = 0;
    if (*g_first_fow_sprite == thingy) {
        *g_first_fow_sprite = thingy->next;
    }
    if (*g_last_fow_sprite == thingy) {
        *g_last_fow_sprite = thingy->prev;
    }
    if (thingy->prev) {
        thingy->prev->next = thingy->next;
    }
    if (thingy->next) {
        thingy->next->prev = thingy->prev;
    }
    thingy->prev = 0;
    thingy->next = 0;

    Thingy* first_free = *g_first_free_fow_sprite;
    if (first_free) {
        if (first_free == *g_first_free_fow_sprite) {
            *g_last_free_fow_sprite = thingy;
        }
        thingy->prev = first_free;
        thingy->next = first_free->next;
        if (first_free->next) {
            first_free->next->prev = thingy;
        }
        first_free->next = thingy;
    } else {
        *g_first_free_fow_sprite = thingy;
        *g_last_free_fow_sprite = thingy;
    }
}

std::vector<Unit*> binding_plasma_find_targets(Point16 position, Unit* reference, u32 player, u32 stacks) {
    EntryArray targets = g_unit_search.radial(position, Protoss::BindingPlasmaRadius)
        .filter(reference, FilterNotSelf | FilterNotSubunit)
        .filter_by([player](Unit* target) {
            if (bw::is_allied_to(player, target->player)) {
                return false;
            }
            if (!target->is_visible_to(player)) {
                return false;
            }
            return true;
        })
        .sort_by_distance(position);
    if (!targets.count()) {
        return {};
    }

    std::vector<Unit*> result;
    u32 target_index = 0;
    while (stacks && target_index < targets.count()) {
        Unit* target = targets[target_index++];
        result.push_back(target);
        stacks--;
    }
    return result;
}

bool binding_plasma_proc(Unit* victim, UnitId::Enum source_unit_id, const BindingPlasmaOptions& options) {
    UnitExt& victim_ext = victim->ext();
    u32 player = victim_ext.values[options.player_value_id];
    const auto targets = binding_plasma_find_targets(victim->position, victim, player, victim_ext.values[options.stack_value_id]);
    if (targets.empty()) {
        victim_ext.timers[options.timer_id] = 0;
        victim_ext.values[options.stack_value_id] = 0;
        return false;
    }

    bool procced = false;
    for (Unit* target : targets) {
        u32 angle = bw::get_angle(victim->position.x, victim->position.y, target->position.x, target->position.y);
        Bullet* bullet = helper::create_bullet((u32)options.weapon_id, victim, victim->position.x, victim->position.y, player, angle);
        if (!bullet) {
            break;
        }
        bw::set_bullet_target(bullet, target);
        victim_ext.values[options.stack_value_id]--;
        procced = true;
    }

    if (!victim_ext.values[options.stack_value_id]) {
        victim_ext.timers[options.timer_id] = 0;
    }
    return procced;
}

bool player_has_transport(u32 player) {
    for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
        if (unit_id_is_transport(unit->id)) {
            return true;
        }
    }
    return false;
}

bool unit_in_combat_within_seconds(Unit* unit, u32 seconds) {
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->damage_dealt_timer >= seconds * 24 && unit_ext->damage_taken_timer >= seconds * 24) {
        return false;
    }
    return true;
}

void update_on_damage_dealt(Unit* unit, u32 damage) {
    UnitExt* unit_ext = get_unit_ext(unit);
    unit_ext->damage_dealt_timer = 0;
    unit_ext->total_damage_dealt += damage;
    switch (unit->id) {
        case UnitId::ZergMortothrokor: {
            if (unit_ext->timers[TimerMortothrokorPassive]) {
                unit_ext->timers[TimerMortothrokorPassive] = 2 * 24;
            }
        } break;
    }
}

void update_on_damage_taken(Unit* unit, u32 damage) {
    UnitExt* unit_ext = get_unit_ext(unit);
    unit_ext->shield_regen_timer = 0;
    unit_ext->damage_taken_timer = 0;
    unit_ext->total_damage_taken += damage;
    switch (unit->id) {
        case UnitId::ZergMortothrokor: {
            if (unit_ext->timers[TimerMortothrokorPassive]) {
                unit_ext->timers[TimerMortothrokorPassive] = 2 * 24;
            }
        } break;
    }
}

bool is_team_game_map(const char* map_name) {
    return strcmp(map_name, "Todas Complex") == 0; // This was for CB2023, should be reworked into a custom trigger. -Pr0nogo
}

Image* get_powerup_overlay(Sprite* sprite) {
    for (Image* image = sprite->images.head; image; image = image->link.next) {
        if (image_id_is_powerup(image->id)) {
            return image;
        }
    }
    return 0;
}

Image* get_addon_connector_image(Unit* unit) {
    u16 image_id = 0;
    switch (unit->id) {
        case UnitId::TerranVestry: {
            image_id = ImageId::VestryConnector;
        } break;
        case UnitId::TerranCovenant: {
            image_id = ImageId::CovenantConnector;
        } break;
        case UnitId::TerranScrapyard:
        case UnitId::TerranPalladium:
        case UnitId::TerranRotary: {
            image_id = ImageId::PalladiumConnector;
        } break;
        case UnitId::TerranCommandment: {
            image_id = ImageId::CommandmentConnector;
        } break;
    }
    if (image_id) {
        return unit->get_overlay(image_id);
    }
    return 0;
}

PlayerColorCode get_player_color_code(u32 player, bool use_green_for_self) {
    PlayerColorCode code = {};
    if (use_green_for_self && player == g_local_nation_id) {
        code.data[0] = BWTC_GREEN[0];
    } else {
        code.data[0] = PlayerColorByte;
        code.data[1] = player + 1;
    }
    return code;
}

void move_camera(Point16 pos) {
    if (pos.x >= 320) {
        pos.x -= 320;
    } else {
        pos.x = 0;
    }
    if (pos.y >= 200) {
        pos.y -= 200;
    } else {
        pos.y = 0;
    }
    helper::move_screen(pos);
}

Quad quad_from_line(Point16 from, Point16 to, f32 line_width) {
    Vec32f line_dir = dir_between(from, to);
    Vec32f p = {-line_dir.y * line_width / 2, line_dir.x * line_width / 2};
    Quad quad = {};
    quad.corners[0].x = from.x - p.x;
    quad.corners[0].y = from.y - p.y;
    quad.corners[1].x = to.x - p.x;
    quad.corners[1].y = to.y - p.y;
    quad.corners[2].x = to.x + p.x;
    quad.corners[2].y = to.y + p.y;
    quad.corners[3].x = from.x + p.x;
    quad.corners[3].y = from.y + p.y;
    return quad;
}

Quad quad_from_rect(Box16 rect) {
    Quad box_quad = {};
    box_quad.top_left.x = rect.left;
    box_quad.top_left.y = rect.top;
    box_quad.top_right.x = rect.right;
    box_quad.top_right.y = rect.top;
    box_quad.bottom_right.x = rect.right;
    box_quad.bottom_right.y = rect.bottom;
    box_quad.bottom_left.x = rect.left;
    box_quad.bottom_left.y = rect.bottom;
    return box_quad;
}

static bool sat_collision_internal(Vec32f* a, u32 a_count, Vec32f* b, u32 b_count) {
    for (u32 i = 0; i < a_count; ++i) {
        Vec32f dir = dir_between(a[i], a[(i + 1) % a_count]);
        Vec32f proj = {-dir.y, dir.x};

        f32 a_min = INFINITY;
        f32 a_max = -INFINITY;
        for (u32 j = 0; j < a_count; ++j) {
            f32 dot = bw::dot_product(a[j], proj);
            if (dot < a_min) a_min = dot;
            if (dot > a_max) a_max = dot;
        }

        f32 b_min = INFINITY;
        f32 b_max = -INFINITY;
        for (u32 j = 0; j < b_count; ++j) {
            f32 dot = bw::dot_product(b[j], proj);
            if (dot < b_min) b_min = dot;
            if (dot > b_max) b_max = dot;
        }

        if (a_max < b_min || b_max < a_min) {
            return false;
        }
    }
    return true;
}

bool sat_collision(Vec32f* a, u32 a_count, Vec32f* b, u32 b_count) {
    return sat_collision_internal(a, a_count, b, b_count) && sat_collision_internal(b, b_count, a, a_count);
}

void find_closest_point_to_line(Vec32f from, Vec32f to, Vec32f point, f32* out_distance, f32* out_t, Vec32f* out_cp) {
    Vec32f ab = bw::vecf_between(from, to);
    Vec32f ap = bw::vecf_between(from, point);

    f32 t = bw::dot_product(ab, ap);
    t /= bw::dot_product(ab, ab);
    t = Clamp(t, 0, 1);

    Vec32f cp = from;
    cp.x += ab.x * t;
    cp.y += ab.y * t;

    if (out_distance) {
        *out_distance = bw::distance_between(cp, point);
    }
    if (out_t) {
        *out_t = t;
    }
    if (out_cp) {
        *out_cp = cp;
    }
}

// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
std::optional<LineLineIntersect> line_line_intersection(Vec32f a0, Vec32f a1, Vec32f b0, Vec32f b1) {
    f32 x1 = a0.x;
    f32 x2 = a1.x;
    f32 x3 = b0.x;
    f32 x4 = b1.x;

    f32 y1 = a0.y;
    f32 y2 = a1.y;
    f32 y3 = b0.y;
    f32 y4 = b1.y;

    f32 denom = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
    if (!denom) return {};

    LineLineIntersect result = {};
    result.t =  ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    result.u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
    result.clamped_t = Clamp(result.t, 0, 1);
    result.clamped_u = Clamp(result.u, 0, 1);
    result.cp = {
        x1 + (x2 - x1) * result.t,
        y1 + (y2 - y1) * result.t,
    };
    result.clamped_first_cp = {
        x1 + (x2 - x1) * result.clamped_t,
        y1 + (y2 - y1) * result.clamped_t,
    };
    result.clamped_second_cp = {
        x3 + (x4 - x3) * result.clamped_u,
        y3 + (y4 - y3) * result.clamped_u,
    };
    result.on_first_line = result.t >= 0 && result.t <= 1;
    result.on_second_line = result.u >= 0 && result.u <= 1;
    result.on_both_lines = result.on_first_line && result.on_second_line;
    return result;
}

// https://math.stackexchange.com/questions/846054/closest-points-on-two-line-segments/4289668#4289668
static std::pair<Vec32f, Vec32f> closest_points_on_segments(Vec32f a0, Vec32f a1, Vec32f b0, Vec32f b1) {
    Vec32f P1 = a0;
    Vec32f P2 = b0;
    Vec32f V1 = vecf_between(a0, a1);
    Vec32f V2 = vecf_between(b0, b1);
    Vec32f V21 = vecf_between(P1, P2);

    f32 v22 = dot_product(V2, V2);
    f32 v11 = dot_product(V1, V1);
    f32 v21 = dot_product(V2, V1);
    f32 v21_1 = dot_product(V21, V1);
    f32 v21_2 = dot_product(V21, V2);
    f32 denom = v21 * v21 - v22 * v11;

    f32 s, t;
    if (abs(denom) < 0.000001) {
        s = 0;
        t = (v11 * s - v21_1) / v21;
    } else {
        s = (v21_2 * v21 - v22 * v21_1) / denom;
        t = (-v21_1 * v21 + v11 * v21_2) / denom;
    }
    s = Clamp(s, 0, 1);
    t = Clamp(t, 0, 1);

    Vec32f cp0 = {
        P1.x + V1.x * s,
        P1.y + V1.y * s,
    };
    Vec32f cp1 = {
        P2.x + V2.x * t,
        P2.y + V2.y * t,
    };

    return {cp0, cp1};
}

bool line_trace(Point16 from, Point16 to, f32 line_width, Box16 rect, f32* out_t, Point16* out_cp) {
    Vec32f best_cp = {};
    Vec32f best_line_pt = {};
    Vec32f best_rect_pt = {};
    f32 best_t = INFINITY;
    f32 best_distance = INFINITY;

    Vec32f line0 = {f32(from.x), f32(from.y)};
    Vec32f line1 = {f32(to.x),   f32(to.y)};
    Vec32f axis = dir_between(line0, line1);
    Quad quad = quad_from_rect(rect);
    for (u32 i = 0; i < 4; i++) {
        Vec32f rect0 = quad.corners[i];
        Vec32f rect1 = quad.corners[(i + 1) % 4];

        const auto intersect = line_line_intersection(line0, line1, rect0, rect1);
        // Note that parallel line to line intersection is impossible, however this should not be a problem for any real shapes
        if (intersect) {
            const auto [line_pt, rect_pt] = closest_points_on_segments(line0, line1, rect0, rect1);
            f32 distance = std::min(distance_between(line_pt, rect_pt), distance_between(intersect->clamped_first_cp, intersect->clamped_second_cp));
            if (abs(distance - best_distance) < 1) {
                if (intersect->t < best_t) {
                    best_cp = intersect->clamped_second_cp;
                    best_line_pt = line_pt;
                    best_rect_pt = rect_pt;
                    best_t = intersect->t;
                    best_distance = distance;
                }
            } else if (distance < best_distance) {
                best_cp = intersect->clamped_second_cp;
                best_line_pt = line_pt;
                best_rect_pt = rect_pt;
                best_t = intersect->t;
                best_distance = distance;
            }
        }
    }

    if (best_distance <= line_width / 2) {
        *out_t = best_t;
        *out_cp = {u16(best_cp.x), u16(best_cp.y)};
        return true;
    }
    return false;
}

void transform_unit(Unit* unit, u16 unit_id) { // Written by DarkenedFantasies for siege transformation.
    Unit* subunit = unit->subunit;
    s8 subunit_hoffset = 0;
    s8 subunit_voffset = 0;
    Image* main_graphic = 0;
    if (subunit) {
        main_graphic = subunit->sprite->main_graphic;
        subunit_hoffset = main_graphic->horizontal_offset;
        subunit_voffset = main_graphic->vertical_offset;
    } else {
        subunit_voffset = subunit_hoffset = -1;
    }

    helper::replace_unit_with_id(unit, unit_id);
    u8 direction = unit->current_direction_1 = unit->velocity_direction_1 = unit->current_direction_2;
    subunit = unit->subunit;

    if (subunit) {
        direction = subunit->current_direction_2;
        if (subunit->id == UnitId::TerranPhalanxTankModeTurret || subunit->id == UnitId::TerranPhalanxSiegeModeTurret) {
            direction += 128; // Flip facing direction for the Phalanx turret. -DF
            subunit->current_direction_2 = direction;
        }
        subunit->current_direction_1 = subunit->velocity_direction_1 = subunit->velocity_direction_2 = direction;
        main_graphic = subunit->sprite->main_graphic;
        main_graphic->horizontal_offset = subunit_hoffset;
        main_graphic->vertical_offset = subunit_voffset;
        helper::set_image_direction(main_graphic, direction);
    }
}

static Unit* get_overlay_unit(Unit* unit) {
    if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit
        && unit->subunit->id != UnitId::TerranWyvernTurret) {
        return unit->subunit;
    }
    return unit;
}

void add_unit_overlay(Unit* unit, ImageSet image_set, const OverlaySettings& settings) {
    unit = get_overlay_unit(unit);
    const auto image_id = image_set.get_for(unit);
    if (unit->get_overlay(image_id)) {
        return;
    }

    if (settings.underlay) {
        unit->sprite->create_underlay(image_id);
    } else {
        unit->sprite->create_top_overlay(image_id);
    }
    if (settings.add_sound) {
        helper::play_sound_from_unit(settings.add_sound.rand(), unit);
    }
}

void remove_unit_overlay(Unit* unit, ImageSet image_set, const OverlaySettings& settings) {
    unit = get_overlay_unit(unit);
    const auto image_id = image_set.get_for(unit);
    if (!unit->get_overlay(image_id)) {
        return;
    }

    if (settings.play_death) {
        for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
            if (image->id != image_id) {
                continue;
            }
            image->play_iscript_animation(IscriptAnimation::Death);
        }
    } else {
        unit->remove_overlay(image_id);
    }
    if (settings.remove_sound) {
        helper::play_sound_from_unit(settings.remove_sound.rand(), unit);
    }
}

void set_unit_overlay(Unit* unit, ImageSet image_set, bool active, const OverlaySettings& settings) {
    if (active) {
        add_unit_overlay(unit, image_set.get_for(unit), settings);
    } else {
        remove_unit_overlay(unit, image_set.get_for(unit), settings);
    }
}

PlayingSound play_sound(SoundSet sound_set, Point16 pos) {
    SoundId::Enum sound_id = sound_set.rand();
    return play_sound_ext(sound_id, {.position = pos, .min_volume = sfx_dat::volume[sound_id]});
}

PlayingSound play_sound(SoundSet sound_set, Unit* unit, bool force_globally) {
    SoundId::Enum sound_id = sound_set.rand();
    return play_sound_ext(sound_id, {.position = force_globally ? Point16{} : unit->position, .unit = unit, .min_volume = sfx_dat::volume[sound_id], .force_globally = force_globally});
}

PlayingSound play_looping_sound(SoundSet sound_set, Point16 pos) {
    SoundId::Enum sound_id = sound_set.rand();
    return play_sound_ext(sound_id, {.position = pos, .min_volume = sfx_dat::volume[sound_id], .looping = true});
}

PlayingSound play_looping_sound(SoundSet sound_set, Unit* unit, bool force_globally) {
    SoundId::Enum sound_id = sound_set.rand();
    return play_sound_ext(sound_id, {.position = force_globally ? Point16{} : unit->position, .unit = unit, .min_volume = sfx_dat::volume[sound_id], .force_globally = force_globally, .looping = true});
}

void play_action_sound(u32 sound_id, Unit* unit) {
    if (helper::play_sound_from_unit(sound_id, unit, g_options_list->flags & 1, 99)) {
        helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 0, sound_id);
    }
}

u32 get_unit_weapon_based_on_target(Unit* attacker, Unit* target) {
    u32 weapon_id = WeaponId::None;
    if (target->is_in_air()) {
        weapon_id = units_dat::air_weapon[attacker->id];
    } else {
        if (!unit_id_is_burrowed_attacker(attacker->id) || attacker->burrowed_attacker_can_attack()) {
            weapon_id = units_dat::ground_weapon[attacker->id];
        }
    }
    return weapon_id;
}

void regress(Unit* unit, u32 regression_id) {
    helper::cancel_unit(unit);
    // We first cancel any on-going morphs, that could technically delete the unit, but that shouldn't happen for current unit id cases
    // I will check if the unit exists anyways, for the sake of not crashing the game if something about that changes - Veeq7
    if (!unit) {
        return;
    }
    helper::increment_unit_death_scores(unit, unit->player);
    bool is_building = units_dat::base_property[regression_id] & UnitProperty::Building;
    is_building = true; // Disable custom eggs for now, since they seem to yield null sprite units sometimes.
    u32 old_id = unit->id;
    u32 regress_to = regression_id;
    if (!is_building) {
        regress_to = units_dat::base_property[regression_id] & UnitProperty::Flyer ? UnitId::ZergCocoon : UnitId::ZergEgg;
    }

    if (unit_id_is_transport(old_id)) {
        helper::kill_all_loaded_units(unit); // @TODO: Get the difference between transport cargo space of new and old ids, and kill excess units. -Pr0nogo
    }

    Unit* new_unit = unit;
    if (unit->id == UnitId::ZergEskigant || unit->id == UnitId::ZergExcisant) {
        new_unit = bw::create_unit_in_game(regress_to, unit->player, unit->position.x, unit->position.y);
    } else if (unit_id_is_larva(regression_id) || units_dat::base_property[regression_id] & UnitProperty::Worker) {
        new_unit = bw::create_unit_in_game(regression_id, unit->player, unit->position.x, unit->position.y);
    } else {
        helper::replace_unit_with_id(unit, regress_to);
        unit->set_hitpoints(unit->get_max_hp_raw());

        bool was_flyer = units_dat::base_property[old_id] & UnitProperty::Flyer;
        bool is_flyer = units_dat::base_property[regress_to] & UnitProperty::Flyer;
        if (was_flyer && !is_flyer) {
            Point16 old_pos = unit->position;
            Point16 new_pos = unit->position;

            helper::fix_target_location(&new_pos, regress_to);
            if (bw::distance_between(old_pos, new_pos) <= 2 * 32) {
                helper::move_unit(unit, new_pos.x, new_pos.y);
            } else {
                unit->remove();
            }
        }
    }
    if (new_unit) {
        if (is_building) {
            UnitExt* new_unit_ext = get_unit_ext(new_unit);
            new_unit_ext->timers[TimerStun] = 1 * 24;
            // TODO: Try calling a cagra-related function to stop regression from ruining cagra spread. -Pr0nogo
            // TODO: Add stun audio visuals. -Veeq7
        } else {
            set_custom_egg(new_unit, regression_id, 2 * 24, 75 * 256); // Disable custom eggs for now, since they seem to yield null sprite units sometimes. -Veeq7
        }
    }

    u32 effect_image_id = ImageId::ZergAirDeathExplosion_Small;
    if (units_dat::base_property[unit->id] & UnitProperty::LargeOverlay) {
        effect_image_id = ImageId::ZergBuildingExplosion;
    } else if (units_dat::base_property[unit->id] & UnitProperty::MediumOverlay) {
        effect_image_id = ImageId::ZergAirDeathExplosion_Large;
    }
    unit->sprite->create_overlay(effect_image_id);
    u32 sound_id = get_death_sfx_id(old_id);
    if (!sound_id) {
        sound_id = SoundId::Zerg_Bldg_ZBldgDth_WAV;
    }
    helper::play_sound_from_unit(sound_id, unit, true, 0);

    if (unit->irradiate_timer) {
        u32 irradiate_image_id = ImageId::IrradiateNew_Small + bw::get_overlay_offset(unit->id);
        if (!unit->get_overlay(irradiate_image_id)) {
            bw::add_unit_overlay(unit, irradiate_image_id);
        }
    }
}

void unfreeze_channeling_unit(Unit* unit) {
    UnitExt& unit_ext = unit->ext();
    switch (unit->id) {
        case UnitId::ProtossStarSovereign: {
            if (unit->is_frozen() && unit_ext.values[ValueProtossSwitchOrderState] && !unit_ext.values[ValueNeedsUnfreezing]) {
                unit->play_iscript_animation(IscriptAnimation::WalkingToIdle);
                unit_ext.timers[TimerStarSovereignGriefOfAllGodsChannel] = 0;
                unit_ext.timers[TimerStarSovereignGriefOfAllGodsDelay] = 0;
                unit->order_signal &= ~127;
                unit_ext.values[ValueNeedsUnfreezing] = 1;
            }
            if (!unit->is_frozen() && unit_ext.values[ValueNeedsUnfreezing]) {
                unit->status &= ~UnitStatus::CanNotReceiveOrders;
                unit->status &= ~UnitStatus::CanNotAttack;
                unit_ext.values[ValueNeedsUnfreezing] = 0;
                unit_ext.values[ValueProtossSwitchOrderState] = 0;
                unit->unfreeze();
                unit->order_to_idle();
            }
        } break;
    }
}

bool unit_can_flee(Unit* unit) {
    if (!orders_dat::Unknown11[unit->main_order_id]
        || !(unit->status & UnitStatus::Reacts)
        || !helper::is_at_home(unit)
        || helper::is_under_carapace_swarm(unit) == 255
        || unit->main_order_id == OrderId::NukeTrack) {
        return false;
    }

    UnitExt* unit_ext = get_unit_ext(unit);
    switch (unit->id) {
        case UnitId::ProtossClarion: {
            if (unit_ext->unit_vectors[UnitVectorPhaseLink].size() <= 1) {
                return false;
            }
        } break;
        case UnitId::ProtossAnalogue:
        case UnitId::ProtossAnthelion: {
            return false;
        } break;
        case UnitId::ProtossLanifect: {
            if (unit_ext->values[ValueDisruptionFieldOn]) {
                return false;
            }
        } break;
        case UnitId::ZergMatraleth: {
            if (unit->main_order_id == OrderId::BlissfulEmbrace) {
                return false;
            }
        } break;
    }
    return true;
}

GameKind get_game_kind() {
    switch (*g_game_type) {
        case GameType::UseMapSettings:
        case GameType::Scenario:
            return GameKind::Scenario;

        case GameType::Competitive:  return GameKind::Competitive;
        case GameType::Omnipresence: return GameKind::Omnipresence;
        case GameType::FactionWars:  return GameKind::FactionWars;
        case GameType::Massacre:     return GameKind::Massacre;
        case GameType::TheTurning:   return GameKind::TheTurning;
        case GameType::Uprising:     return GameKind::Uprising;
        case GameType::Duopoly:      return GameKind::Duopoly;
        case GameType::Embraced:     return GameKind::Embraced;
        case GameType::DarkAges:     return GameKind::DarkAges;

        case GameType::Debug:        return GameKind::Debug;
    }
    return GameKind::Invalid;
}

u32 get_worker_harvest_distance(Unit* unit, Point16 target) {
    u32 distance = bw::distance_between(unit->position, target);
    if (!(unit->pathing_flags & 1)) {
        return distance;
    }
    u32 original_group_id = bw::get_region(unit->position)->group_index;
    u32 group_id = bw::get_region(target)->group_index;
    if (group_id != original_group_id) {
        return UINT_MAX;
    }
    if (distance <= 8 * 32) {
        u32 original_height = bw::get_active_tile_at_coords(unit->position)->ground_height;
        u32 height = bw::get_active_tile_at_coords(target)->ground_height;
        if (height == original_height) {
            return distance;
        }
    }
    return helper::get_path_distance_to(unit, target);
}

Point16 get_rect_edge_point_towards(Point16 pos, Box16 rect, Point16 target) {
    Vec32f dir = bw::dir_between(pos, target);
    f32 max_axis = std::max(abs(dir.x), abs(dir.y));
    if (max_axis) {
        dir.x /= max_axis;
        dir.y /= max_axis;

        u32 w = dir.x < 0 ? rect.left : rect.right;
        u32 h = dir.y < 0 ? rect.top  : rect.bottom;

        pos.x += dir.x * w;
        pos.y += dir.y * h;
    }
    return pos;
}

u32 get_fore_castle_armor_bonus(Unit* unit) {
    u32 bonus = 0;
    switch (unit->id) {
        case UnitId::TerranMinotaur: {
            bonus += 2;
        }
    }
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->values[ValueHasSublimeShepherd]) {
        bonus += 2;
    }
    return bonus;
}

u32 get_ensnare_factor(Unit* unit) {
    u32 factor = 0;
    switch (unit->id) {
        case UnitId::ZergVorvrokor:
        case UnitId::ZergAkistrokor: {
            factor++;
        }
    }
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->timers[TimerEnsnaringBrood] > 0) {
        factor++;
    }
    return factor;
}

u32 get_advisor_sound_from_event_type(u32 player, u32 event_type) {
    /*	if (bw::has_custom_advisor(player)) */
    {
        Advisor advisor = g_globals->current_advisor;
        if (advisor == Advisor::Default) {
            switch (g_player_table[player].race) {
                case RaceId::Terran: {
                    advisor = Advisor::Terran;
                } break;
                case RaceId::Protoss: {
                    advisor = Advisor::Protoss;
                } break;
                case RaceId::Zerg: {
                    advisor = Advisor::Zerg;
                } break;
            }
        }

        switch (event_type) {
            case AdvisorEvent_NoMinerals: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_tAdErr00_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_Advisor_PAdErr00_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoMinerals01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_NoMinerals01, SoundId::MeskAdvisor_NoMinerals04);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoMinerals01, SoundId::AckmedAdvisor_NoMinerals04);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoMinerals01;
                    case Advisor::Artemis:	      return helper::get_random_unit_response(SoundId::ArtemisAdvisor_NoMinerals01, SoundId::ArtemisAdvisor_NoMinerals02);
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoMinerals01, SoundId::ISNAdvisor_NoMinerals04);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoMinerals01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_NoMinerals01, SoundId::KortarAdvisor_NoMinerals04);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoMinerals01, SoundId::GoldenRepublicAdvisor_NoMinerals04);
                }
            } break;
            case AdvisorEvent_NoVespene: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_tAdErr01_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_Advisor_PAdErr01_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoVespene01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_NoVespene01, SoundId::MeskAdvisor_NoVespene05);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoVespene01, SoundId::AckmedAdvisor_NoVespene04);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoVespene01;
                    case Advisor::Artemis:	      return helper::get_random_unit_response(SoundId::ArtemisAdvisor_NoVespene01, SoundId::ArtemisAdvisor_NoVespene02);
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoVespene01, SoundId::ISNAdvisor_NoVespene04);
                    case Advisor::Furen:          return SoundId::FurenAdvisor_NoVespene01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_NoVespene01, SoundId::KortarAdvisor_NoVespene04);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoVespene01, SoundId::GoldenRepublicAdvisor_NoVespene04);
                }
            } break;
            case AdvisorEvent_NoEnergy: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_tAdErr06_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_Advisor_PAdErr06_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoEnergy01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_NoEnergy01, SoundId::MeskAdvisor_NoEnergy05);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoEnergy01, SoundId::AckmedAdvisor_NoEnergy04);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoEnergy01;
                    case Advisor::Artemis:	      return helper::get_random_unit_response(SoundId::ArtemisAdvisor_NoEnergy01, SoundId::ArtemisAdvisor_NoEnergy02);
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoEnergy01, SoundId::ISNAdvisor_NoEnergy04);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoEnergy01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_NoEnergy01, SoundId::KortarAdvisor_NoEnergy04);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoEnergy01, SoundId::GoldenRepublicAdvisor_NoEnergy04);
                }
            } break;
            case AdvisorEvent_NoBuildTerran: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_SCV_TSCErr01_WAV;
                    case Advisor::Protoss:	      return SoundId::Terran_SCV_TSCErr01_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoBuild01;
                    case Advisor::Mesk:		      return SoundId::Terran_SCV_TSCErr01_WAV;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoBuild01, SoundId::AckmedAdvisor_NoBuild01);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoBuild01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_NoBuild01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoBuild01, SoundId::ISNAdvisor_NoBuild02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoBuild01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_NoBuild01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoBuild01, SoundId::GoldenRepublicAdvisor_NoBuild02);
                }
            } break;
            case AdvisorEvent_NoBuildProtoss: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::ProtossAdvisorErrorPlacement;
                    case Advisor::Protoss:	      return SoundId::ProtossAdvisorErrorPlacement;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoWarp01;
                    case Advisor::Mesk:		      return SoundId::ProtossAdvisorErrorPlacement;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoWarp01, SoundId::AckmedAdvisor_NoWarp01);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoWarp01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_NoWarp01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoWarp01, SoundId::ISNAdvisor_NoWarp02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoWarp01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_NoWarp01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoWarp01, SoundId::GoldenRepublicAdvisor_NoWarp02);
                }
            } break;
            case AdvisorEvent_NoBuildZerg: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Zerg_DRONE_ZDrErr00;
                    case Advisor::Protoss:	      return SoundId::Zerg_DRONE_ZDrErr00;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoBuild01;
                    case Advisor::Mesk:		      return SoundId::Zerg_DRONE_ZDrErr00;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoBuild01, SoundId::AckmedAdvisor_NoBuild01);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoBuild01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_NoBuild01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoBuild01, SoundId::ISNAdvisor_NoBuild02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoBuild01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_NoBuild01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoBuild01, SoundId::GoldenRepublicAdvisor_NoBuild02);
                }
            } break;
            case AdvisorEvent_NoLanding: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_tAdErr04_WAV_1; // Fill in
                    case Advisor::Protoss:	      return SoundId::Terran_Advisor_tAdErr04_WAV_1; // Fill in
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_NoLand01;
                    case Advisor::Mesk:		      return SoundId::Terran_Advisor_tAdErr04_WAV_1; // Fill in
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoLand01, SoundId::AckmedAdvisor_NoLand02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoLand01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_NoLand01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoLand01, SoundId::ISNAdvisor_NoLand02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoLand01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_NoLand01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoLand01, SoundId::GoldenRepublicAdvisor_NoLand02);
                }
            } break;
            case AdvisorEvent_LandingInterrupted: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_tAdErr03_WAV_1;
                    case Advisor::Protoss:	      return SoundId::Terran_Advisor_tAdErr03_WAV_1;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_LandingInterrupt01;
                    case Advisor::Mesk:		      return SoundId::Terran_Advisor_tAdErr03_WAV_1;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_LandingInterrupt01, SoundId::AckmedAdvisor_LandingInterrupt02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_LandingInterrupt01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_LandingInterrupt01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_LandingInterrupt01, SoundId::ISNAdvisor_LandingInterrupt02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_LandingInterrupt01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_LandingInterrupt01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_LandingInterrupt01, SoundId::GoldenRepublicAdvisor_LandingInterrupt02);
                }
            } break;
            case AdvisorEvent_PlacementInterrupted: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_SCV_TSCErr00_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_PROBE_PPrErr00_WAV;
                    case Advisor::Zerg:		      return SoundId::Zerg_DRONE_ZDrErr00;
                    case Advisor::Mesk:		      return SoundId::Terran_SCV_TSCErr00_WAV;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_NoBuild01, SoundId::AckmedAdvisor_NoBuild01);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_NoBuild01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_NoBuild01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_NoBuild01, SoundId::ISNAdvisor_NoBuild02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_NoBuild01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_NoBuild01;
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_NoBuild01, SoundId::GoldenRepublicAdvisor_NoBuild02);
                }
            } break;
            case AdvisorEvent_StructureComplete_T: {
                // TODO: fill in
            } break;
            case AdvisorEvent_StructureComplete_P: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::ProtossAdvisorUpdateStructure; // Todo: replace
                    case Advisor::Protoss:	      return SoundId::ProtossAdvisorUpdateStructure;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_Warp01;
                    case Advisor::Mesk:		      return SoundId::MeskAdvisor_Warp01;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_Warp01, SoundId::AckmedAdvisor_Warp02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_Warp01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_Warp01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_Warp01, SoundId::ISNAdvisor_Warp02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_Warp01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_Warp01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_Warp01;
                }
            } break;
            case AdvisorEvent_StructureComplete_Z: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Protoss:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_Addon01;
                    case Advisor::Mesk:		      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Ackmed:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::HADRAL:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Artemis:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::ISN:		      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Furen:	      return SoundId::AtaltrethAdvisor_Addon01; // Todo: replace
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_Build01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_BuildZerg01;
                }
            } break;
            case AdvisorEvent_AddonComplete: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_TAdUpd03_WAV;
                    case Advisor::Protoss:	      return SoundId::Terran_Advisor_TAdUpd03_WAV; // Todo: replace
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_Addon01;
                    case Advisor::Mesk:		      return SoundId::MeskAdvisor_Addon01;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_Addon01, SoundId::AckmedAdvisor_Addon02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_Addon01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_Addon01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_Addon01, SoundId::ISNAdvisor_Addon02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_Addon01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_Addon01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_Addon01;
                }
            } break;
            case AdvisorEvent_TechUpComplete: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_TAdUpd06_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_Advisor_PAdUpd06_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_Tech01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_Tech01, SoundId::MeskAdvisor_Tech02);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_Tech01, SoundId::AckmedAdvisor_Tech02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_Tech01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_Tech01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_Tech01, SoundId::ISNAdvisor_Tech02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_Tech01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_Tech01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_Tech01;
                }
            } break;
            case AdvisorEvent_MissileArmed: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_TAdUpd07_WAV;
                    case Advisor::Protoss:	      return SoundId::Terran_Advisor_TAdUpd07_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_MissileArmed01;
                    case Advisor::Mesk:		      return SoundId::Terran_Advisor_TAdUpd07_WAV;
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_MissileArmed01, SoundId::AckmedAdvisor_MissileArmed02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_MissileArmed01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_MissileArmed01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_MissileArmed01, SoundId::ISNAdvisor_MissileArmed02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_MissileArmed01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_MissileArmed01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_MissileArmed01;
                }
            } break;
            case AdvisorEvent_ArmyUnderAttack: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::ForcesUnderAttack_Terran;
                    case Advisor::Protoss:	      return SoundId::ForcesUnderAttack_Protoss;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_ArmyUnderAttack01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_ArmyUnderAttack01, SoundId::MeskAdvisor_ArmyUnderAttack02);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_ArmyUnderAttack01, SoundId::AckmedAdvisor_ArmyUnderAttack03);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_ArmyUnderAttack01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_ArmyUnderAttack01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_ArmyUnderAttack01, SoundId::ISNAdvisor_ArmyUnderAttack04);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_ArmyUnderAttack01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_ArmyUnderAttack01, SoundId::KortarAdvisor_ArmyUnderAttack04);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_ArmyUnderAttack01, SoundId::GoldenRepublicAdvisor_ArmyUnderAttack04);
                }
            } break;
            case AdvisorEvent_BaseUnderAttack: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::BaseUnderAttack_Terran;
                    case Advisor::Protoss:	      return SoundId::BaseUnderAttack_Protoss;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_BaseUnderAttack01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_BaseUnderAttack01, SoundId::MeskAdvisor_BaseUnderAttack05);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_BaseUnderAttack01, SoundId::AckmedAdvisor_BaseUnderAttack03);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_BaseUnderAttack01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_BaseUnderAttack01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_BaseUnderAttack01, SoundId::ISNAdvisor_BaseUnderAttack01);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_BaseUnderAttack01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_BaseUnderAttack01, SoundId::KortarAdvisor_BaseUnderAttack04);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_BaseUnderAttack01, SoundId::GoldenRepublicAdvisor_BaseUnderAttack04);
                }
            } break;
            case AdvisorEvent_MissileLaunch: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_TAdUpd04_WAV;
                    case Advisor::Protoss:	      return SoundId::Protoss_Advisor_PAdUpd04_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_MissileLaunch01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_MissileLaunch01, SoundId::MeskAdvisor_MissileLaunch02);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_MissileLaunch01, SoundId::AckmedAdvisor_MissileLaunch02);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_MissileLaunch01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_MissileLaunch01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_MissileLaunch01, SoundId::ISNAdvisor_MissileLaunch02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_MissileLaunch01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_MissileLaunch01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_MissileLaunch01;
                }
            } break;
            case AdvisorEvent_AllyUnderAttack_T: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::AdvisorTerranUnderAttackTerran;
                    case Advisor::Protoss:	      return SoundId::AdvisorProtossUnderAttackTerran;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_AllyUnderAttack_Terran01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_AllyUnderAttack_Terran01, SoundId::MeskAdvisor_AllyUnderAttack_Terran02);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_AllyUnderAttack_Terran01, SoundId::AckmedAdvisor_AllyUnderAttack_Terran03);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_AllyUnderAttack_Terran01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_AllyUnderAttack_Terran01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_AllyUnderAttack_Terran01, SoundId::ISNAdvisor_AllyUnderAttack_Terran03);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_AllyUnderAttack_Terran01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_AllyUnderAttack_Terran01, SoundId::KortarAdvisor_AllyUnderAttack_Terran03);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Terran01, SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Terran03);
                }
            } break;
            case AdvisorEvent_AllyUnderAttack_P: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::AdvisorTerranUnderAttackProtoss;
                    case Advisor::Protoss:	      return SoundId::AdvisorProtossUnderAttackProtoss;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_AllyUnderAttack_Protoss01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_AllyUnderAttack_Protoss01, SoundId::MeskAdvisor_AllyUnderAttack_Protoss02);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_AllyUnderAttack_Protoss01, SoundId::AckmedAdvisor_AllyUnderAttack_Protoss03);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_AllyUnderAttack_Protoss01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_AllyUnderAttack_Protoss01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_AllyUnderAttack_Protoss01, SoundId::ISNAdvisor_AllyUnderAttack_Protoss03);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_AllyUnderAttack_Protoss01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_AllyUnderAttack_Protoss01, SoundId::KortarAdvisor_AllyUnderAttack_Protoss03);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Protoss01, SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Protoss03);
                }
            } break;
            case AdvisorEvent_AllyUnderAttack_Z: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::AdvisorTerranUnderAttackZerg;
                    case Advisor::Protoss:	      return SoundId::AdvisorProtossUnderAttackZerg;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_AllyUnderAttack_Zerg01;
                    case Advisor::Mesk:		      return helper::get_random_unit_response(SoundId::MeskAdvisor_AllyUnderAttack_Zerg01, SoundId::MeskAdvisor_AllyUnderAttack_Zerg03);
                    case Advisor::Ackmed:	      return helper::get_random_unit_response(SoundId::AckmedAdvisor_AllyUnderAttack_Zerg01, SoundId::AckmedAdvisor_AllyUnderAttack_Zerg03);
                    case Advisor::HADRAL:	      return SoundId::HADRALAdvisor_AllyUnderAttack_Zerg01;
                    case Advisor::Artemis:	      return SoundId::ArtemisAdvisor_AllyUnderAttack_Zerg01;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_AllyUnderAttack_Zerg01, SoundId::ISNAdvisor_AllyUnderAttack_Zerg01);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_AllyUnderAttack_Zerg01;
                    case Advisor::Kortar:	      return helper::get_random_unit_response(SoundId::KortarAdvisor_AllyUnderAttack_Zerg01, SoundId::KortarAdvisor_AllyUnderAttack_Zerg03);
                    case Advisor::GoldenRepublic: return helper::get_random_unit_response(SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Zerg01, SoundId::GoldenRepublicAdvisor_AllyUnderAttack_Zerg03);
                }
            } break;
            case AdvisorEvent_AbandonAddon: {
                switch (advisor) {
                    case Advisor::Terran:	      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::Protoss:	      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::Zerg:		      return SoundId::AtaltrethAdvisor_AddonAbandon01;
                    case Advisor::Mesk:		      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::Ackmed:	      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::HADRAL:	      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::Artemis:	      return SoundId::Terran_Advisor_TAdUpd05_WAV;
                    case Advisor::ISN:		      return helper::get_random_unit_response(SoundId::ISNAdvisor_AddonAbandon01, SoundId::ISNAdvisor_AddonAbandon02);
                    case Advisor::Furen:	      return SoundId::FurenAdvisor_AddonAbandon01;
                    case Advisor::Kortar:	      return SoundId::KortarAdvisor_AddonAbandon01;
                    case Advisor::GoldenRepublic: return SoundId::GoldenRepublicAdvisor_AddonAbandon01;
                }
            } break;
        }
    }
    return SoundId::None;
}

u32 get_advisor_sound_from_message_id(u32 player, u32 message_id) {
    switch (message_id) {
        case AdvisorMessageId_NoMinerals:     return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoMinerals);
        case AdvisorMessageId_NoVespene:      return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoVespene);
        case AdvisorMessageId_NoBuildZerg:    return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildZerg);
        case AdvisorMessageId_NoBuildProtoss: return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildProtoss);
        case AdvisorMessageId_NoLanding:      return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoLanding);

        case AdvisorMessageId_NoEnergy:
        case AdvisorMessageId_NoEnergy2:
        case AdvisorMessageId_NoEnergy3:
        case 1639266: // TODO: Find why this mysterious message_id is passed here instead of one of the NoEnergy -Pr0nogo
            return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoEnergy);

        case AdvisorMessageId_NoBuildResources:
        case AdvisorMessageId_NoBuildOffMap:
        case AdvisorMessageId_NoBuildMapEdge:
        case AdvisorMessageId_NoBuildStandard:
        case AdvisorMessageId_NoBuildExplore:
        case AdvisorMessageId_NoBuildGeyser:
        case AdvisorMessageId_NoBuildCouldntReach: {
            switch (g_player_table[player].race) {
                case RaceId::Terran:  return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildTerran);
                case RaceId::Protoss: return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildProtoss);
                case RaceId::Zerg:    return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildZerg);
            }
            return bw::get_advisor_sound_from_event_type(player, AdvisorEvent_NoBuildTerran);
        } break;
    }
    return SoundId::None;
}

bool has_custom_advisor(u32 player) {
    if (player != g_local_nation_id) {
        return false;
    }
    return g_globals->current_advisor != Advisor::Default;
}

const Vec32f perimeter[] = {
    {64, -258}, {64, -252}, {198, -198}, {250, -66}, {258, -66},
    {258, 66}, {250, 66}, {198, 198}, {64, 252}, {64, 258},
    {-64, 258}, {-64, 252}, {-198, 198}, {-250, 66}, {-258, 66},
    {-258, -66}, {-250, -66}, {-198, -198}, {-64, -252}, {-64, -258},
    {64, -258}
};

void draw_shape(u32 i_0, float scale, Point32* offset, u8 color) {
    do {
        Point32 line_head = {(s32) (perimeter[i_0].x * scale) + offset[i_0].x, (s32) (perimeter[i_0].y * scale) + offset[i_0].y};
        i_0--;
        Point32 line_tail = {(s32) (perimeter[i_0].x * scale) + offset[i_0].x, (s32) (perimeter[i_0].y * scale) + offset[i_0].y};
        g_graphics.draw_line(line_head.x, line_head.y, line_tail.x, line_tail.y, color, graphics::ON_MAP);
    } while (i_0);
}

void draw_weapon_range(Unit* unit, u8 weapon_id, u8 color) {
    Point16 pos = unit->sprite->position;
    Box16 bounds = units_dat::unit_bounds[unit->id];
    Point32 bounds_wy = {bounds.right + pos.x, -bounds.top + pos.y};
    Point32 bounds_wh = {bounds.right + pos.x, bounds.bottom + pos.y};
    Point32 bounds_xh = {-bounds.left + pos.x, bounds.bottom + pos.y};
    Point32 bounds_xy = {-bounds.left + pos.x, -bounds.top + pos.y};
    Point32 offset[] = {
        bounds_wy, bounds_wy, bounds_wy, bounds_wy, bounds_wy,
        bounds_wh, bounds_wh, bounds_wh, bounds_wh, bounds_wh,
        bounds_xh, bounds_xh, bounds_xh, bounds_xh, bounds_xh,
        bounds_xy, bounds_xy, bounds_xy, bounds_xy, bounds_xy,
        bounds_wy
    };

    if (weapon_id != WeaponId::LobotomyMineExplosion) {
        draw_shape(20, hooks::get_max_weapon_range(unit, weapon_id, nullptr, true) / 256.0f, offset, color);

        if (weapons_dat::min_range[weapon_id]) {
            draw_shape(20,  weapons_dat::min_range[weapon_id] / 256.0f, offset, 71);
        }
    } else {
        draw_shape(20, units_dat::seek_range[unit->id] * 32 / 256.0f, offset, color);
    }
}

void draw_aura_range(Unit* unit, u16 radius, u8 color) {
    Point16 pos = unit->sprite->position;
    Box16 bounds = units_dat::unit_bounds[unit->id];
    Point32 bounds_wy = {bounds.right + pos.x, -bounds.top + pos.y};
    Point32 bounds_wh = {bounds.right + pos.x, bounds.bottom + pos.y};
    Point32 bounds_xh = {-bounds.left + pos.x, bounds.bottom + pos.y};
    Point32 bounds_xy = {-bounds.left + pos.x, -bounds.top + pos.y};
    Point32 offset[] = {
        bounds_wy, bounds_wy, bounds_wy, bounds_wy, bounds_wy,
        bounds_wh, bounds_wh, bounds_wh, bounds_wh, bounds_wh,
        bounds_xh, bounds_xh, bounds_xh, bounds_xh, bounds_xh,
        bounds_xy, bounds_xy, bounds_xy, bounds_xy, bounds_xy,
        bounds_wy
    };

    bw::draw_shape(20, ((float) (radius << 5)) / 256.0F, offset, color);
}

void reveal_attacker(Unit* attacker, Unit* victim) {
    if (!attacker) {
        return;
    }
    u32 flags = 1 << victim->player;
    for (u32 i = 0; i < PlayerLimit; ++i) {
        if (bw::is_vision_allied_to(victim->player, i)) {
            flags |= 1 << i;
        }
    }
    hooks::reveal_sight_at_location(0, flags, attacker->position.x, attacker->position.y, attacker->status & UnitStatus::InAir);
}

void unload_or_kill_loaded_units(Unit* transport) {
    if (helper::unit_is_active_transport(transport)) {
        for (u32 i = 0; i < TransportCap; ++i) {
            Unit* cargo_unit = transport->get_loaded_unit(i);
            if (!cargo_unit) {
                continue;
            }
            helper::unload_unit_proc(cargo_unit);
        }
        if (transport->get_loaded_unit(0)) {
            helper::kill_all_loaded_units(transport);
        }
    }
}

u32 get_cloak_sound_id(Unit* unit) {
    if (unit->status & UnitStatus::CloakingForFree && !unit_id_has_proximity_cloak(unit->id)) {
        return SoundId::Terran_PHOENIX_TPhClo00_WAV;
    }
    switch (unit->get_race()) {
        case RaceId::Terran:
        {
            return SoundId::CloakTerranOn;
        }
        case RaceId::Protoss:
        {
            return SoundId::Terran_PHOENIX_TPhClo00_WAV;
        }
        default:
        {
            return SoundId::Terran_PHOENIX_TPhClo00_WAV;
        }
    }
}

u32 get_decloak_sound_id(Unit* unit) {
    if (unit->status & UnitStatus::CloakingForFree && !unit_id_has_proximity_cloak(unit->id)) {
        return SoundId::Terran_PHOENIX_TPhClo01_WAV;
    }
    switch (unit->get_race()) {
        case RaceId::Terran: {
            return SoundId::CloakTerranOff;
        }
        case RaceId::Protoss: {
            return SoundId::Terran_PHOENIX_TPhClo01_WAV;
        }
        default: {
            return SoundId::Terran_PHOENIX_TPhClo01_WAV;
        }
    }
    return SoundId::Terran_PHOENIX_TPhClo01_WAV;
}

void launch_return_missile(Unit* unit, u32 weapon_id) {
    UnitExt* unit_ext = get_unit_ext(unit);
    Point16 position = unit->position;
    Point16 spr_position = unit->sprite->position;
    Vec32 pos = {(s32) unit_ext->values[ValueMissileReturnX], (s32) unit_ext->values[ValueMissileReturnY]};
    // This currently stores the old value so the function call can overwrite them safely. It's stupid. -Pr0nogo
    unit->position.x = (u16)pos.x;
    unit->position.y = (u16)pos.y;
    unit->sprite->position = unit->position;
    bw::artificial_bullet_from(weapon_id, unit->player, unit, 0, spr_position);
    u32 sound_id = SoundId::LakiziliskProjectileReturn;
    if (weapon_id == WeaponId::SkithrokorSkitteringSpores) {
        sound_id = SoundId::SkithrokorWeaponfire01;
    }
    helper::play_sound_from_unit(sound_id, unit, 1, 0);
    unit->position = position;
    unit->sprite->position = spr_position;
    if (weapon_id == WeaponId::LakiziliskSubterraneanSpines) {
        g_iquare_globals->clear_lakizilisk(unit);
    }
}

void fire_incremental_bullet(Bullet* bullet, u32 weapon_id, Unit* target, Point16 position) {
    g_iquare_globals->add_incremental_bullet_multihit(bullet, target);
    s32 count = g_iquare_globals->get_multihit_count(bullet) - 1;
    bw::create_and_init_thingy(SpriteId::Glave_Wurm_Seeker_Spores_Hit, position.x, position.y, bullet->player);

    u32 base_damage = get_bullet_damage(bullet, weapon_id);
    target->damage_with(weapon_id, base_damage - count * 256, bullet->source_unit, bullet->player);

    if (count >= weapons_dat::damage_amount[weapon_id]) {
        bullet->remove_timer = 0;
        return;
    } else {
        bullet->move_target.unit = bullet->attack_target.unit;
        if (bullet->move_target.unit) {
            bullet->target_position_1 = bullet->move_target.unit->position;
            bullet->target_position_2 = bullet->move_target.unit->position;
        }
    }
}

bool client_selection_id_check(u32 unit_id) {
    for (u32 i = 0; i < SelectionLimit; i++) {
        Unit* unit = g_client_selection_group->unit[i];
        if (unit->id != unit_id) {
            return false;
        }
    }
    return true;
}

u32 client_selection_count() {
    u32 count = 0;
    for (u32 i = 0; i < SelectionLimit; i++) {
        Unit* unit = g_client_selection_group->unit[i];
        if (unit) {
            count++;
        }
    }
    return count;
}

u32 client_selection_unit_id_count(u32 unit_id) {
    u32 count = 0;
    for (u32 i = 0; i < SelectionLimit; i++) {
        Unit* unit = g_client_selection_group->unit[i];
        if (unit && unit->id == unit_id) {
            count++;
        }
    }
    return count;
}

u32 player_selection_count(u32 player) {
    u32 count = 0;
    for (u32 i = 0; i < SelectionLimit; i++) {
        Unit* unit = players_selections->unit[player][i];
        if (unit) {
            count++;
        }
    }
    return count;
}

u32 player_selection_unit_id_count(u32 player, u32 unit_id) {
    u32 count = 0;
    for (u32 i = 0; i < SelectionLimit; i++) {
        Unit* unit = players_selections->unit[player][i];
        if (unit && unit->id == unit_id) {
            count++;
        }
    }
    return count;
}

u32 get_witness_count() {
    u32 count = 0;
    for (u32 i = 0; i < PlayerLimit; i++) {
        if (g_globals->players[i].witness_mode) {
            count++;
        }
    }
    return count;
}

void add_massacre_resource(u32 player_id) {
    if (player_id_is_extended(player_id)) {
        return;
    }
    g_game_vespene[player_id] += Massacre::ResourceReward;
    g_total_vespene[player_id] += Massacre::ResourceReward;
    g_aiscript_controller[player_id].vespene_available += Massacre::ResourceReward;
}

bool game_kind_supports_kills(GameKind game_kind) {
    return game_kind == GameKind::Competitive
        || game_kind == GameKind::Omnipresence
        || game_kind == GameKind::FactionWars
        || game_kind == GameKind::Massacre
        || game_kind == GameKind::TheTurning
        || game_kind == GameKind::Embraced;
}

bool unit_can_fire_secondary_weapon(Unit* unit) {
    auto& unit_ext = unit->ext();
    if (unit_ext.timers[TimerSecondaryWeaponCooldown]) {
        return false;
    }
    return unit->can_attack_transport_checks();
}

void cancel_targeting_and_place_building() {
    if (g_is_targeting) {
        helper::cancel_target_order();
    }
    if (g_is_placing_building) {
        helper::refresh_layers_3_and_4();
        helper::end_building_placement();
    }
}

void select(std::vector<Unit*>& units, bool play_response) {
    if (!units.size()) {
        return;
    }

    if (play_response) {
        Unit* response_unit = 0;
        for (Unit* unit : units) {
            if (unit->player == *g_active_nation_id && hooks::unit_can_rally(unit)) {
                helper::show_rally_target(unit);
            }
            if (!response_unit || helper::compare_unit_rank(unit, response_unit)) {
                response_unit = unit;
            }
        }
        helper::select_building_sfx(response_unit);
    }

    cancel_targeting_and_place_building();
    helper::create_new_unit_selections_from_list(units.data(), units.size());
    helper::cmdact_select(units.data(), units.size());
    g_client_selection_changed = true;
    bw::refresh_console();
}

u32 get_unit_weapon_fire_sound(Unit* unit, u32 weapon_id) {
    u32 sound_id = SoundId::None;
    switch (unit->id) {
        case UnitId::TerranSavantEntropyGauntlets: {
            return bw::rand_between(SoundId::SavantEntropyFire01, SoundId::SavantEntropyFire05);
        }
        case UnitId::TerranSavantPowerSiphon: {
            return bw::rand_between(SoundId::SavantSiphonFire01, SoundId::SavantSiphonFire05);
        }
    }
    if (!sound_id) {
        sound_id = get_weapon_fire_sound(weapon_id);
    }
    return sound_id;
}

u32 get_weapon_fire_sound(u32 weapon_id) {
    switch (weapon_id) {
        case WeaponId::HotshotDispatchShotgun:              return bw::rand_between(SoundId::HotshotWeaponfire01, SoundId::HotshotWeaponfire02);
        case WeaponId::MadcapDanannAutogun:                 return bw::rand_between(SoundId::MadcapWeaponfire01, SoundId::MadcapWeaponfire05);;
        case WeaponId::ApostleCharonRifle:                  return bw::rand_between(SoundId::ApostleWeaponfire01, SoundId::ApostleWeaponfire05);;
        case WeaponId::CyclopsFuturistFlakCannons:          return bw::rand_between(SoundId::CyclopsWeaponfireFuturist01, SoundId::CyclopsWeaponfireFuturist02);
        case WeaponId::ZealotPhaseBlades:	                return SoundId::ZealotWeaponhit01;
        case WeaponId::ZealotStarboundPhaseBlades:          return SoundId::ZealotStarboundWeaponhit01;
        case WeaponId::EcclesiastAstralBlaster:	            return bw::rand_between(SoundId::EcclesiastWeaponfire01, SoundId::EcclesiastWeaponfire04);
        case WeaponId::EcclesiastKhalendricAvengingBlaster:	return bw::rand_between(SoundId::EcclesiastKhalendricWeaponfire01, SoundId::EcclesiastKhalendricWeaponfire03);
        case WeaponId::LegionnairePhaseSpear:	            return SoundId::LegionnaireWeaponfire01;
        case WeaponId::CicatusPhaseClaws:	                return bw::rand_between(SoundId::CicatusWeaponfire01, SoundId::CicatusWeaponfire03);
        case WeaponId::HierophantPhaseSignifier:	        return bw::rand_between(SoundId::HierophantWeaponfire01, SoundId::HierophantWeaponfire02);
        case WeaponId::AshguardPhaseCremator:	            return bw::rand_between(SoundId::AshguardWeaponfire01, SoundId::AshguardWeaponfire05);
        case WeaponId::CharlatanFerventBlaster:	            return bw::rand_between(SoundId::CharlatanWeaponfire01, SoundId::CharlatanWeaponfire03);
        case WeaponId::OscilliantRotatingRay:	            return bw::rand_between(SoundId::OscilliantWeaponfire01, SoundId::OscilliantWeaponfire05);
        case WeaponId::UnarAutonomousRay:	                return bw::rand_between(SoundId::UnarWeaponfire01, SoundId::UnarWeaponfire05);
        case WeaponId::VibranceSensoryOverload:	            return bw::rand_between(SoundId::VibranceWeaponfire01, SoundId::VibranceWeaponfire05);
        case WeaponId::DemiurgeGalacticWeaver:	            return bw::rand_between(SoundId::DemiurgeWeaponfire01, SoundId::DemiurgeWeaponfire03);
        case WeaponId::AnalogueCylindricalBlaster:	        return bw::rand_between(SoundId::AnalogueWeaponfire01, SoundId::AnalogueWeaponfire03);
        case WeaponId::AuroraPhotonBlasters:                return SoundId::Bullet_BlastGn2_wav;
        case WeaponId::ExtatusRepeatingBlaster:             return bw::rand_between(SoundId::ExtatusWeaponfire01, SoundId::ExtatusWeaponfire03);
    }
    return SoundId::None;
}

u32 get_weapon_hit_sound(WeaponId::Enum weapon_id) {
    switch (weapon_id) {
        case WeaponId::OscilliantRotatingRay: return bw::rand_between(SoundId::OscilliantWeaponhit01, SoundId::OscilliantWeaponhit05);
    }
    return SoundId::None;
}

bool unit_regenerates_hp(Unit* unit) {
    UnitExt* unit_ext = get_unit_ext(unit);
    return units_dat::base_property[unit->id] & UnitProperty::RegeneratesHP
        || unit_ext->values[ValueInfested]
        || unit_ext->timers[TimerNathreliskMitosalFog];
}

u32 get_regeneration_factor(Unit* unit) {
    u32 factor = 0;
    UnitExt* unit_ext = get_unit_ext(unit);
    if (unit_ext->timers[TimerNathreliskMitosalFog]) {
        factor += 2;
    }
    return factor;
}

bool unit_has_shields(Unit* unit) {
    // This will potentially allow for more flexible shield crigs and behaviors. -Pr0nogo
    return units_dat::shields_enabled[unit->id];
}

bool can_show_custom_ui() {
    if (g_config.use_alt_for_selection_ui) {
        if (g_is_keycode_used[VK_MENU]) {
            return true;
        }
        return false;
    }
    return true;
}

bool player_is_witness(u32 player) {
    return player < PLAYABLE_PLAYER_COUNT && *g_is_in_game && g_globals->players[player].witness_mode;
}

bool player_is_active(u32 player) {
    return g_player_table[player].type == PlayerType::Human || g_player_table[player].type == PlayerType::Computer;
}

static std::string get_git_head() {
    fs::path git = ".git";
    std::ifstream head_file(git / "HEAD", std::ios::binary);
    if (!head_file.is_open()) {
        return {};
    }

    std::string token;
    if (head_file >> token) {
        if (token != "ref:") {
            return token;
        }

        if (head_file >> token) {
            std::ifstream ref_file(git / token, std::ios::binary);
            if (!ref_file.is_open()) {
                return {};
            }
            
            if (ref_file >> token) {
                return token;
            }
        }
    }

    return {};
}

std::string get_commit_hash() {
    std::string head = get_git_head();
    if (head.size() > 7) {
        head.resize(7);
    }
    return head;
}

bool check_building_collision(UnitId::Enum unit_id, u32 player, Point16 pos) {
    bool is_zerg = units_dat::group_flags[unit_id].is_zerg;
    Point16 size = units_dat::building_dimensions[unit_id];
    u32 tx0 = (pos.x - size.x / 2) / 32;
    u32 tx1 = (pos.x + size.x / 2) / 32;
    u32 ty0 = (pos.y - size.y / 2) / 32;
    u32 ty1 = (pos.y + size.y / 2) / 32;

    for (u32 x = tx0; x <= tx1; x++) {
        if (x >= g_map_tile_size->width) {
            return false;
        }
        for (u32 y = ty0; y <= ty1; y++) {
            if (y >= g_map_tile_size->height) {
                return false;
            }

            const auto& tile = bw::get_tile_at_tile(x, y);
            if (tile.always_unbuildable || tile.currently_occupied) {
                return false;
            }

            if (!is_zerg && player < PlayerLimit) {
                const auto tile_ext = get_tile_ext_at_tile(x, y);
                if (!can_place_on_creep_tile(tile_ext, player)) {
                    return false;
                }
            }
        }
    }

    return true;
}

Point16 align_to_tile_grid(Point16 pos, UnitId::Enum unit_id) {
    const auto [w, h] = units_dat::building_dimensions[unit_id];
    const auto hw = w / 2;
    const auto hh = h / 2;
    return {
        static_cast<u16>((pos.x + 16 - hw) / 32 * 32 + hw),
        static_cast<u16>((pos.y + 16 - hh) / 32 * 32 + hh),
    };
}

u32 active_player_count() {
    u32 player_count = 0;
    for (u32 i = 0; i < 8; i++) {
        if (bw::player_is_witness(i)) {
            continue;
        }
        if (g_player_table[i].type != PlayerType::Human && g_player_table[i].type != PlayerType::Computer) {
            continue;
        }
        player_count++;
    }
    return player_count;
}

bool unit_id_blocked_in_dark_ages(UnitId::Enum unit_id) {
    switch (unit_id) {
        // Allow Utility, Detection, Transports, No Weaponed Craft
        // Note: I wanted no transports, but it's tricky, anchors are transports, ovileth is detection
        case UnitId::TerranTalos:
        case UnitId::TerranTrojan:
        case UnitId::TerranAncile:
        case UnitId::TerranAzazel: // But no Seraph!
        case UnitId::TerranPythean:
            return false;
            
        case UnitId::ProtossArtisan:
        case UnitId::ProtossWitness:
        case UnitId::ProtossEnvoy:
        case UnitId::ProtossDidact:
        case UnitId::ProtossClarion:
            return false;

        case UnitId::ZergGosvileth:
        case UnitId::ZergOvileth:
        case UnitId::ZergIroleth:
        case UnitId::ZergOthstoleth:
        case UnitId::ZergMatraleth:
            return false;

        // Disable Useless Production
        case UnitId::TerranRotary:
        case UnitId::TerranHadron:
            return true;

        case UnitId::ProtossStargate:
        case UnitId::ProtossMonumentOfSin:
            return true;
    }
    return units_dat::base_property[unit_id] & UnitProperty::Flyer;
}

f32 calculate_dark_ages_penalty(f32 range) {
    return range - round(range * 0.8 / 2) * 2;
}

void zap_overlay(Unit* unit) {
    UnitExt& unit_ext = unit->ext();
    if (!unit_ext.timers[TimerClaymoreKelvinMunitionsZap]) {
        u32 width = units_dat::unit_bounds[unit->id].x * 3 / 4;
        u32 height = units_dat::unit_bounds[unit->id].y * 3 / 4;

        s32 x_offset = bw::random() % width - width / 2;
        s32 y_offset = bw::random() % height - height / 2;

        x_offset = Clamp(x_offset, -128, 127);
        y_offset = Clamp(y_offset, -128, 127);
        Unit* overlay_unit = unit;
        if (unit->subunit && units_dat::base_property[unit->subunit->id] & UnitProperty::Subunit) {
            overlay_unit = unit->subunit;
        }
        overlay_unit->sprite->create_top_overlay(ImageId::SalvagePeriodicZap, x_offset, y_offset);
        unit_ext.timers[TimerClaymoreKelvinMunitionsZap] = 7;
    }
}

void play_manual_weaponfire_sound(Unit* unit, WeaponId::Enum weapon_id) {
    if (const auto sound_id = bw::get_weapon_fire_sound(weapon_id)) {
        bw::play_sound(sound_id, unit->position);
    }
}

void play_manual_weaponhit_sound(Unit* unit, WeaponId::Enum weapon_id) {
    if (const auto sound_id = bw::get_weapon_hit_sound(weapon_id)) {
        bw::play_sound(sound_id, unit->position);
    }
}

void binding_plasma_tick(u32 player, Unit* unit, UnitId::Enum source_unit_id, const BindingPlasmaOptions& options) {
    bool has_source = false;
    for (Unit* source : g_unit_search.radial_ex(unit, Protoss::BindingPlasmaSearchRadius, 0, {(u16)source_unit_id})) {
        if (!bw::is_allied_to(source->player, player)) {
            continue;
        }
        has_source = true;
        UnitExt& source_ext = source->ext();
        source_ext.timers[TimerBindingPlasmaChargeOverlay] = 3;
    }

    UnitExt& unit_ext = unit->ext();
    if (has_source) {
        if (unit_ext.timers[options.timer_id] % Protoss::BindingPlasmaTimer == Protoss::BindingPlasmaTimer - 1) {
            bw::add_unit_overlay(unit, ImageId::BindingPlasmaOverlay);
        }
        if (unit_ext.timers[options.timer_id] == 1) {
            if (bw::binding_plasma_proc(unit, source_unit_id, options)) {
                bw::play_sound(options.sound, unit->position);
            }
        }
    } else {
        unit_ext.timers[options.timer_id] = 0;
        unit_ext.values[options.stack_value_id] = 0;
    }
}

} // namespace bw