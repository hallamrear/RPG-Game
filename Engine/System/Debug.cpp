#include "pch.h"
#include "Debug.h"

#ifdef _DEBUG
#include <debugapi.h>
#include <processenv.h>
#include <WinBase.h>
#include <stdio.h>
#include <consoleapi2.h>
#endif

void Debug::SetDebugPrintColour(const LOG_SEVERITY& severity)
{
#ifdef _DEBUG
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	switch (severity)
	{
	case LOG_SEVERITY::MESSAGE: { SetConsoleTextAttribute(hConsole, 15); }	break;
	case LOG_SEVERITY::WARNING: { SetConsoleTextAttribute(hConsole, 14); }	break;
	case LOG_SEVERITY::SEVERE:	{ SetConsoleTextAttribute(hConsole, 12); }	break;
	case LOG_SEVERITY::FATAL:	{ SetConsoleTextAttribute(hConsole, 12); }	break;
	default:
		break;
	}
#endif
}

void Debug::ResetDebugPrintColour()
{
#ifdef _DEBUG
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 15);
#endif
}

void Debug::Log(const LOG_SEVERITY& severity, const char* message, const va_list& argList)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* new_stdout;
	freopen_s(&new_stdout, "CONOUT$", "w", stdout);

	SetDebugPrintColour(severity);

	char buffer[512];
	vsprintf_s(&buffer[0], 512, message, argList);
	printf(buffer);
	OutputDebugString(buffer);

	if (severity >= LOG_SEVERITY::FATAL)
	{
		MessageBox(NULL, buffer, "Fatal Error", MB_OK);
		assert(false);
	}

	ResetDebugPrintColour();
#endif
}

void Debug::Log(const LOG_SEVERITY& severity, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	Log(severity, message, args);
	va_end(args);
}

void Debug::LogMessage(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	Log(LOG_SEVERITY::MESSAGE, message, args);
	va_end(args);
}

void Debug::LogWarning(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	Log(LOG_SEVERITY::WARNING, message, args);
	va_end(args);
}

void Debug::LogSevere(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	Log(LOG_SEVERITY::SEVERE, message, args);
	va_end(args);
}

void Debug::LogFatal(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	Log(LOG_SEVERITY::FATAL, message, args);
	va_end(args);
}
