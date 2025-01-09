#include "Utils.h"
#include "BW/api.h"

void Timer::start(u32 time) {
    m_end_time = g_elapsed_time_frames + time + 1;
}

void Timer::stop() {
    m_end_time = 0;
}

bool Timer::started() const {
    return m_end_time;
}

bool Timer::running() const {
    return started() && g_elapsed_time_frames < m_end_time;
}

bool Timer::elapsed() const {
    return started() && g_elapsed_time_frames >= m_end_time;
}

namespace utils {

std::optional<Point16> pick_random_spawn_point(UnitId::Enum unit_id, u32 samples) {
    const auto buildings = g_unit_search.all().not_neutral().filter(0, FilterBuilding);
    return sample_best(
        [unit_id]() {
            Point16 position{
                static_cast<u16>(bw::random_float() * g_map_width_pixels),
                static_cast<u16>(bw::random_float() * g_map_height_pixels),
            };

            if (units_dat::base_property[unit_id] & UnitProperty::Building) {
                position = bw::align_to_tile_grid(position, unit_id);
            }

            return position;
        },
        [unit_id, &buildings](const Point16& position) -> s32 {
            if (units_dat::base_property[unit_id] & UnitProperty::Building) {
                if (!bw::check_building_collision(unit_id, NeutralHostilePlayer, position)) {
                    return InvalidSample;
                }
            }
            if (!(units_dat::base_property[unit_id] & UnitProperty::Flyer)) {
                if (!helper::can_unit_id_fit_at_xy(position.x, position.y, unit_id)) {
                    return InvalidSample;
                }
            }
            const auto group = bw::get_region(position)->group_index;
            bool valid = unit_id_is_air_spawner(unit_id);
            s32 score = 0;
            for (Unit* building : buildings) {
                s32 distance = bw::distance_between(building->position, position);
                if (distance <= 16 * 32) {
                    return InvalidSample;
                }
                if (!unit_id_is_air_spawner(unit_id) && bw::get_region(building->position)->group_index == group) {
                    valid = true;
                }
                score = std::min(score, distance);
            }
            return valid ? score : InvalidSample;
        },
        samples,
        100
    );
}

} // utils
