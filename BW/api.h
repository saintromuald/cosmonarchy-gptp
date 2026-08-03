// Contains wrappers for various SCBW internal functions, to be used in hook functions.

#pragma once
#define NOMINMAX
#include <windows.h>

#include <string>
#include <map>
#include <math.h>
#include <source_location>
#include <numbers>
#include <filesystem>
namespace fs = std::filesystem;

#include <magic_enum/magic_enum_all.hpp>
using namespace magic_enum::bitwise_operators;

#include "BW/bwdata.h"
#include "BW/enumerations.h"
#include "BW/bwhelper.h"
#include "unit_search.h"
#include "bwutils.h"
#include "CM/cm.h"
#include "utility.h"
#include "interpolation.h"

// Text colors
#define BWTC_RESET		"\002"
#define BWTC_TEAL		"\017"
#define BWTC_GREEN		"\007"
#define BWTC_YELLOW		"\003"
#define BWTC_ORANGE		"\021"
#define BWTC_RED		"\006"
#define BWTC_WHITE		"\004"

#define HarvestTimer 1.75secs
#define HarvestTimerDepleted 3.75secs
#define OrderQueueTimer 0

#define UnloadDelay 4

#define PlayerColorByte 0x0B
struct PlayerColorCode {
    char data[3];
};

enum InterceptType {
    InterceptType_None,

    InterceptType_GroundOnly,
    InterceptType_AirOnly,
    InterceptType_Any,

    InterceptType_Count,
};

enum class GameKind {
    Invalid,

    Unranked,
    Scenario,
    Omnipresence,
    FactionWars,
    Massacre,
    TheTurning,
    Uprising,
    Duopoly,
    Embraced,
    DarkAges,
    Deathmarch,
    Brotherhood,
    Zombocalypse,
    Ranked,

    Debug,
    Count,
};

#define AssertSprite(sprite) __assert_sprite(sprite, __FUNCTION__, __FILE__, __LINE__)
bool __assert_sprite(Sprite* sprite, const char* function, const char* file, u64 line);

template <>
struct std::formatter<TextColor> {
    constexpr std::format_parse_context::iterator parse(std::format_parse_context& ctx) const {
        return ctx.end();
    }

    std::format_context::iterator format(TextColor c, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", static_cast<char>(c));
    }
};

template <size_t Capacity>
void safe_copy(char(&to)[Capacity], const char* from) {
    if (!from) { to[0] = 0; return; }
    auto copy_size = Capacity - 1;
    strncpy(to, from, copy_size);
    to[copy_size] = 0;
}

template <size_t Capacity>
void safe_copy(char(&to)[Capacity], const std::string& from) {
    safe_copy(to, from.c_str());
}

template <size_t Capacity>
void safe_copy(char(&to)[Capacity], std::string_view from) {
    auto copy_size = std::min(Capacity - 1, from.size());
    std::memcpy(to, from.data(), copy_size);
    to[copy_size] = 0;
}

template<size_t Capacity>
void safe_copy(std::array<char, Capacity>& to, const char* from) {
    if (!from) { to[0] = 0; return; }
    auto copy_size = Capacity - 1;
    strncpy(to.data(), from, copy_size);
    to[copy_size] = 0;
}

template <size_t Capacity>
void safe_copy(std::array<char, Capacity>& to, const std::string& from) {
    safe_copy(to, from.c_str());
}

template <size_t Capacity>
void safe_copy(std::array<char, Capacity>& to, std::string_view from) {
    auto copy_size = std::min(Capacity - 1, from.size());
    std::memcpy(to.data(), from.data(), copy_size);
    to[copy_size] = 0;
}

namespace enums {
template <typename T>
using base = std::underlying_type_t<T>;

template <typename T>
base<T> to_base(T value) {
    return static_cast<base<T>>(value);
}
} // enums

namespace bw {

class StatString {
public:
    StatString(const char* stat_string);

    GameData& game_data() { return m_game_data; };
    bool valid() const { return m_valid; };

private:
    template <typename T>
    void load_number(T& output, std::string_view sv) {
        std::from_chars(sv.data(), sv.data() + sv.size(), output, 16);
    }

private:
    GameData m_game_data{};
    bool m_valid = false;
};


s32 get_overlay_offset(u16 unit_id, bool xlarge_allowed = false);

template <typename T, T T_None>
struct GraphicsSet {
    T first = T_None; // Smallest Overlay
    T last = T_None;  // Largest Overlay

    GraphicsSet() = default;
    GraphicsSet(int id) : first{T(id)}, last{T(id)} {}
    GraphicsSet(T id) : first{id}, last{id} {}
    GraphicsSet(T smallest_size, T largest_size) : first{smallest_size}, last{largest_size} {}

    operator bool() const {
        return first != T_None;
    }
    bool operator==(const GraphicsSet&) const = default;

    T get_small() const {
        return get(0);
    }

    T get_medium() const {
        return get(1);
    }

    T get_large() const {
        return get(2);
    }

    T get_xlarge() const {
        return get(3);
    }

