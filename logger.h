/// Basic logging to text file supported by GPTP.
/// Note: If you build the plugin in Release mode, all logging will be disabled.
/// Made by pastelmind

#pragma once

#include <fstream>

namespace GPTP {

class GameLogger {
  public:
    /// Writes a "game start" message to the log file and resets the internal
    /// frame timer.
    bool startGame();

    /// Writes a "game end" message to the log file.
    bool endGame();
    
    template <typename T>
    GameLogger& operator<<(const T& t);
    GameLogger& operator<<(std::ostream& (*func)(std::ostream&));

  private:
    bool checkLogFile();
    bool updateFrame();
    std::ofstream logFile;
    int lastUpdatedFrame;
};

extern bool doLogging;
extern GameLogger logger;

} //GPTP


//-------- Template member function definitions --------//

namespace GPTP {

template <typename T>
GameLogger& GameLogger::operator<<(const T& t) {

    if (doLogging) {
        if (checkLogFile())
            if (updateFrame())
                logFile << t;
    }

    return *this;
}

} //GPTP

