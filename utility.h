#pragma once
#include "SCBW/api.h"

// TODO: consider just returning std::string (probably preferrable?) or making SFile internally have std::vector or making SFile internally have unique_ptr with custom deleter,
//       since currently having to define explicit copy/move is pretty ugly
class SFile {
public:
	static bool exists(const char* filename);
	static SFile load(const char* filename);

	~SFile();
	SFile(const SFile&) = delete;
	SFile& operator=(const SFile&) = delete;
	SFile(SFile&& other) : m_size(std::exchange(other.m_size, 0)), m_data(std::exchange(other.m_data, nullptr)) {}
	SFile& operator=(SFile&& other) { std::swap(*this, other); return *this; };

	u32 size() const { return m_size; }
	char* data() const { return m_data; }

	char* begin() const { return m_data; }
	char* end() const { return m_data + m_size; }

private:
	SFile() {} // use SFile::load
	u32 m_size = 0;
	char* m_data = 0;
};

constexpr u32 hash_string(const char* string) {
	u32 result = 2166136261;
	for (const char* c = string; *c; c++) {
		result ^= *c;
		result *= 16777619;
	}
	return result;
}

consteval auto comptime(auto value) {
	return value;
}

struct StringBuilder {
	u32 capacity;
	u32 size;
	char* data;
};

void sb_resize(StringBuilder* sb, u32 needed_size);
void sb_reserve(StringBuilder* sb, u32 size);
void sb_clear(StringBuilder* sb);
void sb_free(StringBuilder* sb);
void sb_append(StringBuilder* sb, char c);
void sb_printf(StringBuilder* sb, char* format, ...);

#define MemoryBlockSize (1024 * 1024u)
struct MemoryBlock {
	MemoryBlock* next;
	u32 size;
	u32 capacity;
	char data[0];
};

struct MemoryArena {
	MemoryBlock* block;
};

struct TempArena {
	MemoryArena* arena;
	MemoryBlock* block;
	u32 size;
};

char* arena_push(MemoryArena* arena, u32 size);
#define arena_push_struct(arena, type)		 (type*) arena_push(arena, sizeof(type))
#define arena_push_array(arena, count, type) (type*) arena_push(arena, count * sizeof(type))
char* arena_push_string(MemoryArena* arena, char* string);
char* arena_printf(MemoryArena* arena, char* format, ...);

void arena_free_last(MemoryArena* arena);
void arena_free(MemoryArena* arena);

TempArena start_temp_arena(MemoryArena* arena);
void end_temp_arena(TempArena* temp);