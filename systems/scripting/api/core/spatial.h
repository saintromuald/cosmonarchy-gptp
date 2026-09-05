#pragma once
#include "../script_common.h"

namespace lua::api::core {

struct Point {
    s32 x;
    s32 y;

    Point() : x{0}, y{0} {}
    Point(s32 x, s32 y) : x{x}, y{y} {}
    Point(v2u16 v) : x{v.x}, y{v.y} {}
    Point(v2s32 v) : x{v.x}, y{v.y} {}
    Point(v2 v) : x{s32(v.x)}, y{s32(v.y)} {}
    Point(v2fp64 v) : x{s32(v.x)}, y{s32(v.y)} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<Point>("Point",
            sol::call_constructor, sol::constructors<
                Point(),
                Point(s32, s32)
            >(),

            "x", sol::property([](Point v) { return v.x; }, [](Point& v, s32 x) { v.x = x; }),
            "y", sol::property([](Point v) { return v.y; }, [](Point& v, s32 y) { v.y = y; }),

            sol::meta_function::addition, [](const Point& a, const Point& b) { return Point(a.x + b.x, a.y + b.y); },
            sol::meta_function::subtraction, [](const Point& a, const Point& b) { return Point(a.x - b.x, a.y - b.y); },
            sol::meta_function::multiplication, [](const Point& a, f64 b) { return Point(a.x * b, a.y * b); },
            sol::meta_function::division, [](const Point& a, f64 b) { return Point(a.x / b, a.y / b); }
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---A two-dimensional point.
---@class Point
---@overload fun(): Point
---@overload fun(x: integer, y: integer): Point
---@field x integer Horizontal coordinate.
---@field y integer Vertical coordinate.
---@operator add(Point): Point
---@operator sub(Point): Point
---@operator mul(number): Point
---@operator div(number): Point
Point = {}
)lua";
    }

    std::string to_string() const {
        return std::format("Point(x={}, y={})", x, y);
    }

    operator v2u16() const { return v2u16{u16(x), u16(y)}; }
    operator v2s32() const { return v2s32{x, y}; }
    operator v2() const { return v2{fp32(x), fp32(y)}; }
    operator v2fp64() const { return v2fp64{fp64(x), fp64(y)}; }
};

struct Area {
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    Area() : x{0}, y{0}, w{0}, h{0} {}
    Area(s32 x, s32 y, s32 w, s32 h) : x{x}, y{y}, w{w}, h{h} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<Area>("Area",
            sol::call_constructor, sol::constructors<Area()>(),
            "location", sol::factories(
                sol::resolve<Area(u32)>(&Area::from_location),
                sol::resolve<Area(std::string_view)>(&Area::from_location)
            ),
            "xywh", sol::factories(
                sol::resolve<Area(s32, s32, s32, s32)>(&Area::xywh)
            ),
            "ltrb", sol::factories(
                sol::resolve<Area(s32, s32, s32, s32)>(&Area::ltrb)
            ),

            "x", sol::property([](Area v) { return v.x; }, [](Area& v, s32 x) { v.x = x; }),
            "y", sol::property([](Area v) { return v.y; }, [](Area& v, s32 y) { v.y = y; }),
            "w", sol::property([](Area v) { return v.w; }, [](Area& v, s32 w) { v.w = w; }),
            "h", sol::property([](Area v) { return v.h; }, [](Area& v, s32 h) { v.h = h; }),

            "left", sol::property([](Area v) { return v.x; }, [](Area& v, s32 left) { v.w += v.x - left; v.x = left; }),
            "top", sol::property([](Area v) { return v.y; }, [](Area& v, s32 top) { v.h += v.y - top; v.y = top; }),
            "right", sol::property([](Area v) { return v.x + v.w; }, [](Area& v, s32 right) { v.w = right - v.x; }),
            "bottom", sol::property([](Area v) { return v.y + v.h; }, [](Area& v, s32 bottom) { v.h = bottom - v.y; })
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---A rectangular map area.
---@class Area
---@overload fun(): Area
---@field x integer Left coordinate.
---@field y integer Top coordinate.
---@field w integer Width.
---@field h integer Height.
---@field left integer Left edge.
---@field top integer Top edge.
---@field right integer Right edge.
---@field bottom integer Bottom edge.
Area = {}

---Creates an area from a map location.
---@overload fun(location_name: string): Area
---@param location integer Location ID from 0 to 255.
---@return Area area The location area.
---@nodiscard
function Area.location(location) end

---Creates an area from its top-left coordinate, width, and height.
---@param x integer Left coordinate.
---@param y integer Top coordinate.
---@param w integer Width.
---@param h integer Height.
---@return Area area The new area.
---@nodiscard
function Area.xywh(x, y, w, h) end

---Creates an area from its edge coordinates.
---@param left integer Left edge.
---@param top integer Top edge.
---@param right integer Right edge.
---@param bottom integer Bottom edge.
---@return Area area The new area.
---@nodiscard
function Area.ltrb(left, top, right, bottom) end
)lua";
    }

    static Area xywh(s32 x, s32 y, s32 w, s32 h) {
        return Area{x, y, w, h};
    }

    static Area xywh(Box32 box) {
        return xywh(box.x, box.y, box.w, box.h);
    }

    static Area ltrb(s32 left, s32 top, s32 right, s32 bottom) {
        return Area{left, top, right - left, bottom - top};
    }

    static Area ltrb(Box32 box) {
        return ltrb(box.left, box.top, box.right, box.bottom);
    }

    static Area from_location(u32 location_id) {
        if (location_id >= 256) {
            throw sol::error(std::format("Location ID {} is out of range [0, 255]", location_id));
        }
        return ltrb(g_location_table[location_id].dimensions);
    }

    static Area from_location(std::string_view location_name) {
        for (u32 i = 0; i < 256; i++) {
            const auto name = helper::get_chk_string(g_location_table[i].string_id);
            if (location_name == name) {
                return ltrb(g_location_table[i].dimensions);
            }
        }
        throw sol::error(std::format("Location '{}' not found", location_name));
    }

    std::string to_string() const {
        return std::format("Area(x={}, y={}, w={}, h={})", x, y, w, h);
    }

    Box32 box32_xywh() const {
        return Box32{x, y, w, h};
    }

    Box32 box32_ltrb() const {
        return Box32{.left = x, .top = y, .right = x + w, .bottom = y + h};
    }
};

} // namespace lua::api::core