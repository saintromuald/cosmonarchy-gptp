#include "data_ext.h"

static UnitExt unit_ext_array[UNIT_ARRAY_LENGTH];
static TileExt tile_ext_array[256][256];
static BulletExt bullet_ext_array[BULLET_ARRAY_LENGTH];
static SpriteExt sprite_ext_array[SPRITE_ARRAY_LENGTH];
static ImageExt image_ext_array[IMAGE_ARRAY_LENGTH];

void init_ext_variables() {
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i) {
		reset_unit_ext(&g_unit_table[i]);
	}
	memset(tile_ext_array, 0, sizeof(tile_ext_array));
	for (u32 i = 0; i < SPRITE_ARRAY_LENGTH; ++i) {
		reset_sprite_ext(&g_sprite_table[i]);
	}
	for (u32 i = 0; i < IMAGE_ARRAY_LENGTH; ++i) {
		reset_image_ext(&g_image_table[i]);
	}
}

void update_ext_variables() {
	// Units
	for (u32 i = 0; i < UNIT_ARRAY_LENGTH; ++i) {
		UnitExt* unit_ext = &unit_ext_array[i];
		if (unit_ext->energy_regen_timer != UINT_MAX) {
			++unit_ext->energy_regen_timer;
		}
		if (unit_ext->shield_regen_timer != UINT_MAX) {
			++unit_ext->shield_regen_timer;
		}
		if (unit_ext->damage_taken_timer != UINT_MAX) {
			++unit_ext->damage_taken_timer;
		}
		if (unit_ext->damage_dealt_timer != UINT_MAX) {
			++unit_ext->damage_dealt_timer;
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
		reset_bullet_ext(&g_bullet_table[i]);
	}

	// Sprites
	for (u32 i = 0; i < SPRITE_ARRAY_LENGTH; ++i) {
		reset_sprite_ext(&g_sprite_table[i]);
	}

	// Images
	for (u32 i = 0; i < IMAGE_ARRAY_LENGTH; ++i) {
		reset_image_ext(&g_image_table[i]);
	}
}

UnitExt* get_unit_ext(Unit* unit) {
	u32 index = unit - g_unit_table;
	assert(unit && index >= 0 && index < UNIT_ARRAY_LENGTH);
	return &unit_ext_array[index];
}

void reset_unit_ext(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);
	*unit_ext = {};
	// every unit will starts being out of combat
	unit_ext->shield_regen_timer = UINT_MAX;
	unit_ext->damage_taken_timer = UINT_MAX;
	unit_ext->damage_dealt_timer = UINT_MAX;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		unit_ext->last_seen_timer[i] = UINT_MAX;
	}
}

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y) {
	return &tile_ext_array[tile_y][tile_x];
}

TileExt* get_tile_ext_at_coords(u32 x, u32 y) {
	return &tile_ext_array[y / 32][x / 32];
}

BulletExt* get_bullet_ext(Bullet* bullet) {
	u32 index = bullet - g_bullet_table;
	assert(unit && index >= 0 && index < BULLET_ARRAY_LENGTH);
	return &bullet_ext_array[index];
}

void reset_bullet_ext(Bullet* bullet) {
	BulletExt* bullet_ext = get_bullet_ext(bullet);
	*bullet_ext = {};
}

SpriteExt* get_sprite_ext(Sprite* sprite) {
	s32 index = sprite - g_sprite_table;
	assert(sprite && index >= 0 && index <= SPRITE_ARRAY_LENGTH);
	return &sprite_ext_array[index];
}

void reset_sprite_ext(Sprite* sprite) {
	SpriteExt* sprite_ext = get_sprite_ext(sprite);
	*sprite_ext = {};
}

ImageExt* get_image_ext(Image* image) {
	s32 index = image - g_image_table;
	assert(image && index >= 0 && index <= IMAGE_ARRAY_LENGTH);
	return &image_ext_array[index];
}

void reset_image_ext(Image* image) {
	ImageExt* image_ext = get_image_ext(image);
	*image_ext = {};
}

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player) {
	u32 allied_bits = 0;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		if (scbw::is_allied_to(i, player)) {
			allied_bits |= 1 << i;
		}
	}
	return !tile_ext->creep_owners || tile_ext->creep_owners & allied_bits;
}