#pragma once
#include <BW\bwdata.h>
#include <globals.h>
#include <functional>
#include <unordered_set>

enum SearchFilter {
	FilterInvincible			= 1 <<  0,
	FilterNoAir					= 1 <<  1,
	FilterNoGround				= 1 <<  2,
	FilterNoMechanical			= 1 <<  3,
	FilterNoOrganic				= 1 <<  4,
	FilterNoBuilding			= 1 <<  5,
	FilterOnlyOrganic			= 1 <<  6,
	FilterOnlyMechanic			= 1 <<  7,
	FilterEnemy					= 1 <<  8,
	FilterAlly					= 1 <<  9,
	FilterIncludingSelf			= 1 << 10,
	FilterFrontalArc			= 1 << 11,
	FilterSpellcaster			= 1 << 12,
	FilterHasEnergy				= 1 << 13,
	FilterNotSelf				= 1 << 14,
	FilterShielded				= 1 << 15,
	FilterEnabled				= 1 << 16,
	FilterPowered				= 1 << 17,
	FilterCompleted				= 1 << 18,
	FilterNoTrap				= 1 << 19,
	FilterNoHallucination		= 1 << 20,
	FilterCanAttackOrCast		= 1 << 21,
	FilterOwn					= 1 << 22,
	FilterNotWorker				= 1 << 23,
	FilterNotSubunit			= 1 << 24,
	FilterBuilding				= 1 << 25,
	FilterNoEgg					= 1 << 26,
	FilterNoLarva				= 1 << 27,
	FilterNoHangar				= 1 << 28,
	FilterVisible				= 1 << 29,
};

enum SearchFilterCorpse {
	CorpseFilterNoAir			= 1 <<  1,
	CorpseFilterNoGround		= 1 <<  2,
	CorpseFilterNoBuilding		= 1 <<  3,
	CorpseFilterEnemy			= 1 <<  4,
	CorpseFilterAlly			= 1 <<  5,
	CorpseFilterIncludingSelf	= 1 <<  6,
	CorpseFilterSpellcaster		= 1 <<  7,
	CorpseFilterNotSelf			= 1 <<  8,
	CorpseFilterShielded		= 1 <<  9,
	CorpseFilterNoTrap			= 1 << 10,
	CorpseFilterOwn				= 1 << 11,
	CorpseFilterNotWorker		= 1 << 12,
	CorpseFilterNotSubunit		= 1 << 13,
	CorpseFilterBuilding		= 1 << 14,
	CorpseFilterNoEgg			= 1 << 15,
	CorpseFilterNoLarva			= 1 << 16,
	CorpseFilterNoHangar		= 1 << 17,
	CorpseFilterVisible			= 1 << 18,
};

namespace bw {
Box32 clamp_box32(Box32 rect);
u32 distance_between(Point16 p1, Point16 p2);
};

void reset_search_index();
void update_search_index(); // must be done once per frame
void update_search_index_effects(); // must be done once per frame after effects are updated

bool unit_flag_match(Unit* unit, Unit* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids = {});
bool corpse_flag_match(Thingy* target, Thingy* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids);

bool is_point_in_rect(Point16 point, Box32 rect);
bool is_rect_in_rect(Box32 inner, Box32 outer);

// TODO: rename to SearchResults
template <typename T>
class EntryArray {
public:
	void add(T* data) { m_data.push_back(data); }
	void clear() { return m_data.clear(); }
	size_t size() const { return m_data.size(); }

	std::vector<T*>::iterator begin() const { return std::begin(m_data); }
	std::vector<T*>::iterator end() const { return std::begin(m_data); }

    template <typename ScoreFunc>
    EntryArray& sort(ScoreFunc&& score_func) {
        std::ranges::stable_sort(m_data, [&score_func](T* a, T* b) {
            return score_func(a) > score_func(b);	
        });
        return *this;
    }

    EntryArray& sort_by_distance(Point16 center) {
        std::ranges::stable_sort(m_data, [center](T* a, T* b) {
            return bw::distance_between(a->position, center) < bw::distance_between(b->position, center);	
        });
        return *this;
    }

    template <typename FilterFunc>
    EntryArray& filter_by(FilterFunc&& filter_func) {
        std::erase_if(m_data, [&filter_func](T* data) {
            return !filter_func(data);
        });
    }

    EntryArray& filter(Unit* reference, u32 flags = 0, std::unordered_set<u16> allowed_ids = {}) requires std::same_as<T, Unit> {
        filter_by([reference, flags, &allowed_ids](Unit* unit) {
            return unit_flag_match(unit, reference, flags, allowed_ids);
        });
        return *this;
    }

    EntryArray& filter(Thingy* reference, u32 flags = 0, std::unordered_set<u16> allowed_ids = {}) requires std::same_as<T, Thingy> {
        filter_by([reference, flags, &allowed_ids](Thingy* corpse) {
            return corpse_flag_match(corpse, reference, flags, allowed_ids);
        });
        return *this;
    }

    template <typename ScoreFunc>
    T* best(ScoreFunc&& score_func) {
        T* best = nullptr;
        using Score = std::invoke_result_t<ScoreFunc, T*>;
        Score best_score = std::numeric_limits<Score>::min();

        for (T* elem : m_data) {
            Score score = score_func(elem);
            if (score > best_score) {
                best = elem;
                best_score = score;
            }
        }

        return best;
    }

    T* first() {
        for (T* elem : m_data) {
            return elem;
        }
        return nullptr;
    }

    template <typename FilterFunc>
    T* first(FilterFunc&& filter_func) {
        for (T* elem : m_data) {
            if (filter_func(elem)) {
                return elem;
            }
        }
        return nullptr;
    }

