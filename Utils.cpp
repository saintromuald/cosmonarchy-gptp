#include "Utils.h"
#include "BW/api.h"

void Timer::Start(u32 time) {
    m_end_time = g_elapsed_time_frames + time + 1;
}

void Timer::Stop() {
    m_end_time = 0;
}

bool Timer::HasStarted() const {
    return m_end_time;
}

bool Timer::IsRunning() const {
    return HasStarted() && g_elapsed_time_frames < m_end_time;
}

bool Timer::HasElapsed() const {
    return HasStarted() && g_elapsed_time_frames >= m_end_time;
}

std::optional<Point16> pick_random_spawn_position(UnitId::Enum unit_id, u32 samples) {
    const auto buildings = g_unit_search.all().not_neutral().filter(0, FilterBuilding);
    return SampleBest(
        [unit_id]() {
            Point16 position{
                static_cast<u16>(bw::random_float() * g_map_width_pixels),
                static_cast<u16>(bw::random_float() * g_map_height_pixels),
            };

            if (units_dat::base_property[unit_id] & UnitProperty::Building) {
                position = bw::align_to_tile_grid(position);
                // TODO: check placeability
            }

            return position;
        },
        [unit_id, &buildings](const Point16& point) -> s32 {
            if (!(units_dat::base_property[unit_id] & UnitProperty::Flyer)) {
                if (!helper::can_unit_id_fit_at_xy(point.x, point.y, unit_id)) {
                    return InvalidSample;
                }
            }
            s32 score = 0;
            for (u32 player = 0; player < PlayerLimit; player++) {
                if (g_player_table[player].type != PlayerType::Human && g_player_table[player].type != PlayerType::Computer) {
                    continue;
                }

                s32 player_score = 0;
                const auto player_buildings = buildings.clone().owned_by(player);
                for (Unit* building : buildings) {
                    s32 distance = bw::distance_between(building->position, point);
                    if (distance <= 16 * 32) {
                        return InvalidSample;
                    }
                    player_score = std::min(player_score, distance);
                }
                score = std::min(score, player_score);
            }
            return score;
        },
        samples,
        100
    );
}

