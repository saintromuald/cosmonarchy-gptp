#pragma once
#include "types.h"

class Timer {
public:
    void Start(u32 time);
    void Stop();

    bool HasStarted() const;
    bool IsRunning() const;
    bool HasElapsed() const;

private:
    u32 m_end_time = 0;
};

#define InvalidSample -1

template <typename GenFunc, typename ScoreFunc>
auto SampleBest(GenFunc&& gen_func, ScoreFunc&& score_func, u32 max_samples, u32 max_allowed_fails) -> std::optional<decltype(gen_func())> {
    using Result = decltype(gen_func());
    using Score = decltype(score_func(std::declval<Result>()));
    std::optional<Result> result{};
    Score best_score = std::numeric_limits<Score>::min();

    while (max_samples > 0 && max_allowed_fails >= 0) {
        const Result current = gen_func();
        const Score score = score_func(current);
        if (score < 0) {
            max_allowed_fails--;
            continue;
        }
        if (score > best_score) {
            result = current;
            best_score = score;
        }
        max_samples--;
    }

    return result;
}

std::optional<Point16> PickRandomSpawnPosition(UnitId::Enum unit_id, u32 samples = 10);

