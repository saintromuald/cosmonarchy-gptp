#pragma once
#include "trigger_parser.h"
#include "trigger_system.h"
#include "trigger_spawn.h"
#include "trigger_vars.h"

#include <tuple>
#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>

namespace fn {
template <typename>
struct Traits;

template <typename R, typename... Args>
struct Traits<R(Args...)> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
    static constexpr size_t ArgCount = sizeof...(Args);
};

template <typename R, typename... Args>
struct Traits<R(*)(Args...)> : Traits<R(Args...)> {};

template <typename T, typename R, typename... Args>
struct Traits<R(T::*)(Args...)> : Traits<R(Args...)> {};

template <typename T, typename R, typename... Args>
struct Traits<R(T::*)(Args...) const> : Traits<R(Args...)> {};

template <typename Func>
struct Traits : Traits<decltype(&Func::operator())> {};
} // namespace fn

namespace trigger {

class Context;
using Condition = std::function<bool(Context&)>;

struct Action {
    bool is_local;
    std::function<bool(Context&)> func; // false means skip the rest of actions for the current tick (e.g. wait, exit, etc.)
};

template <typename T>
struct Parameter {
    static T get(Context& ctx) {
        return Parameter<std::optional<T>>::get(ctx).value_or({});
    }
};

template <typename Func>
auto ctx_get_params_tuple(Context& ctx) {
    return [&ctx]<typename... Args>(std::type_identity<std::tuple<Args...>>) {
        return std::tuple<Args...>{Parameter<std::remove_cvref_t<Args>>::get(ctx)...};
    }(std::type_identity<typename fn::Traits<Func>::ArgTypes>{});
}

template <typename Func>
Condition make_condition(Func&& condition);

template <typename Func>
Action make_action(Func&& action, bool is_local);

class Registry {
public:
    Registry() = default;

    template <typename Func>
    Registry& add_condition(const std::string& name, Func&& condition) {
        m_conditions.emplace(name, make_condition(std::forward<Func>(condition)));
        return *this;
    }

    template <typename Func>
    Registry& add_action(const std::string& name, Func&& action) {
        m_actions.emplace(name, make_action(std::forward<Func>(action), false));
        return *this;
    }

    template <typename Func>
    Registry& add_local_action(const std::string& name, Func&& action) {
        m_actions.emplace(name, make_action(std::forward<Func>(action), true));
        return *this;
    }

    std::optional<Condition> get_condition(const std::string& name) const {
        const auto it = m_conditions.find(name);
        if (it != m_conditions.end()) {
            return it->second;
        }
        return {};
    }

    std::optional<Action> get_action(const std::string& name) const {
        const auto it = m_actions.find(name);
        if (it != m_actions.end()) {
            return it->second;
        }
        return {};
    }

private:
    std::unordered_map<std::string, Condition> m_conditions;
    std::unordered_map<std::string, Action> m_actions;
};

class Subcommand {
public:
    template <typename Func>
    Subcommand(const char* name, Func&& action)
        : m_name{name}, m_action{make_action(std::forward<Func>(action), false)} {}

    const char* name() const { return m_name; }
    bool invoke(Context& ctx) const { return m_action.func(ctx); }

private:
    const char* m_name;
    Action m_action;
};

class Context {
public:
    Context(Registry& registry, TriggerAction* action, Trigger& trigger, u32 player)
        : m_registry{registry},
            m_parser{helper::get_chk_string(action->text_string)},
            m_trigger{trigger},
            m_trigger_ext{g_globals->trigger_exts[action]},
            m_action{action},
            m_player{u8(player)} {
        m_parser.load_command_pos(ongoing_wait().parser_position);
    }

    bool evaluate_condition() {
        const auto token = m_parser.get_word();
        if (!token) {
            error("Unknown condition");
            return false;
        }
        const auto condition = m_registry.get_condition(std::string{*token});
        if (condition) {
            return (*condition)(*this);
        }
        error("Unknown condition");
        return false;
    }

    bool evaluate_comment_conditions() {
        while (const auto command = m_parser.get_command()) {
            if (*command == "condition" && !evaluate_condition()) {
                return false;
            }
        }
        return true;
    }

