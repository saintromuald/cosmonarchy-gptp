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