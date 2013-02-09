#ifndef GAME_LOG_H
#define GAME_LOG_H

#include <stdarg.h>

namespace Log
{
    enum Severity
    {
        Severity_Debug,
        Severity_Message,
        Severity_Error
    };

    void Initialize(Severity minSeverity=Severity_Message);
    void Shutdown();

    void Log(Severity severity, const char* format, ...);
    void Log(Severity severity, const char* format, va_list argList);
}

void LogDebug(const char* format, ...);
void LogMessage(const char* format, ...);
void LogError(const char* format, ...);

#endif
