#pragma once
#include "types.h"

class Timer {
public:
    void Start(u32 time);
    void Stop();

    bool HasStarted();
    bool IsRunning();
    bool HasElapsed();

private:
    u32 m_end_time = 0;
};

template <typename GenFunc, typename ScoreFunc>
auto SampleBest(const GenFunc& gen_func, const ScoreFunc& score_func, u32 max_samples, u32 max_allowed_fails) -> decltype(gen_func()) {
    using Result = decltype(gen_func());
    using Score = decltype(score_func(*std::declval<Result>()));
    Result result{};
    Score best_score = std::numeric_limits<Score>::min();

    while (max_samples > 0 && max_allowed_fails >= 0) {
        Result current = gen_func();
        if (!current) {
            max_allowed_fails--;
            continue;
        }

        Score score = score_func(*current);
        if (score > best_score) {
            result = current;
            best_score = score;
        }
        max_samples--;
    }

    return result;
}
