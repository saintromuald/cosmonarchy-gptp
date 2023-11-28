#define _CRT_SECURE_NO_WARNINGS

#include "logger.h"

#include <time.h>

namespace GPTP {
	GameLogger logger(false);
	GameLogger critical_logger(true);

	void GameLogger::start_game() {
		if (!critical) {
			if (try_open_log_file()) {
				time_t current_time = 0;
				time(&current_time);
				log_file << "Game started on " << ctime(&current_time) << std::endl;
			}
			last_updated_frame = -1;  // So logging is possible on frame 0
		}
	}

	void GameLogger::end_game() {
		if (!critical) {
			if (try_open_log_file()) {
				time_t current_time = 0;
				time(&current_time);
				log_file << "Game ended on " << ctime(&current_time) << std::endl;
			}
		}
	}

	GameLogger& GameLogger::operator<<(std::ostream& (*func)(std::ostream&)) {
		if (try_open_log_file()) {
			print_frame();
			log_file << func;
		}
		return *this;
	}

	bool GameLogger::try_open_log_file() {
		if (!enabled) {
			return false;
		}
		if (log_file.is_open()) {
			return true;
		}

		time_t current_time = 0;
		time(&current_time);
		char buffer[100] = {};
		if (critical) {
			strftime(buffer, sizeof(buffer), "Critical %Y-%m-%d %Hh %Mm %Ss.log", localtime(&current_time));
		} else {
			strftime(buffer, sizeof(buffer), "Game %Y-%m-%d %Hh %Mm %Ss.log", localtime(&current_time));
		}

		log_file.open(buffer);
		return log_file.is_open();
	}

	void GameLogger::print_frame() {
		if (enabled) {
			const u32 current_frame = *g_elapsed_time_frames;
			if (current_frame > last_updated_frame) {
				last_updated_frame = current_frame;
				log_file << "Frame " << current_frame << ":\n";
			}
		}
	}
}
