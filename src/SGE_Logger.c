#include "SGE_Logger.h"
#include "SGE.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* The output stream to use for log messages */
static FILE *logStream = NULL;

/*
	Parameters:
	1. const char* filePath
	The path to a log file, NULL for stdout.
	
	Description:
	Sets the logStream to either be a file, or stdout based on the filePath parameter.
*/
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
		fprintf(logStream, "\nLOGGER: LOG_BEGIN\nLOGGER: DATE: ");
		fprintf(logStream, timeStr);
	}
}

/*
	Paramters:
	None
	
	Description:
	Closes the log file opened with logSetFile().
	Must be called before exit if the last used stream wasn't stdout.
	Acts the same as logSetFile(NULL);
*/
void SGE_LogCloseFile()
{
	// logStream can either be NULL, stdout or a file pointer.
	if(logStream != NULL)
	{
		if(logStream != stdout) // logStream is a file.
		{
			fprintf(logStream, "LOGGER: LOG_END\n");
			fclose(logStream);
			logStream = NULL;
		}
	}
}

/*
	Parameters:
	1. LogLevel level
	The log level of the message string to print.
	2. const char *format 
	The format string message to print similar to printf().
	3. ...
	Comma seperated args based on the specified format string.
	
	Description:
	Prints a formatted string to the output stream along with
	a preceding string that specifies the log level of the message.
	Essentially a wrapper around fprintf().
*/
void SGE_LogPrintLine(SGE_LogLevel level, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	switch(level)
	{
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
		fprintf(logStream, "LOGGER: Invalid log level passed!\n[INVALID]: ");
		break;
	}
	
	fprintf(logStream, "[");
	fprintf(logStream, SGE_GetCurrentState()->name);
	fprintf(logStream, "]: ");
	
	vfprintf(logStream, format, args);
	fprintf(logStream, "\n");
	va_end(args);
}

void SGE_LogPrint(SGE_LogLevel level, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	switch(level)
	{
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
		fprintf(logStream, "LOGGER: Invalid log level passed!\n[INVALID]: ");
		break;
	}
	
	fprintf(logStream, "[");
	fprintf(logStream, SGE_GetCurrentState()->name);
	fprintf(logStream, "]: ");
	
	vfprintf(logStream, format, args);
	va_end(args);
}

void SGE_LogPrintLineCat(SGE_LogLevel level, const char *catString, const char *format, va_list *args)
{
	va_list args2;
	va_copy(args2, *args);
	
	// If logInit() hasn't been called, set logStream to stdout
	if(logStream == NULL)
	{
		logStream = stdout;
	}
	
	switch(level)
	{
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
		fprintf(logStream, "LOGGER: Invalid log level passed!\n[INVALID]: ");
		break;
	}
	
	fprintf(logStream, "[");
	fprintf(logStream, SGE_GetCurrentState()->name);
	fprintf(logStream, "]: ");
	
	fprintf(logStream, catString);
	
	vfprintf(logStream, format, args2);
	fprintf(logStream, "\n");
	va_end(args2);
}

void SGE_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(logStream, format, args);
	va_end(args);
}