// Contains wrappers for various SCBW internal functions, to be used in
// hook functions.

#pragma once
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations.h>
#include <string>
#include <math.h>
#include <unit_search.h>

#define NOMINMAX
#include <windows.h>

namespace scbw
{
	bool is_any_morph_result(u16 id);
	bool is_conventional_unit_morph_result(u16 id);
	bool is_conventional_unit_morph_source(u16 id, bool allow_larvae = false, bool allow_high_templar = false);
	bool is_zerg_building_morph_result(u16 id);
	bool is_zerg_building_morph_source(u16 id);
	bool is_larval_egg(Unit* unit);
	bool is_zerg_egg_new(Unit* unit, bool allow_larval_eggs = true, bool allow_larvae = true, u16 explicit_unit_id = UnitId::None);
	bool is_zerg_egg(u16 id, bool allow_normal_eggs = true, bool allow_larvae = true);
	void artificial_bullet_from(u8 weaponId, u8 playerId, Unit* source, Unit* target, Point16 target_pt);
	void artificial_bullet_from(u8 weaponId, u8 playerId, Unit* source, Unit* target, u16 x, u16 y);
	std::string read_samase_file(char* name);
	bool teleport(Unit* u, Point16 target, u16 spriteAnimation = 0, bool preserveOrder = false);
	Thingy* create_and_init_thingy(u32 spriteId, s16 x, s16 y, u32 playerId = 11, s32 elevationLevel = 11);
	Point16 rotate_point(Point16 anchor, Point16 point, f64 angle_degrees);
	Vector32 rotate_point(Vector32 anchor, Vector32 point, f64 angle_degrees);

	/// @name Output functions
	//////////////////////////////////////////////////////////////// @{
		
	/// Prints text to the screen, optionally using the given text color.
	void print_string(std::string text, u32 color = GameTextColor::White);
	void print(const char* text, u32 color = GameTextColor::White);
	void printf(const char* format, ...);
	void printf_color(const char* format, u32 color, ...);

	void printf_console(const char* fmt, ...);

	/// Displays an error message and plays a sound for the player.
	///
	/// @param  player  ID of the player who receives the message.
	/// @param  statTxtId ID of the error message string in stat_txt.tbl
	/// @param  sfxId     ID of the sound file in sfxdata.dat.
	void show_error_message_with_sfx(u32 playerId, u32 statTxtId, u32 sfxId);

	//////////////////////////////////////////////////////////////// @}

	u8 has_overlay(Unit* unit);

	void disable_dialog(BinDlg* dialog);															//18640
	void show_dialog(BinDlg* dialog);															//186A0
	void hide_dialog(BinDlg* dialog);															//18700
	void update_dialog(BinDlg* dialog);															//1C400
	/// @name Unit checks
	//////////////////////////////////////////////////////////////// @{

	/// Checks whether the transport can hold this unit.
	///
	/// This function checks whether [transport] has any status effects (Lockdown,
	/// Maelstrom, etc.), is owned by the same player, and has enough space for
	/// [unit]. It does NOT check whether [transport] is a hallucination, or is an
	/// Overlord without the transport upgrade, or whether the [unit] itself has any
	/// status effects. Note that this function is affected by the EXE edits
	/// "change bunker size check to organic flag check" and
	/// "remove bunker size check" in FireGraft.
	bool can_be_entered_by(Unit* transport, Unit* unit);

	/// Checks weapons.dat targeting flags to see if the @p target can be hit by the
	/// @p weapon.
	///
	/// @param  weaponId  If this is an invalid weapons.dat ID, returns false.
	/// @param  target    If nullptr is passed, the function checks whether the
	///                   weapon can target a position on the ground. If @p target
	///                   is invincible, the function returns false.
	bool can_weapon_target_unit(u8 weaponId, Unit* target = nullptr,
		Unit* attacker = nullptr);

	/// Checks if @p unit is under a Dark Swarm. This does NOT check whether the
	/// unit is a ground unit or a building.
	bool is_under_dark_swarm(Unit* unit);
	Unit* get_dark_swarm_unit(Unit* unit);
	Unit* get_dark_swarm_from_position(u16 x, u16 y, u16 radius);
	//Checks if the @p target can be targeted by player @p castingPlayer with
	//tech/spell @p techId
	//If yes, return 0, else return error message id in stat_txt.tbl to use
	u16 get_tech_user_error_message(Unit* target, u8 castingPlayer, u16 techId);