    T get_for(Unit* unit) const {
        return get(bw::get_overlay_offset(unit->id, true));
    }

private:
    T get(u32 index) const {
        if (first > last) {
            return std::clamp<T>(T(s32(first) - s32(index)), last, first);
        } else {
            return std::clamp<T>(T(s32(first) + s32(index)), first, last);
        }
    }
};
using ImageSet = GraphicsSet<ImageId::Enum, ImageId::None>;
using SpriteSet = GraphicsSet<SpriteId::Enum, SpriteId::None>;

enum class RandType { Sync, Local };

struct SoundSet {
    SoundId::Enum first = SoundId::None;
    SoundId::Enum last = SoundId::None;
    RandType random_type = RandType::Sync;

    SoundSet() = default;
    SoundSet(s32 id) : first{SoundId::Enum(id)}, last{SoundId::Enum(id)} {}
    SoundSet(SoundId::Enum id) : first{id}, last{id} {}
    SoundSet(SoundId::Enum first, SoundId::Enum last, RandType random_type = RandType::Sync) : first{first}, last{last}, random_type{random_type} {
        if (first > last) {
            std::swap(this->first, this->last);
        }
    }

    operator bool() const {
        return first != SoundId::None;
    }
    bool operator==(const SoundSet&) const = default;

    SoundId::Enum rand(RandMessage message = {}, std::source_location src = std::source_location::current()) const {
        if (first == last) return first;
        if (random_type == RandType::Local) {
            return (SoundId::Enum) bw::rand_between_local(first, last, std::move(message), src);
        }
        return (SoundId::Enum) bw::rand_between(first, last, std::move(message), src);
    }

    SoundSet as_local() {
        auto copy = *this;
        copy.random_type = RandType::Local;
        return copy;
    }

    SoundSet as_sync() {
        auto copy = *this;
        copy.random_type = RandType::Sync;
        return copy;
    }
};

struct OverlaySettings {
    bool underlay   : 1 = false;
    bool play_death : 1 = false;
    SoundSet add_sound = SoundId::None;
    SoundSet remove_sound = SoundId::None;
};

struct BindingPlasmaOptions {
    u32 timer_id;
    u32 stack_value_id;
    u32 player_value_id;
    WeaponId::Enum weapon_id;
    SoundSet sound;
};

struct TeleportOptions {
    bool preserve_order : 1 = false;
    bool check_distance : 1 = true;
};

inline bool harvester_is_active(Unit* unit, Unit* target) {
    UnitExt* unit_ext = get_unit_ext(unit);
    UnitExt* target_ext = get_unit_ext(target);
    u32 activity = unit_ext->values[ValueHarvesterActivity];
    if (target_ext->values[ValueAssignedHarvesters] <= 2) {
        // Until 2 workers, we don't want to wander synced pairs (to allow worker micro),
        return activity > 0 && activity <= HarvestTimer - 24;
    } else {
        // After 2 workers, we always wander since it's always optimal
        return activity > 0;
    }
}

std::filesystem::path get_starcraft_path();
bool is_any_morph_result(u16 id);
bool is_conventional_unit_morph_result(u16 id);
bool is_conventional_unit_morph_source(u16 id, bool allow_larvae = false);
bool is_zerg_building_morph_result(u16 id);
bool is_zerg_building_morph_source(u16 id);
bool is_larval_egg(Unit* unit);
bool is_zerg_egg_new(Unit* unit, bool allow_larval_eggs = true, bool allow_larvae = true, u16 explicit_unit_id = UnitId::None);
bool is_zerg_egg(u16 id, bool allow_normal_eggs = true, bool allow_larvae = true);
Bullet* artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, v2u16 target_pt, v2u16 origin_pt = {});
Bullet* artificial_bullet_from(u8 weapon_id, u8 player, Unit* source, Unit* target, u16 x, u16 y);
std::string read_samase_file(char* name);
bool teleport_to_point(Unit* unit, v2u16 target, u16 sprite_id = 0, bool preserve_order = false);
bool teleport_to_unit(Unit* unit, Unit* target, u32 radius, const TeleportOptions& options, u16 sprite_id = 0);
Thingy* create_and_init_thingy(u32 spriteId, s16 x, s16 y, u32 player = 11, s32 elevation_level = 11);
v2u16 rotate_point(v2u16 anchor, v2u16 point, fp32 angle_degrees);
v2s32 rotate_point(v2s32 anchor, v2s32 point, fp32 angle_degrees);
v2 rotate_point(v2 anchor, v2 point, fp32 angle_degrees);

/// @name Output functions
//////////////////////////////////////////////////////////////// @{

/// Prints text to the screen, optionally using the given text color.
void print(const char* text, TextColor color = TextColor::White);
void printf(const char* format, ...);
void printf(TextColor color, const char* format, ...);

void printf_console(const char* fmt, ...);

