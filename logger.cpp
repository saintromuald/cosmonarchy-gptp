#define _CRT_SECURE_NO_WARNINGS

#include "logger.h"
#include <ctime>
#include <SCBW/scbwdata.h>

namespace GPTP {

	GameLogger logger;
	bool doLogging = false;

	bool GameLogger::startGame() {
		if (doLogging) {

			if (checkLogFile()) {
				time_t currentTime;
				time(&currentTime);

				logFile << "Game started on " << ctime(&currentTime) << std::endl;
			}

			lastUpdatedFrame = -1;  //So logging is possible on frame 0

		}

		return true;
	}

	bool GameLogger::endGame() {
		if (doLogging) {
			if (checkLogFile()) {
				time_t currentTime;
				time(&currentTime);
				logFile << "Game ended on " << ctime(&currentTime) << std::endl;
			}
		}

		return true;
	}

	GameLogger& GameLogger::operator<<(std::ostream& (*func)(std::ostream&)) {
		if (doLogging) {
			if (checkLogFile())
				if (updateFrame())
					logFile << func;
		}

		return *this;
	}

	bool GameLogger::checkLogFile() {
		if (doLogging) {
			if (logFile.is_open())
				return true;

			time_t currentTime;
			time(&currentTime);

			char buffer[100];
			strftime(buffer, sizeof(buffer), "Game %Y-%m-%d %Hh %Mm %Ss.log",
				localtime(&currentTime));

			logFile.open(buffer);
			return !logFile.fail();

		} else {
			return true;
		}
	}

	bool GameLogger::updateFrame() {
		if (doLogging) {

			const int currentFrame = *elapsedTimeFrames;

			if (currentFrame > lastUpdatedFrame) {
				lastUpdatedFrame = currentFrame;
				logFile << "Frame " << currentFrame << ":\n";
			}

		}

		return true;
	}

} //GPTP
