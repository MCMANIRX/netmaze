#pragma once
#include "Logger.h"
#include <iostream>
#include <cstdarg>    // For variadic arguments
#include <cstdio>     // For snprintf
#define DECAY_THRESH 5
int counts;
Logger::Logger() : yPos(0) {
     counts= 0;


    // Initialize yPos and other necessary members
}
void Logger::update() {

        if(log.size()<1)
            return;
    
    
        counts++;
        if(counts==DECAY_THRESH){
            log.erase(log.begin());
            counts = 0;
        }

}

Logger& Logger::getInstance() {
    static Logger instance;  // Singleton instance
    return instance;
}

void Logger::setFont(GRRLIB_texImg *_font) {
    font = _font;
}


void Logger::logf(const char* format, ...) {
    char buffer[1024];  // Buffer for formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);  // Format the string
    va_end(args);

    log.emplace_back(buffer);  // Store the formatted string in the log
    counts = 0;
}

void Logger::printLog() {
    yPos=0;
    for (const auto& entry : log) {
        GRRLIB_Printf   ( 32, 16+32*yPos++, font, 0xffffffff, 1,entry.c_str());
    }
    //clearLog();
}
void Logger::clearLog() {
    log.clear();   // Clear all entries from the log
}

