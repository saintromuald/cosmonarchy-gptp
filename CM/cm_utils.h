#pragma once
#include "cm.h"
#include "BW/api.h"

enum class GameKind;
namespace bw {
GameKind get_game_kind();
} //bw

Box32 box32_rect(Point16 pos, u32 w, u32 h);
Box32 box32_square(Point16 pos, u32 w);

bool is_hostile_or_neutral(u32 player, u32 other_player);
void replay_witness_event(Unit* unit, SpectatorEvent event_type);
u32 get_construction_graphic(Unit* unit);
void init_omnipresence();
void init_duopoly();
void disconnect_addon_no_event(Unit* unit);
u32 get_bullet_damage(Bullet* bullet, u32 weapon_id);
const char* game_kind_to_string(GameKind game_kind);
const char* game_kind_to_filename_string(GameKind game_kind);
const char* race_to_string(RaceId::Enum race);
const char* race_to_table_string(RaceId::Enum race);
bool order_id_ignores_collision(u32 order_id);
bool unit_should_be_autoattacked_by_player(Unit* unit, u32 player);
void cancel_all_training(Unit* unit);
bool play_unit_banter(Unit* victim, u16 speaker_id);
void rescue_unit(Unit* unit, u32 player);
bool can_auto_heal(Unit* unit);
bool unit_can_be_repulsed(Unit* unit);
void specialist_energy_error(Unit* unit);
bool unit_should_give_gas_in_massacre_mode(Unit* unit);
bool game_kind_has_neutral_hostile(GameKind game_kind = bw::get_game_kind());
bool player_detects_unit(u32 player, Unit* unit);
void fire_amitosis_weapon(Unit* attacker, WeaponId::Enum weapon_id);
void set_up_mines(Unit* unit);
UnitId::Enum get_factional_unit_id(u32 player, UnitId::Enum unit_id);
u32 get_modified_damage(Unit* unit, WeaponId::Enum weapon_id);
const char* get_unit_name_ext(UnitId::Enum unit_id);
u32 get_banter_cooldown(UnitId::Enum unit_id);
UnitId::Enum get_factional_regression(Unit* unit);
void init_corpse(Thingy* corpse, SpriteId::Enum id, u32 player);
bool game_kind_has_zombies(GameKind game_kind = bw::get_game_kind());
bool game_kind_doubles_units(GameKind game_kind = bw::get_game_kind());
bool chat_message_is_allowed(const char* text);
Faction get_random_faction(RaceId::Enum race);

namespace WeaponId {
const char* get_name(Unit* unit, WeaponId::Enum weapon_id);
} // namespace WeaponId

UnitId::Enum get_factional_regression(UnitId::Enum unit_id, Faction faction);
UnitId::Enum get_factional_regression(Unit* unit);