template <typename... Args>
void println(std::format_string<Args...> fmt = " ", Args&&... args) {
    print(std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <typename... Args>
void println(TextColor color, std::format_string<Args...> fmt, Args&&... args) {
    print(std::format(fmt, std::forward<Args>(args)...).c_str(), color);
}

/// Displays an error message and plays a new_sound for the player.
///
/// @param  player  ID of the player who receives the message.
/// @param  statTxtId ID of the error message string in stat_txt.tbl
/// @param  sfxId     ID of the new_sound file in sfxdata.dat.
void show_error_message_with_sfx(u32 player, u32 statTxtId, u32 sfxId);

//////////////////////////////////////////////////////////////// @}

u8 overlay_size(Unit* unit);

void disable_dialog(BinDlg* dialog);    //18640
void show_dialog(BinDlg* dialog);       //186A0
void hide_dialog(BinDlg* dialog);       //18700
void update_dialog(BinDlg* dialog);     //1C400


/// @name Unit checks
//////////////////////////////////////////////////////////////// @{

/// Checks whether the transport can hold this unit.
///
/// This function checks whether [transport] has any status effects (LobotomyGrenade,
/// Maelstrom, etc.), is owned by the same player, and has enough space for
/// [unit]. It does NOT check whether [transport] is a hallucination, or is an
/// Ovileth without the transport upgrade, or whether the [unit] itself has any
/// status effects. Note that this function is affected by the EXE edits
/// "change bunker count check to organic flag check" and
/// "remove bunker count check" in FireGraft.
bool can_be_entered_by(Unit* transport, Unit* unit);

/// Checks weapons.dat targeting flags to see if the @p target can be hit by the
/// @p weapon.
///
/// @param  weapon_id  If this is an invalid weapons.dat ID, returns false.
/// @param  target    If nullptr is passed, the function checks whether the
///                   weapon can target a position on the inside. If @p target
///                   is invincible, the function returns false.
bool can_weapon_target_unit(u8 weapon_id, Unit* target = nullptr,
    Unit* attacker = nullptr);

/// Checks if @p unit is under a Dark Swarm. This does NOT check whether the
/// unit is a inside unit or a building.
bool is_under_carapace_swarm(Unit* unit);
Unit* get_carapace_swarm_unit(Unit* unit);
Unit* get_dark_swarm_from_position(u16 x, u16 y, u16 radius);
//Checks if the @p target can be targeted by player @p castingPlayer with
//tech/spell @p techId
//If yes, return 0, else return error message id in stat_txt.tbl to use
u16 get_tech_user_error_message(Unit* target, u8 casting_player, u16 tech_id);

//////////////////////////////////////////////////////////////// @}

/// @name Graphics and geometry
//////////////////////////////////////////////////////////////// @{

/// Calculates the angle (in binary radians) of the arrow that starts at
/// (xStart, yStart) and ends at (xEnd, yEnd).
s32 get_angle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd);

inline s32 get_angle(v2u16 start, v2u16 end) {
    return get_angle(start.x, start.y, end.x, end.y);
}

inline s32 get_angle(v2s32 start, v2s32 end) {
    return get_angle(start.x, start.y, end.x, end.y);
}

inline fp32 bw_angle_to_normal(s32 bw_angle);
/// StarCraft's internal function used to quickly calculate distances between
/// two points (x1, y1) and (x2, y2;).
/// Warning: This function is inaccurate for long distances.
u32 distance_between(s32 x1, s32 y1, s32 x2, s32 y2);

inline u32 distance_between(v2u16 p1, v2u16 p2) {
    return distance_between(p1.x, p1.y, p2.x, p2.y);
}

/// Distance, but rectangular instead of circular (max of x dist and y dist)
u32 distance_manhattan_between(s32 x1, s32 y1, s32 x2, s32 y2);

inline u32 distance_manhattan_between(v2u16 p1, v2u16 p2) {
    return distance_manhattan_between(p1.x, p1.y, p2.x, p2.y);
}

/// Calculates the X Cartesian coordinate from the given polar coordinates
/// (distance, angle), using StarCraft's internal data.
s32 get_polar_x(s32 distance, u8 angle);

/// Calculates the Y Cartesian coordinate from the given polar coordinates
/// (distance, angle), using StarCraft's internal data.
s32 get_polar_y(s32 distance, u8 angle);

inline Box16 clamp_box16(Box16 rect) {
    return {
        std::clamp<s16>(rect.left, 0, g_map_tile_size->width * 32),
        std::clamp<s16>(rect.top, 0, g_map_tile_size->height * 32),
        std::clamp<s16>(rect.right, 0, g_map_tile_size->width * 32),
        std::clamp<s16>(rect.bottom, 0, g_map_tile_size->height * 32),
    };
}

inline Box32 clamp_box32(Box32 rect) {
    return {
        std::clamp<s32>(rect.left, 0, g_map_tile_size->width * 32),
        std::clamp<s32>(rect.top, 0, g_map_tile_size->height * 32),
        std::clamp<s32>(rect.right, 0, g_map_tile_size->width * 32),
        std::clamp<s32>(rect.bottom, 0, g_map_tile_size->height * 32),
    };
}

inline v2 facing_dir(Unit* unit) {
    return v2{
        g_angle_distance[unit->current_direction_1].x / 256fp,
        g_angle_distance[unit->current_direction_1].y / 256fp,
    }.norm();
}

inline v2 facing_dir(Bullet* bullet) {
    return v2{
        g_angle_distance[bullet->current_direction_1].x / 256fp,
        g_angle_distance[bullet->current_direction_1].y / 256fp,
    }.norm();
}

inline fp32 deg_to_rad(fp32 degrees) {
    return degrees * fp32::pi() / 180;
}

inline fp32 rad_to_deg(fp32 radians) {
    return radians * 180 / fp32::pi();
}

inline fp32 deg_to_dot(fp32 angle) {
    return fpm::cos(deg_to_rad(angle));
}

inline fp32 dot_to_degree(fp32 dot) {
    return rad_to_deg(fpm::acos(dot));
}

inline v2 vec_perpendicular(v2 vec) {
    return {-vec.y, vec.x};
}

inline v2 vec_reversed(v2 vec) {
    return {-vec.x, -vec.y};
}

inline bool unit_is_facing(Unit* unit, v2u16 position) {
    v2 angle = facing_dir(unit);
    v2 dir = unit->position.dir_to(position);
    fp32 dot = angle.dot(dir);
    return dot >= deg_to_dot(120fp / 2);
}

inline bool unit_is_exposed_to(Unit* unit, v2u16 position) {
    v2 angle = facing_dir(unit);
    v2 dir = unit->position.dir_to(position);
    fp32 dot = angle.dot(dir);
    return dot <= deg_to_dot(120fp / 2);
}

inline fp32 vec_length(v2 vec) {
    return vec.len();
}

//////////////////////////////////////////////////////////////// @}

/// @name Game information
//////////////////////////////////////////////////////////////// @{

/// Checks whether the game is in Brood War mode (instead of vanilla StarCraft).
inline bool is_brood_war_mode() {
    return g_is_brood_war;
}

/// Checks whether the given cheat flag is enabled.
inline bool is_cheat_enabled(u32 cheatFlag) {
    return ((*g_cheat_state) & cheatFlag) != 0;
}

/// Checks whether the game is paused.
inline bool is_game_paused() {
    return (*g_is_game_paused) != 0;
}

/// Checks whether the game is in replay mode.
inline bool in_replay() {
    return (g_is_in_replay) != 0;
}

bool is_witness();

inline bool in_replay_or_witness() {
    return in_replay() || is_witness();
}

//////////////////////////////////////////////////////////////// @}

/// @name Player information
//////////////////////////////////////////////////////////////// @{

/// Checks whether @p playerB is recognized by @p playerA as an ally.
/// Warning: The opposite may not necessarily be true!
inline bool is_allied_to(u8 a, u8 b) {
    if (a == NeutralHostilePlayer && b == NeutralHostilePlayer) return true;
    if (a == NeutralHostilePlayer || b == NeutralHostilePlayer) return false;
    return player_alliance[a].flags[b];
}

inline bool is_vision_allied_to(u8 a, u8 b) {
    if (a == NeutralHostilePlayer && b == NeutralHostilePlayer) return true;
    if (a == NeutralHostilePlayer || b == NeutralHostilePlayer) return false;
    return player_vision->flags[a] & (1 << b);
}

inline bool are_mutual_allies(u32 a, u32 b) {
    return bw::is_allied_to(a, b) && bw::is_allied_to(b, a);
}

/// Checks whether the @p unit is recognized by @p player as an enemy.
/// This will work even when the owner of the @p unit has left the game.
/// @see Unit::is_target_enemy().
inline bool is_unit_enemy(u8 player, Unit* unit) {
    //Identical to function @ 0x0047B740
    u8 owner = unit->player;
    if (owner == NeutralPlayer) {
        owner = unit->sprite->player;
    }
    return player_alliance[player].flags[owner] == 0;
}

u32 get_sized_overlay(Unit* unit, u32 initial);
/// Returns the amount of remaining supply (total available - total used) for
/// the @p player, using @p raceId to determine the appropriate race to use.
/// This is affected by the "Food For Thought" cheat flag.
s32 get_supply_remaining(u8 player, u8 raceId);

/*
u32 get_flingy_data(u32 entry, u32 index);
u32 get_sprite_data(u32 entry, u32 index);
u32 get_image_data(u32 entry, u32 index);
u32 get_unit_data(u32 entry, u32 index);
*/

//Unit* get_generic_unit(Unit* unit, u32 id);
u32 get_generic_rust_value(Unit* u, u32 val);
//u32 get_generic_value(Unit* u, u32 val);
void set_sprite_data(u32 entry, u32 index, u32 value);
s32 get_generic_variable(u32 val);
//u32 get_generic_timer(Unit* u, u32 val);
u32 get_aise_value(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2);
/*void set_generic_unit(Unit* unit, u32 id, Unit* targ);
void set_generic_value(Unit* u, u32 id, u32 val);*/
void set_generic_rust_value(Unit* u, u32 id, u32 val);
void set_generic_variable(u32 id, s32 val);
void set_generic_rust_unit(Unit* unit, u32 id, Unit* targ);
Unit* get_generic_rust_unit(Unit* unit, u32 id);
/*void set_generic_timer(Unit* u, u32 id, u32 val);

void add_generic_value(Unit* u, u32 id, s32 val);*/
void make_to_hold_position(Unit* unit);

void send_gptp_aise_command(Unit* unit1, Unit* unit2, u32 command, u32 v1, u32 v2, u32 v3);
/// Checks whether the @p player has the @p techId researched.
/// Note: This uses TechId::Enum, instead of ScTech::Enum and BwTech::Enum.
bool has_tech_researched(u8 player, u16 techId);

/// Adjusts x,y addon coordinates of quarry and similar unit ids

void adjust_addon_x(u16 addonId, u16 sourceId, u16* x);
void adjust_addon_y(u16 addonId, u16 sourceId, u16* y);
void adjust_addon_coordinates(u16 addonId, u16 sourceId, u16* x, u16* y);

u8 get_upgrade_max_level(u8 player, u8 upgradeId);
bool has_tech_available(u8 player, u16 techId);
//u32 tech_use_allowed(Unit* unit, u16 techId, u32 player);
/// Sets the player's tech research state for @p techId to @p is_researched.
/// Note: This uses TechId::Enum, instead of ScTech::Enum and BwTech::Enum.
void set_tech_research_state(u8 player, u16 techId, bool is_researched);

/// Returns the player's upgrade level of @p upgradeId. Note that this uses
/// UpgradeId::Enum, instead of ScUpgrades::Enum and BwUpgrades::Enum.
u8 get_upgrade_level(u8 player, u8 upgradeId);

/// Sets the player's upgrade level of @p upgradeId. Note that this uses
/// UpgradeId::Enum, instead of ScUpgrades::Enum and BwUpgrades::Enum.
void set_upgrade_level(u8 player, u8 upgradeId, u8 level);

//////////////////////////////////////////////////////////////// @}

/// @name Map information
//////////////////////////////////////////////////////////////// @{

/// Returns the pointer to the active tile data at (x, y).
inline ActiveTile* get_active_tile_at_coords(u32 x, u32 y) {
    return &(*g_active_tile_array)[(x / 32) + (y / 32) * g_map_tile_size->width];
}

inline ActiveTile* get_active_tile_at_coords(v2u16 point) {
    return get_active_tile_at_coords(point.x, point.y);
}

inline ActiveTile* get_active_tile_at_tile(u32 tile_x, u32 tile_y) {
    return &(*g_active_tile_array)[tile_x + tile_y * g_map_tile_size->width];
}

inline ActiveTile& get_tile_at_coords(u32 x, u32 y) {
    return (*g_active_tile_array)[(x / 32) + (y / 32) * g_map_tile_size->width];
}

inline ActiveTile& get_tile_at_coords(v2u16 point) {
    return get_tile_at_coords(point.x, point.y);
}

inline ActiveTile& get_tile_at_tile(u32 tile_x, u32 tile_y) {
    return (*g_active_tile_array)[tile_x + tile_y * g_map_tile_size->width];
}

inline ActiveTile& get_tile_at_tile(TileCoord tile) {
    return get_tile_at_tile(tile.x, tile.y);
}

inline u16& getTileIdAt(s32 x, s32 y) {
    return (*tileIdArray)[(x / 32) + g_map_tile_size->width * (y / 32)];
}

/// Returns the elevation of the tile at (x, y). 0 for low terrain, 1 for
/// medium, and 2 for high terrain.
u32 get_ground_height_at_pos(s32 x, s32 y);

//////////////////////////////////////////////////////////////// @}

/// @name Moving Units
//////////////////////////////////////////////////////////////// @{

/// Attempts to move the unit to @p (x, y), or to a nearby position if possible.
///
/// @return true if successful, false otherwise.

// standard way of moving, checks and correct unit and terrain collision
// bugs:
// 1. moving slightly will try to correct collision with itself, resulting in incorrect movement
// 2. moving onto a inside unit stunlocks it
// 3. moving onto an edge unit makes it move slightly
bool move_unit(Unit* unit, s16 x, s16 y);

// forces the move without any corrections,
// fixes bug 1 and 2,
// useful for attachment
bool move_unit_force(Unit* unit, s16 x, s16 y);

// cancels if the unit would end up on invalid terrain / collision,
// fixes bug 1,
// useful for small movements like repulsion or pulling
bool move_unit_improved(Unit* unit, s16 x, s16 y);

/// Removes various references to the @p unit.
/// Details not understood. See the source for move_unit().
void prepare_unit_move(Unit* unit, bool hide_unit = false);

// 
void prepare_unit_move_custom(Unit* unit);

/// Checks whether the given unit overlaps other units, and if so, whether it
/// can be moved to an unoccupied nearby position. The resulting position is
/// saved to @p outPos.
///
/// @return True if the unit does not collide with other units, or can be moved
///         to a nearby non-colliding position.
bool check_unit_collision_pos(Unit* unit, const v2u16* in_pos, v2u16* out_pos, Box16* move_area = 0, bool hide_error_message = false, u32 some_flag = 0);

/// Updates pathing data and various information related to the @p unit.
/// Details not understood.  See the source for move_unit().
void refresh_unit_after_move(Unit* unit);

//////////////////////////////////////////////////////////////// @}

/// @name Utility functions
//////////////////////////////////////////////////////////////// @{

/// Attempts to create a unit at the specified coordinates. If the unit cannot
/// be created (e.g. there is no space), this function displays an error message
/// and returns nullptr instead. This function is the same one used for creating
/// pre-placed units in UMS maps.
///
/// @return           The created unit or nullptr if the unit cannot be created.
//function used for broodlings and similar stuff
Unit* create_unit_in_game(u16 unit_id, u8 player, u32 x, u32 y, Unit* creator = nullptr, bool init_ai = true, bool give_massacre_resource = true);

Unit* create_hangar_unit(Unit* source, u32 unit_id);
void create_hangar_units(Unit* source, u32 count, u32 unit_id);

/// Calculates the images.dat overlay adjustment for the given unit.
/// This piece of code is used throughout StarCraft.exe for various status
/// effects, such as Ensnare, Plague, and Defensive Matrix.
u32 get_unit_overlay_adjustment(Unit* const unit);

/// Refreshes the screen position within the given rectangle box in pixels.
void refresh_screen(s32 left, s32 top, s32 right, s32 bottom);

/// Refreshes the entire screen.
void refresh_screen();

// random and random_float was moved to common.h to avoid circular include

//////////////////////////////////////////////////////////////// @}

/// @name Needs research
//////////////////////////////////////////////////////////////// @{

    /// Tells StarCraft to refresh the console (current button set, unit portrait,
    /// and status bar). not completely understood.
void refresh_console();

// Moves the camera/screen to the unit
void move_screen_to_unit(Unit* unit, u32 player = 8);

// Creates a minimap ping at x,y (relative position)
void minimap_ping(u32 x, u32 y, s32 color, u32 player = 8);

// Functionally identical to the [playfram] opcode in iscript.bin.
inline void play_frame(Image* image, u16 frameset) {
    if (image->frameset != frameset) {
        image->frameset = frameset;
        u16 frameIndex = frameset + image->direction;
        if (image->frame_index != frameIndex) {
            image->flags |= ImageFlags::Redraw;	//Order the game to redraw the image
            image->frame_index = frameIndex;
        }
    }
}

/// Sets the data of location @p locNumber.
inline void set_location(s32 locNumber, s32 left, s32 top, s32 right, s32 bottom, s32 flags) {
    Location* location = &g_location_table[locNumber];
    location->dimensions.left = left;
    location->dimensions.top = top;
    location->dimensions.right = right;
    location->dimensions.bottom = bottom;
    location->flags = flags;
}

/// Sets the data of the Anywhere location.
inline void set_anywhere_location() {
    static constexpr auto AnywhereLocation = 63;
    set_location(AnywhereLocation, 0, 0, 32 * g_map_tile_size->width, 32 * g_map_tile_size->height, 63);
}

/// Sets or clears the "is currently inside the game loop" property.
/// This must be called in next_frame() to use bw::random() there.
inline bool set_ingame_loop_state(bool new_state) {
    // Identical to function @ 0x004DC540
    b32 previous_state = g_is_in_game_loop;
    g_is_in_game_loop = (new_state ? 1 : 0);
    return previous_state != 0;
}

//////////////////////////////////////////////////////////////// @}

bool can_spawn_broodling(Unit* target);
void clone_unit_traits(Unit* new_unit, Unit* old_unit);
Unit* create_hidden_unit(UnitId::Enum unit_id, s32 count, u32 player, v2u16 position, Unit* creator, bool clone = false);
Unit* create_unit_from_bullet(UnitId::Enum unit_id, u32 player, v2u16 position, Unit* reference = nullptr);
void create_unit_from_ability(s32 unit_id, s32 count, u16 player, v2u16 position, Unit* creator = nullptr);
u16 get_last_queue_slot_id(Unit* a1);
s32 dir_saturate(s32 direction, s32 value);
fp32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon, bool default_to_max_bonus);
void set_offset_to_parent_special_overlay(Image* image);
bool is_slowed(Unit* unit);
void hide_and_disable_unit(Unit* unit);
void restore_func(Unit* unit);
void refresh_debuffs(Unit* unit);
void inherit_debuffs(Unit* parent, Unit* child);
void apply_ensnare(Unit* unit);
bool is_enabled(Unit* unit);
bool is_debuffed(Unit* unit);
s32 get_upgrade_type(Unit* unit);
s32 get_final_damage(Unit* unit, u16 weapon_id);
bool order_is_spell(s32 order_id);
void create_shield_overlay(Unit* unit, u32 attack_direction);
void decloak_proc(Unit* unit, u32 timer);
void decloak_passive_proc(Unit* unit);
void decloak_proximity_proc(Unit* unit);
bool is_passive_decloakable(Unit* unit);
bool is_proximity_decloakable(Unit* unit);
bool unit_can_nuke(Unit* unit);
UnitId::Enum get_trap_id(Unit* unit);
bool unit_is_ranged_attacker(Unit* unit);
void play_work_animation(Unit* unit);
bool unit_id_is_available_without_an_addon(u16 id);
void apply_hallucinating(Unit* unit, u8 player, u32 time);
void madrigal_dirge_fuse(Unit* unit, UnitExt* unit_ext);
void update_signify_status(Unit* unit, UnitExt* unit_ext);
bool is_unit_isolated(Unit* unit);
bool transport_is_full(Unit* transport);
bool transport_is_empty(Unit* transport);

