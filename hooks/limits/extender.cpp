#include "extender.h"

void Extender::uplink(std::vector<u32> address_list, u32 data)
{
	for (auto a : address_list)
	{
		memory_patch(a, data);
	}
}

bool starts_with(std::string line, char c)
{
	if (line.length() == 0)
	{
		return false;
	}
	return line[0] == c;
}

std::vector<std::string> split_by(std::string line, char delimiter)
{
	std::string tmp;
	std::stringstream ss(line);
	std::vector<std::string> result;
	while (std::getline(ss, tmp, delimiter))
	{
		result.push_back(tmp);
	}
	return result;
}
char tolower(char c)
{
	if (c <= 'Z' && c >= 'A')
		return c - ('Z' - 'z');
	return c;
}
std::string to_lowercase(std::string line)
{
	std::string str;
	for (auto a : line)
	{
		str.push_back(tolower(a));
	}
	return str;
}
#include <logger.h>
s32 Extender::get_file_entry_count(char* name, u32 entries)
{
	//	std::string str = scbw::read_samase_file(name);
	const char* data = nullptr;
	u32 len = 0;
	auto data_ptr = &data;
	auto len_ptr = &len;
	u32 val = reinterpret_cast<u32>(name);
	//GPTP::logger << "log1...\n";
	auto length = scbw::get_aise_value(nullptr, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
	char* buf = (char*)scbw::get_aise_value(nullptr, nullptr, AiseId::ReadSamaseFile_Ptr, 0, 0);
	//GPTP::logger << "log2...\n";

	GPTP::logger << "Read samase file: " << (u32)buf << " " << length << '\n';

	std::string str;
	str = std::string(buf, length);
	//
	//GPTP::logger << "Read samase file: " << str << '\n';



	std::vector<std::string> lines = split_by(str, '\n');
	GPTP::logger << "Str size: " << str.size() << ", Line size: " << lines.size() << '\n';
	/*
	if (str.count() > 0) {
		GPTP::logger << str << '\n';
	}*/
	u32 count = 0;
	for (auto line : lines)
	{

		if (starts_with(line, '#') || starts_with(line, ';') || line.length() == 0)
		{
			//GPTP::logger << "bad line, continue" << '\n';
			continue;
		}
		std::vector<std::string> command_list = split_by(line, ':');
		//GPTP::logger << "split length: " << command_list.count() << '\n';
		if (command_list.size() != 2)
		{
			continue;
		}
		auto entry_name = command_list[0];
		auto id = identifier(to_lowercase(entry_name));
		//GPTP::logger << "identifier: " << id << '\n';
		if (id != NO_IDENTIFIER)
		{
			count++;
		}
	}
	count /= entries;
	return count;
}

static inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](s32 ch)
	{
		return !std::isspace(ch);
	}));
}
static inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](s32 ch)
	{
		return !std::isspace(ch);
	}).base(), s.end());
}
static inline void trim(std::string& s)
{
	ltrim(s);
	rtrim(s);
}
bool last_char(std::string s, char c)
{
	if (s.size() > 0 && s[s.size() - 1] == c)
	{
		return true;
	}
	return false;
}
void Extender::parse_set(char* name, u32 default_max_limit, u32 entries)
{
	//std::string str = scbw::read_samase_file(name);



	const char* data = nullptr;
	u32 len = 0;
	auto data_ptr = &data;
	auto len_ptr = &len;
	u32 val = reinterpret_cast<u32>(name);
	//GPTP::logger << "log1...\n";
	auto length = scbw::get_aise_value(nullptr, reinterpret_cast<Unit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<s32>(data_ptr), reinterpret_cast<s32>(len_ptr));
	char* buf = (char*)scbw::get_aise_value(nullptr, nullptr, AiseId::ReadSamaseFile_Ptr, 0, 0);
	//GPTP::logger << "log2...\n";
	GPTP::logger << "Read samase file2: " << (u32)buf << " " << length << '\n';
	std::string str;
	str = std::string(buf, length);



	std::vector<std::string> lines = split_by(str, '\n');
	u32 entry_count = 0;
	u32 current_entry = default_max_limit;
	GPTP::logger << "parse set reader2: " << lines.size() << '\n';
	for (auto line : lines)
	{
		if (starts_with(line, '#') || starts_with(line, ';') || line.length() == 0)
		{
			continue;
		}
		std::vector<std::string> command_list = split_by(line, ':');
		if (command_list.size() != 2)
		{
			continue;
		}
		GPTP::logger << "Next line, cmds = 2" << '\n';
		auto entry_name = command_list[0];
		auto value_string = command_list[1];
		trim(entry_name);
		trim(value_string);
		auto id = identifier(to_lowercase(entry_name));
		if (id != NO_IDENTIFIER)
		{

			if (is_string_id(id))
			{
				GPTP::logger << "Is string id\n";
				set_value(current_entry, id, value_string);
			}
			else
			{
				GPTP::logger << "Is integer id\n";
				u32 val = parse_entry_value(value_string, id);
				set_value(current_entry, id, val);
			}
			entry_count += 1;
			if (entry_count == entries)
			{
				entry_count = 0;
				current_entry += 1;
			}
		}
	}
}