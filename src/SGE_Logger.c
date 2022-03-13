#include "SGE_Logger.h"
#include "SGE.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* The output stream to use for log messages */
static FILE *logStream = NULL;

/* Log level filter flag that decides what messages are printed */
static int logFilterFlag =  SGE_LOG_CLEAN;

void SGE_LogSetFilter(int flags)
{
	logFilterFlag = flags;
}

/* Tests whether a log message should be ignored based on it's level and the current filter flag. */
static bool SGE_LogShouldIgnore(SGE_LogLevel level)
{
	if(logFilterFlag & level)
	{
		return false;
	}
	return true;
}

void SGE_LogSetFile(const char *filePath)
{
	time_t currentTime;
	char *timeStr;
	
	SGE_LogCloseFile();
	
	if(filePath == NULL)
	{
		logStream = stdout;
		return;
	}
	
	logStream = fopen(filePath, "a+");
	if(logStream == NULL)
	{
		logStream = stdout;
		printf("LOGGER: Failed to open log file, using stdout!\n");
	}
	else
	{
		currentTime = time(NULL);
		timeStr = ctime(&currentTime);
		fprintf(logStream, "LOGGER: LOG_BEGIN\nLOGGER: DATE: ");
		fprintf(logStream, timeStr);
	}
}

void SGE_LogCloseFile()
{
	/* logStream can either be NULL, stdout or a file pointer. */
	if(logStream != NULL)
	{
		if(logStream != stdout) // logStream is a file.
		{
			fprintf(logStream, "LOGGER: LOG_END\n\n");
			fclose(logStream);
			logStream = NULL;
		}
	}
}

static void SGE_LogPrintTags(SGE_LogLevel level)
{
	switch(level)
	{
		case SGE_LOG_INFO:
		fprintf(logStream, "[ INFO ] ");
		break;

		case SGE_LOG_DEBUG:
		fprintf(logStream, "[ DEBUG ] ");
		break;
		
		case SGE_LOG_ERROR:
		fprintf(logStream, "[ ERROR ] ");
		break;
		
		case SGE_LOG_WARNING:
		fprintf(logStream, "[WARNING] ");
		break;
		
		default:
		fprintf(logStream, "[LOGGER]: Invalid log level passed!\n[INVALID] ");
		break;
	}
	
	fprintf(logStream, "[");
	fprintf(logStream, SGE_GetCurrentSceneName());
	fprintf(logStream, "]: ");
}

void SGE_LogPrintLine(SGE_LogLevel level, const char *format, ...)
{
	/* Filter message */
	if(SGE_LogShouldIgnore(level))
	{
		return;
	}

	va_list args;
	va_start(args, format);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	SGE_LogPrintTags(level);
	
	vfprintf(logStream, format, args);
	fprintf(logStream, "\n");
	va_end(args);
}

void SGE_LogPrint(SGE_LogLevel level, const char *format, ...)
{
	if(SGE_LogShouldIgnore(level))
	{
		return;
	}

	va_list args;
	va_start(args, format);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	SGE_LogPrintTags(level);
	
	vfprintf(logStream, format, args);
	va_end(args);
}

void SGE_LogPrintLineCat(SGE_LogLevel level, const char *catString, const char *format, va_list *args)
{
	if(SGE_LogShouldIgnore(level))
	{
		return;
	}

	va_list args2;
	va_copy(args2, *args);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	SGE_LogPrintTags(level);
	
	fprintf(logStream, catString);
	
	vfprintf(logStream, format, args2);
	fprintf(logStream, "\n");
	va_end(args2);
}

void SGE_printf(SGE_LogLevel level, const char *format, ...)
{
	if(SGE_LogShouldIgnore(level))
	{
		return;
	}

	va_list args;
	va_start(args, format);
	vfprintf(logStream, format, args);
	va_end(args);
}