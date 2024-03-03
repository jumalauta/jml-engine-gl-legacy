#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "system/debug/debug.h"
#include "system/datatypes/memory.h"
#include "system/datatypes/datatypes.h"
#include "io.h"

#define PATH_SIZE 2048
static char startPath[PATH_SIZE] = {'\0'};
const char *getStartPath()
{
	return (const char*)startPath;
}

void setStartPath(const char *executablePath)
{
	startPath[0] = '\0';

	char *pathWithoutExecutable = strrchr(executablePath,'/');
	if (pathWithoutExecutable)
	{
		unsigned int startPathLength = strlen(executablePath)-strlen(pathWithoutExecutable)+2;
		assert(startPathLength < PATH_SIZE);

		//remove file name, example: "/usr/local/executable_name" => "/usr/local/"
		snprintf(startPath, startPathLength, "%s", executablePath);
	}

	debugPrintf("Start path: '%s'", startPath);
}

int fileExists(const char *filename)
{
	struct stat buffer;   
	return (filename && stat(filename, &buffer) == 0);
}

int fileModified(const char *filename, time_t *fileLastModifiedTime)
{
	struct stat buffer;   
	if (stat(filename, &buffer) == 0)
	{
		int fileModified = 0;
		if (*fileLastModifiedTime < buffer.st_mtime)
		{
			fileModified = 1;
		}
		
		*fileLastModifiedTime = buffer.st_mtime;
		return fileModified;
	}
	
	*fileLastModifiedTime = 0;
	return -1;
}

const char* getFilePath(const char *filename)
{
	if (fileExists(filename))
	{
		return filename;
	}

	//Windows hooligans away!
	if (strchr((const char*)filename, '\\') != NULL)
	{
		debugErrorPrintf("File path '%s' must not contains backslashes!", filename);
		assert(strchr((const char*)filename, '\\') == NULL);
	}
	
	char *file = (char*)memoryAllocateGeneral(NULL, PATH_SIZE*sizeof(char), NULL);
	
	snprintf(file, PATH_SIZE, "%s", filename);
	if (strchr((const char*)file, '/') == NULL)
	{
		snprintf(file, PATH_SIZE, "data/%s", filename);
	}
	char fullPath[PATH_SIZE];
	snprintf(fullPath, PATH_SIZE, "%s%s", startPath, file);

	if (!fileExists((const char*)fullPath))
	{
		//attempt to check if lower case version exists
		stringToLower(file);

		//This is a hack for searching PNG file name instead of the name described in 3DS file
		if (endsWithIgnoreCase(file, ".jpg") || endsWithIgnoreCase(file, ".bmp"))
		{
			debugWarningPrintf("Image file type not supported for '%s'. Searching for PNG instead.",(const char*)file);

			file[strlen((const char*)file)-4] = '\0';
			strncat(file, ".png", PATH_SIZE-strlen(file)-1);
		}
	}

	snprintf(fullPath, PATH_SIZE, "%s%s", startPath, file);
	snprintf(file, PATH_SIZE, "%s", fullPath);
	if (!fileExists((const char*)file))
	{
		debugWarningPrintf("Couldn't find file '%s'!", filename);
	}
	
	file = (char*)memoryAllocateGeneral((void*)file, sizeof(char)*(strlen(file)+1), NULL);
	
	return (const char*)file;
}

char *ioReadFileToBuffer(const char *file, unsigned int *count)
{
	const char *filePath = getFilePath(file);
	char *content = NULL;
	
	int f;
	f = open(filePath, O_RDONLY);
	*count = lseek(f, 0, SEEK_END);
	close(f);
	
	if (file != NULL)
	{
		FILE *fp = fopen(filePath,"rt");
		
		if (fp != NULL)
		{
			if (*count > 0)
			{
				content = (char *)malloc(sizeof(char) * (*count+1));
				*count = fread(content,sizeof(char),*count,fp);
				content[*count] = '\0';
			}
			fclose(fp);
		}
	}

	return content;
}

char* strtok_reentrant(char *str, const char *delim, char **nextp)
{
	char *ret;

	if (str == NULL)
	{
		str = *nextp;
	}

	str += strspn(str, delim);

	if (*str == '\0')
	{
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str)
	{
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}
