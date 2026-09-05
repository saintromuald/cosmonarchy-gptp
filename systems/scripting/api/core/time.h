#pragma once
#include "../script_common.h"

namespace lua::api::core {

class Time {
public:
    static void install(ScriptManager& manager, sol::state& lua) {
        // TODO: support fixed point instead of integer -Veeq7
        lua.set_function("secs", [](u32 seconds) {
            return seconds * 24;
        });
        lua.set_function("mins", [](u32 minutes) {
            return minutes * 60 * 24;
        });
        lua.set_function("hours", [](u32 hours) {
            return hours * 60 * 60 * 24;
        });

        lua.new_usertype<Time>("Time", sol::no_constructor,
            "elapsed", sol::var(std::cref(g_elapsed_time_frames))
        );
        lua["Time"] = Time{};
    }

    static const char* lsp_annotations() {
        return R"lua(
---Converts seconds to game ticks.
---@param seconds integer Duration in seconds.
---@return integer ticks Duration in game ticks.
---@nodiscard
function secs(seconds) end

---Converts minutes to game ticks.
---@param minutes integer Duration in minutes.
---@return integer ticks Duration in game ticks.
---@nodiscard
function mins(minutes) end

---Converts hours to game ticks.
---@param hours integer Duration in hours.
---@return integer ticks Duration in game ticks.
---@nodiscard
function hours(hours) end

---Current game time.
---@class Time
---@field elapsed integer Elapsed game time in ticks. Read-only.
Time = {}
)lua";
    }

    std::string to_string() const {
        return std::format("Time(elapsed={})", g_elapsed_time_frames);
    }
};

} // namespace lua::api::core
