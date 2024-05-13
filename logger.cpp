#define _CRT_SECURE_NO_WARNINGS

#include "logger.h"

#include <time.h>

namespace gptp {

void GameLogger::log_game_start() {
	if (try_open_log_file()) {
		time_t current_time = 0;
		time(&current_time);
		log_file << "Game started on " << ctime(&current_time) << std::endl;
	}
}

void GameLogger::log_game_end() {
	if (try_open_log_file()) {
		time_t current_time = 0;
		time(&current_time);
		log_file << "Game ended on " << ctime(&current_time) << std::endl;
	}
	log_end();
}

void GameLogger::log_end() {
	log_file.close();
}

GameLogger& GameLogger::operator<<(std::ostream& (*func)(std::ostream&)) {
	if (try_open_log_file()) {
		print_frame();
		log_file << func;
	}
	return *this;
}

bool GameLogger::try_open_log_file() {
	if (!m_enabled) {
		return false;
	}
	if (!log_file.is_open()) {
		std::error_code ec;
		fs::create_directory("logs", ec);

		std::stringstream ss;
		time_t current_time = time(nullptr);
		ss << "logs/" << m_name << " " << std::put_time(localtime(&current_time), "%Y-%m-%d %Hh %Mm %Ss") << ".log";
		log_file.open(ss.str());
	}
	return log_file.is_open();
}

void GameLogger::print_frame() {
	if (!m_enabled) {
		return;
	}

	const u32 current_frame = *g_elapsed_time_frames;
	if (current_frame > m_last_updated_frame) {
		m_last_updated_frame = current_frame;
		log_file << "Frame " << current_frame << ":\n";
	}
}

}
