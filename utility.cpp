#include "utility.h"
#include <stdarg.h>

bool file_exists(char* filename) {
	HANDLE file = 0;
	helper::SFileOpenFile(filename, &file);
	if (file) {
		helper::SFileCloseFile(file);
		return true;
	}
	return false;
}

LoadedFile load_entire_sfile(char* filename) {
	HANDLE file = 0;
	helper::SFileOpenFile(filename, &file);
	if (!file) {
		scbw::printf("Error: File %s missing!", filename);
		return {};
	}

	DWORD high = 0;
	DWORD size = helper::SFileGetFileSize(file, &high);
	if (high) {
		scbw::printf("Error: File %s too big!", filename);
		return {};
	}

	LoadedFile loaded_file = {};
	loaded_file.size = size;
	if (loaded_file.size) {
		loaded_file.data = (char*)helper::SMemAlloc(loaded_file.size + 1);
		helper::SFileReadFile(file, loaded_file.data, loaded_file.size, 0, 0);
		loaded_file.data[loaded_file.size] = 0;
	}
	helper::SFileCloseFile(file);

	return loaded_file;
}

void free_loaded_file(LoadedFile* file) {
	if (file->data) {
		helper::SMemFree(file->data);
	}
	file->size = 0;
	file->data = 0;
}

u32 hash_string(char* string) {
	u32 hash = 5381;
	for (char* c = string; *c; ++c) {
		hash = ((hash << 5) + hash) + *c;
	}
	return hash;
}

void sb_resize(StringBuilder* sb, u32 needed_size) {
	if (sb->size + needed_size > sb->capacity) {
		u32 new_capacity = sb->capacity ? sb->capacity * 2 + needed_size : 512 + needed_size;
		if (new_capacity < sb->capacity) { // Overflow
			new_capacity = UINT_MAX;
		}
		char* new_data = (char*)helper::SMemAlloc(new_capacity + 1);
		if (new_data) {
			if (sb->data) {
				memcpy(new_data, sb->data, sb->size);
				helper::SMemFree(sb->data);
			}
			sb->capacity = new_capacity;
			sb->data = new_data;
		}
	}
}

void sb_reserve(StringBuilder* sb, u32 size) {
	sb_resize(sb, size);
	sb->size += size;
}

void sb_clear(StringBuilder* sb) {
	memset(sb->data, 0, sb->size);
	sb->size = 0;
}

void sb_free(StringBuilder* sb) {
	helper::SMemFree(sb->data);
	sb->size = 0;
	sb->capacity = 0;
	sb->data = 0;
}

void sb_append(StringBuilder* sb, char c) {
	sb_resize(sb, 1);
	sb->data[sb->size++] = c;
}

void sb_printf(StringBuilder* sb, char* format, ...) {
	va_list va = {};
	va_start(va, format);
	u32 size = vsnprintf(0, 0, format, va);
	va_end(va);
	sb_resize(sb, size);

	va_start(va, format);
	vsprintf(sb->data + sb->size, format, va);
	va_end(va);
	sb->size += size;
}

char* arena_push(MemoryArena* arena, u32 size) {
	MemoryBlock* block = arena->block;
	if (!block || block->size + size > block->capacity) {
		if (block->size) {
			u32 capacity = std::max(size, MemoryBlockSize);
			block = (MemoryBlock*)helper::SMemAlloc(sizeof(MemoryBlock) + capacity);
			block->capacity = capacity;
			block->prev = arena->block;
			arena->block = block;
		}
	}
	char* result = block->data + block->size;
	memset(result, 0, size);
	block->size += size;
	return result;
}

char* arena_push_string(MemoryArena* arena, char* string) {
	u32 size = strlen(string) + 1;
	char* result = arena_push(arena, size);
	memcpy(result, string, size);
	return result;
}

char* arena_printf(MemoryArena* arena, char* format, ...) {
	va_list va = {};
	va_start(va, format);
	u32 size = vsnprintf(0, 0, format, va);
	va_end(va);
	
	char* result = arena_push(arena, size + 1);
	va_start(va, format);
	vsprintf(result, format, va);
	va_end(va);
	return result;
}

void arena_free_last(MemoryArena* arena) {
	if (arena->block) {
		MemoryBlock* block = arena->block;
		arena->block = block->prev;
		helper::SMemFree(block);
	}
}

void arena_free(MemoryArena* arena) {
	while (arena->block) {
		arena_free_last(arena);
	}
}

TempArena start_temp_arena(MemoryArena* arena) {
	TempArena temp = {};
	temp.arena = arena;
	temp.block = arena->block;
	temp.size  = arena->block ? arena->block->size : 0;
	return temp;
}

void end_temp_arena(TempArena* temp) {
	while (temp->arena->block && temp->arena->block != temp->block) {
		arena_free_last(temp->arena);
	}
	if (temp->block) {
		temp->block->size = temp->size;
	}
}