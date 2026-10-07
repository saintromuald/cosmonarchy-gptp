#include "systems/graphics/translucency.h"

#include "BW/bwdata.h"

#include <array>

namespace graphics {
namespace {

// True for palette entries inside the current tileset's color-cycling ranges; those
// animate in the live palette, so a blend output matched onto them fades in and out.
bool cycling_range_contains(u8 index) {
    const u8* spec = g_tileset_cycle_specs[g_tileset_id];
    if (!spec[8]) {
        return false;
    }
    const u8* entry = spec + 10;
    while (true) {
        if (entry[-1] <= index && index <= entry[0]) {
            return true;
        }
        if (entry[6] == 0) {
            return false;
        }
        entry += 8;
    }
}

u32 palette_rgb_hash(const Palette& palette) {
    u32 hash = 2166136261u; // FNV-1a
    for (const RGBA& color : palette.colors) {
        hash = (hash ^ color.r) * 16777619u;
        hash = (hash ^ color.g) * 16777619u;
        hash = (hash ^ color.b) * 16777619u;
    }
    return hash;
}

std::array<u8, 256> g_row_slots[2]{};

struct RowCache {
    u32 palette_hash = 0;
    u8 tint = 0;
    ui::PalettePreserve mask = ui::PalettePreserve::None;
    bool built = false;
};
RowCache g_row_cache[2];

} // namespace

u8 translucent_index() {
    return g_translucent_index ? g_translucent_index : TranslucentIndex;
}

void build_translucency_row(u8* row, const Palette& palette, u32 weight,
    u8 translucent_index, ui::PalettePreserve preserve, bool exclude_cycling) {
    const u32 background_weight = weight;     // dest share out of 8
    const u32 image_weight = 8 - weight;      // src share out of 8
    const RGBA ic = palette.colors[translucent_index];

    for (u32 dest = 0; dest < 256; ++dest) {
        const RGBA dc = palette.colors[dest];
        const int blend_r = static_cast<int>((ic.r * image_weight + dc.r * background_weight) >> 3);
        const int blend_g = static_cast<int>((ic.g * image_weight + dc.g * background_weight) >> 3);
        const int blend_b = static_cast<int>((ic.b * image_weight + dc.b * background_weight) >> 3);

        u32 best_distance = 0xffffffffu;
        u8 best_index = translucent_index;
        for (u32 candidate = 0; candidate < 256; ++candidate) {
            if (candidate == 0) continue; // colorkey
            if (exclude_cycling && cycling_range_contains(static_cast<u8>(candidate))) continue;
            if (ui::preserves_index(preserve, candidate)) continue;
            const RGBA cc = palette.colors[candidate];
            const int dr = cc.r - blend_r;
            const int dg = cc.g - blend_g;
            const int db = cc.b - blend_b;
            const u32 distance = static_cast<u32>(dr * dr + dg * dg + db * db);
            if (distance < best_distance) {
                best_distance = distance;
                best_index = static_cast<u8>(candidate);
            }
        }
        row[dest] = best_index;
    }
}

namespace {

const u8* cached_translucency_row(int slot, u8 tint) {
    RowCache& cache = g_row_cache[slot];
    const u32 hash = palette_rgb_hash(g_init_palette);
    if (!cache.built || hash != cache.palette_hash || tint != cache.tint
        || g_translucency_preserve != cache.mask) {
        build_translucency_row(g_row_slots[slot].data(), g_init_palette, TranslucencyWeight, tint,
            g_translucency_preserve, slot != 0);
        cache.palette_hash = hash;
        cache.tint = tint;
        cache.mask = g_translucency_preserve;
        cache.built = true;
    }
    return g_row_slots[slot].data();
}

} // namespace

const u8* active_translucency_row() {
    return cached_translucency_row(0, translucent_index());
}

const u8* active_alt_translucency_row() {
    return cached_translucency_row(1, g_translucent_index_alt);
}

} // namespace graphics
