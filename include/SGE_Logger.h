#ifndef __SGE_LOGGER_H__
#define __SGE_LOGGER_H__

#include <stddef.h>

typedef enum
{
	SGE_LOG_DEBUG,   // General debug information
	SGE_LOG_WARNING, // Program continues with warning 
	SGE_LOG_ERROR    // Program exits with error
} SGE_LogLevel;

void SGE_LogSetFile(const char *filePath);
void SGE_LogCloseFile();
void SGE_LogPrintLine(SGE_LogLevel level, const char *text, ...);
void SGE_LogPrint(SGE_LogLevel level, const char *format, ...);
void SGE_LogPrintLineCat(SGE_LogLevel level, const char *catString, const char *format, va_list *args);
void SGE_printf(const char *format, ...);

#endif