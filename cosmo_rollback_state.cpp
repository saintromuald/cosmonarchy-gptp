// Cosmonarchy rollback: gptp state serialization (ADR-0005, Layer C).
//
// Hand-written, versioned binary serializer for gptp's per-frame simulation state.
// Exposes four C exports the cosmo_rollback.dll harness calls at the frame boundary
// (0x004D9670) to snapshot/restore plugin state so re-simulation is exact.
//
// First cut: *g_iquare_globals (effects / multihit bullets / line-attack / spine hits /
// bounce bullet). Raw Unit*/Bullet* are stored as stable, validatable identities
// (UnitIndex / bullet pool index) and rebuilt on load, dropping references that do not
// resolve. The *g_globals trigger-state slice and g_ext are Phase 2.
//
// Inert unless cosmo_gptp_rollback_enable(1) is called: normal play never touches it.

#include "globals.h"
#include "effect_field.h"
#include "BW/structures.h"
#include "bullet_extender.h"
#include "data_ext.h"
#include "BW/bwdata.h"

#include <cstring>
#include <cstddef>
#include <variant>
#include <vector>
#include <algorithm>

namespace {

constexpr u32 GPTS_MAGIC = 0x53545047; // 'GPTS'
constexpr u32 GPTS_VERSION = 2;

bool g_rollback_enabled = false;

// ---------------------------------------------------------------------------
// Little-endian writer. `out == nullptr` means "measure only".
// ---------------------------------------------------------------------------
struct Writer {
    u8* out;
    u32 cap;
    u32 len = 0;
    bool overflow = false;

    void raw(const void* p, u32 n) {
        if (out) {
            if (len + n > cap) { overflow = true; return; }
            std::memcpy(out + len, p, n);
        }
        len += n;
    }
    void u8v(u8 v) { raw(&v, 1); }
    void u16v(u16 v) { raw(&v, 2); }
    void u32v(u32 v) { raw(&v, 4); }
    void u64v(u64 v) { raw(&v, 8); }
    void s32v(s32 v) { raw(&v, 4); }
    void v2(v2u16 p) { u16v(p.x); u16v(p.y); }
};

struct Reader {
    const u8* in;
    u32 len;
    u32 pos = 0;
    bool bad = false;

