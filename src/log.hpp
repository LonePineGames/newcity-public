#pragma once

#include <stdint.h>
#include <string>

const uint16_t MAX_NUM_LOGS = 8;
const std::string LOG_FILENAME = "game_log";
const std::string LOG_EXT = ".log";

void initLogging();
void sendLogFile();
void removeSentinelFile();