inline u32 get_region_id(v2u16 position) {
    return helper::get_region_id(position.x, position.y);
}

inline u32 get_region_id(TileCoord tile) {
    return get_region_id(v2u16(tile.x * 32 + 16, tile.y * 32 + 16));
}

inline AiRegion* get_ai_region(u32 player, u32 index) {
    return &g_ai_region_captains[player][index];
}

inline AiRegion* get_ai_region(u32 player, v2u16 position) {
    return get_ai_region(player, get_region_id(position));
}

inline Region* get_region(u32 index) {
    return &(*g_sai_paths)->regions[index];
}

inline Region* get_region(v2u16 position) {
    return get_region(get_region_id(position));
}

inline Region* get_region(TileCoord tile) {
    return get_region(v2u16(tile.x * 32 + 16, tile.y * 32 + 16));
}

v2u16 get_region_center(Region* region);
Box32 get_region_area(Region* region);

void set_ai_preference(u16 unit_id, u32 player);
Box32 unit_area(Unit* unit);
void clear_creep(Box32 area);
u32 get_closest_resarea(v2u16 position);
AiTown* get_closest_town(u32 player, v2u16 position);
u32 get_ai_region_military_count(AiRegion* region);
void ai_defend(u32 player, u32 region_id);
void ask_for_ai_help(Unit* victim, Unit* attacker);

