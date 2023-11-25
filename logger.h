// Basic logging to text file supported by GPTP. Made by pastelmind

#pragma once

#include "types.h"
#include <fstream>

namespace GPTP
{
	struct GameLogger
	{
		// Writes a "game start" message to the log file and resets the internal frame timer.
		bool start_game();
		// Writes a "game end" message to the log file.
		bool end_game();
		template <typename T>
		GameLogger& operator<<(const T& t);
		GameLogger& operator<<(std::ostream& (*func)(std::ostream&));
		bool check_log_file();
		bool update_frame();
		std::ofstream log_file;
		s32 last_updated_frame;
	};
	extern bool logging_enabled;
	extern GameLogger logger;
}

namespace GPTP
{
	template <typename T>
	GameLogger& GameLogger::operator<<(const T& t)
	{
		if (logging_enabled)
		{
			if (check_log_file())
			{
				if (update_frame())
				{
					log_file << t;
				}
			}
		}
		return *this;
	}
}
