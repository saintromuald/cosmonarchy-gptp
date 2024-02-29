#include "data_ext.h"

#define AiTownCap 100

static UnitExt unit_ext_array[UNIT_ARRAY_LENGTH];
static TileExt tile_ext_array[256][256];
static BulletExt bullet_ext_array[BULLET_ARRAY_LENGTH];
static ImageExt image_ext_array[IMAGE_ARRAY_LENGTH];
static SpriteExt sprite_ext_array[SPRITE_ARRAY_LENGTH];
static ThingyExt thingy_ext_array[THINGY_ARRAY_LENGTH];
static AiTownExt ai_town_ext_array[PlayerLimit][AiTownCap];

void init_ext_variables() {
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i) {
		reset_unit_ext(&unit_ext_array[i]);
	}
	memset(tile_ext_array, 0, sizeof(tile_ext_array));
	// TODO: reset tiles?
	for (u32 i = 0; i < IMAGE_ARRAY_LENGTH; ++i) {
		reset_image_ext(&g_image_table[i]);
	}
	for (u32 i = 0; i < SPRITE_ARRAY_LENGTH; ++i) {
		reset_sprite_ext(&g_sprite_table[i]);
	}
	for (u32 i = 0; i < THINGY_ARRAY_LENGTH; ++i) {
		reset_thingy_ext(&g_thingy_table[i]);
	}
	// TODO: reset ai towns?
}

void update_ext_variables() {
	// Units
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i) {
		UnitExt* unit_ext = &unit_ext_array[i];
		if (unit_ext->pause_timers) {
			continue;
		}

		if (unit_ext->energy_regen_timer != UINT_MAX) {
			unit_ext->energy_regen_timer++;
		}
		if (unit_ext->shield_regen_timer != UINT_MAX) {
			unit_ext->shield_regen_timer++;
		}
		if (unit_ext->damage_taken_timer != UINT_MAX) {
			unit_ext->damage_taken_timer++;
		}
		if (unit_ext->damage_dealt_timer != UINT_MAX) {
			unit_ext->damage_dealt_timer++;
		}
		for (u32 j = 0; j < PlayerLimit; ++j) {
			if (unit_ext->last_seen_timer[j] != UINT_MAX) {
				++unit_ext->last_seen_timer[j];
			}
			if (unit_ext->ai_timer_avoid_targeting[j]) {
				unit_ext->ai_timer_avoid_targeting[j] -= 1;
			}
		}
		for (u32 j = 0; j < TimerCount; ++j) {
			if (unit_ext->timers[j] > 0) {
				unit_ext->timers[j]--;
			}
		}
		for (u32 j = 0; j < TimerVectorCount; ++j) {
			for (auto& timer : unit_ext->timer_vectors[j]) {
				if (timer > 0) {
					timer--;
				}
			}
			unit_ext->timer_vectors[j].erase(std::remove_if(unit_ext->timer_vectors[j].begin(), unit_ext->timer_vectors[j].end(), [&](u32& timer) {
				return timer == 0;
			}), unit_ext->timer_vectors[j].end());
		}
	}

	// Tiles
	for (u32 y = 0; y < g_map_tile_size->height; ++y) {
		for (u32 x = 0; x < g_map_tile_size->width; ++x) {
			TileExt* tile_ext = get_tile_ext_at_tile(x, y);
			tile_ext->creep_owners = 0;
		}
	}

	// Bullets
	for (u32 i = 0; i < BULLET_ARRAY_LENGTH; ++i) {

	}

	// Sprites
	for (u32 i = 0; i < SPRITE_ARRAY_LENGTH; ++i) {

	}

	// Images
	for (u32 i = 0; i < IMAGE_ARRAY_LENGTH; ++i) {

	}

	// Thingies
	for (u32 i = 0; i < THINGY_ARRAY_LENGTH; ++i) {
		Thingy* thingy = &g_thingy_table[i];
		if (!thingy->sprite) {
			continue;
		}

		ThingyExt* thingy_ext = get_thingy_ext(thingy);
		thingy_ext->activity_timer++;
		if (is_inactive_corpse(thingy)) {
			thingy->sprite->flags |= SpriteFlags::RemoveAfterDeath;
			thingy->sprite->play_iscript_animation(IscriptAnimation::Death);
		}

		if (bw::get_game_kind() == GameKind_TheTurning && thingy_ext->activity_timer >= 24) {
			if (!(units_dat::base_property[thingy_ext->unit_id] & UnitProperty::Building)) {
				reclaim_corpse(thingy, TheTurningZombiePlayerId);
			}
		}

		if (thingy_ext->avenge_or_ascend_player && thingy_ext->activity_timer >= 24) {
			avenge_or_ascend_corpse(thingy, thingy_ext->avenge_or_ascend_player - 1);
		}
	}
}

