#pragma once
#include "SCBW/api.h"

struct LoadedFile {
	size_t size;
	char* data;
};

bool file_exists(char* filename);
LoadedFile load_entire_sfile(char* filename);
void free_loaded_file(LoadedFile* file);
u32 hash_string(char* string);