void spawn_custom_creep_at(v2u16 pos, u32 unit_id, bool more_creep, bool is_building);
void spawn_custom_creep(Unit* unit);
void spread_custom_creep(Unit* unit);
void update_custom_creep_disappearance(Unit* unit);
AiScript* run_ai(u32 player, Box32 area, s32 name);
AiScript* run_ai(u32 player, v2u16 pos, s32 name);
AiScript* run_default_ai(u32 player, v2u16 pos);
void give_ai_ex(Unit* unit);
u32 unit_get_creep_spreader_id(Unit* unit);
bool unit_queue_full(Unit* unit);
bool all_selected_units_have_addon(u32 unit_id);
bool unit_is_stunned(Unit* unit);
u32 get_zerg_regression(UnitId::Enum unit_id, Faction faction, bool completed, bool from_gosvileth, bool from_agrileth);
u32 get_zerg_regression(Unit* unit);
bool unit_is_allied_or_detected(Unit* unit, u32 player);
u32 get_ground_path_distance_tiles(v2u16 a, v2u16 b);
bool order_id_is_aggressive(u32 order_id);
bool ai_unit_id_can_flee_three_quarters(u32 unit_id);
bool ai_unit_id_can_flee_two_thirds(u32 unit_id);
bool buttonset_id_is_submenu(u32 buttonset_id);
u32 get_custom_secondary_weapon_from_unit_id(u32 unit_id);
void set_bullet_target(Bullet* bullet, v2u16 target);
void set_bullet_target(Bullet* bullet, Unit* target);
void set_bullet_target_follow(Bullet* bullet, Target target);
void set_bullet_target_absolute(Bullet* bullet, Target target);
void set_bullet_direction(Bullet* bullet, v2u16 target);
void set_bullet_direction(Bullet* bullet, Unit* target);
bool move_unit_ai(Unit* unit, AiRegion* ai_region);
void astral_blessing_proc(Unit* victim, u32 attacking_player_id, u32 weapon_id);
void shield_heal_proc(Unit* healer, u32 heal_source_player, u32 range, u32 heal_amount);
void heal_proc(Unit* heal_source, u32 heal_source_player, u32 range, u32 heal_amount);
bool is_attack_region(AiRegion* ai_region);
void prepare_building_area(v2u16 pos, u32 unit_id, Unit* ignore = 0);
void town_add_spending_request(AiTown& town, u32 count, u32 unit_id, u32 priority, bool request_base = true);
void military_add_spending_request(u32 player, u32 unit_id, v2u16 position, u32 priority, bool request_base = true);
void guard_add_spending_request(u32 player, GuardAi* guard_ai, u32 priority, bool request_base = true);
GuardAi* create_guard_ai(u32 player, u32 unit_id, v2u16 position);
u8 get_damage_factor_for_tooltip(u8 weapon_id, Unit* unit);
void delete_ai_script(AiScript* ai_script);
void delete_town(AiTown* ai_town);
void delete_fow_sprite(Thingy* thingy);

