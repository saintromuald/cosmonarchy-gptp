#pragma once
#include <BW\bwdata.h>
#include <globals.h>
#include <functional>
#include <unordered_set>
#include <ranges>
#include <variant>

#include "arena.h"
#include "effect_field.h"

enum SearchFilter : u64 {
	FilterInvincible			= 1ull <<  0,
	FilterNoAir					= 1ull <<  1,
	FilterNoGround				= 1ull <<  2,
	FilterNoMechanical			= 1ull <<  3,
	FilterNoOrganic				= 1ull <<  4,
	FilterNoBuilding			= 1ull <<  5,
	FilterOnlyOrganic			= 1ull <<  6,
	FilterOnlyMechanic			= 1ull <<  7,
	FilterEnemy					= 1ull <<  8,
	FilterAlly					= 1ull <<  9,
	FilterIncludingSelf			= 1ull << 10,
	FilterFrontalArc			= 1ull << 11,
	FilterSpellcaster			= 1ull << 12,
	FilterHasEnergy				= 1ull << 13,
	FilterNotSelf				= 1ull << 14,
	FilterShielded				= 1ull << 15,
	FilterEnabled				= 1ull << 16,
	FilterPowered				= 1ull << 17,
	FilterCompleted				= 1ull << 18,
	FilterNoTrap				= 1ull << 19,
	FilterNoHallucination		= 1ull << 20,
	FilterCanAttackOrCast		= 1ull << 21,
	FilterOwn					= 1ull << 22,
	FilterNotWorker				= 1ull << 23,
	FilterNotSubunit			= 1ull << 24,
	FilterBuilding				= 1ull << 25,
	FilterNoEgg					= 1ull << 26,
	FilterNoLarva				= 1ull << 27,
	FilterNoHangar				= 1ull << 28,
	FilterVisible				= 1ull << 29,
	FilterTownCenter			= 1ull << 30,
	FilterLifted                = 1ull << 31,
	FilterComputer              = 1ull << 32,
	FilterCombatant             = 1ull << 33,
	FilterNeutral               = 1ull << 34,
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
u32 distance_between(v2u16 p1, v2u16 p2);
bool are_mutual_allies(u32 a, u32 b);
};

u16 bw_random();

void refresh_search_index(); // must be done once per frame
void update_search_index_effects(); // must be done once per frame after effects are updated

bool unit_flag_match(Unit* unit, Unit* center = nullptr, u32 flags = 0, const std::unordered_set<u16>& allowed_unit_ids = {});
bool corpse_flag_match(Thingy* target, Thingy* center = nullptr, u32 flags = 0, const std::unordered_set<u16>& allowed_unit_ids = {});

bool is_point_in_rect(v2u16 point, Box32 rect);
bool is_rect_in_rect(Box32 inner, Box32 outer);

template <typename... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };

inline mem::Arena g_search_arena;

constexpr size_t EntryArrayDefaultReserve = 8;

// TODO: rename to SearchResults
template <typename T>
class EntryArray {
public:
    EntryArray(size_t reserve = EntryArrayDefaultReserve) {
        m_data.reserve(reserve);
    }

	void add(T* data) { m_data.push_back(data); }
	void clear() { return m_data.clear(); }
	size_t count() const { return m_data.size(); }
    T*& operator[](u32 index) { return m_data[index]; }

	std::vector<T*>::const_iterator begin() const { return std::begin(m_data); }
	std::vector<T*>::const_iterator end() const { return std::end(m_data); }
	std::vector<T*>::iterator begin() { return std::begin(m_data); }
	std::vector<T*>::iterator end() { return std::end(m_data); }

    // We need to use "deducing this", otherwise it would destruct the vector in chains in for each loops
    // since otherwise we are implicitly converting to lvalue reference, and losing rvalue category
    //
    // This implementation means returning a move of self on each chain call on a temporary (rvalue),
    // and means returning a reference to self on each call otherwise

    template <typename Func>
    auto apply_if(this auto&& self, bool condition, Func&& func) {
        if (condition) return func(std::forward<decltype(self)>(self));
        return self;
    }

    template <typename ScoreFunc>
    auto sort(this auto&& self, ScoreFunc&& score_func) {
        using Pair = std::pair<T*, std::invoke_result_t<ScoreFunc, T*>>;
        auto pairs = self.m_data
            | std::views::transform([&score_func](T* a) { return Pair{a, score_func(a)}; })
            | std::ranges::to<mem::Arena::vector<Pair>>(g_search_arena.allocator());
        std::ranges::stable_sort(pairs, std::less<>{}, &Pair::second);
        std::ranges::transform(pairs, self.m_data.begin(), &Pair::first);
        return self;
    }

    static v2u16 position_of(T* entity) {
        if constexpr (std::same_as<T, Thingy>) {
            return entity->sprite->position;
        } else {
            return entity->position;
        }
    }

    auto sort_by_distance(this auto&& self, v2u16 center) {
        return self.sort([center](T* a) { return bw::distance_between(position_of(a), center); });
    }

