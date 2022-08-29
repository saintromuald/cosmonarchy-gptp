#pragma once
#include "SCBW/api.h"

struct LoadedFile {
	u32 size;
	char* data;
};

bool file_exists(char* filename);
LoadedFile load_entire_sfile(char* filename);
void free_loaded_file(LoadedFile* file);
u32 hash_string(char* string);

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