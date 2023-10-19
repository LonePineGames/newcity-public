#pragma once

void parseBinaryMap();
void initErrorHandling();
void handleSignal(int sig);
void handleError(const char* format, ...);
void clearErrorState();
void logStacktrace();
bool anyErrorDetected();