    auto filter_by(this auto&& self, auto&& filter_func) {
        std::erase_if(self.m_data, [&filter_func](T* data) { return !filter_func(data); });
        return self;
    }

    auto filter(this auto&& self, Unit* reference = nullptr, u32 flags = 0, const std::unordered_set<u16>& allowed_ids = {}) requires std::same_as<T, Unit> {
        return self.filter_by([reference, flags, &allowed_ids](Unit* unit) {
            return unit_flag_match(unit, reference, flags, allowed_ids);
        });
    }

    auto filter(this auto&& self, Thingy* reference = nullptr, u32 flags = 0, const std::unordered_set<u16>& allowed_ids = {}) requires std::same_as<T, Thingy> {
        return self.filter_by([reference, flags, &allowed_ids](Thingy* corpse) {
            return corpse_flag_match(corpse, reference, flags, allowed_ids);
        });
    }

    auto owned_by(this auto&& self, u32 player) requires std::same_as<T, Unit> {
        return self.filter_by([player](Unit* unit) {
            return unit->player == player;
        });
    }

    auto not_owned_by(this auto&& self, u32 player) requires std::same_as<T, Unit> {
        return self.filter_by([player](Unit* unit) {
            return unit->player != player;
        });
    }

    auto allied_to(this auto&& self, u32 player) requires std::same_as<T, Unit> {
        return self.filter_by([player](Unit* unit) {
            return bw::are_mutual_allies(unit->player, player);
        });
    }

    auto hostile_to(this auto&& self, u32 player) requires std::same_as<T, Unit> {
        return self.filter_by([player](Unit* unit) {
            return !bw::are_mutual_allies(unit->player, player);
        });
    }

    auto not_neutral(this auto&& self) requires std::same_as<T, Unit> {
        return self.filter_by([](Unit* unit) {
            return unit->player < PlayerLimit;
        });
    }

    // TODO: add filter methods (to eventually simplify filter to just flags):
    // facing(dir, angle_deg), not_facing(dir, angle_deg),
    // including(unit), excluding(unit),
    // allowed_unit_ids(unit_ids), excluded_unit_ids(unit_ids),

    T* best(auto&& score_func) {
        T* best = nullptr;
        using Score = std::invoke_result_t<decltype(score_func), T*>;
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

    T* closest_to(v2u16 pos) {
        T* best = nullptr;
        u32 best_distance = UINT_MAX;

        for (T* elem : m_data) {
            u32 distance = bw::distance_between(elem->position, pos);
            if (distance < best_distance) {
                best_distance = distance;
                best = elem;
            }
        }
        return best;
    }

    auto take(this auto&& self, u32 count) {
        if (self.m_data.size() > count) {
            self.m_data.resize(count);
        }
        return self;
    }

    auto take_fraction(this auto&& self, fp32 fraction) {
        return self.take(u32(self.count() * fraction));
    }

    T* first(auto&& filter_func) {
        for (T* elem : m_data) {
            if (filter_func(elem)) {
                return elem;
            }
        }
        return nullptr;
    }

    auto add_units_from_connected_transport(this auto&& self, Unit* unit) requires std::same_as<T, Unit> {
        if (!(unit->status & UnitStatus::InTransport) || !unit->connected_unit) {
            return self;
        }

        Unit* transport = unit->connected_unit;
        for (u32 i = 0; i < TransportCap; ++i) {
            Unit* cargo_unit = transport->get_loaded_unit(i);
            if (!cargo_unit) {
                continue;
            }
            self.add(cargo_unit);
        }

        return self;
    }

    auto shuffle(this auto&& self) requires std::same_as<T, Unit> {
        struct BwRandomGenerator {
            using result_type = unsigned short;

            result_type operator()() {
                return bw_random();
            }

            static constexpr result_type min() { return 0; }
            static constexpr result_type max() { return 32767; }
        };
        std::ranges::shuffle(self.m_data, BwRandomGenerator{});
        return self;
    }

    // Tbh I don't think this is necessary since on lvalue references all the functions should copy automatically,
    // but marking them all const seems incorrect for rvalue reference context
    EntryArray<T> clone() const {
        return *this;
    }

private:
    mem::Arena::vector<T*> m_data{g_search_arena.allocator<T*>()};
};

template <typename T>
class SearchIndex {
public:
    static constexpr u32 EntryLimit = UnitLimit;
    static constexpr u32 CellSize = 8;
    static constexpr u32 CellPxSize = (CellSize * 32);
    static constexpr u32 CellCount = (256 / CellSize);

    struct Entry {
        Entry* next;
        v2u16 position;
        T* data;
    };

    void reset() {
        m_size = 0;
        memset(m_entries, 0, sizeof(m_entries));
        memset(m_grid, 0, sizeof(m_grid));
    }

