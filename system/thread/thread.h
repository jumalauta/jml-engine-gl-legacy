#ifndef SYSTEM_THREAD_THREAD_H_
#define SYSTEM_THREAD_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int loadOpenGlMainContext(void);
extern void threadQueueInit(void);
extern void threadQueueDeinit(void);

extern unsigned int threadGetCurrentId(void);
extern void threadGlobalMutexLock(void);
extern void threadGlobalMutexUnlock(void);
extern void threadInit(unsigned int threadCount);
extern void threadDeinit(void);
extern void threadAsyncCall(void (*function)(void*), void *data);
extern int threadIsAsyncCallRunning(void);
extern void threadWaitAsyncCalls(void);
extern int threadIsEnabled(void);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /* SYSTEM_THREAD_THREAD_H_ */
