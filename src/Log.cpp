#include "Log.h"

#include <Windows.h>

#include <stdio.h>

namespace Log
{

const int kBufferSize = 4096;

Severity gMinSeverity = Severity_Message;

static void Output(Severity severity, const char* text)
{

    const char* kSeverityTags[] = { "[DEBUG] ", NULL, "[ERROR] " };
    const char* tag = kSeverityTags[severity];

    if (tag != NULL)
    {
        OutputDebugStringA(tag);
    }

    OutputDebugStringA(text);
    OutputDebugStringA("\n");
    
}

void Initialize(Severity minSeverity)
{
    gMinSeverity = minSeverity;
}

void Shutdown()
{
    
}

void Log(Severity severity, const char* format, ...)
{
    
    va_list argList;

    va_start(argList, format);    
    Log(severity, format, argList);
    va_end(argList);

}

void Log(Severity severity, const char* format, va_list argList)
{
    
    if (severity < gMinSeverity)
    {
        return;
    }

    char buffer[kBufferSize];
    vsnprintf(buffer, kBufferSize, format, argList);    
    Output(severity, buffer);

}

}

void LogDebug(const char* format, ...)
{

    va_list argList;

    va_start(argList, format);
    Log::Log(Log::Severity_Debug, format, argList);
    va_end(argList);

}

void LogMessage(const char* format, ...)
{

    va_list argList;

    va_start(argList, format);
    Log::Log(Log::Severity_Message, format, argList);
    va_end(argList);

}

void LogError(const char* format, ...)
{

    va_list argList;

    va_start(argList, format);
    Log::Log(Log::Severity_Error, format, argList);
    va_end(argList);

}