std::vector<Unit*> binding_plasma_find_targets(v2u16 position, Unit* reference, u32 player, u32 stacks);
bool binding_plasma_proc(Unit* victim, UnitId::Enum source_unit_id, const BindingPlasmaOptions& options);
void binding_plasma_tick(u32 player, Unit* unit, UnitId::Enum source_unit_id, const BindingPlasmaOptions& options);

bool player_has_transport(u32 player);
bool unit_in_combat_within_seconds(Unit* unit, u32 seconds);
void update_on_damage_dealt(Unit* unit, u32 damage);
void update_on_damage_taken(Unit* unit, u32 damage);
bool is_team_game_map(const char* filename);
Image* get_powerup_overlay(Sprite* sprite);
Image* get_addon_connector_image(Unit* unit);
PlayerColorCode get_player_color_code(u32 player, bool use_green_for_self);
void move_camera(v2u16 pos);
bool are_mutual_allies(u32 player1, u32 player2);

struct LineLineIntersect {
    fp32 t;
    fp32 u;
    fp32 clamped_t;
    fp32 clamped_u;
    v2 cp;
    v2 clamped_first_cp;
    v2 clamped_second_cp;
    bool on_first_line;
    bool on_second_line;
    bool on_both_lines;
};

struct LineTraceResult {
    fp32 t;
    v2u16 contact_point;
};

