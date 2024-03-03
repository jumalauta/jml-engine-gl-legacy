#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "graphicsIncludes.h"

#include "system/debug/debug.h"
#include "system/player/player.h"

#include "thread.h"

typedef struct {
#ifdef __X11__
	Display *display;
	GLXContext context;
	GLXDrawable drawable;
	XVisualInfo* visualInfo;
#elif WINDOWS
	HDC hdc;
	HGLRC context;
#endif
	int ok;
} openGlContext_t;

static openGlContext_t mainContext;

static SDL_mutex* globalMutex = NULL;
static SDL_sem* threadSemaphore = NULL;
static unsigned int maxThreads = 0;

typedef struct thread_t thread_t;

struct thread_t {
	void (*function)(void*);
	void *data;
	struct thread_t *next;
};

typedef struct {
	thread_t *head;
	thread_t *tail;
	int active;
	SDL_Thread *sdlThread;
	SDL_mutex* mutex;
	openGlContext_t context;
} threadQueue_t;

static threadQueue_t* queues = NULL;
static unsigned int queueBalancer = 0;

static void queueAddThread(thread_t *thread)
{
	assert(thread);
	assert(queues);

	//dumb queue even load balancing implementation
	queueBalancer++;
	if (queueBalancer >= maxThreads)
	{
		queueBalancer = 0;
	}


	threadQueue_t *queue = &queues[queueBalancer];

	//SDL_LockMutex(queue->mutex);

	if (queue->head == NULL)
	{
		queue->head = thread;
	}
	else
	{
		queue->tail->next = (struct thread_t*)thread;
	}

	queue->tail = thread;

	//SDL_UnlockMutex(queue->mutex);
}

static void queueProcess(threadQueue_t *queue)
{
	assert(queue);

	//SDL_LockMutex(queue->mutex);
	thread_t *first = queue->head;
	if (first == NULL)
	{
		SDL_Delay(1);
		return;
	}
	queue->head = (thread_t*)first->next;
	//SDL_UnlockMutex(queue->mutex);

	assert(first->function);
	first->function(first->data);
	free(first);

}

int loadOpenGlMainContext(void)
{
	openGlContext_t *context = &mainContext;
	context->ok = 0;
#ifdef __X11__
	
	/*//XInitThreads usage creates segmentation fault in the end during SDL_Quit
	if (!XInitThreads())
	{
		debugErrorPrintf("Could not init X11 threads!");
		return 0;
	}*/

	context->display = glXGetCurrentDisplay();
	if (context->display == NULL)
	{
		debugErrorPrintf("Could not get display!");
		return context->ok;
	}

	static int attributeList[] = { GLX_RGBA, None };
	context->visualInfo = glXChooseVisual(context->display, DefaultScreen(context->display), attributeList);
	if (context->visualInfo == NULL)
	{
		debugErrorPrintf("Could not get visual info!");
		return context->ok;
	}

	context->drawable = glXGetCurrentDrawable();
	if (context->drawable == None)
	{
		debugErrorPrintf("Could not get drawable!");
		return context->ok;
	}

	context->context = glXGetCurrentContext();
	if (context->context == NULL)
	{
		debugErrorPrintf("Could not get context!");
		return context->ok;
	}

	context->ok = 1;

#elif WINDOWS
	context->hdc = wglGetCurrentDC();
	if (context->hdc == NULL)
	{
		debugErrorPrintf("Could not get current DC!");
		return context->ok;
	}
	
	context->context = wglGetCurrentContext();
	if (context->context == NULL)
	{
		debugErrorPrintf("Could not get current context!");
		return context->ok;
	}

	context->ok = 1;
#else

	debugErrorPrintf("Functionality not implemented! context:'0x%p'", context);
	context->ok = 0;
#endif

	return context->ok;
}

static int openGlTemporaryContextMakeCurrent(openGlContext_t *mainContext, openGlContext_t *context)
{
	assert(mainContext);
	assert(context);

#ifdef __X11__
	threadGlobalMutexLock();

	if (glXMakeCurrent(mainContext->display, mainContext->drawable, context->context) == False)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not make temporary context current!");
		return 0;
	}

	threadGlobalMutexUnlock();

#elif WINDOWS
	threadGlobalMutexLock();

	if (wglMakeCurrent(mainContext->hdc, context->context) == FALSE)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not make context current!");
		return 0;
	}

	threadGlobalMutexUnlock();

#else
	debugWarningPrintf("Functionality not implemented!");
	return 0;
#endif

	return 1;
}

static int openGlTemporaryContextInit(openGlContext_t *mainContext, openGlContext_t *context)
{
	assert(mainContext);
	assert(context);

#ifdef __X11__
	threadGlobalMutexLock();

	context->context = glXCreateContext(mainContext->display, mainContext->visualInfo, mainContext->context, GL_TRUE);
	if (context->context == NULL)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not create temporary context!");
		return 0;
	}

	//openGlTemporaryContextMakeCurrent(mainContext, context);

	threadGlobalMutexUnlock();

#elif WINDOWS
	threadGlobalMutexLock();

	context->context = wglCreateContext(mainContext->hdc);
	if (context->context == NULL)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not create temporary context!");
		return 0;
	}

	if (wglShareLists(mainContext->context, context->context) == FALSE)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not share lists!");
		return 0;
	}

	//openGlTemporaryContextMakeCurrent(mainContext, context);
		
	threadGlobalMutexUnlock();

#else
	debugWarningPrintf("Functionality not implemented!");
	return 0;
#endif

	return 1;
}

