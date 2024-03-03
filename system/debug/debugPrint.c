#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "system/timer/timer.h"
#include "system/thread/thread.h"

#include "debug.h"

static int isDebugOutputToWindow = 0;
static int debug = 0;

#define MAX_ERROR_LOG 2048
#define LOG_HEADER_SIZE 256
#define TIME_STRING_SIZE 16

static char lastErrorLine[MAX_ERROR_LOG] = {'\0'};
static char screenLog[MAX_ERROR_LOG] = {'\0'};
static char screenLogTemp[MAX_ERROR_LOG] = {'\0'};

//#define file_out stdout
#define file_normal_out stdout
#define file_error_out stderr

#define debugNote file_normal_out
#define debugWarning file_normal_out
#define debugError file_normal_out
#define debugOut file_normal_out

int isDebug()
{
	return debug;
}

void setDebug(int _debug)
{
	debug = _debug;
}

void clearScreenLog()
{
	screenLog[0] = '\0';
	lastErrorLine[0] = '\0';
}

char *getScreenLog()
{
	return screenLog;
}

static void appendToScreenLog(const char *string)
{
	strncat(screenLog, string, MAX_ERROR_LOG-strlen(screenLog)-strlen(string)-1);
}

void __debugPrintf(const char *fileName, const char *functionName, int sourceLine, int style, const char *fmt, ...)
{
	if (!isDebug() && style < DEBUG_STYLE_ERROR)
	{
		return;
	}

	int size = 128;
	char *p = NULL, *np = NULL;
	va_list ap;
	
	if ((p = malloc(size)) == NULL)
	{
		return;
	}

	char __debugPrintf_time_str[TIME_STRING_SIZE];
	timerSetTimeString(__debugPrintf_time_str);
	char logHeaderString[LOG_HEADER_SIZE];
	snprintf(logHeaderString, LOG_HEADER_SIZE, "[%s] %X %s:%s():%d:", __debugPrintf_time_str, threadGetCurrentId(), fileName, functionName, sourceLine);

	while (1)
	{
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		int n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
		{
			if (isDebugOutputToWindow)
			{
				/*window output routine*/
			}

			//Prevent threads from writing log simultaneously
//			threadGlobalMutexLock();

			switch(style)
			{
				case DEBUG_STYLE_SCREEN_LOG: //Screen log
					screenLogTemp[0] = '\0';
					snprintf(screenLogTemp, MAX_ERROR_LOG, "[%s]:\n%s\n", __debugPrintf_time_str, p);

					clearScreenLog();
					appendToScreenLog(screenLogTemp);
					break;

				case DEBUG_STYLE_ERROR: //ERROR
					fprintf(debugError, "%s\nERROR: %s\n", logHeaderString, p);
					
					if (strcmp(lastErrorLine, p))
					{
						snprintf(lastErrorLine, MAX_ERROR_LOG, "%s", p);
						
						screenLogTemp[0] = '\0';
						snprintf(screenLogTemp, MAX_ERROR_LOG, "%s\nERROR: %s\n", logHeaderString, p);
						appendToScreenLog(screenLogTemp);
					}

					fflush(debugError);
					break;
					
				case DEBUG_STYLE_WARNING:
					fprintf(debugWarning, "%s\nWARNING: %s\n", logHeaderString, p);
					fflush(debugWarning);
					break;
					
				case DEBUG_STYLE_INFO:
					fprintf(debugNote, "%s\nINFO: %s\n", logHeaderString, p);
					fflush(debugNote);
					break;
					
				case DEBUG_STYLE_DEBUG:
					fprintf(debugOut, "%s\n%s\n", logHeaderString, p);
					fflush(debugOut);
					break;
					
				default:
					break;
			}

//			threadGlobalMutexUnlock();

			free(p);
			return;
		}
		/* Else try again with more space. */
		if (n > -1) /* glibc 2.1 */
		{
			size = n+1; /* precisely what is needed */
		}
		else /* glibc 2.0 */
		{
			size *= 2;  /* twice the old size */
		}
		if ((np = realloc (p, size)) == NULL) {
			free(p);
			p = NULL;
			return;
		} else {
			p = np;
		}
	}
}
