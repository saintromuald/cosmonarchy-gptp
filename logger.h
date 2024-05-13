// Basic logging to text file supported by GPTP. Made by pastelmind

#pragma once

#include <fstream>

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
		if (try_open_log_file()) {
			print_frame();
			log_file << t;
		}
		return *this;
	}

	GameLogger& operator<<(std::ostream& (*func)(std::ostream&));
	inline void set_enabled(bool enabled) { this->m_enabled = enabled; }

private:
	bool try_open_log_file();
	void print_frame();
	
private:
	const char* m_name;
	bool m_enabled;
	s32 m_last_updated_frame = -1; // So logging is possible on frame 0
	std::ofstream log_file;
};

inline GameLogger g_logger{"Game", false};
inline GameLogger g_critical_logger{"Critical", true};
inline GameLogger g_sync_random_logger{"Sync Randoms", false};
inline GameLogger g_local_random_logger{"Local Randoms", false};

}