	//////////////////////////////////////////////////////////////// @}

	/// @name Graphics and geometry
	//////////////////////////////////////////////////////////////// @{

	/// Calculates the angle (in binary radians) of the arrow that starts at
	/// (xStart, yStart) and ends at (xEnd, yEnd).
	s32 get_angle(s32 xStart, s32 yStart, s32 xEnd, s32 yEnd);

	inline s32 get_angle(Point16 start, Point16 end)
	{
		return get_angle(start.x, start.y, end.x, end.y);
	}

	inline s32 get_angle(Vector32 start, Vector32 end)
	{
		return get_angle(start.x, start.y, end.x, end.y);
	}

	inline f64 bw_angle_to_normal(s32 bw_angle);
	/// StarCraft's internal function used to quickly calculate distances between
	/// two points (x1, y1) and (x2, y2).
	/// Warning: This function is inaccurate for long distances.
	u32 get_distance_fast(s32 x1, s32 y1, s32 x2, s32 y2);

	inline u32 get_distance_fast(Point16 p1, Point16 p2)
	{
		return get_distance_fast(p1.x, p1.y, p2.x, p2.y);
	}

	/// Distance, but in a square instead of circle (max of x dist and y dist)
	inline u32 get_distance_square(s32 x1, s32 y1, s32 x2, s32 y2);

	/// Calculates the X Cartesian coordinate from the given polar coordinates
	/// (distance, angle), using StarCraft's internal data.
	inline s32 get_polar_x(s32 distance, u8 angle);

	/// Calculates the Y Cartesian coordinate from the given polar coordinates
	/// (distance, angle), using StarCraft's internal data.
	inline s32 get_polar_y(s32 distance, u8 angle);

	inline Point16 clamp_point(Point16 point)
	{
		if (point.x < 0)
		{
			point.x = 0;
		}
		else if (point.x > map_tile_size->width * 32)
		{
			point.x = map_tile_size->width * 32;
		}

		if (point.y < 0)
		{
			point.y = 0;
		}
		else if (point.y > map_tile_size->height * 32)
		{
			point.y = map_tile_size->height * 32;
		}

		return point;
	}

	inline Box32 clampRect(Box32 rect)
	{
		if (rect.left < 0)
		{
			rect.left = 0;
		}
		else if (rect.left > map_tile_size->width * 32)
		{
			rect.left = map_tile_size->width * 32;
		}
		
		if (rect.right < 0)
		{
			rect.right = 0;
		}
		else if (rect.right > map_tile_size->width * 32)
		{
			rect.right = map_tile_size->width * 32;
		}

		if (rect.top < 0)
		{
			rect.top = 0;
		}
		else if (rect.top > map_tile_size->height * 32)
		{
			rect.top = map_tile_size->height * 32;
		}

		if (rect.bottom < 0)
		{
			rect.bottom = 0;
		}
		else if (rect.bottom > map_tile_size->height * 32)
		{
			rect.bottom = map_tile_size->height * 32;
		}

		return rect;
	}

	inline Vector32 get_vector_between_points(Point16 source, Point16 target)
	{
		Vector32 diff = { target.x, target.y };
		diff.x -= source.x;
		diff.y -= source.y;
		return diff;
	}

	inline Vector32f get_normalized_vector(Vector32 vector)
	{
		f32 dist = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
		return { vector.x / dist, vector.y / dist };
	}

	inline Vector32f get_normalized_vector(Vector32f vector)
	{
		f32 dist = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
		return { vector.x / dist, vector.y / dist };
	}

	inline f32 dot_product(Vector32f a, Vector32f b)
	{
		return a.x * b.x + a.y * b.y;
	}

	inline f32 get_vector_distance(Vector32 vector)
	{
		return sqrt(pow(vector.x, 2) + pow(vector.y, 2));
	}

	inline f32 get_vector_distance(Vector32f vector)
	{
		return sqrt(pow(vector.x, 2) + pow(vector.y, 2));
	}

	inline Vector32f getUnitDirectionVector(Unit* unit)
	{
		return { (f32)sin(scbw::bw_angle_to_normal(unit->current_direction_1) * 3.14159f / 180.0f),
			(f32)-cos(scbw::bw_angle_to_normal(unit->current_direction_1) * 3.14159f / 180.0f) };
	}

	//////////////////////////////////////////////////////////////// @}

	/// @name Game information
	//////////////////////////////////////////////////////////////// @{

