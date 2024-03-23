// Basic logging to text file supported by GPTP. Made by pastelmind

#pragma once

#include <fstream>

#include "BW/bwdata.h"
#include "config.h"

namespace GPTP {
	class GameLogger {
	public:
		GameLogger(bool critical) : enabled(critical), critical(critical) {}

		void start_game();
		void end_game();

		template <typename T>
		GameLogger& operator<<(const T& t);
		GameLogger& operator<<(std::ostream& (*func)(std::ostream&));
		inline void set_enabled(bool enabled) { this->enabled = enabled; }

	private:
		bool try_open_log_file();
		void print_frame();
		
	private:
		bool enabled;
		bool critical;
		std::ofstream log_file;
		s32 last_updated_frame;
	};

	extern GameLogger logger;
	extern GameLogger critical_logger;

	template <typename T>
	GameLogger& GameLogger::operator<<(const T& t) {
		if (try_open_log_file()) {
			print_frame();
			log_file << t;
		}
		return *this;
	}
}
