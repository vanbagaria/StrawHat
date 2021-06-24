#ifndef __SGE_LOGGER_H__
#define __SGE_LOGGER_H__

#include <stddef.h>
#include <stdarg.h>

/**
 *  The severity level of a log message.
 */
typedef enum SGE_LogLevel
{
	SGE_LOG_INFO    =  1,   // Important information
	SGE_LOG_DEBUG   =  2,   // General debug information
	SGE_LOG_WARNING =  4,   // Program continues with warning
	SGE_LOG_ERROR   =  8,   // Program exits with error
	
	SGE_LOG_CLEAN   = 13,   // Everything but LOG_DEBUG
	SGE_LOG_ALL     = 15    // Show all log output
} SGE_LogLevel;

/**
 *  \brief Sets the filtering flag for log printing functions
 *  \param flags Bitwise OR 'ed SGE_LogLevel flags
 */
void SGE_LogSetFilter(int flags);

/**
 *  \brief Sets the log stream to either be a file, or stdout based on the filePath parameter.
 *  \param filePath The path to a log file, NULL for stdout.
 */
void SGE_LogSetFile(const char *filePath);

/**
 *  \brief Closes the log file opened with SGE_LogSetFile().
 *  \note Must be called before exit if the last used stream wasn't stdout.
 *  Acts the same as SGE_LogSetFile(NULL);
 */
void SGE_LogCloseFile();

/**
 *  \brief Prints a formatted string to the log stream along with
 *  a preceding tag that specifies the log level of the message.
 * 
 *  \param level
 *  The log level of the message string to print.
 *  \param format
 *  The message string to print with format specifiers similar to printf().
 *  \param ...
 *  Comma seperated arguments based on the specified format string.
*/
void SGE_LogPrintLine(SGE_LogLevel level, const char *text, ...);

/**
 *  \brief Same as SGE_LogPrintLine(), but does not add a newline after the message.
 */
void SGE_LogPrint(SGE_LogLevel level, const char *format, ...);

/**
 *  \brief Allows creating a wrapper around SGE_LogPrintLine() that adds a custom string after the tags.
 */
void SGE_LogPrintLineCat(SGE_LogLevel level, const char *catString, const char *format, va_list *args);

/**
 *  \brief Wrapper for standard printf() that does not add the log level tag.
 *  \note Use this instead of printf() so the output goes to the right stream.
 */
void SGE_printf(SGE_LogLevel level, const char *format, ...);

#endif