	/// Checks whether the game is in Brood War mode (instead of vanilla StarCraft).
	inline bool is_brood_war_mode()
	{
		return (*bw_is_brood_war) != 0;
	}

	/// Checks whether the given cheat flag is enabled.
	inline bool is_cheat_enabled(u32 cheatFlag)
	{
		return ((*bw_cheat_state) & cheatFlag) != 0;
	}

	/// Checks whether the game is paused.
	inline bool is_game_paused()
	{
		return (*bw_is_game_paused) != 0;
	}

	/// Checks whether the game is in replay mode.
	inline bool isInReplay()
	{
		return (*bw_is_in_replay) != 0;
	}

	//////////////////////////////////////////////////////////////// @}

	/// @name Player information
	//////////////////////////////////////////////////////////////// @{

	/// Checks whether @p playerB is recognized by @p playerA as an ally.
	/// Warning: The opposite may not necessarily be true!
	inline bool is_allied_to(u8 playerA, u8 playerB)
	{
		return 0 != player_alliance[playerA].flags[playerB];
	}

	inline bool is_vision_allied_to(u8 playerA, u8 playerB)
	{
		return 0 != player_vision[playerA].flags[playerB];
	}

	/// Checks whether the @p unit is recognized by @p player as an enemy.
	/// This will work even when the owner of the @p unit has left the game.
	/// @see Unit::is_target_enemy().
	inline bool is_unit_enemy(u8 playerId, Unit* unit)
	{
		//Identical to function @ 0x0047B740
		u8 unitOwner = unit->player;
		if (unitOwner == 11)
			unitOwner = unit->sprite->playerId;
		return 0 == player_alliance[playerId].flags[unitOwner];
	}

	u32 get_sized_overlay(Unit* unit, u32 initial);
	/// Returns the amount of remaining supply (total available - total used) for
	/// the @p player, using @p raceId to determine the appropriate race to use.
	/// This is affected by the "Food For Thought" cheat flag.
	s32 get_supply_remaining(u8 playerId, u8 raceId);
	u32 get_flingy_data(u32 entry, u32 index);

	u32 get_sprite_data(u32 entry, u32 index);
	u32 get_image_data(u32 entry, u32 index);
	u32 get_unit_data(u32 entry, u32 index);
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
	bool has_tech_researched(u8 playerId, u16 techId);

	/// Adjusts x,y addon coordinates of quarry and similar unit ids

	void adjust_addon_x(u16 addonId, u16 sourceId, u16* x);
	void adjust_addon_y(u16 addonId, u16 sourceId, u16* y);
	void adjust_addon_coordinates(u16 addonId, u16 sourceId, u16* x, u16* y);

	u8 get_upgrade_max_level(u8 playerId, u8 upgradeId);
	bool has_tech_available(u8 playerId, u16 techId);
	//u32 tech_use_allowed(Unit* unit, u16 techId, u32 player);
	/// Sets the player's tech research state for @p techId to @p isResearched.
	/// Note: This uses TechId::Enum, instead of ScTech::Enum and BwTech::Enum.
	void set_tech_research_state(u8 playerId, u16 techId, bool isResearched);

	/// Returns the player's upgrade level of @p upgradeId. Note that this uses
	/// UpgradeId::Enum, instead of ScUpgrades::Enum and BwUpgrades::Enum.
	u8 get_upgrade_level(u8 playerId, u8 upgradeId);

	/// Sets the player's upgrade level of @p upgradeId. Note that this uses
	/// UpgradeId::Enum, instead of ScUpgrades::Enum and BwUpgrades::Enum.
	void set_upgrade_level(u8 playerId, u8 upgradeId, u8 level);

	//////////////////////////////////////////////////////////////// @}

	/// @name Map information
	//////////////////////////////////////////////////////////////// @{

	/// Returns the pointer to the active tile data at (x, y).
	inline ActiveTile& get_stupid_reference_to_active_tile_at(s32 x, s32 y)
	{
		return (*activeTileArray)[(x / 32) + map_tile_size->width * (y / 32)];
	}

	inline ActiveTile* get_active_tile_at_coords(u32 x, u32 y) {
		return &(*activeTileArray)[(x / 32) + (y / 32) * map_tile_size->width];
	}

	inline ActiveTile* get_active_tile_at_tile(u32 tile_x, u32 tile_y) {
		return &(*activeTileArray)[tile_x + tile_y * map_tile_size->width];
	}

