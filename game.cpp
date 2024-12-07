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
    std::unordered_set<TileCoord> visited;
    std::unordered_set<u16> regions_to_remove;
    for (u32 i = 0; i < to_visit.size(); i++) {
        const auto tile = to_visit[i];
        const auto region_id = bw::get_region_id(tile);
        if (!visited.insert(tile).second) {
            continue;
        }
        regions_to_remove.insert(region_id);

        set_collision(tile);
        for (const auto [dx, dy] : std::initializer_list{std::pair{1, 0}, {0, 1}, {-1, 0}, {0, -1}}) {
            s32 x = static_cast<s32>(tile.x) + dx;
            s32 y = static_cast<s32>(tile.y) + dy;
            if (x < 0 || x >= g_map_tile_size->width || y < 0 || y >= g_map_tile_size->height) {
                continue;
            }
            TileCoord next_tile{x, y};
            if (region_id != bw::get_region_id(next_tile)) {
                continue;
            }
            to_visit.push_back(next_tile);
        }
    }
    for (const auto region_id : regions_to_remove) {
        const auto region = bw::get_region(region_id);
        for (u32 i = 0; i < region->neighbor_count; i++) {
            auto neighbor = bw::get_region(region->neighbors[i]);
            neighbor->remove_neighbour(region_id);
        }
        region->neighbor_count = 0;
        region->group_index = 0;
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