    bool evaluate_comment_actions() {
        bool has_commands = false;
        while (const auto command = m_parser.get_command()) {
            if (*command == "condition" || command->starts_with("#")) {
                continue;
            }
            const auto action = m_registry.get_action(std::string{*command});
            if (!action) {
                // Note: We can't always error here, since not all comments are crigs
                if (has_commands) {
                    error("Unknown action");
                }
                continue;
            }
            has_commands = true;
            if (action->is_local && !is_local_player()) {
                continue; // Skip local actions for non-local players
            }
            if (!action->func(*this)) {
                return false;
            }
        }
        return true;
    }

    template <typename... Subcommands> requires (std::is_same_v<Subcommands, Subcommand> && ...)
    bool subcommands(Subcommands&&... subcommands) {
        const auto command = m_parser.get_string(false);
        if (!command) {
            error("Unknown subcommand");
            return false;
        }
        bool result = true;
        const auto found = ((sv_iequals(*command, subcommands.name()) ? (result = subcommands.invoke(*this), true) : false) || ...);
        if (!found) {
            error("Unknown subcommand");
        }
        return result;
    }

    template <typename... Args>
    void error(std::string_view title, Args&&... args) {
        m_had_errors = true;
        m_parser.error(title, std::forward<Args>(args)...);
    }

    Parser& parser() { return m_parser; }
    u32 command_start_pos() const { return m_parser.command_start_pos(); }
    Trigger& trigger() { return m_trigger; }
    TriggerExt& trigger_ext() { return m_trigger_ext; }
    TriggerAction* action() const { return m_action; }
    u8 player() const { return m_player; }
    bool is_local_player() const { return m_player == g_local_nation_id; }
    OngoingWait& ongoing_wait() { return m_trigger_ext.ongoing_waits[m_player]; }

