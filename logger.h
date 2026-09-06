// Basic logging to text file supported by GPTP. Made by pastelmind

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <sstream>
#include <magic_enum/magic_enum_all.hpp>

#include "BW/bwdata.h"
#include "config.h"

namespace gptp {

class GameLogger {
public:
	GameLogger(const char* name, bool enabled)
		: m_name{name}, m_enabled{enabled} {}

	void log_game_start();
	void log_game_end();
	void log_end();

	template <typename T>
	GameLogger& operator<<(const T& t) {
        print_frame();
		if (try_open_log_file()) {
			m_log_file << t;
		}
        m_game_buffer << t;
		return *this;
	}

	GameLogger& operator<<(std::ostream& (*func)(std::ostream&));
	inline void set_enabled(bool enabled) { m_enabled = enabled; }

	inline const char* name() const { return m_name; }
	inline std::string buffered_string() const { return m_game_buffer.str(); }
	inline std::string_view buffered_view() const { return m_game_buffer.view(); }

private:
	bool try_open_log_file();
	void print_frame();
	
private:
	const char* m_name;
	bool m_enabled;
	s32 m_last_updated_frame = -1; // So logging is possible on frame 0
	std::ofstream m_log_file;
	std::stringstream m_game_buffer;
};

inline GameLogger g_logger{"Game", false};
inline GameLogger g_critical_logger{"Critical", true};
inline GameLogger g_sync_random_logger{"Sync Randoms", false};
inline GameLogger g_local_random_logger{"Local Randoms", false};

inline std::vector<GameLogger*> g_loggers{
	&g_logger,
	&g_critical_logger,
	&g_sync_random_logger,
	&g_local_random_logger
};

}
