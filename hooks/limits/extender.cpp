#include "extender.h"

void Extender::uplink(std::vector<u32> address_list, u32 data){
	for (auto a : address_list) {
		memoryPatch(a, data);
	}
}

bool starts_with(std::string line, char c) {
	if (line.length() == 0) {
		return false;
	}
	return line[0] == c;
}

std::vector<std::string> split_by(std::string line, char delimiter) {
	std::string tmp;
	std::stringstream ss(line);
	std::vector<std::string> result;
	while (std::getline(ss, tmp, delimiter)) {
		result.push_back(tmp);
	}
	return result;
}
char tolower(char c) {
	if (c <= 'Z' && c >= 'A')
		return c - ('Z' - 'z');
	return c;
}
std::string to_lowercase(std::string line) {
	std::string str;
	for (auto a : line) {
		str.push_back(tolower(a));
	}
	return str;
}

int Extender::get_file_entry_count(char* name, int entries) {
	std::string str = scbw::readSamaseFile(name);
	std::vector<std::string> lines = split_by(str, '\n');
	for (auto line : lines) {
		if (starts_with(line, '#') || starts_with(line, ';') || line.length() == 0) {
			continue;
		}
		std::vector<std::string> command_list = split_by(line, ':');
		if (command_list.size() != 2) {
			continue;
		}
		auto entry_name = command_list[0];
		auto id = identifier(to_lowercase(entry_name));
		if (id != NO_IDENTIFIER) {

		}
	}
	return 0;
}