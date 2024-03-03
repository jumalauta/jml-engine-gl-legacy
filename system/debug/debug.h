#ifndef EXH_SYSTEM_DEBUG_DEBUGPRINT_H_
#define EXH_SYSTEM_DEBUG_DEBUGPRINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_STYLE_SCREEN_LOG 10
#define DEBUG_STYLE_ERROR 3
#define DEBUG_STYLE_WARNING 2
#define DEBUG_STYLE_INFO 1
#define DEBUG_STYLE_DEBUG 0

#ifdef NDEBUG

#define debugPrintf (void)sizeof
#define debugNotePrintf (void)sizeof
#define debugWarningPrintf (void)sizeof
#define debugErrorPrintf (void)sizeof
#define screenPrintf (void)sizeof

#else

#define debugPrintf(...) __debugPrintf(__FILE__, __func__, __LINE__, DEBUG_STYLE_DEBUG, __VA_ARGS__)
#define debugNotePrintf(...) __debugPrintf(__FILE__, __func__, __LINE__, DEBUG_STYLE_INFO, __VA_ARGS__)
#define debugWarningPrintf(...) __debugPrintf(__FILE__, __func__, __LINE__, DEBUG_STYLE_WARNING, __VA_ARGS__)
#define debugErrorPrintf(...) __debugPrintf(__FILE__, __func__, __LINE__, DEBUG_STYLE_ERROR, __VA_ARGS__)
#define screenPrintf(...) __debugPrintf(__FILE__, __func__, __LINE__, DEBUG_STYLE_SCREEN_LOG, __VA_ARGS__)

extern int isDebug();
extern void setDebug(int _debug);
extern void __debugPrintf(const char *fileName, const char *functionName, int sourceLine, int style, const char *fmt, ...);

extern int isOpenGlError(void);
extern int getOpenGlError(void);
extern void printOpenGlErrors(void);
extern void printOpenGlShaderInfo(unsigned int obj);
extern void clearScreenLog();
extern char *getScreenLog();

#endif

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif
