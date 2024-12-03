#include "Game.h"

namespace {
void set_collision(TileCoord tile) {
    auto& active_tile = bw::get_tile_at_tile(tile);
    active_tile.is_walkable = false;
    active_tile.is_unwalkable = true;
    active_tile.always_unbuildable = true;
    active_tile.cliff_edge = false;
}

void fill_collision(std::vector<TileCoord> to_visit) {
    for (u32 i = 0; i < to_visit.size(); i++) {
        const auto tile = to_visit[i];
        const auto region = bw::get_region(tile);
        const auto was_cliff_edge = bw::get_tile_at_tile(tile).cliff_edge;

        set_collision(tile);
        for (const auto [dx, dy] : std::initializer_list{std::pair{1, 0}, {0, 1}, {-1, 0}, {0, -1}}) {
            s32 x = static_cast<s32>(tile.x) + dx;
            s32 y = static_cast<s32>(tile.y) + dy;
            if (x < 0 || x >= g_map_tile_size->width) {
                continue;
            }
            if (y < 0 || y >= g_map_tile_size->height) {
                continue;
            }

            TileCoord next_tile{x, y};
            const auto next_region = bw::get_region(next_tile);
            if (next_region->group_index != region->group_index) {
                continue;
            }

            const auto& next_active_tile = bw::get_tile_at_tile(next_tile);
            if (next_active_tile.cliff_edge || (!was_cliff_edge && next_active_tile.is_walkable)) {
                to_visit.push_back(next_tile);
            }
        }
    }
}
} // namespace

void Game::first_frame() {
    // Collision Brushes
    for (const auto& tile : m_collision_set_tiles) {
        set_collision(tile);
    }
    fill_collision(m_collision_fill_tiles);
}

void Game::tick() {
    // Quick Casting
    if (m_quick_casting.timeout && !g_is_targeting) {
        m_quick_casting.timeout--;
    }
    if (m_quick_casting.building_timeout && !g_is_placing_building) {
        m_quick_casting.building_timeout--;
    }
}