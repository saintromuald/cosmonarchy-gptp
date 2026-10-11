#define _CRT_SECURE_NO_WARNINGS

#include "logger.h"

#include <time.h>

namespace gptp {

void GameLogger::log_game_start() {
    m_game_buffer.str({});
    m_game_buffer.clear();
    m_last_updated_frame = -1;
    time_t current_time = 0;
    time(&current_time);
	if (try_open_log_file()) {
		m_log_file << "Game started on " << ctime(&current_time) << std::endl;
	}
    m_game_buffer << "Game started on " << ctime(&current_time) << std::endl;
}

void GameLogger::log_game_end() {
    time_t current_time = 0;
    time(&current_time);
	if (try_open_log_file()) {
		m_log_file << "Game ended on " << ctime(&current_time) << std::endl;
	}
    m_game_buffer << "Game ended on " << ctime(&current_time) << std::endl;
	log_end();
}

void GameLogger::log_end() {
	m_log_file.close();
}

GameLogger& GameLogger::operator<<(std::ostream& (*func)(std::ostream&)) {
    print_frame();
	if (try_open_log_file()) {
		m_log_file << func;
	}
    m_game_buffer << func;
	return *this;
}

bool GameLogger::try_open_log_file() {
	if (!m_enabled) {
		return false;
	}
	if (!m_log_file.is_open()) {
		std::error_code ec;
		fs::create_directory("logs", ec);

		std::stringstream ss;
		time_t current_time = time(nullptr);
		ss << "logs/" << m_name << " " << std::put_time(localtime(&current_time), "%Y-%m-%d %Hh %Mm %Ss") << ".log";
		m_log_file.open(ss.str());
	}
	return m_log_file.is_open();
}

void GameLogger::print_frame() {
    const u32 current_frame = g_elapsed_time_frames;
    m_game_buffer << "Frame " << current_frame << ":\n";
	if (!m_enabled) {
		return;
	}

	if (m_log_file.is_open()) {
        if (current_frame > m_last_updated_frame) {
            m_last_updated_frame = current_frame;
            m_log_file << "Frame " << current_frame << ":\n";
        }
	}
}

}
