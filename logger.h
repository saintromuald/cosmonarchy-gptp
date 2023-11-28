// Basic logging to text file supported by GPTP. Made by pastelmind

#pragma once

#include <fstream>

#include "SCBW/scbwdata.h"
#include "config.h"

namespace GPTP {
	struct GameLogger {
		GameLogger(bool critical) : enabled(critical), critical(critical) {}

		// Writes a "game start" message to the log file and resets the internal frame timer.
		void start_game();

		// Writes a "game end" message to the log file.
		void end_game();

		template <typename T>
		GameLogger& operator<<(const T& t);
		GameLogger& operator<<(std::ostream& (*func)(std::ostream&));
		bool try_open_log_file();
		void print_frame();
		
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