Quad quad_from_line(v2u16 from, v2u16 to, fp32 line_width);
Quad quad_from_rect(Box16 rect);
bool sat_collision(v2* a, u32 a_count, v2* b, u32 b_count);
void find_closest_point_to_line(v2 from, v2 to, v2 point, fp32* out_distance = 0, fp32* out_t = 0, v2* out_cp = 0);
std::optional<LineLineIntersect> line_line_intersection(v2 a0, v2 a1, v2 b0, v2 b1);
std::optional<LineTraceResult> line_trace(v2u16 from, v2u16 to, fp32 line_width, Box16 rect);
void transform_unit(Unit* unit, u16 unit_id);

void add_unit_overlay(Unit* unit, ImageSet image_set, const OverlaySettings& settings = {});
void remove_unit_overlay(Unit* unit, ImageSet image_set, const OverlaySettings& settings = {});
void set_unit_overlay(Unit* unit, ImageSet image_set, bool active, const OverlaySettings& settings = {});

PlayingSound play_sound(SoundSet sound_set, v2u16 pos = {});
PlayingSound play_sound(SoundSet sound_set, Unit* unit, bool force_globally = false);
PlayingSound play_looping_sound(SoundSet sound_set, v2u16 pos = {});
PlayingSound play_looping_sound(SoundSet sound_set, Unit* unit, bool force_globally = false);

