#pragma once

enum LOG_SEVERITY : int
{
	MESSAGE = 0,
	WARNING = 1,
	SEVERE = 2,
	FATAL = 3
};

#ifdef _DEBUG
#include <assert.h>
#include <stdarg.h>
#define CUSTOM_ASSERT(expression) if(!(expression)) { Debug::Log(LOG_SEVERITY::FATAL, "Condition: %s\n Value: %i\n File: %s\n Line: %i\n", #expression, (int)expression, __FILE__, __LINE__); assert(expression); }
#else
#define CUSTOM_ASSERT(expression)
#endif

class Debug
{
private:
	static void SetDebugPrintColour(const LOG_SEVERITY& severity);
	static void ResetDebugPrintColour();

	static void Log(const LOG_SEVERITY& severity, const char* message, const va_list& argList);

public:
	static void Log(const LOG_SEVERITY& severity, const char* message, ...);
	static void LogMessage(const char* message, ...);
	static void LogWarning(const char* message, ...);
	static void LogSevere(const char* message, ...);
	static void LogFatal(const char* message, ...);
};

