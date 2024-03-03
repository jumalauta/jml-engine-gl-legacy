#ifdef JAVASCRIPT

#ifndef EXH_SYSTEM_JAVASCRIPT_JAVASCRIPT_H_
#define EXH_SYSTEM_JAVASCRIPT_JAVASCRIPT_H_

extern int jsInit();
extern void jsCallClassMethod(const char *class, const char *method, const char *effectClassName);
extern void jsEvalString(const char *string);
extern void jsEvalFile(const char *file);
extern void jsGarbageCollect();
extern void jsInitEngine();
extern int jsDeinit();

extern void jsAddInputEvent(void* eventPtr);
extern void jsSetUseInput(int _useInput);
extern int jsIsUseInput(void);

void *jsGetDuktapeContext();

#endif /*EXH_SYSTEM_JAVASCRIPT_JAVASCRIPT_H_*/

#endif /*JAVASCRIPT*/