void play_action_sound(u32 sound_id, Unit* unit);
u32 get_unit_weapon_based_on_target(Unit* attacker, Unit* target);
void regress(Unit* unit, u32 regression_id);
void unfreeze_channeling_unit(Unit* unit);
bool unit_can_flee(Unit* unit);
GameKind get_game_kind();
u32 get_worker_harvest_distance(Unit* unit, v2u16 target);
v2u16 get_rect_edge_point_towards(v2u16 pos, Box16 rect, v2u16 target);
u32 get_fore_castle_armor_bonus(Unit* unit);
u32 get_ensnare_factor(Unit* unit);
u32 get_advisor_sound_from_event_type(u32 player, u32 event_type);
u32 get_advisor_sound_from_message_id(u32 player, u32 message_id);
bool has_custom_advisor(u32 player);
void draw_shape(u32 i_0, float scale, Point32* offset, u8 color);
void draw_weapon_range(Unit* unit, u8 weapon_id, u8 color);
void draw_aura_range(Unit* unit, u16 radius, u8 color);
void reveal_attacker(Unit* attacker, Unit* victim);
void unload_or_kill_loaded_units(Unit* transport);
u32 get_cloak_sound_id(Unit* unit);
u32 get_decloak_sound_id(Unit* unit);
void launch_return_missile(Unit* unit, u32 weapon_id);
void fire_incremental_bullet(Bullet* bullet, u32 weapon_id, Unit* target, v2u16 position);
bool client_selection_id_check(u32 unit_id);
u32 client_selection_count();
u32 client_selection_unit_id_count(u32 unit_id);
u32 player_selection_count(u32 player);
u32 player_selection_unit_id_count(u32 player, u32 unit_id);
u32 get_witness_count();
void add_massacre_resource(u32 player_id);
bool game_kind_supports_kills(GameKind game_kind);
bool unit_can_fire_secondary_weapon(Unit* unit);
void cancel_targeting_and_place_building();
void select(std::vector<Unit*>& units, bool play_response);
u32 get_unit_weapon_fire_sound(Unit* unit, u32 weapon_id);
u32 get_weapon_fire_sound(u32 weapon_id, Unit* unit);
bool unit_regenerates_hp(Unit* unit);
fp32 get_regeneration_factor(Unit* unit);
bool unit_has_shields(Unit* unit);
bool can_show_custom_ui();
bool player_is_witness(u32 player);
bool player_is_active(u32 player);
std::string get_commit_hash();
bool check_building_collision(UnitId::Enum unit_id, u32 player, v2u16 pos);
v2u16 align_to_tile_grid(v2u16 pos, UnitId::Enum unit_id);
u32 active_player_count();
u32 human_count();
u32 human_or_defeated_human_count();
u32 human_or_witness_count();
bool unit_id_blocked_in_dark_ages(UnitId::Enum unit_id);
fp32 calculate_dark_ages_penalty(fp32 range);
void zap_overlay(Unit* unit);
void play_manual_weaponfire_sound(Unit* unit, WeaponId::Enum weapon_id);
void play_manual_weaponhit_sound(Unit* unit, WeaponId::Enum weapon_id);
u32 get_weapon_hit_sound(WeaponId::Enum weapon_id, Unit* unit);
v2u16 get_building_reposition(Unit* unit, v2u16 position);
fp32 unit_id_total_cost(u32 unit_id, Faction faction, bool regression_unstable);
bool is_melee_game(u8 game_type, u8 game_subtype);
bool minerals_exist_for_player(u32 player);
void set_merge_move_target(Unit* unit, Unit* target);
} // namespace bw
