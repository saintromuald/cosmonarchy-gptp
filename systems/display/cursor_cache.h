#pragma once

#include "../../types.h"
#include "../../BW/structures.h"
#include "../../systems/graphics/grp.h"

#include <unordered_map>

#define NOMINMAX
#include <windows.h>

// Cursor center location in grp to offset the sprite draw
constexpr s32 CursorHotspotBase = 126;

struct CursorKey {
    const graphics::GrpFile* grp = nullptr;
    u16 frame = 0;
    u32 scale_fixed = 0;

    bool operator==(const CursorKey&) const = default;
};

struct CursorKeyHash {
    size_t operator()(const CursorKey& key) const noexcept {
        size_t seed = std::hash<const graphics::GrpFile*>{}(key.grp);
        seed ^= std::hash<u16>{}(key.frame) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<u32>{}(key.scale_fixed) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class CursorCache {
public:
    static CursorCache& instance();

    ~CursorCache();
    CursorCache(const CursorCache&) = delete;
    CursorCache& operator=(const CursorCache&) = delete;

    HCURSOR get_cursor(const graphics::GrpFile* grp, u16 frame_index);
    void clear();

private:
    CursorCache() = default;

    HCURSOR create_cursor(const graphics::GrpFile* grp, u16 frame_index, f32 scale);

    std::unordered_map<CursorKey, HCURSOR, CursorKeyHash> m_cursors;
    Palette m_palette{};
    u32 m_scale_fixed = 10000;
};
