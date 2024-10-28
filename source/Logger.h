#ifndef LOGGER_H
#define LOGGER_H

#include <vector>
#include <string>
#include <grrlib.h>
#include <gccore.h>

class Logger {
private:
    std::vector<std::string> log;  // Vector to store logged strings
    int yPos;                      // Tracks the y-position for each log
    GRRLIB_texImg *font;

    Logger();  // Private constructor for singleton pattern

public:
    static Logger& getInstance();  // Returns the singleton instance of Logger
    void setFont(GRRLIB_texImg *_font);

    void logf(const char* format, ...);  // Function to log formatted string
    void printLog();  // Function to print all logs with incrementing yPos
    void clearLog();
    void update();
};

#endif  // LOGGER_H
