#pragma once

#include "storm/storm.h"

#include <memory>
#include <tuple>

namespace smem {
template <typename T>
struct deleter {
    void operator()(T* p) const noexcept {
        if (!p) {
            return;
        }

        std::destroy_at(p);
        SMemFree(p);
    }
};

template <typename T, std::size_t N>
struct deleter<T[N]> {
    void operator()(T* p) const noexcept {
        if (!p) {
            return;
        }

        std::destroy_n(p, N);
        SMemFree(p);
    }
};

template <typename T>
struct deleter<T[]> {
    std::size_t count{};

    void operator()(T* p) const noexcept {
        if (!p) {
            return;
        }

        std::destroy_n(p, count);
        SMemFree(p);
    }
};

template <typename T>
struct ptr_impl {
    using type = std::unique_ptr<T, deleter<T>>;
};

template <typename T, std::size_t N>
struct ptr_impl<T[N]> {
    using type = std::unique_ptr<T[], deleter<T[N]>>;
};

template <typename T>
struct ptr_impl<T[]> {
    using type = std::unique_ptr<T[], deleter<T[]>>;
};

template <typename T>
using ptr = typename ptr_impl<T>::type;

template <typename T, typename... Args>
ptr<T> make(Args&&... args) {
    using E = std::remove_extent_t<T>;
    if constexpr (std::is_unbounded_array_v<T>) {
        static_assert(sizeof...(Args) == 1,
            "make<T[]>(count) expects exactly one argument.");

        std::size_t count = static_cast<std::size_t>(
            std::get<0>(std::forward_as_tuple(args...)));

        E* p = static_cast<E*>(SMemAlloc(sizeof(E) * count));
        if (!p) {
            throw std::bad_alloc();
        }

        std::size_t i = 0;
        try {
            for (; i < count; ++i) {
                std::construct_at(p + i);
            }
        } catch (...) {
            std::destroy_n(p, i);
            SMemFree(p);
            throw;
        }

        return ptr<T>{p, deleter<T>{count}};
    } else if constexpr (std::is_bounded_array_v<T>) {
        constexpr std::size_t count = std::extent_v<T>;

        E* p = static_cast<E*>(SMemAlloc(sizeof(E) * count));
        if (!p) {
            throw std::bad_alloc();
        }

        std::size_t i = 0;

        try {
            for (; i < count; ++i) {
                std::construct_at(p + i);
            }
        } catch (...) {
            std::destroy_n(p, i);
            SMemFree(p);
            throw;
        }

        return ptr<T>{p};
    } else {
        E* p = static_cast<E*>(SMemAlloc(sizeof(E)));
        if (!p) {
            throw std::bad_alloc();
        }

        try {
            std::construct_at(p, std::forward<Args>(args)...);
        } catch (...) {
            SMemFree(p);
            throw;
        }

        return ptr<T>{p};
    }
}
} // namespace smem