    bool need(u32 n) { if (pos + n > len) { bad = true; return false; } return true; }
    u8 u8v() { if (!need(1)) return 0; u8 v; std::memcpy(&v, in + pos, 1); pos += 1; return v; }
    u16 u16v() { if (!need(2)) return 0; u16 v; std::memcpy(&v, in + pos, 2); pos += 2; return v; }
    u32 u32v() { if (!need(4)) return 0; u32 v; std::memcpy(&v, in + pos, 4); pos += 4; return v; }
    u64 u64v() { if (!need(8)) return 0; u64 v; std::memcpy(&v, in + pos, 8); pos += 8; return v; }
    s32 s32v() { if (!need(4)) return 0; s32 v; std::memcpy(&v, in + pos, 4); pos += 4; return v; }
    v2u16 v2() { v2u16 p; p.x = u16v(); p.y = u16v(); return p; }
};

// ---------------------------------------------------------------------------
// Pointer identities.
// ---------------------------------------------------------------------------
u32 enc_unit(Unit* u) {
    if (!u) return 0;
    UnitIndex i(u);
    return i.value;
}
Unit* dec_unit(u32 v) {
    if (!v) return nullptr;
    UnitIndex i;
    i.value = v;
    return i.get(true); // validates index + unique_id generation
}

u16 enc_bullet(Bullet* b) {
    if (!b) return 0;
    ptrdiff_t idx = b - new_bullet_table;
    if (idx < 0 || idx >= BulletLimit) return 0;
    return (u16)(idx + 1);
}
Bullet* dec_bullet(u16 idx) {
    if (!idx || idx > BulletLimit) return nullptr;
    Bullet* b = &new_bullet_table[idx - 1];
    return b->sprite ? b : nullptr;
}

void write_effect(Writer& w, const EffectField& f) {
    w.u64v(f.ref);
    w.u32v((u32)f.id);
    w.v2(f.position);
    if (std::holds_alternative<u32>(f.radius)) {
        w.u8v(0);
        w.u32v(std::get<u32>(f.radius));
    } else {
        w.u8v(1);
        w.v2(std::get<v2u16>(f.radius));
    }
    w.u32v(f.current_time);
    w.u32v(f.max_time);
    w.u32v((u32)f.sprite_id);
    w.u32v(f.elevation);
    w.u32v(f.player);
    w.u32v(enc_unit(f.source));
    w.u32v(enc_unit(f.origin));
    w.u32v(f.intervals);
    w.v2(f.move_target);
    w.u32v(f.move_speed);
    w.s32v(f.tick);
}

void read_effect(Reader& r, EffectField& f) {
    f.ref = r.u64v();
    f.id = (EffectId)r.u32v();
    f.position = r.v2();
    u8 tag = r.u8v();
    if (tag == 0) {
        f.radius = r.u32v();
    } else {
        f.radius = r.v2();
    }
    f.current_time = r.u32v();
    f.max_time = r.u32v();
    f.sprite_id = (SpriteId::Enum)r.u32v();
    f.elevation = r.u32v();
    f.player = r.u32v();
    f.source = dec_unit(r.u32v());
    f.origin = dec_unit(r.u32v());
    f.intervals = r.u32v();
    f.move_target = r.v2();
    f.move_speed = r.u32v();
    f.tick = r.s32v();
    // Viewer/audio-only fields are not serialized: re-created lazily by tick_effect.
    f.thingy = nullptr;
    f.playing_sound = PlayingSound{};
}

void write_iquare(Writer& w) {
    IquareGlobals* g = g_iquare_globals.get();
    if (!g) {
        w.u8v(0); // null sentinel
        return;
    }
    w.u8v(1);
    w.u8v(g->unitid_extender ? 1 : 0);
    w.u64v(g->m_next_effect_field_ref);

    w.u32v((u32)g->m_effects.size());
    for (const auto& f : g->m_effects) write_effect(w, f);

    w.u32v((u32)g->m_effects_to_add.size());
    for (const auto& f : g->m_effects_to_add) write_effect(w, f);

    w.u32v((u32)g->m_line_attack_hits.size());
    for (const auto& h : g->m_line_attack_hits) {
        w.u32v(enc_unit(h.source));
        w.u32v(enc_unit(h.target));
    }

    w.u32v((u32)g->m_multihit_bullets.size());
    for (const auto& m : g->m_multihit_bullets) {
        w.u16v(enc_bullet(m.bullet));
        w.s32v(m.divider);
        w.u32v((u32)m.already_hit.size());
        for (Unit* u : m.already_hit) w.u32v(enc_unit(u));
    }

    // Deterministic order for the spine-hit map (key-sorted).
    std::vector<std::pair<u32, const std::vector<Unit*>*>> spine;
    spine.reserve(g->m_lakizilisk_spine_hits.size());
    for (const auto& kv : g->m_lakizilisk_spine_hits) {
        spine.push_back({ enc_unit(kv.first), &kv.second });
    }
    std::sort(spine.begin(), spine.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    w.u32v((u32)spine.size());
    for (const auto& kv : spine) {
        w.u32v(kv.first);
        w.u32v((u32)kv.second->size());
        for (Unit* u : *kv.second) w.u32v(enc_unit(u));
    }

    w.u16v(enc_bullet(g->bounce_bullet));
}

void read_iquare(Reader& r) {
    if (r.u8v() == 0) return; // was null
    IquareGlobals* g = g_iquare_globals.get();
    if (!g) return;
    g->unitid_extender = r.u8v() != 0;
    g->m_next_effect_field_ref = r.u64v();

    u32 n = r.u32v();
    g->m_effects.clear();
    g->m_effects.resize(n);
    for (u32 i = 0; i < n && !r.bad; i++) read_effect(r, g->m_effects[i]);

    n = r.u32v();
    g->m_effects_to_add.clear();
    g->m_effects_to_add.resize(n);
    for (u32 i = 0; i < n && !r.bad; i++) read_effect(r, g->m_effects_to_add[i]);

    n = r.u32v();
    g->m_line_attack_hits.clear();
    for (u32 i = 0; i < n && !r.bad; i++) {
        LineAttackHit h;
        h.source = dec_unit(r.u32v());
        h.target = dec_unit(r.u32v());
        if (h.source && h.target) g->m_line_attack_hits.push_back(h);
    }

    n = r.u32v();
    g->m_multihit_bullets.clear();
    for (u32 i = 0; i < n && !r.bad; i++) {
        MultihitBullet m;
        m.bullet = dec_bullet(r.u16v());
        m.divider = r.s32v();
        u32 c = r.u32v();
        for (u32 j = 0; j < c && !r.bad; j++) {
            Unit* u = dec_unit(r.u32v());
            if (u) m.already_hit.push_back(u);
        }
        if (m.bullet) g->m_multihit_bullets.push_back(m);
    }

    n = r.u32v();
    g->m_lakizilisk_spine_hits.clear();
    for (u32 i = 0; i < n && !r.bad; i++) {
        Unit* key = dec_unit(r.u32v());
        u32 c = r.u32v();
        std::vector<Unit*> vals;
        for (u32 j = 0; j < c && !r.bad; j++) {
            Unit* u = dec_unit(r.u32v());
            if (u) vals.push_back(u);
        }
        if (key && !vals.empty()) g->m_lakizilisk_spine_hits[key] = std::move(vals);
    }

    g->bounce_bullet = dec_bullet(r.u16v());
}

// --- Phase 2: g_ext (per-unit/per-bullet sidecar) ---
// UnitExt has std::vector members (value_vectors/unit_vectors/timer_vectors) but they are
// the LAST members, so we memcpy the POD prefix (scalars, fixed arrays, optional/Link,
// pool pointers — all consistent once the engine snapshot has restored the pools) and
// serialize the vectors' contents separately. BulletExt is POD -> raw memcpy.

constexpr u32 UNIT_EXT_PREFIX = (u32)offsetof(UnitExt, value_vectors);

void write_ext(Writer& w) {
    for (u32 i = 0; i < (u32)UNIT_ARRAY_LENGTH; i++) {
        UnitExt* e = get_unit_ext(&g_unit_table[i]);
        w.raw(e, UNIT_EXT_PREFIX);
        for (int v = 0; v < (int)ValueVectorCount; v++) {
            const auto& vec = e->value_vectors[v];
            w.u32v((u32)vec.size());
            for (u32 x : vec) w.u32v(x);
        }
        for (int v = 0; v < (int)UnitVectorCount; v++) {
            const auto& vec = e->unit_vectors[v];
            w.u32v((u32)vec.size());
            for (Unit* u : vec) w.u32v(enc_unit(u));
        }
        for (int v = 0; v < (int)TimerVectorCount; v++) {
            const auto& vec = e->timer_vectors[v];
            w.u32v((u32)vec.size());
            for (u32 x : vec) w.u32v(x);
        }
    }
    for (u32 i = 0; i < (u32)BulletLimit; i++) {
        BulletExt* e = get_bullet_ext(&new_bullet_table[i]);
        w.raw(e, sizeof(BulletExt));
    }
}

void read_ext(Reader& r) {
    for (u32 i = 0; i < (u32)UNIT_ARRAY_LENGTH && !r.bad; i++) {
        UnitExt* e = get_unit_ext(&g_unit_table[i]);
        if (!r.need(UNIT_EXT_PREFIX)) return;
        std::memcpy(e, r.in + r.pos, UNIT_EXT_PREFIX);
        r.pos += UNIT_EXT_PREFIX;
        for (int v = 0; v < (int)ValueVectorCount; v++) {
            u32 n = r.u32v();
            e->value_vectors[v].resize(n);
            for (u32 j = 0; j < n && !r.bad; j++) e->value_vectors[v][j] = r.u32v();
        }
        for (int v = 0; v < (int)UnitVectorCount; v++) {
            u32 n = r.u32v();
            e->unit_vectors[v].clear();
            for (u32 j = 0; j < n && !r.bad; j++) {
                Unit* u = dec_unit(r.u32v());
                if (u) e->unit_vectors[v].push_back(u);
            }
        }
        for (int v = 0; v < (int)TimerVectorCount; v++) {
            u32 n = r.u32v();
            e->timer_vectors[v].resize(n);
            for (u32 j = 0; j < n && !r.bad; j++) e->timer_vectors[v][j] = r.u32v();
        }
    }
    for (u32 i = 0; i < (u32)BulletLimit && !r.bad; i++) {
        BulletExt* e = get_bullet_ext(&new_bullet_table[i]);
        if (!r.need((u32)sizeof(BulletExt))) return;
        std::memcpy(e, r.in + r.pos, sizeof(BulletExt));
        r.pos += sizeof(BulletExt);
    }
}

u32 serialize(u8* dst, u32 cap) {
    Writer w{ dst, cap, 0, false };
    w.u32v(GPTS_MAGIC);
    w.u32v(GPTS_VERSION);
    u32 len_pos = w.len;
    w.u32v(0); // total_len placeholder
    u32 start = w.len;
    write_iquare(w);
    write_ext(w);
    if (w.overflow) return 0;
    u32 body = w.len - start;
    if (dst) {
        std::memcpy(dst + len_pos, &body, 4);
    }
    return w.len;
}

} // namespace

extern "C" __declspec(dllexport) void cosmo_gptp_rollback_enable(int on) {
    g_rollback_enabled = on != 0;
}

extern "C" __declspec(dllexport) u32 cosmo_gptp_state_size() {
    if (!g_rollback_enabled) return 0;
    return serialize(nullptr, 0);
}

extern "C" __declspec(dllexport) u32 cosmo_gptp_save(u8* dst, u32 cap) {
    if (!g_rollback_enabled || !dst) return 0;
    return serialize(dst, cap);
}

extern "C" __declspec(dllexport) u32 cosmo_gptp_load(const u8* src, u32 len) {
    if (!g_rollback_enabled || !src || len < 12) return 0;
    Reader r{ src, len, 0, false };
    if (r.u32v() != GPTS_MAGIC) return 0;
    if (r.u32v() != GPTS_VERSION) return 0;
    (void)r.u32v(); // total_len
    read_iquare(r);
    read_ext(r);
    return r.bad ? 0 : 1;
}