    void add(T* data, v2u16 position) {
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

    EntryArray<T> rect(Box32 rect, u32 limit = 0) {
		EntryArray<T> result(limit != 0 ? limit : EntryArrayDefaultReserve);

        rect = bw::clamp_box32(rect);
        const u32 x0 = rect.left / CellPxSize;
        const u32 y0 = rect.top / CellPxSize;
        const u32 x1 = (rect.right + CellPxSize - 1) / CellPxSize;
        const u32 y1 = (rect.bottom + CellPxSize - 1) / CellPxSize;

        for (u32 x = x0; x < x1; x++) {
            for (u32 y = y0; y < y1; y++) {
                Box32 tile_rect = {
                    x * CellPxSize,
                    y * CellPxSize,
                    x * CellPxSize + CellPxSize,
                    y * CellPxSize + CellPxSize
                };
                bool is_fully_covered = is_rect_in_rect(tile_rect, rect);

                for (Entry* entry = m_grid[x][y]; entry; entry = entry->next) {
                    if (is_fully_covered || is_point_in_rect(entry->position, rect)) {
						result.add(entry->data);
                        if (limit && result.count() >= limit) {
                            goto end;
                        }
                    }
                }
            }
        }

    end:
        filter_junk(result);
        return result;
    }

    EntryArray<T> all(u32 limit = 0) {
		EntryArray<T> result(limit != 0 ? limit : m_size);
        for (u32 i = 0; i < m_size; i++) {
            result.add(m_entries[i].data);
            if (limit && result.count() >= limit) {
                goto end;
            }
        }

    end:
        filter_junk(result);
        return result;
    }

    EntryArray<T> square(v2u16 center, u32 radius, u32 limit = 0) {
        return rect({
            center.x - (s32)radius,
            center.y - (s32)radius,
            center.x + (s32)radius,
            center.y + (s32)radius
        }, limit);
    }

    EntryArray<T> radial(v2u16 center, u32 radius, u32 limit = 0) {
		EntryArray<T> result(limit != 0 ? limit : EntryArrayDefaultReserve);

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
        for (u32 x = x0; x < x1; x++) {
            for (u32 y = y0; y < y1; y++) {
                v2u16 tile_center{
                    u16(x * CellPxSize + CellPxSize / 2),
                    u16(y * CellPxSize + CellPxSize / 2)
                };
                bool is_fully_covered = bw::distance_between(center, tile_center) + CellPxSize * 1.414fp <= fp32(radius);

                for (Entry* entry = m_grid[x][y]; entry; entry = entry->next) {
                    if (is_fully_covered || bw::distance_between(center, entry->position) <= radius) {
						result.add(entry->data);
                        if (limit && result.count() >= limit) {
                            goto end;
                        }
                    }
                }
            }
        }

    end:
        filter_junk(result);
		return result;
    }

    EntryArray<T> square(v2u16 center, SearchRadius radius) {
        return std::visit(Overloaded{
            [this, center](v2u16 size) {
                return rect(Box32{
                    center.x - size.x / 2,
                    center.y - size.y / 2,
                    center.x + size.x / 2,
                    center.y + size.y / 2
                });
            },
            [this, center](u32 radius) {
                return square(center, radius);
            },
        }, radius);
    }

    EntryArray<T> search(v2u16 center, SearchRadius radius) {
        return std::visit(Overloaded{
            [this, center](v2u16 size) {
                // TODO: consider oval? then rename to radial
                return rect(Box32{
                    center.x - size.x / 2,
                    center.y - size.y / 2,
                    center.x + size.x / 2,
                    center.y + size.y / 2
                });
            },
            [this, center](u32 radius) {
                return radial(center, radius);
            },
        }, radius);
    }
    
    EntryArray<Unit> square_ex(Unit* unit, u32 radius, u32 filter = 0, const std::unordered_set<u16>& allowed_unit_ids = {}) requires std::same_as<T, Unit> {
        return square(unit->position, radius).filter(unit, filter, allowed_unit_ids);
    }

    EntryArray<Unit> radial_ex(Unit* unit, u32 radius, u32 filter = 0, const std::unordered_set<u16>& allowed_unit_ids = {}) requires std::same_as<T, Unit> {
        return radial(unit->position, radius).filter(unit, filter, allowed_unit_ids);
    }
    
    EntryArray<Unit> square_ex(const EffectField& effect, u32 filter = 0, const std::unordered_set<u16>& allowed_unit_ids = {}) requires std::same_as<T, Unit> {
        return square(effect.position, effect.radius).filter(effect.source, filter, allowed_unit_ids);
    }

    EntryArray<Unit> search_ex(const EffectField& effect, u32 filter = 0, const std::unordered_set<u16>& allowed_unit_ids = {}) requires std::same_as<T, Unit> {
        return search(effect.position, effect.radius).filter(effect.source, filter, allowed_unit_ids);
    }

    void filter_junk(EntryArray<T>& array) {
        array.filter_by(Overloaded {
            [](Unit* unit) {
                return !unit->is_dead();
            },
            [](Bullet* bullet) {
                return bullet->hitpoints;
            },
            [](Thingy* corpse) {
                return corpse->sprite && !(corpse->sprite->flags & SpriteFlags::RemoveAfterDeath);
            },
            [](auto*) {
                return true;
            },
        });
    }

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