#pragma once
#include <vector>

namespace bw {
void printf(const char* format, ...);
}

namespace arena {

class Arena {
public:
    Arena() = default;
    ~Arena() {
        free_all();
    }

    Arena(const Arena&) = delete;
    Arena(Arena&&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena& operator=(Arena&&) = delete;

    struct Block {
        Block* m_next; 
        size_t m_size;
        size_t m_cursor;
        char m_data[];

        size_t free_space() const {
            return m_size - m_cursor;
        }

        void* allocate(size_t size) {
            if (free_space() < size) {
                return nullptr;
            }

            void* result = m_data + m_cursor;
            m_cursor += size;
            return result;
        }
    };

    void* allocate(size_t size) {
        m_allocated += size;
        if (m_first) {
            if (auto result = m_first->allocate(size)) {
                return result;
            }
        }
        Block* block = allocate_block(size);
        return block->allocate(size);
    }

    void reset() {
        while (m_first) {
            Block* block = m_first;
            m_first = block->m_next; 
            block->m_next = m_free;
            m_free = block;
        }

        // bw::printf("Allocated %fKB", m_allocated / 1024.f);
        m_allocated = 0;
    }

    void free_all() {
        while (m_first) {
            Block* block = m_first;
            m_first = block->m_next; 
            free(block);
        }
        while (m_free) {
            Block* block = m_free;
            m_free = block->m_next; 
            free(block);
        }
    }
   
private:
    Block* allocate_block(size_t min_size) {
        Block* block;
        if (m_free && m_free->m_size >= min_size) {
            block = m_free;
            m_free = m_free->m_next;
            block->m_cursor = 0;
        } else {
            size_t size = std::max<size_t>(256 * 1024, min_size * 4);
            block = (Block*)calloc(1, sizeof(Block) + size);
            block->m_size = size;
        }
        block->m_next = m_first;
        m_first = block;
        return block;
    }

private:
    Block* m_first = nullptr;
    Block* m_free = nullptr;
    size_t m_allocated = 0;
};

template <typename T>
struct Allocator {
    using value_type = T;

    Allocator(Arena* arena) : m_arena{arena} {}

    template <typename Y>
    Allocator(const Allocator<Y>& other) : m_arena{other.m_arena} {}

    template <typename Y>
    bool operator==(const Allocator<Y>& other) const { return m_arena == other.m_arena; }

    template <typename Y>
    bool operator!=(const Allocator<Y>& other) const { return !(*this == other); }

    T* allocate(size_t count) {
        return (T*)m_arena->allocate(count * sizeof(T));
    }

    void deallocate(void* ptr, size_t count) {
        // noop
    }

    Arena* m_arena;
};

template <typename T>
using vector = std::vector<T, Allocator<T>>;

};
