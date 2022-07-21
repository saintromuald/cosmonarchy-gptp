#include "trigger_utils.h"

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
		loaded_file.data = (char*) calloc(loaded_file.size + 1, 1);
		helper::SFileReadFile(file, loaded_file.data, loaded_file.size, 0, 0);
		loaded_file.data[loaded_file.size] = 0;
	}
	helper::SFileCloseFile(file);

	return loaded_file;
}

void free_loaded_file(LoadedFile* file) {
	if (file->data) {
		free(file->data);
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