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
