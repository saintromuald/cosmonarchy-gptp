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

int Extender::get_file_entry_count(char* name, u32 entries) {
	std::string str = scbw::readSamaseFile(name);
	std::vector<std::string> lines = split_by(str, '\n');
	u32 count = 0;
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
			count++;
		}
	}
	count /= entries;
	return count;
}

static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}
void Extender::parse_set(char* name, u32 default_max_limit, u32 entries) {
	std::string str = scbw::readSamaseFile(name);
	std::vector<std::string> lines = split_by(str, '\n');
	u32 entry_count = 0;
	u32 current_entry = default_max_limit;
	for (auto line : lines) {
		if (starts_with(line, '#') || starts_with(line, ';') || line.length() == 0) {
			continue;
		}
		std::vector<std::string> command_list = split_by(line, ':');
		if (command_list.size() != 2) {
			continue;
		}
		auto entry_name = command_list[0];
		auto value_string = command_list[1];
		trim(entry_name);
		trim(value_string);

		auto id = identifier(to_lowercase(entry_name));
		if (id != NO_IDENTIFIER) {
			u32 value = parse_entry_value(value_string, id);
			//
			//
			entry_count += 1;
			if (entry_count == entries) {
				entry_count = 0;
				current_entry += 1;
			}
		}
	}
}