    bool had_errors() const { return m_had_errors; }
    void clear_errors() { m_had_errors = false; }

private:
    Registry& m_registry;
    Parser m_parser;
    Trigger& m_trigger;
    TriggerExt& m_trigger_ext;
    TriggerAction* m_action;
    u8 m_player;
    bool m_had_errors = false;
};

template <typename Func>
Condition make_condition(Func&& condition) {
    return [condition = std::forward<Func>(condition)](Context& ctx) {
        ctx.clear_errors();
        auto params_tuple = ctx_get_params_tuple<Func>(ctx);
        if (ctx.had_errors()) {
            return false;
        }
        return std::apply(condition, params_tuple);
    };
}

template <typename Func>
Action make_action(Func&& action, bool is_local) {
    return Action{
        .is_local = is_local,
        .func = [action = std::forward<Func>(action)](Context& ctx) {
            ctx.clear_errors();
            auto params_tuple = ctx_get_params_tuple<Func>(ctx);
            if (ctx.had_errors()) {
                return true;
            }

            if constexpr (std::is_same_v<typename fn::Traits<Func>::ReturnType, void>) {
                std::apply(action, params_tuple);
                return true;
            } else {
                return std::apply(action, params_tuple);
            }
        },
    };
}

template <>
struct Parameter<Context> {
    static Context& get(Context& context) {
        return context;
    }
};

template <typename T>
struct Parameter<std::optional<Any<T>>> {
    static std::optional<Any<T>> get(Context& ctx) {
        auto& parser = ctx.parser();
        const auto marker = parser.marker();
        const auto token = parser.get_word();
        if (token == "*") {
            return Any<T>{.value = {}, .any = true};
        }
        parser.unwind(marker);
        return Any<T>{.value = Parameter<T>::get(ctx), .any = false};
    }
};


template <>
struct Parameter<std::optional<std::string_view>> {
    static std::optional<std::string_view> get(Context& ctx) {
        return ctx.parser().get_string();
    }
};

template <>
struct Parameter<std::optional<std::string>> {
    static std::optional<std::string> get(Context& ctx) {
        const auto value = ctx.parser().get_string();
        if (!value) {
            return {};
        }
        return std::string{*value};
    }
};

template <>
struct Parameter<std::optional<Identifier>> {
    static std::optional<Identifier> get(Context& ctx) {
        const auto value = ctx.parser().get_identifier();
        if (!value) {
            return {};
        }
        return Identifier{*value};
    }
};


template <std::integral T>
struct Parameter<std::optional<T>> {
    static std::optional<T> get(Context& ctx) {
        return ctx.parser().get_integer<T>();
    }
};

template <std::floating_point T>
struct Parameter<std::optional<T>> {
    static std::optional<T> get(Context& ctx) {
        return ctx.parser().get_float<T>();
    }
};

template <>
struct Parameter<std::optional<bool>> {
    static std::optional<bool> get(Context& ctx) {
        return ctx.parser().get_bool();
    }
};

template <>
struct Parameter<std::optional<Area>> {
    static std::optional<Area> get(Context& ctx) {
        return ctx.parser().get_area();
    }
};

template <>
struct Parameter<std::optional<v2u16>> {
    static std::optional<v2u16> get(Context& ctx) {
        return ctx.parser().get_position();
    }
};

template <typename T> requires std::is_enum_v<T>
struct Parameter<std::optional<T>> {
    static std::optional<T> get(Context& ctx) {
        return ctx.parser().get_magic_enum<T>();
    }
};

template <typename T> requires std::is_enum_v<T>
struct Parameter<T> {
    static T get(Context& ctx) {
        const auto value = Parameter<std::optional<T>>::get(ctx);
        if (!value) {
            ctx.error("Invalid enum");
        }
        return value.value_or(T{});
    }
};

template <>
struct Parameter<std::optional<PlayerBitset>> {
    static std::optional<PlayerBitset> get(Context& ctx) {
        return ctx.parser().get_player_bitset();
    }
};

template <>
struct Parameter<std::optional<UnitId::Enum>> {
    static std::optional<UnitId::Enum> get(Context& ctx) {
        return ctx.parser().get_unit_id();
    }
};

template <>
struct Parameter<std::optional<UnitIdBitset>> {
    static std::optional<UnitIdBitset> get(Context& ctx) {
        return ctx.parser().get_unit_id_bitset();
    }
};

template <>
struct Parameter<std::optional<WeaponId::Enum>> {
    static std::optional<WeaponId::Enum> get(Context& ctx) {
        return ctx.parser().get_weapon_id();
    }
};


template <>
struct Parameter<std::optional<WeaponIdBitset>> {
    static std::optional<WeaponIdBitset> get(Context& ctx) {
        return ctx.parser().get_weapon_id_bitset();
    }
};

template <>
struct Parameter<std::optional<flingy_id::Enum>> {
    static std::optional<flingy_id::Enum> get(Context& ctx) {
        return ctx.parser().get_flingy_id();
    }
};

template <>
struct Parameter<std::optional<FlingyIdBitset>> {
    static std::optional<FlingyIdBitset> get(Context& ctx) {
        return ctx.parser().get_flingy_id_bitset();
    }
};


template <>
struct Parameter<std::optional<StatOperator>> {
    static std::optional<StatOperator> get(Context& ctx) {
        return ctx.parser().get_stat_operator();
    }
};


template <>
struct Parameter<std::optional<Designator>> {
    static std::optional<Designator> get(Context& ctx) {
        return ctx.parser().get_designator();
    }
};

template <>
struct Parameter<std::optional<Quota>> {
    static std::optional<Quota> get(Context& ctx) {
        return ctx.parser().get_quota();
    }
};

template <>
struct Parameter<Quota> {
    static Quota get(Context& ctx) {
        return Parameter<std::optional<Quota>>::get(ctx).value_or({
            .type = QuotaType_GreaterOrEqual,
            .value = 0,
        });
    }
};

template <>
struct Parameter<EqualityQuota> {
    static EqualityQuota get(Context& ctx) {
        const auto quota = ctx.parser().get_quota(QuotaType_Equal).value_or(Quota{
            .type = QuotaType_Equal,
            .value = 0,
        });
        return EqualityQuota{quota};
    }
};

template <>
struct Parameter<std::optional<Recolor>> {
    static std::optional<Recolor> get(Context& ctx) {
        return ctx.parser().get_recolor();
    }
};

template <>
struct Parameter<std::optional<RecolorSet>> {
    static std::optional<RecolorSet> get(Context& ctx) {
        return ctx.parser().get_recolor_set();
    }
};

template <>
struct Parameter<Recolor> {
    static Recolor get(Context& ctx) {
        return Parameter<std::optional<Recolor>>::get(ctx).value_or(default_recolor());
    }
};

template <>
struct Parameter<RecolorSet> {
    static RecolorSet get(Context& ctx) {
        return Parameter<std::optional<RecolorSet>>::get(ctx).value_or(default_recolor_set());
    }
};

template <bool AllowNeutral>
struct Parameter<std::optional<PlayerId<AllowNeutral>>> {
    static std::optional<PlayerId<AllowNeutral>> get(Context& ctx) {
        return ctx.parser().get_player<AllowNeutral>();
    }
};

template <bool AllowNeutral>
struct Parameter<PlayerId<AllowNeutral>> {
    static PlayerId<AllowNeutral> get(Context& ctx) {
        auto player = ctx.parser().get_player<AllowNeutral>();
        if (!player) {
            ctx.error("Invalid player");
        }
        return player.value_or(PlayerId<AllowNeutral>{});
    }
};

template <>
struct Parameter<std::optional<Ticks>> {
    static std::optional<Ticks> get(Context& ctx) {
        return ctx.parser().get_time_ticks();
    }
};

template <>
struct Parameter<std::optional<Seconds>> {
    static std::optional<Seconds> get(Context& ctx) {
        return ctx.parser().get_time_seconds();
    }
};

template <>
struct Parameter<std::optional<TimeQuota>> {
    static std::optional<TimeQuota> get(Context& ctx) {
        return ctx.parser().get_time_quota();
    }
};

template <>
struct Parameter<TimeQuota> {
    static TimeQuota get(Context& ctx) {
        return Parameter<std::optional<TimeQuota>>::get(ctx).value_or({{
            .type = QuotaType_LessOrEqual,
            .value = 0,
        }});
    }
};

template <>
struct Parameter<std::optional<VarQuota>> {
    static std::optional<VarQuota> get(Context& ctx) {
        return ctx.parser().get_var_quota();
    }
};

template <>
struct Parameter<VarQuota> {
    static VarQuota get(Context& ctx) {
        return Parameter<std::optional<VarQuota>>::get(ctx).value_or({{
            .type = QuotaType_Equal,
            .value = 0,
        }, false});
    }
};

template <>
struct Parameter<std::optional<TimerQuota>> {
    static std::optional<TimerQuota> get(Context& ctx) {
        return ctx.parser().get_timer_quota();
    }
};

template <>
struct Parameter<TimerQuota> {
    static TimerQuota get(Context& ctx) {
        return Parameter<std::optional<TimerQuota>>::get(ctx).value_or({{
            .type = QuotaType_Equal,
            .value = 0,
        }, false});
    }
};

template <>
struct Parameter<std::optional<VarValue>> {
    static std::optional<VarValue> get(Context& ctx) {
        const auto value = ctx.parser().get_var_value(false);
        if (!value) {
            return {};
        }
        return VarValue{.value = *value};
    }
};

template <>
struct Parameter<std::optional<TimerValue>> {
    static std::optional<TimerValue> get(Context& ctx) {
        const auto value = ctx.parser().get_var_value(true);
        if (!value) {
            return {};
        }
        return TimerValue{.value = *value};
    }
};


template <>
struct Parameter<std::optional<IscriptAnimation::Enum>> {
    static std::optional<IscriptAnimation::Enum> get(Context& ctx) {
        return ctx.parser().get_magic_enum<IscriptAnimation::Enum>();
    }
};


template <>
struct Parameter<std::optional<SoundId::Enum>> {
    static std::optional<SoundId::Enum> get(Context& ctx) {
        const auto value = ctx.parser().get_integer();
        if (!value) {
            return {};
        }
        return SoundId::Enum(*value);
    }
};

template <>
struct Parameter<std::optional<SpriteSelector>> {
    static std::optional<SpriteSelector> get(Context& ctx) {
        return ctx.parser().get_sprite_selector();
    }
};

template <>
struct Parameter<std::optional<Location*>> {
    static std::optional<Location*> get(Context& ctx) {
        return ctx.parser().get_location();
    }
};

}; // namespace trigger
