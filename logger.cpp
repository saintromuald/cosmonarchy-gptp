#define _CRT_SECURE_NO_WARNINGS

#include "logger.h"
#include <ctime>
#include <SCBW/scbwdata.h>

namespace GPTP
{
	GameLogger logger;
	bool logging_enabled = false; // Now handled by 'verbose' config option in config.yml

	bool GameLogger::start_game()
	{
		if (logging_enabled)
		{
			if (check_log_file())
			{
				time_t current_time;
				time(&current_time);
				log_file << "Game started on " << ctime(&current_time) << std::endl;
			}
			last_updated_frame = -1;  // So logging is possible on frame 0
		}
		return true;
	}

	bool GameLogger::end_game()
	{
		if (logging_enabled)
		{
			if (check_log_file())
			{
				time_t current_time;
				time(&current_time);
				log_file << "Game ended on " << ctime(&current_time) << std::endl;
			}
		}
		return true;
	}

	GameLogger& GameLogger::operator<<(std::ostream& (*func)(std::ostream&))
	{
		if (logging_enabled)
		{
			if (check_log_file())
			{
				if (update_frame())
				{
					log_file << func;
				}
			}
		}
		return *this;
	}

	bool GameLogger::check_log_file()
	{
		if (logging_enabled)
		{
			if (log_file.is_open())
			{
				return true;
			}
			time_t current_time;
			time(&current_time);

			char buffer[100];
			strftime(buffer, sizeof(buffer), "Game %Y-%m-%d %Hh %Mm %Ss.log", localtime(&current_time));

			log_file.open(buffer);
			return !log_file.fail();
		} else {
			return true;
		}
	}

	bool GameLogger::update_frame()
	{
		if (logging_enabled)
		{
			const s32 current_frame = *g_elapsed_time_frames;
			if (current_frame > last_updated_frame)
			{
				last_updated_frame = current_frame;
				log_file << "Frame " << current_frame << ":\n";
			}
		}
		return true;
	}

}