UnitExt* get_unit_ext(Unit* unit) {
	u32 index = unit - g_unit_table;
	assert(unit && index >= 0 && index < UNIT_ARRAY_LENGTH);
	return &unit_ext_array[index];
}

void reset_unit_ext(UnitExt* unit_ext) {
	*unit_ext = {};
	// every unit will start being out of combat
	unit_ext->shield_regen_timer = UINT_MAX;
	unit_ext->damage_taken_timer = UINT_MAX;
	unit_ext->damage_dealt_timer = UINT_MAX;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		unit_ext->last_seen_timer[i] = UINT_MAX;
	}
}

void unit_death_start(Unit* unit) {
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i) { // set dead unit pointers to nullptr and erase them from unit vectors
		UnitExt* other_ext = &unit_ext_array[i];
		for (u32 i = 0; i < UnitCount; ++i) {
			if (other_ext->units[i] == unit) {
				other_ext->units[i] = nullptr;
			}
		}

		for (u32 i = 0; i < UnitVectorCount; ++i) {
			auto& vec = other_ext->unit_vectors[i];
			vec.erase(std::remove_if(vec.begin(), vec.end(), [&](Unit* vecUnit) {
				return vecUnit == unit;
			}), vec.end());
		}
	}

	if (unit->sprite) {
		SpriteExt* sprite_ext = get_sprite_ext(unit->sprite);
		if (sprite_ext->owner == unit) {
			sprite_ext->owner = 0;
		}
	}

	UnitExt* unit_ext = get_unit_ext(unit);
	unit_ext->pause_timers = true;
	if (unit_id_is_capped_gas_structure(unit->id)) {
		unit_death_end(unit);
	}
}

void unit_death_end(Unit* unit) {
	reset_unit_ext(get_unit_ext(unit));
}

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y) {
	return &tile_ext_array[tile_y][tile_x];
}

TileExt* get_tile_ext_at_coords(u32 x, u32 y) {
	return &tile_ext_array[y / 32][x / 32];
}

BulletExt* get_bullet_ext(Bullet* bullet) {
	u32 index = bullet - g_bullet_table;
	assert(bullet && index >= 0 && index < BULLET_ARRAY_LENGTH);
	return &bullet_ext_array[index];
}

void reset_bullet_ext(Bullet* bullet) {
	BulletExt* bullet_ext = get_bullet_ext(bullet);
	*bullet_ext = {};
}

SpriteExt* get_sprite_ext(Sprite* sprite) {
	s32 index = sprite - g_sprite_table;
	assert(sprite && index >= 0 && index < SPRITE_ARRAY_LENGTH);
	return &sprite_ext_array[index];
}

void reset_sprite_ext(Sprite* sprite) {
	SpriteExt* sprite_ext = get_sprite_ext(sprite);
	*sprite_ext = {};
}

ImageExt* get_image_ext(Image* image) {
	s32 index = image - g_image_table;
	assert(image && index >= 0 && index < IMAGE_ARRAY_LENGTH);
	return &image_ext_array[index];
}

void reset_image_ext(Image* image) {
	ImageExt* image_ext = get_image_ext(image);
	*image_ext = {};
}

ThingyExt* get_thingy_ext(Thingy* thingy) {
	s32 index = thingy - g_thingy_table;
	assert(thingy && index >= 0 && index < THINGY_ARRAY_LENGTH);
	return &thingy_ext_array[index];
}

void reset_thingy_ext(Thingy* thingy) {
	ThingyExt* thingy_ext = get_thingy_ext(thingy);
	*thingy_ext = {};
}

AiTownExt* get_town_ext(AiTown* town) {
	s32 player = town->player;
	s32 index = town - g_ai_town_list[player].array->towns;
	assert(town && player >= 0 && player < PlayerLimit && index >= 0 && index < AiTownCap);
	return &ai_town_ext_array[player][index];
}

void reset_town_ext(AiTown* town) {
	AiTownExt* town_ext = get_town_ext(town);
	*town_ext = {};
}

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player) {
	u32 allied_bits = 0;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		if (bw::is_allied_to(i, player)) {
			allied_bits |= 1 << i;
		}
	}
	return !tile_ext->creep_owners || tile_ext->creep_owners & allied_bits;
}