    EntryArray& add_units_from_connected_transport(Unit* unit) requires std::same_as<T, Unit> {
        if (!(unit->status & UnitStatus::InTransport) || !unit->connected_unit) {
            return;
        }

        Unit* transport = unit->connected_unit;
        for (u32 i = 0; i < TransportCap; ++i) {
            Unit* cargo_unit = transport->get_loaded_unit(i);
            if (!cargo_unit) {
                continue;
            }
            add(cargo_unit);
        }

        return *this;
    }

private:
	std::vector<T*> m_data;
};

template <>
EntryArray<Thingy>& EntryArray<Thingy>::sort_by_distance(Point16 center) {
    std::erase_if(m_data, [](Thingy* a) { return !a->sprite; });
    std::ranges::stable_sort(m_data, [center](Thingy* a, Thingy* b) {
        return bw::distance_between(a->sprite->position, center) < bw::distance_between(b->sprite->position, center);	
    });
    return *this;
}

template <typename T>
class SearchIndex {
public:
    static constexpr u32 EntryLimit = UnitLimit;
    static constexpr u32 CellSize = 8;
    static constexpr u32 CellPxSize = (CellSize * 32);
    static constexpr u32 CellCount = (256 / CellSize);

    struct Entry {
        Entry* next;
        Point16 position;
        T* data;
    };

    void reset() {
        m_size = 0;
        memset(m_entries, 0, sizeof(m_entries));
        memset(m_grid, 0, sizeof(m_grid));
    }

    void add(T* data, Point16 position) {
        if (m_size >= EntryLimit) {
            return;
        }

        Entry& entry = m_entries[m_size++];
        entry.position = position;
        entry.data = data;

        u32 x = position.x / CellPxSize;
        u32 y = position.y / CellPxSize;
        entry.next = m_grid[x][y];
        m_grid[x][y] = &entry;
    }

    EntryArray<T*> rect(Box32 rect) {
		EntryArray<T*> result;

        rect = bw::clamp_box32(rect);
        const u32 x0 = rect.left / CellPxSize;
        const u32 y0 = rect.top / CellPxSize;
        const u32 x1 = (rect.right + CellPxSize - 1) / CellPxSize;
        const u32 y1 = (rect.bottom + CellPxSize - 1) / CellPxSize;
        for (u32 x = x0; x <= x1; x++) {
            for (u32 y = y0; y <= y1; y++) {
                Box32 tile_rect = {
                    x * CellPxSize,
                    y * CellPxSize,
                    x * CellPxSize + CellPxSize,
                    y * CellPxSize + CellPxSize
                };
                bool is_fully_covered = is_rect_in_rect(tile_rect, rect);

                for (Entry& entry = m_grid[x][y]; entry; entry = entry->next) {
                    if (is_fully_covered || is_point_in_rect(entry.position, rect)) {
						result.push_back(entry.data);
                    }
                }
            }
        }

        filter_junk(result);
        return result;
    }

    EntryArray<T*> square(Point16 center, u32 radius) {
        return rect({
            center.x - (s32)radius,
            center.y - (s32)radius,
            center.x + (s32)radius,
            center.y + (s32)radius
        });
    }

    EntryArray<T*> radial(Point16 center, u32 radius) {
		EntryArray<T*> result;

        Box32 rect = {
            center.x - (s32)radius,
            center.y - (s32)radius,
            center.x + (s32)radius,
            center.y + (s32)radius
        };
        rect = bw::clamp_box32(rect);

        const u32 x0 = rect.left / CellPxSize;
        const u32 y0 = rect.top / CellPxSize;
        const u32 x1 = (rect.right + CellPxSize - 1) / CellPxSize;
        const u32 y1 = (rect.bottom + CellPxSize - 1) / CellPxSize;
        for (u32 x = x0; x <= x1; x++) {
            for (u32 y = y0; y <= y1; y++) {
                Point16 tile_center = {
                    x * CellPxSize + CellPxSize / 2,
                    y * CellPxSize + CellPxSize / 2
                };
                bool is_fully_covered = bw::distance_between(center, tile_center) + CellPxSize * 1.414 <= radius;

                for (Entry& entry = m_grid[x][y]; entry; entry = entry->next) {
                    if (is_fully_covered || bw::distance_between(center, entry->position) <= radius) {
						result.push_back(entry.data);
                    }
                }
            }
        }

        filter_junk(result);
		return result;
    }
    
    // TODO: rect_ex, square_ex, radial_ex for search + default filters

    void filter_junk(EntryArray<T*>& array) {}

private:
	u32 m_size;
	Entry m_entries[EntryLimit];
	Entry* m_grid[CellCount][CellCount];
};

inline SearchIndex<Unit>        g_unit_search;
inline SearchIndex<Unit>        g_seen_unit_search[PlayerLimit];
inline SearchIndex<Bullet>      g_bullet_search;
inline SearchIndex<Thingy>      g_corpse_search;
inline SearchIndex<EffectField> g_effect_search;

template <>
void SearchIndex<Unit>::filter_junk(EntryArray<Unit*>& entry_array) {
    entry_array.filter_by([](Unit* unit) {
        return !unit->is_dead();
    });
}

template <>
void SearchIndex<Bullet>::filter_junk(EntryArray<Bullet*>& entry_array) {
    entry_array.filter_by([](Bullet* bullet) {
        return bullet->hitpoints;
    });
}

template <>
void SearchIndex<Thingy>::filter_junk(EntryArray<Thingy*>& entry_array) {
    entry_array.filter_by([](Thingy* corpse) {
        return !(corpse->sprite->flags & SpriteFlags::RemoveAfterDeath);
    });
}