	inline u16& getTileIdAt(s32 x, s32 y)
	{
		return (*tileIdArray)[(x / 32) + map_tile_size->width * (y / 32)];
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
	// 2. moving onto a ground unit stunlocks it
	// 3. moving onto an air unit makes it move slightly
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
	void prepare_unit_move(Unit* unit, bool hideUnit = false);

	// 
	void prepare_unit_move_custom(Unit* unit);

	/// Checks whether the given unit overlaps other units, and if so, whether it
	/// can be moved to an unoccupied nearby position. The resulting position is
	/// saved to @p outPos.
	///
	/// @return True if the unit does not collide with other units, or can be moved
	///         to a nearby non-colliding position.
	bool check_unit_collision_pos(Unit* unit, const Point16* inPos, Point16* outPos, Box16* moveArea = nullptr, bool hideErrorMsg = false, u32 someFlag = 0);

	/// Moves the unit's position to @p (x, y). If the unit is a ground unit and the
	/// the target position is on unwalkable terrain, this function moves the unit
	/// to the nearest walkable tile instead.
	void set_unit_position(Unit* unit, u16 x, u16 y);

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
	Unit* create_unit_in_game(u16 unit_id, u8 player, u32 x, u32 y, Unit* creator = nullptr, bool initAI = true);

	Unit* create_hangar_unit(Unit* source, u32 unit_id);
	void create_hangar_units(Unit* source, u32 count, u32 unit_id);

	/// Calculates the images.dat overlay adjustment for the given unit.
	/// This piece of code is used throughout StarCraft.exe for various status
	/// effects, such as Ensnare, Plague, and Defensive Matrix.
	u32 get_unit_overlay_adjustment(Unit* const unit);

	/// Refreshes the screen position within the given rectangle area in pixels.
	void refresh_screen(s32 left, s32 top, s32 right, s32 bottom);

	/// Refreshes the entire screen.
	void refresh_screen();

	/// Generates a pseudorandom number between min and max, inclusive. This is
	/// identical to the function used internally by StarCraft, and is guaranteed to
	/// generate the same sequence in a replay.
	/// NOTE: The RNG has a maximum distance of 0 - 32767.
	u32 rand_between(u32 min, u32 max);

	/// Generates a pseudorandom number between 0 and 32767, inclusive. This is
	/// identical to the function used internally by StarCraft, and is guaranteed to
	/// generate the same sequence in a replay.
	u16 random();

	//////////////////////////////////////////////////////////////// @}

	/// @name Needs research
	//////////////////////////////////////////////////////////////// @{

		/// Tells StarCraft to refresh the console (current button set, unit portrait,
		/// and status bar). not completely understood.
	void refresh_console();

	// Moves the camera/screen to the unit
	void move_screen_to_unit(Unit* unit, u32 playerId = 8);

	// Creates a minimap ping at x,y (relative pos)
	void minimap_ping(u32 x, u32 y, s32 color, u32 playerId = 8);

	// Functionally identical to the [playfram] opcode in iscript.bin.
	inline void playFrame(Image* image, u16 frameset)
	{
		if (image->frameSet != frameset)
		{
			image->frameSet = frameset;
			u16 frameIndex = frameset + image->direction;
			if (image->frameIndex != frameIndex)
			{
				image->flags |= ImageFlags::Redraw;	//Order the game to redraw the image
				image->frameIndex = frameIndex;
			}
		}
	}

	/// Sets the data of the Anywhere location.
	inline void setAnywhereLocation()
	{
		Location* location = &locationTable[63];
		location->dimensions.left = 0;
		location->dimensions.top = 0;
		location->dimensions.right = 32 * map_tile_size->width;
		location->dimensions.bottom = 32 * map_tile_size->height;
		location->flags = 63;
	}

	/// Sets the data of location @p locNumber.
	inline void setLocation(s32 locNumber, s32 left, s32 top, s32 right, s32 bottom, s32 flags)
	{
		Location* location = &locationTable[locNumber];
		location->dimensions.left = left;
		location->dimensions.top = top;
		location->dimensions.right = right;
		location->dimensions.bottom = bottom;
		location->flags = flags;
	}

	/// Sets or clears the "is currently inside the game loop" property.
	/// This must be called in next_frame() to use scbw::random() there.
	inline bool set_ingame_loop_state(bool newState)
	{
		//Identical to function @ 0x004DC540
		b32 previousState = *is_in_game_loop;
		*is_in_game_loop = (newState ? 1 : 0);
		return previousState != 0;
	}

	//////////////////////////////////////////////////////////////// @}

	void range_modified_fire(u8 weaponId, Unit* unit, s16 x, s16 y, u32 offset);
	bool can_spawn_broodling(Unit* target);
	s32 get_broodling_count_from_supply(Unit* target);
	void create_unit_from_ability(s32 unit_id, s32 count, u16 playerId, s32 x, s32 y, u32 remove_timer = 1800, Unit* creator = nullptr);
	Unit** get_source_matraleths(Unit* unit);
	u16 get_last_queue_slot_id(Unit* a1);
	s32 dir_saturate(s32 direction, s32 value);
	bool can_use_cliff_advantage(Unit* unit, u8 weapon);
	s32 get_cliff_advantage(Unit* unit, Unit* target_unit, u8 weapon);
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
	s32 get_overlay_offset(u16 unit_id, bool xlargeAllowed = false);
	s32 get_final_damage(Unit* unit, u16 weaponId);
	bool unit_id_is_pacifist(Unit* unit);
	bool order_is_spell(s32 order_id);
	bool can_be_parasited(Unit* unit);
	void apply_parasite(Unit* unit, u8 playerID, Unit* queen);
	void create_shield_overlay(Unit* unit, u32 attackDirection);
	void decloak_proc(Unit* unit);
	bool is_decloakable(Unit* unit);
	bool unit_can_nuke(Unit* unit);
	bool unit_is_trap(Unit* unit);
	bool unit_is_ranged_attacker(Unit* unit);
	void play_work_animation(Unit* unit);
	bool unit_id_is_available_without_an_addon(u16 id);
	void apply_new_hallucination(Unit* unit, u8 playerId, u32 time);
	bool weapon_pierces_targets(u16 weaponId);
	void madrigal_dirge_fuse(Unit* unit, UnitExt* unitExt);
	void update_signify_status(Unit* unit, UnitExt* unitExt);
	bool is_unit_isolated(Unit* unit);
	bool unit_id_is_zerg_supply(u16 unit_id);
	void mind_control(Unit* source, Unit* target);
	void recede_mind_control(Unit* target);
	bool is_zerg_production(u16 unit_id);
	bool unit_id_is_town_center(u16 unit_id);
	bool unit_id_is_shared(u16 unit_id);
	bool transport_is_full(Unit* transport);
	bool transport_is_empty(Unit* transport);
	bool unit_id_uses_xlarge_overlays(u16 unit_id);
	bool unit_id_is_resource_node(u16 unit_id, bool count_gas_structures);

	void set_cmbw_ai_preference(u16 unit_id, u32 player);
	Box32 unit_area(Unit* unit);
	void clear_creep(Box32 area);
	u32 get_closest_resarea(Point16 position);
	AiTown* get_closest_town(u32 player, Point16 position);
	u32 get_region_id(Point16 position);
	AiRegion* get_ai_region(u32 player, u32 index);
	u32 get_ai_region_military_count(AiRegion* region);
	Region* get_region(u32 index);
	Point16 get_region_center(Region* region);
	void ai_defend(u32 player, u32 region_id, u32 priority, bool recurse);
	void ask_for_ai_help(Unit* victim, Unit* attacker);
	bool unit_id_is_capped_gas_structure(u16 unit_id);
	bool unit_id_is_gas_structure(u16 unit_id);
	bool unit_id_is_smaller_building(u16 unit_id);
	EntryArray get_writhing_web_sovroleths(Unit* unit);

	void spawn_custom_creep(Unit* unit);
	void spread_custom_creep(Unit* unit);
	void update_custom_creep_disappearance(Unit* unit);
	void blissful_embrace(Unit* target, u32 player);

	AiScript* run_ai(u32 player, Box32 area, s32 name);
	AiScript* run_ai(u32 player, Point16 pos, s32 name);
	
	b32 unit_id_is_tech(u32 unit_id);
	b32 unit_id_is_tech_tier_structure(u32 unit_id);

	void give_ai_ex(Unit* unit, bool prefer_military);

	u32 unit_get_creep_spreader_id(Unit* unit);

	bool sprite_id_is_corpse(u32 sprite_id);

	bool unit_queue_full(Unit* unit);
	bool all_selected_units_have_addon(u32 unit_id);

	bool unit_is_stunned(Unit* unit);
}