static int openGlTemporaryContextDeinit(openGlContext_t *mainContext, openGlContext_t *context)
{
	assert(mainContext);
	assert(context);

#ifdef __X11__
	threadGlobalMutexLock();

	if (glXMakeCurrent(mainContext->display, mainContext->drawable, NULL) == False)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not make main context current!");
		return 0;
	}

	glXDestroyContext(mainContext->display, context->context);

	threadGlobalMutexUnlock();

#elif WINDOWS
	threadGlobalMutexLock();

	if (wglMakeCurrent(NULL, NULL) == FALSE)
	{
		threadGlobalMutexUnlock();
		debugErrorPrintf("Could not make context current!");
		return 0;
	}

	wglDeleteContext(context->context);

	threadGlobalMutexUnlock();

#else
	debugErrorPrintf("Functionality not implemented!");
	return 0;
#endif

	return 1;
}

static int threadRun(void *data);
void threadQueueInit(void)
{
	if (maxThreads == 0)
	{
		return;
	}

	queueBalancer = 0;
	queues = (threadQueue_t*)malloc(sizeof(threadQueue_t)*maxThreads);
	assert(queues);

	char threadIdString[32];
	int stringLength = 1024;
	char *log = (char*)malloc(sizeof(char)*(stringLength));
	sprintf(log, "Started %d threads: ", maxThreads);

	unsigned int i;
	for(i=0; i < maxThreads; i++)
	{
		threadQueue_t *queue = &queues[i];

		queue->head = NULL;
		queue->tail = NULL;

		queue->active = 1;

		queue->mutex = SDL_CreateMutex();

		openGlTemporaryContextInit(&mainContext, &queue->context);

		queue->sdlThread = SDL_CreateThread(threadRun, (void*)queue);
		assert(queue->sdlThread);

		snprintf(threadIdString, 32, " %X", SDL_GetThreadID(queue->sdlThread));
		strncat(log, threadIdString, stringLength-strlen(log));
	}

	debugPrintf(log);
	free(log);
}

void threadQueueDeinit(void)
{
	unsigned int i;
	for(i=0; i < maxThreads; i++)
	{
		threadQueue_t *queue = &queues[i];

		queue->active = 0;

		int returnValue = 0;
		SDL_WaitThread(queue->sdlThread, &returnValue);
		if (returnValue == -1)
		{
			debugErrorPrintf("Queue '%d' did not exit successfully!", i);
		}

		SDL_DestroyMutex(queue->mutex);
		queue->mutex = NULL;

		assert(queue->head == NULL);
	}

	if (maxThreads > 0)
	{
		debugPrintf("Ended %d threads", maxThreads);
	}

	if (queues)
	{
		free(queues);
		queues = NULL;
	}
}

unsigned int threadGetCurrentId(void)
{
	return SDL_ThreadID();
}

void threadGlobalMutexLock(void)
{
	if (globalMutex != NULL)
	{
		SDL_LockMutex(globalMutex);
	}
}

void threadGlobalMutexUnlock(void)
{
	if (globalMutex != NULL)
	{
		SDL_UnlockMutex(globalMutex);
	}
}

void threadInit(unsigned int threadCount)
{
#ifdef MORPHOS
	maxThreads = 0;
#else
	maxThreads = threadCount;
#endif

	if (!mainContext.ok)
	{
		maxThreads = 0;
	}

	if (maxThreads == 0)
	{
		debugPrintf("No multithreading in use.");
		return;
	}
	else
	{
		debugPrintf("%d threads supported.", maxThreads);
	}

	globalMutex = SDL_CreateMutex();
	assert(globalMutex);

	threadSemaphore = SDL_CreateSemaphore(maxThreads);
	assert(threadSemaphore);
}

void threadDeinit(void)
{
	if (threadSemaphore != NULL)
	{
		SDL_DestroySemaphore(threadSemaphore);
		threadSemaphore = NULL;
	}

	if (globalMutex != NULL)
	{
		SDL_DestroyMutex(globalMutex);
		globalMutex = NULL;
	}
}

static int threadRun(void *data)
{
	if (SDL_SemWait(threadSemaphore) == -1)
	{
		return -1;
	}

	threadQueue_t *queue = (threadQueue_t*)data;
	assert(queue);

	if (!openGlTemporaryContextMakeCurrent(&mainContext, &queue->context))
	{
		return -1;
	}
	
	while(queue->active)
	{
		queueProcess(queue);
	}
	
	glFinish();

	if (!openGlTemporaryContextDeinit(&mainContext, &queue->context))
	{
		return -1;
	}

	SDL_SemPost(threadSemaphore);

	return 0;
}

void threadAsyncCall(void (*function)(void*), void *data)
{
	assert(function);

	if (threadIsEnabled())
	{
		thread_t *threadData = (thread_t*)malloc(sizeof(thread_t));
		assert(threadData);
		threadData->function = function;
		threadData->data = data;
		threadData->next = NULL;

		queueAddThread(threadData);
	}
	else
	{
		//fall-back for no threading
		function(data);
	}
	
}

int threadIsAsyncCallRunning(void)
{
	if (threadIsEnabled())
	{
		unsigned int i;
		for(i=0; i < maxThreads; i++)
		{
			threadQueue_t *queue = &queues[i];
			if (queue->head != NULL)
			{
				return 1;
			}
		}
	}

	return 0;
}

void threadWaitAsyncCalls(void)
{
	while(threadIsAsyncCallRunning())
	{
		playerDrawLoaderBar();
		SDL_Delay(1);
	}
}

int threadIsEnabled(void)
{
	if (maxThreads > 0 && queues)
	{
		return 1;
	}

	return 0;
}

