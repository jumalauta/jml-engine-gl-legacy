#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "system/debug/debug.h"

#include "string.h"

int startsWith(const char *string, const char *prefix)
{
	if (!string || !prefix)
	{
        return 0;
	}

    size_t stringLength = strlen(string);
    size_t prefixLength = strlen(prefix);

    if (prefixLength > stringLength)
	{
        return 0;
	}

    if (strncmp(string, prefix, prefixLength) == 0)
	{
		return 1;
	}
	
	return 0;
}

int startsWithIgnoreCase(const char *string, const char *prefix)
{
	if (!string || !prefix)
	{
        return 0;
	}

    size_t stringLength = strlen(string);
    size_t prefixLength = strlen(prefix);

    if (prefixLength > stringLength)
	{
        return 0;
	}

    if (strncasecmp(string, prefix, prefixLength) == 0)
	{
		return 1;
	}
	
	return 0;
}

int endsWith(const char *string, const char *suffix)
{
    if (!string || !suffix)
	{
        return 0;
	}

    size_t stringLength = strlen(string);
    size_t suffixLength = strlen(suffix);

    if (suffixLength > stringLength)
	{
        return 0;
	}

    if (strncmp(string + stringLength - suffixLength, suffix, suffixLength) == 0)
	{
		return 1;
	}
	
	return 0;
}

int endsWithIgnoreCase(const char *string, const char *suffix)
{
    if (!string || !suffix)
	{
        return 0;
	}

	size_t stringLength = strlen(string);
	size_t suffixLength = strlen(suffix);

    if (suffixLength > stringLength)
	{
        return 0;
	}

    if (strncasecmp(string + stringLength - suffixLength, suffix, suffixLength) == 0)
	{
		return 1;
	}

	return 0;
}

void stringToLower(char *string)
{
	size_t i;
	for(i = 0; i < strlen((const char*)string); i++)
	{
		string[i] = tolower(string[i]);
	}
}
